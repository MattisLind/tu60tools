	.text
	.globl _main, ___main, _start

_start:
	mov	$010000,sp
	jsr	pc, _main
	halt

___main:
	rts	pc

