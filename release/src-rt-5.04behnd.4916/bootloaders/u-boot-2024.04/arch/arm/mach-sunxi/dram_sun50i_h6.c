// SPDX-License-Identifier: GPL-2.0+
/*
 * sun50i H6 platform dram controller init
 *
 * (C) Copyright 2017      Icenowy Zheng <icenowy@aosc.io>
 *
 */
#include <init.h>
#include <log.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/dram.h>
#include <asm/arch/cpu.h>
#include <asm/arch/prcm.h>
#include <linux/bitops.h>
#include <linux/delay.h>

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

static void mctl_sys_init(struct dram_para *para);
static void mctl_com_init(struct dram_para *para);
static bool mctl_channel_init(struct dram_para *para);

static bool mctl_core_init(struct dram_para *para)
{
	mctl_sys_init(para);
	mctl_com_init(para);
	switch (para->type) {
	case SUNXI_DRAM_TYPE_LPDDR3:
	case SUNXI_DRAM_TYPE_DDR3:
		mctl_set_timing_params(para);
		break;
	default:
		panic("Unsupported DRAM type!");
	};
	return mctl_channel_init(para);
}

/* PHY initialisation */
static void mctl_phy_pir_init(u32 val)
{
	struct sunxi_mctl_phy_reg * const mctl_phy =
			(struct sunxi_mctl_phy_reg *)SUNXI_DRAM_PHY0_BASE;

	writel(val, &mctl_phy->pir);
	writel(val | BIT(0), &mctl_phy->pir);	/* Start initialisation. */
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

static void mbus_configure_port(u8 port,
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
	       CCM_PLL5_CTRL_N(para->clk * 2 / 24), &ccm->pll5_cfg);
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

	if (!para->bus_full_width)
		cols -= 1;

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
	case 7:
		mctl_ctl->addrmap[3] = 0x1F1F1F00;
		mctl_ctl->addrmap[4] = 0x1F1F;
		break;
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

	/* The bonding ID seems to be always 7. */
	if (readl(SUNXI_SIDC_BASE + 0x100) == 7)	/* bonding ID */
		clrbits_le32(&mctl_com->cr, BIT(27));
	else if (readl(SUNXI_SIDC_BASE + 0x100) == 3)
		setbits_le32(&mctl_com->cr, BIT(27));

	if (para->clk > 408)
		reg_val = 0xf00;
	else if (para->clk > 246)
		reg_val = 0x1f00;
	else
		reg_val = 0x3f00;
	clrsetbits_le32(&mctl_com->unk_0x008, 0x3f00, reg_val);

	/* TODO: DDR4 */
	reg_val = MSTR_BURST_LENGTH(8) | MSTR_ACTIVE_RANKS(para->ranks);
	if (para->type == SUNXI_DRAM_TYPE_LPDDR3)
		reg_val |= MSTR_DEVICETYPE_LPDDR3;
	if (para->type == SUNXI_DRAM_TYPE_DDR3)
		reg_val |= MSTR_DEVICETYPE_DDR3 | MSTR_2TMODE;
	if (para->bus_full_width)
		reg_val |= MSTR_BUSWIDTH_FULL;
	else
		reg_val |= MSTR_BUSWIDTH_HALF;
	writel(reg_val | BIT(31), &mctl_ctl->mstr);

	if (para->type == SUNXI_DRAM_TYPE_LPDDR3)
		reg_val = DCR_LPDDR3 | DCR_DDR8BANK;
	if (para->type == SUNXI_DRAM_TYPE_DDR3)
		reg_val = DCR_DDR3 | DCR_DDR8BANK | DCR_DDR2T;
	writel(reg_val | 0x400, &mctl_phy->dcr);

	if (para->ranks == 2)
		writel(0x0303, &mctl_ctl->odtmap);
	else
		writel(0x0201, &mctl_ctl->odtmap);

	/* TODO: DDR4 */
	if (para->type == SUNXI_DRAM_TYPE_LPDDR3) {
		tmp = para->clk * 7 / 2000;
		reg_val = 0x0400;
		reg_val |= (tmp + 7) << 24;
		reg_val |= (((para->clk < 400) ? 3 : 4) - tmp) << 16;
	} else if (para->type == SUNXI_DRAM_TYPE_DDR3) {
		reg_val = 0x06000400;	/* TODO?: Use CL - CWL value in [7:0] */
	} else {
		panic("Only (LP)DDR3 supported (type = %d)\n", para->type);
	}
	writel(reg_val, &mctl_ctl->odtcfg);

	if (!para->bus_full_width) {
		writel(0x0, &mctl_phy->dx[2].gcr[0]);
		writel(0x0, &mctl_phy->dx[3].gcr[0]);
	}
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

	if (para->type != SUNXI_DRAM_TYPE_LPDDR3)
		return;

	for (i = 1; i < 14; i++) {
		val = readl(&mctl_phy->acbdlr[i]);
		val += 0x0a0a0a0a;
		writel(val, &mctl_phy->acbdlr[i]);
	}
}

static bool mctl_channel_init(struct dram_para *para)
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

	if (sunxi_dram_is_lpddr(para->type))
		clrbits_le32(&mctl_phy->dtcr[1], BIT(1));
	if (para->ranks == 2) {
		writel(0x00010001, &mctl_phy->rankidr);
		writel(0x20000, &mctl_phy->odtcr);
	} else {
		writel(0x0, &mctl_phy->rankidr);
		writel(0x10000, &mctl_phy->odtcr);
	}

	/* set bits [3:0] to 1? 0 not valid in ZynqMP d/s */
	if (para->type == SUNXI_DRAM_TYPE_LPDDR3)
		clrsetbits_le32(&mctl_phy->dtcr[0], 0xF0000000, 0x10000040);
	else
		clrsetbits_le32(&mctl_phy->dtcr[0], 0xF0000000, 0x10000000);
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

	val = PIR_ZCAL | PIR_DCAL | PIR_PHYRST | PIR_DRAMINIT | PIR_QSGATE |
	      PIR_RDDSKW | PIR_WRDSKW | PIR_RDEYE | PIR_WREYE;
	if (para->type == SUNXI_DRAM_TYPE_DDR3)
		val |= PIR_DRAMRST | PIR_WL;
	mctl_phy_pir_init(val);

	/* TODO: DDR4 types ? */
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

	if (readl(&mctl_phy->pgsr[0]) & 0xff00000) {
		/* Oops! There's something wrong! */
		debug("PLL = %x\n", readl(0x3001010));
		debug("DRAM PHY PGSR0 = %x\n", readl(&mctl_phy->pgsr[0]));
		for (i = 0; i < 4; i++)
			debug("DRAM PHY DX%dRSR0 = %x\n", i, readl(&mctl_phy->dx[i].rsr[0]));
		debug("Error while initializing DRAM PHY!\n");

		return false;
	}

	if (sunxi_dram_is_lpddr(para->type))
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

	return true;
}

static void mctl_auto_detect_rank_width(struct dram_para *para)
{
	/* this is minimum size that it's supported */
	para->cols = 8;
	para->rows = 13;

	/*
	 * Previous versions of this driver tried to auto detect the rank
	 * and width by looking at controller registers. However this proved
	 * to be not reliable, so this approach here is the more robust
	 * solution. Check the git history for details.
	 *
	 * Strategy here is to test most demanding combination first and least
	 * demanding last, otherwise HW might not be fully utilized. For
	 * example, half bus width and rank = 1 combination would also work
	 * on HW with full bus width and rank = 2, but only 1/4 RAM would be
	 * visible.
	 */

	debug("testing 32-bit width, rank = 2\n");
	para->bus_full_width = 1;
	para->ranks = 2;
	if (mctl_core_init(para))
		return;

	debug("testing 32-bit width, rank = 1\n");
	para->bus_full_width = 1;
	para->ranks = 1;
	if (mctl_core_init(para))
		return;

	debug("testing 16-bit width, rank = 2\n");
	para->bus_full_width = 0;
	para->ranks = 2;
	if (mctl_core_init(para))
		return;

	debug("testing 16-bit width, rank = 1\n");
	para->bus_full_width = 0;
	para->ranks = 1;
	if (mctl_core_init(para))
		return;

	panic("This DRAM setup is currently not supported.\n");
}

static void mctl_auto_detect_dram_size(struct dram_para *para)
{
	/* TODO: non-(LP)DDR3 */

	/* detect row address bits */
	para->cols = 8;
	para->rows = 18;
	mctl_core_init(para);

	for (para->rows = 13; para->rows < 18; para->rows++) {
		/* 8 banks, 8 bit per byte and 16/32 bit width */
		if (mctl_mem_matches((1 << (para->rows + para->cols +
					    4 + para->bus_full_width))))
			break;
	}

	/* detect column address bits */
	para->cols = 11;
	mctl_core_init(para);

	for (para->cols = 8; para->cols < 11; para->cols++) {
		/* 8 bits per byte and 16/32 bit width */
		if (mctl_mem_matches(1 << (para->cols + 1 +
					   para->bus_full_width)))
			break;
	}
}

unsigned long mctl_calc_size(struct dram_para *para)
{
	u8 width = para->bus_full_width ? 4 : 2;

	/* TODO: non-(LP)DDR3 */

	/* 8 banks */
	return (1ULL << (para->cols + para->rows + 3)) * width * para->ranks;
}

#define SUN50I_H6_LPDDR3_DX_WRITE_DELAYS			\
	{{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },	\
	 {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },	\
	 {  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,  4,  0 },	\
	 {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }}
#define SUN50I_H6_LPDDR3_DX_READ_DELAYS					\
	{{  4,  4,  4,  4,  4,  4,  4,  4,  4,  0,  0,  0,  0,  0 },	\
	 {  4,  4,  4,  4,  4,  4,  4,  4,  4,  0,  0,  0,  0,  0 },	\
	 {  4,  4,  4,  4,  4,  4,  4,  4,  4,  0,  0,  0,  0,  0 },	\
	 {  4,  4,  4,  4,  4,  4,  4,  4,  4,  0,  0,  0,  0,  0 }}

#define SUN50I_H6_DDR3_DX_WRITE_DELAYS				\
	{{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },	\
	 {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },	\
	 {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },	\
	 {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }}
#define SUN50I_H6_DDR3_DX_READ_DELAYS					\
	{{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },	\
	 {  4,  4,  4,  4,  4,  4,  4,  4,  4,  0,  0,  0,  0,  0 },	\
	 {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },	\
	 {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }}

unsigned long sunxi_dram_init(void)
{
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;
	struct sunxi_prcm_reg *const prcm =
		(struct sunxi_prcm_reg *)SUNXI_PRCM_BASE;
	struct dram_para para = {
		.clk = CONFIG_DRAM_CLK,
#ifdef CONFIG_SUNXI_DRAM_H6_LPDDR3
		.type = SUNXI_DRAM_TYPE_LPDDR3,
		.dx_read_delays  = SUN50I_H6_LPDDR3_DX_READ_DELAYS,
		.dx_write_delays = SUN50I_H6_LPDDR3_DX_WRITE_DELAYS,
#elif defined(CONFIG_SUNXI_DRAM_H6_DDR3_1333)
		.type = SUNXI_DRAM_TYPE_DDR3,
		.dx_read_delays  = SUN50I_H6_DDR3_DX_READ_DELAYS,
		.dx_write_delays = SUN50I_H6_DDR3_DX_WRITE_DELAYS,
#endif
	};

	unsigned long size;

	setbits_le32(&prcm->res_cal_ctrl, BIT(8));
	clrbits_le32(&prcm->ohms240, 0x3f);

	mctl_auto_detect_rank_width(&para);
	mctl_auto_detect_dram_size(&para);

	mctl_core_init(&para);

	size = mctl_calc_size(&para);

	clrsetbits_le32(&mctl_com->cr, 0xf0, (size >> (10 + 10 + 4)) & 0xf0);

	mctl_set_master_priority();

	return size;
};
