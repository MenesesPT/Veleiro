// readpwm.c

#include <stdio.h>
#include <wiringPi.h>

volatile unsigned int last_on = 0;
volatile unsigned int last_off = 0;
volatile unsigned int on = 0;
volatile char lock = 0;

void pwmIntRising(void) {
  unsigned int microsec = micros();
  if (lock) return;
  last_on = on;
  on = microsec;
}

void pwmIntFalling(void) {
  unsigned int microsec = micros();
  if (lock) return;
  last_off = microsec;
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
  }
}

float pwmCalculate() {
  float result = 0.0;
  lock = 1;
  if (on > last_off) {
    result = (float)(last_off - last_on) / (float)(on - last_on);
    if (result > 1) {
      result = 1.0;
    }
  } else {
    result = (float)(last_off - on) / (float)(on - last_on);
    if (result > 1) {
      result = 0.0;
      // printf("L_ON %u  L_OFF %u ON %u\n", last_on, last_off, on);
      // printf("Num: %u , Den: %u\n\n", (last_off - last_on), (on - last_on));
    }
  }
  lock = 0;
  // result *= 100;
  result -= 0.215;
  result *= 488.5;
  if (result < 0.0 || result > 359.99) result = 0.0;

  // result *= 3.79;
  return result;
}

int main(void) {
  wiringPiSetup();
  pinMode(4, INPUT);
  pullUpDnControl(4, PUD_UP);
  // wiringPiISR(1, INT_EDGE_FALLING, &pwmIntFalling);
  // wiringPiISR(1, INT_EDGE_RISING, &pwmIntRising);
  wiringPiISR(4, INT_EDGE_BOTH, &pwmInt);
  while (1) {
    fflush(stdout);
    printf("\rPWM Read: %6.2f", pwmCalculate());
    delay(250);
  }
  printf("\n");
  return 0;
}