#ifndef GPS_H
#define GPS_H

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <wiringSerial.h>

#include "../include/gps/TinyGPS.h"

#define GPS_PATH "/dev/ttyS0"
#define GPS_SPEED 9600
#define GPS_FIX_MAX_AGE 10000

class GPS {
 private:
  static int fgps;

 public:
  static TinyGPSPlus gps;

  static bool setup();
  static bool feed();
  static bool hasFailed();
};

class Point {
 private:
  double _lat;
  double _lng;

 public:
  Point() : _lat(0.0), _lng(0.0) {}
  Point(double lat, double lng) : _lat(lat), _lng(lng) {}
  double lat() const { return _lat; }
  double lng() const { return _lng; }
  void setLat(double lat) { _lat = lat; }
  void setLng(double lng) { _lng = lng; }
};

#endif  // GPS_H