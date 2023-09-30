#include <cstdio>

#include "../include/lora/lora.h"
#include "../include/lora/packet.h"

#define SPI_CHANNEL 0
#define SS_PIN 6
#define DIO0_PIN 7
#define RST_PIN 0

int main() {
  printf("Setting up LoRa\n");
  LoRa lora(SPI_CHANNEL, SS_PIN, DIO0_PIN, RST_PIN);
  if (lora.begin()) {
    printf("LoRa setup successful: chipset version 0x%02x\n", lora.version());
    printf("Configuring radio\n");
    lora.setFrequency(LoRa::FREQ_868)
        ->setTXPower(2)  // max 14(ANACOM), def 17, boost 20 [dBm]
        ->setSpreadFactor(LoRa::SF_7)
        ->setBandwidth(LoRa::BW_125k)
        ->setCodingRate(LoRa::CR_45)
        ->setSyncWord('s')
        ->setHeaderMode(LoRa::HM_EXPLICIT)
        ->disableCRC();
    printf("  TX power     : %d dB\n", lora.getTXPower());
    printf("  Frequency    : %d Hz\n", lora.getFrequency());
    printf("  Spread factor: %d\n", lora.getSpreadFactor());
    printf("  Bandwidth    : %d Hz\n", lora.bw[lora.getBandwidth()]);
    printf("  Coding Rate  : 4/%d\n", lora.getCodingRate() + 4);
    printf("  Sync word    : 0x%02x\n", lora.getSyncWord());
    printf("  Header mode  : %s\n", lora.getHeaderMode() == LoRa::HM_IMPLICIT ? "Implicit" : "Explicit");
    printf("Receiving...\n");
    while (true) {
      LoRaPacket p = lora.receivePacket();
      printf("Received packet\n");
      printf("  Bytes   : %d\n", p.payloadLength());
      printf("  RSSI    : %d dBm\n", p.getPacketRSSI());
      printf("  SNR     : %.1f dB\n", p.getSNR());
      printf("  Freq err: %d Hz\n", p.getFreqErr());
      printf("  Payload : \n%s\n", p.getPayload());
      return 0;
      unsigned char* msg = p.getPayload();
      for (int i = 0; i < 22; i++) printf("%X ", msg[i]);
      uint8_t ID;
      float LAT, LNG, SIZ, CRS, SPD;
      memcpy(&ID, msg + 1, 1);
      memcpy(&LAT, msg + 2, 4);
      memcpy(&LNG, msg + 6, 4);
      memcpy(&SIZ, msg + 10, 4);
      memcpy(&CRS, msg + 14, 4);
      memcpy(&SPD, msg + 18, 4);
      printf("  Payload : %c %hhu %f %f %f %f %f\n", msg[0], ID, LAT, LNG, SIZ, CRS, SPD);
      printf("\n");
    }
  }
}

// Handle static
// uint8_t ID;
// float LAT, LNG, SIZ;
// memcpy(&ID, msg + 1, 1);
// memcpy(&LAT, msg + 2, 4);
// memcpy(&LNG, msg + 6, 4);
// memcpy(&SIZ, msg + 10, 4);
// printf("  Payload : %u %hhu %f %f %f\n", sizeof(float), ID, LAT, LNG, SIZ);