#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <stdint.h>

#include <ctime>
#include <iostream>

#define DYNAMIC_OBSTACLE_TTL 60  // segundos
#define STATIC_OBSTACLE_TTL 600  // segundos

class Obstacle {
 public:
  enum type { Static, Dynamic };

 private:
  type _type;
  uint8_t _id;
  float _lat, _lng, _siz, _crs, _spd;
  std::time_t _last_update;

 public:
  Obstacle(uint8_t Id, float Lat, float Lng, float Siz);
  Obstacle(uint8_t Id, float Lat, float Lng, float Siz, float Crs, float Spd);
  std::string toString();
  bool isExpired(std::time_t current_time);
};

#endif  // OBSTACLE_H
