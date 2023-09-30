import express from 'express';
import http from 'http';
import { Server } from 'socket.io';
import { Serial } from './Serial.js';
const app = express();

const httpServer = http.createServer(app);
const options = {
  cors: {
    origin: "*",
    methods: ["GET", "POST"]
  }
};

const io = new Server(httpServer, options);

let coords = { lat: 38.915151042644205, lng: -9.010660423555128 };

app.use(express.static('../www'))
httpServer.listen(3000);
console.log("Listening");

let sock = new Serial();

io.on("connection", socket => {
  socket.emit("Coords", coords);
  console.log("Socket connected");
  socket.on("newNav", (n) => {
    let b = new Buffer(2);
    b[0] = 78; //N
    b[1] = n;
    console.log(b)
    if (!sock.port) return;
    sock.port.write(b);
    sock.port.flush();
  });

  socket.on("newMission", (rs) => {
    if (!sock.port) return;
    let b = new Buffer(2);
    b[0] = 82; //R
    b[1] = rs.length;
    sock.port.write(b);
    rs.forEach(r => {
      var sizeBytes = new Buffer(8);
      sizeBytes.writeFloatLE(r.lat, 0);
      sizeBytes.writeFloatLE(r.lng, 4);
      sock.port.write(sizeBytes);
    });
    sock.port.flush();
  });

  if (!sock.port) return;
  sock.port.write('s');
  sock.port.flush();

});


sock.init().then(
  () => {
    sock.setCallback((data) => {
      try {
        const obj = JSON.parse(data);
        if (obj.hasOwnProperty('lat') && obj.hasOwnProperty('lng')) {
          coords.lat = obj.lat;
          coords.lng = obj.lng;
          io.emit("Coords", coords);
        }
        if (obj.hasOwnProperty('rssi'))
          io.emit("LoRa", obj.rssi);
        if (obj.hasOwnProperty('wind'))
          io.emit("Wind", obj.wind / 10);
        if (obj.hasOwnProperty('hdg'))
          io.emit("HDG", obj.hdg / 10);
        if (obj.hasOwnProperty('tilt'))
          io.emit("Tilt", obj.tilt);
        if (obj.hasOwnProperty('ip'))
          io.emit("ip", obj.ip);
        if (obj.hasOwnProperty('r'))
          io.emit("Radio", obj.r);
        if (obj.hasOwnProperty('nav'))
          io.emit("Nav", obj.nav);
      } catch {

      } finally { }


    });
  }
);



//app.listen(3000);