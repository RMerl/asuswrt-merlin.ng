/*
 * sun50i H6 LPDDR3 timings
 *
 * (C) Copyright 2017      Icenowy Zheng <icenowy@aosc.io>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/dram.h>
#include <asm/arch/cpu.h>

static u32 mr_lpddr3[12] = {
	0x00000000, 0x00000043, 0x0000001a, 0x00000001,
	0x00000000, 0x00000000, 0x00000048, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000003,
};

/* TODO: flexible timing */
void mctl_set_timing_params(struct dram_para *para)
{
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;
	struct sunxi_mctl_phy_reg * const mctl_phy =
			(struct sunxi_mctl_phy_reg *)SUNXI_DRAM_PHY0_BASE;
	int i;

	u8 tccd		= 2;
	u8 tfaw		= max(ns_to_t(50), 4);
	u8 trrd		= max(ns_to_t(10), 2);
	u8 trcd		= max(ns_to_t(24), 2);
	u8 trc		= ns_to_t(70);
	u8 txp		= max(ns_to_t(8), 2);
	u8 twtr		= max(ns_to_t(8), 2);
	u8 trtp		= max(ns_to_t(8), 2);
	u8 twr		= max(ns_to_t(15), 2);
	u8 trp		= ns_to_t(18);
	u8 tras		= ns_to_t(42);
	u8 twtr_sa	= ns_to_t(5);
	u8 tcksrea	= ns_to_t(11);
	u16 trefi	= ns_to_t(3900) / 32;
	u16 trfc	= ns_to_t(210);
	u16 txsr	= ns_to_t(220);

	if (CONFIG_DRAM_CLK % 400 == 0) {
		/* Round up these parameters */
		twtr_sa++;
		tcksrea++;
	}

	u8 tmrw		= 5;
	u8 tmrd		= 5;
	u8 tmod		= 12;
	u8 tcke		= 3;
	u8 tcksrx	= 5;
	u8 tcksre	= 5;
	u8 tckesr	= 5;
	u8 trasmax	= CONFIG_DRAM_CLK / 60;
	u8 txs		= 4;
	u8 txsdll	= 4;
	u8 txsabort	= 4;
	u8 txsfast	= 4;

	u8 tcl		= 5; /* CL 10 */
	u8 tcwl		= 3; /* CWL 6 */
	u8 t_rdata_en	= twtr_sa + 8;

	u32 tdinit0	= (200 * CONFIG_DRAM_CLK) + 1;		/* 200us */
	u32 tdinit1	= (100 * CONFIG_DRAM_CLK) / 1000 + 1;	/* 100ns */
	u32 tdinit2	= (11 * CONFIG_DRAM_CLK) + 1;		/* 11us */
	u32 tdinit3	= (1 * CONFIG_DRAM_CLK) + 1;		/* 1us */

	u8 twtp		= tcwl + 4 + twr + 1;
	/*
	 * The code below for twr2rd and trd2wr follows the IP core's
	 * document from ZynqMP and i.MX7. The BSP has both number
	 * substracted by 2.
	 */
	u8 twr2rd	= tcwl + 4 + 1 + twtr;
	u8 trd2wr	= tcl + 4 + (tcksrea >> 1) - tcwl + 1;

	/* set mode registers */
	for (i = 0; i < ARRAY_SIZE(mr_lpddr3); i++)
		writel(mr_lpddr3[i], &mctl_phy->mr[i]);

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

	/* Configure PHY timing */
	writel(tdinit0 | (tdinit1 << 20), &mctl_phy->ptr[3]);
	writel(tdinit2 | (tdinit3 << 18), &mctl_phy->ptr[4]);

	/* set refresh timing */
	writel((trefi << 16) | trfc, &mctl_ctl->rfshtmg);
}
