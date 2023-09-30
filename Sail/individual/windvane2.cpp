// readpwm.c

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

// #define AUTOCALIBRATE
#define AVG_SIZE 10

volatile unsigned int last_on = 0;
volatile unsigned int last_off = 0;
volatile unsigned int on = 1;
volatile char lock = 0;
volatile float last_measures[AVG_SIZE];
volatile unsigned short last_measures_index = 0;

#ifdef AUTOCALIBRATE
float min_result = 1.0;
float max_result = 0.0;
#else
float min_result = 0.2108;
float max_result = 0.9516;
#endif

float pwmCalculate() {
  float result = 0.0;
  lock = 1;
  result = (float)(last_off - on) / (float)(on - last_on);
  lock = 0;
  // invalid measurement
  if (result < 0.01 || result > 0.99) return -1.0;
#ifdef AUTOCALIBRATE
  if (min_result > result && result > 0.18) min_result = result;
  if (max_result < result && result < 0.98) max_result = result;
#endif
  float newResult = ((result - min_result) / (max_result - min_result)) * 360.0;
  if (newResult > 360.0 || newResult < 0.0) newResult = 0.0;
  return newResult;
}

void pwmInt(void) {
  unsigned short pin = digitalRead(4);
  unsigned int microsec = micros();
  if (lock) return;
  if (pin == HIGH) {
    last_on = on;
    on = microsec;
  } else {
    last_off = microsec;
    float angle = pwmCalculate();
    if (angle >= -0.0) {
      last_measures[last_measures_index] = angle;
      last_measures_index++;
      if (last_measures_index > AVG_SIZE) last_measures_index = 0;
    }
  }
}
// As angles wrap arround doing a simple average does not work
// Instead a conversion to cartesian coordinate system is performed
// Then the average is performed and converted back to polar
float calcAvg() {
  float real_part = 0, img_part = 0, result;
  int i;
  lock = 1;
  for (i = 0; i < AVG_SIZE; i++) {
    // Conversion to cartesian and sum of each part
    // cos() and sin() expect an argument in radians so
    //  we convert the angles from degrees to radians
    real_part += cos(last_measures[i] * M_PI / 180);
    img_part += sin(last_measures[i] * M_PI / 180);
  }
  lock = 0;
  // Divide by the number of samples to get the average
  real_part /= AVG_SIZE;
  img_part /= AVG_SIZE;

  // Convert back to polar and then from radians to degrees
  result = atan2(img_part, real_part) * 180 / M_PI;

  // result can be between -180 and 180, convert to 0, 360
  if (result < 0) result += 360;

  return result;
}

int isCal() { return min_result < 0.25 && max_result > 0.92; }

int main(void) {
  wiringPiSetup();
  pinMode(4, INPUT);
  pullUpDnControl(4, PUD_UP);
  wiringPiISR(4, INT_EDGE_BOTH, &pwmInt);
  while (1) {
    fflush(stdout);
    // printf("\rPWM Read: %6.2f\tMIN:%5.4f-MAX:%5.4f Cal:%c", pwmCalculate(), min_result, max_result, isCal() ? 'Y' : 'N');
    delay(100);

    //printf("\rAvg: %6.2f", calcAvg());
    printf("%6.2f,", calcAvg());
  }
  printf("\n");
  return 0;
}