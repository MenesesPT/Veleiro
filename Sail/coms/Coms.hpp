#ifndef COMS_H
#define COMS_H

#define OBSTACLESFILE "/home/pi/obstacles.txt"
#define REMOTECONTROLDELAY 1000

#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>

#include "../logger/Logger.hpp"
#include "../planner.hpp"
#include "../servos/Servo.hpp"
#include "LoRaComs.hpp"
#include "Obstacle.hpp"
#include "RF.hpp"

class Coms {
 public:
  enum inUse { RFinUse, LORAinUse };
  enum navigationModes { MANUAL, AUTO, RTH, HED, LOITER };

 private:
  static LoRaComs lora;
  static RF rf;
  static inUse in_use;
  static std::map<int, Obstacle> obstacles;
  static std::time_t _last_RF;
  static void handleLoRa();
  static void deleteExpiredObstacles();
  static void sendIp();

 public:
  static int setup();
  static bool available();
  static bool manualControl();
  static void handle(uint8_t* mode);
  static void log();
  static void writeObstacles();
  static void sendTelemetry(float lat, float lng, float spd, float wind, float hdg, uint8_t roll, uint8_t nav);
};

#endif  // COMS_H