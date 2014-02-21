tu60tools
=========

Tools for the TU60 tape drive

The TU60 tools is a command line tool to exercise the TU60 drive on a PDP-11. It shoulw work on any PDP-11 that has the M7892
TA11 board, TU60 drive and at least 16 k memory.

To build requires to make a crosscompiler. 

Download latest binutils from the GNU software repository, put in src directory and build
```
src/configure --target=pdp11-aout --disable-nls
make all
sudo make install
```
Then download latest GCC and build:
```
src/configure --target=pdp11-aout --disable-nls --without-headers --enable-languages=c
make all-gcc
sudo make install-gcc
```
There is a simple build.sh that compiles everything. The compiler is set for using PDP-11/10 instruction set which has 
caused some troubles since there are several bugs generating certain code for certain C constructs. 

The start address is set to 10000. But obviously this can be changed. But please change the SP init in the crt0.s then.

I normally use PDP11GUI to load the code into the machine. I have plans to make an Absolute Binary file to be loaded by an ABS
loader instead since it takes quite long time to load.

There is also a host tool, TU60write to write files to a deccassette from a host computer.
Communication with the host takes place over serial line with CSR 176500.

It consists of source files tu60write.c and hdlcproto_host.c

