import SerialPort from 'serialport';
import Readline from '@serialport/parser-readline';
import { sleep } from './utils.js';
const sleepTIME = 3000;
export class Serial {
  constructor() {
    this.path = undefined;
    this.port = undefined;
    this.parser = undefined;
    this.callbacks = [];
  }

  async init() {
    await this.connectArduino();
    this.setCallback((data) => console.log('Received data from Arduino:', data));
  }

  async connectArduino() {
    await this.findArduino();
    //Wait for Arduino connection
    while (this.path == undefined) {
      console.warn(`Arduino was not found, trying again in ${sleepTIME / 1000}s...`)
      await sleep(sleepTIME);
      await this.findArduino();
    }
    //Connect to the port
    this.port = new SerialPort(this.path, {
      baudRate: 9600
    });
    this.bindPort();
  }

  bindPort() {
    this.port.on('error', err => console.error("Error on Serial port", err));
    this.port.on('close', async () => {
      console.warn("Lost connection with Arduino, reconnecting...");
      this.path = undefined;
      this.port = undefined;
      this.parser = undefined;
      await sleep(sleepTIME);
      this.connectArduino();
    });

    this.parser = this.port.pipe(new Readline({ delimiter: '\n' }));

    this.parser.on('data', data => {
      this.callbacks.forEach(call => {
        call(data);
      });
    });
  }

  setCallback(func) {
    this.callbacks.push(func);
  }

  async findArduino() {
    try {
      const ports = await SerialPort.list();
      ports.forEach((port) => {
        if (port.manufacturer == 'wch.cn' || port.manufacturer.includes("Arduino")) {
          this.path = port.path;
          console.log(`Found Arduino on port ${port.path}`);
        }
      });
    } catch (error) {
      this.path = undefined;
      console.error(error);
    }
  }
}



