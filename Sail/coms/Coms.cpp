#include "Coms.hpp"

#include <arpa/inet.h>   ///< getsockname
#include <errno.h>       ///< errno
#include <netinet/in.h>  ///< sockaddr_in
#include <sys/socket.h>  ///< socket
#include <unistd.h>      ///< close

#include <cstdint>
#include <cstring>  ///< memset
#include <ctime>
#include <fstream>
#include <iostream>
#include <iostream>  ///< cout
#include <iterator>
#include <map>

#include "../logger/Logger.hpp"
#include "../servos/Servo.hpp"
#include "LoRaComs.hpp"
#include "RF.hpp"

LoRaComs Coms::lora;
RF Coms::rf;
Coms::inUse Coms::in_use = Coms::inUse::RFinUse;
std::map<int, Obstacle> Coms::obstacles;
std::time_t Coms::_last_RF = std::time(0);

void Coms::deleteExpiredObstacles() {
  std::map<int, Obstacle>::iterator it;
  std::time_t current_time = std::time(0);
  it = Coms::obstacles.begin();
  while (it != Coms::obstacles.end()) {
    if (it->second.isExpired(current_time)) {
      char idstr[10];
      sprintf(idstr, "%d", it->first);
      Logger::logMessage("Obstacle has expired ID:" + std::string(idstr));
      it = Coms::obstacles.erase(it);
    } else {
      it++;
    }
  }
}

void Coms::writeObstacles() {
  std::ofstream myfile;
  myfile.open(OBSTACLESFILE, std::ios::trunc);
  if (!myfile.is_open()) return;
  Coms::deleteExpiredObstacles();
  std::map<int, Obstacle>::iterator it;
  for (it = Coms::obstacles.begin(); it != Coms::obstacles.end(); it++) {
    myfile << it->second.toString();
  }
  myfile.close();
}

void Coms::handleLoRa() {
  uint8_t ID;
  float LAT, LNG, SIZ, CRS, SPD;
  size_t length;
  unsigned char* msg;
  char idstr[10];
  LoRaPacket p = Coms::lora.read();
  msg = p.getPayload();
  length = p.payloadLength();
  if (msg[0] == 'o' && length == 14) {
    // static obstacle
    memcpy(&ID, msg + 1, 1);
    memcpy(&LAT, msg + 2, 4);
    memcpy(&LNG, msg + 6, 4);
    memcpy(&SIZ, msg + 10, 4);
    Obstacle o = Obstacle(ID, LAT, LNG, SIZ);

    obstacles.insert_or_assign(ID, o);

    sprintf(idstr, "%d", ID);
    Logger::logMessage("Received fixed obstacle packet ID:" + std::string(idstr));

    Coms::writeObstacles();

  } else if (msg[0] == 'O' && length == 22) {
    // moving obstacle
    memcpy(&ID, msg + 1, 1);
    memcpy(&LAT, msg + 2, 4);
    memcpy(&LNG, msg + 6, 4);
    memcpy(&SIZ, msg + 10, 4);
    memcpy(&CRS, msg + 14, 4);
    memcpy(&SPD, msg + 18, 4);

    Obstacle o = Obstacle(ID, LAT, LNG, SIZ, CRS, SPD);

    obstacles.insert_or_assign(ID, o);
    sprintf(idstr, "%d", ID);
    Logger::logMessage("Received mobile obstacle packet ID: " + std::string(idstr));
    Coms::writeObstacles();
  }
  // TODO Check with prints
  return;
}

int Coms::setup() {
  if (Coms::rf.hasFailed()) return 1;
  if (Coms::lora.hasFailed()) return 2;
  return 0;
}
bool Coms::available() { return Coms::rf.available() || Coms::lora.available(); }

void getIp(char* buffer) {
  const char* google_dns_server = "8.8.8.8";
  int dns_port = 53;

  struct sockaddr_in serv;
  int sock = socket(AF_INET, SOCK_DGRAM, 0);

  // Socket could not be created
  if (sock < 0) {
    std::cout << "Socket error" << std::endl;
  }

  memset(&serv, 0, sizeof(serv));
  serv.sin_family = AF_INET;
  serv.sin_addr.s_addr = inet_addr(google_dns_server);
  serv.sin_port = htons(dns_port);

  int err = connect(sock, (const struct sockaddr*)&serv, sizeof(serv));
  if (err < 0) {
    std::cout << "Error number: " << errno << ". Error message: " << strerror(errno) << std::endl;
  }

  struct sockaddr_in name;
  socklen_t namelen = sizeof(name);
  err = getsockname(sock, (struct sockaddr*)&name, &namelen);

  const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, 80);
  if (p != NULL) {
    // std::cout << "Local IP address is: " << buffer << std::endl;
  } else {
    std::cout << "Error number: " << errno << ". Error message: " << strerror(errno) << std::endl;
  }

  close(sock);
}

void Coms::handle(uint8_t* mode) {
  if (Coms::rf.available()) {
    uint8_t value = Coms::rf.read();
    // Confirmar se é controlo remoto
    if ((value & 0b10000000) == 0b10000000) {
      int pos = value & 0b00111111;
      if ((value & 0b01000000) == 0) {
        // Vela
        Servo::setSail(pos * 100.0 / 64);
      } else {
        // Leme
        Servo::setRudder(pos * 100.0 / 64);
      }
    } else {
      char temp[2];
      switch (value) {
        case 'k':
        case 'K':
          Coms::sendIp();
          break;
        case 'a':
          break;
        case 't':
          // received confirmation
          Coms::_last_RF = std::time(0);
          if (Coms::in_use == Coms::inUse::LORAinUse) {
            Coms::in_use = Coms::inUse::RFinUse;
            Logger::logMessage("Received reply on RF, switching to RF");
          }
          break;
        case 'N':
          *mode = Coms::rf.read();
          Coms::rf.write('n');
          break;
        case 'R': {
          Planner::resetWaypoint();
          uint8_t num_waypoints = Coms::rf.read();
          while (num_waypoints) {
            uint8_t b[4];
            float lat, lng;
            for (size_t i = 0; i < 4; i++) {
              b[i] = Coms::rf.read();
            }
            memcpy(&lat, b, 4);
            for (size_t i = 0; i < 4; i++) {
              b[i] = Coms::rf.read();
            }
            memcpy(&lng, b, 4);
            Planner::addWaypoint(lat, lng);
            num_waypoints--;
            std::cout << "Adding waypoint " << lat << ", " << lng << std::endl;
          }
          Coms::rf.write('r');
          Logger::logMessage("New mission uploaded!");
        } break;
        default:
          sprintf(temp, "%d", value);
          Logger::logMessage("Unknown message received: " + std::string(temp));
      }
    }
  }

  if (Coms::lora.available()) handleLoRa();
}

void Coms::sendIp() {
  char ip[80];
  std::string ss;
  getIp(ip);
  ss.append(ip);
  Logger::logMessage("IP requested, sending IP: " + ss);

  Coms::rf.write('i');
  for (int i = 0; ip[i] != '\0'; i++) {
    Coms::rf.write(ip[i]);
  }
}
void Coms::log() {
  // TODO
}

void Coms::sendTelemetry(float lat, float lng, float spd, float wind, float hdg, uint8_t roll, uint8_t nav) {
  uint8_t buf[18];
  buf[0] = 'T';
  memcpy(buf + 1, &lat, 4);
  memcpy(buf + 5, &lng, 4);
  spd *= 10;
  buf[9] = (uint8_t)spd;
  wind *= 10;
  uint16_t new_wind = (uint16_t)wind;
  memcpy(buf + 10, &new_wind, 2);
  hdg *= 10;
  uint16_t new_hdg = (uint16_t)hdg;
  memcpy(buf + 12, &new_hdg, 2);
  buf[14] = roll;
  buf[15] = nav;

  if (Coms::in_use == Coms::inUse::RFinUse) {
    for (size_t i = 0; i < 16; i++) {
      Coms::rf.write(buf[i]);
    }
    if (Coms::_last_RF + 5 < std::time(0)) {
      Coms::in_use = Coms::inUse::LORAinUse;
      Logger::logMessage("No replies on RF, switching to LoRa.");
    }
  } else {
    if (Coms::_last_RF + 15 < std::time(0)) {
      Logger::logMessage("Trying RF coms...");
      for (size_t i = 0; i < 16; i++) {
        Coms::rf.write(buf[i]);
      }
      Coms::_last_RF = std::time(0) - 5;
    } else {
      LoRaPacket p(buf, 16);
      Coms::lora.write(&p);
    }
  }
}