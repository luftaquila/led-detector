import { Server } from 'socket.io'
import dotenv from 'dotenv'

import { Device, DevicePool } from './device.mjs'

dotenv.config();

// online devices holder
let devices = new DevicePool();

// socket server
const PORT = 10042;
const io = new Server(PORT);
io.sockets.on("connection", socket => {
  if(socket.handshake.query.deviceId) {
    console.log("DEVICE ONLINE: " + socket.handshake.query.deviceId);
    socket.join("device");

    // find device on pool by id
    let target = devices.getDeviceById(socket.handshake.query.deviceId);

    // if device already exists on pool
    if (target) {
      target.online = true;
      target.socket = socket;
    }

    // if device is new to pool
    else devices.add(new Device(socket.handshake.query.deviceId, socket, socket.handshake.query.CdS_count));

    socket.emit("UPDATE_THRESHOLD", { threshold: devices.getDeviceBySocket(socket).threshold });

    socket.on("ANSWER_VALUE", data => {
      console.log("ANSWER_VALUE", data);
      console.log();
    });

    socket.on("ANSWER_STATUS", data => {
      console.log("ANSWER_STATUS", data);
      console.log();
    });

    // do nothing on successful update
    socket.on("OK_UPDATE_THRESHOLD", data => {
      console.log();
      console.log("OK_UPDATE_THRESHOLD", data);
      console.log();
    });

    /* NOT STABLE
    socket.on("disconnect", reason => {
      let target = devices.getDeviceBySocket(socket);
      target.online = false;
    });
    */
  }
});

// tester code
setInterval(() => {
  let target = devices.getDeviceById('0');
  if(target) {
    target.socket.emit("REQUEST_VALUE");
    target.socket.emit("REQUEST_STATUS");

  }
}, 1000);

setInterval(() => {
  let target = devices.getDeviceById('0');
  if(target) {
    target.threshold = [getRandomInt(0, 4095), getRandomInt(0, 4095), getRandomInt(0, 4095), getRandomInt(0, 4095), getRandomInt(0, 4095), getRandomInt(0, 4095)];
    target.socket.emit("UPDATE_THRESHOLD", { threshold: target.threshold });
  }
}, 30000);

function getRandomInt(min, max) {
  min = Math.ceil(min);
  max = Math.floor(max);
  return Math.floor(Math.random() * (max - min + 1)) + min;
}
