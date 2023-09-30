#include "GPS.hpp"

TinyGPSPlus GPS::gps;
int GPS::fgps = -1;

bool GPS::setup() {
  GPS::fgps = serialOpen(GPS_PATH, GPS_SPEED);
  if (GPS::fgps < 0) {
    fprintf(stderr, "Não foi possível comunicar com o GPS: %s\n", strerror(errno));
    return false;
  }
  for (int i = 0; i < 100; i++) {
    if (serialDataAvail(GPS::fgps)) return true;
    delay(10);
  }
  return false;
}

bool GPS::feed() {
  bool new_data = false;

  // TinyGPSPlus a;
  while (serialDataAvail(GPS::fgps)) {
    GPS::gps.encode(serialGetchar(fgps));
    new_data = true;
  }
  return new_data;
}

bool GPS::hasFailed() { return !GPS::gps.location.isValid() || GPS::gps.location.age() > GPS_FIX_MAX_AGE; }
