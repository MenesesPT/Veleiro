#include <LoRa.h>
#include <SPI.h>


uint8_t bufa[18];
uint8_t i_buf = 0;
float lat, lng;
uint16_t wind, hdg;

uint8_t bufb[18];
uint8_t i_bufb = 0;
float latb, lngb;
uint16_t windb, hdgb;

int8_t nav_mode = -1;
bool lora = false;

void onReceive(int packetSize) {
  lora = true;
  char c = (char)LoRa.read();
  switch (c) {
    case 'n':
      Serial.println("{\"change_nav\": true}");
      break;
    case 'r':
      Serial.println("{\"mission\": true}");
      break;
    case 'T':
      {
        while (i_bufb < 15) {
          bufb[i_bufb++] = LoRa.read();
        }
        int rssi = LoRa.packetRssi();
        LoRa.beginPacket();
        LoRa.print('t');
        LoRa.endPacket();
        LoRa.receive();
        i_bufb = 0;
        memcpy(&latb, bufb, 4);
        memcpy(&lngb, bufb + 4, 4);
        memcpy(&windb, bufb + 9, 2);
        memcpy(&hdgb, bufb + 11, 2);
        Serial.print("{\"lat\":");
        Serial.print(latb, 9);
        Serial.print(", \"lng\":");
        Serial.print(lngb, 9);
        Serial.print(", \"spd\":");
        Serial.print(bufb[8]);
        Serial.print(", \"wind\":");
        Serial.print(windb);
        Serial.print(", \"hdg\":");
        Serial.print(hdgb);
        Serial.print(", \"tilt\":");
        Serial.print((int8_t)bufb[13]);
        Serial.print(", \"nav\":");
        Serial.print(bufb[14]);
        Serial.print(", \"rssi\":");  //RF
        Serial.print(rssi);
        Serial.println("}");
      }
      break;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(9600);

  while (!LoRa.begin(8681E5)) {
    delay(10000);
  }
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.setSyncWord(0x73);
  LoRa.setTxPower(2);  //14Anacom 17max
                       //LoRa.enableCrc();
  //Serial1.write('K');
  // register the receive callback
  LoRa.onReceive(onReceive);

  // put the radio into receive mode
  LoRa.receive();

  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  digitalWrite(6, HIGH);
  digitalWrite(7, HIGH);
  digitalWrite(8, HIGH);
  digitalWrite(9, HIGH);
}


void loop() {
  // put your main code here, to run repeatedly:
  uint8_t length = 0;
  if (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case 's':
      case 'S':
        Serial1.write('K');  //keep alive
        break;
      case 'N':
        if (!lora) {
          Serial1.write('N');
          nav_mode = Serial.read();
          Serial1.write(nav_mode);
        } else {
          LoRa.beginPacket();
          LoRa.print('N');
          nav_mode = Serial.read();
          LoRa.print(nav_mode);
          LoRa.endPacket();
          LoRa.receive();
        }
        break;
      case 'R':
        length = Serial.read();
        uint16_t i = length * 8;
        if (!lora) {
          Serial1.write('R');
          Serial1.write(length);
          while (i) {
            while(!Serial.available());
            Serial1.write(Serial.read());
            i--;
          }

        } else {
          LoRa.beginPacket();
          LoRa.print('R');
          LoRa.print(length);
          while (i) {
            while(!Serial.available());
            LoRa.print(Serial.read());
            i--;
          }
          LoRa.endPacket();
          LoRa.receive();
        }
        length = 0;
        break;
    }
  }

  if (Serial1.available()) {
    lora = false;
    char c = Serial1.read();
    switch (c) {
      case 'i':
      case 'I':
        Serial.print("{\"ip\":\"");
        while (!Serial1.available())
          ;
        while (Serial1.available()) {
          Serial.write(Serial1.read());
          delay(5);
        }
        Serial.flush();
        Serial.println("\"}");
        break;
      case 'n':
        Serial.println("{\"change_nav\": true}");
        break;
      case 'r':
        Serial.println("{\"mission\": true}");
        break;
      case 'T':
        {
          while (i_buf < 15) {
            while (!Serial1.available())
              ;
            bufa[i_buf++] = Serial1.read();
          }
          Serial1.write('t');
          i_buf = 0;
          memcpy(&lat, bufa, 4);
          memcpy(&lng, bufa + 4, 4);
          memcpy(&wind, bufa + 9, 2);
          memcpy(&hdg, bufa + 11, 2);
          Serial.print("{\"lat\":");
          Serial.print(lat, 9);
          Serial.print(", \"lng\":");
          Serial.print(lng, 9);
          Serial.print(", \"spd\":");
          Serial.print(bufa[8]);
          Serial.print(", \"wind\":");
          Serial.print(wind);
          Serial.print(", \"hdg\":");
          Serial.print(hdg);
          Serial.print(", \"tilt\":");
          Serial.print((int8_t)bufa[13]);
          Serial.print(", \"nav\":");
          Serial.print(bufa[14]);
          Serial.print(", \"r\":0");  //RF
          Serial.println("}");
        }
        break;
    }
  }

  if (digitalRead(6) == LOW) {
    if (nav_mode != 4) {
      //Loiter
      nav_mode = 4;
      if (!lora) {
        Serial1.write('N');
        Serial1.write(4);
      } else {
        LoRa.beginPacket();
        LoRa.print('N');
        LoRa.print(4);
        LoRa.endPacket();
        LoRa.receive();
      }
    }

  } else if (digitalRead(7) == LOW) {
    if (nav_mode != 0) {
      //Manual
      nav_mode = 0;
      if (!lora) {
        Serial1.write('N');
        Serial1.write(0);
      } else {
        LoRa.beginPacket();
        LoRa.print('N');
        LoRa.print(0);
        LoRa.endPacket();
        LoRa.receive();
      }
    }

  } else if (digitalRead(8) == LOW) {
    if (nav_mode != 1) {
      //Auto
      nav_mode = 1;
      if (!lora) {
        Serial1.write('N');
        Serial1.write(1);
      } else {
        LoRa.beginPacket();
        LoRa.print('N');
        LoRa.print(1);
        LoRa.endPacket();
        LoRa.receive();
      }
    }

  } else if (digitalRead(9) == LOW) {
    if (nav_mode != 2) {
      //RTH
      nav_mode = 2;
      if (!lora) {
        Serial1.write('N');
        Serial1.write(2);
      } else {
        LoRa.beginPacket();
        LoRa.print('N');
        LoRa.print(2);
        LoRa.endPacket();
        LoRa.receive();
      }
    }
  }
  if (nav_mode == 0) {
    int sail = analogRead(A0) >> 4;
    int rudder = analogRead(A1) >> 4;
    if(sail > 64) sail = 64;
    if(rudder > 64) rudder = 64;
    uint8_t value_sail = 0b10000000 + sail;
    uint8_t value_rudder = 0b11000000 + rudder;
    if (lora) {
      LoRa.beginPacket();
      LoRa.print(value_sail);
      LoRa.endPacket();
      delay(200);
      LoRa.beginPacket();
      LoRa.print(value_rudder);
      LoRa.endPacket();
      LoRa.receive();
    } else {
      Serial1.write(value_sail);
      Serial1.write(value_rudder);
    }
    delay(200);
  }
  delay(10);
}
