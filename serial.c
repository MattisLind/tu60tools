#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>

int serInit(char * serialPort ) {
  int serfd;
  struct termios toptions;
  serfd = open(serialPort, O_RDWR |  O_NONBLOCK); 

  if (serfd == -1)  {
    perror("Failed to open serial port ");
    return -1;
  }

  if (tcgetattr(serfd, &toptions) < 0) {
    perror("Couldn't  get terminal attributes");
    return -1;
  }
  cfsetispeed(&toptions, B9600);
  cfsetospeed(&toptions, B9600);
  
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

  tcsetattr(serfd, TCSANOW, &toptions);
  if( tcsetattr(serfd, TCSAFLUSH, &toptions) < 0) {
    perror("Couldn't set terminal attributes");
    return -1;
  }
  sleep(1);
  tcflush(serfd, TCIOFLUSH);
  return serfd;
}


void serClose (fd) {
  tcflush(fd, TCIOFLUSH);
  close(fd);
}
