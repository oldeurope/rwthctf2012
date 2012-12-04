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
