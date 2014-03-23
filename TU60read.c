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
  int i, j;
  char fileName[11];
  char header[32];
  char typeSet=0;
  struct tm tm;
  char dateBuf[24];
  int client =0;
  int maxSize;
  char buf[130];
  char type;
  char * filebuf;
  time_t creationTime;
  char tmpbuf[65];
  int tmp;
  char drive=0; 
  int driveSet=0;
  short status;
  int readingFilename;
  char * basestr;
  char readSize [2];
  short numBlocks;
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
      fprintf(stderr, "Usage: %s -p serialport -t filetype [-d drive]",
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

  readSize[0] = 32;
  readSize[1] = 0;
  // Send command to read header of file
  readBlock(drive, readSize, 2);
  // receive result
  ret = readHdlcFrame(&cmd,&drive, buf , 34, &framesize);
  status = buf[0]<<8 && buf[1];
  fprintf (stderr, "Received CMD=%d RET=%d drive=%d framesize=%d status=%04X\n", cmd, ret, drive, framesize,status);
  for (i=0;i<32;i++) fprintf(stderr, "%02X ", ((int)buf[i+2]) & 0xff);
  // parse the header
  memset (fileName, 0,11);
  j=0;
  for (i=0;i<6;i++) {
    if(buf[i] == ' ') {
      break;
    }
    else {
      fileName[j++] = buf[i];
    }
  }
  fileName[j++] = '.';
  for (i=6;i<9;i++) {
    if(buf[i] == ' ') {
      break;
    }
    else {
      fileName[j++] = buf[i];
    }
  }
  fprintf (stderr, "filname: %s\n", fileName);
  fprintf (stderr, "filetype: %d\n", buf[9]);
  numBlocks = buf[10]<<8 + buf[11];
  fprintf (stderr, "numBlocks: %d\n", numBlocks);
  size = numBlocks * 128;
  filebuf = malloc (size);
  i = 0;
  // parse date
  tm.tm_sec = 0; 
  tm.tm_min = 0; 
  tm.tm_hour = 0;
  tm.tm_isdst = -1;
  tm.tm_year = (buf[18]-0x30) * 10 + (buf[19]-0x30); 
  tm.tm_mon = ((buf[16]-0x30) * 10 + (buf[17]-0x30))-1; 
  tm.tm_mday = (buf[14]-0x30) * 10 + (buf[15]-0x30); 
  creationTime = mktime (&tm);
  fprintf (stderr, "File creation time : %s\n", ctime(&creationTime));
  // read 128 byte data blocks
  do {
    readBlock(drive, buf, 130);
    // receive result
    fprintf(stderr, "Before readHdlcFrame\n");
    ret = readHdlcFrame(&cmd,&drive, buf, 130, &framesize);
    status = buf[0]<<8 && buf[1];
    fprintf (stderr, "Received CMD=%d RET=%d drive=%d framesize=%d status =%04X\n", cmd, ret, drive, framesize,status);
    memcpy(filebuf+i, buf+2,128);
    i+=128; size-=128;
   }
  while (size > 0);  
  serClose(serfd);
  close(filefd);
}
