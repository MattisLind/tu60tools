	.text

	.even
	.globl ___ashlhi3
___ashlhi3:
	mov r5, -(sp)
	mov sp, r5
	br L_2
L_3:
	mov 04(r5), r0
	asl r0
	mov r0, 04(r5)
	mov 06(r5), r0
	mov r0, r1
	dec r1
	mov r1, 06(r5)
L_2:
	tst 06(r5)
	bgt L_3
	mov 04(r5), r0
	mov (sp)+, r5
	rts pc
