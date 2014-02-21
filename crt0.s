	.text
	.globl _main, ___main, _start

_start:
	mov	$010000,sp
	mov	$_rxint,000060
	mov     $0,000062
	jsr	pc, _main
	halt

___main:
	rts	pc

_rxint:
	jsr 	pc, _rxserv
	rti
	