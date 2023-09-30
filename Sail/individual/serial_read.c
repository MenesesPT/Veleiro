#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <wiringSerial.h>

int main() {
  int fd;

  if ((fd = serialOpen("/dev/ttyS0", 115200)) < 0) {
    fprintf(stderr, "Unable to open serial device: %s\n", strerror(errno));
    return 1;
  }

  // serialPuts(fd, "$PMTK251,115200*1F\r\n");
  // Loop, getting and printing characters

  for (;;) {
    putchar(serialGetchar(fd));
    fflush(stdout);
  }
}
