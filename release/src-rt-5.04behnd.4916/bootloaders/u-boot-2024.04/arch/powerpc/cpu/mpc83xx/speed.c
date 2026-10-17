// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 */

#ifndef CONFIG_CLK_MPC83XX

#include <common.h>
#include <clock_legacy.h>
#include <mpc83xx.h>
#include <command.h>
#include <vsprintf.h>
#include <asm/global_data.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

/* ----------------------------------------------------------------- */

typedef enum {
	_unk,
	_off,
	_byp,
	_x8,
	_x4,
	_x2,
	_x1,
	_1x,
	_1_5x,
	_2x,
	_2_5x,
	_3x
} mult_t;

typedef struct {
	mult_t core_csb_ratio;
	mult_t vco_divider;
} corecnf_t;

static corecnf_t corecnf_tab[] = {
	{_byp, _byp},		/* 0x00 */
	{_byp, _byp},		/* 0x01 */
	{_byp, _byp},		/* 0x02 */
	{_byp, _byp},		/* 0x03 */
	{_byp, _byp},		/* 0x04 */
	{_byp, _byp},		/* 0x05 */
	{_byp, _byp},		/* 0x06 */
	{_byp, _byp},		/* 0x07 */
	{_1x, _x2},		/* 0x08 */
	{_1x, _x4},		/* 0x09 */
	{_1x, _x8},		/* 0x0A */
	{_1x, _x8},		/* 0x0B */
	{_1_5x, _x2},		/* 0x0C */
	{_1_5x, _x4},		/* 0x0D */
	{_1_5x, _x8},		/* 0x0E */
	{_1_5x, _x8},		/* 0x0F */
	{_2x, _x2},		/* 0x10 */
	{_2x, _x4},		/* 0x11 */
	{_2x, _x8},		/* 0x12 */
	{_2x, _x8},		/* 0x13 */
	{_2_5x, _x2},		/* 0x14 */
	{_2_5x, _x4},		/* 0x15 */
	{_2_5x, _x8},		/* 0x16 */
	{_2_5x, _x8},		/* 0x17 */
	{_3x, _x2},		/* 0x18 */
	{_3x, _x4},		/* 0x19 */
	{_3x, _x8},		/* 0x1A */
	{_3x, _x8},		/* 0x1B */
};

/* ----------------------------------------------------------------- */

/*
 *
 */
int get_clocks(void)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	u32 pci_sync_in;
	u8 spmf;
	u8 clkin_div;
	u32 sccr;
	u32 corecnf_tab_index;
	u8 corepll;
	u32 lcrr;

	u32 csb_clk;
#if defined(CONFIG_ARCH_MPC8308) || defined(CONFIG_ARCH_MPC831X) || \
	defined(CONFIG_ARCH_MPC834X) || defined(CONFIG_ARCH_MPC837X)
	u32 tsec1_clk;
	u32 tsec2_clk;
	u32 usbdr_clk;
#endif
#ifdef CONFIG_ARCH_MPC834X
	u32 usbmph_clk;
#endif
	u32 core_clk;
	u32 i2c1_clk;
#if !defined(CONFIG_ARCH_MPC832X)
	u32 i2c2_clk;
#endif
#if defined(CONFIG_FSL_ESDHC)
	u32 sdhc_clk;
#endif
	u32 enc_clk;
	u32 lbiu_clk;
	u32 lclk_clk;
	u32 mem_clk;
#if defined(CONFIG_ARCH_MPC8360)
	u32 mem_sec_clk;
#endif
#if defined(CONFIG_QE)
	u32 qepmf;
	u32 qepdf;
	u32 qe_clk;
	u32 brg_clk;
#endif
#if defined(CONFIG_ARCH_MPC8308) || defined(CONFIG_ARCH_MPC831X) || \
	defined(CONFIG_ARCH_MPC837X)
	u32 pciexp1_clk;
	u32 pciexp2_clk;
#endif
#if defined(CONFIG_ARCH_MPC837X)
	u32 sata_clk;
#endif

	if ((im->sysconf.immrbar & IMMRBAR_BASE_ADDR) != (u32) im)
		return -1;

	clkin_div = ((im->clk.spmr & SPMR_CKID) >> SPMR_CKID_SHIFT);

	if (im->reset.rcwh & HRCWH_PCI_HOST) {
#if CONFIG_SYS_CLK_FREQ != 0
		pci_sync_in = get_board_sys_clk() / (1 + clkin_div);
#else
		pci_sync_in = 0xDEADBEEF;
#endif
	} else {
#if defined(CONFIG_83XX_PCICLK)
		pci_sync_in = CONFIG_83XX_PCICLK;
#else
		pci_sync_in = 0xDEADBEEF;
#endif
	}

	spmf = (im->clk.spmr & SPMR_SPMF) >> SPMR_SPMF_SHIFT;
	csb_clk = pci_sync_in * (1 + clkin_div) * spmf;

	sccr = im->clk.sccr;

#if defined(CONFIG_ARCH_MPC8308) || defined(CONFIG_ARCH_MPC831X) || \
	defined(CONFIG_ARCH_MPC834X) || defined(CONFIG_ARCH_MPC837X)
	switch ((sccr & SCCR_TSEC1CM) >> SCCR_TSEC1CM_SHIFT) {
	case 0:
		tsec1_clk = 0;
		break;
	case 1:
		tsec1_clk = csb_clk;
		break;
	case 2:
		tsec1_clk = csb_clk / 2;
		break;
	case 3:
		tsec1_clk = csb_clk / 3;
		break;
	default:
		/* unknown SCCR_TSEC1CM value */
		return -2;
	}
#endif

#if defined(CONFIG_ARCH_MPC830X) || defined(CONFIG_ARCH_MPC831X) || \
	defined(CONFIG_ARCH_MPC834X) || defined(CONFIG_ARCH_MPC837X)
	switch ((sccr & SCCR_USBDRCM) >> SCCR_USBDRCM_SHIFT) {
	case 0:
		usbdr_clk = 0;
		break;
	case 1:
		usbdr_clk = csb_clk;
		break;
	case 2:
		usbdr_clk = csb_clk / 2;
		break;
	case 3:
		usbdr_clk = csb_clk / 3;
		break;
	default:
		/* unknown SCCR_USBDRCM value */
		return -3;
	}
#endif

#if defined(CONFIG_ARCH_MPC8308) || defined(CONFIG_ARCH_MPC834X) || \
	defined(CONFIG_ARCH_MPC837X)
	switch ((sccr & SCCR_TSEC2CM) >> SCCR_TSEC2CM_SHIFT) {
	case 0:
		tsec2_clk = 0;
		break;
	case 1:
		tsec2_clk = csb_clk;
		break;
	case 2:
		tsec2_clk = csb_clk / 2;
		break;
	case 3:
		tsec2_clk = csb_clk / 3;
		break;
	default:
		/* unknown SCCR_TSEC2CM value */
		return -4;
	}
#elif defined(CONFIG_ARCH_MPC8313)
	tsec2_clk = tsec1_clk;

	if (!(sccr & SCCR_TSEC1ON))
		tsec1_clk = 0;
	if (!(sccr & SCCR_TSEC2ON))
		tsec2_clk = 0;
#endif

#if defined(CONFIG_ARCH_MPC834X)
	switch ((sccr & SCCR_USBMPHCM) >> SCCR_USBMPHCM_SHIFT) {
	case 0:
		usbmph_clk = 0;
		break;
	case 1:
		usbmph_clk = csb_clk;
		break;
	case 2:
		usbmph_clk = csb_clk / 2;
		break;
	case 3:
		usbmph_clk = csb_clk / 3;
		break;
	default:
		/* unknown SCCR_USBMPHCM value */
		return -5;
	}

	if (usbmph_clk != 0 && usbdr_clk != 0 && usbmph_clk != usbdr_clk) {
		/* if USB MPH clock is not disabled and
		 * USB DR clock is not disabled then
		 * USB MPH & USB DR must have the same rate
		 */
		return -6;
	}
#endif
	switch ((sccr & SCCR_ENCCM) >> SCCR_ENCCM_SHIFT) {
	case 0:
		enc_clk = 0;
		break;
	case 1:
		enc_clk = csb_clk;
		break;
	case 2:
		enc_clk = csb_clk / 2;
		break;
	case 3:
		enc_clk = csb_clk / 3;
		break;
	default:
		/* unknown SCCR_ENCCM value */
		return -7;
	}

#if defined(CONFIG_FSL_ESDHC)
	switch ((sccr & SCCR_SDHCCM) >> SCCR_SDHCCM_SHIFT) {
	case 0:
		sdhc_clk = 0;
		break;
	case 1:
		sdhc_clk = csb_clk;
		break;
	case 2:
		sdhc_clk = csb_clk / 2;
		break;
	case 3:
		sdhc_clk = csb_clk / 3;
		break;
	default:
		/* unknown SCCR_SDHCCM value */
		return -8;
	}
#endif

#if defined(CONFIG_ARCH_MPC834X)
	i2c1_clk = tsec2_clk;
#elif defined(CONFIG_ARCH_MPC8360)
	i2c1_clk = csb_clk;
#elif defined(CONFIG_ARCH_MPC832X)
	i2c1_clk = enc_clk;
#elif defined(CONFIG_ARCH_MPC8308) || defined(CONFIG_ARCH_MPC831X)
	i2c1_clk = enc_clk;
#elif defined(CONFIG_FSL_ESDHC)
	i2c1_clk = sdhc_clk;
#elif defined(CONFIG_ARCH_MPC837X)
	i2c1_clk = enc_clk;
#endif
#if !defined(CONFIG_ARCH_MPC832X)
	i2c2_clk = csb_clk; /* i2c-2 clk is equal to csb clk */
#endif

#if defined(CONFIG_ARCH_MPC8308) || defined(CONFIG_ARCH_MPC831X) || \
	defined(CONFIG_ARCH_MPC837X)
	switch ((sccr & SCCR_PCIEXP1CM) >> SCCR_PCIEXP1CM_SHIFT) {
	case 0:
		pciexp1_clk = 0;
		break;
	case 1:
		pciexp1_clk = csb_clk;
		break;
	case 2:
		pciexp1_clk = csb_clk / 2;
		break;
	case 3:
		pciexp1_clk = csb_clk / 3;
		break;
	default:
		/* unknown SCCR_PCIEXP1CM value */
		return -9;
	}

	switch ((sccr & SCCR_PCIEXP2CM) >> SCCR_PCIEXP2CM_SHIFT) {
	case 0:
		pciexp2_clk = 0;
		break;
	case 1:
		pciexp2_clk = csb_clk;
		break;
	case 2:
		pciexp2_clk = csb_clk / 2;
		break;
	case 3:
		pciexp2_clk = csb_clk / 3;
		break;
	default:
		/* unknown SCCR_PCIEXP2CM value */
		return -10;
	}
#endif

#if defined(CONFIG_ARCH_MPC837X)
	switch ((sccr & SCCR_SATA1CM) >> SCCR_SATA1CM_SHIFT) {
	case 0:
		sata_clk = 0;
		break;
	case 1:
		sata_clk = csb_clk;
		break;
	case 2:
		sata_clk = csb_clk / 2;
		break;
	case 3:
		sata_clk = csb_clk / 3;
		break;
	default:
		/* unknown SCCR_SATA1CM value */
		return -11;
	}
#endif

	lbiu_clk = csb_clk *
		   (1 + ((im->clk.spmr & SPMR_LBIUCM) >> SPMR_LBIUCM_SHIFT));
	lcrr = (im->im_lbc.lcrr & LCRR_CLKDIV) >> LCRR_CLKDIV_SHIFT;
	switch (lcrr) {
	case 2:
	case 4:
	case 8:
		lclk_clk = lbiu_clk / lcrr;
		break;
	default:
		/* unknown lcrr */
		return -12;
	}

	mem_clk = csb_clk *
		  (1 + ((im->clk.spmr & SPMR_DDRCM) >> SPMR_DDRCM_SHIFT));
	corepll = (im->clk.spmr & SPMR_COREPLL) >> SPMR_COREPLL_SHIFT;

#if defined(CONFIG_ARCH_MPC8360)
	mem_sec_clk = csb_clk * (1 +
		       ((im->clk.spmr & SPMR_LBIUCM) >> SPMR_LBIUCM_SHIFT));
#endif

	corecnf_tab_index = ((corepll & 0x1F) << 2) | ((corepll & 0x60) >> 5);
	if (corecnf_tab_index > (ARRAY_SIZE(corecnf_tab))) {
		/* corecnf_tab_index is too high, possibly wrong value */
		return -11;
	}
	switch (corecnf_tab[corecnf_tab_index].core_csb_ratio) {
	case _byp:
	case _x1:
	case _1x:
		core_clk = csb_clk;
		break;
	case _1_5x:
		core_clk = (3 * csb_clk) / 2;
		break;
	case _2x:
		core_clk = 2 * csb_clk;
		break;
	case _2_5x:
		core_clk = (5 * csb_clk) / 2;
		break;
	case _3x:
		core_clk = 3 * csb_clk;
		break;
	default:
		/* unknown core to csb ratio */
		return -13;
	}

#if defined(CONFIG_QE)
	qepmf = (im->clk.spmr & SPMR_CEPMF) >> SPMR_CEPMF_SHIFT;
	qepdf = (im->clk.spmr & SPMR_CEPDF) >> SPMR_CEPDF_SHIFT;
	qe_clk = (pci_sync_in * qepmf) / (1 + qepdf);
	brg_clk = qe_clk / 2;
#endif

	gd->arch.csb_clk = csb_clk;
#if defined(CONFIG_ARCH_MPC8308) || defined(CONFIG_ARCH_MPC831X) || \
	defined(CONFIG_ARCH_MPC834X) || defined(CONFIG_ARCH_MPC837X)
	gd->arch.tsec1_clk = tsec1_clk;
	gd->arch.tsec2_clk = tsec2_clk;
	gd->arch.usbdr_clk = usbdr_clk;
#endif
#if defined(CONFIG_ARCH_MPC834X)
	gd->arch.usbmph_clk = usbmph_clk;
#endif
#if defined(CONFIG_FSL_ESDHC)
	gd->arch.sdhc_clk = sdhc_clk;
#endif
	gd->arch.core_clk = core_clk;
	gd->arch.i2c1_clk = i2c1_clk;
#if !defined(CONFIG_ARCH_MPC832X)
	gd->arch.i2c2_clk = i2c2_clk;
#endif
	gd->arch.enc_clk = enc_clk;
	gd->arch.lbiu_clk = lbiu_clk;
	gd->arch.lclk_clk = lclk_clk;
	gd->mem_clk = mem_clk;
#if defined(CONFIG_ARCH_MPC8360)
	gd->arch.mem_sec_clk = mem_sec_clk;
#endif
#if defined(CONFIG_QE)
	gd->arch.qe_clk = qe_clk;
	gd->arch.brg_clk = brg_clk;
#endif
#if defined(CONFIG_ARCH_MPC8308) || defined(CONFIG_ARCH_MPC831X) || \
	defined(CONFIG_ARCH_MPC837X)
	gd->arch.pciexp1_clk = pciexp1_clk;
	gd->arch.pciexp2_clk = pciexp2_clk;
#endif
#if defined(CONFIG_ARCH_MPC837X)
	gd->arch.sata_clk = sata_clk;
#endif
	gd->pci_clk = pci_sync_in;
	gd->cpu_clk = gd->arch.core_clk;
	gd->bus_clk = gd->arch.csb_clk;
	return 0;

}

/********************************************
 * get_bus_freq
 * return system bus freq in Hz
 *********************************************/
ulong get_bus_freq(ulong dummy)
{
	return gd->arch.csb_clk;
}

/********************************************
 * get_ddr_freq
 * return ddr bus freq in Hz
 *********************************************/
ulong get_ddr_freq(ulong dummy)
{
	return gd->mem_clk;
}

int get_serial_clock(void)
{
	return get_bus_freq(0);
}

static int do_clocks(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	char buf[32];

	printf("Clock configuration:\n");
	printf("  Core:                %-4s MHz\n",
	       strmhz(buf, gd->arch.core_clk));
	printf("  Coherent System Bus: %-4s MHz\n",
	       strmhz(buf, gd->arch.csb_clk));
#if defined(CONFIG_QE)
	printf("  QE:                  %-4s MHz\n",
	       strmhz(buf, gd->arch.qe_clk));
	printf("  BRG:                 %-4s MHz\n",
	       strmhz(buf, gd->arch.brg_clk));
#endif
	printf("  Local Bus Controller:%-4s MHz\n",
	       strmhz(buf, gd->arch.lbiu_clk));
	printf("  Local Bus:           %-4s MHz\n",
	       strmhz(buf, gd->arch.lclk_clk));
	printf("  DDR:                 %-4s MHz\n", strmhz(buf, gd->mem_clk));
#if defined(CONFIG_ARCH_MPC8360)
	printf("  DDR Secondary:       %-4s MHz\n",
	       strmhz(buf, gd->arch.mem_sec_clk));
#endif
	printf("  SEC:                 %-4s MHz\n",
	       strmhz(buf, gd->arch.enc_clk));
	printf("  I2C1:                %-4s MHz\n",
	       strmhz(buf, gd->arch.i2c1_clk));
#if !defined(CONFIG_ARCH_MPC832X)
	printf("  I2C2:                %-4s MHz\n",
	       strmhz(buf, gd->arch.i2c2_clk));
#endif
#if defined(CONFIG_FSL_ESDHC)
	printf("  SDHC:                %-4s MHz\n",
	       strmhz(buf, gd->arch.sdhc_clk));
#endif
#if defined(CONFIG_ARCH_MPC8308) || defined(CONFIG_ARCH_MPC831X) || \
	defined(CONFIG_ARCH_MPC834X) || defined(CONFIG_ARCH_MPC837X)
	printf("  TSEC1:               %-4s MHz\n",
	       strmhz(buf, gd->arch.tsec1_clk));
	printf("  TSEC2:               %-4s MHz\n",
	       strmhz(buf, gd->arch.tsec2_clk));
	printf("  USB DR:              %-4s MHz\n",
	       strmhz(buf, gd->arch.usbdr_clk));
#endif
#if defined(CONFIG_ARCH_MPC834X)
	printf("  USB MPH:             %-4s MHz\n",
	       strmhz(buf, gd->arch.usbmph_clk));
#endif
#if defined(CONFIG_ARCH_MPC8308) || defined(CONFIG_ARCH_MPC831X) || \
	defined(CONFIG_ARCH_MPC837X)
	printf("  PCIEXP1:             %-4s MHz\n",
	       strmhz(buf, gd->arch.pciexp1_clk));
	printf("  PCIEXP2:             %-4s MHz\n",
	       strmhz(buf, gd->arch.pciexp2_clk));
#endif
#if defined(CONFIG_ARCH_MPC837X)
	printf("  SATA:                %-4s MHz\n",
	       strmhz(buf, gd->arch.sata_clk));
#endif
	return 0;
}

U_BOOT_CMD(clocks, 1, 0, do_clocks,
	"print clock configuration",
	"    clocks"
);

#endif
