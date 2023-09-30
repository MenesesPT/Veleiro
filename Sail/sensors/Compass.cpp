#include "Compass.hpp"

int Compass::compass;
float Compass::last_heading;
int was_failed = 0;

int Compass::readWord(int reg) {
  int high = 0, low = 0;
  high = wiringPiI2CReadReg8(Compass::compass, reg);
  low = wiringPiI2CReadReg8(Compass::compass, reg + 1);
  return high << 8 | low;
}

float Compass::getHeading() {
  if (!Compass::hasFailed()) Compass::last_heading = readWord(2) / 10.0;
  return Compass::last_heading;
}

signed char Compass::getPitch() { return wiringPiI2CReadReg8(Compass::compass, 4); }

signed char Compass::getRoll() {
  if (Compass::hasFailed()) return 60;
  return wiringPiI2CReadReg8(Compass::compass, 5);
}

int Compass::getTemperature() { return readWord(24); }

bool Compass::hasFailed() {
  if (wiringPiI2CReadReg8(Compass::compass, 1) == -1) {
    was_failed = 50;
    return true;
  } else if (was_failed) {
    was_failed--;
    return true;
  }
  return false;
}

bool Compass::setup() {
  Compass::compass = wiringPiI2CSetup(0x60);
  // Versão do software da bussola (3)
  return wiringPiI2CReadReg8(Compass::compass, 0) == 3;
}