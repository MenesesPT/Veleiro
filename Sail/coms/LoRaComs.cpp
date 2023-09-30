#include "LoRaComs.hpp"

#include <wiringSerial.h>

#include "../include/lora/lora.h"
#include "../include/lora/packet.h"

LoRaComs::LoRaComs() {
  LoRaComs::_lora = new LoRa(SPI_CHANNEL, SS_PIN, DIO0_PIN, RST_PIN);
  if (LoRaComs::_lora->begin()) {
    LoRaComs::initialized = true;
    LoRaComs::_lora->setFrequency(LoRa::FREQ_868)
        ->setTXPower(2)  // max 14(ANACOM), def 17, boost 20 [dBm]
        ->setSpreadFactor(LoRa::SF_7)
        ->setBandwidth(LoRa::BW_125k)
        ->setCodingRate(LoRa::CR_45)
        ->setSyncWord('s')
        ->setHeaderMode(LoRa::HM_EXPLICIT)
        ->disableCRC();
  }
}

bool LoRaComs::hasFailed() { return !LoRaComs::initialized; }

bool LoRaComs::available() { return LoRaComs::_lora->packetAvailable(); }

size_t LoRaComs::write(LoRaPacket* value) { return LoRaComs::_lora->transmitPacket(value); }

LoRaPacket LoRaComs::read() { return LoRaComs::_lora->receivePacket(); }

unsigned int LoRaComs::lastHeartbeat() { return LoRaComs::last_heartbeat; }