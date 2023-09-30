#include "RF.hpp"

#include <wiringSerial.h>

RF::RF() { file = serialOpen(RFSERIAL, RFBAUD); }

bool RF::hasFailed() { return file < 0; }

bool RF::available() { return serialDataAvail(file); }

void RF::write(uint8_t value) { return serialPutchar(file, value); }

uint8_t RF::read() { return serialGetchar(file); }

unsigned int RF::lastHeartbeat() { return last_heartbeat; };