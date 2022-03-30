// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007 Michal Simek
 *
 * Michal  SIMEK <monstr@monstr.eu>
 */

#include <common.h>
#include <asm/asm.h>

void _hw_exception_handler (void)
{
	int address = 0;
	int state = 0;

	/* loading address of exception EAR */
	MFS(address, rear);
	/* loading excetpion state register ESR */
	MFS(state, resr);
	printf("Hardware exception at 0x%x address\n", address);
	R17(address);
	printf("Return address from exception 0x%x\n", address);
	switch (state & 0x1f) {	/* mask on exception cause */
	case 0x1:
		puts("Unaligned data access exception\n");
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
#ifdef MICROBLAZE_V5
	case 0x7:
		puts("Priviledged or stack protection violation exception\n");
		break;
	case 0x1000:
		puts("Exception in delay slot\n");
		break;
#endif
	default:
		puts("Undefined cause\n");
		break;
	}
	printf("Unaligned %sword access\n", ((state & 0x800) ? "" : "half"));
	printf("Unaligned %s access\n", ((state & 0x400) ? "store" : "load"));
	printf("Register R%x\n", (state & 0x3E) >> 5);
	hang();
}

#ifdef CONFIG_SYS_USR_EXCEP
void _exception_handler (void)
{
	puts("User vector_exception\n");
	hang();
}
#endif
