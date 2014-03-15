#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <libgen.h>


int readHdlcFrame(char *, char *, char *, int, int *);
void writeHdlcFrame (char, char, char *, int);
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

#define CMD_WRITE 0
#define CMD_WFG 1
#define CMD_ACK 2
#define CMD_NACK 3
#define CMD_WRITE_RESULT (0x80 | CMD_WRITE)
#define CMD_WFG_RESULT (0x80 | CMD_WFG)

void writeBlock(char drive, char * buf, int size) {
  writeHdlcFrame (CMD_WRITE, drive, buf, size);
}

void writeFileGap(char drive) {
  writeHdlcFrame (CMD_WFG,drive, NULL, 0); 
}

void sendAck(char drive) {
  writeHdlcFrame (CMD_ACK, drive, NULL, 0);
}

void sendNack(char drive) {
  writeHdlcFrame (CMD_NACK, drive, NULL, 0);
}

int main (int argc, char *argv[])
{
  int  filefd, ret, opt;
  struct termios toptions;
  char b; 
  int n, timeout, portSet=0;
  char cmd;
  char serialPort[256];
  char serialPort2[20];
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

  while ((opt = getopt(argc, argv, "t:p:d:")) != -1) {
    switch (opt) {
    case 't':
      type = atoi(optarg);
      typeSet = 1;
      break;
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
  if (!typeSet) {
    fprintf (stderr, "Expected -t <type>\n");
    exit(EXIT_FAILURE);
  }
  if (optind >= argc && !client) {
    fprintf(stderr, "Expected argument after options\n");
    exit(EXIT_FAILURE);
  }

  if (!client) {
    printf("file argument = %s\n", argv[optind]);


    filedes = open (argv[optind], O_RDONLY);
      printf ("0");
    if (filedes == -1) {
      perror("Failed to open file");
      exit(EXIT_FAILURE);
    }
    printf ("1");
    ret = fstat(filedes, &st);
    if (ret != 0) {
      perror("Failed to stat file");
      exit(EXIT_FAILURE);
    }
    size = st.st_size;

    printf ("file size=%d", size);

    filebuf = malloc (size);
    if (filebuf==NULL) {
      perror("Failed to allocate memory");
      exit(EXIT_FAILURE);    
    }

    memset(filebuf, 0, size);

    ret = read (filedes, filebuf, size);
    if ((ret < 0) || (ret != size)) {
      perror ("Failed to read file into memory");
      exit(EXIT_FAILURE);
      }
  }
  fprintf (stderr, "Seriaport=%s\n", serialPort);
  serfd = open(serialPort, O_RDWR |  O_NONBLOCK); 

  if (serfd == -1)  {
    perror("Failed to open serial port ");
    exit(1);
  }

  printf ("Opened serialport  \n");    
  if (tcgetattr(serfd, &toptions) < 0) {
    perror("Couldn't  get terminal attributes");
    exit(1);
  }
  printf ("got attributes\n");    
  cfsetispeed(&toptions, B9600);
  printf ("Setting inspeed\n");    
  cfsetospeed(&toptions, B9600);
  printf ("Setting outspeed \n");    
  
  // 8N1
  toptions.c_cflag &= ~CSIZE;
  toptions.c_cflag |= CS8;
  toptions.c_cflag &= ~PARENB;
  toptions.c_cflag &= ~CSTOPB;
  // no flow control
  toptions.c_cflag &= ~CRTSCTS;

  toptions.c_cflag |= CREAD | CLOCAL;            // turn on READ & ignore ctrl lines
  toptions.c_iflag &= ~(IXON | IXOFF | IXANY);   // turn off s/w flow ctrl

  toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);   //  raw mode
  toptions.c_oflag &= ~OPOST; // raw mode

  toptions.c_cc[VMIN]  = 0;
  toptions.c_cc[VTIME] = 0;

  printf ("Before setting attributes TCSANOW \n");    
  tcsetattr(serfd, TCSANOW, &toptions);
  printf ("Before setting attributes TCSAFLUSH \n");
  if( tcsetattr(serfd, TCSAFLUSH, &toptions) < 0) {
    perror("Couldn't set terminal attributes");
    exit(1);
  }
  printf ("Before sleep\n");
  sleep(1);
  printf ("About to flush\n");
  tcflush(serfd, TCIOFLUSH);
    
  printf ("After flush\n");
  sleep(1);
  printf ("Now transmitting\n");

  memset(header, 32, 0);
  i=0;
    // write 32 byte header block
  basestr = basename (argv[optind]);
  readingFilename=1;
  while(*basestr) {
    if (readingFilename) {
      if (*basestr == '.') {
	readingFilename=0;
	for (;i<6;i++) header[i]=' ';
	i = 0;
	basestr++;
	continue;	
      }
      if (i<6) {
	header[i] = *basestr;
      }
    }
    else {
      if (i<3) {
	header[i+6] = *basestr;
      }
      else break;
    }
    basestr++;
    i++;
  }
  //fprintf(stderr, "filename %s\n", header);


    header[9] = type;
    header[10] = 0; // high byte record length
    header[11] = 0x80; // low byte record length
    header[12] = 0;  // sequence
    header[13] = 0;  // continued
    ctime = st.st_ctime;
    localtime_r(&ctime, &tm);
    sprintf(dateBuf, "%02d%02d%02d",tm.tm_mday, tm.tm_mon+1, tm.tm_year-100 );
    fprintf(stderr, "Date written %s\n", dateBuf);
    strncpy(header+14, dateBuf, 6);
    fprintf (stderr, "***HEADER=");
    for (i=0;i<32;i++) fprintf(stderr, "%02X ", ((int)header[i]) & 0xff);
    fprintf(stderr, "\n");
    fprintf(stderr, "size=%d\n", size);
    writeBlock(drive, header, 32);
    fprintf (stderr, "************ HEJ 2*********\n");
    // receive result
    ret = readHdlcFrame(&cmd,&drive, (char *) &status , 2, &framesize);
    fprintf (stderr, "Received CMD=%d RET=%d drive=%d framesize=%d status=%04X\n", cmd, ret, drive, framesize,status);
    i = 0;
    // write 128 byte data blocks
    do {
      //sleep(1);
      writeBlock(drive, filebuf+i, 128);
      i+=128; size-=128;
      // receive result
      ret = readHdlcFrame(&cmd,&drive, (char *) &status, 2, &framesize);
      fprintf (stderr, "Received CMD=%d RET=%d drive=%d framesize=%d status =%04X\n", cmd, ret, drive, framesize,status);

    }
    while (size > 0);
    
    // write file gap
    writeFileGap(drive);
         // receive result
    ret = readHdlcFrame(&cmd,&drive, (char *) &status, 2, &framesize);
    fprintf (stderr, "Received CMD=%d RET=%d drive=%d framesize=%d status =%04X\n", cmd, ret, drive, framesize,status);
  tcflush(serfd, TCIOFLUSH);
  close(serfd);
  close(filefd);
}
