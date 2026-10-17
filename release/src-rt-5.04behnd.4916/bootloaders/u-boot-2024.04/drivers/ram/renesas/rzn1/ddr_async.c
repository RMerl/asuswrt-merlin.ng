// SPDX-License-Identifier: BSD-2-Clause
/*
 * RZ/N1 DDR Controller initialisation
 *
 * The DDR Controller register values for a specific DDR device, mode and
 * frequency are generated using a Cadence tool.
 *
 * Copyright (C) 2015 Renesas Electronics Europe Ltd
 */
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <ram.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <wait_bit.h>
#include <renesas/ddr_ctrl.h>

void clk_rzn1_reset_state(struct clk *clk, int on);

DECLARE_GLOBAL_DATA_PTR;

struct cadence_ddr_info {
	struct udevice *dev;
	void __iomem *ddrc;
	void __iomem *phy;
	struct clk clk_ddrc;
	struct clk hclk_ddrc;
	struct regmap *syscon;
	bool enable_ecc;
	bool enable_8bit;
	u32 ddr_size;

	/* These two used only during .probe */
	u32 *reg0;
	u32 *reg350;
};

static inline u32 cadence_readl(void __iomem *addr, unsigned int offset)
{
	return readl(addr + offset);
}

static inline void cadence_writel(void __iomem *addr, unsigned int offset,
				  u32 data)
{
	debug("%s: addr = 0x%p, value = 0x%08x\n", __func__, addr + offset, data);
	writel(data, addr + offset);
}

#define ddrc_readl(off)		cadence_readl(priv->ddrc, off)
#define ddrc_writel(val, off)	cadence_writel(priv->ddrc, off, val)

#define phy_readl(off)		cadence_readl(priv->phy, off)
#define phy_writel(val, off)	cadence_writel(priv->phy, off, val)

#define RZN1_DDR3_SINGLE_BANK 3
#define RZN1_DDR3_DUAL_BANK 32

#define FUNCCTRL	0x00
#define  FUNCCTRL_MASKSDLOFS	(0x18 << 16)
#define  FUNCCTRL_DVDDQ_1_5V	BIT(8)
#define  FUNCCTRL_RESET_N	BIT(0)
#define DLLCTRL		0x04
#define  DLLCTRL_ASDLLOCK	BIT(26)
#define  DLLCTRL_MFSL_500MHz	(2 << 1)
#define  DLLCTRL_MDLLSTBY	BIT(0)
#define ZQCALCTRL	0x08
#define  ZQCALCTRL_ZQCALEND	BIT(30)
#define  ZQCALCTRL_ZQCALRSTB	BIT(0)
#define ZQODTCTRL	0x0c
#define RDCTRL		0x10
#define RDTMG		0x14
#define FIFOINIT	0x18
#define  FIFOINIT_RDPTINITEXE	BIT(8)
#define  FIFOINIT_WRPTINITEXE	BIT(0)
#define OUTCTRL		0x1c
#define  OUTCTRL_ADCMDOE	BIT(0)
#define WLCTRL1		0x40
#define  WLCTRL1_WLSTR		BIT(24)
#define DQCALOFS1	0xe8

/* DDR PHY setup */
static void ddr_phy_init(struct cadence_ddr_info *priv, int ddr_type)
{
	u32 val;

	/* Disable DDR Controller clock and FlexWAY connection */
	clk_disable(&priv->hclk_ddrc);
	clk_disable(&priv->clk_ddrc);

	clk_rzn1_reset_state(&priv->hclk_ddrc, 0);
	clk_rzn1_reset_state(&priv->clk_ddrc, 0);

	/* Enable DDR Controller clock and FlexWAY connection */
	clk_enable(&priv->clk_ddrc);
	clk_enable(&priv->hclk_ddrc);

	/* DDR PHY Soft reset assert */
	ddrc_writel(FUNCCTRL_MASKSDLOFS | FUNCCTRL_DVDDQ_1_5V, FUNCCTRL);

	clk_rzn1_reset_state(&priv->hclk_ddrc, 1);
	clk_rzn1_reset_state(&priv->clk_ddrc, 1);

	/* DDR PHY setup */
	phy_writel(DLLCTRL_MFSL_500MHz | DLLCTRL_MDLLSTBY, DLLCTRL);
	phy_writel(0x00000182, ZQCALCTRL);
	if (ddr_type == RZN1_DDR3_DUAL_BANK)
		phy_writel(0xAB330031, ZQODTCTRL);
	else if (ddr_type == RZN1_DDR3_SINGLE_BANK)
		phy_writel(0xAB320051, ZQODTCTRL);
	else /* DDR2 */
		phy_writel(0xAB330071, ZQODTCTRL);
	phy_writel(0xB545B544, RDCTRL);
	phy_writel(0x000000B0, RDTMG);
	phy_writel(0x020A0806, OUTCTRL);
	if (ddr_type == RZN1_DDR3_DUAL_BANK)
		phy_writel(0x80005556, WLCTRL1);
	else
		phy_writel(0x80005C5D, WLCTRL1);
	phy_writel(0x00000101, FIFOINIT);
	phy_writel(0x00004545, DQCALOFS1);

	/* Step 9 MDLL reset release */
	val = phy_readl(DLLCTRL);
	val &= ~DLLCTRL_MDLLSTBY;
	phy_writel(val, DLLCTRL);

	/* Step 12 Soft reset release */
	val = phy_readl(FUNCCTRL);
	val |= FUNCCTRL_RESET_N;
	phy_writel(val, FUNCCTRL);

	/* Step 13 FIFO pointer initialize */
	phy_writel(FIFOINIT_RDPTINITEXE | FIFOINIT_WRPTINITEXE, FIFOINIT);

	/* Step 14 Execute ZQ Calibration */
	val = phy_readl(ZQCALCTRL);
	val |= ZQCALCTRL_ZQCALRSTB;
	phy_writel(val, ZQCALCTRL);

	/* Step 15 Wait for 200us or more, or wait for DFIINITCOMPLETE to be "1" */
	wait_for_bit_le32(priv->phy + DLLCTRL, DLLCTRL_ASDLLOCK, true, 1, false);
	wait_for_bit_le32(priv->phy + ZQCALCTRL, ZQCALCTRL_ZQCALEND, true, 1, false);

	/* Step 16 Enable Address and Command output */
	val = phy_readl(OUTCTRL);
	val |= OUTCTRL_ADCMDOE;
	phy_writel(val, OUTCTRL);

	/* Step 17 Wait for 200us or more(from MRESETB=0) */
	udelay(200);
}

static void ddr_phy_enable_wl(struct cadence_ddr_info *priv)
{
	u32 val;

	/* Step 26 (Set Write Leveling) */
	val = phy_readl(WLCTRL1);
	val |= WLCTRL1_WLSTR;
	phy_writel(val, WLCTRL1);
}

#define RZN1_V_DDR_BASE               0x80000000      /* RZ/N1D only */

static void rzn1_ddr3_single_bank(void *ddr_ctrl_base)
{
	/* CS0 */
	cdns_ddr_set_mr1(ddr_ctrl_base, 0,
			 MR1_ODT_IMPEDANCE_60_OHMS,
			 MR1_DRIVE_STRENGTH_40_OHMS);
	cdns_ddr_set_mr2(ddr_ctrl_base, 0,
			 MR2_DYNAMIC_ODT_OFF,
			 MR2_SELF_REFRESH_TEMP_EXT);

	/* ODT_WR_MAP_CS0 = 1, ODT_RD_MAP_CS0 = 0 */
	cdns_ddr_set_odt_map(ddr_ctrl_base, 0, 0x0100);
}

static int rzn1_dram_init(struct cadence_ddr_info *priv)
{
	u32 version;
	u32 ddr_start_addr = 0;

	ddr_phy_init(priv, RZN1_DDR3_SINGLE_BANK);

	/*
	 * Override DDR PHY Interface (DFI) related settings
	 * DFI is the internal interface between the DDR controller and the DDR PHY.
	 * These settings are specific to the board and can't be known by the settings
	 * provided for each DDR model within the generated include.
	 */
	priv->reg350[351 - 350] = 0x001e0000;
	priv->reg350[352 - 350] = 0x1e680000;
	priv->reg350[353 - 350] = 0x02000020;
	priv->reg350[354 - 350] = 0x02000200;
	priv->reg350[355 - 350] = 0x00000c30;
	priv->reg350[356 - 350] = 0x00009808;
	priv->reg350[357 - 350] = 0x020a0706;
	priv->reg350[372 - 350] = 0x01000000;

	/*
	 * On ES1.0 devices, the DDR start address that the DDR Controller sees
	 * is the physical address of the DDR. However, later devices changed it
	 * to be 0 in order to fix an issue with DDR out-of-range detection.
	 */
#define RZN1_SYSCTRL_REG_VERSION 412
	regmap_read(priv->syscon, RZN1_SYSCTRL_REG_VERSION, &version);
	if (version == 0x10)
		ddr_start_addr = RZN1_V_DDR_BASE;

	if (priv->enable_ecc)
		priv->ddr_size = priv->ddr_size / 2;

	/* DDR Controller is always in ASYNC mode */
	cdns_ddr_ctrl_init(priv->ddrc, 1,
			   priv->reg0, priv->reg350,
			   ddr_start_addr, priv->ddr_size,
			   priv->enable_ecc, priv->enable_8bit);

	rzn1_ddr3_single_bank(priv->ddrc);
	cdns_ddr_set_diff_cs_delays(priv->ddrc, 2, 7, 2, 2);
	cdns_ddr_set_same_cs_delays(priv->ddrc, 0, 7, 0, 0);
	cdns_ddr_set_odt_times(priv->ddrc, 5, 6, 6, 0, 4);
	cdns_ddr_ctrl_start(priv->ddrc);

	ddr_phy_enable_wl(priv);

	if (priv->enable_ecc) {
		/*
		 * Any read before a write will trigger an ECC un-correctable error,
		 * causing a data abort. However, this is also true for any read with a
		 * size less than the AXI bus width. So, the only sensible solution is
		 * to write to all of DDR now and take the hit...
		 */
		memset((void *)RZN1_V_DDR_BASE, 0xff, priv->ddr_size);
	}

	return 0;
}

static int cadence_ddr_get_info(struct udevice *udev, struct ram_info *info)
{
	info->base = 0;
	info->size = gd->ram_size;

	return 0;
}

static struct ram_ops cadence_ddr_ops = {
	.get_info = cadence_ddr_get_info,
};

static int cadence_ddr_test(long *base, long maxsize)
{
	volatile long *addr = base;
	long           cnt;

	maxsize /= sizeof(long);

	for (cnt = 1; cnt <= maxsize; cnt <<= 1) {
		addr[cnt - 1] = ~cnt;
	}

	for (cnt = 1; cnt <= maxsize; cnt <<= 1) {
		if (addr[cnt - 1] != ~cnt) {
			return 0;
		}
	}

	return 1;
}

static int cadence_ddr_probe(struct udevice *dev)
{
	struct cadence_ddr_info *priv = dev_get_priv(dev);
	ofnode subnode;
	int ret;

	priv->dev = dev;

	priv->ddrc = dev_remap_addr_name(dev, "ddrc");
	if (!priv->ddrc) {
		dev_err(dev, "No reg property for Cadence DDR CTRL\n");
		return -EINVAL;
	}

	priv->phy = dev_remap_addr_name(dev, "phy");
	if (!priv->phy) {
		dev_err(dev, "No reg property for Cadence DDR PHY\n");
		return -EINVAL;
	}

	ret = clk_get_by_name(dev, "clk_ddrc", &priv->clk_ddrc);
	if (ret) {
		dev_err(dev, "No clock for Cadence DDR\n");
		return ret;
	}

	ret = clk_get_by_name(dev, "hclk_ddrc", &priv->hclk_ddrc);
	if (ret) {
		dev_err(dev, "No HCLK for Cadence DDR\n");
		return ret;
	}

	priv->syscon = syscon_regmap_lookup_by_phandle(dev, "syscon");
	if (IS_ERR(priv->syscon)) {
		dev_err(dev, "No syscon node found\n");
		return PTR_ERR(priv->syscon);
	}

	priv->enable_ecc = dev_read_bool(dev, "enable-ecc");
	priv->enable_8bit = dev_read_bool(dev, "enable-8bit");

	priv->reg0 = malloc(88 * sizeof(u32));
	priv->reg350 = malloc(25 * sizeof(u32));
	if (!priv->reg0 || !priv->reg350)
		panic("malloc failure\n");

	/* There may be multiple DDR configurations to try */
	dev_for_each_subnode(subnode, dev) {
		ret = ofnode_read_u32(subnode, "size", &priv->ddr_size);
		if (ret) {
			dev_err(dev, "No size for Cadence DDR\n");
			continue;
		}

		ret = ofnode_read_u32_array(subnode, "cadence,ctl-000", priv->reg0, 88);
		if (ret) {
			dev_err(dev, "No cadence,ctl-000\n");
			continue;
		}

		ret = ofnode_read_u32_array(subnode, "cadence,ctl-350", priv->reg350, 25);
		if (ret) {
			dev_err(dev, "No cadence,ctl-350\n");
			continue;
		}

		if (rzn1_dram_init(priv))
			continue;

		if (cadence_ddr_test((long *)RZN1_V_DDR_BASE, priv->ddr_size)) {
			gd->ram_base = RZN1_V_DDR_BASE;
			gd->ram_size = priv->ddr_size;
			break;
		}
	}

	if (!priv->ddr_size)
		panic("No valid DDR to start");

	free(priv->reg350);
	free(priv->reg0);

	return 0;
}

static const struct udevice_id cadence_ddr_ids[] = {
	{ .compatible = "cadence,ddr-ctrl" },
	{ }
};

U_BOOT_DRIVER(cadence_ddr) = {
	.name		= "cadence_ddr",
	.id		= UCLASS_RAM,
	.of_match	= cadence_ddr_ids,
	.ops		= &cadence_ddr_ops,
	.probe		= cadence_ddr_probe,
	.priv_auto	= sizeof(struct cadence_ddr_info),
	.flags		= DM_FLAG_PRE_RELOC,
};
