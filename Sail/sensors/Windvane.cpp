#include "Windvane.hpp"

volatile unsigned int Windvane::last_on = 0;
volatile unsigned int Windvane::last_off = 0;
volatile unsigned int Windvane::on = 1;
volatile bool Windvane::lock = false;
volatile float Windvane::last_measures[WINDVANE_AVG_SIZE];
volatile unsigned short Windvane::last_measures_index = 0;
#ifdef WINDVANE_AUTOCALIBRATE
volatile float Windvane::min_result = 1.0;
volatile float Windvane::max_result = 0.0;
#endif

float Windvane::pwmCalculate() {
  float result = 0.0;
  Windvane::lock = true;
  result = (float)(Windvane::last_off - Windvane::on) / (float)(Windvane::on - Windvane::last_on);
  Windvane::lock = 0;
  // invalid measurement
  if (result < 0.01 || result > 0.99) return -1.0;
#ifdef WINDVANE_AUTOCALIBRATE
  if (Windvane::min_result > result && result > 0.18) Windvane::min_result = result;
  if (Windvane::max_result < result && result < 0.98) Windvane::max_result = result;
#endif
  float newResult = ((result - Windvane::min_result) / (Windvane::max_result - Windvane::min_result)) * 360.0;
  if (newResult > 360.0 || newResult < 0.0) newResult = 0.0;
  return newResult;
}

void Windvane::pwmInterrupt() {
  unsigned short pin = digitalRead(WINDVANE_PIN);
  unsigned int microsec = micros();
  if (Windvane::lock) return;
  if (pin == HIGH) {
    Windvane::last_on = Windvane::on;
    Windvane::on = microsec;
  } else {
    Windvane::last_off = microsec;
    float angle = pwmCalculate();
    if (angle >= -0.0) {
      Windvane::last_measures[last_measures_index] = angle;
      Windvane::last_measures_index++;
      if (Windvane::last_measures_index > WINDVANE_AVG_SIZE) Windvane::last_measures_index = 0;
    }
  }
}

bool Windvane::isCalibrated() { return Windvane::min_result < 0.25 && Windvane::max_result > 0.92; }

bool Windvane::hasFailed() {
  // TODO: Test
  unsigned int microsec = micros();
  return Windvane::last_off + 10000 < microsec;
}
float Windvane::getWindDirection() {
  float real_part = 0, img_part = 0, result;
  int i;
  Windvane::lock = 1;
  for (i = 0; i < WINDVANE_AVG_SIZE; i++) {
    // Conversion to cartesian and sum of each part
    // cos() and sin() expect an argument in radians so
    //  we convert the angles from degrees to radians
    real_part += cos(Windvane::last_measures[i] * M_PI / 180);
    img_part += sin(Windvane::last_measures[i] * M_PI / 180);
  }
  Windvane::lock = 0;
  // Divide by the number of samples to get the average
  real_part /= WINDVANE_AVG_SIZE;
  img_part /= WINDVANE_AVG_SIZE;

  // Convert back to polar and then from radians to degrees
  result = atan2(img_part, real_part) * 180 / M_PI;

  // result can be between -180 and 180, convert to 0, 360
  if (result < 0) result += 360;

  return result;
}

bool Windvane::setup() {
  pinMode(WINDVANE_PIN, INPUT);
  pullUpDnControl(WINDVANE_PIN, PUD_UP);
  wiringPiISR(WINDVANE_PIN, INT_EDGE_BOTH, Windvane::pwmInterrupt);
  for (int i = 0; i < 100; i++) {
    if (Windvane::last_measures_index) return true;
    delay(10);
  }
  return false;
}

float Windvane::calculateWind(float current_hed) {
  float wind = Windvane::getWindDirection();
  if (current_hed - wind < 0) return current_hed - wind + 360.0;
  return current_hed - wind;
}
