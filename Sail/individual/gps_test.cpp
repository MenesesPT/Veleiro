#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <wiringSerial.h>

#include "../include/gps/TinyGPS.h"

#define GPS_PATH "/dev/ttyS0"
#define GPS_SPEED 9600

TinyGPSPlus gps;

bool openGPS(int *fgps) {
  *fgps = serialOpen(GPS_PATH, GPS_SPEED);
  if (*fgps < 0) {
    fprintf(stderr, "Não foi possível comunicar com o GPS: %s\n", strerror(errno));
    return false;
  }
  return true;
}

int main() {
  wiringPiSetupSys();
  int fgps = -1;
  if (!openGPS(&fgps)) return 1;
  for (;;) {
    if (serialDataAvail(fgps))
      gps.encode(serialGetchar(fgps));
    else {
      if (gps.location.isUpdated() && gps.altitude.isUpdated() && gps.location.isValid() && gps.altitude.isValid()) {
        printf("LAT: %lf\t", gps.location.lat());
        printf("LON: %lf\t", gps.location.lng());
        printf("ALT: %lf\t", gps.altitude.meters());
        printf("AGE: %u\t", gps.location.age());
        printf("BEARING: %lf\t", gps.course.deg());
        printf("TIME: %02u:%02u:%02uZ %02u/%02u/%02u\n\r", gps.time.hour(), gps.time.minute(), gps.time.second(), gps.date.day(),
               gps.date.month(), gps.date.year());
      }
      delay(200);
    }
  }

  return 0;
}