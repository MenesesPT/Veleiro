#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wiringPiI2C.h>
#include <wiringSerial.h>

#include "../include/gps/TinyGPS.h"

#define FILE_LOCATION "Logs/"
#define GPS_PATH "/dev/ttyS0"
#define GPS_SPEED 9600

volatile unsigned int last_on = 0;
volatile unsigned int last_off = 0;
volatile unsigned int on = 0;
volatile char lock = 0;

void pwmInt(void) {
  unsigned short pin = digitalRead(1);
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
  do {
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
    if (result < 0.0 || result > 359.99) {
      result = 0.0;
      delay(5);
    }
  } while (result == 0.0);
  // result *= 3.79;
  return result;
}

int readWord(int device, int reg) {
  int high = 0, low = 0;
  high = wiringPiI2CReadReg8(device, reg);
  low = wiringPiI2CReadReg8(device, reg + 1);
  return high << 8 | low;
}

bool openGPS(int *fgps) {
  *fgps = serialOpen(GPS_PATH, GPS_SPEED);
  return *fgps >= 0;
}

void processGPS(int fgps, TinyGPSPlus *gps) {
  while (serialDataAvail(fgps)) {
    gps->encode(serialGetchar(fgps));
  }
}

bool updatedGPS(TinyGPSPlus *gps) {
  return gps->location.isUpdated() && gps->altitude.isUpdated() && gps->location.isValid() && gps->altitude.isValid() && gps->date.day();
}

bool setupGPS(int *fgps) {
  printf("Setting up GPS...\n");
  if (!openGPS(fgps)) {
    fprintf(stderr, "Comunication with GPS was not possible: %s\n", strerror(errno));
    return false;
  }
  printf("GPS comunication opened. Waiting for valid data...\n");
  return true;
}

int main() {
  char buffer[100];
  int i = 1;
  int fgps = -1;
  int compass;
  sprintf(buffer, FILE_LOCATION "%d.csv", i);
  while (access(buffer, F_OK) == 0) {
    sprintf(buffer, FILE_LOCATION "%d.csv", ++i);
  }

  FILE *fp;
  fp = fopen(buffer, "w+");
  if (fp == NULL) {
    perror("Failed");
    return 1;
  }
  printf("Writing to file: ");
  puts(buffer);

  fputs("Date,Time,Lat,Lon,GPSBearing,Bearing,Pitch,Roll,Wind\n", fp);
  fflush(fp);

  TinyGPSPlus *gps = new TinyGPSPlus();
  wiringPiSetup();
  if (!setupGPS(&fgps)) return 2;

  compass = wiringPiI2CSetup(0x60);
  // Versão do software da bussola
  if (wiringPiI2CReadReg8(compass, 0) != 3) {
    printf("Não foi possível comunicar com a bússola!\n");
    return 3;
  }

  pinMode(1, INPUT);
  pullUpDnControl(1, PUD_UP);

  wiringPiISR(1, INT_EDGE_BOTH, &pwmInt);

  while (true) {
    processGPS(fgps, gps);
    if (updatedGPS(gps)) {
      // printGPSinfo(gps);
      gps->altitude.meters();
      fprintf(fp, "%02u/%02u/%02u,", gps->date.day(), gps->date.month(), gps->date.year());
      fprintf(fp, "%02u:%02u:%02uZ,", gps->time.hour(), gps->time.minute(), gps->time.second());
      fprintf(fp, "%lf,", gps->location.lat());
      fprintf(fp, "%lf,", gps->location.lng());
      fprintf(fp, "%lf,", gps->course.deg());
      fprintf(fp, "%3.1f,", readWord(compass, 2) / 10.0);
      fprintf(fp, "%hhd,", wiringPiI2CReadReg8(compass, 4));
      fprintf(fp, "%hhd,", wiringPiI2CReadReg8(compass, 5));
      fprintf(fp, "%6.2f\n", pwmCalculate());
      fflush(fp);
    }
    delay(200);
  }
  delete (gps);

  fclose(fp);
  return 0;
}