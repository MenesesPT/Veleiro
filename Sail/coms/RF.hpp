#ifndef RF_H
#define RF_H

#define RFSERIAL "/dev/ttyAMA1"
#define RFBAUD 9600

#include <wiringSerial.h>

#include <cstdint>

class RF {
 private:
  unsigned int last_heartbeat = 0;

 public:
  int file;
  RF();
  bool hasFailed();
  void write(uint8_t value);
  bool available();
  uint8_t read();
  unsigned int lastHeartbeat();
};

#endif  // RF_H