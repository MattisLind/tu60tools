char readSerialChar();
void writeSerialChar(char);

#define CRC16 0x8005;
unsigned short out;
void computeCrcChar (char  tmp) {
  unsigned short bit_flag;
  int i;
  for (i=0; i<8; i++) {
    bit_flag = out & 0x8000;
    out <<= 1;
    out |= tmp & 1; // item a) work from the least significant bits
    tmp = tmp >> 1;
    if(bit_flag) out ^= CRC16;
  }
}

unsigned short reverseCrcBits()
{
  unsigned short crc = 0;
  unsigned short i = 0x8000;
  unsigned short  j = 0x0001;
  for (; i != 0; i >>=1, j <<= 1) {
    if (i & out) crc |= j;
  }
  return crc;
}


#define WAITING_FOR_FLAG 0
#define WAITING_FOR_END_FLAG 2
#define FLAG_RECEIVED 1
#define END_OF_FRAME 3
#define COMMAND 0
#define SIZELO 1
#define SIZEHI 2
#define DATA 3
#define CRC1 4
#define DRIVE 5
#define CRC2 6

int readHdlcFrame(char * cmd, char * drive, char * buf, int maxSize, int * size) {
  int state = WAITING_FOR_FLAG;
  char ch, sum=0;
  int demuxState=COMMAND, i=0;
  out=0;
  while (state==WAITING_FOR_FLAG) {
    ch = readSerialChar();
    if (ch==0x7e) {
      state = FLAG_RECEIVED;
    }
  }
  while (state==FLAG_RECEIVED) {
    ch = readSerialChar();
    if (ch == 0x7e) { // end of frame 
      state = END_OF_FRAME;
      continue;
    }
    if (ch == 0x7d) { // escape char
      ch = readSerialChar();
      ch ^=0x20;
    }
    switch (demuxState) {
    case COMMAND:
      *cmd = ch;
      computeCrcChar(ch);
      demuxState = DRIVE;
      break;
    case DRIVE:
      *drive = ch;
      computeCrcChar(ch);
      demuxState = SIZELO;
      break;
    case SIZELO:
      *size = (0xff & (int) ch);
      computeCrcChar(ch);
      demuxState= SIZEHI;
      break;
    case SIZEHI:
      *size += (0xff & ((int) ch))*256;
      computeCrcChar(ch); 
      demuxState = DATA;
      if (*size==0) demuxState=CRC1;
      break;
    case DATA:
      buf[i]=ch;
      computeCrcChar(ch); 
      i++;
      if (i > maxSize) {
	return 1;
      }
      if (i == *size) {
	demuxState = CRC1;
      }    
      break;
    case CRC1:
      computeCrcChar(ch); 
      demuxState = CRC2;
      break;
    case CRC2:
      computeCrcChar(ch); 
      state = WAITING_FOR_END_FLAG;
      break;
    }
  }
  do {
    ch = readSerialChar();
  } while (ch != 0x7e);
  return !out;
}

void writeEscapedChar (char ch) {
  if ((ch == 0x7e) || (ch == 0x7d) || ((ch < 0x20) && (ch >= 0x00))) {
    writeSerialChar (0x7d);
    writeSerialChar (ch ^ 0x20);
  }
  else writeSerialChar(ch);
}

void writeHdlcFrame (char cmd, char drive, char * buf, int size) {
  unsigned short crc, tmp; 
  int i;
  out=0;
  // write start of frame 
  writeSerialChar(0x7e);
  // write command byte
  writeEscapedChar(cmd);
  computeCrcChar(cmd); 
  writeEscapedChar(drive);
  computeCrcChar(drive); 
  writeEscapedChar(size & 0xff);
  computeCrcChar(size & 0xff);
  tmp = size;
  for (i=0; i<8; i++) tmp = tmp >> 1;
  writeEscapedChar(tmp & 0xff);
  computeCrcChar(tmp & 0xff);
  for (i=0; i<size; i++) {
    computeCrcChar(buf[i]);
    writeEscapedChar(buf[i]);
  }
  computeCrcChar(0);
  computeCrcChar(0);
  crc=reverseCrcBits();
  tmp = crc;
  writeEscapedChar(crc & 0xff);
  for (i=0; i<8; i++) tmp = tmp >> 1;
  writeEscapedChar(tmp & 0xff);
  writeSerialChar(0x7e);
}

