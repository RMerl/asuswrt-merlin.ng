// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * Copyright (C) 2011 Andes Technology Corporation
 * Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */

#include <common.h>
#include <asm/ptrace.h>
#include <asm/system.h>
#undef INTERRUPT_MODE

static int int_flag;

int irq_flags;		/* needed by asm-nds32/system.h */

int GIE_STATUS(void)
{
	int ret;

	__asm__ __volatile__ (
		"mfsr	$p0, $psw\n\t"
		"andi	%0, %0, 0x1\n\t"
		: "=r" (ret)
		:
		: "memory"
	);
	return ret;
}

#ifdef CONFIG_USE_INTERRUPT

int interrupt_init(void)
{
	return 0;
}
/* enable interrupts */
void enable_interrupts(void)
{
	local_irq_restore(int_flag);
}

/*
 * disable interrupts
 * Return true if GIE is enabled before we disable it.
 */
int disable_interrupts(void)
{

	int gie_ori_status;

	gie_ori_status = GIE_STATUS();

	local_irq_save(int_flag);

	return gie_ori_status;
}
#endif

void bad_mode(void)
{
	panic("Resetting CPU ...\n");
	reset_cpu(0);
}

void show_regs(struct pt_regs *regs)
{
	const char *processor_modes[] = {"USER", "SuperUser" , "HyperVisor"};

	printf("\n");
	printf("pc : [<%08lx>]	sp: [<%08lx>]\n"
		"lp : %08lx  gp : %08lx  fp : %08lx\n",
		regs->ipc, regs->sp, regs->lp, regs->gp, regs->fp);
	printf("D1H: %08lx  D1L: %08lx  D0H: %08lx  D0L: %08lx\n",
		regs->d1hi, regs->d1lo, regs->d0hi, regs->d0lo);
	printf("r27: %08lx  r26: %08lx  r25: %08lx  r24: %08lx\n",
		regs->p1, regs->p0, regs->r[25], regs->r[24]);
	printf("r23: %08lx  r22: %08lx  r21: %08lx  r20: %08lx\n",
		regs->r[23], regs->r[22], regs->r[21], regs->r[20]);
	printf("r19: %08lx  r18: %08lx  r17: %08lx  r16: %08lx\n",
		regs->r[19], regs->r[18], regs->r[17], regs->r[16]);
	printf("r15: %08lx  r14: %08lx  r13: %08lx  r12: %08lx\n",
		regs->r[15], regs->r[14], regs->r[13], regs->r[12]);
	printf("r11: %08lx  r10: %08lx  r9 : %08lx  r8 : %08lx\n",
		regs->r[11], regs->r[10], regs->r[9], regs->r[8]);
	printf("r7 : %08lx  r6 : %08lx  r5 : %08lx  r4 : %08lx\n",
		regs->r[7], regs->r[6], regs->r[5], regs->r[4]);
	printf("r3 : %08lx  r2 : %08lx  r1 : %08lx  r0 : %08lx\n",
		regs->r[3], regs->r[2], regs->r[1], regs->r[0]);
	printf("  Interrupts %s  Mode %s\n",
		interrupts_enabled(regs) ? "on" : "off",
		processor_modes[processor_mode(regs)]);
}

void do_interruption(struct pt_regs *pt_regs, int EVIC_num)
{
	const char *interruption_type[] = {
		"Reset",
		"TLB Fill",
		"TLB Not Present",
		"TLB Misc",
		"VLPT Miss",
		"Cache Parity Error",
		"Debug",
		"General Exception",
		"External Interrupt"
	};

	printf("%s\n", interruption_type[EVIC_num]);
	show_regs(pt_regs);
	bad_mode();
}
