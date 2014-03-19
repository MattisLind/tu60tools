#define printf tfp_printf
#define WFG 000001
#define WRITE 000003
#define READ 000005
#define SRF 000007
#define SRB 000011
#define SFF 000013
#define SFB 000015
#define REWIND 000017
#define NULL ((short) 0)

int putchar (ch)
{
  volatile unsigned short * txcsr = (unsigned short *) 0177564; 
  volatile unsigned short * txbuf = (unsigned short *) 0177566;
  while (!(0200 & *txcsr)) {
  };
  *txbuf = ch;
}

int puts (char * str) {
  char ch;
  while ((ch=*str++)) {
    putchar(ch);
  }
}

char getchar () {
  volatile unsigned short * rxcsr = (unsigned short *) 0177560; 
  volatile unsigned short * rxbuf = (unsigned short *) 0177562;
  while (!(0200 & *rxcsr)) {
  };
  return *rxbuf;

}

//
// return 0 if no data available
// return non zero if data available

int pollConsole () {
  volatile unsigned short * rxcsr = (unsigned short *) 0177560; 
  return ((0200 & *rxcsr)==0200); 
}

unsigned short readTA11CSR () {
  volatile unsigned short * csr = (unsigned short *) 0177500; 
  return *csr;
}

unsigned short readTA11DBUF () {
  volatile unsigned short * dbuf = (unsigned short *) 0177502; 
  return *dbuf;
}

void writeTA11CSR(unsigned short cmd) {
  volatile unsigned short * csr = (unsigned short *) 0177500; 
  *csr = cmd;
}

void writeTA11DBUF(unsigned short data) {
  volatile unsigned short * dbuf = (unsigned short *) 0177502; 
  *dbuf = data;
}

short writeContinuous (short drive) {
  unsigned char data=0;
  unsigned short cmd;
  cmd = drive << 8;
  cmd |= WRITE;
  writeTA11CSR(cmd);
  while (!pollConsole()) {
    // wait for transfer request
    while (!(000200 & readTA11CSR()));
    writeTA11DBUF((unsigned short) data++);
  }
}

short writeBlock (short drive, unsigned char * buf, short size) {
  unsigned short cmd;
  // Wait for unit to become ready
  while (!(000040 & readTA11CSR()));
  cmd = drive << 8;
  cmd |= WRITE;
  writeTA11CSR(cmd);
  while (size>0) {
    // wait for transfer request
    while (!(000200 & readTA11CSR()));
    writeTA11DBUF((unsigned short) *(buf++));
    size--;
  }
  while (!(000200 & readTA11CSR()));
  // do ILBS sequence
  cmd =  readTA11CSR() | 000020;
  writeTA11CSR(cmd);
  while (!(000040 & readTA11CSR()));
  return readTA11CSR();
}

short readContinuous (short drive) {
  unsigned short cmd;
  cmd = drive << 8;
  cmd |= READ;
  printf ("Now writing READ cmd\r\n");
  writeTA11CSR(cmd);
  while (!pollConsole()) {
    // wait for transfer request
    while (!(0100240 & readTA11CSR()));
    if (0100040 & readTA11CSR()) break;
    readTA11DBUF( );
  }
}

short readBlock (short drive, unsigned char * buf, short size) {
  unsigned short cmd;
  // Wait for unit to become ready
  while (!(000040 & readTA11CSR()));
  cmd = drive << 8;
  cmd |= READ;
  printf ("Now writing READ cmd\r\n");
  writeTA11CSR(cmd);
  while (size>0) {
    // wait for transfer request
    while (!(0100240 & readTA11CSR()));
    if (0100040 & readTA11CSR()) break;
    *(buf++) = (unsigned char) readTA11DBUF( );
    size--;
  }
  if (0100040 & readTA11CSR()) {
    printf("Fail: remaining bytes to read size=%d\n", size);
  }
  else {
    while (!(000200 & readTA11CSR()));
    // do ILBS sequence
    cmd =  readTA11CSR() | 000020;
    writeTA11CSR(cmd);
    printf ("Now waiting for READY after read.\r\n");
    while (!(000040 & readTA11CSR()));
  }
  return readTA11CSR();
}

short doSimpleCommand(cmd,drive) {
  // Wait for unit to become ready
  while (!(000040 & readTA11CSR()));
  cmd = cmd | (drive << 8);
  writeTA11CSR(cmd);
  while (!(000040 & readTA11CSR()));
  return readTA11CSR();
}


short spaceBackwardFile (drive) {
  return doSimpleCommand(SRF, drive);
}

short writeFileGap (drive) {
  return doSimpleCommand(WFG, drive);
}

short spaceBackwardBlock (drive) {
  return doSimpleCommand(SRB, drive);
}

short spaceForwardFile (drive) {
  return doSimpleCommand(SFF,drive);
}

short spaceForwardBlock (drive) {
  return doSimpleCommand(SFB,drive);
}

short rewind (drive) {
  return doSimpleCommand(REWIND, drive);
}

void putch (void * p, char c) {
  putchar (c);
}

void enableRxInterrupt() {
  volatile unsigned short * rxcsr = (unsigned short *) 0177560; 
  *rxcsr |= 000100;
}

void disableRxInterrupt() {
  volatile unsigned short * rxcsr = (unsigned short *) 0177560; 
  *rxcsr &= 177677;
}

char readSerialChar (int * ex) {
  volatile unsigned short * rxcsr = (unsigned short *) 0176500; 
  volatile unsigned short * rxbuf = (unsigned short *) 0176502;
  *ex = 0;
  while (!(0200 & *rxcsr)) {
    if (pollConsole()) {
      *ex = 1;
      break;
    }
  };
  return *rxbuf;
}

void writeSerialChar (char ch) {
  volatile unsigned short * txcsr = (unsigned short *) 0176504; 
  volatile unsigned short * txbuf = (unsigned short *) 0176506;
  while (!(0200 & *txcsr)) {
  };
  *txbuf = ch;
}

#define CMD_WRITE 0
#define CMD_WFG 1
#define CMD_SFF 2
#define CMD_SFB 3
#define CMD_SBF 4
#define CMD_SBB 5
#define CMD_READ 6
#define CMD_REWIND 7
#define CMD_WRITE_RESULT (0x80 | CMD_WRITE)
#define CMD_WFG_RESULT (0x80 | CMD_WFG)


int main () {
  init_printf((void *) 0, putch);
  char ch = 0;
  unsigned char writeBuf[129];
  unsigned char readBuf[129];
  char buf[130];
  int ret;
  char cmd;
  short drive = 0, i;
  short patternSize;
  int size;
  int readSize;
  readBuf[128]=0;
  while (ch != 'q' && ch != 'Q') {
    printf ("TU60 Exerciser drive %d\r\n", drive);
    printf ("Rewind tape: 0\r\n");
    printf ("Write block: 1\r\n");
    printf ("Read block: 2\r\n");
    printf ("Write file gap: 3\r\n");
    printf ("Space Forward Block: 4\r\n");
    printf ("Space Backward Block: 5\r\n");
    printf ("Space Forward File: 6\r\n");
    printf ("Space Backward File: 7\r\n");
    printf ("Toggle drive: 8\r\n");
    printf ("Send RESET signal on bus: 9\r\n");
    printf ("Set test pattern: A\r\n");
    printf ("Write continuous: B\r\n");
    printf ("Read continuous: C\r\n");
    printf ("Start remote mode: R\r\n");
    printf ("TU60EXERCISER> ");
    ch = getchar();
    putchar (ch);
    printf("\r\n");
    switch (ch) {
    case '0':
      printf ("Result: %04x\r\n", rewind(drive));
      break;
    case '1':
      printf ("Result: %04x\r\n", writeBlock (drive,writeBuf, patternSize));
      break;
    case '2':
      printf ("Result: %04x\r\n", readBlock (drive, readBuf, patternSize));
      printf ("Read string=%s\r\n", readBuf);
      break;
    case '3':
      printf ("Result: %04x\r\n", writeFileGap (drive));
      break;
    case '4':
      printf ("Result: %04x\r\n", spaceForwardBlock (drive));
      break;
    case '5':
      printf ("Result: %04x\r\n", spaceBackwardBlock (drive));
      break;
    case '6':
      printf ("Result: %04x\r\n", spaceForwardFile (drive));
      break;
    case '7':
      printf ("Result: %04x\r\n", spaceBackwardFile (drive));
      break;
    case '8':
      drive = drive ^ 000001;
      printf ("Selected drive %d\r\n", drive);
      break;
    case '9':
      doReset();
      printf ("Sent RESET signal\r\n");
      break; 
    case 'b':
    case 'B':
      writeContinuous(drive);
      break;
    case 'c':
    case 'C':
      readContinuous(drive);
      break;
    case 'A':
    case 'a':
      for (i=0; i<sizeof (writeBuf); i++){
	writeBuf[i] = 0;
      }
      do {
	patternSize = 0;
	printf ("Enter size of test pattern 0..128 > ");
        do {
	  ch = getchar();
	  if ((ch >= '0') && (ch <= '9' )) {
	    patternSize = patternSize * 10;
	    patternSize += (short) (ch - 0x30);
	    putchar(ch);
	  }
	  else if (ch != '\r') continue;
	}
	while (ch != '\r'); 
      }
      while ((patternSize < 1) || (patternSize>128));
      printf ("\r\n");
      printf ("Enter testpattern, %d bytes > ", patternSize);
      i=0;
      do {
	ch = getchar ();
	if (ch!='\r') writeBuf[i++] = ch;
	putchar(ch);
      } while ((ch != '\r') && (i < patternSize));
      printf ("\r\n");
      printf ("Testpattern: %s \r\n", writeBuf);
      break;
    case 'R':
    case 'r':
      do {
	ret = readHdlcFrame(&cmd, &drive, buf, 128, &size);
	printf("Received cmd=%d drive=%d size=%04X ret=%d\n\r", cmd, drive, size, ret);
	switch (cmd) {
	case CMD_WRITE:
	  ret = writeBlock(drive, buf, size);
	  writeHdlcFrame (cmd | 0x80, drive, &ret, 2);
	  break;
	case CMD_WFG:
	  ret = writeFileGap(drive);
	  writeHdlcFrame (cmd | 0x80, drive, &ret, 2);
	  break;
	case CMD_SFF:
	  ret = spaceForwardFile(drive);
	  writeHdlcFrame (cmd | 0x80, drive, &ret, 2);
	  break;
	case CMD_SBF:
	  ret = spaceBackwardFile(drive);
	  writeHdlcFrame (cmd | 0x80, drive, &ret, 2);
	  break;
	case CMD_SFB:
	  ret = spaceForwardBlock(drive);
	  writeHdlcFrame (cmd | 0x80, drive, &ret, 2);
	  break;
	case CMD_SBB:
	  ret = spaceBackwardBlock(drive);
	  writeHdlcFrame (cmd | 0x80, drive, &ret, 2);
	  break;
	case CMD_READ:
	  if (size == 2) {
	    readSize = buf[0] + 256*buf[1];
	    ret = readBlock(drive,buf+2,readSize);
	    buf[0] = 0xff & ret;
	    for (i=0; i<8; i++) ret = ret >> 1;
	    buf[1] = 0xff & ret;
	    writeHdlcFrame (cmd | 0x80, drive, buf, readSize+2);
	  }
	  break;
	case CMD_REWIND:
	  ret = rewind(drive);
	  writeHdlcFrame (cmd | 0x80, drive, &ret, 2);
	  break;
	default:
	  printf ("Unhandled cmd = %d\r\n", cmd);
	  break;
	}
      } while (1);
      break;
    default:
      continue;
    }
  }
  return 0;
}
