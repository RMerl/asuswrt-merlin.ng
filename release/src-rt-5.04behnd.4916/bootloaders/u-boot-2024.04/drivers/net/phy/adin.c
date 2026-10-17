// SPDX-License-Identifier: GPL-2.0+
/**
 *  Driver for Analog Devices Industrial Ethernet PHYs
 *
 * Copyright 2019 Analog Devices Inc.
 * Copyright 2022 Variscite Ltd.
 * Copyright 2022 Josua Mayer <josua@solid-run.com>
 */
#include <common.h>
#include <phy.h>
#include <linux/bitops.h>
#include <linux/bitfield.h>

#define PHY_ID_ADIN1300				0x0283bc30
#define ADIN1300_EXT_REG_PTR			0x10
#define ADIN1300_EXT_REG_DATA			0x11

#define ADIN1300_GE_CLK_CFG_REG			0xff1f
#define   ADIN1300_GE_CLK_CFG_MASK		GENMASK(5, 0)
#define   ADIN1300_GE_CLK_CFG_RCVR_125		BIT(5)
#define   ADIN1300_GE_CLK_CFG_FREE_125		BIT(4)
#define   ADIN1300_GE_CLK_CFG_REF_EN		BIT(3)
#define   ADIN1300_GE_CLK_CFG_HRT_RCVR		BIT(2)
#define   ADIN1300_GE_CLK_CFG_HRT_FREE		BIT(1)
#define   ADIN1300_GE_CLK_CFG_25		BIT(0)

#define ADIN1300_GE_RGMII_CFG			0xff23
#define ADIN1300_GE_RGMII_RX_MSK		GENMASK(8, 6)
#define ADIN1300_GE_RGMII_RX_SEL(x)		\
		FIELD_PREP(ADIN1300_GE_RGMII_RX_MSK, x)
#define ADIN1300_GE_RGMII_GTX_MSK		GENMASK(5, 3)
#define ADIN1300_GE_RGMII_GTX_SEL(x)		\
		FIELD_PREP(ADIN1300_GE_RGMII_GTX_MSK, x)
#define ADIN1300_GE_RGMII_RXID_EN		BIT(2)
#define ADIN1300_GE_RGMII_TXID_EN		BIT(1)
#define ADIN1300_GE_RGMII_EN			BIT(0)

/* RGMII internal delay settings for rx and tx for ADIN1300 */
#define ADIN1300_RGMII_1_60_NS			0x0001
#define ADIN1300_RGMII_1_80_NS			0x0002
#define	ADIN1300_RGMII_2_00_NS			0x0000
#define	ADIN1300_RGMII_2_20_NS			0x0006
#define	ADIN1300_RGMII_2_40_NS			0x0007

/**
 * struct adin_cfg_reg_map - map a config value to aregister value
 * @cfg		value in device configuration
 * @reg		value in the register
 */
struct adin_cfg_reg_map {
	int cfg;
	int reg;
};

static const struct adin_cfg_reg_map adin_rgmii_delays[] = {
	{ 1600, ADIN1300_RGMII_1_60_NS },
	{ 1800, ADIN1300_RGMII_1_80_NS },
	{ 2000, ADIN1300_RGMII_2_00_NS },
	{ 2200, ADIN1300_RGMII_2_20_NS },
	{ 2400, ADIN1300_RGMII_2_40_NS },
	{ },
};

static int adin_lookup_reg_value(const struct adin_cfg_reg_map *tbl, int cfg)
{
	size_t i;

	for (i = 0; tbl[i].cfg; i++) {
		if (tbl[i].cfg == cfg)
			return tbl[i].reg;
	}

	return -EINVAL;
}

static u32 adin_get_reg_value(struct phy_device *phydev,
			      const char *prop_name,
			      const struct adin_cfg_reg_map *tbl,
			      u32 dflt)
{
	u32 val;
	int rc;

	ofnode node = phy_get_ofnode(phydev);
	if (!ofnode_valid(node)) {
		printf("%s: failed to get node\n", __func__);
		return -EINVAL;
	}

	if (ofnode_read_u32(node, prop_name, &val)) {
		printf("%s: failed to find %s, using default %d\n",
		       __func__, prop_name, dflt);
		return dflt;
	}

	debug("%s: %s = '%d'\n", __func__, prop_name, val);

	rc = adin_lookup_reg_value(tbl, val);
	if (rc < 0) {
		printf("%s: Unsupported value %u for %s using default (%u)\n",
		      __func__, val, prop_name, dflt);
		return dflt;
	}

	return rc;
}

/**
 * adin_get_phy_mode_override - Get phy-mode override for adin PHY
 *
 * The function gets phy-mode string from property 'adi,phy-mode-override'
 * and return its index in phy_interface_strings table, or -1 in error case.
 */
phy_interface_t adin_get_phy_mode_override(struct phy_device *phydev)
{
	ofnode node = phy_get_ofnode(phydev);
	const char *phy_mode_override;
	const char *prop_phy_mode_override = "adi,phy-mode-override";
	int i;

	phy_mode_override = ofnode_read_string(node, prop_phy_mode_override);
	if (!phy_mode_override)
		return PHY_INTERFACE_MODE_NA;

	debug("%s: %s = '%s'\n",
	      __func__, prop_phy_mode_override, phy_mode_override);

	for (i = 0; i < PHY_INTERFACE_MODE_MAX; i++)
		if (!strcmp(phy_mode_override, phy_interface_strings[i]))
			return (phy_interface_t) i;

	printf("%s: Invalid PHY interface '%s'\n", __func__, phy_mode_override);

	return PHY_INTERFACE_MODE_NA;
}

static u16 adin_ext_read(struct phy_device *phydev, const u32 regnum)
{
	u16 val;

	phy_write(phydev, MDIO_DEVAD_NONE, ADIN1300_EXT_REG_PTR, regnum);
	val = phy_read(phydev, MDIO_DEVAD_NONE, ADIN1300_EXT_REG_DATA);

	debug("%s: adin@0x%x 0x%x=0x%x\n", __func__, phydev->addr, regnum, val);

	return val;
}

static int adin_ext_write(struct phy_device *phydev, const u32 regnum, const u16 val)
{
	debug("%s: adin@0x%x 0x%x=0x%x\n", __func__, phydev->addr, regnum, val);

	phy_write(phydev, MDIO_DEVAD_NONE, ADIN1300_EXT_REG_PTR, regnum);

	return phy_write(phydev, MDIO_DEVAD_NONE, ADIN1300_EXT_REG_DATA, val);
}

static int adin_extread(struct phy_device *phydev, int addr, int devaddr,
			int regnum)
{
	return adin_ext_read(phydev, regnum);
}

static int adin_extwrite(struct phy_device *phydev, int addr,
			 int devaddr, int regnum, u16 val)
{
	return adin_ext_write(phydev, regnum, val);
}

static int adin_config_clk_out(struct phy_device *phydev)
{
	ofnode node = phy_get_ofnode(phydev);
	const char *val = NULL;
	u8 sel = 0;

	val = ofnode_read_string(node, "adi,phy-output-clock");
	if (!val) {
		/* property not present, do not enable GP_CLK pin */
	} else if (strcmp(val, "25mhz-reference") == 0) {
		sel |= ADIN1300_GE_CLK_CFG_25;
	} else if (strcmp(val, "125mhz-free-running") == 0) {
		sel |= ADIN1300_GE_CLK_CFG_FREE_125;
	} else if (strcmp(val, "adaptive-free-running") == 0) {
		sel |= ADIN1300_GE_CLK_CFG_HRT_FREE;
	} else {
		pr_err("%s: invalid adi,phy-output-clock\n", __func__);
		return -EINVAL;
	}

	if (ofnode_read_bool(node, "adi,phy-output-reference-clock"))
		sel |= ADIN1300_GE_CLK_CFG_REF_EN;

	return adin_ext_write(phydev, ADIN1300_GE_CLK_CFG_REG,
			      ADIN1300_GE_CLK_CFG_MASK & sel);
}

static int adin_config_rgmii_mode(struct phy_device *phydev)
{
	u16 reg_val;
	u32 val;
	phy_interface_t phy_mode_override = adin_get_phy_mode_override(phydev);

	if (phy_mode_override != PHY_INTERFACE_MODE_NA) {
		phydev->interface = phy_mode_override;
	}

	reg_val = adin_ext_read(phydev, ADIN1300_GE_RGMII_CFG);

	if (!phy_interface_is_rgmii(phydev)) {
		/* Disable RGMII */
		reg_val &= ~ADIN1300_GE_RGMII_EN;
		return adin_ext_write(phydev, ADIN1300_GE_RGMII_CFG, reg_val);
	}

	/* Enable RGMII */
	reg_val |= ADIN1300_GE_RGMII_EN;

	/* Enable / Disable RGMII RX Delay */
	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID ||
	    phydev->interface == PHY_INTERFACE_MODE_RGMII_RXID) {
		reg_val |= ADIN1300_GE_RGMII_RXID_EN;

		val = adin_get_reg_value(phydev, "adi,rx-internal-delay-ps",
					 adin_rgmii_delays,
					 ADIN1300_RGMII_2_00_NS);
		reg_val &= ~ADIN1300_GE_RGMII_RX_MSK;
		reg_val |= ADIN1300_GE_RGMII_RX_SEL(val);
	} else {
		reg_val &= ~ADIN1300_GE_RGMII_RXID_EN;
	}

	/* Enable / Disable RGMII RX Delay */
	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID ||
	    phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID) {
		reg_val |= ADIN1300_GE_RGMII_TXID_EN;

		val = adin_get_reg_value(phydev, "adi,tx-internal-delay-ps",
					 adin_rgmii_delays,
					 ADIN1300_RGMII_2_00_NS);
		reg_val &= ~ADIN1300_GE_RGMII_GTX_MSK;
		reg_val |= ADIN1300_GE_RGMII_GTX_SEL(val);
	} else {
		reg_val &= ~ADIN1300_GE_RGMII_TXID_EN;
	}

	return adin_ext_write(phydev, ADIN1300_GE_RGMII_CFG, reg_val);
}

static int adin1300_config(struct phy_device *phydev)
{
	int ret;

	printf("ADIN1300 PHY detected at addr %d\n", phydev->addr);

	ret = adin_config_clk_out(phydev);
	if (ret < 0)
		return ret;

	ret = adin_config_rgmii_mode(phydev);

	if (ret < 0)
		return ret;

	return genphy_config(phydev);
}

U_BOOT_PHY_DRIVER(ADIN1300) = {
	.name = "ADIN1300",
	.uid = PHY_ID_ADIN1300,
	.mask = 0xffffffff,
	.features = PHY_GBIT_FEATURES,
	.config = adin1300_config,
	.startup = genphy_startup,
	.shutdown = genphy_shutdown,
	.readext = adin_extread,
	.writeext = adin_extwrite,
};
