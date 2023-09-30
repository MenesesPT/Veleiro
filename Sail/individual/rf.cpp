#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringSerial.h>

int main() {
  int fd;
  wiringPiSetupSys();
  if ((fd = serialOpen("/dev/ttyAMA1", 9600)) < 0) {
    fprintf(stderr, "Unable to open serial device: %s\n", strerror(errno));
    return 1;
  }

  // serialPuts(fd, "$PMTK251,115200*1F\r\n");
  // Loop, getting and printing characters

  serialPuts(fd, "ping");
  unsigned int time = millis();
  for (;;) {
    if (serialDataAvail(fd) > 0) {
      putchar(serialGetchar(fd));
      printf("%u\n", millis() - time);
      fflush(stdout);
    }
  }
}
