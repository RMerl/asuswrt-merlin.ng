// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * m8xx.c
 *
 * CPU specific code
 *
 * written or collected and sometimes rewritten by
 * Magnus Damm <damm@bitsmart.com>
 *
 * minor modifications by
 * Wolfgang Denk <wd@denx.de>
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <mpc8xx.h>
#include <netdev.h>
#include <asm/cache.h>
#include <asm/cpm_8xx.h>
#include <linux/compiler.h>
#include <asm/io.h>

#if defined(CONFIG_OF_LIBFDT)
#include <linux/libfdt.h>
#include <fdt_support.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

static int check_CPU(long clock, uint pvr, uint immr)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;
	uint k;
	char buf[32];

	/* the highest 16 bits should be 0x0050 for a 860 */

	if (PVR_VER(pvr) != PVR_VER(PVR_8xx))
		return -1;

	k = (immr << 16) |
	    in_be16(&immap->im_cpm.cp_dparam16[PROFF_REVNUM / sizeof(u16)]);

	/*
	 * Some boards use sockets so different CPUs can be used.
	 * We have to check chip version in run time.
	 */
	switch (k) {
		/* MPC866P/MPC866T/MPC859T/MPC859DSL/MPC852T */
	case 0x08010004:		/* Rev. A.0 */
		printf("MPC866xxxZPnnA");
		break;
	case 0x08000003:		/* Rev. 0.3 */
		printf("MPC866xxxZPnn");
		break;
	case 0x09000000:		/* 870/875/880/885 */
		puts("MPC885ZPnn");
		break;

	default:
		printf("unknown MPC86x (0x%08x)", k);
		break;
	}

	printf(" at %s MHz: ", strmhz(buf, clock));

	print_size(checkicache(), " I-Cache ");
	print_size(checkdcache(), " D-Cache");

	/* do we have a FEC (860T/P or 852/859/866/885)? */

	out_be32(&immap->im_cpm.cp_fec.fec_addr_low, 0x12345678);
	if (in_be32(&immap->im_cpm.cp_fec.fec_addr_low) == 0x12345678)
		printf(" FEC present");

	putc('\n');

	return 0;
}

/* ------------------------------------------------------------------------- */

int checkcpu(void)
{
	ulong clock = gd->cpu_clk;
	uint immr = get_immr();	/* Return full IMMR contents */
	uint pvr = get_pvr();

	puts("CPU:   ");

	return check_CPU(clock, pvr, immr);
}

/* ------------------------------------------------------------------------- */
/* L1 i-cache                                                                */

int checkicache(void)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;
	memctl8xx_t __iomem *memctl = &immap->im_memctl;
	u32 cacheon = rd_ic_cst() & IDC_ENABLED;
	/* probe in flash memoryarea */
	u32 k = in_be32(&memctl->memc_br0) & ~0x00007fff;
	u32 m;
	u32 lines = -1;

	wr_ic_cst(IDC_UNALL);
	wr_ic_cst(IDC_INVALL);
	wr_ic_cst(IDC_DISABLE);
	__asm__ volatile ("isync");

	while (!((m = rd_ic_cst()) & IDC_CERR2)) {
		wr_ic_adr(k);
		wr_ic_cst(IDC_LDLCK);
		__asm__ volatile ("isync");

		lines++;
		k += 0x10;	/* the number of bytes in a cacheline */
	}

	wr_ic_cst(IDC_UNALL);
	wr_ic_cst(IDC_INVALL);

	if (cacheon)
		wr_ic_cst(IDC_ENABLE);
	else
		wr_ic_cst(IDC_DISABLE);

	__asm__ volatile ("isync");

	return lines << 4;
};

/* ------------------------------------------------------------------------- */
/* L1 d-cache                                                                */
/* call with cache disabled                                                  */

int checkdcache(void)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;
	memctl8xx_t __iomem *memctl = &immap->im_memctl;
	u32 cacheon = rd_dc_cst() & IDC_ENABLED;
	/* probe in flash memoryarea */
	u32 k = in_be32(&memctl->memc_br0) & ~0x00007fff;
	u32 m;
	u32 lines = -1;

	wr_dc_cst(IDC_UNALL);
	wr_dc_cst(IDC_INVALL);
	wr_dc_cst(IDC_DISABLE);

	while (!((m = rd_dc_cst()) & IDC_CERR2)) {
		wr_dc_adr(k);
		wr_dc_cst(IDC_LDLCK);
		lines++;
		k += 0x10;	/* the number of bytes in a cacheline */
	}

	wr_dc_cst(IDC_UNALL);
	wr_dc_cst(IDC_INVALL);

	if (cacheon)
		wr_dc_cst(IDC_ENABLE);
	else
		wr_dc_cst(IDC_DISABLE);

	return lines << 4;
};

/* ------------------------------------------------------------------------- */

void upmconfig(uint upm, uint *table, uint size)
{
	uint i;
	uint addr = 0;
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;
	memctl8xx_t __iomem *memctl = &immap->im_memctl;

	for (i = 0; i < size; i++) {
		out_be32(&memctl->memc_mdr, table[i]);		/* (16-15) */
		out_be32(&memctl->memc_mcr, addr | upm);	/* (16-16) */
		addr++;
	}
}

/* ------------------------------------------------------------------------- */

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong msr, addr;

	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	/* Checkstop Reset enable */
	setbits_be32(&immap->im_clkrst.car_plprcr, PLPRCR_CSR);

	/* Interrupts and MMU off */
	__asm__ volatile ("mtspr    81, 0");
	__asm__ volatile ("mfmsr    %0" : "=r" (msr));

	msr &= ~0x1030;
	__asm__ volatile ("mtmsr    %0" : : "r" (msr));

	/*
	 * Trying to execute the next instruction at a non-existing address
	 * should cause a machine check, resulting in reset
	 */
#ifdef CONFIG_SYS_RESET_ADDRESS
	addr = CONFIG_SYS_RESET_ADDRESS;
#else
	/*
	 * note: when CONFIG_SYS_MONITOR_BASE points to a RAM address,
	 * CONFIG_SYS_MONITOR_BASE - sizeof (ulong) is usually a valid address.
	 * Better pick an address known to be invalid on your system and assign
	 * it to CONFIG_SYS_RESET_ADDRESS.
	 * "(ulong)-1" used to be a good choice for many systems...
	 */
	addr = CONFIG_SYS_MONITOR_BASE - sizeof(ulong);
#endif
	((void (*)(void)) addr)();
	return 1;
}

/* ------------------------------------------------------------------------- */

/*
 * Get timebase clock frequency (like cpu_clk in Hz)
 *
 * See sections 14.2 and 14.6 of the User's Manual
 */
unsigned long get_tbclk(void)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;
	ulong oscclk, factor, pll;

	if (in_be32(&immap->im_clkrst.car_sccr) & SCCR_TBS)
		return gd->cpu_clk / 16;

	pll = in_be32(&immap->im_clkrst.car_plprcr);

#define PLPRCR_val(a) ((pll & PLPRCR_ ## a ## _MSK) >> PLPRCR_ ## a ## _SHIFT)

	/*
	 * For newer PQ1 chips (MPC866/87x/88x families), PLL multiplication
	 * factor is calculated as follows:
	 *
	 *		     MFN
	 *	     MFI + -------
	 *		   MFD + 1
	 * factor =  -----------------
	 *	     (PDF + 1) * 2^S
	 *
	 */
	factor = (PLPRCR_val(MFI) + PLPRCR_val(MFN) / (PLPRCR_val(MFD) + 1)) /
		 (PLPRCR_val(PDF) + 1) / (1 << PLPRCR_val(S));

	oscclk = gd->cpu_clk / factor;

	if ((in_be32(&immap->im_clkrst.car_sccr) & SCCR_RTSEL) == 0 ||
	    factor > 2)
		return oscclk / 4;

	return oscclk / 16;
}

/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
int cpu_eth_init(bd_t *bis)
{
#if defined(CONFIG_MPC8XX_FEC)
	fec_initialize(bis);
#endif
	return 0;
}
