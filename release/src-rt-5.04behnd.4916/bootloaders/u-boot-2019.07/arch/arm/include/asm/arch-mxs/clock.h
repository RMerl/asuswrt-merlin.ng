/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Freescale i.MX23/i.MX28 Clock
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 */

#ifndef __CLOCK_H__
#define __CLOCK_H__

enum mxc_clock {
	MXC_ARM_CLK = 0,
	MXC_AHB_CLK,
	MXC_IPG_CLK,
	MXC_EMI_CLK,
	MXC_GPMI_CLK,
	MXC_IO0_CLK,
	MXC_IO1_CLK,
	MXC_XTAL_CLK,
	MXC_SSP0_CLK,
#ifdef CONFIG_MX28
	MXC_SSP1_CLK,
	MXC_SSP2_CLK,
	MXC_SSP3_CLK,
#endif
};

enum mxs_ioclock {
	MXC_IOCLK0 = 0,
	MXC_IOCLK1,
};

enum mxs_sspclock {
	MXC_SSPCLK0 = 0,
#ifdef CONFIG_MX28
	MXC_SSPCLK1,
	MXC_SSPCLK2,
	MXC_SSPCLK3,
#endif
};

uint32_t mxc_get_clock(enum mxc_clock clk);

void mxs_set_ioclk(enum mxs_ioclock io, uint32_t freq);
void mxs_set_sspclk(enum mxs_sspclock ssp, uint32_t freq, int xtal);
void mxs_set_ssp_busclock(unsigned int bus, uint32_t freq);
void mxs_set_lcdclk(uint32_t __maybe_unused lcd_base, uint32_t freq);

/* Compatibility with the FEC Ethernet driver */
#define	imx_get_fecclk()	mxc_get_clock(MXC_AHB_CLK)

#endif	/* __CLOCK_H__ */
