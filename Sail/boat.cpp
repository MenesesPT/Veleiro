#include <stdlib.h>
#include <wiringPi.h>

#include <array>
#include <cmath>  // std::abs
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "Level1.hpp"
#include "coms/Coms.hpp"
#include "logger/Logger.hpp"
#include "planner.hpp"
#include "sensors/Compass.hpp"
#include "sensors/GPS.hpp"
#include "sensors/Windvane.hpp"
#include "servos/Servo.hpp"

#define VALUESFILE "/home/pi/initial_values.txt"

#define LOOPDELAY 100
#define HDOP_REQUIRED_ACC 5

Coms::navigationModes navMode = Coms::navigationModes::MANUAL;

bool GPS_fail = false;
bool Windvane_fail = false;
bool Compass_fail = false;
Point loiter = Point(0, 0);

std::string exec(const char* cmd) {
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

int setup() {
  wiringPiSetup();
  if (!Logger::setup()) {
    Logger::logMessage("Logger failed to initialize!");
    return 1;
  }
  Logger::logMessage("Logger initialized successfully");
  if (!GPS::setup()) {
    Logger::logMessage("GPS failed to initialize!");
    return 2;
  }
  // Get GPS TIME
  for (int i = 0; i < 100; i++) {
    GPS::feed();
    if (GPS::gps.date.isValid()) break;
    delay(10);
  }
  Logger::logMessage("GPS initialized successfully");
  if (!Windvane::setup()) {
    Logger::logMessage("Windvane failed to initialize!");
    return 3;
  }
  Logger::logMessage("Windvane initialized successfully");
  if (!Compass::setup()) {
    Logger::logMessage("Compass failed to initialize!");
    return 4;
  }
  Logger::logMessage("Compass initialized successfully");
  if (!Servo::setup()) {
    Logger::logMessage("Servos failed to initialize!");
    return 5;
  }
  Logger::logMessage("Servos initialized successfully");
  int coms_setup = Coms::setup();
  if (coms_setup == 1) {
    Logger::logMessage("RF failed to initialize!");
    return 6;
  }
  if (coms_setup == 2) {
    Logger::logMessage("LoRa failed to initialize!");
    return 7;
  }
  Logger::logMessage("Coms initialized successfully");
  return 0;
}

void printWindvane() {
  printf("WINDVANE Dir:%6.4f\tCal:%c\tFail:%c\n", Windvane::getWindDirection(), Windvane::isCalibrated() ? 'Y' : 'N',
         Windvane::hasFailed() ? 'Y' : 'N');
  fflush(stdout);
}
void printCompass() {
  printf("COMPASS Dir:%5.1f\tPitch:%3hhdº\tRoll:%3hhdº\tFail:%c\n", Compass::getHeading(), Compass::getPitch(), Compass::getRoll(),
         Compass::hasFailed() ? 'Y' : 'N');
  fflush(stdout);
}
void printGPS() {
  printf("LAT: %lf\t", GPS::gps.location.lat());
  printf("LON: %lf\t", GPS::gps.location.lng());
  printf("ALT: %6.1lf\t", GPS::gps.altitude.meters());
  printf("AGE: %6.3fs\t", GPS::gps.location.age() / (float)1000);
  printf("BEARING: %6.2lf\t", GPS::gps.course.deg());
  printf("SATS: %1ld\t", GPS::gps.satellites.value());
  printf("TIME: %02u:%02u:%02uZ %02u/%02u/%02u\t", GPS::gps.time.hour(), GPS::gps.time.minute(), GPS::gps.time.second(),
         GPS::gps.date.day(), GPS::gps.date.month(), GPS::gps.date.year());
  printf("FAIL: %c\t", GPS::hasFailed() ? 'Y' : 'N');
  printf("HDOP: %1.2f\n", GPS::gps.hdop.hdop());
}

void changeNavMode(Coms::navigationModes newMode) {
  if (navMode != newMode) {
    navMode = newMode;
    std::string mode = "N/A";
    switch (newMode) {
      case Coms::navigationModes::MANUAL:
        mode = "Manual";
        break;
      case Coms::navigationModes::RTH:
        mode = "RTH";
        break;
      case Coms::navigationModes::AUTO:
        mode = "Auto";
        break;
      case Coms::navigationModes::LOITER:
        loiter.setLat(GPS::gps.location.lat());
        loiter.setLng(GPS::gps.location.lng());
        mode = "Loiter";
        break;

      default:
        break;
    }
    Logger::logMessage("Setting new navigation mode: " + mode);
  }
}

void changeNavModeInt(uint8_t mode) {
  switch (mode) {
    case 0:
      changeNavMode(Coms::navigationModes::MANUAL);
      break;
    case 1:
      changeNavMode(Coms::navigationModes::AUTO);
      break;
    case 2:
      changeNavMode(Coms::navigationModes::RTH);
      break;
    case 4:
      changeNavMode(Coms::navigationModes::LOITER);
      break;

    default:
      break;
  }
}

void checkFailures() {
  if (GPS::hasFailed() && !GPS_fail) {
    Logger::logMessage("GPS has failed! Trying to go home...");
    changeNavMode(Coms::navigationModes::RTH);
    GPS_fail = true;
  } else if (!GPS::hasFailed() && GPS_fail) {
    Logger::logMessage("GPS is working again!");
    GPS_fail = false;
  }

  if (Windvane::hasFailed() && !Windvane_fail) {
    Logger::logMessage("Windvane has failed! Using last wind direction...");
    Windvane_fail = true;
  } else if (!Windvane::hasFailed() && Windvane_fail) {
    Logger::logMessage("Windvane is working again!");
    Windvane_fail = false;
  }

  if (Compass::hasFailed() && !Compass_fail) {
    Logger::logMessage("Compass has failed! Using direction from GPS");
    Compass_fail = true;
  } else if (!Compass::hasFailed() && Compass_fail) {
    Logger::logMessage("Compass is working again!");
    Compass_fail = false;
  }
}

void initialize() {
  Logger::logMessage("Setting servos to mid position");
  Servo::setSail(50.0);
  delay(LOOPDELAY);
  Servo::setRudder(50.0);
  delay(LOOPDELAY);

  Logger::logMessage("Waiting for valid GPS signal...");
  // Wait for a valid GPS signal
  while (!GPS::gps.location.isValid()) {
    uint8_t mode = 255;
    Coms::handle(&mode);
    GPS::feed();
    delay(LOOPDELAY);
  }
  Logger::logMessage("A valid GPS signal was found. Waiting for better accuracy...");
  // std::cout << GPS::gps.hdop.value() << std::endl;
  while (!GPS::gps.hdop.isValid() || GPS::gps.hdop.hdop() > HDOP_REQUIRED_ACC || GPS::gps.hdop.hdop() <= 0.0001) {
    std::cout << "\tCurrent HDOP:" << GPS::gps.hdop.hdop() << " (required: <" << HDOP_REQUIRED_ACC << ")" << '\r' << std::flush;
    uint8_t mode = 255;
    Coms::handle(&mode);
    GPS::feed();
    delay(LOOPDELAY);
  }
  Logger::logMessage("An accurate GPS signal was found.");
  std::ostringstream home_pos;
  home_pos << GPS::gps.location.lat() << ", " << GPS::gps.location.lng();
  Logger::logMessage("Setting home position (" + home_pos.str() + ")");
  Planner::setHomePos();
  changeNavMode(Coms::navigationModes::LOITER);
}

void writeLvl2Values(Point p, float heading) {
  //{"Lat": 44.34, "Lng": 56.78, "Speed": 2.4, "Next_WP_Lat": 23.45, "Next_WP_Lng": 67.89, "Wind": 20.0, "Course_From_Last_WP": 90.0}
  std::ofstream myfile;
  myfile.open(VALUESFILE, std::ios::trunc);
  if (!myfile.is_open()) return;
  std::cout.precision(9);
  myfile << "{\"Lat\": " << GPS::gps.location.lat() << ", \"Lng\": " << GPS::gps.location.lng() << ", \"Speed\": " << GPS::gps.speed.knots()
         << ", \"Next_WP_Lat\": " << p.lat() << ", \"Next_WP_Lng\": " << p.lng() << ", \"Wind\": " << Windvane::calculateWind(heading)
         << ", \"Course_From_Last_WP\": " << heading << "}";

  myfile.close();
  std::cout.precision(5);
}

int main() {
  int setup_result = setup();
  float heading = 0;
  if (setup_result) return setup_result;
  Logger::logMessage("Setup completed");
  initialize();

  while (true) {
    delay(LOOPDELAY);
    uint8_t mode = 255;
    double leme, vela;
    Coms::handle(&mode);
    if (mode < 255) {
      changeNavModeInt(mode);
    }
    GPS::feed();
    checkFailures();
    if (Compass_fail)
      heading = GPS::gps.course.deg();
    else
      heading = Compass::getHeading();
    if (GPS::gps.altitude.isUpdated()) {
      if (navMode != Coms::navigationModes::MANUAL) {
        if (navMode == Coms::navigationModes::AUTO) {
          if (Planner::distanceToCurWaypoint() < 5) {
            Logger::logMessage("Arrived at planned waypoint");
            if (Planner::nextWaypoint() >= Planner::numWaypoints()) {
              Logger::logMessage("Mission has finished");
              changeNavMode(Coms::navigationModes::LOITER);
              Planner::resetWaypointNum();
            }
          }
        }

        // TODO: AUTO MODES
        Coms::writeObstacles();
        if (navMode == Coms::navigationModes::RTH)
          writeLvl2Values(Planner::getHome(), heading);
        else if (navMode == Coms::navigationModes::LOITER)
          writeLvl2Values(loiter, heading);
        else
          writeLvl2Values(Planner::getCurrentWaypoint(), heading);
        std::string result = exec("python3 /home/pi/Alexandre/script.py");
        // std::string result = "1500";
        std::cout << "Próximo ângulo: " << result;
        float angulo_correcao = heading - std::stof(result);
        if (angulo_correcao < -180)
          angulo_correcao += 360;
        else if (angulo_correcao > 180)
          angulo_correcao -= 360;
        Level1InferenceEngine(angulo_correcao, std::abs(Windvane::getWindDirection()), std::abs(Compass::getRoll()), &leme, &vela);
        leme -= 1000;
        vela -= 1000;
        leme /= 10;
        vela /= 10;
        // SWITCH
        Servo::setRudder(vela);
        Servo::setSail(leme);
      }
      if (navMode == Coms::navigationModes::RTH)
        Logger::logStatus(Planner::getHome(), Windvane::calculateWind(heading), Planner::headingToTarget(), heading, navMode);
      else if (navMode == Coms::navigationModes::LOITER)
        Logger::logStatus(loiter, Windvane::calculateWind(heading), Planner::headingToTarget(), heading, navMode);
      else
        Logger::logStatus(Planner::getNextWaypoint(), Windvane::calculateWind(heading), Planner::headingToTarget(), heading, navMode);
      Coms::sendTelemetry(GPS::gps.location.lat(), GPS::gps.location.lng(), GPS::gps.speed.knots(), Windvane::calculateWind(heading),
                          heading, Compass::getRoll(), navMode);
      printGPS();
    }
  }

  return 0;
}
