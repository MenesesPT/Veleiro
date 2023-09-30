#include "Logger.hpp"

std::ofstream Logger::logfile;
std::ofstream Logger::csvfile;

bool Logger::setup() {
  char buffer[100];
  int i = 1;
  sprintf(buffer, LOGS_LOCATION "%d.txt", i);
  while (access(buffer, F_OK) == 0) {
    sprintf(buffer, CSVS_LOCATION "%d.txt", ++i);
  }
  Logger::logfile.open(buffer);
  std::cout << "Writing log to: " << buffer << '\n';

  sprintf(buffer, LOGS_LOCATION "%d.csv", i);
  Logger::csvfile.open(buffer);
  std::cout << "Writing csv to: " << buffer << '\n';

  if (Logger::csvfile.is_open()) {
    // Time,Lat,Lon,Spd,Course,Hdop,Sats,|Next_lat,Next_lon,Windvane,Calculated_wind,Compass_crs,tilt,optimalDir,mode
    Logger::csvfile << "Time,LAT,LNG,Speed,Course,HDOP,Sats,NextLat,NextLng,Wind,CalculatedWind,Compass,Tilt,OptimalCrs,Mode" << std::endl;
  }
  return Logger::logfile.is_open() && Logger::csvfile.is_open();
}

void Logger::logMessage(std::string msg, bool to_screen, int precision) {
  std::ostringstream msg_stream;
  if (GPS::gps.date.isValid()) {
    msg_stream << "[" << GPS::gps.date.year() << "/" << std::setw(2) << std::setfill('0') << (short)GPS::gps.date.month() << "/"
               << std::setw(2) << std::setfill('0') << (short)GPS::gps.date.day() << " - ";
    msg_stream << std::setw(2) << std::setfill('0') << (short)GPS::gps.time.hour() << ":" << std::setw(2) << std::setfill('0')
               << (short)GPS::gps.time.minute() << ":" << std::setw(2) << std::setfill('0') << (short)GPS::gps.time.second() << "] ";
  } else {
    msg_stream << "[0000/00/00 - 00:00:00] ";
  }
  msg_stream << std::fixed << std::setprecision(precision) << msg << std::endl;
  if (to_screen) {
    std::cout << msg_stream.str();
  }
  Logger::logfile << msg_stream.str();
  Logger::logfile.flush();
}

void Logger::logStatus(Point _next_wp, float _calculated_wind, float _opt_crs, float heading, int _mode) {
  // Time,Lat,Lon,Spd,Course,Hdop,Sats,|Next_lat,Next_lon,Windvane,Calculated_wind,Compass_crs,tilt,optimalDir,mode
  std::ostringstream msg_stream;
  msg_stream << GPS::gps.time.value() << ',';
  msg_stream << GPS::gps.location.lat() << ',';
  msg_stream << GPS::gps.location.lng() << ',';
  msg_stream << GPS::gps.speed.knots() << ',';
  msg_stream << GPS::gps.course.deg() << ',';
  msg_stream << GPS::gps.hdop.hdop() << ',';
  msg_stream << GPS::gps.satellites.value() << ',';
  msg_stream << _next_wp.lat() << ',';
  msg_stream << _next_wp.lng() << ',';
  msg_stream << Windvane::getWindDirection() << ',';
  msg_stream << _calculated_wind << ',';
  msg_stream << heading << ',';
  msg_stream << static_cast<int>(Compass::getRoll()) << ',';
  msg_stream << _opt_crs << ',';
  switch (_mode) {
    case 4:
      msg_stream << "Loiter";
      break;
    case 0:
      msg_stream << "Manual";
      break;
    case 1:
      msg_stream << "Auto";
      break;
    case 2:
      msg_stream << "RTS";
      break;

    default:
      msg_stream << "N/A";
      break;
  }
  msg_stream << std::endl;
  Logger::csvfile << msg_stream.str();
  Logger::csvfile.flush();
}