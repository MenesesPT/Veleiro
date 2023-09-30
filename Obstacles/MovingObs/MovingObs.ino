
#include <LoRa.h>
#include <SPI.h>
#include <TinyGPS.h>

#define WAITTIME 10000UL  //Miliseconds

TinyGPS gps;

uint32_t last_send = 0;

const uint8_t TYPE = 'O';
const uint8_t ID = 2U;
float LAT;
float LNG;
uint16_t CRS;
uint16_t SPD;
const float SIZ = 1.0;
uint8_t msg_buf[18];

void setup() {
  // initialize both serial ports:
  Serial.begin(9600);  // SoftSerial port to get GPS data.
  while (!LoRa.begin(8681E5)) {
    delay(10000);
  }
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.setSyncWord(0x73);
  LoRa.setTxPower(2);  //14Anacom 17max
                       //LoRa.enableCrc();
}

void loop() {
  while (Serial.available()) {
    gps.encode(Serial.read());
  }
  if (millis() - last_send > WAITTIME) {
    uint32_t age;
    float crs, spd;
    gps.f_get_position(&LAT, &LNG, &age);
    crs = gps.f_course();
    spd = gps.f_speed_knots();
    if (LAT == TinyGPS::GPS_INVALID_F_ANGLE || LNG == TinyGPS::GPS_INVALID_F_ANGLE || CRS == TinyGPS::GPS_INVALID_F_ANGLE || SPD == TinyGPS::GPS_INVALID_F_SPEED) {
      //dont send
    } else {
      CRS = (uint16_t)(crs * 10);
      SPD = (uint16_t)(spd * 10);
      memcpy(msg_buf, &TYPE, 1);
      memcpy(msg_buf + 1, &ID, 1);
      memcpy(msg_buf + 2, &LAT, 4);
      memcpy(msg_buf + 6, &LNG, 4);
      memcpy(msg_buf + 10, &SIZ, 4);
      memcpy(msg_buf + 14, &CRS, 2);
      memcpy(msg_buf + 16, &SPD, 2);
      LoRa.beginPacket();
      LoRa.write(msg_buf, 18);
      LoRa.endPacket();
      LoRa.sleep();
    }
    last_send = millis();
  }

  delay(100);
}
