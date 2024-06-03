// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2007,2009 Wind River Systems, Inc. <www.windriver.com>
 *
 * Copyright 2007 Embedded Specialties, Inc.
 *
 * Copyright 2004, 2007 Freescale Semiconductor.
 *
 * (C) Copyright 2002 Scott McNutt <smcnutt@artesyncp.com>
 */

#include <common.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_pci.h>
#include <fsl_ddr_sdram.h>
#include <asm/fsl_serdes.h>
#include <spd_sdram.h>
#include <netdev.h>
#include <tsec.h>
#include <miiphy.h>
#include <linux/libfdt.h>
#include <fdt_support.h>

void local_bus_init(void);

int board_early_init_f (void)
{
	return 0;
}

int checkboard (void)
{
	volatile ccsr_local_ecm_t *ecm = (void *)(CONFIG_SYS_MPC85xx_ECM_ADDR);
	volatile u_char *rev= (void *)CONFIG_SYS_BD_REV;

	printf ("Board: Wind River SBC8548 Rev. 0x%01x\n",
			in_8(rev) >> 4);

	/*
	 * Initialize local bus.
	 */
	local_bus_init ();

	out_be32(&ecm->eedr, 0xffffffff);	/* clear ecm errors */
	out_be32(&ecm->eeer, 0xffffffff);	/* enable ecm errors */
	return 0;
}

/*
 * Initialize Local Bus
 */
void
local_bus_init(void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	volatile fsl_lbc_t *lbc = LBC_BASE_ADDR;

	uint clkdiv, lbc_mhz, lcrr = CONFIG_SYS_LBC_LCRR;
	sys_info_t sysinfo;

	get_sys_info(&sysinfo);

	lbc_mhz = sysinfo.freq_localbus / 1000000;
	clkdiv = sysinfo.freq_systembus / sysinfo.freq_localbus;

	debug("LCRR=0x%x, CD=%d, MHz=%d\n", lcrr, clkdiv, lbc_mhz);

	out_be32(&gur->lbiuiplldcr1, 0x00078080);
	if (clkdiv == 16) {
		out_be32(&gur->lbiuiplldcr0, 0x7c0f1bf0);
	} else if (clkdiv == 8) {
		out_be32(&gur->lbiuiplldcr0, 0x6c0f1bf0);
	} else if (clkdiv == 4) {
		out_be32(&gur->lbiuiplldcr0, 0x5c0f1bf0);
	}

	/*
	 * Local Bus Clock > 83.3 MHz. According to timing
	 * specifications set LCRR[EADC] to 2 delay cycles.
	 */
	if (lbc_mhz > 83) {
		lcrr &= ~LCRR_EADC;
		lcrr |= LCRR_EADC_2;
	}

	/*
	 * According to MPC8548ERMAD Rev. 1.3, 13.3.1.16, 13-30
	 * disable PLL bypass for Local Bus Clock > 83 MHz.
	 */
	if (lbc_mhz >= 66)
		lcrr &= (~LCRR_DBYP);	/* DLL Enabled */

	else
		lcrr |= LCRR_DBYP;	/* DLL Bypass */

	out_be32(&lbc->lcrr, lcrr);
	asm("sync;isync;msync");

	 /*
	 * According to MPC8548ERMAD Rev.1.3 read back LCRR
	 * and terminate with isync
	 */
	lcrr = in_be32(&lbc->lcrr);
	asm ("isync;");

	/* let DLL stabilize */
	udelay(500);

	out_be32(&lbc->ltesr, 0xffffffff);	/* Clear LBC error IRQs */
	out_be32(&lbc->lteir, 0xffffffff);	/* Enable LBC error IRQs */
}

/*
 * Initialize SDRAM memory on the Local Bus.
 */
void lbc_sdram_init(void)
{
#if defined(CONFIG_SYS_LBC_SDRAM_SIZE)

	uint idx;
	const unsigned long size = CONFIG_SYS_LBC_SDRAM_SIZE * 1024 * 1024;
	volatile fsl_lbc_t *lbc = LBC_BASE_ADDR;
	uint *sdram_addr = (uint *)CONFIG_SYS_LBC_SDRAM_BASE;
	uint *sdram_addr2 = (uint *)(CONFIG_SYS_LBC_SDRAM_BASE + size/2);

	puts("    SDRAM: ");

	print_size(size, "\n");

	/*
	 * Setup SDRAM Base and Option Registers
	 */
	set_lbc_or(3, CONFIG_SYS_OR3_PRELIM);
	set_lbc_br(3, CONFIG_SYS_BR3_PRELIM);
	set_lbc_or(4, CONFIG_SYS_OR4_PRELIM);
	set_lbc_br(4, CONFIG_SYS_BR4_PRELIM);

	out_be32(&lbc->lbcr, CONFIG_SYS_LBC_LBCR);
	asm("msync");

	out_be32(&lbc->lsrt,  CONFIG_SYS_LBC_LSRT);
	out_be32(&lbc->mrtpr, CONFIG_SYS_LBC_MRTPR);
	asm("msync");

	/*
	 * Issue PRECHARGE ALL command.
	 */
	out_be32(&lbc->lsdmr, CONFIG_SYS_LBC_LSDMR_PCHALL);
	asm("sync;msync");
	*sdram_addr = 0xff;
	ppcDcbf((unsigned long) sdram_addr);
	*sdram_addr2 = 0xff;
	ppcDcbf((unsigned long) sdram_addr2);
	udelay(100);

	/*
	 * Issue 8 AUTO REFRESH commands.
	 */
	for (idx = 0; idx < 8; idx++) {
		out_be32(&lbc->lsdmr, CONFIG_SYS_LBC_LSDMR_ARFRSH);
		asm("sync;msync");
		*sdram_addr = 0xff;
		ppcDcbf((unsigned long) sdram_addr);
		*sdram_addr2 = 0xff;
		ppcDcbf((unsigned long) sdram_addr2);
		udelay(100);
	}

	/*
	 * Issue 8 MODE-set command.
	 */
	out_be32(&lbc->lsdmr, CONFIG_SYS_LBC_LSDMR_MRW);
	asm("sync;msync");
	*sdram_addr = 0xff;
	ppcDcbf((unsigned long) sdram_addr);
	*sdram_addr2 = 0xff;
	ppcDcbf((unsigned long) sdram_addr2);
	udelay(100);

	/*
	 * Issue RFEN command.
	 */
	out_be32(&lbc->lsdmr, CONFIG_SYS_LBC_LSDMR_RFEN);
	asm("sync;msync");
	*sdram_addr = 0xff;
	ppcDcbf((unsigned long) sdram_addr);
	*sdram_addr2 = 0xff;
	ppcDcbf((unsigned long) sdram_addr2);
	udelay(200);    /* Overkill. Must wait > 200 bus cycles */

#endif	/* enable SDRAM init */
}

#if defined(CONFIG_SYS_DRAM_TEST)
int
testdram(void)
{
	uint *pstart = (uint *) CONFIG_SYS_MEMTEST_START;
	uint *pend = (uint *) CONFIG_SYS_MEMTEST_END;
	uint *p;

	printf("Testing DRAM from 0x%08x to 0x%08x\n",
	       CONFIG_SYS_MEMTEST_START,
	       CONFIG_SYS_MEMTEST_END);

	printf("DRAM test phase 1:\n");
	for (p = pstart; p < pend; p++)
		*p = 0xaaaaaaaa;

	for (p = pstart; p < pend; p++) {
		if (*p != 0xaaaaaaaa) {
			printf ("DRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf("DRAM test phase 2:\n");
	for (p = pstart; p < pend; p++)
		*p = 0x55555555;

	for (p = pstart; p < pend; p++) {
		if (*p != 0x55555555) {
			printf ("DRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf("DRAM test passed.\n");
	return 0;
}
#endif

#ifdef CONFIG_PCI1
static struct pci_controller pci1_hose;
#endif	/* CONFIG_PCI1 */

#ifdef CONFIG_PCI
void
pci_init_board(void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	int first_free_busno = 0;

#ifdef CONFIG_PCI1
	struct fsl_pci_info pci_info;
	u32 devdisr = in_be32(&gur->devdisr);
	u32 pordevsr = in_be32(&gur->pordevsr);
	u32 porpllsr = in_be32(&gur->porpllsr);

	if (!(devdisr & MPC85xx_DEVDISR_PCI1)) {
		uint pci_32 = pordevsr & MPC85xx_PORDEVSR_PCI1_PCI32;
		uint pci_arb = pordevsr & MPC85xx_PORDEVSR_PCI1_ARB;
		uint pci_clk_sel = porpllsr & MPC85xx_PORDEVSR_PCI1_SPD;
		uint pci_speed = CONFIG_SYS_CLK_FREQ;	/* get_clock_freq() */

		printf("PCI: Host, %d bit, %s MHz, %s, %s\n",
			(pci_32) ? 32 : 64,
			(pci_speed == 33000000) ? "33" :
			(pci_speed == 66000000) ? "66" : "unknown",
			pci_clk_sel ? "sync" : "async",
			pci_arb ? "arbiter" : "external-arbiter");

		SET_STD_PCI_INFO(pci_info, 1);
		set_next_law(pci_info.mem_phys,
			law_size_bits(pci_info.mem_size), pci_info.law);
		set_next_law(pci_info.io_phys,
			law_size_bits(pci_info.io_size), pci_info.law);

		first_free_busno = fsl_pci_init_port(&pci_info,
					&pci1_hose, first_free_busno);
	} else {
		printf("PCI: disabled\n");
	}

	puts("\n");
#else
	setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_PCI1); /* disable */
#endif

	setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_PCI2); /* disable PCI2 */

	fsl_pcie_init_board(first_free_busno);
}
#endif

int board_eth_init(bd_t *bis)
{
	tsec_standard_init(bis);
	pci_eth_init(bis);
	return 0;	/* otherwise cpu_eth_init gets run */
}

int last_stage_init(void)
{
	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

#ifdef CONFIG_FSL_PCI_INIT
	FT_FSL_PCI_SETUP;
#endif

	return 0;
}
#endif
