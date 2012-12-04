var crypto = require('crypto');
var events = require('events');
var fs = require('fs');
var http = require('http');
var sql = require('sqlite3');
var url = require('url');
var util = require('util');

var my_id = crypto.randomBytes(4).readUInt32LE(0);

// ---------- Constants ----------
/** @const */ var DEFAULT_PORT = 8080;
/** @const */ var FLAG_LENGTH = 16;
/** @const */ var MAX_KEY_USE = 10;
/** @const */ var DFA_SEED = 9237503;

/** @const */ var SERVER_REQ_LENGTH = 20;
/** @const */ var SERVER_SET_FLAG = "8";
/** @const */ var SERVER_GET_FLAG = "5";
/** @const */ var SERVER_REPLICATE_FLAG = "3";
/** @const */ var SERVER_GET_ID = "a";
/** @const */ var SERVER_GET_ALL = "f";

// ---------- Higher Order Functions ----------
function map(array, fn) {
  var result = [];
  array.forEach(function(elem) {
    result.push(fn(elem));
  });
  return result;
}

function filter(array, fn) {
  var result = [];
  array.forEach(function(elem) {
    if (fn(elem)) {
      result.push(elem);
    }
  });
  return result;
}

function lfold(array, fn, start) {
  var result = start;
  array.forEach(function(elem) {
    result = fn(result, elem);
  });
  return result;
}

// ---------- Random ----------
/** @constructor */
function XORGen(seed) {
  this.y = seed;
}

XORGen.prototype.gen = function() {
  this.y = this.y ^ (this.y << 13);
  this.y = this.y >> 17;
  this.y = this.y ^ (this.y << 5);
  return this.y;
}

XORGen.prototype.reseed = function() {
  var buffer = crypto.randomBytes(4);
  this.y = buffer.readUInt32LE(0);
}

// ---------- Util ----------
function permutateToDict(list, rng) {
  var unused = list.slice(0);
  var result = {};
  for (var i = 0; i < list.length; i++) {
    var no = rng.gen() % list.length;
    while (unused[no] === undefined) {
      no = (no + 1) % unused.length;
    }
    result[list[i]] = unused[no];
    unused[no] = undefined;
  }
  return result;
}

function permutateToList(list, rng) {
  var copy = list.slice(0);
  var result = [];
  for (var i = 0; i < copy.length; i++) {
    var no = rng.gen() % copy.length;
    while (copy[no] === undefined) {
      no = (no + 1) % copy.length;
    }
    result.push(copy[no]);
    copy[no] = undefined;
  }
  return result;
}

function loadPeers() {
  var data = fs.readFileSync('peers.txt', 'utf8');
  data = filter(data.split('\n'), function(elem) { return typeof(elem) == 'string' && elem.length > 0 });
  return map(data, function(elem) {
    var s = elem.split(' ');
    return {'host': s[0], 'port': parseInt(s[1])};
  });
}

// ---------- Crypto ----------
/** @constructor */
function Crypto(seed) {
  this.rng = new XORGen(seed);
  this.key = this.rng.gen();
  this.uses = 0;
}

Crypto.prototype.setKey = function(key) {
  this.key = key;
}

Crypto.prototype.encrypt = function(message) {
  var mbuf = new Buffer(message);
  for (var i = 0; i < mbuf.length; i++) {
    mbuf[i] = mbuf[i] ^ this.key;
  }
  this.uses = (this.uses + 1) % MAX_KEY_USE;
  if (this.uses == 0) {
    this.key = this.rng.gen();
  }
  return {'key': this.key, 'cipher': mbuf.toString('hex')};
}

Crypto.prototype.decrypt = function(cipher) {
  var mbuf = null;
  try {
    mbuf = new Buffer(cipher, 'hex');
  }
  catch (e) {
    return JSON.stringify(e);
  }
  for (var i = 0; i < mbuf.length; i++) {
    mbuf[i] = mbuf[i] ^ this.key;
  }
  return mbuf.toString('utf8');
}

// ---------- Replicator ----------
/** @constructor */
function Replicator(replicas, dfa) {
  this.peers = loadPeers();
  this.replicas = replicas;
  this.rng = new XORGen(crypto.randomBytes(4).readUInt32LE(0));
  this.dfa = dfa;
}

Replicator.prototype.replicate = function(id, flag) {
  this.peers = loadPeers();
  var abs_replicas = Math.ceil(this.peers.length * this.replicas);
  var rep_nodes = permutateToList(this.peers, this.rng).splice(0, abs_replicas);
  for (var idx in rep_nodes) {
    var peer = rep_nodes[idx];
    util.debug('Replicating ' + id + ': ' + flag + ' to ' + peer.host + ':' + peer.port);
    var request = http.request({ hostname: peer.host,
                                 port: peer.port,
                                 path: '/afd/' + this.dfa.findWordForState(SERVER_REPLICATE_FLAG, SERVER_REQ_LENGTH)
                                               + '?flag=' + flag + '&id=' + id + '&' + (351142).toString(28) + '=' + my_id
                               }, /** @param response {HTTPResponse} */function(response) {
                                    var data = '';
                                    response.on('data', function(chunk) {
                                      if (chunk !== undefined) {
                                        data = data + chunk.toString('utf8');
                                      }
                                    });
                                    response.on('end', function(chunk) {
                                      if (chunk !== undefined) {
                                        data = data + chunk.toString('utf8');
                                      }
                                      console.log('Reply to replicate request: ' + data);
                                    });
                                  });
    request.on('error', function(peer, error) {
      util.error('Error replicating data: ID ' + id + ' to ' + peer.host + ':' + peer.port + ' with error: ' + error.message);
    }.bind(this, peer));
    request.end();
  }
}

// ---------- DB ----------
/** @constructor */
function DB(path, replicator) {
  /** @type {sql.Database} */
  this.db = new sql.Database(path, sql.OPEN_READWRITE | sql.OPEN_CREATE,  this.onOpen.bind(this));
  this.rng = new XORGen(crypto.randomBytes(4).readUInt32LE(0));
  this.crypt = new Crypto(this.rng.gen());
  this.replicator = replicator;
}

util.inherits(DB, require('events').EventEmitter);

DB.prototype.onOpen = function(error) {
  if (error !== null) {
    util.error(error);
    process.exit(43);
    return;
  }

  this.db.run("CREATE TABLE IF NOT EXISTS flags ( flag TEXT, key INT )");
  this['emit']('init');
}

DB.prototype.addFlag = function(flag, callback) {
  var that = this;
  var msg = this.crypt.encrypt(flag);
  this.storeFlag(msg['cipher'], function(error) {
    if (error == null) {
      console.log('Storing: ' + JSON.stringify(msg) + ' at key: ' + this["lastID"]);
      that.replicator.replicate(this["lastID"], msg['cipher']);
      callback(error, this["lastID"], msg["key"]);
    }
    else {
      callback(error, 0, '');
    }
  });
}

DB.prototype.storeFlag = function(flag, callback) {
  this.db.run("INSERT OR IGNORE INTO flags ( rowid, flag, key ) VALUES (?, ?, ?)", [crypto.randomBytes(4).readUInt32LE(0), flag, 0], callback);
}

DB.prototype.replicateFlag = function(id, flag, key, callback) {
  this.db.run("INSERT OR IGNORE INTO flags ( rowid, flag, key ) VALUES (?, ?, ?)", [id, flag, key], callback);
}

DB.prototype.getFlag = function(id, callback) {
  this.db.get("SELECT rowid, flag FROM flags WHERE rowid = ?", [id], callback);
}

DB.prototype.getIDForFlag = function(flag, callback) {
  this.db.get("SELECT rowid, flag, key FROM flags WHERE flag == ?", [flag], callback);
}

// ---------- DFA ----------
/** @constructor */
function DFA(seed) {
  var that = this;
  this.rng = new XORGen(seed);
  this.letters = ['0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'];
  this.start = 'a';
  this.transitions = {};
  map(this.letters, function(letter) {
    that.transitions[letter] = permutateToDict(that.letters, that.rng);
  });
  this.rng.reseed();
}

DFA.prototype.parse = function(word) {
  var state = this.start;
  for (var i = 0; i < word.length; i++) {
    state = this.transitions[state][word[i]];
  }
  return state;
}

DFA.prototype.findWordForState = function(desired_state, length) {
  var word = "";
  var state = this.start;
  for (var i = 0; i < length - 1; i++) {
    var trans = this.letters[this.rng.gen() % this.letters.length];
    word = word + trans;
    state = this.transitions[state][trans];
  }
  for (var trans in this.transitions[state]) {
    if (this.transitions[state][trans] == desired_state) {
      word = word + trans;
      break;
    }
  }
  return word;
}

// ---------- RingBuffer ---------
/** @constructor */
function RingBuffer(size) {
  if (size === undefined) {
    size = 10;
  }
  this.size = size;
  this.buffer = [];
  this.pos = 0;
}

RingBuffer.prototype.add = function(item) {
  this.buffer[this.pos] = item;
  this.pos = (this.pos + 1) % this.size;
}

RingBuffer.prototype.contains = function(item) {
  for (var buf in this.buffer) {
    if (this.buffer[buf] === item) {
      return true;
    }
  }
  return false;
}

// ---------- Server related stuff ----------
/** @constructor */
function Server(port, db_path) {
  this.port = port;
  this.peers = loadPeers();
  this.dfa = new DFA(DFA_SEED);
  this.db = new DB(db_path, new Replicator(.6, this.dfa));
  this.buf = new RingBuffer(100);
}

Server.prototype.onRequest = function(request, response) {
  request.on('end', function() {
    var url_request = url.parse(request.url, true);
    var path = filter(url_request.pathname.split('/'), function(e) { return e != ''; });
    if (path.length == 0) {
      return this.error(response, 404, 'Invalid path');
    }
    if (path[0] == 'afd') {
      if (path.length < 2 || path[1].length < SERVER_REQ_LENGTH) {
        return this.error(response, 404, 'Invalid path');
      }
      try {
        var state = this.dfa.parse(path[1]);
        var mapping = {};
        mapping[SERVER_GET_FLAG] = this.requestGetFlag;
        mapping[SERVER_SET_FLAG] = this.requestSetFlag;
        mapping[SERVER_REPLICATE_FLAG] = this.requestReplicateFlag;
        mapping[SERVER_GET_ID] = this.requestGetMyID;
        mapping[SERVER_GET_ALL] = this.requestGetAllFlags;

        if (mapping[state] === undefined) {
          response.end();
        }
        else {
          this.makeRequest(path[1], mapping[state], request, response, url_request);
        }
      }
      catch (e) {
        util.error(e);
        return this.error(response, 500, 'Internal Server Error');
      }
    }
    else {
      response.end();
    }
  }.bind(this));
}

Server.prototype.makeRequest = function(path, func, request, response, url_request) {
  if (this.buf.contains(path)) {
    response.end(JSON.stringify({status: "failed", message: "I dontz likz replayz"}));
  }
  else {
    this.buf.add(path);
    return func.call(this, request, response, url_request);
  }
}

Server.prototype.requestGetFlag = function(request, response, url_request) {
  var id = parseInt(url_request.query["id"]);
  this.db.getFlag(id, function(error, row) {
    if (error != null) {
      this.error(response, 500, error);
    }
    else if (row !== undefined) {
      response.end(JSON.stringify({"status": "success", "id": row["rowid"], "flag": row["flag"]}));
    }
    else {
      response.end(JSON.stringify({"status": "failed", "error": "No value for this key"}));
    }
  }.bind(this));
}

Server.prototype.requestSetFlag = function(request, response, /** @type {ParsedURL} */url_request) {
  var flag = String(url_request.query['flag']);
  this.db.addFlag(flag, function(error, id, key) {
    if (error == null) {
      response.end(JSON.stringify({"status": "success", "id": id, "key": key}), 'utf8');
    }
    else {
      this.error(response, 500, error);
    }
  }.bind(this));
}

Server.prototype.requestReplicateFlag = function(request, response, /** @type {ParsedURL} */url_request) {
  var flag = String(url_request.query["flag"]);
  var id = parseInt(String(url_request.query["id"]));
  if (id === undefined || flag === undefined) {
    this.error(response, 400, "Bad Request");
  }
  console.log('Replicate: ' + id + ' -> ' + flag);
  this.db.replicateFlag(id, flag, 0, function(error) {
    if (error == null) {
      response.end(JSON.stringify({"status": "success"}));
    }
    else {
      util.error("Error replicating flag: " + error.toString('utf8'));
      this.error(response, 500, error);
    }
  }.bind(this));
}

Server.prototype.requestGetMyID = function(request, response, /** @type {ParsedURL} */url_request) {
  response.end(JSON.stringify({'status': 'success', 'my_id': my_id}));
}

Server.prototype.requestGetAllFlags = function(request, response, /** @type {ParseURL} */url_request) {
  this.db.getAllFlags(function(error, row){
    if (error == null) {
      response.write('ID: ' + row['rowid'] + ' FLAG: ' + row['flag']);
    }
    else {
      response.end();
    }
  }, function(){
    response.end();
  });
}

/**
* @param response {HTTPResponse}
*/
Server.prototype.error = function(response, error_code, error_msg) {
  response.writeHead(error_code, error_msg);
  response.end();
}

Server.prototype.start = function() {
  var server = http.createServer(this.onRequest.bind(this));
  server.listen(port);
}

var EXIT_SUCCESS = 0;
var EXIT_DOWN = 1;
var EXIT_BROKEN = 2;
var EXIT_ERROR = 3;
var EXIT_SQLITE = 42;

var CON_TIMEOUT = 5000;

var FLAG_LENGTH = 16;

var command = process.argv[2];
var ip = process.argv[3];
var flag = process.argv[4];
var port = (process.argv[5] === undefined) ? DEFAULT_PORT : parseInt(process.argv[5]);
var dfa = new DFA(DFA_SEED);
var rng = new XORGen(crypto.randomBytes(4).readUInt32LE(0));
var db = new DB('checker.sqlite', new DummyReplicator());
var peers = loadPeers();

var PEER_READ = Math.ceil(peers.length * 1);

// ---------- Dummy Replicator ----------
function DummyReplicator() {
}

DummyReplicator.prototype.replicate = function(id, flag) {
}

// ---------- core checker functionality ----------

function fix_id(nid) {
  if (typeof nid == 'string') {
    nid = parseInt(nid);
  }

  if (nid >= 0) {
    return nid;
  }

  var s = (nid+1).toString(2);
  var ns = '11'
  for (var i in s) {
    ns += (s[i] == '0') ? '1' : '0';
  }
  return parseInt(ns,2);
}

function gen_flag() {
  var result = '';
  var chars = '01234567890abcdef';
  for (var i = 0; i < FLAG_LENGTH; i++) {
    result += chars.substr(rng.gen() % chars.length, 1);
  }
  return result;
}

function make_request(request_type, query, callback) {
  var req = http.request({
    hostname: ip,
    port: port,
    path: '/afd/' + dfa.findWordForState(request_type, SERVER_REQ_LENGTH) + query
  }, null);
  req.on('response', function(response) {
    var data = '';
    if (response.statusCode != 200) {
      console.log('Server replied with status code: ' + response.statusCode);
      process.exit(EXIT_BROKEN);
    }
    response.on('data', function(chunk) {
      data += (chunk === undefined) ? '' : chunk.toString('utf8');
    });
    response.on('end', function(chunk) {
      data += (chunk === undefined) ? '' : chunk.toString('utf8');
      var reply = {};
      try {
        reply = JSON.parse(data);
      }
      catch (e) {
        console.log('Server responded with invalid JSON');
        console.log(data);
        process.exit(EXIT_BROKEN);
      }
      if (reply["status"] == "success") {
        callback(reply);
      }
      else {
        console.log(reply);
        process.exit(EXIT_BROKEN);
      }
    });
  });
  req.setTimeout(CON_TIMEOUT, function() {
    console.log('Connection timeout');
    process.exit(EXIT_DOWN);
  });
  req.on('error', function(error) {
    console.log(error);
    process.exit(EXIT_DOWN);
  });
  req.end();
}

function get() {
  global.responses = {}
  global.requests = {"pending": PEER_READ};
  global.cur_peers = permutateToList(peers, rng).slice(0,PEER_READ);
  global.id = 0;
  global.key = 0;

  db.getIDForFlag(flag, function(error, row) {
    if (error != null) {
      console.error(error);
      process.exit(EXIT_ERROR);
    }
    if (row == null) {
      console.error('Could not find the id for this flag');
      process.exit(EXIT_ERROR);
    }
    //global.id = fix_id(row['rowid']);
    global.id = row['rowid'];
    global.key = row['key'];
    console.error('GETTING ID: ' + global.id)
    map(cur_peers, doRequest);
  });
}

function createKnownReplicaTable(callback, callback2, callback3) {
  db.db.run("CREATE TABLE IF NOT EXISTS replica ( id INT, flag TEXT, key INT, ip TEXT )", function(error) {
    if (error != null) {
      console.error('ERROR creating Replica-table');
      console.error(error);
      process.exit(EXIT_ERROR);
    }
    callback(callback2, callback3);
  });
}

function createReplica(callback, callback2) {
  var gflag = gen_flag();
  var c = new Crypto(rng);
  var cflag = c.encrypt(cflag);
  var id = rng.gen();
  make_request(SERVER_REPLICATE_FLAG, '?id=' + id + '&flag=' + gflag, function(reply) {
    db.db.run("INSERT OR IGNORE INTO replica ( id, flag, key, ip ) VALUES ( ?, ?, ?, ? )", [], function(error) {
      if (error != null) {
        console.error('ERROR creating Replica');
        console.error(error);
        process.exit(EXIT_ERROR);
      }
      else {
        callback(callback2);
      }
    });
  });
}

function checkReplica(callback) {
  db.db.get("SELECT rowid, * FROM replica WHERE ip = ? ORDER BY RANDOM() LIMIT 1", [ip], function(error, row) {
    if (error != null) {
      console.error('ERROR checking Replica');
      console.error(error);
      process.exit(EXIT_ERROR);
    }
    make_request(SERVER_GET_FLAG, '?id=' + row['id'], function(reply) {
      var c = new Crypto(rng);
      c.setKey(row['key']);
      var dflag = c.decrypt(reply['flag']);
      if (dflag != row['flag']) {
        console.log('You dont replicate? No points for you sir!');
        console.log('This error also appears if someone managed to delete your database');
        db.db.run("DELETE FROM replica WHERE rowid = ?", [row['rowid']], function(error) {
          process.exit(EXIT_BROKEN);
        });
      }
      else {
        callback();
      }
    });
  });
}

function put() {
  make_request(SERVER_SET_FLAG, '?flag=' + flag, function(reply) {
    db.replicateFlag(reply["id"], flag, reply["key"], function(error) {
      if (error === null) {
        //reply['id'] = fix_id(reply['id']);
        console.error('Stored flag ' + flag + ' at id ' + reply['id']);
        process.exit(EXIT_SUCCESS);
      }
      else {
        if (error.code == 'SQLITE_BUSY') {
          console.error('Sqlite is busy, please try again');
          process.exit(EXIT_SQLITE);
        }
        console.error(error);
        process.exit(EXIT_ERROR);
      }
    });
  });
}

function doRequest(peer) {
  var req = http.request({
    hostname: peer.host,
    port: peer.port,
    path: '/afd/' + dfa.findWordForState(SERVER_GET_FLAG, SERVER_REQ_LENGTH) + "?id=" + id
  }, null);
  req.on('response', function(response) {
    var data = '';
    response.on('data', function(chunk) { if (chunk !== undefined) { data += chunk.toString('utf8'); } });
    response.on('end', function(chunk) {
      data += (chunk === undefined) ? '' : chunk.toString('utf8');
      var reply = {};
      try {
        reply = JSON.parse(data);
      }
      catch (e) {
        console.error('Invalid response from : ' + peer.host + ':' + peer.port);
        finishRequest(req, null);
      }
      if (reply['status'] == 'success') {
        finishRequest(req, reply);
      }
      else {
        finishRequest(req, null);
      }
    });
  });
  req.setTimeout(CON_TIMEOUT, finishRequest.bind(null, req, null));
  req.on('error', finishRequest.bind(null, req, null));
  req.end();
}

function finishRequest(cur_request, cur_response, error) {
  global.requests.pending -= 1;
  if (cur_response == null || error !== undefined) {
    //console.error('Error in request to: ' + cur_request._headers['host']);
    if (error !== undefined) {
      console.error(error);
    }
  }
  else {
    var cur_flag = cur_response["flag"];
    if (cur_flag != null) {
      if (global.responses[cur_flag] === undefined) {
        global.responses[cur_flag] = 0;
      }
      global.responses[cur_flag] += 1;
    }
    else {
      console.error('Recieved empty flag from: ' + cur_request._headers['host']);
    }
  }
  if (global.requests.pending == 0) {
    var best_response_idx = undefined;
    console.error(JSON.stringify(global.responses));
    for (var f in global.responses) {
      if (best_response_idx === undefined || global.responses[f] > global.responses[best_response_idx]) {
        best_response_idx = f;
      }
    }
    if (best_response_idx === undefined) {
      console.log('Tested ' + PEER_READ + ' peers and none had your flag');
      process.exit(EXIT_BROKEN);
    }
    var c = new Crypto(rng);
    c.setKey(global.key);
    var dflag = c.decrypt(best_response_idx);
    if (dflag == flag) {
      process.exit(EXIT_SUCCESS);
    }
    else {
      console.log('Bad flag found: ' + new Buffer(dflag).toString('hex'));
      console.error('Expected: ' + flag);
      process.exit(EXIT_BROKEN);
    }
  }
}

process.on('uncaughtException', function(err) {
  if (err.code == 'SQLITE_BUSY') {
    console.error('Please retry, SQLITE busy');
    process.exit(EXIT_SQLITE);
  }
  console.error('Unexpected error:');
  util.error(err);
  process.exit(EXIT_ERROR);
});

db.on('init', function() {
  try {
    if (command == "put") {
      put();
    }
    if (command == "get") {
      get();
    }
  }
  catch (err) {
    console.error('This should not have happened');
    if (err.code == 'SQLITE_BUSY') {
      console.error('Please retry: sqlite is busy');
      process.exit(EXIT_SQLITE);
    }
    console.error(err);
    process.exit(EXIT_ERROR);
  }
});


