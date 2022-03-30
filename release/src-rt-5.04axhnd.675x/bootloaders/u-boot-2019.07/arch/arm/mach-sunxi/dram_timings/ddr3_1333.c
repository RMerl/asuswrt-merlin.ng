#include <common.h>
#include <asm/arch/dram.h>
#include <asm/arch/cpu.h>

void mctl_set_timing_params(uint16_t socid, struct dram_para *para)
{
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;

	u8 tccd		= 2;
	u8 tfaw		= ns_to_t(50);
	u8 trrd		= max(ns_to_t(10), 4);
	u8 trcd		= ns_to_t(15);
	u8 trc		= ns_to_t(53);
	u8 txp		= max(ns_to_t(8), 3);
	u8 twtr		= max(ns_to_t(8), 4);
	u8 trtp		= max(ns_to_t(8), 4);
	u8 twr		= max(ns_to_t(15), 3);
	u8 trp		= ns_to_t(15);
	u8 tras		= ns_to_t(38);
	u16 trefi	= ns_to_t(7800) / 32;
	u16 trfc	= ns_to_t(350);

	u8 tmrw		= 0;
	u8 tmrd		= 4;
	u8 tmod		= 12;
	u8 tcke		= 3;
	u8 tcksrx	= 5;
	u8 tcksre	= 5;
	u8 tckesr	= 4;
	u8 trasmax	= 24;

	u8 tcl		= 6; /* CL 12 */
	u8 tcwl		= 4; /* CWL 8 */
	u8 t_rdata_en	= 4;
	u8 wr_latency	= 2;

	u32 tdinit0	= (500 * CONFIG_DRAM_CLK) + 1;		/* 500us */
	u32 tdinit1	= (360 * CONFIG_DRAM_CLK) / 1000 + 1;	/* 360ns */
	u32 tdinit2	= (200 * CONFIG_DRAM_CLK) + 1;		/* 200us */
	u32 tdinit3	= (1 * CONFIG_DRAM_CLK) + 1;		/* 1us */

	u8 twtp		= tcwl + 2 + twr;	/* WL + BL / 2 + tWR */
	u8 twr2rd	= tcwl + 2 + twtr;	/* WL + BL / 2 + tWTR */
	u8 trd2wr	= tcl + 2 + 1 - tcwl;	/* RL + BL / 2 + 2 - WL */

	/* set mode register */
	writel(0x1c70, &mctl_ctl->mr[0]);	/* CL=11, WR=12 */
	writel(0x40, &mctl_ctl->mr[1]);
	writel(0x18, &mctl_ctl->mr[2]);		/* CWL=8 */
	writel(0x0, &mctl_ctl->mr[3]);

	if (socid == SOCID_R40)
		writel(0x3, &mctl_ctl->lp3mr11);	/* odt_en[7:4] */

	/* set DRAM timing */
	writel(DRAMTMG0_TWTP(twtp) | DRAMTMG0_TFAW(tfaw) |
	       DRAMTMG0_TRAS_MAX(trasmax) | DRAMTMG0_TRAS(tras),
	       &mctl_ctl->dramtmg[0]);
	writel(DRAMTMG1_TXP(txp) | DRAMTMG1_TRTP(trtp) | DRAMTMG1_TRC(trc),
	       &mctl_ctl->dramtmg[1]);
	writel(DRAMTMG2_TCWL(tcwl) | DRAMTMG2_TCL(tcl) |
	       DRAMTMG2_TRD2WR(trd2wr) | DRAMTMG2_TWR2RD(twr2rd),
	       &mctl_ctl->dramtmg[2]);
	writel(DRAMTMG3_TMRW(tmrw) | DRAMTMG3_TMRD(tmrd) | DRAMTMG3_TMOD(tmod),
	       &mctl_ctl->dramtmg[3]);
	writel(DRAMTMG4_TRCD(trcd) | DRAMTMG4_TCCD(tccd) | DRAMTMG4_TRRD(trrd) |
	       DRAMTMG4_TRP(trp), &mctl_ctl->dramtmg[4]);
	writel(DRAMTMG5_TCKSRX(tcksrx) | DRAMTMG5_TCKSRE(tcksre) |
	       DRAMTMG5_TCKESR(tckesr) | DRAMTMG5_TCKE(tcke),
	       &mctl_ctl->dramtmg[5]);

	/* set two rank timing */
	clrsetbits_le32(&mctl_ctl->dramtmg[8], (0xff << 8) | (0xff << 0),
			((socid == SOCID_H5 ? 0x33 : 0x66) << 8) | (0x10 << 0));

	/* set PHY interface timing, write latency and read latency configure */
	writel((0x2 << 24) | (t_rdata_en << 16) | (0x1 << 8) |
	       (wr_latency << 0), &mctl_ctl->pitmg[0]);

	/* set PHY timing, PTR0-2 use default */
	writel(PTR3_TDINIT0(tdinit0) | PTR3_TDINIT1(tdinit1), &mctl_ctl->ptr[3]);
	writel(PTR4_TDINIT2(tdinit2) | PTR4_TDINIT3(tdinit3), &mctl_ctl->ptr[4]);

	/* set refresh timing */
	writel(RFSHTMG_TREFI(trefi) | RFSHTMG_TRFC(trfc), &mctl_ctl->rfshtmg);
}
