#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <unistd.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "../sensors/Compass.hpp"
#include "../sensors/GPS.hpp"
#include "../sensors/Windvane.hpp"

#define LOGS_LOCATION "Logs/"
#define CSVS_LOCATION "Logs/"

class Logger {
 private:
  static std::ofstream logfile;
  static std::ofstream csvfile;

 public:
  static bool setup();
  static void logMessage(std::string msg, bool to_screen = true, int precision = 2);
  static void logStatus(Point _next_wp, float _calculated_wind, float _opt_crs, float heading, int _mode);
};

#endif  // LOGGER_H