#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <wiringSerial.h>

#include <cstdio>

#include "../include/gps/TinyGPS.h"
#include "../include/lora/lora.h"
#include "../include/lora/packet.h"

#define GPS_PATH "/dev/ttyS0"
#define GPS_SPEED 115200
#define LAT_OFFSET -0.32835113962
#define LON_OFFSET -0.12196427888

#define SPI_CHANNEL 0
#define SS_PIN 6
#define DIO0_PIN 7
#define RST_PIN 0

bool openGPS(int *fgps) {
  *fgps = serialOpen(GPS_PATH, GPS_SPEED);
  return *fgps >= 0;
}

void processGPS(int fgps, TinyGPSPlus *gps) {
  while (serialDataAvail(fgps)) {
    gps->encode(serialGetchar(fgps));
  }
}

bool updatedGPS(TinyGPSPlus *gps) {
  return gps->location.isUpdated() && gps->altitude.isUpdated() && gps->location.isValid() && gps->altitude.isValid();
}

void printGPSinfo(TinyGPSPlus *gps) {
  printf("LAT: %lf\t", gps->location.lat() + LAT_OFFSET);
  printf("LON: %lf\t", gps->location.lng() + LON_OFFSET);
  printf("ALT: %lf\t", gps->altitude.meters());
  printf("AGE: %u\t", gps->location.age());
  printf("TIME: %02u:%02u:%02u\n\r", gps->time.hour(), gps->time.minute(), gps->time.second());
}

bool setupGPS(int *fgps) {
  printf("Setting up GPS...\n");
  if (!openGPS(fgps)) {
    fprintf(stderr, "Comunication with GPS was not possible: %s\n", strerror(errno));
    return false;
  }
  printf("GPS comunication opened. Waiting for valid data...\n");
  return true;
}

void printLoRaInfo(LoRa *lora) {
  printf("- TX power     : %d dB\n", lora->getTXPower());
  printf("- Frequency    : %d Hz\n", lora->getFrequency());
  printf("- Spread factor: %d\n", lora->getSpreadFactor());
  printf("- Bandwidth    : %d Hz\n", lora->bw[lora->getBandwidth()]);
  printf("- Coding Rate  : 4/%d\n", lora->getCodingRate() + 4);
  printf("- Sync word    : 0x%02x\n", lora->getSyncWord());
  printf("- Header mode  : %s\n", lora->getHeaderMode() == LoRa::HM_IMPLICIT ? "Implicit" : "Explicit");
}

size_t transmitLoRaGPS(LoRa *lora, TinyGPSPlus *gps) {
  float lat = gps->location.lat() + LAT_OFFSET;
  float lon = gps->location.lng() + LON_OFFSET;
  unsigned char tmp[2 * sizeof(float)];
  memcpy(tmp, &lat, sizeof(float));
  memcpy(tmp + sizeof(float), &lon, sizeof(float));

  LoRaPacket p(tmp, 2 * sizeof(float));
  size_t bytes = lora->transmitPacket(&p);
  printf(" %d bytes transmitted\n", bytes);
  return bytes;
}

bool setupLoRa(LoRa *lora) {
  printf("Setting up LoRa...\n");

  if (lora->begin()) {
    printf("Configuring radio\n");
    lora->setFrequency(LoRa::FREQ_868)
        ->setTXPower(2)
        ->setSpreadFactor(LoRa::SF_7)
        ->setBandwidth(LoRa::BW_125k)
        ->setCodingRate(LoRa::CR_48)
        ->setSyncWord(0x12)
        ->setHeaderMode(LoRa::HM_EXPLICIT)
        ->enableCRC();
    // RESETS LoRa Board perdi aqui umas belas 2 horas...
    // printf("LoRa setup successful: chipset version 0x%02x\n", lora->version());
    printLoRaInfo(lora);
    return true;
  }
  return false;
}

int main() {
  int fgps = -1, i = 0;
  TinyGPSPlus *gps = new TinyGPSPlus();
  LoRa *lora = new LoRa(SPI_CHANNEL, SS_PIN, DIO0_PIN, RST_PIN);
  wiringPiSetup();
  if (!setupGPS(&fgps)) return 1;
  if (!setupLoRa(lora)) return 2;
  printLoRaInfo(lora);
  while (true) {
    processGPS(fgps, gps);
    if (updatedGPS(gps)) {
      printGPSinfo(gps);
      // Transmit every 5 updates
      if (i == 0) {
        transmitLoRaGPS(lora, gps);
      }
      i++;
      i %= 5;
    }
    delay(200);
  }
  delete (lora);
  delete (gps);
  return 0;
}