// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 */

/*
 * CPU specific code for the MPC83xx family.
 *
 * Derived from the MPC8260 and MPC85xx.
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <mpc83xx.h>
#include <asm/processor.h>
#include <linux/libfdt.h>
#include <tsec.h>
#include <netdev.h>
#include <fsl_esdhc.h>
#if defined(CONFIG_BOOTCOUNT_LIMIT) && !defined(CONFIG_ARCH_MPC831X)
#include <linux/immap_qe.h>
#include <asm/io.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_CPU_MPC83XX
int checkcpu(void)
{
	volatile immap_t *immr;
	ulong clock = gd->cpu_clk;
	u32 pvr = get_pvr();
	u32 spridr;
	char buf[32];
	int ret;
	int i;

	const struct cpu_type {
		char name[15];
		u32 partid;
	} cpu_type_list [] = {
		CPU_TYPE_ENTRY(8308),
		CPU_TYPE_ENTRY(8309),
		CPU_TYPE_ENTRY(8311),
		CPU_TYPE_ENTRY(8313),
		CPU_TYPE_ENTRY(8314),
		CPU_TYPE_ENTRY(8315),
		CPU_TYPE_ENTRY(8321),
		CPU_TYPE_ENTRY(8323),
		CPU_TYPE_ENTRY(8343),
		CPU_TYPE_ENTRY(8347_TBGA_),
		CPU_TYPE_ENTRY(8347_PBGA_),
		CPU_TYPE_ENTRY(8349),
		CPU_TYPE_ENTRY(8358_TBGA_),
		CPU_TYPE_ENTRY(8358_PBGA_),
		CPU_TYPE_ENTRY(8360),
		CPU_TYPE_ENTRY(8377),
		CPU_TYPE_ENTRY(8378),
		CPU_TYPE_ENTRY(8379),
	};

	immr = (immap_t *)CONFIG_SYS_IMMR;

	ret = prt_83xx_rsr();
	if (ret)
		return ret;

	puts("CPU:   ");

	switch (pvr & 0xffff0000) {
		case PVR_E300C1:
			printf("e300c1, ");
			break;

		case PVR_E300C2:
			printf("e300c2, ");
			break;

		case PVR_E300C3:
			printf("e300c3, ");
			break;

		case PVR_E300C4:
			printf("e300c4, ");
			break;

		default:
			printf("Unknown core, ");
	}

	spridr = immr->sysconf.spridr;

	for (i = 0; i < ARRAY_SIZE(cpu_type_list); i++)
		if (cpu_type_list[i].partid == PARTID_NO_E(spridr)) {
			puts("MPC");
			puts(cpu_type_list[i].name);
			if (IS_E_PROCESSOR(spridr))
				puts("E");
			if ((SPR_FAMILY(spridr) == SPR_834X_FAMILY ||
			     SPR_FAMILY(spridr) == SPR_836X_FAMILY) &&
			    REVID_MAJOR(spridr) >= 2)
				puts("A");
			printf(", Rev: %d.%d", REVID_MAJOR(spridr),
			       REVID_MINOR(spridr));
			break;
		}

	if (i == ARRAY_SIZE(cpu_type_list))
		printf("(SPRIDR %08x unknown), ", spridr);

	printf(" at %s MHz, ", strmhz(buf, clock));

	printf("CSB: %s MHz\n", strmhz(buf, gd->arch.csb_clk));

	return 0;
}
#endif

#ifndef CONFIG_SYSRESET
int
do_reset (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	ulong msr;
#ifndef MPC83xx_RESET
	ulong addr;
#endif

	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	puts("Resetting the board.\n");

#ifdef MPC83xx_RESET

	/* Interrupts and MMU off */
	msr = mfmsr();
	msr &= ~(MSR_EE | MSR_IR | MSR_DR);
	mtmsr(msr);

	/* enable Reset Control Reg */
	immap->reset.rpr = 0x52535445;
	sync();
	isync();

	/* confirm Reset Control Reg is enabled */
	while(!((immap->reset.rcer) & RCER_CRE))
		;

	udelay(200);

	/* perform reset, only one bit */
	immap->reset.rcr = RCR_SWHR;

#else	/* ! MPC83xx_RESET */

	immap->reset.rmr = RMR_CSRE;    /* Checkstop Reset enable */

	/* Interrupts and MMU off */
	msr = mfmsr();
	msr &= ~(MSR_ME | MSR_EE | MSR_IR | MSR_DR);
	mtmsr(msr);

	/*
	 * Trying to execute the next instruction at a non-existing address
	 * should cause a machine check, resulting in reset
	 */
	addr = CONFIG_SYS_RESET_ADDRESS;

	((void (*)(void)) addr) ();
#endif	/* MPC83xx_RESET */

	return 1;
}
#endif

/*
 * Get timebase clock frequency (like cpu_clk in Hz)
 */
#ifndef CONFIG_TIMER
unsigned long get_tbclk(void)
{
	return (gd->bus_clk + 3L) / 4L;
}
#endif

#if defined(CONFIG_WATCHDOG)
void watchdog_reset (void)
{
	int re_enable = disable_interrupts();

	/* Reset the 83xx watchdog */
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	immr->wdt.swsrr = 0x556c;
	immr->wdt.swsrr = 0xaa39;

	if (re_enable)
		enable_interrupts ();
}
#endif

#ifndef CONFIG_DM_ETH
/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
int cpu_eth_init(bd_t *bis)
{
#if defined(CONFIG_UEC_ETH)
	uec_standard_init(bis);
#endif

#if defined(CONFIG_TSEC_ENET)
	tsec_standard_init(bis);
#endif
	return 0;
}
#endif /* !CONFIG_DM_ETH */

/*
 * Initializes on-chip MMC controllers.
 * to override, implement board_mmc_init()
 */
int cpu_mmc_init(bd_t *bis)
{
#ifdef CONFIG_FSL_ESDHC
	return fsl_esdhc_mmc_init(bis);
#else
	return 0;
#endif
}

void ppcDWstore(unsigned int *addr, unsigned int *value)
{
	asm("lfd 1, 0(%1)\n\t"
	    "stfd 1, 0(%0)"
	    :
	    : "r" (addr), "r" (value)
	    : "memory");
}

void ppcDWload(unsigned int *addr, unsigned int *ret)
{
	asm("lfd 1, 0(%0)\n\t"
	    "stfd 1, 0(%1)"
	    :
	    : "r" (addr), "r" (ret)
	    : "memory");
}
