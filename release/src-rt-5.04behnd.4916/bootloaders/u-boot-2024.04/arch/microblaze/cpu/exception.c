// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007 Michal Simek
 *
 * Michal  SIMEK <monstr@monstr.eu>
 */

#include <hang.h>
#include <stdio.h>
#include <asm/asm.h>

void _hw_exception_handler (void)
{
	int address = 0;
	int state = 0;

	/* loading address of exception EAR */
	MFS(address, rear);
	/* loading exception state register ESR */
	MFS(state, resr);
	printf("Hardware exception at 0x%x address\n", address);
	R17(address);

	if (CONFIG_IS_ENABLED(XILINX_MICROBLAZE0_DELAY_SLOT_EXCEP) &&
	    (state & 0x1000)) {
		/*
		 * For exceptions in delay slots, the return address is stored
		 * in the Branch Target Register (BTR), rather than R17.
		 */
		MFS(address, rbtr);

		puts("Exception in delay slot\n");
	}

	switch (state & 0x1f) {	/* mask on exception cause */
	case 0x1:
		puts("Unaligned data access exception\n");

		printf("Unaligned %sword access\n", ((state & 0x800) ? "" : "half"));
		printf("Unaligned %s access\n", ((state & 0x400) ? "store" : "load"));
		printf("Register R%x\n", (state & 0x3E0) >> 5);
		break;
	case 0x2:
		puts("Illegal op-code exception\n");
		break;
	case 0x3:
		puts("Instruction bus error exception\n");
		break;
	case 0x4:
		puts("Data bus error exception\n");
		break;
	case 0x5:
		puts("Divide by zero exception\n");
		break;
	case 0x7:
		puts("Priviledged or stack protection violation exception\n");
		break;
	default:
		puts("Undefined cause\n");
		break;
	}

	printf("Return address from exception 0x%x\n", address);
	hang();
}

#if CONFIG_IS_ENABLED(XILINX_MICROBLAZE0_USR_EXCEP)
void _exception_handler (void)
{
	puts("User vector_exception\n");
	hang();
}
#endif
