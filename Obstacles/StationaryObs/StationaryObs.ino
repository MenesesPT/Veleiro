#include <LoRa.h>
#include <SPI.h>

#define WAITTIME 30000UL  //Miliseconds

const uint8_t TYPE = 'o';
const uint8_t ID = 1U;
const float LAT = 39.00000000;
const float LNG = -9.08000000;
const float SIZ = 1.0F;

uint8_t msg_buf[14];

void setup() {
  while (!LoRa.begin(8681E5)) {
    delay(10000);
  }
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.setSyncWord(0x73);
  LoRa.setTxPower(2);  //14Anacom 17max
                       //LoRa.enableCrc();
  memcpy(msg_buf, &TYPE, 1);
  memcpy(msg_buf + 1, &ID, 1);
  memcpy(msg_buf + 2, &LAT, 4);
  memcpy(msg_buf + 6, &LNG, 4);
  memcpy(msg_buf + 10, &SIZ, 4);
}

void loop() {
  // send packet
  LoRa.beginPacket();
  LoRa.write(msg_buf, 14);
  LoRa.endPacket();
  LoRa.sleep();

  delay(WAITTIME);
}
