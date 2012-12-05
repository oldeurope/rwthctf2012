// vim:ts=2:et:sw=2:
// ctfgibson eventserver
// Listen for incoming messages on UDP
// Distribute messages to connected TCP clients

net = require('net');
dgram = require('dgram')

var clients = [];

// Send a message to all clients
function broadcast(message, sender) {
  clients.forEach(function (client) {
    if (client === sender) return;
    client.write(message);
  });

  process.stdout.write(".");
}

listen_socket = dgram.createSocket("udp4");
listen_socket.addListener('message', function (msg, rinfo) {
  //console.log('got message from '+ rinfo.address +' port: '+ rinfo.port);
  //console.log('data len: '+ rinfo.size + " data: "+ msg.toString('ascii', 0, rinfo.size));
  broadcast(msg+"\n", null); 
});
listen_socket.bind(8000);

// Start a TCP Server
net.createServer(function (socket) {

  socket.name = socket.remoteAddress + ":" + socket.remotePort 

  clients.push(socket);

  socket.on('end', function () {
    clients.splice(clients.indexOf(socket), 1);
  });

  socket.on('error', function(exception){
    process.stdout.write("!");
  });
  
}).listen(5000);

// Put a friendly message on the terminal of the server.
console.log("ctfgibson\n\nFirehose: TCP:5000\nInput: UDP:8000");
