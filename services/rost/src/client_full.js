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

var command = process.argv[2];
if (command == "get" || command == "set" || command == "rep") {
  var dfa = new DFA(DFA_SEED);
  var server = process.argv[3];
  var port = parseInt(process.argv[4]);
  var id = process.argv[5];
  var flag = (command == "set") ? process.argv[5] : process.argv[6];
  var state = SERVER_GET_FLAG;
  var query_string = '?id=' + id;
  if (command == "set") {
    state = SERVER_SET_FLAG;
    query_string = '?flag=' + flag;
  }
  else if (command == "rep") {
    state = SERVER_REPLICATE_FLAG;
    query_string = '?id=' + id + '&flag=' + flag;
  }

  var req = http.request({
    hostname: server,
    port: port,
    path: '/afd/' + dfa.findWordForState(state, SERVER_REQ_LENGTH) + query_string
  }, null);
  req.on('response', function(response) {
    response.on('data', function(chunk) {
      console.log(chunk.toString('utf8'));
    });
  });
  req.end();
}
