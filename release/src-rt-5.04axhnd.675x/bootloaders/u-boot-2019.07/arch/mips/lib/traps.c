// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 1994 - 1999, 2000, 01, 06 Ralf Baechle
 * Copyright (C) 1995, 1996 Paul M. Antoine
 * Copyright (C) 1998 Ulf Carlsson
 * Copyright (C) 1999 Silicon Graphics, Inc.
 * Kevin D. Kissell, kevink@mips.com and Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 2002, 2003, 2004, 2005, 2007  Maciej W. Rozycki
 * Copyright (C) 2000, 2001, 2012 MIPS Technologies, Inc.  All rights reserved.
 * Copyright (C) 2014, Imagination Technologies Ltd.
 */

#include <common.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/system.h>

DECLARE_GLOBAL_DATA_PTR;

static void show_regs(const struct pt_regs *regs)
{
	const int field = 2 * sizeof(unsigned long);
	unsigned int cause = regs->cp0_cause;
	unsigned int exccode;
	int i;

	/*
	 * Saved main processor registers
	 */
	for (i = 0; i < 32; ) {
		if ((i % 4) == 0)
			printf("$%2d   :", i);
		if (i == 0)
			printf(" %0*lx", field, 0UL);
		else if (i == 26 || i == 27)
			printf(" %*s", field, "");
		else
			printf(" %0*lx", field, regs->regs[i]);

		i++;
		if ((i % 4) == 0)
			puts("\n");
	}

	printf("Hi    : %0*lx\n", field, regs->hi);
	printf("Lo    : %0*lx\n", field, regs->lo);

	/*
	 * Saved cp0 registers
	 */
	printf("epc   : %0*lx (text %0*lx)\n", field, regs->cp0_epc,
	       field, regs->cp0_epc - gd->reloc_off);
	printf("ra    : %0*lx (text %0*lx)\n", field, regs->regs[31],
	       field, regs->regs[31] - gd->reloc_off);

	printf("Status: %08x\n", (uint32_t) regs->cp0_status);

	exccode = (cause & CAUSEF_EXCCODE) >> CAUSEB_EXCCODE;
	printf("Cause : %08x (ExcCode %02x)\n", cause, exccode);

	if (1 <= exccode && exccode <= 5)
		printf("BadVA : %0*lx\n", field, regs->cp0_badvaddr);

	printf("PrId  : %08x\n", read_c0_prid());
}

void do_reserved(const struct pt_regs *regs)
{
	puts("\nOoops:\n");
	show_regs(regs);
	hang();
}

void do_ejtag_debug(const struct pt_regs *regs)
{
	const int field = 2 * sizeof(unsigned long);
	unsigned long depc;
	unsigned int debug;

	depc = read_c0_depc();
	debug = read_c0_debug();

	printf("SDBBP EJTAG debug exception: c0_depc = %0*lx, DEBUG = %08x\n",
	       field, depc, debug);
}

static void set_handler(unsigned long offset, void *addr, unsigned long size)
{
	unsigned long ebase = gd->irq_sp;

	memcpy((void *)(ebase + offset), addr, size);
	flush_cache(ebase + offset, size);
}

void trap_init(ulong reloc_addr)
{
	unsigned long ebase = gd->irq_sp;

	set_handler(0x180, &except_vec3_generic, 0x80);
	set_handler(0x280, &except_vec_ejtag_debug, 0x80);

	write_c0_ebase(ebase);
	clear_c0_status(ST0_BEV);
	execution_hazard_barrier();
}
