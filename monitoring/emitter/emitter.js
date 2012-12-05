// vim:et:ts=2:sw=2
// 
// Cubism collector for system metrics
// Collects interface stats, CPU/proc numbers, memory, nfconntrack count and
// OpenVPN clients
//
// Author: Johannes Gilger <heipei@hackvalue.de>

process.env.TZ = 'CET';

var util = require("util"),
    cube = require("cube"),
    options = require("./config"),
    spawn = require('child_process').spawn,
    os = require("os");

config = {
  interval: 10
}

var hostname = os.hostname();
var ifaces=os.networkInterfaces();

ifaces.eth0.forEach(function(details){
  if (details.family=='IPv4') {
    hostname = hostname.toLowerCase().replace(/-/g,'') + "_" + details.address.split(".").join("");
    return true;
  }
});

util.log("starting emitter on " + hostname);

var emitter = cube.emitter(options["collector"]);
var counters = {};

function readstats() {
  var interfaces = spawn('grep', ['[[:digit:]]:', '/proc/net/dev']);
  interfaces.stdout.on('data', function(data) {
    var lines = data.toString().split('\n');
    lines.forEach(function(line) {
      var values = line.toString().split(/ +/);

      if(values.length < 18)
        return;

      var iface = values[1].slice(0,-1);

      if(!counters.network) {
        counters.network = {};
      }

      if(!counters.network[iface]) {
          counters.network[iface] = {};
      }

      counters.network[iface].rxb = parseInt(values[2]);
      counters.network[iface].rxp = parseInt(values[3]);
      counters.network[iface].txb = parseInt(values[10]);
      counters.network[iface].txp = parseInt(values[11]);
    });
  });

  var memory = spawn('egrep', ['(Mem|Buffers|Cached)', '/proc/meminfo']);
  memory.stdout.on('data', function(data) {
      var values = data.toString().split(/ +/);
      if(!counters.mem)
        counters.mem = {};

      counters.mem.total = parseInt(values[1]) * 1024;
      counters.mem.free = parseInt(values[3]) * 1024;
      counters.mem.buffers = parseInt(values[5]) * 1024;
      counters.mem.cached = parseInt(values[7]) * 1024;

  });

  var process = spawn('egrep', ['(cpu |procs)', '/proc/stat']);
  process.stdout.on('data', function(data) {
      var values = data.toString().split(/[ \n]/);
      if(!counters.procs)
        counters.procs = {};
      if(!counters.cpu)
        counters.cpu = {};

      counters.cpu.user_jiffies = parseInt(values[2]);
      counters.cpu.nice_jiffies = parseInt(values[3]);
      counters.cpu.system_jiffies = parseInt(values[4]);

      counters.cpu.total_jiffies = parseInt(values[2]) + parseInt(values[3]) + parseInt(values[4]) +parseInt(values[5]) + parseInt(values[6]) + parseInt(values[7]) + parseInt(values[8]);

      counters.procs.running = parseInt(values[13]);
      counters.procs.blocked = parseInt(values[15]);

  });

  var natcount = spawn('/sbin/sysctl', ['-n', '-e', 'net.netfilter.nf_conntrack_count']);
  natcount.stdout.on('data', function(data) {
    if(!counters.network)
      return;
    counters.network.natcount = parseInt(data.toString());
  });

  var vpn = spawn('/bin/egrep', ['-Rhc', '^[[:alnum:]]+.ctf.itsec.rwth-aachen.de', '/etc/openvpn/']);
  vpn.stdout.on('data', function(data) {
    if(!counters.vpn) {
      counters.vpn = {};
    }
    
    counters.vpn.clients_connected = 0;
    var client_counts = data.toString().split('\n');
    for(var idx=0; idx<client_counts.length-1; idx++) {
      counters.vpn.clients_connected = counters.vpn.clients_connected + parseInt(client_counts[idx]);
    }

  });
  vpn.stderr.on('data', function(data) {
    console.log(data.toString());
  });

  if(!counters.procs || !counters.mem || !counters.network) {
    util.log("Nothing to send yet!");
    return;
  }

  util.log("Emitting data: " + JSON.stringify(counters));
  emitter.send({
    type: hostname,
    data: counters
  });
}

setInterval(readstats, config.interval * 1000);

//emitter.close();
util.log("Waiting...");
