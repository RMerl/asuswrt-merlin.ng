// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2004-2009 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <mpc83xx.h>
#include <ioports.h>
#include <asm/io.h>
#include <asm/processor.h>
#ifdef CONFIG_USB_EHCI_FSL
#include <usb/ehci-ci.h>
#endif

#include "lblaw/lblaw.h"
#include "elbc/elbc.h"
#include "sysio/sysio.h"
#include "arbiter/arbiter.h"
#include "initreg/initreg.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_QE
extern qe_iop_conf_t qe_iop_conf_tab[];
extern void qe_config_iopin(u8 port, u8 pin, int dir,
			 int open_drain, int assign);
extern void qe_init(uint qe_base);
extern void qe_reset(void);

static void config_qe_ioports(void)
{
	u8	port, pin;
	int	dir, open_drain, assign;
	int	i;

	for (i = 0; qe_iop_conf_tab[i].assign != QE_IOP_TAB_END; i++) {
		port		= qe_iop_conf_tab[i].port;
		pin		= qe_iop_conf_tab[i].pin;
		dir		= qe_iop_conf_tab[i].dir;
		open_drain	= qe_iop_conf_tab[i].open_drain;
		assign		= qe_iop_conf_tab[i].assign;
		qe_config_iopin(port, pin, dir, open_drain, assign);
	}
}
#endif

/*
 * Breathe some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f (volatile immap_t * im)
{
	__be32 sccr_mask =
#ifdef CONFIG_SYS_SCCR_ENCCM /* Encryption clock mode */
		SCCR_ENCCM |
#endif
#ifdef CONFIG_SYS_SCCR_PCICM /* PCI & DMA clock mode */
		SCCR_PCICM |
#endif
#ifdef CONFIG_SYS_SCCR_PCIEXP1CM	/* PCIE1 clock mode */
		SCCR_PCIEXP1CM |
#endif
#ifdef CONFIG_SYS_SCCR_PCIEXP2CM	/* PCIE2 clock mode */
		SCCR_PCIEXP2CM |
#endif
#ifdef CONFIG_SYS_SCCR_TSECCM /* all TSEC's clock mode */
		SCCR_TSECCM |
#endif
#ifdef CONFIG_SYS_SCCR_TSEC1CM /* TSEC1 clock mode */
		SCCR_TSEC1CM |
#endif
#ifdef CONFIG_SYS_SCCR_TSEC2CM /* TSEC2 clock mode */
		SCCR_TSEC2CM |
#endif
#ifdef CONFIG_SYS_SCCR_TSEC1ON /* TSEC1 clock switch */
		SCCR_TSEC1ON |
#endif
#ifdef CONFIG_SYS_SCCR_TSEC2ON /* TSEC2 clock switch */
		SCCR_TSEC2ON |
#endif
#ifdef CONFIG_SYS_SCCR_USBMPHCM /* USB MPH clock mode */
		SCCR_USBMPHCM |
#endif
#ifdef CONFIG_SYS_SCCR_USBDRCM /* USB DR clock mode */
		SCCR_USBDRCM |
#endif
#ifdef CONFIG_SYS_SCCR_SATACM /* SATA controller clock mode */
		SCCR_SATACM |
#endif
		0;
	__be32 sccr_val =
#ifdef CONFIG_SYS_SCCR_ENCCM /* Encryption clock mode */
		(CONFIG_SYS_SCCR_ENCCM << SCCR_ENCCM_SHIFT) |
#endif
#ifdef CONFIG_SYS_SCCR_PCICM /* PCI & DMA clock mode */
		(CONFIG_SYS_SCCR_PCICM << SCCR_PCICM_SHIFT) |
#endif
#ifdef CONFIG_SYS_SCCR_PCIEXP1CM	/* PCIE1 clock mode */
		(CONFIG_SYS_SCCR_PCIEXP1CM << SCCR_PCIEXP1CM_SHIFT) |
#endif
#ifdef CONFIG_SYS_SCCR_PCIEXP2CM	/* PCIE2 clock mode */
		(CONFIG_SYS_SCCR_PCIEXP2CM << SCCR_PCIEXP2CM_SHIFT) |
#endif
#ifdef CONFIG_SYS_SCCR_TSECCM /* all TSEC's clock mode */
		(CONFIG_SYS_SCCR_TSECCM << SCCR_TSECCM_SHIFT) |
#endif
#ifdef CONFIG_SYS_SCCR_TSEC1CM /* TSEC1 clock mode */
		(CONFIG_SYS_SCCR_TSEC1CM << SCCR_TSEC1CM_SHIFT) |
#endif
#ifdef CONFIG_SYS_SCCR_TSEC2CM /* TSEC2 clock mode */
		(CONFIG_SYS_SCCR_TSEC2CM << SCCR_TSEC2CM_SHIFT) |
#endif
#ifdef CONFIG_SYS_SCCR_TSEC1ON /* TSEC1 clock switch */
		(CONFIG_SYS_SCCR_TSEC1ON << SCCR_TSEC1ON_SHIFT) |
#endif
#ifdef CONFIG_SYS_SCCR_TSEC2ON /* TSEC2 clock switch */
		(CONFIG_SYS_SCCR_TSEC2ON << SCCR_TSEC2ON_SHIFT) |
#endif
#ifdef CONFIG_SYS_SCCR_USBMPHCM /* USB MPH clock mode */
		(CONFIG_SYS_SCCR_USBMPHCM << SCCR_USBMPHCM_SHIFT) |
#endif
#ifdef CONFIG_SYS_SCCR_USBDRCM /* USB DR clock mode */
		(CONFIG_SYS_SCCR_USBDRCM << SCCR_USBDRCM_SHIFT) |
#endif
#ifdef CONFIG_SYS_SCCR_SATACM /* SATA controller clock mode */
		(CONFIG_SYS_SCCR_SATACM << SCCR_SATACM_SHIFT) |
#endif
		0;

	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *) (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET);

	/* global data region was cleared in start.S */

	/* system performance tweaking */
	clrsetbits_be32(&im->arbiter.acr, acr_mask, acr_val);

	clrsetbits_be32(&im->sysconf.spcr, spcr_mask, spcr_val);

	clrsetbits_be32(&im->clk.sccr, sccr_mask, sccr_val);

	/* RSR - Reset Status Register - clear all status (4.6.1.3) */
	gd->arch.reset_status = __raw_readl(&im->reset.rsr);
	__raw_writel(~(RSR_RES), &im->reset.rsr);

	/* AER - Arbiter Event Register - store status */
	gd->arch.arbiter_event_attributes = __raw_readl(&im->arbiter.aeatr);
	gd->arch.arbiter_event_address = __raw_readl(&im->arbiter.aeadr);

	/*
	 * RMR - Reset Mode Register
	 * contains checkstop reset enable (4.6.1.4)
	 */
	__raw_writel(RMR_CSRE & (1<<RMR_CSRE_SHIFT), &im->reset.rmr);

	/* LCRR - Clock Ratio Register (10.3.1.16)
	 * write, read, and isync per MPC8379ERM rev.1 CLKDEV field description
	 */
	clrsetbits_be32(&im->im_lbc.lcrr, lcrr_mask, lcrr_val);
	__raw_readl(&im->im_lbc.lcrr);
	isync();

	/* Enable Time Base & Decrementer ( so we will have udelay() )*/
	setbits_be32(&im->sysconf.spcr, SPCR_TBEN);

	/* System General Purpose Register */
#ifdef CONFIG_SYS_SICRH
#if defined(CONFIG_ARCH_MPC834X) || defined(CONFIG_ARCH_MPC8313)
	/* regarding to MPC34x manual rev.1 bits 28..29 must be preserved */
	__raw_writel((im->sysconf.sicrh & 0x0000000C) | CONFIG_SYS_SICRH,
		     &im->sysconf.sicrh);
#else
	__raw_writel(CONFIG_SYS_SICRH, &im->sysconf.sicrh);
#endif
#endif
#ifdef CONFIG_SYS_SICRL
	__raw_writel(CONFIG_SYS_SICRL, &im->sysconf.sicrl);
#endif
#ifdef CONFIG_SYS_GPR1
	__raw_writel(CONFIG_SYS_GPR1, &im->sysconf.gpr1);
#endif
#ifdef CONFIG_SYS_DDRCDR /* DDR control driver register */
	__raw_writel(CONFIG_SYS_DDRCDR, &im->sysconf.ddrcdr);
#endif
#ifdef CONFIG_SYS_OBIR /* Output buffer impedance register */
	__raw_writel(CONFIG_SYS_OBIR, &im->sysconf.obir);
#endif

#ifdef CONFIG_QE
	/* Config QE ioports */
	config_qe_ioports();
#endif
	/* Set up preliminary BR/OR regs */
	init_early_memctl_regs();

	/* Local Access window setup */
#if defined(CONFIG_SYS_LBLAWBAR0_PRELIM) && defined(CONFIG_SYS_LBLAWAR0_PRELIM)
	im->sysconf.lblaw[0].bar = CONFIG_SYS_LBLAWBAR0_PRELIM;
	im->sysconf.lblaw[0].ar = CONFIG_SYS_LBLAWAR0_PRELIM;
#else
#error	CONFIG_SYS_LBLAWBAR0_PRELIM & CONFIG_SYS_LBLAWAR0_PRELIM must be defined
#endif

#if defined(CONFIG_SYS_LBLAWBAR1_PRELIM) && defined(CONFIG_SYS_LBLAWAR1_PRELIM)
	im->sysconf.lblaw[1].bar = CONFIG_SYS_LBLAWBAR1_PRELIM;
	im->sysconf.lblaw[1].ar = CONFIG_SYS_LBLAWAR1_PRELIM;
#endif
#if defined(CONFIG_SYS_LBLAWBAR2_PRELIM) && defined(CONFIG_SYS_LBLAWAR2_PRELIM)
	im->sysconf.lblaw[2].bar = CONFIG_SYS_LBLAWBAR2_PRELIM;
	im->sysconf.lblaw[2].ar = CONFIG_SYS_LBLAWAR2_PRELIM;
#endif
#if defined(CONFIG_SYS_LBLAWBAR3_PRELIM) && defined(CONFIG_SYS_LBLAWAR3_PRELIM)
	im->sysconf.lblaw[3].bar = CONFIG_SYS_LBLAWBAR3_PRELIM;
	im->sysconf.lblaw[3].ar = CONFIG_SYS_LBLAWAR3_PRELIM;
#endif
#if defined(CONFIG_SYS_LBLAWBAR4_PRELIM) && defined(CONFIG_SYS_LBLAWAR4_PRELIM)
	im->sysconf.lblaw[4].bar = CONFIG_SYS_LBLAWBAR4_PRELIM;
	im->sysconf.lblaw[4].ar = CONFIG_SYS_LBLAWAR4_PRELIM;
#endif
#if defined(CONFIG_SYS_LBLAWBAR5_PRELIM) && defined(CONFIG_SYS_LBLAWAR5_PRELIM)
	im->sysconf.lblaw[5].bar = CONFIG_SYS_LBLAWBAR5_PRELIM;
	im->sysconf.lblaw[5].ar = CONFIG_SYS_LBLAWAR5_PRELIM;
#endif
#if defined(CONFIG_SYS_LBLAWBAR6_PRELIM) && defined(CONFIG_SYS_LBLAWAR6_PRELIM)
	im->sysconf.lblaw[6].bar = CONFIG_SYS_LBLAWBAR6_PRELIM;
	im->sysconf.lblaw[6].ar = CONFIG_SYS_LBLAWAR6_PRELIM;
#endif
#if defined(CONFIG_SYS_LBLAWBAR7_PRELIM) && defined(CONFIG_SYS_LBLAWAR7_PRELIM)
	im->sysconf.lblaw[7].bar = CONFIG_SYS_LBLAWBAR7_PRELIM;
	im->sysconf.lblaw[7].ar = CONFIG_SYS_LBLAWAR7_PRELIM;
#endif
#ifdef CONFIG_SYS_GPIO1_PRELIM
	im->gpio[0].dat = CONFIG_SYS_GPIO1_DAT;
	im->gpio[0].dir = CONFIG_SYS_GPIO1_DIR;
#endif
#ifdef CONFIG_SYS_GPIO2_PRELIM
	im->gpio[1].dat = CONFIG_SYS_GPIO2_DAT;
	im->gpio[1].dir = CONFIG_SYS_GPIO2_DIR;
#endif
#if defined(CONFIG_USB_EHCI_FSL) && defined(CONFIG_ARCH_MPC831X)
	uint32_t temp;
	struct usb_ehci *ehci = (struct usb_ehci *)CONFIG_SYS_FSL_USB1_ADDR;

	/* Configure interface. */
	setbits_be32(&ehci->control, REFSEL_16MHZ | UTMI_PHY_EN);

	/* Wait for clock to stabilize */
	do {
		temp = __raw_readl(&ehci->control);
		udelay(1000);
	} while (!(temp & PHY_CLK_VALID));
#endif
}

int cpu_init_r (void)
{
#ifdef CONFIG_QE
	uint qe_base = CONFIG_SYS_IMMR + 0x00100000; /* QE immr base */

	qe_init(qe_base);
	qe_reset();
#endif
	return 0;
}

/*
 * Print out the bus arbiter event
 */
#if defined(CONFIG_DISPLAY_AER_FULL)
static int print_83xx_arb_event(int force)
{
	static char* event[] = {
		"Address Time Out",
		"Data Time Out",
		"Address Only Transfer Type",
		"External Control Word Transfer Type",
		"Reserved Transfer Type",
		"Transfer Error",
		"reserved",
		"reserved"
	};
	static char* master[] = {
		"e300 Core Data Transaction",
		"reserved",
		"e300 Core Instruction Fetch",
		"reserved",
		"TSEC1",
		"TSEC2",
		"USB MPH",
		"USB DR",
		"Encryption Core",
		"I2C Boot Sequencer",
		"JTAG",
		"reserved",
		"eSDHC",
		"PCI1",
		"PCI2",
		"DMA",
		"QUICC Engine 00",
		"QUICC Engine 01",
		"QUICC Engine 10",
		"QUICC Engine 11",
		"reserved",
		"reserved",
		"reserved",
		"reserved",
		"SATA1",
		"SATA2",
		"SATA3",
		"SATA4",
		"reserved",
		"PCI Express 1",
		"PCI Express 2",
		"TDM-DMAC"
	};
	static char *transfer[] = {
		"Address-only, Clean Block",
		"Address-only, lwarx reservation set",
		"Single-beat or Burst write",
		"reserved",
		"Address-only, Flush Block",
		"reserved",
		"Burst write",
		"reserved",
		"Address-only, sync",
		"Address-only, tlbsync",
		"Single-beat or Burst read",
		"Single-beat or Burst read",
		"Address-only, Kill Block",
		"Address-only, icbi",
		"Burst read",
		"reserved",
		"Address-only, eieio",
		"reserved",
		"Single-beat write",
		"reserved",
		"ecowx - Illegal single-beat write",
		"reserved",
		"reserved",
		"reserved",
		"Address-only, TLB Invalidate",
		"reserved",
		"Single-beat or Burst read",
		"reserved",
		"eciwx - Illegal single-beat read",
		"reserved",
		"Burst read",
		"reserved"
	};

	int etype = (gd->arch.arbiter_event_attributes & AEATR_EVENT)
		    >> AEATR_EVENT_SHIFT;
	int mstr_id = (gd->arch.arbiter_event_attributes & AEATR_MSTR_ID)
		      >> AEATR_MSTR_ID_SHIFT;
	int tbst = (gd->arch.arbiter_event_attributes & AEATR_TBST)
		   >> AEATR_TBST_SHIFT;
	int tsize = (gd->arch.arbiter_event_attributes & AEATR_TSIZE)
		    >> AEATR_TSIZE_SHIFT;
	int ttype = (gd->arch.arbiter_event_attributes & AEATR_TTYPE)
		    >> AEATR_TTYPE_SHIFT;

	if (!force && !gd->arch.arbiter_event_address)
		return 0;

	puts("Arbiter Event Status:\n");
	printf("       Event Address: 0x%08lX\n",
	       gd->arch.arbiter_event_address);
	printf("       Event Type:    0x%1x  = %s\n", etype, event[etype]);
	printf("       Master ID:     0x%02x = %s\n", mstr_id, master[mstr_id]);
	printf("       Transfer Size: 0x%1x  = %d bytes\n", (tbst<<3) | tsize,
				tbst ? (tsize ? tsize : 8) : 16 + 8 * tsize);
	printf("       Transfer Type: 0x%02x = %s\n", ttype, transfer[ttype]);

	return gd->arch.arbiter_event_address;
}

#elif defined(CONFIG_DISPLAY_AER_BRIEF)

static int print_83xx_arb_event(int force)
{
	if (!force && !gd->arch.arbiter_event_address)
		return 0;

	printf("Arbiter Event Status: AEATR=0x%08lX, AEADR=0x%08lX\n",
		gd->arch.arbiter_event_attributes,
		gd->arch.arbiter_event_address);

	return gd->arch.arbiter_event_address;
}
#endif /* CONFIG_DISPLAY_AER_xxxx */

#ifndef CONFIG_CPU_MPC83XX
/*
 * Figure out the cause of the reset
 */
int prt_83xx_rsr(void)
{
	static struct {
		ulong mask;
		char *desc;
	} bits[] = {
		{
		RSR_SWSR, "Software Soft"}, {
		RSR_SWHR, "Software Hard"}, {
		RSR_JSRS, "JTAG Soft"}, {
		RSR_CSHR, "Check Stop"}, {
		RSR_SWRS, "Software Watchdog"}, {
		RSR_BMRS, "Bus Monitor"}, {
		RSR_SRS,  "External/Internal Soft"}, {
		RSR_HRS,  "External/Internal Hard"}
	};
	static int n = ARRAY_SIZE(bits);
	ulong rsr = gd->arch.reset_status;
	int i;
	char *sep;

	puts("Reset Status:");

	sep = " ";
	for (i = 0; i < n; i++)
		if (rsr & bits[i].mask) {
			printf("%s%s", sep, bits[i].desc);
			sep = ", ";
		}
	puts("\n");

#if defined(CONFIG_DISPLAY_AER_FULL) || defined(CONFIG_DISPLAY_AER_BRIEF)
	print_83xx_arb_event(rsr & RSR_BMRS);
#endif
	puts("\n");

	return 0;
}
#endif
