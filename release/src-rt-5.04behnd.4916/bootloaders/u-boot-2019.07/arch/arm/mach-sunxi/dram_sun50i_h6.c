/*
 * sun50i H6 platform dram controller init
 *
 * (C) Copyright 2017      Icenowy Zheng <icenowy@aosc.io>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/dram.h>
#include <asm/arch/cpu.h>
#include <linux/bitops.h>
#include <linux/kconfig.h>

/*
 * The DRAM controller structure on H6 is similar to the ones on A23/A80:
 * they all contains 3 parts, COM, CTL and PHY. (As a note on A33/A83T/H3/A64
 * /H5/R40 CTL and PHY is composed).
 *
 * COM is allwinner-specific. On H6, the address mapping function is moved
 * from COM to CTL (with the standard ADDRMAP registers on DesignWare memory
 * controller).
 *
 * CTL (controller) and PHY is from DesignWare.
 *
 * The CTL part is a bit similar to the one on A23/A80 (because they all
 * originate from DesignWare), but gets more registers added.
 *
 * The PHY part is quite new, not seen in any previous Allwinner SoCs, and
 * not seen on other SoCs in U-Boot. The only SoC that is also known to have
 * similar PHY is ZynqMP.
 */

/*
 * The delay parameters below allow to allegedly specify delay times of some
 * unknown unit for each individual bit trace in each of the four data bytes
 * the 32-bit wide access consists of. Also three control signals can be
 * adjusted individually.
 */
#define NR_OF_BYTE_LANES	(32 / BITS_PER_BYTE)
/* The eight data lines (DQn) plus DM, DQS, DQS/DM/DQ Output Enable and DQSN */
#define WR_LINES_PER_BYTE_LANE	(BITS_PER_BYTE + 4)
/*
 * The eight data lines (DQn) plus DM, DQS, DQS/DM/DQ Output Enable, DQSN,
 * Termination and Power down
 */
#define RD_LINES_PER_BYTE_LANE	(BITS_PER_BYTE + 6)
struct dram_para {
	u32 clk;
	enum sunxi_dram_type type;
	u8 cols;
	u8 rows;
	u8 ranks;
	const u8 dx_read_delays[NR_OF_BYTE_LANES][RD_LINES_PER_BYTE_LANE];
	const u8 dx_write_delays[NR_OF_BYTE_LANES][WR_LINES_PER_BYTE_LANE];
};

static void mctl_sys_init(struct dram_para *para);
static void mctl_com_init(struct dram_para *para);
static void mctl_set_timing_lpddr3(struct dram_para *para);
static void mctl_channel_init(struct dram_para *para);

static void mctl_core_init(struct dram_para *para)
{
	mctl_sys_init(para);
	mctl_com_init(para);
	switch (para->type) {
	case SUNXI_DRAM_TYPE_LPDDR3:
		mctl_set_timing_lpddr3(para);
		break;
	default:
		panic("Unsupported DRAM type!");
	};
	mctl_channel_init(para);
}

static void mctl_phy_pir_init(u32 val)
{
	struct sunxi_mctl_phy_reg * const mctl_phy =
			(struct sunxi_mctl_phy_reg *)SUNXI_DRAM_PHY0_BASE;

	writel(val | BIT(0), &mctl_phy->pir);
	mctl_await_completion(&mctl_phy->pgsr[0], BIT(0), BIT(0));
}

enum {
	MBUS_PORT_CPU           = 0,
	MBUS_PORT_GPU           = 1,
	MBUS_PORT_MAHB          = 2,
	MBUS_PORT_DMA           = 3,
	MBUS_PORT_VE            = 4,
	MBUS_PORT_CE            = 5,
	MBUS_PORT_TSC0          = 6,
	MBUS_PORT_NDFC0         = 8,
	MBUS_PORT_CSI0          = 11,
	MBUS_PORT_DI0           = 14,
	MBUS_PORT_DI1           = 15,
	MBUS_PORT_DE300         = 16,
	MBUS_PORT_IOMMU         = 25,
	MBUS_PORT_VE2           = 26,
	MBUS_PORT_USB3        = 37,
	MBUS_PORT_PCIE          = 38,
	MBUS_PORT_VP9           = 39,
	MBUS_PORT_HDCP2       = 40,
};

enum {
	MBUS_QOS_LOWEST = 0,
	MBUS_QOS_LOW,
	MBUS_QOS_HIGH,
	MBUS_QOS_HIGHEST
};
inline void mbus_configure_port(u8 port,
				bool bwlimit,
				bool priority,
				u8 qos,
				u8 waittime,
				u8 acs,
				u16 bwl0,
				u16 bwl1,
				u16 bwl2)
{
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;

	const u32 cfg0 = ( (bwlimit ? (1 << 0) : 0)
			   | (priority ? (1 << 1) : 0)
			   | ((qos & 0x3) << 2)
			   | ((waittime & 0xf) << 4)
			   | ((acs & 0xff) << 8)
			   | (bwl0 << 16) );
	const u32 cfg1 = ((u32)bwl2 << 16) | (bwl1 & 0xffff);

	debug("MBUS port %d cfg0 %08x cfg1 %08x\n", port, cfg0, cfg1);
	writel(cfg0, &mctl_com->master[port].cfg0);
	writel(cfg1, &mctl_com->master[port].cfg1);
}

#define MBUS_CONF(port, bwlimit, qos, acs, bwl0, bwl1, bwl2)	\
	mbus_configure_port(MBUS_PORT_ ## port, bwlimit, false, \
			    MBUS_QOS_ ## qos, 0, acs, bwl0, bwl1, bwl2)

static void mctl_set_master_priority(void)
{
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;

	/* enable bandwidth limit windows and set windows size 1us */
	writel(399, &mctl_com->tmr);
	writel(BIT(16), &mctl_com->bwcr);

	MBUS_CONF(  CPU,  true, HIGHEST, 0,  256,  128,  100);
	MBUS_CONF(  GPU,  true,    HIGH, 0, 1536, 1400,  256);
	MBUS_CONF( MAHB,  true, HIGHEST, 0,  512,  256,   96);
	MBUS_CONF(  DMA,  true,    HIGH, 0,  256,  100,   80);
	MBUS_CONF(   VE,  true,    HIGH, 2, 8192, 5500, 5000);
	MBUS_CONF(   CE,  true,    HIGH, 2,  100,   64,   32);
	MBUS_CONF( TSC0,  true,    HIGH, 2,  100,   64,   32);
	MBUS_CONF(NDFC0,  true,    HIGH, 0,  256,  128,   64);
	MBUS_CONF( CSI0,  true,    HIGH, 0,  256,  128,  100);
	MBUS_CONF(  DI0,  true,    HIGH, 0, 1024,  256,   64);
	MBUS_CONF(DE300,  true, HIGHEST, 6, 8192, 2800, 2400);
	MBUS_CONF(IOMMU,  true, HIGHEST, 0,  100,   64,   32);
	MBUS_CONF(  VE2,  true,    HIGH, 2, 8192, 5500, 5000);
	MBUS_CONF( USB3,  true,    HIGH, 0,  256,  128,   64);
	MBUS_CONF( PCIE,  true,    HIGH, 2,  100,   64,   32);
	MBUS_CONF(  VP9,  true,    HIGH, 2, 8192, 5500, 5000);
	MBUS_CONF(HDCP2,  true,    HIGH, 2,  100,   64,   32);
}

static u32 mr_lpddr3[12] = {
	0x00000000, 0x00000043, 0x0000001a, 0x00000001,
	0x00000000, 0x00000000, 0x00000048, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000003,
};

/* TODO: flexible timing */
static void mctl_set_timing_lpddr3(struct dram_para *para)
{
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;
	struct sunxi_mctl_phy_reg * const mctl_phy =
			(struct sunxi_mctl_phy_reg *)SUNXI_DRAM_PHY0_BASE;

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

	/* set mode register */
	memcpy(mctl_phy->mr, mr_lpddr3, sizeof(mr_lpddr3));

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

static void mctl_sys_init(struct dram_para *para)
{
	struct sunxi_ccm_reg * const ccm =
			(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;

	/* Put all DRAM-related blocks to reset state */
	clrbits_le32(&ccm->mbus_cfg, MBUS_ENABLE | MBUS_RESET);
	clrbits_le32(&ccm->dram_gate_reset, BIT(0));
	udelay(5);
	writel(0, &ccm->dram_gate_reset);
	clrbits_le32(&ccm->pll5_cfg, CCM_PLL5_CTRL_EN);
	clrbits_le32(&ccm->dram_clk_cfg, DRAM_MOD_RESET);

	udelay(5);

	/* Set PLL5 rate to doubled DRAM clock rate */
	writel(CCM_PLL5_CTRL_EN | CCM_PLL5_LOCK_EN |
	       CCM_PLL5_CTRL_N(para->clk * 2 / 24 - 1), &ccm->pll5_cfg);
	mctl_await_completion(&ccm->pll5_cfg, CCM_PLL5_LOCK, CCM_PLL5_LOCK);

	/* Configure DRAM mod clock */
	writel(DRAM_CLK_SRC_PLL5, &ccm->dram_clk_cfg);
	setbits_le32(&ccm->dram_clk_cfg, DRAM_CLK_UPDATE);
	writel(BIT(RESET_SHIFT), &ccm->dram_gate_reset);
	udelay(5);
	setbits_le32(&ccm->dram_gate_reset, BIT(0));

	/* Disable all channels */
	writel(0, &mctl_com->maer0);
	writel(0, &mctl_com->maer1);
	writel(0, &mctl_com->maer2);

	/* Configure MBUS and enable DRAM mod reset */
	setbits_le32(&ccm->mbus_cfg, MBUS_RESET);
	setbits_le32(&ccm->mbus_cfg, MBUS_ENABLE);
	setbits_le32(&ccm->dram_clk_cfg, DRAM_MOD_RESET);
	udelay(5);

	/* Unknown hack from the BSP, which enables access of mctl_ctl regs */
	writel(0x8000, &mctl_ctl->unk_0x00c);
}

static void mctl_set_addrmap(struct dram_para *para)
{
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;
	u8 cols = para->cols;
	u8 rows = para->rows;
	u8 ranks = para->ranks;

	/* Ranks */
	if (ranks == 2)
		mctl_ctl->addrmap[0] = rows + cols - 3;
	else
		mctl_ctl->addrmap[0] = 0x1F;

	/* Banks, hardcoded to 8 banks now */
	mctl_ctl->addrmap[1] = (cols - 2) | (cols - 2) << 8 | (cols - 2) << 16;

	/* Columns */
	mctl_ctl->addrmap[2] = 0;
	switch (cols) {
	case 8:
		mctl_ctl->addrmap[3] = 0x1F1F0000;
		mctl_ctl->addrmap[4] = 0x1F1F;
		break;
	case 9:
		mctl_ctl->addrmap[3] = 0x1F000000;
		mctl_ctl->addrmap[4] = 0x1F1F;
		break;
	case 10:
		mctl_ctl->addrmap[3] = 0;
		mctl_ctl->addrmap[4] = 0x1F1F;
		break;
	case 11:
		mctl_ctl->addrmap[3] = 0;
		mctl_ctl->addrmap[4] = 0x1F00;
		break;
	case 12:
		mctl_ctl->addrmap[3] = 0;
		mctl_ctl->addrmap[4] = 0;
		break;
	default:
		panic("Unsupported DRAM configuration: column number invalid\n");
	}

	/* Rows */
	mctl_ctl->addrmap[5] = (cols - 3) | ((cols - 3) << 8) | ((cols - 3) << 16) | ((cols - 3) << 24);
	switch (rows) {
	case 13:
		mctl_ctl->addrmap[6] = (cols - 3) | 0x0F0F0F00;
		mctl_ctl->addrmap[7] = 0x0F0F;
		break;
	case 14:
		mctl_ctl->addrmap[6] = (cols - 3) | ((cols - 3) << 8) | 0x0F0F0000;
		mctl_ctl->addrmap[7] = 0x0F0F;
		break;
	case 15:
		mctl_ctl->addrmap[6] = (cols - 3) | ((cols - 3) << 8) | ((cols - 3) << 16) | 0x0F000000;
		mctl_ctl->addrmap[7] = 0x0F0F;
		break;
	case 16:
		mctl_ctl->addrmap[6] = (cols - 3) | ((cols - 3) << 8) | ((cols - 3) << 16) | ((cols - 3) << 24);
		mctl_ctl->addrmap[7] = 0x0F0F;
		break;
	case 17:
		mctl_ctl->addrmap[6] = (cols - 3) | ((cols - 3) << 8) | ((cols - 3) << 16) | ((cols - 3) << 24);
		mctl_ctl->addrmap[7] = (cols - 3) | 0x0F00;
		break;
	case 18:
		mctl_ctl->addrmap[6] = (cols - 3) | ((cols - 3) << 8) | ((cols - 3) << 16) | ((cols - 3) << 24);
		mctl_ctl->addrmap[7] = (cols - 3) | ((cols - 3) << 8);
		break;
	default:
		panic("Unsupported DRAM configuration: row number invalid\n");
	}

	/* Bank groups, DDR4 only */
	mctl_ctl->addrmap[8] = 0x3F3F;
}

static void mctl_com_init(struct dram_para *para)
{
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;
	struct sunxi_mctl_phy_reg * const mctl_phy =
			(struct sunxi_mctl_phy_reg *)SUNXI_DRAM_PHY0_BASE;
	u32 reg_val, tmp;

	mctl_set_addrmap(para);

	setbits_le32(&mctl_com->cr, BIT(31));
	/*
	 * This address is magic; it's in SID memory area, but there's no
	 * known definition of it.
	 * On my Pine H64 board it has content 7.
	 */
	if (readl(0x03006100) == 7)
		clrbits_le32(&mctl_com->cr, BIT(27));
	else if (readl(0x03006100) == 3)
		setbits_le32(&mctl_com->cr, BIT(27));

	if (para->clk > 408)
		reg_val = 0xf00;
	else if (para->clk > 246)
		reg_val = 0x1f00;
	else
		reg_val = 0x3f00;
	clrsetbits_le32(&mctl_com->unk_0x008, 0x3f00, reg_val);

	/* TODO: half DQ, non-LPDDR3 types */
	writel(MSTR_DEVICETYPE_LPDDR3 | MSTR_BUSWIDTH_FULL |
	       MSTR_BURST_LENGTH(8) | MSTR_ACTIVE_RANKS(para->ranks) |
	       0x80000000, &mctl_ctl->mstr);
	writel(DCR_LPDDR3 | DCR_DDR8BANK | 0x400, &mctl_phy->dcr);

	if (para->ranks == 2)
		writel(0x0303, &mctl_ctl->odtmap);
	else
		writel(0x0201, &mctl_ctl->odtmap);

	/* TODO: non-LPDDR3 types */
	tmp = para->clk * 7 / 2000;
	reg_val = 0x0400;
	reg_val |= (tmp + 7) << 24;
	reg_val |= (((para->clk < 400) ? 3 : 4) - tmp) << 16;
	writel(reg_val, &mctl_ctl->odtcfg);

	/* TODO: half DQ */
}

static void mctl_bit_delay_set(struct dram_para *para)
{
	struct sunxi_mctl_phy_reg * const mctl_phy =
			(struct sunxi_mctl_phy_reg *)SUNXI_DRAM_PHY0_BASE;
	int i, j;
	u32 val;

	for (i = 0; i < 4; i++) {
		val = readl(&mctl_phy->dx[i].bdlr0);
		for (j = 0; j < 4; j++)
			val += para->dx_write_delays[i][j] << (j * 8);
		writel(val, &mctl_phy->dx[i].bdlr0);

		val = readl(&mctl_phy->dx[i].bdlr1);
		for (j = 0; j < 4; j++)
			val += para->dx_write_delays[i][j + 4] << (j * 8);
		writel(val, &mctl_phy->dx[i].bdlr1);

		val = readl(&mctl_phy->dx[i].bdlr2);
		for (j = 0; j < 4; j++)
			val += para->dx_write_delays[i][j + 8] << (j * 8);
		writel(val, &mctl_phy->dx[i].bdlr2);
	}
	clrbits_le32(&mctl_phy->pgcr[0], BIT(26));

	for (i = 0; i < 4; i++) {
		val = readl(&mctl_phy->dx[i].bdlr3);
		for (j = 0; j < 4; j++)
			val += para->dx_read_delays[i][j] << (j * 8);
		writel(val, &mctl_phy->dx[i].bdlr3);

		val = readl(&mctl_phy->dx[i].bdlr4);
		for (j = 0; j < 4; j++)
			val += para->dx_read_delays[i][j + 4] << (j * 8);
		writel(val, &mctl_phy->dx[i].bdlr4);

		val = readl(&mctl_phy->dx[i].bdlr5);
		for (j = 0; j < 4; j++)
			val += para->dx_read_delays[i][j + 8] << (j * 8);
		writel(val, &mctl_phy->dx[i].bdlr5);

		val = readl(&mctl_phy->dx[i].bdlr6);
		val += (para->dx_read_delays[i][12] << 8) |
		       (para->dx_read_delays[i][13] << 16);
		writel(val, &mctl_phy->dx[i].bdlr6);
	}
	setbits_le32(&mctl_phy->pgcr[0], BIT(26));
	udelay(1);

	for (i = 1; i < 14; i++) {
		val = readl(&mctl_phy->acbdlr[i]);
		val += 0x0a0a0a0a;
		writel(val, &mctl_phy->acbdlr[i]);
	}
}

static void mctl_channel_init(struct dram_para *para)
{
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;
	struct sunxi_mctl_phy_reg * const mctl_phy =
			(struct sunxi_mctl_phy_reg *)SUNXI_DRAM_PHY0_BASE;
	int i;
	u32 val;

	setbits_le32(&mctl_ctl->dfiupd[0], BIT(31) | BIT(30));
	setbits_le32(&mctl_ctl->zqctl[0], BIT(31) | BIT(30));
	writel(0x2f05, &mctl_ctl->sched[0]);
	setbits_le32(&mctl_ctl->rfshctl3, BIT(0));
	setbits_le32(&mctl_ctl->dfimisc, BIT(0));
	setbits_le32(&mctl_ctl->unk_0x00c, BIT(8));
	clrsetbits_le32(&mctl_phy->pgcr[1], 0x180, 0xc0);
	/* TODO: non-LPDDR3 types */
	clrsetbits_le32(&mctl_phy->pgcr[2], GENMASK(17, 0), ns_to_t(7800));
	clrbits_le32(&mctl_phy->pgcr[6], BIT(0));
	clrsetbits_le32(&mctl_phy->dxccr, 0xee0, 0x220);
	/* TODO: VT compensation */
	clrsetbits_le32(&mctl_phy->dsgcr, BIT(0), 0x440060);
	clrbits_le32(&mctl_phy->vtcr[1], BIT(1));

	for (i = 0; i < 4; i++)
		clrsetbits_le32(&mctl_phy->dx[i].gcr[0], 0xe00, 0x800);
	for (i = 0; i < 4; i++)
		clrsetbits_le32(&mctl_phy->dx[i].gcr[2], 0xffff, 0x5555);
	for (i = 0; i < 4; i++)
		clrsetbits_le32(&mctl_phy->dx[i].gcr[3], 0x3030, 0x1010);

	udelay(100);

	if (para->ranks == 2)
		setbits_le32(&mctl_phy->dtcr[1], 0x30000);
	else
		clrsetbits_le32(&mctl_phy->dtcr[1], 0x30000, 0x10000);

	clrbits_le32(&mctl_phy->dtcr[1], BIT(1));
	if (para->ranks == 2) {
		writel(0x00010001, &mctl_phy->rankidr);
		writel(0x20000, &mctl_phy->odtcr);
	} else {
		writel(0x0, &mctl_phy->rankidr);
		writel(0x10000, &mctl_phy->odtcr);
	}

	/* TODO: non-LPDDR3 types */
	clrsetbits_le32(&mctl_phy->dtcr[0], 0xF0000000, 0x10000040);
	if (para->clk <= 792) {
		if (para->clk <= 672) {
			if (para->clk <= 600)
				val = 0x300;
			else
				val = 0x400;
		} else {
			val = 0x500;
		}
	} else {
		val = 0x600;
	}
	/* FIXME: NOT REVIEWED YET */
	clrsetbits_le32(&mctl_phy->zq[0].zqcr, 0x700, val);
	clrsetbits_le32(&mctl_phy->zq[0].zqpr[0], 0xff,
			CONFIG_DRAM_ZQ & 0xff);
	clrbits_le32(&mctl_phy->zq[0].zqor[0], 0xfffff);
	setbits_le32(&mctl_phy->zq[0].zqor[0], (CONFIG_DRAM_ZQ >> 8) & 0xff);
	setbits_le32(&mctl_phy->zq[0].zqor[0], (CONFIG_DRAM_ZQ & 0xf00) - 0x100);
	setbits_le32(&mctl_phy->zq[0].zqor[0], (CONFIG_DRAM_ZQ & 0xff00) << 4);
	clrbits_le32(&mctl_phy->zq[1].zqpr[0], 0xfffff);
	setbits_le32(&mctl_phy->zq[1].zqpr[0], (CONFIG_DRAM_ZQ >> 16) & 0xff);
	setbits_le32(&mctl_phy->zq[1].zqpr[0], ((CONFIG_DRAM_ZQ >> 8) & 0xf00) - 0x100);
	setbits_le32(&mctl_phy->zq[1].zqpr[0], (CONFIG_DRAM_ZQ & 0xff0000) >> 4);
	if (para->type == SUNXI_DRAM_TYPE_LPDDR3) {
		for (i = 1; i < 14; i++)
			writel(0x06060606, &mctl_phy->acbdlr[i]);
	}

	/* TODO: non-LPDDR3 types */
	mctl_phy_pir_init(PIR_ZCAL | PIR_DCAL | PIR_PHYRST | PIR_DRAMINIT |
			  PIR_QSGATE | PIR_RDDSKW | PIR_WRDSKW | PIR_RDEYE |
			  PIR_WREYE);

	/* TODO: non-LPDDR3 types */
	for (i = 0; i < 4; i++)
		writel(0x00000909, &mctl_phy->dx[i].gcr[5]);

	for (i = 0; i < 4; i++) {
		if (IS_ENABLED(CONFIG_DRAM_ODT_EN))
			val = 0x0;
		else
			val = 0xaaaa;
		clrsetbits_le32(&mctl_phy->dx[i].gcr[2], 0xffff, val);

		if (IS_ENABLED(CONFIG_DRAM_ODT_EN))
			val = 0x0;
		else
			val = 0x2020;
		clrsetbits_le32(&mctl_phy->dx[i].gcr[3], 0x3030, val);
	}

	mctl_bit_delay_set(para);
	udelay(1);

	setbits_le32(&mctl_phy->pgcr[6], BIT(0));
	clrbits_le32(&mctl_phy->pgcr[6], 0xfff8);
	for (i = 0; i < 4; i++)
		clrbits_le32(&mctl_phy->dx[i].gcr[3], ~0x3ffff);
	udelay(10);

	if (readl(&mctl_phy->pgsr[0]) & 0x400000)
	{
		/*
		 * Detect single rank.
		 * TODO: also detect half DQ.
		 */
		if ((readl(&mctl_phy->dx[0].rsr[0]) & 0x3) == 2 &&
		    (readl(&mctl_phy->dx[1].rsr[0]) & 0x3) == 2 &&
		    (readl(&mctl_phy->dx[2].rsr[0]) & 0x3) == 2 &&
		    (readl(&mctl_phy->dx[3].rsr[0]) & 0x3) == 2) {
			para->ranks = 1;
			/* Restart DRAM initialization from scratch. */
			mctl_core_init(para);
			return;
		}
		else {
			panic("This DRAM setup is currently not supported.\n");
		}
	}

	if (readl(&mctl_phy->pgsr[0]) & 0xff00000) {
		/* Oops! There's something wrong! */
		debug("PLL = %x\n", readl(0x3001010));
		debug("DRAM PHY PGSR0 = %x\n", readl(&mctl_phy->pgsr[0]));
		for (i = 0; i < 4; i++)
			debug("DRAM PHY DX%dRSR0 = %x\n", i, readl(&mctl_phy->dx[i].rsr[0]));
		panic("Error while initializing DRAM PHY!\n");
	}

	clrsetbits_le32(&mctl_phy->dsgcr, 0xc0, 0x40);
	clrbits_le32(&mctl_phy->pgcr[1], 0x40);
	clrbits_le32(&mctl_ctl->dfimisc, BIT(0));
	writel(1, &mctl_ctl->swctl);
	mctl_await_completion(&mctl_ctl->swstat, 1, 1);
	clrbits_le32(&mctl_ctl->rfshctl3, BIT(0));

	setbits_le32(&mctl_com->unk_0x014, BIT(31));
	writel(0xffffffff, &mctl_com->maer0);
	writel(0x7ff, &mctl_com->maer1);
	writel(0xffff, &mctl_com->maer2);
}

static void mctl_auto_detect_dram_size(struct dram_para *para)
{
	/* TODO: non-LPDDR3, half DQ */
	/*
	 * Detect rank number by the code in mctl_channel_init. Furtherly
	 * when DQ detection is available it will also be executed there.
	 */
	mctl_core_init(para);

	/* detect row address bits */
	para->cols = 8;
	para->rows = 18;
	mctl_core_init(para);

	for (para->rows = 13; para->rows < 18; para->rows++) {
		/* 8 banks, 8 bit per byte and 32 bit width */
		if (mctl_mem_matches((1 << (para->rows + para->cols + 5))))
			break;
	}

	/* detect column address bits */
	para->cols = 11;
	mctl_core_init(para);

	for (para->cols = 8; para->cols < 11; para->cols++) {
		/* 8 bits per byte and 32 bit width */
		if (mctl_mem_matches(1 << (para->cols + 2)))
			break;
	}
}

unsigned long mctl_calc_size(struct dram_para *para)
{
	/* TODO: non-LPDDR3, half DQ */

	/* 8 banks, 32-bit (4 byte) data width */
	return (1ULL << (para->cols + para->rows + 3)) * 4 * para->ranks;
}

#define SUN50I_H6_DX_WRITE_DELAYS				\
	{{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },	\
	 {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },	\
	 {  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,  4,  0 },	\
	 {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }}
#define SUN50I_H6_DX_READ_DELAYS					\
	{{  4,  4,  4,  4,  4,  4,  4,  4,  4,  0,  0,  0,  0,  0 },	\
	 {  4,  4,  4,  4,  4,  4,  4,  4,  4,  0,  0,  0,  0,  0 },	\
	 {  4,  4,  4,  4,  4,  4,  4,  4,  4,  0,  0,  0,  0,  0 },	\
	 {  4,  4,  4,  4,  4,  4,  4,  4,  4,  0,  0,  0,  0,  0 }}

unsigned long sunxi_dram_init(void)
{
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;
	struct dram_para para = {
		.clk = CONFIG_DRAM_CLK,
		.type = SUNXI_DRAM_TYPE_LPDDR3,
		.ranks = 2,
		.cols = 11,
		.rows = 14,
		.dx_read_delays  = SUN50I_H6_DX_READ_DELAYS,
		.dx_write_delays = SUN50I_H6_DX_WRITE_DELAYS,
	};

	unsigned long size;

	/* RES_CAL_CTRL_REG in BSP U-boot*/
	setbits_le32(0x7010310, BIT(8));
	clrbits_le32(0x7010318, 0x3f);

	mctl_auto_detect_dram_size(&para);

	mctl_core_init(&para);

	size = mctl_calc_size(&para);

	clrsetbits_le32(&mctl_com->cr, 0xf0, (size >> (10 + 10 + 4)) & 0xf0);

	mctl_set_master_priority();

	return size;
};
