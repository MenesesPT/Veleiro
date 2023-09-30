#include <stdio.h>

int main() {
  FILE *fp;
  fp = fopen("/dev/servoblaster", "w+");
  fputs("0=00%\n", fp);
  fclose(fp);
  return 0;
}