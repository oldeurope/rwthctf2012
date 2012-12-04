var process = {}
process.argv = []
process.exit = function(){}

var crypto = {}
/** @constructor */
crypto.randomBytes = function(){}
crypto.randomBytes.prototype.readUInt32LE = function(){}

var events = {};
/** @constructor */
events.EventEmitter = function() {}

var fs = {}
fs.readFileSync = function(){}

var http = {}
http.request = function(){}
http.createServer = function(){}
/** @constructor */
http.createServer.prototype.listen = function(){}

/** @constructor */
var HTTPResponse = function(){}
HTTPResponse.prototype.on = function(){}
HTTPResponse.prototype.end = function(){}
HTTPResponse.prototype.writeHead = function(){}

var sql = {}
/** @constructor */
sql.Database = function(){}
sql.Database.prototype.run = function(){}
sql.Database.prototype.get = function(){}
sql.Database.prototype.each = function(){}
sql.OPEN_READWRITE = 0
sql.OPEN_CREATE = 0

function run(){}
function get(){}


var url = {}
url.parse = function(){}

var util = {}
util.debug = function(){}
util.error = function(){}
util.inherits = function(){}

/** @constructor */
var ParsedURL = function(){}
ParsedURL.path = 0;
ParsedURL.query = 0;
