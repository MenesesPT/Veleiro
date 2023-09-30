#ifndef SERVO_H
#define SERVO_H

#define SAILSERVO 24
#define RUDDERSERVO 1

#include <stdio.h>
#include <wiringPi.h>

class Servo {
 private:
  static float sailSetPoint;
  static float rudderSetPoint;

 public:
  static bool setup();
  static float getSail();
  static float getRudder();
  static void setSail(float set_point);
  static void setRudder(float set_point);
};

#endif  // SERVO_H