/*
 * sun50i H616 LPDDR3 timings, as programmed by Allwinner's boot0
 *
 * The chips are probably able to be driven by a faster clock, but boot0
 * uses a more conservative timing (as usual).
 *
 * (C) Copyright 2020 Jernej Skrabec <jernej.skrabec@siol.net>
 * Based on H6 DDR3 timings:
 * (C) Copyright 2018,2019 Arm Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/dram.h>
#include <asm/arch/cpu.h>

void mctl_set_timing_params(const struct dram_para *para)
{
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;

	u8 tccd		= 2;
	u8 tfaw		= ns_to_t(50);
	u8 trrd		= max(ns_to_t(6), 4);
	u8 trcd		= ns_to_t(24);
	u8 trc		= ns_to_t(70);
	u8 txp		= max(ns_to_t(8), 3);
	u8 trtp		= max(ns_to_t(8), 2);
	u8 trp		= ns_to_t(27);
	u8 tras		= ns_to_t(41);
	u16 trefi	= ns_to_t(7800) / 64;
	u16 trfc	= ns_to_t(210);
	u16 txsr	= 88;

	u8 tmrw		= 5;
	u8 tmrd		= 5;
	u8 tmod		= max(ns_to_t(15), 12);
	u8 tcke		= max(ns_to_t(6), 3);
	u8 tcksrx	= max(ns_to_t(12), 4);
	u8 tcksre	= max(ns_to_t(12), 4);
	u8 tckesr	= tcke + 2;
	u8 trasmax	= (para->clk / 2) / 16;
	u8 txs		= ns_to_t(360) / 32;
	u8 txsdll	= 16;
	u8 txsabort	= 4;
	u8 txsfast	= 4;
	u8 tcl		= 7;
	u8 tcwl		= 4;
	u8 t_rdata_en	= 12;
	u8 t_wr_lat	= 6;

	u8 twtp		= 16;
	u8 twr2rd	= trtp + 9;
	u8 trd2wr	= 13;

	/* DRAM timing grabbed from tvbox with LPDDR3 memory */
	writel((twtp << 24) | (tfaw << 16) | (trasmax << 8) | tras,
	       &mctl_ctl->dramtmg[0]);
	writel((txp << 16) | (trtp << 8) | trc, &mctl_ctl->dramtmg[1]);
	writel((tcwl << 24) | (tcl << 16) | (trd2wr << 8) | twr2rd,
	       &mctl_ctl->dramtmg[2]);
	writel((tmrw << 20) | (tmrd << 12) | tmod, &mctl_ctl->dramtmg[3]);
	writel((trcd << 24) | (tccd << 16) | (trrd << 8) | trp,
	       &mctl_ctl->dramtmg[4]);
	writel((tcksrx << 24) | (tcksre << 16) | (tckesr << 8) | tcke,
	       &mctl_ctl->dramtmg[5]);
	/* Value suggested by ZynqMP manual and used by libdram */
	writel((txp + 2) | 0x02020000, &mctl_ctl->dramtmg[6]);
	writel((txsfast << 24) | (txsabort << 16) | (txsdll << 8) | txs,
	       &mctl_ctl->dramtmg[8]);
	writel(0x00020208, &mctl_ctl->dramtmg[9]);
	writel(0xE0C05, &mctl_ctl->dramtmg[10]);
	writel(0x440C021C, &mctl_ctl->dramtmg[11]);
	writel(8, &mctl_ctl->dramtmg[12]);
	writel(0xA100002, &mctl_ctl->dramtmg[13]);
	writel(txsr, &mctl_ctl->dramtmg[14]);

	writel(0x4f0112, &mctl_ctl->init[0]);
	writel(0x420000, &mctl_ctl->init[1]);
	writel(0xd05, &mctl_ctl->init[2]);
	writel(0x83001c, &mctl_ctl->init[3]);
	writel(0x00010000, &mctl_ctl->init[4]);

	writel(0, &mctl_ctl->dfimisc);
	clrsetbits_le32(&mctl_ctl->rankctl, 0xff0, 0x660);

	/* Configure DFI timing */
	writel(t_wr_lat | 0x2000000 | (t_rdata_en << 16) | 0x808000,
	       &mctl_ctl->dfitmg0);
	writel(0x100202, &mctl_ctl->dfitmg1);

	/* set refresh timing */
	writel((trefi << 16) | trfc, &mctl_ctl->rfshtmg);
}
