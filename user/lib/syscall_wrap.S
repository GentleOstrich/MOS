#include <asm/asm.h>

LEAF(msyscall)
	// Just use 'syscall' instruction and return.

	/* Exercise 4.1: Your code here. */
	syscall
	jr ra
	nop
END(msyscall)

LEAF(getsp)
	move v0, sp
	jr ra
	nop
END(getsp)