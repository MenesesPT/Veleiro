#ifndef WINDVANE_H
#define WINDVANE_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

// #define WINDVANE_AUTOCALIBRATE
#define WINDVANE_AVG_SIZE 10
#define WINDVANE_PIN 4

// using namespace std;

class Windvane {
 private:
  static volatile unsigned int last_on;
  static volatile unsigned int last_off;
  static volatile unsigned int on;
  static volatile bool lock;
  static volatile float last_measures[WINDVANE_AVG_SIZE];
  static volatile unsigned short last_measures_index;
#ifdef WINDVANE_AUTOCALIBRATE
  static volatile float min_result;
  static volatile float max_result;
#else
  static constexpr float min_result = 0.2128f;
  static constexpr float max_result = 0.9496f;
#endif

  static float pwmCalculate();
  static void pwmInterrupt();

 public:
  static bool setup();
  static bool isCalibrated();
  static bool hasFailed();
  static float getWindDirection();
  static float calculateWind(float current_hed);
};

#endif  // WINDVANE_H
