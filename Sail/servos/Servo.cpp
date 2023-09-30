#include "Servo.hpp"

float Servo::sailSetPoint = -1;
float Servo::rudderSetPoint = -1;

bool Servo::setup() {
  // wiringPiSetupGpio();
  pinMode(SAILSERVO, PWM_OUTPUT);
  pinMode(RUDDERSERVO, PWM_OUTPUT);
  pwmSetMode(PWM_MODE_MS);
  pwmSetRange(2000);
  pwmSetClock(192);
  // pullUpDnControl(SAILSERVO, PUD_OFF);
  // pullUpDnControl(RUDDERSERVO, PUD_OFF);
  return true;
}

float Servo::getSail() { return Servo::sailSetPoint; }

float Servo::getRudder() { return Servo::rudderSetPoint; }

// Duty cycle limits (typical) 100ms-200ms
void Servo::setSail(float set_point) {
  if (set_point < 0.0 || set_point > 100.0) return;
  Servo::sailSetPoint = set_point;
  // min 60 max 260
  pwmWrite(SAILSERVO, (int)(1900 - set_point));
}

void Servo::setRudder(float set_point) {
  if (set_point < 0.0 || set_point > 100.0) return;
  Servo::rudderSetPoint = set_point;
  // max 250 min 50
  pwmWrite(RUDDERSERVO, (int)(set_point + 100));
}
