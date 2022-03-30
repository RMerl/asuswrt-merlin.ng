// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Josef Baumgartner <josef.baumgartner@telex.de>
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <asm/processor.h>


extern void _exc_handler(void);
extern void _int_handler(void);

static void show_frame(struct pt_regs *fp)
{
	printf ("Vector Number: %d  Format: %02x  Fault Status: %01x\n\n", (fp->vector & 0x3fc) >> 2,
		fp->format, (fp->vector & 0x3) | ((fp->vector & 0xc00) >> 8));
	printf ("PC: %08lx    SR: %08lx    SP: %08lx\n", fp->pc, (long) fp->sr, (long) fp);
	printf ("D0: %08lx    D1: %08lx    D2: %08lx    D3: %08lx\n",
		fp->d0, fp->d1, fp->d2, fp->d3);
	printf ("D4: %08lx    D5: %08lx    D6: %08lx    D7: %08lx\n",
		fp->d4, fp->d5, fp->d6, fp->d7);
	printf ("A0: %08lx    A1: %08lx    A2: %08lx    A3: %08lx\n",
		fp->a0, fp->a1, fp->a2, fp->a3);
	printf ("A4: %08lx    A5: %08lx    A6: %08lx\n",
		fp->a4, fp->a5, fp->a6);
}

void exc_handler(struct pt_regs *fp) {
	printf("\n\n*** Unexpected exception ***\n");
	show_frame (fp);
	printf("\n*** Please Reset Board! ***\n");
	for(;;);
}

void trap_init(ulong value) {
	unsigned long *vec = (ulong *)value;
	int i;

	for(i = 2; i < 25; i++) {
		vec[i] = (unsigned long)_exc_handler;
	}
	for(i = 25; i < 32; i++) {
		vec[i] = (unsigned long)_int_handler;
	}
	for(i = 32; i < 64; i++) {
		vec[i] = (unsigned long)_exc_handler;
	}
	for(i = 64; i < 256; i++) {
		vec[i] = (unsigned long)_int_handler;
	}

	setvbr(value);		/* set vector base register to new table */
}
