// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#include <config.h>
#include <dm.h>
#include <errno.h>
#include <fdt_support.h>
#include <pci.h>
#include <malloc.h>
#include <miiphy.h>
#include <misc.h>
#include <net.h>
#include <netdev.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/libfdt.h>

#include <asm/arch/csrs/csrs-xcv.h>

#define XCVX_BASE		0x87E0DB000000ULL

/* Initialize XCV block */
void xcv_init_hw(void)
{
	union xcvx_reset reset;
	union xcvx_dll_ctl xcv_dll_ctl;

	/* Take the DLL out of reset */
	reset.u = readq(XCVX_BASE + XCVX_RESET(0));
	reset.s.dllrst = 0;
	writeq(reset.u, XCVX_BASE + XCVX_RESET(0));

	/* Take the clock tree out of reset */
	reset.u = readq(XCVX_BASE + XCVX_RESET(0));
	reset.s.clkrst = 0;
	writeq(reset.u, XCVX_BASE + XCVX_RESET(0));

	/* Once the 125MHz ref clock is stable, wait 10us for DLL to lock */
	udelay(10);

	/* Optionally, bypass the DLL setting */
	xcv_dll_ctl.u = readq(XCVX_BASE + XCVX_DLL_CTL(0));
	xcv_dll_ctl.s.clkrx_set = 0;
	xcv_dll_ctl.s.clkrx_byp = 1;
	xcv_dll_ctl.s.clktx_byp = 0;
	writeq(xcv_dll_ctl.u, XCVX_BASE + XCVX_DLL_CTL(0));

	/* Enable the compensation controller */
	reset.u = readq(XCVX_BASE + XCVX_RESET(0));
	reset.s.comp = 1;
	writeq(reset.u, XCVX_BASE + XCVX_RESET(0));
	reset.u = readq(XCVX_BASE + XCVX_RESET(0));

	/* Wait for 1040 reference clock cycles for the compensation state
	 * machine lock.
	 */
	udelay(100);

	/* Enable the XCV block */
	reset.u = readq(XCVX_BASE + XCVX_RESET(0));
	reset.s.enable = 1;
	writeq(reset.u, XCVX_BASE + XCVX_RESET(0));

	/* set XCV(0)_RESET[CLKRST] to 1 */
	reset.u = readq(XCVX_BASE + XCVX_RESET(0));
	reset.s.clkrst = 1;
	writeq(reset.u, XCVX_BASE + XCVX_RESET(0));
}

/*
 * Configure XCV link based on the speed
 * link_up   : Set to 1 when link is up otherwise 0
 * link_speed: The speed of the link.
 */
void xcv_setup_link(bool link_up, int link_speed)
{
	union xcvx_ctl xcv_ctl;
	union xcvx_reset reset;
	union xcvx_batch_crd_ret xcv_crd_ret;
	int speed = 2;

	/* Check RGMII link */
	if (link_speed == 100)
		speed = 1;
	else if (link_speed == 10)
		speed = 0;

	if (link_up) {
		/* Set operating speed */
		xcv_ctl.u = readq(XCVX_BASE + XCVX_CTL(0));
		xcv_ctl.s.speed = speed;
		writeq(xcv_ctl.u, XCVX_BASE + XCVX_CTL(0));

		/* Datapaths come out of reset
		 * - The datapath resets will disengage BGX from the
		 *   RGMII interface
		 * - XCV will continue to return TX credits for each tick
		 *   that is sent on the TX data path
		 */
		reset.u = readq(XCVX_BASE + XCVX_RESET(0));
		reset.s.tx_dat_rst_n = 1;
		reset.s.rx_dat_rst_n = 1;
		writeq(reset.u, XCVX_BASE + XCVX_RESET(0));

		/* Enable packet flow */
		reset.u = readq(XCVX_BASE + XCVX_RESET(0));
		reset.s.tx_pkt_rst_n = 1;
		reset.s.rx_pkt_rst_n = 1;
		writeq(reset.u, XCVX_BASE + XCVX_RESET(0));

		xcv_crd_ret.u = readq(XCVX_BASE + XCVX_BATCH_CRD_RET(0));
		xcv_crd_ret.s.crd_ret = 1;
		writeq(xcv_crd_ret.u, XCVX_BASE + XCVX_BATCH_CRD_RET(0));
	} else {
		/* Enable packet flow */
		reset.u = readq(XCVX_BASE + XCVX_RESET(0));
		reset.s.tx_pkt_rst_n = 0;
		reset.s.rx_pkt_rst_n = 0;
		writeq(reset.u, XCVX_BASE + XCVX_RESET(0));
		reset.u = readq(XCVX_BASE + XCVX_RESET(0));
	}
}
