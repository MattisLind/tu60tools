#include "tu60.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
int serfd;

char readSerialChar () {
  char buf, n;
  do {
    n=read(serfd, &buf, 1);
    usleep( 1 * 1000 );  // wait 1 msec try again
  } while (n==0);  
  //  fprintf (stderr, "RX %02X:\"%c\" ", 0xff & buf, buf);
  return buf;
}

void writeSerialChar (char ch) {
  //fprintf (stderr, "TX=%02x:\"%c\" ", 0xff & ch, ch);
  write(serfd, &ch, 1);
}


void readBlock(char drive, char * buf, int size) {
  fprintf(stderr, "WriteBlock drive=%d size=%d\n",drive, size); 
  writeHdlcFrame (CMD_READ, drive, buf, size);
}


int main (int argc, char *argv[])
{
  int  filefd, ret, opt;
  char b; 
  int n, timeout, portSet=0;
  char cmd;
  char serialPort[256];
  struct stat st;
  int size;
  int framesize;
  int filedes;
  int i;
  char header[32];
  char typeSet=0;
  struct tm tm;
  char dateBuf[24];
  int client =0;
  int maxSize;
  char buf[128];
  char type;
  char * filebuf;
  time_t ctime;
  char tmpbuf[65];
  int tmp;
  char drive=0; 
  int driveSet=0;
  short status;
  int readingFilename;
  char * basestr;
  char readSize [2];

  while ((opt = getopt(argc, argv, "p:d:")) != -1) {
    switch (opt) {
    case 'd':
      drive = atoi(optarg);
      driveSet = 1;
      break;
    case 'p':
      strncpy(serialPort, optarg, 255);
      portSet=1;
      break;
    default: /* '?' */
      fprintf(stderr, "Usage: %s -p serialport -t filetype [-d drive]  filename\n",
	      argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  if (!portSet) {
    fprintf (stderr, "Expected -p <serial port>\n");
    exit(EXIT_FAILURE);
  }
  if (optind >= argc && !client) {
    fprintf(stderr, "Expected argument after options\n");
    exit(EXIT_FAILURE);
  }

  if ((serfd = serInit(serialPort))<0) {
    exit(-1);
  }
    
  sleep(1);

  readSize[0] = 128;
  readSize[1] = 0;
    readBlock(drive, readSize, 2);
    fprintf (stderr, "************ HEJ 2*********\n");
    // receive result
    ret = readHdlcFrame(&cmd,&drive, (char *) &status , 2, &framesize);
    fprintf (stderr, "Received CMD=%d RET=%d drive=%d framesize=%d status=%04X\n", cmd, ret, drive, framesize,status);
    i = 0;
    // read 128 byte data blocks
    do {
      readBlock(drive, filebuf+i, 128);
      i+=128; size-=128;
      // receive result
      fprintf(stderr, "Before readHdlcFrame\n");
      ret = readHdlcFrame(&cmd,&drive, (char *) &status, 2, &framesize);
      fprintf (stderr, "Received CMD=%d RET=%d drive=%d framesize=%d status =%04X\n", cmd, ret, drive, framesize,status);

    }
    while (size > 0);
    
    serClose(serfd);
  close(filefd);
}
