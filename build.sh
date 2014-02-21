#!/bin/bash
pdp11-aout-gcc -m10  -Ttext 1000 -msoft-float  -nostartfiles  -nodefaultlibs  -nostdlib   crt0.s printf.c tu60exerciser.c divmulmod.s ashlhi3.s xorhi3.s doReset.s hdlcproto.c
pdp11-aout-objcopy -O binary a.out tu60exerciser_sa10000.bin

