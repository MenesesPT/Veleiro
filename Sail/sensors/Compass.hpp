#ifndef COMPASS_H
#define COMPASS_H

#include <wiringPiI2C.h>

#define COMPASS_ADDRESS 0x60

class Compass {
 private:
  static float last_heading;
  static int compass;
  static int readWord(int reg);

 public:
  static float getHeading();
  static signed char getPitch();
  static signed char getRoll();
  static int getTemperature();
  static bool setup();
  static bool hasFailed();
};

#endif  // COMPASS_H
