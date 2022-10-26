class Device {
  constructor(id, socket, CdS_count) {
    this._id = id;
    this._socket = socket;
    this._online = true;
    this._threshold = Array(Number(CdS_count)).fill(2500);
  }

  get id() { return this._id; }
  get socket() { return this._socket; }
  get threshold() { return this._threshold }

  set online(status) { this._online = status; }
  set socket(socket) { this._socket = socket; }
  set threshold(threshold) { this._threshold = threshold; }
}

class DevicePool {
  constructor() { this._devices = []; }
  getDeviceById(id)    { return this._devices.find(o => o.id === id); }
  getDeviceBySocket(socket)    { return this._devices.find(o => o.socket.id === socket.id); }
  add(device)   {
    this._devices.push(device);
  }
}

export { Device, DevicePool }