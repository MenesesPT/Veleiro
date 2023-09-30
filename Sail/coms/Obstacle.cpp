#include "Obstacle.hpp"

#include <stdint.h>

#include <ctime>
#include <iostream>

Obstacle::Obstacle(uint8_t Id, float Lat, float Lng, float Siz) : _type(Static), _id(Id), _lat(Lat), _lng(Lng), _siz(Siz) {
  _last_update = std::time(0);
}

Obstacle::Obstacle(uint8_t Id, float Lat, float Lng, float Siz, float Crs, float Spd)
    : _type(Dynamic), _id(Id), _lat(Lat), _lng(Lng), _siz(Siz), _crs(Crs), _spd(Spd) {
  _last_update = std::time(0);
}

std::string Obstacle::toString() {
  std::string result = "{\"id\": ";
  result += std::to_string(_id);
  result += ", \"lat\": ";
  result += std::to_string(_lat);
  result += ", \"lng\": ";
  result += std::to_string(_lng);
  result += ", \"man\": 0";
  result += ", \"Tipo\": 0}\n";
  return result;
}

bool Obstacle::isExpired(std::time_t current_time) {
  if (_type != Obstacle::Static) return _last_update + DYNAMIC_OBSTACLE_TTL < current_time;
  return _last_update + STATIC_OBSTACLE_TTL < current_time;
}
