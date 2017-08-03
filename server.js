
const WebSocket = require('ws');

const wss = new WebSocket.Server({ port: 8887 });


function dynamicSort(property) {
    var sortOrder = 1;
    if(property[0] === "-") {
        sortOrder = -1;
        property = property.substr(1);
    }
    return function (a,b) {
        var result = (a[property] < b[property]) ? -1 : (a[property] > b[property]) ? 1 : 0;
        return result * sortOrder;
    }
}

// Broadcast to all.
wss.broadcast = function broadcast(sender, data) {
  wss.clients.forEach(function each(client) {
    if (client.readyState === WebSocket.OPEN) {
        if (client != sender) {
            client.send(data);
        }
    }
  });
};

var cur_id = 0;

var scores = {};


wss.on('connection', function connection(ws) {

  cur_id++;

  if (cur_id == 256) {
      cur_id = 0;
  }
  
  ws.id = cur_id;
  scores[ws.id] = 0;
  ws.send('I'+ws.id);
    
  wss.broadcast(ws, "J" + ws.id);

  ws.on('close', function() {
      wss.broadcast(ws, "L" + ws.id);
      scores[ws.id] = null;
  });

  ws.on('message', function incoming(data) {
    console.log(ws.id + ": " + data);

    // Broadcast to everyone.
    wss.broadcast(ws, data);
    
    if (data.startsWith('K')) {
        var killer = data.slice(1).split(';')[0];
        var killed = data.slice(1).split(';')[1];

        if (scores[killer]) {
            scores[killer]++;
        } else {
            scores[killer] = 1;
        }
        
    }
  });
});
