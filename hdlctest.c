#include <stdio.h>
#include <string.h>

char buf [512];
char * bufp =buf;


void writeHdlcFrame (char cmd, char drive, char * buf, int size);
int readHdlcFrame(char * cmd, char * drive, char * buf, int maxSize, int * size);

int main () {
  char cmd, drive, b[256], * str = "HelloWorld!";
  int ret, size;
  writeHdlcFrame (0x01, 0x00, str, strlen(str));
  printf("Reading\n");
  bufp =buf;
  ret = readHdlcFrame(&cmd, &drive, b, 256, &size);
  fprintf(stderr, "ret=%d cmd=%d drive=%d buf=%s size=%d\n", ret, cmd & 0xff, drive & 0xff, b, size); 
  return 0;
}



void writeSerialChar (char ch) {
  fprintf (stderr, "CH=%02X ", 0xff & ch);
  (*bufp++)=ch;
} 


char readSerialChar () {
  fprintf (stderr, "ch=%02X ", 0xff & *bufp);
  return *bufp++;
}

