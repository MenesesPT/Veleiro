#include <stdio.h>
#include <wiringPiI2C.h>

int readWord(int device, int reg) {
  int high = 0, low = 0;
  high = wiringPiI2CReadReg8(device, reg);
  low = wiringPiI2CReadReg8(device, reg + 1);
  return high << 8 | low;
}

int main() {
  int compass;
  compass = wiringPiI2CSetup(0x60);
  // Versão do software da bussola
  if (wiringPiI2CReadReg8(compass, 0) != 3) {
    printf("Não foi possível comunicar com a bússola!\n");
    return 1;
  }

  printf("BearingSim:  %3.1d\n", wiringPiI2CReadReg8(compass, 1));
  printf("Bearing:     %3.1f\n", readWord(compass, 2) / 10.0);
  printf("Pitch:       %hhdº\n", wiringPiI2CReadReg8(compass, 4));
  printf("Roll:        %hhdº\n", wiringPiI2CReadReg8(compass, 5));
  printf("Temperature: %dºC\n", readWord(compass, 24));
}
