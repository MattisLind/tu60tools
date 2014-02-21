	.text

	.even
	.globl ___xorhi3
___xorhi3:
	mov r5, -(sp)
	mov sp, r5
	mov 06(r5), r0
	mov 04(r5), r1
	mov r0, r2
	com r2
	mov r1, r3
	com r3
	bic r3,r2
	bic r1,r0
	bis r2,r0
	mov (sp)+, r5
	rts pc
