#ifndef LoRaComs_H
#define LoRaComs_H

#define SPI_CHANNEL 0
#define SS_PIN 6
#define DIO0_PIN 7
#define RST_PIN 0

#include <wiringSerial.h>

#include <cstdint>

#include "../include/lora/lora.h"
#include "../include/lora/packet.h"

class LoRaComs {
 private:
  bool initialized = false;
  unsigned int last_heartbeat = 0;
  LoRa* _lora;

 public:
  LoRaComs();
  bool hasFailed();
  size_t write(LoRaPacket* value);
  bool available();
  LoRaPacket read();
  unsigned int lastHeartbeat();
};

#endif  // LoRaComs_H