// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 MediaTek Inc.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 * Author: Mark Lee <mark-mc.lee@mediatek.com>
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <miiphy.h>
#include <net.h>
#include <regmap.h>
#include <reset.h>
#include <syscon.h>
#include <wait_bit.h>
#include <asm/cache.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/ioport.h>
#include <linux/mdio.h>
#include <linux/mii.h>
#include <linux/printk.h>

#include "mtk_eth.h"

#define NUM_TX_DESC		24
#define NUM_RX_DESC		24
#define TX_TOTAL_BUF_SIZE	(NUM_TX_DESC * PKTSIZE_ALIGN)
#define RX_TOTAL_BUF_SIZE	(NUM_RX_DESC * PKTSIZE_ALIGN)
#define TOTAL_PKT_BUF_SIZE	(TX_TOTAL_BUF_SIZE + RX_TOTAL_BUF_SIZE)

#define MT753X_NUM_PHYS		5
#define MT753X_NUM_PORTS	7
#define MT753X_DFL_SMI_ADDR	31
#define MT753X_SMI_ADDR_MASK	0x1f

#define MT753X_PHY_ADDR(base, addr) \
	(((base) + (addr)) & 0x1f)

#define GDMA_FWD_TO_CPU \
	(0x20000000 | \
	GDM_ICS_EN | \
	GDM_TCS_EN | \
	GDM_UCS_EN | \
	STRP_CRC | \
	(DP_PDMA << MYMAC_DP_S) | \
	(DP_PDMA << BC_DP_S) | \
	(DP_PDMA << MC_DP_S) | \
	(DP_PDMA << UN_DP_S))

#define GDMA_BRIDGE_TO_CPU \
	(0xC0000000 | \
	GDM_ICS_EN | \
	GDM_TCS_EN | \
	GDM_UCS_EN | \
	(DP_PDMA << MYMAC_DP_S) | \
	(DP_PDMA << BC_DP_S) | \
	(DP_PDMA << MC_DP_S) | \
	(DP_PDMA << UN_DP_S))

#define GDMA_FWD_DISCARD \
	(0x20000000 | \
	GDM_ICS_EN | \
	GDM_TCS_EN | \
	GDM_UCS_EN | \
	STRP_CRC | \
	(DP_DISCARD << MYMAC_DP_S) | \
	(DP_DISCARD << BC_DP_S) | \
	(DP_DISCARD << MC_DP_S) | \
	(DP_DISCARD << UN_DP_S))

enum mtk_switch {
	SW_NONE,
	SW_MT7530,
	SW_MT7531,
	SW_MT7988,
};

/* struct mtk_soc_data -	This is the structure holding all differences
 *				among various plaforms
 * @caps			Flags shown the extra capability for the SoC
 * @ana_rgc3:			The offset for register ANA_RGC3 related to
 *				sgmiisys syscon
 * @gdma_count:			Number of GDMAs
 * @pdma_base:			Register base of PDMA block
 * @txd_size:			Tx DMA descriptor size.
 * @rxd_size:			Rx DMA descriptor size.
 */
struct mtk_soc_data {
	u32 caps;
	u32 ana_rgc3;
	u32 gdma_count;
	u32 pdma_base;
	u32 txd_size;
	u32 rxd_size;
};

struct mtk_eth_priv {
	char pkt_pool[TOTAL_PKT_BUF_SIZE] __aligned(ARCH_DMA_MINALIGN);

	void *tx_ring_noc;
	void *rx_ring_noc;

	int rx_dma_owner_idx0;
	int tx_cpu_owner_idx0;

	void __iomem *fe_base;
	void __iomem *gmac_base;
	void __iomem *sgmii_base;
	void __iomem *gsw_base;

	struct regmap *ethsys_regmap;

	struct regmap *infra_regmap;

	struct regmap *usxgmii_regmap;
	struct regmap *xfi_pextp_regmap;
	struct regmap *xfi_pll_regmap;
	struct regmap *toprgu_regmap;

	struct mii_dev *mdio_bus;
	int (*mii_read)(struct mtk_eth_priv *priv, u8 phy, u8 reg);
	int (*mii_write)(struct mtk_eth_priv *priv, u8 phy, u8 reg, u16 val);
	int (*mmd_read)(struct mtk_eth_priv *priv, u8 addr, u8 devad, u16 reg);
	int (*mmd_write)(struct mtk_eth_priv *priv, u8 addr, u8 devad, u16 reg,
			 u16 val);

	const struct mtk_soc_data *soc;
	int gmac_id;
	int force_mode;
	int speed;
	int duplex;
	bool pn_swap;

	struct phy_device *phydev;
	int phy_interface;
	int phy_addr;

	enum mtk_switch sw;
	int (*switch_init)(struct mtk_eth_priv *priv);
	void (*switch_mac_control)(struct mtk_eth_priv *priv, bool enable);
	u32 mt753x_smi_addr;
	u32 mt753x_phy_base;
	u32 mt753x_pmcr;
	u32 mt753x_reset_wait_time;

	struct gpio_desc rst_gpio;
	int mcm;

	struct reset_ctl rst_fe;
	struct reset_ctl rst_mcm;
};

static void mtk_pdma_write(struct mtk_eth_priv *priv, u32 reg, u32 val)
{
	writel(val, priv->fe_base + priv->soc->pdma_base + reg);
}

static void mtk_pdma_rmw(struct mtk_eth_priv *priv, u32 reg, u32 clr,
			 u32 set)
{
	clrsetbits_le32(priv->fe_base + priv->soc->pdma_base + reg, clr, set);
}

static void mtk_gdma_write(struct mtk_eth_priv *priv, int no, u32 reg,
			   u32 val)
{
	u32 gdma_base;

	if (no == 2)
		gdma_base = GDMA3_BASE;
	else if (no == 1)
		gdma_base = GDMA2_BASE;
	else
		gdma_base = GDMA1_BASE;

	writel(val, priv->fe_base + gdma_base + reg);
}

static void mtk_fe_rmw(struct mtk_eth_priv *priv, u32 reg, u32 clr, u32 set)
{
	clrsetbits_le32(priv->fe_base + reg, clr, set);
}

static u32 mtk_gmac_read(struct mtk_eth_priv *priv, u32 reg)
{
	return readl(priv->gmac_base + reg);
}

static void mtk_gmac_write(struct mtk_eth_priv *priv, u32 reg, u32 val)
{
	writel(val, priv->gmac_base + reg);
}

static void mtk_gmac_rmw(struct mtk_eth_priv *priv, u32 reg, u32 clr, u32 set)
{
	clrsetbits_le32(priv->gmac_base + reg, clr, set);
}

static void mtk_ethsys_rmw(struct mtk_eth_priv *priv, u32 reg, u32 clr,
			   u32 set)
{
	uint val;

	regmap_read(priv->ethsys_regmap, reg, &val);
	val &= ~clr;
	val |= set;
	regmap_write(priv->ethsys_regmap, reg, val);
}

static void mtk_infra_rmw(struct mtk_eth_priv *priv, u32 reg, u32 clr,
			  u32 set)
{
	uint val;

	regmap_read(priv->infra_regmap, reg, &val);
	val &= ~clr;
	val |= set;
	regmap_write(priv->infra_regmap, reg, val);
}

static u32 mtk_gsw_read(struct mtk_eth_priv *priv, u32 reg)
{
	return readl(priv->gsw_base + reg);
}

static void mtk_gsw_write(struct mtk_eth_priv *priv, u32 reg, u32 val)
{
	writel(val, priv->gsw_base + reg);
}

/* Direct MDIO clause 22/45 access via SoC */
static int mtk_mii_rw(struct mtk_eth_priv *priv, u8 phy, u8 reg, u16 data,
		      u32 cmd, u32 st)
{
	int ret;
	u32 val;

	val = (st << MDIO_ST_S) |
	      ((cmd << MDIO_CMD_S) & MDIO_CMD_M) |
	      (((u32)phy << MDIO_PHY_ADDR_S) & MDIO_PHY_ADDR_M) |
	      (((u32)reg << MDIO_REG_ADDR_S) & MDIO_REG_ADDR_M);

	if (cmd == MDIO_CMD_WRITE || cmd == MDIO_CMD_ADDR)
		val |= data & MDIO_RW_DATA_M;

	mtk_gmac_write(priv, GMAC_PIAC_REG, val | PHY_ACS_ST);

	ret = wait_for_bit_le32(priv->gmac_base + GMAC_PIAC_REG,
				PHY_ACS_ST, 0, 5000, 0);
	if (ret) {
		pr_warn("MDIO access timeout\n");
		return ret;
	}

	if (cmd == MDIO_CMD_READ || cmd == MDIO_CMD_READ_C45) {
		val = mtk_gmac_read(priv, GMAC_PIAC_REG);
		return val & MDIO_RW_DATA_M;
	}

	return 0;
}

/* Direct MDIO clause 22 read via SoC */
static int mtk_mii_read(struct mtk_eth_priv *priv, u8 phy, u8 reg)
{
	return mtk_mii_rw(priv, phy, reg, 0, MDIO_CMD_READ, MDIO_ST_C22);
}

/* Direct MDIO clause 22 write via SoC */
static int mtk_mii_write(struct mtk_eth_priv *priv, u8 phy, u8 reg, u16 data)
{
	return mtk_mii_rw(priv, phy, reg, data, MDIO_CMD_WRITE, MDIO_ST_C22);
}

/* Direct MDIO clause 45 read via SoC */
static int mtk_mmd_read(struct mtk_eth_priv *priv, u8 addr, u8 devad, u16 reg)
{
	int ret;

	ret = mtk_mii_rw(priv, addr, devad, reg, MDIO_CMD_ADDR, MDIO_ST_C45);
	if (ret)
		return ret;

	return mtk_mii_rw(priv, addr, devad, 0, MDIO_CMD_READ_C45,
			  MDIO_ST_C45);
}

/* Direct MDIO clause 45 write via SoC */
static int mtk_mmd_write(struct mtk_eth_priv *priv, u8 addr, u8 devad,
			 u16 reg, u16 val)
{
	int ret;

	ret = mtk_mii_rw(priv, addr, devad, reg, MDIO_CMD_ADDR, MDIO_ST_C45);
	if (ret)
		return ret;

	return mtk_mii_rw(priv, addr, devad, val, MDIO_CMD_WRITE,
			  MDIO_ST_C45);
}

/* Indirect MDIO clause 45 read via MII registers */
static int mtk_mmd_ind_read(struct mtk_eth_priv *priv, u8 addr, u8 devad,
			    u16 reg)
{
	int ret;

	ret = priv->mii_write(priv, addr, MII_MMD_ACC_CTL_REG,
			      (MMD_ADDR << MMD_CMD_S) |
			      ((devad << MMD_DEVAD_S) & MMD_DEVAD_M));
	if (ret)
		return ret;

	ret = priv->mii_write(priv, addr, MII_MMD_ADDR_DATA_REG, reg);
	if (ret)
		return ret;

	ret = priv->mii_write(priv, addr, MII_MMD_ACC_CTL_REG,
			      (MMD_DATA << MMD_CMD_S) |
			      ((devad << MMD_DEVAD_S) & MMD_DEVAD_M));
	if (ret)
		return ret;

	return priv->mii_read(priv, addr, MII_MMD_ADDR_DATA_REG);
}

/* Indirect MDIO clause 45 write via MII registers */
static int mtk_mmd_ind_write(struct mtk_eth_priv *priv, u8 addr, u8 devad,
			     u16 reg, u16 val)
{
	int ret;

	ret = priv->mii_write(priv, addr, MII_MMD_ACC_CTL_REG,
			      (MMD_ADDR << MMD_CMD_S) |
			      ((devad << MMD_DEVAD_S) & MMD_DEVAD_M));
	if (ret)
		return ret;

	ret = priv->mii_write(priv, addr, MII_MMD_ADDR_DATA_REG, reg);
	if (ret)
		return ret;

	ret = priv->mii_write(priv, addr, MII_MMD_ACC_CTL_REG,
			      (MMD_DATA << MMD_CMD_S) |
			      ((devad << MMD_DEVAD_S) & MMD_DEVAD_M));
	if (ret)
		return ret;

	return priv->mii_write(priv, addr, MII_MMD_ADDR_DATA_REG, val);
}

/*
 * MT7530 Internal Register Address Bits
 * -------------------------------------------------------------------
 * | 15  14  13  12  11  10   9   8   7   6 | 5   4   3   2 | 1   0  |
 * |----------------------------------------|---------------|--------|
 * |              Page Address              |  Reg Address  | Unused |
 * -------------------------------------------------------------------
 */

static int mt753x_reg_read(struct mtk_eth_priv *priv, u32 reg, u32 *data)
{
	int ret, low_word, high_word;

	if (priv->sw == SW_MT7988) {
		*data = mtk_gsw_read(priv, reg);
		return 0;
	}

	/* Write page address */
	ret = mtk_mii_write(priv, priv->mt753x_smi_addr, 0x1f, reg >> 6);
	if (ret)
		return ret;

	/* Read low word */
	low_word = mtk_mii_read(priv, priv->mt753x_smi_addr, (reg >> 2) & 0xf);
	if (low_word < 0)
		return low_word;

	/* Read high word */
	high_word = mtk_mii_read(priv, priv->mt753x_smi_addr, 0x10);
	if (high_word < 0)
		return high_word;

	if (data)
		*data = ((u32)high_word << 16) | (low_word & 0xffff);

	return 0;
}

static int mt753x_reg_write(struct mtk_eth_priv *priv, u32 reg, u32 data)
{
	int ret;

	if (priv->sw == SW_MT7988) {
		mtk_gsw_write(priv, reg, data);
		return 0;
	}

	/* Write page address */
	ret = mtk_mii_write(priv, priv->mt753x_smi_addr, 0x1f, reg >> 6);
	if (ret)
		return ret;

	/* Write low word */
	ret = mtk_mii_write(priv, priv->mt753x_smi_addr, (reg >> 2) & 0xf,
			    data & 0xffff);
	if (ret)
		return ret;

	/* Write high word */
	return mtk_mii_write(priv, priv->mt753x_smi_addr, 0x10, data >> 16);
}

static void mt753x_reg_rmw(struct mtk_eth_priv *priv, u32 reg, u32 clr,
			   u32 set)
{
	u32 val;

	mt753x_reg_read(priv, reg, &val);
	val &= ~clr;
	val |= set;
	mt753x_reg_write(priv, reg, val);
}

/* Indirect MDIO clause 22/45 access */
static int mt7531_mii_rw(struct mtk_eth_priv *priv, int phy, int reg, u16 data,
			 u32 cmd, u32 st)
{
	ulong timeout;
	u32 val, timeout_ms;
	int ret = 0;

	val = (st << MDIO_ST_S) |
	      ((cmd << MDIO_CMD_S) & MDIO_CMD_M) |
	      ((phy << MDIO_PHY_ADDR_S) & MDIO_PHY_ADDR_M) |
	      ((reg << MDIO_REG_ADDR_S) & MDIO_REG_ADDR_M);

	if (cmd == MDIO_CMD_WRITE || cmd == MDIO_CMD_ADDR)
		val |= data & MDIO_RW_DATA_M;

	mt753x_reg_write(priv, MT7531_PHY_IAC, val | PHY_ACS_ST);

	timeout_ms = 100;
	timeout = get_timer(0);
	while (1) {
		mt753x_reg_read(priv, MT7531_PHY_IAC, &val);

		if ((val & PHY_ACS_ST) == 0)
			break;

		if (get_timer(timeout) > timeout_ms)
			return -ETIMEDOUT;
	}

	if (cmd == MDIO_CMD_READ || cmd == MDIO_CMD_READ_C45) {
		mt753x_reg_read(priv, MT7531_PHY_IAC, &val);
		ret = val & MDIO_RW_DATA_M;
	}

	return ret;
}

static int mt7531_mii_ind_read(struct mtk_eth_priv *priv, u8 phy, u8 reg)
{
	u8 phy_addr;

	if (phy >= MT753X_NUM_PHYS)
		return -EINVAL;

	phy_addr = MT753X_PHY_ADDR(priv->mt753x_phy_base, phy);

	return mt7531_mii_rw(priv, phy_addr, reg, 0, MDIO_CMD_READ,
			     MDIO_ST_C22);
}

static int mt7531_mii_ind_write(struct mtk_eth_priv *priv, u8 phy, u8 reg,
				u16 val)
{
	u8 phy_addr;

	if (phy >= MT753X_NUM_PHYS)
		return -EINVAL;

	phy_addr = MT753X_PHY_ADDR(priv->mt753x_phy_base, phy);

	return mt7531_mii_rw(priv, phy_addr, reg, val, MDIO_CMD_WRITE,
			     MDIO_ST_C22);
}

static int mt7531_mmd_ind_read(struct mtk_eth_priv *priv, u8 addr, u8 devad,
			       u16 reg)
{
	u8 phy_addr;
	int ret;

	if (addr >= MT753X_NUM_PHYS)
		return -EINVAL;

	phy_addr = MT753X_PHY_ADDR(priv->mt753x_phy_base, addr);

	ret = mt7531_mii_rw(priv, phy_addr, devad, reg, MDIO_CMD_ADDR,
			    MDIO_ST_C45);
	if (ret)
		return ret;

	return mt7531_mii_rw(priv, phy_addr, devad, 0, MDIO_CMD_READ_C45,
			     MDIO_ST_C45);
}

static int mt7531_mmd_ind_write(struct mtk_eth_priv *priv, u8 addr, u8 devad,
				u16 reg, u16 val)
{
	u8 phy_addr;
	int ret;

	if (addr >= MT753X_NUM_PHYS)
		return 0;

	phy_addr = MT753X_PHY_ADDR(priv->mt753x_phy_base, addr);

	ret = mt7531_mii_rw(priv, phy_addr, devad, reg, MDIO_CMD_ADDR,
			    MDIO_ST_C45);
	if (ret)
		return ret;

	return mt7531_mii_rw(priv, phy_addr, devad, val, MDIO_CMD_WRITE,
			     MDIO_ST_C45);
}

static int mtk_mdio_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	struct mtk_eth_priv *priv = bus->priv;

	if (devad < 0)
		return priv->mii_read(priv, addr, reg);
	else
		return priv->mmd_read(priv, addr, devad, reg);
}

static int mtk_mdio_write(struct mii_dev *bus, int addr, int devad, int reg,
			  u16 val)
{
	struct mtk_eth_priv *priv = bus->priv;

	if (devad < 0)
		return priv->mii_write(priv, addr, reg, val);
	else
		return priv->mmd_write(priv, addr, devad, reg, val);
}

static int mtk_mdio_register(struct udevice *dev)
{
	struct mtk_eth_priv *priv = dev_get_priv(dev);
	struct mii_dev *mdio_bus = mdio_alloc();
	int ret;

	if (!mdio_bus)
		return -ENOMEM;

	/* Assign MDIO access APIs according to the switch/phy */
	switch (priv->sw) {
	case SW_MT7530:
		priv->mii_read = mtk_mii_read;
		priv->mii_write = mtk_mii_write;
		priv->mmd_read = mtk_mmd_ind_read;
		priv->mmd_write = mtk_mmd_ind_write;
		break;
	case SW_MT7531:
	case SW_MT7988:
		priv->mii_read = mt7531_mii_ind_read;
		priv->mii_write = mt7531_mii_ind_write;
		priv->mmd_read = mt7531_mmd_ind_read;
		priv->mmd_write = mt7531_mmd_ind_write;
		break;
	default:
		priv->mii_read = mtk_mii_read;
		priv->mii_write = mtk_mii_write;
		priv->mmd_read = mtk_mmd_read;
		priv->mmd_write = mtk_mmd_write;
	}

	mdio_bus->read = mtk_mdio_read;
	mdio_bus->write = mtk_mdio_write;
	snprintf(mdio_bus->name, sizeof(mdio_bus->name), dev->name);

	mdio_bus->priv = (void *)priv;

	ret = mdio_register(mdio_bus);

	if (ret)
		return ret;

	priv->mdio_bus = mdio_bus;

	return 0;
}

static int mt753x_core_reg_read(struct mtk_eth_priv *priv, u32 reg)
{
	u8 phy_addr = MT753X_PHY_ADDR(priv->mt753x_phy_base, 0);

	return priv->mmd_read(priv, phy_addr, 0x1f, reg);
}

static void mt753x_core_reg_write(struct mtk_eth_priv *priv, u32 reg, u32 val)
{
	u8 phy_addr = MT753X_PHY_ADDR(priv->mt753x_phy_base, 0);

	priv->mmd_write(priv, phy_addr, 0x1f, reg, val);
}

static int mt7530_pad_clk_setup(struct mtk_eth_priv *priv, int mode)
{
	u32 ncpo1, ssc_delta;

	switch (mode) {
	case PHY_INTERFACE_MODE_RGMII:
		ncpo1 = 0x0c80;
		ssc_delta = 0x87;
		break;
	default:
		printf("error: xMII mode %d not supported\n", mode);
		return -EINVAL;
	}

	/* Disable MT7530 core clock */
	mt753x_core_reg_write(priv, CORE_TRGMII_GSW_CLK_CG, 0);

	/* Disable MT7530 PLL */
	mt753x_core_reg_write(priv, CORE_GSWPLL_GRP1,
			      (2 << RG_GSWPLL_POSDIV_200M_S) |
			      (32 << RG_GSWPLL_FBKDIV_200M_S));

	/* For MT7530 core clock = 500Mhz */
	mt753x_core_reg_write(priv, CORE_GSWPLL_GRP2,
			      (1 << RG_GSWPLL_POSDIV_500M_S) |
			      (25 << RG_GSWPLL_FBKDIV_500M_S));

	/* Enable MT7530 PLL */
	mt753x_core_reg_write(priv, CORE_GSWPLL_GRP1,
			      (2 << RG_GSWPLL_POSDIV_200M_S) |
			      (32 << RG_GSWPLL_FBKDIV_200M_S) |
			      RG_GSWPLL_EN_PRE);

	udelay(20);

	mt753x_core_reg_write(priv, CORE_TRGMII_GSW_CLK_CG, REG_GSWCK_EN);

	/* Setup the MT7530 TRGMII Tx Clock */
	mt753x_core_reg_write(priv, CORE_PLL_GROUP5, ncpo1);
	mt753x_core_reg_write(priv, CORE_PLL_GROUP6, 0);
	mt753x_core_reg_write(priv, CORE_PLL_GROUP10, ssc_delta);
	mt753x_core_reg_write(priv, CORE_PLL_GROUP11, ssc_delta);
	mt753x_core_reg_write(priv, CORE_PLL_GROUP4, RG_SYSPLL_DDSFBK_EN |
			      RG_SYSPLL_BIAS_EN | RG_SYSPLL_BIAS_LPF_EN);

	mt753x_core_reg_write(priv, CORE_PLL_GROUP2,
			      RG_SYSPLL_EN_NORMAL | RG_SYSPLL_VODEN |
			      (1 << RG_SYSPLL_POSDIV_S));

	mt753x_core_reg_write(priv, CORE_PLL_GROUP7,
			      RG_LCDDS_PCW_NCPO_CHG | (3 << RG_LCCDS_C_S) |
			      RG_LCDDS_PWDB | RG_LCDDS_ISO_EN);

	/* Enable MT7530 core clock */
	mt753x_core_reg_write(priv, CORE_TRGMII_GSW_CLK_CG,
			      REG_GSWCK_EN | REG_TRGMIICK_EN);

	return 0;
}

static void mt7530_mac_control(struct mtk_eth_priv *priv, bool enable)
{
	u32 pmcr = FORCE_MODE;

	if (enable)
		pmcr = priv->mt753x_pmcr;

	mt753x_reg_write(priv, PMCR_REG(6), pmcr);
}

static int mt7530_setup(struct mtk_eth_priv *priv)
{
	u16 phy_addr, phy_val;
	u32 val, txdrv;
	int i;

	if (!MTK_HAS_CAPS(priv->soc->caps, MTK_TRGMII_MT7621_CLK)) {
		/* Select 250MHz clk for RGMII mode */
		mtk_ethsys_rmw(priv, ETHSYS_CLKCFG0_REG,
			       ETHSYS_TRGMII_CLK_SEL362_5, 0);

		txdrv = 8;
	} else {
		txdrv = 4;
	}

	/* Modify HWTRAP first to allow direct access to internal PHYs */
	mt753x_reg_read(priv, HWTRAP_REG, &val);
	val |= CHG_TRAP;
	val &= ~C_MDIO_BPS;
	mt753x_reg_write(priv, MHWTRAP_REG, val);

	/* Calculate the phy base address */
	val = ((val & SMI_ADDR_M) >> SMI_ADDR_S) << 3;
	priv->mt753x_phy_base = (val | 0x7) + 1;

	/* Turn off PHYs */
	for (i = 0; i < MT753X_NUM_PHYS; i++) {
		phy_addr = MT753X_PHY_ADDR(priv->mt753x_phy_base, i);
		phy_val = priv->mii_read(priv, phy_addr, MII_BMCR);
		phy_val |= BMCR_PDOWN;
		priv->mii_write(priv, phy_addr, MII_BMCR, phy_val);
	}

	/* Force MAC link down before reset */
	mt753x_reg_write(priv, PMCR_REG(5), FORCE_MODE);
	mt753x_reg_write(priv, PMCR_REG(6), FORCE_MODE);

	/* MT7530 reset */
	mt753x_reg_write(priv, SYS_CTRL_REG, SW_SYS_RST | SW_REG_RST);
	udelay(100);

	val = (IPG_96BIT_WITH_SHORT_IPG << IPG_CFG_S) |
	      MAC_MODE | FORCE_MODE |
	      MAC_TX_EN | MAC_RX_EN |
	      BKOFF_EN | BACKPR_EN |
	      (SPEED_1000M << FORCE_SPD_S) |
	      FORCE_DPX | FORCE_LINK;

	/* MT7530 Port6: Forced 1000M/FD, FC disabled */
	priv->mt753x_pmcr = val;

	/* MT7530 Port5: Forced link down */
	mt753x_reg_write(priv, PMCR_REG(5), FORCE_MODE);

	/* Keep MAC link down before starting eth */
	mt753x_reg_write(priv, PMCR_REG(6), FORCE_MODE);

	/* MT7530 Port6: Set to RGMII */
	mt753x_reg_rmw(priv, MT7530_P6ECR, P6_INTF_MODE_M, P6_INTF_MODE_RGMII);

	/* Hardware Trap: Enable Port6, Disable Port5 */
	mt753x_reg_read(priv, HWTRAP_REG, &val);
	val |= CHG_TRAP | LOOPDET_DIS | P5_INTF_DIS |
	       (P5_INTF_SEL_GMAC5 << P5_INTF_SEL_S) |
	       (P5_INTF_MODE_RGMII << P5_INTF_MODE_S);
	val &= ~(C_MDIO_BPS | P6_INTF_DIS);
	mt753x_reg_write(priv, MHWTRAP_REG, val);

	/* Setup switch core pll */
	mt7530_pad_clk_setup(priv, priv->phy_interface);

	/* Lower Tx Driving for TRGMII path */
	for (i = 0 ; i < NUM_TRGMII_CTRL ; i++)
		mt753x_reg_write(priv, MT7530_TRGMII_TD_ODT(i),
				 (txdrv << TD_DM_DRVP_S) |
				 (txdrv << TD_DM_DRVN_S));

	for (i = 0 ; i < NUM_TRGMII_CTRL; i++)
		mt753x_reg_rmw(priv, MT7530_TRGMII_RD(i), RD_TAP_M, 16);

	/* Turn on PHYs */
	for (i = 0; i < MT753X_NUM_PHYS; i++) {
		phy_addr = MT753X_PHY_ADDR(priv->mt753x_phy_base, i);
		phy_val = priv->mii_read(priv, phy_addr, MII_BMCR);
		phy_val &= ~BMCR_PDOWN;
		priv->mii_write(priv, phy_addr, MII_BMCR, phy_val);
	}

	return 0;
}

static void mt7531_core_pll_setup(struct mtk_eth_priv *priv, int mcm)
{
	/* Step 1 : Disable MT7531 COREPLL */
	mt753x_reg_rmw(priv, MT7531_PLLGP_EN, EN_COREPLL, 0);

	/* Step 2: switch to XTAL output */
	mt753x_reg_rmw(priv, MT7531_PLLGP_EN, SW_CLKSW, SW_CLKSW);

	mt753x_reg_rmw(priv, MT7531_PLLGP_CR0, RG_COREPLL_EN, 0);

	/* Step 3: disable PLLGP and enable program PLLGP */
	mt753x_reg_rmw(priv, MT7531_PLLGP_EN, SW_PLLGP, SW_PLLGP);

	/* Step 4: program COREPLL output frequency to 500MHz */
	mt753x_reg_rmw(priv, MT7531_PLLGP_CR0, RG_COREPLL_POSDIV_M,
		       2 << RG_COREPLL_POSDIV_S);
	udelay(25);

	/* Currently, support XTAL 25Mhz only */
	mt753x_reg_rmw(priv, MT7531_PLLGP_CR0, RG_COREPLL_SDM_PCW_M,
		       0x140000 << RG_COREPLL_SDM_PCW_S);

	/* Set feedback divide ratio update signal to high */
	mt753x_reg_rmw(priv, MT7531_PLLGP_CR0, RG_COREPLL_SDM_PCW_CHG,
		       RG_COREPLL_SDM_PCW_CHG);

	/* Wait for at least 16 XTAL clocks */
	udelay(10);

	/* Step 5: set feedback divide ratio update signal to low */
	mt753x_reg_rmw(priv, MT7531_PLLGP_CR0, RG_COREPLL_SDM_PCW_CHG, 0);

	/* add enable 325M clock for SGMII */
	mt753x_reg_write(priv, MT7531_ANA_PLLGP_CR5, 0xad0000);

	/* add enable 250SSC clock for RGMII */
	mt753x_reg_write(priv, MT7531_ANA_PLLGP_CR2, 0x4f40000);

	/*Step 6: Enable MT7531 PLL */
	mt753x_reg_rmw(priv, MT7531_PLLGP_CR0, RG_COREPLL_EN, RG_COREPLL_EN);

	mt753x_reg_rmw(priv, MT7531_PLLGP_EN, EN_COREPLL, EN_COREPLL);

	udelay(25);
}

static int mt7531_port_sgmii_init(struct mtk_eth_priv *priv,
				  u32 port)
{
	if (port != 5 && port != 6) {
		printf("mt7531: port %d is not a SGMII port\n", port);
		return -EINVAL;
	}

	/* Set SGMII GEN2 speed(2.5G) */
	mt753x_reg_rmw(priv, MT7531_PHYA_CTRL_SIGNAL3(port),
		       SGMSYS_SPEED_2500, SGMSYS_SPEED_2500);

	/* Disable SGMII AN */
	mt753x_reg_rmw(priv, MT7531_PCS_CONTROL_1(port),
		       SGMII_AN_ENABLE, 0);

	/* SGMII force mode setting */
	mt753x_reg_write(priv, MT7531_SGMII_MODE(port), SGMII_FORCE_MODE);

	/* Release PHYA power down state */
	mt753x_reg_rmw(priv, MT7531_QPHY_PWR_STATE_CTRL(port),
		       SGMII_PHYA_PWD, 0);

	return 0;
}

static int mt7531_port_rgmii_init(struct mtk_eth_priv *priv, u32 port)
{
	u32 val;

	if (port != 5) {
		printf("error: RGMII mode is not available for port %d\n",
		       port);
		return -EINVAL;
	}

	mt753x_reg_read(priv, MT7531_CLKGEN_CTRL, &val);
	val |= GP_CLK_EN;
	val &= ~GP_MODE_M;
	val |= GP_MODE_RGMII << GP_MODE_S;
	val |= TXCLK_NO_REVERSE;
	val |= RXCLK_NO_DELAY;
	val &= ~CLK_SKEW_IN_M;
	val |= CLK_SKEW_IN_NO_CHANGE << CLK_SKEW_IN_S;
	val &= ~CLK_SKEW_OUT_M;
	val |= CLK_SKEW_OUT_NO_CHANGE << CLK_SKEW_OUT_S;
	mt753x_reg_write(priv, MT7531_CLKGEN_CTRL, val);

	return 0;
}

static void mt7531_phy_setting(struct mtk_eth_priv *priv)
{
	int i;
	u32 val;

	for (i = 0; i < MT753X_NUM_PHYS; i++) {
		/* Enable HW auto downshift */
		priv->mii_write(priv, i, 0x1f, 0x1);
		val = priv->mii_read(priv, i, PHY_EXT_REG_14);
		val |= PHY_EN_DOWN_SHFIT;
		priv->mii_write(priv, i, PHY_EXT_REG_14, val);

		/* PHY link down power saving enable */
		val = priv->mii_read(priv, i, PHY_EXT_REG_17);
		val |= PHY_LINKDOWN_POWER_SAVING_EN;
		priv->mii_write(priv, i, PHY_EXT_REG_17, val);

		val = priv->mmd_read(priv, i, 0x1e, PHY_DEV1E_REG_0C6);
		val &= ~PHY_POWER_SAVING_M;
		val |= PHY_POWER_SAVING_TX << PHY_POWER_SAVING_S;
		priv->mmd_write(priv, i, 0x1e, PHY_DEV1E_REG_0C6, val);
	}
}

static void mt7531_mac_control(struct mtk_eth_priv *priv, bool enable)
{
	u32 pmcr = FORCE_MODE_LNK;

	if (enable)
		pmcr = priv->mt753x_pmcr;

	mt753x_reg_write(priv, PMCR_REG(5), pmcr);
	mt753x_reg_write(priv, PMCR_REG(6), pmcr);
}

static int mt7531_setup(struct mtk_eth_priv *priv)
{
	u16 phy_addr, phy_val;
	u32 val;
	u32 pmcr;
	u32 port5_sgmii;
	int i;

	priv->mt753x_phy_base = (priv->mt753x_smi_addr + 1) &
				MT753X_SMI_ADDR_MASK;

	/* Turn off PHYs */
	for (i = 0; i < MT753X_NUM_PHYS; i++) {
		phy_addr = MT753X_PHY_ADDR(priv->mt753x_phy_base, i);
		phy_val = priv->mii_read(priv, phy_addr, MII_BMCR);
		phy_val |= BMCR_PDOWN;
		priv->mii_write(priv, phy_addr, MII_BMCR, phy_val);
	}

	/* Force MAC link down before reset */
	mt753x_reg_write(priv, PMCR_REG(5), FORCE_MODE_LNK);
	mt753x_reg_write(priv, PMCR_REG(6), FORCE_MODE_LNK);

	/* Switch soft reset */
	mt753x_reg_write(priv, SYS_CTRL_REG, SW_SYS_RST | SW_REG_RST);
	udelay(100);

	/* Enable MDC input Schmitt Trigger */
	mt753x_reg_rmw(priv, MT7531_SMT0_IOLB, SMT_IOLB_5_SMI_MDC_EN,
		       SMT_IOLB_5_SMI_MDC_EN);

	mt7531_core_pll_setup(priv, priv->mcm);

	mt753x_reg_read(priv, MT7531_TOP_SIG_SR, &val);
	port5_sgmii = !!(val & PAD_DUAL_SGMII_EN);

	/* port5 support either RGMII or SGMII, port6 only support SGMII. */
	switch (priv->phy_interface) {
	case PHY_INTERFACE_MODE_RGMII:
		if (!port5_sgmii)
			mt7531_port_rgmii_init(priv, 5);
		break;
	case PHY_INTERFACE_MODE_2500BASEX:
		mt7531_port_sgmii_init(priv, 6);
		if (port5_sgmii)
			mt7531_port_sgmii_init(priv, 5);
		break;
	default:
		break;
	}

	pmcr = MT7531_FORCE_MODE |
	       (IPG_96BIT_WITH_SHORT_IPG << IPG_CFG_S) |
	       MAC_MODE | MAC_TX_EN | MAC_RX_EN |
	       BKOFF_EN | BACKPR_EN |
	       FORCE_RX_FC | FORCE_TX_FC |
	       (SPEED_1000M << FORCE_SPD_S) | FORCE_DPX |
	       FORCE_LINK;

	priv->mt753x_pmcr = pmcr;

	/* Keep MAC link down before starting eth */
	mt753x_reg_write(priv, PMCR_REG(5), FORCE_MODE_LNK);
	mt753x_reg_write(priv, PMCR_REG(6), FORCE_MODE_LNK);

	/* Turn on PHYs */
	for (i = 0; i < MT753X_NUM_PHYS; i++) {
		phy_addr = MT753X_PHY_ADDR(priv->mt753x_phy_base, i);
		phy_val = priv->mii_read(priv, phy_addr, MII_BMCR);
		phy_val &= ~BMCR_PDOWN;
		priv->mii_write(priv, phy_addr, MII_BMCR, phy_val);
	}

	mt7531_phy_setting(priv);

	/* Enable Internal PHYs */
	val = mt753x_core_reg_read(priv, CORE_PLL_GROUP4);
	val |= MT7531_BYPASS_MODE;
	val &= ~MT7531_POWER_ON_OFF;
	mt753x_core_reg_write(priv, CORE_PLL_GROUP4, val);

	return 0;
}

static void mt7988_phy_setting(struct mtk_eth_priv *priv)
{
	u16 val;
	u32 i;

	for (i = 0; i < MT753X_NUM_PHYS; i++) {
		/* Enable HW auto downshift */
		priv->mii_write(priv, i, 0x1f, 0x1);
		val = priv->mii_read(priv, i, PHY_EXT_REG_14);
		val |= PHY_EN_DOWN_SHFIT;
		priv->mii_write(priv, i, PHY_EXT_REG_14, val);

		/* PHY link down power saving enable */
		val = priv->mii_read(priv, i, PHY_EXT_REG_17);
		val |= PHY_LINKDOWN_POWER_SAVING_EN;
		priv->mii_write(priv, i, PHY_EXT_REG_17, val);
	}
}

static void mt7988_mac_control(struct mtk_eth_priv *priv, bool enable)
{
	u32 pmcr = FORCE_MODE_LNK;

	if (enable)
		pmcr = priv->mt753x_pmcr;

	mt753x_reg_write(priv, PMCR_REG(6), pmcr);
}

static int mt7988_setup(struct mtk_eth_priv *priv)
{
	u16 phy_addr, phy_val;
	u32 pmcr;
	int i;

	priv->gsw_base = regmap_get_range(priv->ethsys_regmap, 0) + GSW_BASE;

	priv->mt753x_phy_base = (priv->mt753x_smi_addr + 1) &
				MT753X_SMI_ADDR_MASK;

	/* Turn off PHYs */
	for (i = 0; i < MT753X_NUM_PHYS; i++) {
		phy_addr = MT753X_PHY_ADDR(priv->mt753x_phy_base, i);
		phy_val = priv->mii_read(priv, phy_addr, MII_BMCR);
		phy_val |= BMCR_PDOWN;
		priv->mii_write(priv, phy_addr, MII_BMCR, phy_val);
	}

	switch (priv->phy_interface) {
	case PHY_INTERFACE_MODE_USXGMII:
		/* Use CPU bridge instead of actual USXGMII path */

		/* Set GDM1 no drop */
		mtk_fe_rmw(priv, PSE_NO_DROP_CFG_REG, 0, PSE_NO_DROP_GDM1);

		/* Enable GDM1 to GSW CPU bridge */
		mtk_gmac_rmw(priv, GMAC_MAC_MISC_REG, 0, BIT(0));

		/* XGMAC force link up */
		mtk_gmac_rmw(priv, GMAC_XGMAC_STS_REG, 0, P1_XGMAC_FORCE_LINK);

		/* Setup GSW CPU bridge IPG */
		mtk_gmac_rmw(priv, GMAC_GSW_CFG_REG, GSWTX_IPG_M | GSWRX_IPG_M,
			     (0xB << GSWTX_IPG_S) | (0xB << GSWRX_IPG_S));
		break;
	default:
		printf("Error: MT7988 GSW does not support %s interface\n",
		       phy_string_for_interface(priv->phy_interface));
		break;
	}

	pmcr = MT7988_FORCE_MODE |
	       (IPG_96BIT_WITH_SHORT_IPG << IPG_CFG_S) |
	       MAC_MODE | MAC_TX_EN | MAC_RX_EN |
	       BKOFF_EN | BACKPR_EN |
	       FORCE_RX_FC | FORCE_TX_FC |
	       (SPEED_1000M << FORCE_SPD_S) | FORCE_DPX |
	       FORCE_LINK;

	priv->mt753x_pmcr = pmcr;

	/* Keep MAC link down before starting eth */
	mt753x_reg_write(priv, PMCR_REG(6), FORCE_MODE_LNK);

	/* Turn on PHYs */
	for (i = 0; i < MT753X_NUM_PHYS; i++) {
		phy_addr = MT753X_PHY_ADDR(priv->mt753x_phy_base, i);
		phy_val = priv->mii_read(priv, phy_addr, MII_BMCR);
		phy_val &= ~BMCR_PDOWN;
		priv->mii_write(priv, phy_addr, MII_BMCR, phy_val);
	}

	mt7988_phy_setting(priv);

	return 0;
}

static int mt753x_switch_init(struct mtk_eth_priv *priv)
{
	int ret;
	int i;

	/* Global reset switch */
	if (priv->mcm) {
		reset_assert(&priv->rst_mcm);
		udelay(1000);
		reset_deassert(&priv->rst_mcm);
		mdelay(priv->mt753x_reset_wait_time);
	} else if (dm_gpio_is_valid(&priv->rst_gpio)) {
		dm_gpio_set_value(&priv->rst_gpio, 0);
		udelay(1000);
		dm_gpio_set_value(&priv->rst_gpio, 1);
		mdelay(priv->mt753x_reset_wait_time);
	}

	ret = priv->switch_init(priv);
	if (ret)
		return ret;

	/* Set port isolation */
	for (i = 0; i < MT753X_NUM_PORTS; i++) {
		/* Set port matrix mode */
		if (i != 6)
			mt753x_reg_write(priv, PCR_REG(i),
					 (0x40 << PORT_MATRIX_S));
		else
			mt753x_reg_write(priv, PCR_REG(i),
					 (0x3f << PORT_MATRIX_S));

		/* Set port mode to user port */
		mt753x_reg_write(priv, PVC_REG(i),
				 (0x8100 << STAG_VPID_S) |
				 (VLAN_ATTR_USER << VLAN_ATTR_S));
	}

	return 0;
}

static void mtk_xphy_link_adjust(struct mtk_eth_priv *priv)
{
	u16 lcl_adv = 0, rmt_adv = 0;
	u8 flowctrl;
	u32 mcr;

	mcr = mtk_gmac_read(priv, XGMAC_PORT_MCR(priv->gmac_id));
	mcr &= ~(XGMAC_FORCE_TX_FC | XGMAC_FORCE_RX_FC);

	if (priv->phydev->duplex) {
		if (priv->phydev->pause)
			rmt_adv = LPA_PAUSE_CAP;
		if (priv->phydev->asym_pause)
			rmt_adv |= LPA_PAUSE_ASYM;

		if (priv->phydev->advertising & ADVERTISED_Pause)
			lcl_adv |= ADVERTISE_PAUSE_CAP;
		if (priv->phydev->advertising & ADVERTISED_Asym_Pause)
			lcl_adv |= ADVERTISE_PAUSE_ASYM;

		flowctrl = mii_resolve_flowctrl_fdx(lcl_adv, rmt_adv);

		if (flowctrl & FLOW_CTRL_TX)
			mcr |= XGMAC_FORCE_TX_FC;
		if (flowctrl & FLOW_CTRL_RX)
			mcr |= XGMAC_FORCE_RX_FC;

		debug("rx pause %s, tx pause %s\n",
		      flowctrl & FLOW_CTRL_RX ? "enabled" : "disabled",
		      flowctrl & FLOW_CTRL_TX ? "enabled" : "disabled");
	}

	mcr &= ~(XGMAC_TRX_DISABLE);
	mtk_gmac_write(priv, XGMAC_PORT_MCR(priv->gmac_id), mcr);
}

static void mtk_phy_link_adjust(struct mtk_eth_priv *priv)
{
	u16 lcl_adv = 0, rmt_adv = 0;
	u8 flowctrl;
	u32 mcr;

	mcr = (IPG_96BIT_WITH_SHORT_IPG << IPG_CFG_S) |
	      (MAC_RX_PKT_LEN_1536 << MAC_RX_PKT_LEN_S) |
	      MAC_MODE | FORCE_MODE |
	      MAC_TX_EN | MAC_RX_EN |
	      DEL_RXFIFO_CLR |
	      BKOFF_EN | BACKPR_EN;

	switch (priv->phydev->speed) {
	case SPEED_10:
		mcr |= (SPEED_10M << FORCE_SPD_S);
		break;
	case SPEED_100:
		mcr |= (SPEED_100M << FORCE_SPD_S);
		break;
	case SPEED_1000:
	case SPEED_2500:
		mcr |= (SPEED_1000M << FORCE_SPD_S);
		break;
	};

	if (priv->phydev->link)
		mcr |= FORCE_LINK;

	if (priv->phydev->duplex) {
		mcr |= FORCE_DPX;

		if (priv->phydev->pause)
			rmt_adv = LPA_PAUSE_CAP;
		if (priv->phydev->asym_pause)
			rmt_adv |= LPA_PAUSE_ASYM;

		if (priv->phydev->advertising & ADVERTISED_Pause)
			lcl_adv |= ADVERTISE_PAUSE_CAP;
		if (priv->phydev->advertising & ADVERTISED_Asym_Pause)
			lcl_adv |= ADVERTISE_PAUSE_ASYM;

		flowctrl = mii_resolve_flowctrl_fdx(lcl_adv, rmt_adv);

		if (flowctrl & FLOW_CTRL_TX)
			mcr |= FORCE_TX_FC;
		if (flowctrl & FLOW_CTRL_RX)
			mcr |= FORCE_RX_FC;

		debug("rx pause %s, tx pause %s\n",
		      flowctrl & FLOW_CTRL_RX ? "enabled" : "disabled",
		      flowctrl & FLOW_CTRL_TX ? "enabled" : "disabled");
	}

	mtk_gmac_write(priv, GMAC_PORT_MCR(priv->gmac_id), mcr);
}

static int mtk_phy_start(struct mtk_eth_priv *priv)
{
	struct phy_device *phydev = priv->phydev;
	int ret;

	ret = phy_startup(phydev);

	if (ret) {
		debug("Could not initialize PHY %s\n", phydev->dev->name);
		return ret;
	}

	if (!phydev->link) {
		debug("%s: link down.\n", phydev->dev->name);
		return 0;
	}

	if (!priv->force_mode) {
		if (priv->phy_interface == PHY_INTERFACE_MODE_USXGMII)
			mtk_xphy_link_adjust(priv);
		else
			mtk_phy_link_adjust(priv);
	}

	debug("Speed: %d, %s duplex%s\n", phydev->speed,
	      (phydev->duplex) ? "full" : "half",
	      (phydev->port == PORT_FIBRE) ? ", fiber mode" : "");

	return 0;
}

static int mtk_phy_probe(struct udevice *dev)
{
	struct mtk_eth_priv *priv = dev_get_priv(dev);
	struct phy_device *phydev;

	phydev = phy_connect(priv->mdio_bus, priv->phy_addr, dev,
			     priv->phy_interface);
	if (!phydev)
		return -ENODEV;

	phydev->supported &= PHY_GBIT_FEATURES;
	phydev->advertising = phydev->supported;

	priv->phydev = phydev;
	phy_config(phydev);

	return 0;
}

static void mtk_sgmii_an_init(struct mtk_eth_priv *priv)
{
	/* Set SGMII GEN1 speed(1G) */
	clrsetbits_le32(priv->sgmii_base + priv->soc->ana_rgc3,
			SGMSYS_SPEED_2500, 0);

	/* Enable SGMII AN */
	setbits_le32(priv->sgmii_base + SGMSYS_PCS_CONTROL_1,
		     SGMII_AN_ENABLE);

	/* SGMII AN mode setting */
	writel(SGMII_AN_MODE, priv->sgmii_base + SGMSYS_SGMII_MODE);

	/* SGMII PN SWAP setting */
	if (priv->pn_swap) {
		setbits_le32(priv->sgmii_base + SGMSYS_QPHY_WRAP_CTRL,
			     SGMII_PN_SWAP_TX_RX);
	}

	/* Release PHYA power down state */
	clrsetbits_le32(priv->sgmii_base + SGMSYS_QPHY_PWR_STATE_CTRL,
			SGMII_PHYA_PWD, 0);
}

static void mtk_sgmii_force_init(struct mtk_eth_priv *priv)
{
	/* Set SGMII GEN2 speed(2.5G) */
	setbits_le32(priv->sgmii_base + priv->soc->ana_rgc3,
		     SGMSYS_SPEED_2500);

	/* Disable SGMII AN */
	clrsetbits_le32(priv->sgmii_base + SGMSYS_PCS_CONTROL_1,
			SGMII_AN_ENABLE, 0);

	/* SGMII force mode setting */
	writel(SGMII_FORCE_MODE, priv->sgmii_base + SGMSYS_SGMII_MODE);

	/* SGMII PN SWAP setting */
	if (priv->pn_swap) {
		setbits_le32(priv->sgmii_base + SGMSYS_QPHY_WRAP_CTRL,
			     SGMII_PN_SWAP_TX_RX);
	}

	/* Release PHYA power down state */
	clrsetbits_le32(priv->sgmii_base + SGMSYS_QPHY_PWR_STATE_CTRL,
			SGMII_PHYA_PWD, 0);
}

static void mtk_xfi_pll_enable(struct mtk_eth_priv *priv)
{
	u32 val = 0;

	/* Add software workaround for USXGMII PLL TCL issue */
	regmap_write(priv->xfi_pll_regmap, XFI_PLL_ANA_GLB8,
		     RG_XFI_PLL_ANA_SWWA);

	regmap_read(priv->xfi_pll_regmap, XFI_PLL_DIG_GLB8, &val);
	val |= RG_XFI_PLL_EN;
	regmap_write(priv->xfi_pll_regmap, XFI_PLL_DIG_GLB8, val);
}

static void mtk_usxgmii_reset(struct mtk_eth_priv *priv)
{
	switch (priv->gmac_id) {
	case 1:
		regmap_write(priv->toprgu_regmap, 0xFC, 0x0000A004);
		regmap_write(priv->toprgu_regmap, 0x18, 0x88F0A004);
		regmap_write(priv->toprgu_regmap, 0xFC, 0x00000000);
		regmap_write(priv->toprgu_regmap, 0x18, 0x88F00000);
		regmap_write(priv->toprgu_regmap, 0x18, 0x00F00000);
		break;
	case 2:
		regmap_write(priv->toprgu_regmap, 0xFC, 0x00005002);
		regmap_write(priv->toprgu_regmap, 0x18, 0x88F05002);
		regmap_write(priv->toprgu_regmap, 0xFC, 0x00000000);
		regmap_write(priv->toprgu_regmap, 0x18, 0x88F00000);
		regmap_write(priv->toprgu_regmap, 0x18, 0x00F00000);
		break;
	}

	mdelay(10);
}

static void mtk_usxgmii_setup_phya_an_10000(struct mtk_eth_priv *priv)
{
	regmap_write(priv->usxgmii_regmap, 0x810, 0x000FFE6D);
	regmap_write(priv->usxgmii_regmap, 0x818, 0x07B1EC7B);
	regmap_write(priv->usxgmii_regmap, 0x80C, 0x30000000);
	ndelay(1020);
	regmap_write(priv->usxgmii_regmap, 0x80C, 0x10000000);
	ndelay(1020);
	regmap_write(priv->usxgmii_regmap, 0x80C, 0x00000000);

	regmap_write(priv->xfi_pextp_regmap, 0x9024, 0x00C9071C);
	regmap_write(priv->xfi_pextp_regmap, 0x2020, 0xAA8585AA);
	regmap_write(priv->xfi_pextp_regmap, 0x2030, 0x0C020707);
	regmap_write(priv->xfi_pextp_regmap, 0x2034, 0x0E050F0F);
	regmap_write(priv->xfi_pextp_regmap, 0x2040, 0x00140032);
	regmap_write(priv->xfi_pextp_regmap, 0x50F0, 0x00C014AA);
	regmap_write(priv->xfi_pextp_regmap, 0x50E0, 0x3777C12B);
	regmap_write(priv->xfi_pextp_regmap, 0x506C, 0x005F9CFF);
	regmap_write(priv->xfi_pextp_regmap, 0x5070, 0x9D9DFAFA);
	regmap_write(priv->xfi_pextp_regmap, 0x5074, 0x27273F3F);
	regmap_write(priv->xfi_pextp_regmap, 0x5078, 0xA7883C68);
	regmap_write(priv->xfi_pextp_regmap, 0x507C, 0x11661166);
	regmap_write(priv->xfi_pextp_regmap, 0x5080, 0x0E000AAF);
	regmap_write(priv->xfi_pextp_regmap, 0x5084, 0x08080D0D);
	regmap_write(priv->xfi_pextp_regmap, 0x5088, 0x02030909);
	regmap_write(priv->xfi_pextp_regmap, 0x50E4, 0x0C0C0000);
	regmap_write(priv->xfi_pextp_regmap, 0x50E8, 0x04040000);
	regmap_write(priv->xfi_pextp_regmap, 0x50EC, 0x0F0F0C06);
	regmap_write(priv->xfi_pextp_regmap, 0x50A8, 0x506E8C8C);
	regmap_write(priv->xfi_pextp_regmap, 0x6004, 0x18190000);
	regmap_write(priv->xfi_pextp_regmap, 0x00F8, 0x01423342);
	regmap_write(priv->xfi_pextp_regmap, 0x00F4, 0x80201F20);
	regmap_write(priv->xfi_pextp_regmap, 0x0030, 0x00050C00);
	regmap_write(priv->xfi_pextp_regmap, 0x0070, 0x02002800);
	ndelay(1020);
	regmap_write(priv->xfi_pextp_regmap, 0x30B0, 0x00000020);
	regmap_write(priv->xfi_pextp_regmap, 0x3028, 0x00008A01);
	regmap_write(priv->xfi_pextp_regmap, 0x302C, 0x0000A884);
	regmap_write(priv->xfi_pextp_regmap, 0x3024, 0x00083002);
	regmap_write(priv->xfi_pextp_regmap, 0x3010, 0x00022220);
	regmap_write(priv->xfi_pextp_regmap, 0x5064, 0x0F020A01);
	regmap_write(priv->xfi_pextp_regmap, 0x50B4, 0x06100600);
	regmap_write(priv->xfi_pextp_regmap, 0x3048, 0x40704000);
	regmap_write(priv->xfi_pextp_regmap, 0x3050, 0xA8000000);
	regmap_write(priv->xfi_pextp_regmap, 0x3054, 0x000000AA);
	regmap_write(priv->xfi_pextp_regmap, 0x306C, 0x00000F00);
	regmap_write(priv->xfi_pextp_regmap, 0xA060, 0x00040000);
	regmap_write(priv->xfi_pextp_regmap, 0x90D0, 0x00000001);
	regmap_write(priv->xfi_pextp_regmap, 0x0070, 0x0200E800);
	udelay(150);
	regmap_write(priv->xfi_pextp_regmap, 0x0070, 0x0200C111);
	ndelay(1020);
	regmap_write(priv->xfi_pextp_regmap, 0x0070, 0x0200C101);
	udelay(15);
	regmap_write(priv->xfi_pextp_regmap, 0x0070, 0x0202C111);
	ndelay(1020);
	regmap_write(priv->xfi_pextp_regmap, 0x0070, 0x0202C101);
	udelay(100);
	regmap_write(priv->xfi_pextp_regmap, 0x30B0, 0x00000030);
	regmap_write(priv->xfi_pextp_regmap, 0x00F4, 0x80201F00);
	regmap_write(priv->xfi_pextp_regmap, 0x3040, 0x30000000);
	udelay(400);
}

static void mtk_usxgmii_an_init(struct mtk_eth_priv *priv)
{
	mtk_xfi_pll_enable(priv);
	mtk_usxgmii_reset(priv);
	mtk_usxgmii_setup_phya_an_10000(priv);
}

static void mtk_mac_init(struct mtk_eth_priv *priv)
{
	int i, ge_mode = 0;
	u32 mcr;

	switch (priv->phy_interface) {
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII:
		ge_mode = GE_MODE_RGMII;
		break;
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_2500BASEX:
		if (MTK_HAS_CAPS(priv->soc->caps, MTK_GMAC2_U3_QPHY)) {
			mtk_infra_rmw(priv, USB_PHY_SWITCH_REG, QPHY_SEL_MASK,
				      SGMII_QPHY_SEL);
		}

		ge_mode = GE_MODE_RGMII;
		mtk_ethsys_rmw(priv, ETHSYS_SYSCFG0_REG, SYSCFG0_SGMII_SEL_M,
			       SYSCFG0_SGMII_SEL(priv->gmac_id));
		if (priv->phy_interface == PHY_INTERFACE_MODE_SGMII)
			mtk_sgmii_an_init(priv);
		else
			mtk_sgmii_force_init(priv);
		break;
	case PHY_INTERFACE_MODE_MII:
	case PHY_INTERFACE_MODE_GMII:
		ge_mode = GE_MODE_MII;
		break;
	case PHY_INTERFACE_MODE_RMII:
		ge_mode = GE_MODE_RMII;
		break;
	default:
		break;
	}

	/* set the gmac to the right mode */
	mtk_ethsys_rmw(priv, ETHSYS_SYSCFG0_REG,
		       SYSCFG0_GE_MODE_M << SYSCFG0_GE_MODE_S(priv->gmac_id),
		       ge_mode << SYSCFG0_GE_MODE_S(priv->gmac_id));

	if (priv->force_mode) {
		mcr = (IPG_96BIT_WITH_SHORT_IPG << IPG_CFG_S) |
		      (MAC_RX_PKT_LEN_1536 << MAC_RX_PKT_LEN_S) |
		      MAC_MODE | FORCE_MODE |
		      MAC_TX_EN | MAC_RX_EN |
		      BKOFF_EN | BACKPR_EN |
		      FORCE_LINK;

		switch (priv->speed) {
		case SPEED_10:
			mcr |= SPEED_10M << FORCE_SPD_S;
			break;
		case SPEED_100:
			mcr |= SPEED_100M << FORCE_SPD_S;
			break;
		case SPEED_1000:
		case SPEED_2500:
			mcr |= SPEED_1000M << FORCE_SPD_S;
			break;
		}

		if (priv->duplex)
			mcr |= FORCE_DPX;

		mtk_gmac_write(priv, GMAC_PORT_MCR(priv->gmac_id), mcr);
	}

	if (MTK_HAS_CAPS(priv->soc->caps, MTK_GMAC1_TRGMII) &&
	    !MTK_HAS_CAPS(priv->soc->caps, MTK_TRGMII_MT7621_CLK)) {
		/* Lower Tx Driving for TRGMII path */
		for (i = 0 ; i < NUM_TRGMII_CTRL; i++)
			mtk_gmac_write(priv, GMAC_TRGMII_TD_ODT(i),
				       (8 << TD_DM_DRVP_S) |
				       (8 << TD_DM_DRVN_S));

		mtk_gmac_rmw(priv, GMAC_TRGMII_RCK_CTRL, 0,
			     RX_RST | RXC_DQSISEL);
		mtk_gmac_rmw(priv, GMAC_TRGMII_RCK_CTRL, RX_RST, 0);
	}
}

static void mtk_xmac_init(struct mtk_eth_priv *priv)
{
	u32 sts;

	switch (priv->phy_interface) {
	case PHY_INTERFACE_MODE_USXGMII:
		mtk_usxgmii_an_init(priv);
		break;
	default:
		break;
	}

	/* Set GMAC to the correct mode */
	mtk_ethsys_rmw(priv, ETHSYS_SYSCFG0_REG,
		       SYSCFG0_GE_MODE_M << SYSCFG0_GE_MODE_S(priv->gmac_id),
		       0);

	if (priv->gmac_id == 1) {
		mtk_infra_rmw(priv, TOPMISC_NETSYS_PCS_MUX,
			      NETSYS_PCS_MUX_MASK, MUX_G2_USXGMII_SEL);
	} else if (priv->gmac_id == 2) {
		sts = mtk_gmac_read(priv, XGMAC_STS(priv->gmac_id));
		sts |= XGMAC_FORCE_LINK;
		mtk_gmac_write(priv, XGMAC_STS(priv->gmac_id), sts);
	}

	/* Force GMAC link down */
	mtk_gmac_write(priv, GMAC_PORT_MCR(priv->gmac_id), FORCE_MODE);
}

static void mtk_eth_fifo_init(struct mtk_eth_priv *priv)
{
	char *pkt_base = priv->pkt_pool;
	struct mtk_tx_dma_v2 *txd;
	struct mtk_rx_dma_v2 *rxd;
	int i;

	mtk_pdma_rmw(priv, PDMA_GLO_CFG_REG, 0xffff0000, 0);
	udelay(500);

	memset(priv->tx_ring_noc, 0, NUM_TX_DESC * priv->soc->txd_size);
	memset(priv->rx_ring_noc, 0, NUM_RX_DESC * priv->soc->rxd_size);
	memset(priv->pkt_pool, 0xff, TOTAL_PKT_BUF_SIZE);

	flush_dcache_range((ulong)pkt_base,
			   (ulong)(pkt_base + TOTAL_PKT_BUF_SIZE));

	priv->rx_dma_owner_idx0 = 0;
	priv->tx_cpu_owner_idx0 = 0;

	for (i = 0; i < NUM_TX_DESC; i++) {
		txd = priv->tx_ring_noc + i * priv->soc->txd_size;

		txd->txd1 = virt_to_phys(pkt_base);
		txd->txd2 = PDMA_TXD2_DDONE | PDMA_TXD2_LS0;

		if (MTK_HAS_CAPS(priv->soc->caps, MTK_NETSYS_V3))
			txd->txd5 = PDMA_V2_TXD5_FPORT_SET(priv->gmac_id == 2 ?
							   15 : priv->gmac_id + 1);
		else if (MTK_HAS_CAPS(priv->soc->caps, MTK_NETSYS_V2))
			txd->txd5 = PDMA_V2_TXD5_FPORT_SET(priv->gmac_id + 1);
		else
			txd->txd4 = PDMA_V1_TXD4_FPORT_SET(priv->gmac_id + 1);

		pkt_base += PKTSIZE_ALIGN;
	}

	for (i = 0; i < NUM_RX_DESC; i++) {
		rxd = priv->rx_ring_noc + i * priv->soc->rxd_size;

		rxd->rxd1 = virt_to_phys(pkt_base);

		if (MTK_HAS_CAPS(priv->soc->caps, MTK_NETSYS_V2) ||
		    MTK_HAS_CAPS(priv->soc->caps, MTK_NETSYS_V3))
			rxd->rxd2 = PDMA_V2_RXD2_PLEN0_SET(PKTSIZE_ALIGN);
		else
			rxd->rxd2 = PDMA_V1_RXD2_PLEN0_SET(PKTSIZE_ALIGN);

		pkt_base += PKTSIZE_ALIGN;
	}

	mtk_pdma_write(priv, TX_BASE_PTR_REG(0),
		       virt_to_phys(priv->tx_ring_noc));
	mtk_pdma_write(priv, TX_MAX_CNT_REG(0), NUM_TX_DESC);
	mtk_pdma_write(priv, TX_CTX_IDX_REG(0), priv->tx_cpu_owner_idx0);

	mtk_pdma_write(priv, RX_BASE_PTR_REG(0),
		       virt_to_phys(priv->rx_ring_noc));
	mtk_pdma_write(priv, RX_MAX_CNT_REG(0), NUM_RX_DESC);
	mtk_pdma_write(priv, RX_CRX_IDX_REG(0), NUM_RX_DESC - 1);

	mtk_pdma_write(priv, PDMA_RST_IDX_REG, RST_DTX_IDX0 | RST_DRX_IDX0);
}

static int mtk_eth_start(struct udevice *dev)
{
	struct mtk_eth_priv *priv = dev_get_priv(dev);
	int i, ret;

	/* Reset FE */
	reset_assert(&priv->rst_fe);
	udelay(1000);
	reset_deassert(&priv->rst_fe);
	mdelay(10);

	if (MTK_HAS_CAPS(priv->soc->caps, MTK_NETSYS_V2) ||
	    MTK_HAS_CAPS(priv->soc->caps, MTK_NETSYS_V3))
		setbits_le32(priv->fe_base + FE_GLO_MISC_REG, PDMA_VER_V2);

	/* Packets forward to PDMA */
	mtk_gdma_write(priv, priv->gmac_id, GDMA_IG_CTRL_REG, GDMA_FWD_TO_CPU);

	for (i = 0; i < priv->soc->gdma_count; i++) {
		if (i == priv->gmac_id)
			continue;

		mtk_gdma_write(priv, i, GDMA_IG_CTRL_REG, GDMA_FWD_DISCARD);
	}

	if (MTK_HAS_CAPS(priv->soc->caps, MTK_NETSYS_V3)) {
		if (priv->sw == SW_MT7988 && priv->gmac_id == 0) {
			mtk_gdma_write(priv, priv->gmac_id, GDMA_IG_CTRL_REG,
				       GDMA_BRIDGE_TO_CPU);
		}

		mtk_gdma_write(priv, priv->gmac_id, GDMA_EG_CTRL_REG,
			       GDMA_CPU_BRIDGE_EN);
	}

	udelay(500);

	mtk_eth_fifo_init(priv);

	if (priv->switch_mac_control)
		priv->switch_mac_control(priv, true);

	/* Start PHY */
	if (priv->sw == SW_NONE) {
		ret = mtk_phy_start(priv);
		if (ret)
			return ret;
	}

	mtk_pdma_rmw(priv, PDMA_GLO_CFG_REG, 0,
		     TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN);
	udelay(500);

	return 0;
}

static void mtk_eth_stop(struct udevice *dev)
{
	struct mtk_eth_priv *priv = dev_get_priv(dev);

	if (priv->switch_mac_control)
		priv->switch_mac_control(priv, false);

	mtk_pdma_rmw(priv, PDMA_GLO_CFG_REG,
		     TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN, 0);
	udelay(500);

	wait_for_bit_le32(priv->fe_base + priv->soc->pdma_base + PDMA_GLO_CFG_REG,
			  RX_DMA_BUSY | TX_DMA_BUSY, 0, 5000, 0);
}

static int mtk_eth_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct mtk_eth_priv *priv = dev_get_priv(dev);
	unsigned char *mac = pdata->enetaddr;
	u32 macaddr_lsb, macaddr_msb;

	macaddr_msb = ((u32)mac[0] << 8) | (u32)mac[1];
	macaddr_lsb = ((u32)mac[2] << 24) | ((u32)mac[3] << 16) |
		      ((u32)mac[4] << 8) | (u32)mac[5];

	mtk_gdma_write(priv, priv->gmac_id, GDMA_MAC_MSB_REG, macaddr_msb);
	mtk_gdma_write(priv, priv->gmac_id, GDMA_MAC_LSB_REG, macaddr_lsb);

	return 0;
}

static int mtk_eth_send(struct udevice *dev, void *packet, int length)
{
	struct mtk_eth_priv *priv = dev_get_priv(dev);
	u32 idx = priv->tx_cpu_owner_idx0;
	struct mtk_tx_dma_v2 *txd;
	void *pkt_base;

	txd = priv->tx_ring_noc + idx * priv->soc->txd_size;

	if (!(txd->txd2 & PDMA_TXD2_DDONE)) {
		debug("mtk-eth: TX DMA descriptor ring is full\n");
		return -EPERM;
	}

	pkt_base = (void *)phys_to_virt(txd->txd1);
	memcpy(pkt_base, packet, length);
	flush_dcache_range((ulong)pkt_base, (ulong)pkt_base +
			   roundup(length, ARCH_DMA_MINALIGN));

	if (MTK_HAS_CAPS(priv->soc->caps, MTK_NETSYS_V2) ||
	    MTK_HAS_CAPS(priv->soc->caps, MTK_NETSYS_V3))
		txd->txd2 = PDMA_TXD2_LS0 | PDMA_V2_TXD2_SDL0_SET(length);
	else
		txd->txd2 = PDMA_TXD2_LS0 | PDMA_V1_TXD2_SDL0_SET(length);

	priv->tx_cpu_owner_idx0 = (priv->tx_cpu_owner_idx0 + 1) % NUM_TX_DESC;
	mtk_pdma_write(priv, TX_CTX_IDX_REG(0), priv->tx_cpu_owner_idx0);

	return 0;
}

static int mtk_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct mtk_eth_priv *priv = dev_get_priv(dev);
	u32 idx = priv->rx_dma_owner_idx0;
	struct mtk_rx_dma_v2 *rxd;
	uchar *pkt_base;
	u32 length;

	rxd = priv->rx_ring_noc + idx * priv->soc->rxd_size;

	if (!(rxd->rxd2 & PDMA_RXD2_DDONE)) {
		debug("mtk-eth: RX DMA descriptor ring is empty\n");
		return -EAGAIN;
	}

	if (MTK_HAS_CAPS(priv->soc->caps, MTK_NETSYS_V2) ||
	    MTK_HAS_CAPS(priv->soc->caps, MTK_NETSYS_V3))
		length = PDMA_V2_RXD2_PLEN0_GET(rxd->rxd2);
	else
		length = PDMA_V1_RXD2_PLEN0_GET(rxd->rxd2);

	pkt_base = (void *)phys_to_virt(rxd->rxd1);
	invalidate_dcache_range((ulong)pkt_base, (ulong)pkt_base +
				roundup(length, ARCH_DMA_MINALIGN));

	if (packetp)
		*packetp = pkt_base;

	return length;
}

static int mtk_eth_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct mtk_eth_priv *priv = dev_get_priv(dev);
	u32 idx = priv->rx_dma_owner_idx0;
	struct mtk_rx_dma_v2 *rxd;

	rxd = priv->rx_ring_noc + idx * priv->soc->rxd_size;

	if (MTK_HAS_CAPS(priv->soc->caps, MTK_NETSYS_V2) ||
	    MTK_HAS_CAPS(priv->soc->caps, MTK_NETSYS_V3))
		rxd->rxd2 = PDMA_V2_RXD2_PLEN0_SET(PKTSIZE_ALIGN);
	else
		rxd->rxd2 = PDMA_V1_RXD2_PLEN0_SET(PKTSIZE_ALIGN);

	mtk_pdma_write(priv, RX_CRX_IDX_REG(0), idx);
	priv->rx_dma_owner_idx0 = (priv->rx_dma_owner_idx0 + 1) % NUM_RX_DESC;

	return 0;
}

static int mtk_eth_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct mtk_eth_priv *priv = dev_get_priv(dev);
	ulong iobase = pdata->iobase;
	int ret;

	/* Frame Engine Register Base */
	priv->fe_base = (void *)iobase;

	/* GMAC Register Base */
	priv->gmac_base = (void *)(iobase + GMAC_BASE);

	/* MDIO register */
	ret = mtk_mdio_register(dev);
	if (ret)
		return ret;

	/* Prepare for tx/rx rings */
	priv->tx_ring_noc = (void *)
		noncached_alloc(priv->soc->txd_size * NUM_TX_DESC,
				ARCH_DMA_MINALIGN);
	priv->rx_ring_noc = (void *)
		noncached_alloc(priv->soc->rxd_size * NUM_RX_DESC,
				ARCH_DMA_MINALIGN);

	/* Set MAC mode */
	if (priv->phy_interface == PHY_INTERFACE_MODE_USXGMII)
		mtk_xmac_init(priv);
	else
		mtk_mac_init(priv);

	/* Probe phy if switch is not specified */
	if (priv->sw == SW_NONE)
		return mtk_phy_probe(dev);

	/* Initialize switch */
	return mt753x_switch_init(priv);
}

static int mtk_eth_remove(struct udevice *dev)
{
	struct mtk_eth_priv *priv = dev_get_priv(dev);

	/* MDIO unregister */
	mdio_unregister(priv->mdio_bus);
	mdio_free(priv->mdio_bus);

	/* Stop possibly started DMA */
	mtk_eth_stop(dev);

	return 0;
}

static int mtk_eth_of_to_plat(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct mtk_eth_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args args;
	struct regmap *regmap;
	const char *str;
	ofnode subnode;
	int ret;

	priv->soc = (const struct mtk_soc_data *)dev_get_driver_data(dev);
	if (!priv->soc) {
		dev_err(dev, "missing soc compatible data\n");
		return -EINVAL;
	}

	pdata->iobase = (phys_addr_t)dev_remap_addr(dev);

	/* get corresponding ethsys phandle */
	ret = dev_read_phandle_with_args(dev, "mediatek,ethsys", NULL, 0, 0,
					 &args);
	if (ret)
		return ret;

	priv->ethsys_regmap = syscon_node_to_regmap(args.node);
	if (IS_ERR(priv->ethsys_regmap))
		return PTR_ERR(priv->ethsys_regmap);

	if (MTK_HAS_CAPS(priv->soc->caps, MTK_INFRA)) {
		/* get corresponding infracfg phandle */
		ret = dev_read_phandle_with_args(dev, "mediatek,infracfg",
						 NULL, 0, 0, &args);

		if (ret)
			return ret;

		priv->infra_regmap = syscon_node_to_regmap(args.node);
		if (IS_ERR(priv->infra_regmap))
			return PTR_ERR(priv->infra_regmap);
	}

	/* Reset controllers */
	ret = reset_get_by_name(dev, "fe", &priv->rst_fe);
	if (ret) {
		printf("error: Unable to get reset ctrl for frame engine\n");
		return ret;
	}

	priv->gmac_id = dev_read_u32_default(dev, "mediatek,gmac-id", 0);

	/* Interface mode is required */
	pdata->phy_interface = dev_read_phy_mode(dev);
	priv->phy_interface = pdata->phy_interface;
	if (pdata->phy_interface == PHY_INTERFACE_MODE_NA) {
		printf("error: phy-mode is not set\n");
		return -EINVAL;
	}

	/* Force mode or autoneg */
	subnode = ofnode_find_subnode(dev_ofnode(dev), "fixed-link");
	if (ofnode_valid(subnode)) {
		priv->force_mode = 1;
		priv->speed = ofnode_read_u32_default(subnode, "speed", 0);
		priv->duplex = ofnode_read_bool(subnode, "full-duplex");

		if (priv->speed != SPEED_10 && priv->speed != SPEED_100 &&
		    priv->speed != SPEED_1000 && priv->speed != SPEED_2500 &&
		    priv->speed != SPEED_10000) {
			printf("error: no valid speed set in fixed-link\n");
			return -EINVAL;
		}
	}

	if (priv->phy_interface == PHY_INTERFACE_MODE_SGMII ||
	    priv->phy_interface == PHY_INTERFACE_MODE_2500BASEX) {
		/* get corresponding sgmii phandle */
		ret = dev_read_phandle_with_args(dev, "mediatek,sgmiisys",
						 NULL, 0, 0, &args);
		if (ret)
			return ret;

		regmap = syscon_node_to_regmap(args.node);

		if (IS_ERR(regmap))
			return PTR_ERR(regmap);

		priv->sgmii_base = regmap_get_range(regmap, 0);

		if (!priv->sgmii_base) {
			dev_err(dev, "Unable to find sgmii\n");
			return -ENODEV;
		}

		priv->pn_swap = ofnode_read_bool(args.node, "pn_swap");
	} else if (priv->phy_interface == PHY_INTERFACE_MODE_USXGMII) {
		/* get corresponding usxgmii phandle */
		ret = dev_read_phandle_with_args(dev, "mediatek,usxgmiisys",
						 NULL, 0, 0, &args);
		if (ret)
			return ret;

		priv->usxgmii_regmap = syscon_node_to_regmap(args.node);
		if (IS_ERR(priv->usxgmii_regmap))
			return PTR_ERR(priv->usxgmii_regmap);

		/* get corresponding xfi_pextp phandle */
		ret = dev_read_phandle_with_args(dev, "mediatek,xfi_pextp",
						 NULL, 0, 0, &args);
		if (ret)
			return ret;

		priv->xfi_pextp_regmap = syscon_node_to_regmap(args.node);
		if (IS_ERR(priv->xfi_pextp_regmap))
			return PTR_ERR(priv->xfi_pextp_regmap);

		/* get corresponding xfi_pll phandle */
		ret = dev_read_phandle_with_args(dev, "mediatek,xfi_pll",
						 NULL, 0, 0, &args);
		if (ret)
			return ret;

		priv->xfi_pll_regmap = syscon_node_to_regmap(args.node);
		if (IS_ERR(priv->xfi_pll_regmap))
			return PTR_ERR(priv->xfi_pll_regmap);

		/* get corresponding toprgu phandle */
		ret = dev_read_phandle_with_args(dev, "mediatek,toprgu",
						 NULL, 0, 0, &args);
		if (ret)
			return ret;

		priv->toprgu_regmap = syscon_node_to_regmap(args.node);
		if (IS_ERR(priv->toprgu_regmap))
			return PTR_ERR(priv->toprgu_regmap);
	}

	/* check for switch first, otherwise phy will be used */
	priv->sw = SW_NONE;
	priv->switch_init = NULL;
	priv->switch_mac_control = NULL;
	str = dev_read_string(dev, "mediatek,switch");

	if (str) {
		if (!strcmp(str, "mt7530")) {
			priv->sw = SW_MT7530;
			priv->switch_init = mt7530_setup;
			priv->switch_mac_control = mt7530_mac_control;
			priv->mt753x_smi_addr = MT753X_DFL_SMI_ADDR;
			priv->mt753x_reset_wait_time = 1000;
		} else if (!strcmp(str, "mt7531")) {
			priv->sw = SW_MT7531;
			priv->switch_init = mt7531_setup;
			priv->switch_mac_control = mt7531_mac_control;
			priv->mt753x_smi_addr = MT753X_DFL_SMI_ADDR;
			priv->mt753x_reset_wait_time = 200;
		} else if (!strcmp(str, "mt7988")) {
			priv->sw = SW_MT7988;
			priv->switch_init = mt7988_setup;
			priv->switch_mac_control = mt7988_mac_control;
			priv->mt753x_smi_addr = MT753X_DFL_SMI_ADDR;
			priv->mt753x_reset_wait_time = 50;
		} else {
			printf("error: unsupported switch\n");
			return -EINVAL;
		}

		priv->mcm = dev_read_bool(dev, "mediatek,mcm");
		if (priv->mcm) {
			ret = reset_get_by_name(dev, "mcm", &priv->rst_mcm);
			if (ret) {
				printf("error: no reset ctrl for mcm\n");
				return ret;
			}
		} else {
			gpio_request_by_name(dev, "reset-gpios", 0,
					     &priv->rst_gpio, GPIOD_IS_OUT);
		}
	} else {
		ret = dev_read_phandle_with_args(dev, "phy-handle", NULL, 0,
						 0, &args);
		if (ret) {
			printf("error: phy-handle is not specified\n");
			return ret;
		}

		priv->phy_addr = ofnode_read_s32_default(args.node, "reg", -1);
		if (priv->phy_addr < 0) {
			printf("error: phy address is not specified\n");
			return ret;
		}
	}

	return 0;
}

static const struct mtk_soc_data mt7988_data = {
	.caps = MT7988_CAPS,
	.ana_rgc3 = 0x128,
	.gdma_count = 3,
	.pdma_base = PDMA_V3_BASE,
	.txd_size = sizeof(struct mtk_tx_dma_v2),
	.rxd_size = sizeof(struct mtk_rx_dma_v2),
};

static const struct mtk_soc_data mt7986_data = {
	.caps = MT7986_CAPS,
	.ana_rgc3 = 0x128,
	.gdma_count = 2,
	.pdma_base = PDMA_V2_BASE,
	.txd_size = sizeof(struct mtk_tx_dma_v2),
	.rxd_size = sizeof(struct mtk_rx_dma_v2),
};

static const struct mtk_soc_data mt7981_data = {
	.caps = MT7981_CAPS,
	.ana_rgc3 = 0x128,
	.gdma_count = 2,
	.pdma_base = PDMA_V2_BASE,
	.txd_size = sizeof(struct mtk_tx_dma_v2),
	.rxd_size = sizeof(struct mtk_rx_dma_v2),
};

static const struct mtk_soc_data mt7629_data = {
	.ana_rgc3 = 0x128,
	.gdma_count = 2,
	.pdma_base = PDMA_V1_BASE,
	.txd_size = sizeof(struct mtk_tx_dma),
	.rxd_size = sizeof(struct mtk_rx_dma),
};

static const struct mtk_soc_data mt7623_data = {
	.caps = MT7623_CAPS,
	.gdma_count = 2,
	.pdma_base = PDMA_V1_BASE,
	.txd_size = sizeof(struct mtk_tx_dma),
	.rxd_size = sizeof(struct mtk_rx_dma),
};

static const struct mtk_soc_data mt7622_data = {
	.ana_rgc3 = 0x2028,
	.gdma_count = 2,
	.pdma_base = PDMA_V1_BASE,
	.txd_size = sizeof(struct mtk_tx_dma),
	.rxd_size = sizeof(struct mtk_rx_dma),
};

static const struct mtk_soc_data mt7621_data = {
	.caps = MT7621_CAPS,
	.gdma_count = 2,
	.pdma_base = PDMA_V1_BASE,
	.txd_size = sizeof(struct mtk_tx_dma),
	.rxd_size = sizeof(struct mtk_rx_dma),
};

static const struct udevice_id mtk_eth_ids[] = {
	{ .compatible = "mediatek,mt7988-eth", .data = (ulong)&mt7988_data },
	{ .compatible = "mediatek,mt7986-eth", .data = (ulong)&mt7986_data },
	{ .compatible = "mediatek,mt7981-eth", .data = (ulong)&mt7981_data },
	{ .compatible = "mediatek,mt7629-eth", .data = (ulong)&mt7629_data },
	{ .compatible = "mediatek,mt7623-eth", .data = (ulong)&mt7623_data },
	{ .compatible = "mediatek,mt7622-eth", .data = (ulong)&mt7622_data },
	{ .compatible = "mediatek,mt7621-eth", .data = (ulong)&mt7621_data },
	{}
};

static const struct eth_ops mtk_eth_ops = {
	.start = mtk_eth_start,
	.stop = mtk_eth_stop,
	.send = mtk_eth_send,
	.recv = mtk_eth_recv,
	.free_pkt = mtk_eth_free_pkt,
	.write_hwaddr = mtk_eth_write_hwaddr,
};

U_BOOT_DRIVER(mtk_eth) = {
	.name = "mtk-eth",
	.id = UCLASS_ETH,
	.of_match = mtk_eth_ids,
	.of_to_plat = mtk_eth_of_to_plat,
	.plat_auto	= sizeof(struct eth_pdata),
	.probe = mtk_eth_probe,
	.remove = mtk_eth_remove,
	.ops = &mtk_eth_ops,
	.priv_auto	= sizeof(struct mtk_eth_priv),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
