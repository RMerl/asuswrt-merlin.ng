/*
 * sun50i H6 DDR3-1333 timings, as programmed by Allwinner's boot0
 * for some TV boxes with the H6 and DDR3 memory.
 *
 * The chips are probably able to be driven by a faster clock, but boot0
 * uses a more conservative timing (as usual).
 *
 * (C) Copyright 2018,2019 Arm Ltd.
 *   based on previous work by:
 *   (C) Copyright 2017      Icenowy Zheng <icenowy@aosc.io>
 *
 * References used:
 * - JEDEC DDR3 SDRAM standard:	JESD79-3F.pdf
 * - Samsung K4B2G0446D datasheet
 * - ZynqMP UG1087 register DDRC/PHY documentation
 *
 * Many thanks to Jernej Skrabec for contributing some fixes!
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/dram.h>
#include <asm/arch/cpu.h>

/*
 * Only the first four are used for DDR3(?)
 * MR0: BL8, seq. read burst, no test, fast exit (DLL on), no DLL reset,
 *	CAS latency (CL): 11, write recovery (WR): 12
 * MR1: DLL enabled, output strength RZQ/6, Rtt_norm RZQ/2,
 *	write levelling disabled, TDQS disabled, output buffer enabled
 * MR2: manual full array self refresh, dynamic ODT off,
 *	CAS write latency (CWL): 8
 */
static u32 mr_ddr3[7] = {
	0x00001c70, 0x00000040, 0x00000018, 0x00000000,
	0x00000000, 0x00000400, 0x00000848,
};

/* TODO: flexible timing */
void mctl_set_timing_params(struct dram_para *para)
{
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;
	struct sunxi_mctl_phy_reg * const mctl_phy =
			(struct sunxi_mctl_phy_reg *)SUNXI_DRAM_PHY0_BASE;
	int i;

	u8 tccd		= 2;			/* JEDEC: 4nCK */
	u8 tfaw		= ns_to_t(50);		/* JEDEC: 30 ns w/ 1K pages */
	u8 trrd		= max(ns_to_t(6), 4);	/* JEDEC: max(6 ns, 4nCK) */
	u8 trcd		= ns_to_t(15);		/* JEDEC: 13.5 ns */
	u8 trc		= ns_to_t(53);		/* JEDEC: 49.5 ns */
	u8 txp		= max(ns_to_t(6), 3);	/* JEDEC: max(6 ns, 3nCK) */
	u8 twtr		= max(ns_to_t(8), 2);	/* JEDEC: max(7.5 ns, 4nCK) */
	u8 trtp		= max(ns_to_t(8), 2);	/* JEDEC: max(7.5 ns, 4nCK) */
	u8 twr		= ns_to_t(15);		/* JEDEC: 15 ns */
	u8 trp		= ns_to_t(15);		/* JEDEC: >= 13.75 ns */
	u8 tras		= ns_to_t(38);		/* JEDEC >= 36 ns, <= 9*trefi */
	u8 twtr_sa	= 2;			/* ? */
	u8 tcksrea	= 4;			/* ? */
	u16 trefi	= ns_to_t(7800) / 32;	/* JEDEC: 7.8us@Tcase <= 85C */
	u16 trfc	= ns_to_t(350);		/* JEDEC: 160 ns for 2Gb */
	u16 txsr	= 4;			/* ? */

	u8 tmrw		= 0;			/* ? */
	u8 tmrd		= 4;			/* JEDEC: 4nCK */
	u8 tmod		= max(ns_to_t(15), 12);	/* JEDEC: max(15 ns, 12nCK) */
	u8 tcke		= max(ns_to_t(6), 3);	/* JEDEC: max(5.625 ns, 3nCK) */
	u8 tcksrx	= max(ns_to_t(10), 5);	/* JEDEC: max(10 ns, 5nCK) */
	u8 tcksre	= max(ns_to_t(10), 5);	/* JEDEC: max(10 ns, 5nCK) */
	u8 tckesr	= tcke + 1;		/* JEDEC: tCKE(min) + 1nCK */
	u8 trasmax	= 24;			/* JEDEC: tREFI * 9 */
	u8 txs		= ns_to_t(360) / 32;	/* JEDEC: max(5nCK,tRFC+10ns) */
	u8 txsdll	= 4;			/* JEDEC: 512 nCK */
	u8 txsabort	= 4;			/* ? */
	u8 txsfast	= 4;			/* ? */
	u8 tcl		= 6;			/* JEDEC: CL / 2 => 6 */
	u8 tcwl		= 4;			/* JEDEC: 8 */
	u8 t_rdata_en	= 7;			/* ? */

	u32 tdinit0	= (500 * CONFIG_DRAM_CLK) + 1;	/* 500us */
	u32 tdinit1	= (360 * CONFIG_DRAM_CLK) / 1000 + 1;
	u32 tdinit2	= (200 * CONFIG_DRAM_CLK) + 1;
	u32 tdinit3	= (1 * CONFIG_DRAM_CLK) + 1;	/* 1us */

	u8 twtp		= tcwl + 2 + twr;	/* (WL + BL / 2 + tWR) / 2 */
	u8 twr2rd	= tcwl + 2 + twtr;	/* (WL + BL / 2 + tWTR) / 2 */
	u8 trd2wr	= 5;			/* (RL + BL / 2 + 2 - WL) / 2 */

	if (tcl + 1 >= trtp + trp)
		trtp = tcl + 2 - trp;

	/* set mode registers */
	for (i = 0; i < ARRAY_SIZE(mr_ddr3); i++)
		writel(mr_ddr3[i], &mctl_phy->mr[i]);

	/* set DRAM timing */
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
	writel(txsr, &mctl_ctl->dramtmg[14]);

	clrsetbits_le32(&mctl_ctl->init[0], (3 << 30), (1 << 30));
	writel(0, &mctl_ctl->dfimisc);
	clrsetbits_le32(&mctl_ctl->rankctl, 0xff0, 0x660);

	/*
	 * Set timing registers of the PHY.
	 * Note: the PHY is clocked 2x from the DRAM frequency.
	 */
	writel((trrd << 25) | (tras << 17) | (trp << 9) | (trtp << 1),
	       &mctl_phy->dtpr[0]);
	writel((tfaw << 17) | 0x28000400 | (tmrd << 1), &mctl_phy->dtpr[1]);
	writel(((txs << 6) - 1) | (tcke << 17), &mctl_phy->dtpr[2]);
	writel(((txsdll << 22) - (0x1 << 16)) | twtr_sa | (tcksrea << 8),
	       &mctl_phy->dtpr[3]);
	writel((txp << 1) | (trfc << 17) | 0x800, &mctl_phy->dtpr[4]);
	writel((trc << 17) | (trcd << 9) | (twtr << 1), &mctl_phy->dtpr[5]);
	writel(0x0505, &mctl_phy->dtpr[6]);

	/* Configure DFI timing */
	writel(tcl | 0x2000200 | (t_rdata_en << 16) | 0x808000,
	       &mctl_ctl->dfitmg0);
	writel(0x040201, &mctl_ctl->dfitmg1);

	/* Configure PHY timing. Zynq uses different registers. */
	writel(tdinit0 | (tdinit1 << 20), &mctl_phy->ptr[3]);
	writel(tdinit2 | (tdinit3 << 18), &mctl_phy->ptr[4]);

	/* set refresh timing */
	writel((trefi << 16) | trfc, &mctl_ctl->rfshtmg);
}
