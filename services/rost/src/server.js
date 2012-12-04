var port = DEFAULT_PORT;
var db_path = 'data.sqlite';
if (!isNaN(parseInt(process.argv[2]))) {
  port = parseInt(process.argv[2]);
  if (process.argv[3] !== undefined) {
    db_path = process.argv[3];
  }
}

var server = new Server(port, db_path);
server.start();
