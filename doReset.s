	.text

	.even
	.globl _doReset
_doReset:
	mov r5, -(sp)
	mov sp, r5
	reset
	mov (sp)+, r5
	rts pc
