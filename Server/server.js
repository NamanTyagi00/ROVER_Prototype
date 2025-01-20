const path = require('path');
const express = require('express');
const WebSocket = require('ws');

const app = express();

const WS_PORT = 8888;
const HTTP_PORT = 8000;
const wsServer = new WebSocket.Server({ port: WS_PORT }, () => console.log(`WS Server is listening at ${WS_PORT}`));

let connectedClients = [];

wsServer.on('connection', (ws, req) => {
  const id = req.headers['sec-websocket-key'];
  connectedClients.push(ws);
  ws.id = id;
  console.log(`Client connected: ${id}`);

  ws.on('message', message => {
    try {
      const data = JSON.parse(message);
      if (data.role) {
        //if data has data.role
        ws.role = data.role;
        console.log(`Client identified with role: ${ws.role}`);
      } else {
        // other json strings
        console.log(data);
      }
    } catch (e) {
      //non JSON Strings
      
      Object.values(connectedClients).forEach((client, i) => {
        if ((client.readyState === WebSocket.OPEN) && (client.role == 'application'||'python' )) {

          client.send(message);
        } else {
          delete connectedClients[client.id];
        }
      });
    }
  });

  ws.on('close', () => {
    console.log(`Client with role ${ws.role} disconnected`);
    delete connectedClients[ws.id];
  });
});
