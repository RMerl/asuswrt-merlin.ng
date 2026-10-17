// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - https://www.ti.com/
 * Jean-Jacques Hiblot <jjhiblot@ti.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <asm/gpio.h>
#include <dm/lists.h>
#include <dm/device-internal.h>
#include <regmap.h>
#include <reset-uclass.h>
#include <dt-bindings/phy/phy.h>

#include <dt-bindings/phy/phy-ti.h>

#define WIZ_MAX_INPUT_CLOCKS	4
/* To include mux clocks, divider clocks and gate clocks */
#define WIZ_MAX_OUTPUT_CLOCKS	32

#define WIZ_MAX_LANES		4
#define WIZ_MUX_NUM_CLOCKS	3
#define WIZ_DIV_NUM_CLOCKS_16G	2
#define WIZ_DIV_NUM_CLOCKS_10G	1

#define WIZ_SERDES_CTRL		0x404
#define WIZ_SERDES_TOP_CTRL	0x408
#define WIZ_SERDES_RST		0x40c
#define WIZ_SERDES_TYPEC	0x410
#define WIZ_LANECTL(n)		(0x480 + (0x40 * (n)))
#define WIZ_LANEDIV(n)		(0x484 + (0x40 * (n)))

#define WIZ_MAX_LANES		4
#define WIZ_MUX_NUM_CLOCKS	3
#define WIZ_DIV_NUM_CLOCKS_16G	2
#define WIZ_DIV_NUM_CLOCKS_10G	1

#define WIZ_SERDES_TYPEC_LN10_SWAP	BIT(30)
#define WIZ_SERDES_TYPEC_LN23_SWAP	BIT(31)

enum wiz_lane_standard_mode {
	LANE_MODE_GEN1,
	LANE_MODE_GEN2,
	LANE_MODE_GEN3,
	LANE_MODE_GEN4,
};

enum wiz_refclk_mux_sel {
	PLL0_REFCLK,
	PLL1_REFCLK,
	REFCLK_DIG,
};

enum wiz_refclk_div_sel {
	CMN_REFCLK,
	CMN_REFCLK1,
};

enum wiz_clock_input {
	WIZ_CORE_REFCLK,
	WIZ_EXT_REFCLK,
	WIZ_CORE_REFCLK1,
	WIZ_EXT_REFCLK1,
};

/*
 * List of master lanes used for lane swapping
 */
enum wiz_typec_master_lane {
	LANE0 = 0,
	LANE2 = 2,
};

static const struct reg_field por_en = REG_FIELD(WIZ_SERDES_CTRL, 31, 31);
static const struct reg_field phy_reset_n = REG_FIELD(WIZ_SERDES_RST, 31, 31);
static const struct reg_field pll1_refclk_mux_sel =
					REG_FIELD(WIZ_SERDES_RST, 29, 29);
static const struct reg_field pll1_refclk_mux_sel_2 =
					REG_FIELD(WIZ_SERDES_RST, 22, 23);
static const struct reg_field pll0_refclk_mux_sel =
					REG_FIELD(WIZ_SERDES_RST, 28, 28);
static const struct reg_field pll0_refclk_mux_sel_2 =
					REG_FIELD(WIZ_SERDES_RST, 28, 29);
static const struct reg_field refclk_dig_sel_16g =
					REG_FIELD(WIZ_SERDES_RST, 24, 25);
static const struct reg_field refclk_dig_sel_10g =
					REG_FIELD(WIZ_SERDES_RST, 24, 24);
static const struct reg_field pma_cmn_refclk_int_mode =
					REG_FIELD(WIZ_SERDES_TOP_CTRL, 28, 29);
static const struct reg_field pma_cmn_refclk1_int_mode =
					REG_FIELD(WIZ_SERDES_TOP_CTRL, 20, 21);
static const struct reg_field pma_cmn_refclk_mode =
					REG_FIELD(WIZ_SERDES_TOP_CTRL, 30, 31);
static const struct reg_field pma_cmn_refclk_dig_div =
					REG_FIELD(WIZ_SERDES_TOP_CTRL, 26, 27);
static const struct reg_field pma_cmn_refclk1_dig_div =
					REG_FIELD(WIZ_SERDES_TOP_CTRL, 24, 25);

static const struct reg_field p_enable[WIZ_MAX_LANES] = {
	REG_FIELD(WIZ_LANECTL(0), 30, 31),
	REG_FIELD(WIZ_LANECTL(1), 30, 31),
	REG_FIELD(WIZ_LANECTL(2), 30, 31),
	REG_FIELD(WIZ_LANECTL(3), 30, 31),
};

enum p_enable { P_ENABLE = 2, P_ENABLE_FORCE = 1, P_ENABLE_DISABLE = 0 };

static const struct reg_field p_align[WIZ_MAX_LANES] = {
	REG_FIELD(WIZ_LANECTL(0), 29, 29),
	REG_FIELD(WIZ_LANECTL(1), 29, 29),
	REG_FIELD(WIZ_LANECTL(2), 29, 29),
	REG_FIELD(WIZ_LANECTL(3), 29, 29),
};

static const struct reg_field p_raw_auto_start[WIZ_MAX_LANES] = {
	REG_FIELD(WIZ_LANECTL(0), 28, 28),
	REG_FIELD(WIZ_LANECTL(1), 28, 28),
	REG_FIELD(WIZ_LANECTL(2), 28, 28),
	REG_FIELD(WIZ_LANECTL(3), 28, 28),
};

static const struct reg_field p_standard_mode[WIZ_MAX_LANES] = {
	REG_FIELD(WIZ_LANECTL(0), 24, 25),
	REG_FIELD(WIZ_LANECTL(1), 24, 25),
	REG_FIELD(WIZ_LANECTL(2), 24, 25),
	REG_FIELD(WIZ_LANECTL(3), 24, 25),
};

static const struct reg_field p0_fullrt_div[WIZ_MAX_LANES] = {
	REG_FIELD(WIZ_LANECTL(0), 22, 23),
	REG_FIELD(WIZ_LANECTL(1), 22, 23),
	REG_FIELD(WIZ_LANECTL(2), 22, 23),
	REG_FIELD(WIZ_LANECTL(3), 22, 23),
};

static const struct reg_field p_mac_div_sel0[WIZ_MAX_LANES] = {
	REG_FIELD(WIZ_LANEDIV(0), 16, 22),
	REG_FIELD(WIZ_LANEDIV(1), 16, 22),
	REG_FIELD(WIZ_LANEDIV(2), 16, 22),
	REG_FIELD(WIZ_LANEDIV(3), 16, 22),
};

static const struct reg_field p_mac_div_sel1[WIZ_MAX_LANES] = {
	REG_FIELD(WIZ_LANEDIV(0), 0, 8),
	REG_FIELD(WIZ_LANEDIV(1), 0, 8),
	REG_FIELD(WIZ_LANEDIV(2), 0, 8),
	REG_FIELD(WIZ_LANEDIV(3), 0, 8),
};

struct wiz_clk_mux_sel {
	enum wiz_refclk_mux_sel mux_sel;
	u32			table[WIZ_MAX_INPUT_CLOCKS];
	const char		*node_name;
	u32			num_parents;
	u32			parents[WIZ_MAX_INPUT_CLOCKS];
};

struct wiz_clk_div_sel {
	enum wiz_refclk_div_sel div_sel;
	const char		*node_name;
};

static struct wiz_clk_mux_sel clk_mux_sel_16g[] = {
	{
		/*
		 * Mux value to be configured for each of the input clocks
		 * in the order populated in device tree
		 */
		.num_parents = 2,
		.parents = { WIZ_CORE_REFCLK, WIZ_EXT_REFCLK },
		.mux_sel = PLL0_REFCLK,
		.table = { 1, 0 },
		.node_name = "pll0-refclk",
	},
	{
		.num_parents = 2,
		.parents = { WIZ_CORE_REFCLK1, WIZ_EXT_REFCLK1 },
		.mux_sel = PLL1_REFCLK,
		.table = { 1, 0 },
		.node_name = "pll1-refclk",
	},
	{
		.num_parents = 4,
		.parents = { WIZ_CORE_REFCLK, WIZ_CORE_REFCLK1, WIZ_EXT_REFCLK, WIZ_EXT_REFCLK1 },
		.mux_sel = REFCLK_DIG,
		.table = { 1, 3, 0, 2 },
		.node_name = "refclk-dig",
	},
};

static struct wiz_clk_mux_sel clk_mux_sel_10g[] = {
	{
		/*
		 * Mux value to be configured for each of the input clocks
		 * in the order populated in device tree
		 */
		.num_parents = 2,
		.parents = { WIZ_CORE_REFCLK, WIZ_EXT_REFCLK },
		.mux_sel = PLL0_REFCLK,
		.table = { 1, 0 },
		.node_name = "pll0-refclk",
	},
	{
		.num_parents = 2,
		.parents = { WIZ_CORE_REFCLK, WIZ_EXT_REFCLK },
		.mux_sel = PLL1_REFCLK,
		.table = { 1, 0 },
		.node_name = "pll1-refclk",
	},
	{
		.num_parents = 2,
		.parents = { WIZ_CORE_REFCLK, WIZ_EXT_REFCLK },
		.mux_sel = REFCLK_DIG,
		.table = { 1, 0 },
		.node_name = "refclk-dig",
	},
};

static const struct wiz_clk_mux_sel clk_mux_sel_10g_2_refclk[] = {
	{
		.num_parents = 3,
		.parents = { WIZ_CORE_REFCLK, WIZ_CORE_REFCLK1, WIZ_EXT_REFCLK },
		.table = { 2, 3, 0 },
		.node_name = "pll0-refclk",
	},
	{
		.num_parents = 3,
		.parents = { WIZ_CORE_REFCLK, WIZ_CORE_REFCLK1, WIZ_EXT_REFCLK },
		.table = { 2, 3, 0 },
		.node_name = "pll1-refclk",
	},
	{
		.num_parents = 3,
		.parents = { WIZ_CORE_REFCLK, WIZ_CORE_REFCLK1, WIZ_EXT_REFCLK },
		.table = { 2, 3, 0 },
		.node_name = "refclk-dig",
	},
};

static struct wiz_clk_div_sel clk_div_sel[] = {
	{
		.div_sel = CMN_REFCLK,
		.node_name = "cmn-refclk-dig-div",
	},
	{
		.div_sel = CMN_REFCLK1,
		.node_name = "cmn-refclk1-dig-div",
	},
};

enum wiz_type {
	J721E_WIZ_16G,
	J721E_WIZ_10G,
	AM64_WIZ_10G,
	J784S4_WIZ_10G,
	J721S2_WIZ_10G,
};

struct wiz_data {
	enum wiz_type type;
	const struct reg_field *pll0_refclk_mux_sel;
	const struct reg_field *pll1_refclk_mux_sel;
	const struct reg_field *refclk_dig_sel;
	const struct reg_field *pma_cmn_refclk1_dig_div;
	const struct reg_field *pma_cmn_refclk1_int_mode;
	const struct wiz_clk_mux_sel *clk_mux_sel;
	unsigned int clk_div_sel_num;
};

static const struct wiz_data j721e_16g_data = {
	.type = J721E_WIZ_16G,
	.pll0_refclk_mux_sel = &pll0_refclk_mux_sel,
	.pll1_refclk_mux_sel = &pll1_refclk_mux_sel,
	.refclk_dig_sel = &refclk_dig_sel_16g,
	.pma_cmn_refclk1_dig_div = &pma_cmn_refclk1_dig_div,
	.clk_mux_sel = clk_mux_sel_16g,
	.clk_div_sel_num = WIZ_DIV_NUM_CLOCKS_16G,
};

static const struct wiz_data j721e_10g_data = {
	.type = J721E_WIZ_10G,
	.pll0_refclk_mux_sel = &pll0_refclk_mux_sel,
	.pll1_refclk_mux_sel = &pll1_refclk_mux_sel,
	.refclk_dig_sel = &refclk_dig_sel_10g,
	.clk_mux_sel = clk_mux_sel_10g,
	.clk_div_sel_num = WIZ_DIV_NUM_CLOCKS_10G,
};

static struct wiz_data am64_10g_data = {
	.type = AM64_WIZ_10G,
	.pll0_refclk_mux_sel = &pll0_refclk_mux_sel,
	.pll1_refclk_mux_sel = &pll1_refclk_mux_sel,
	.refclk_dig_sel = &refclk_dig_sel_10g,
	.clk_mux_sel = clk_mux_sel_10g,
	.clk_div_sel_num = WIZ_DIV_NUM_CLOCKS_10G,
};

static struct wiz_data j784s4_wiz_10g = {
	.type = J784S4_WIZ_10G,
	.pll0_refclk_mux_sel = &pll0_refclk_mux_sel_2,
	.pll1_refclk_mux_sel = &pll1_refclk_mux_sel_2,
	.refclk_dig_sel = &refclk_dig_sel_16g,
	.pma_cmn_refclk1_int_mode = &pma_cmn_refclk1_int_mode,
	.clk_mux_sel = clk_mux_sel_10g_2_refclk,
	.clk_div_sel_num = WIZ_DIV_NUM_CLOCKS_10G,
};

static struct wiz_data j721s2_10g_data = {
	.type = J721S2_WIZ_10G,
	.pll0_refclk_mux_sel = &pll0_refclk_mux_sel,
	.pll1_refclk_mux_sel = &pll1_refclk_mux_sel,
	.refclk_dig_sel = &refclk_dig_sel_10g,
	.clk_mux_sel = clk_mux_sel_10g,
	.clk_div_sel_num = WIZ_DIV_NUM_CLOCKS_10G,
};

#define WIZ_TYPEC_DIR_DEBOUNCE_MIN	100	/* ms */
#define WIZ_TYPEC_DIR_DEBOUNCE_MAX	1000

struct wiz {
	struct regmap		*regmap;
	enum wiz_type		type;
	struct wiz_clk_mux_sel	*clk_mux_sel;
	struct wiz_clk_div_sel	*clk_div_sel;
	unsigned int		clk_div_sel_num;
	struct regmap_field	*por_en;
	struct regmap_field	*phy_reset_n;
	struct regmap_field	*phy_en_refclk;
	struct regmap_field	*p_enable[WIZ_MAX_LANES];
	struct regmap_field	*p_align[WIZ_MAX_LANES];
	struct regmap_field	*p_raw_auto_start[WIZ_MAX_LANES];
	struct regmap_field	*p_standard_mode[WIZ_MAX_LANES];
	struct regmap_field	*p_mac_div_sel0[WIZ_MAX_LANES];
	struct regmap_field	*p_mac_div_sel1[WIZ_MAX_LANES];
	struct regmap_field	*p0_fullrt_div[WIZ_MAX_LANES];
	struct regmap_field	*pma_cmn_refclk_int_mode;
	struct regmap_field	*pma_cmn_refclk1_int_mode;
	struct regmap_field	*pma_cmn_refclk_mode;
	struct regmap_field	*pma_cmn_refclk_dig_div;
	struct regmap_field	*pma_cmn_refclk1_dig_div;
	struct regmap_field	*div_sel_field[WIZ_DIV_NUM_CLOCKS_16G];
	struct regmap_field	*mux_sel_field[WIZ_MUX_NUM_CLOCKS];

	struct udevice		*dev;
	u32			num_lanes;
	struct gpio_desc	*gpio_typec_dir;
	u32			lane_phy_type[WIZ_MAX_LANES];
	u32			master_lane_num[WIZ_MAX_LANES];
	struct clk		*input_clks[WIZ_MAX_INPUT_CLOCKS];
	unsigned int		id;
	const struct wiz_data	*data;
};

struct wiz_div_clk {
	struct clk parent_clk;
	struct wiz *wiz;
};

struct wiz_mux_clk {
	struct clk parent_clks[4];
	struct wiz *wiz;
};

struct wiz_clk {
	struct wiz *wiz;
};

struct wiz_reset {
	struct wiz *wiz;
};

static ulong wiz_div_clk_get_rate(struct clk *clk)
{
	struct udevice *dev = clk->dev;
	struct wiz_div_clk *priv = dev_get_priv(dev);
	struct wiz_clk_div_sel *data = dev_get_plat(dev);
	struct wiz *wiz = priv->wiz;
	ulong parent_rate = clk_get_rate(&priv->parent_clk);
	u32 val;

	regmap_field_read(wiz->div_sel_field[data->div_sel], &val);

	return parent_rate >> val;
}

static ulong wiz_div_clk_set_rate(struct clk *clk, ulong rate)
{
	struct udevice *dev = clk->dev;
	struct wiz_div_clk *priv = dev_get_priv(dev);
	struct wiz_clk_div_sel *data = dev_get_plat(dev);
	struct wiz *wiz = priv->wiz;
	ulong parent_rate = clk_get_rate(&priv->parent_clk);
	u32 div = parent_rate / rate;

	div = __ffs(div);
	regmap_field_write(wiz->div_sel_field[data->div_sel], div);

	return parent_rate >> div;
}

const struct clk_ops wiz_div_clk_ops = {
	.get_rate = wiz_div_clk_get_rate,
	.set_rate = wiz_div_clk_set_rate,
};

int wiz_div_clk_probe(struct udevice *dev)
{
	struct wiz_div_clk *priv = dev_get_priv(dev);
	struct clk parent_clk;
	int rc;

	rc = clk_get_by_index(dev, 0, &parent_clk);
	if (rc) {
		dev_err(dev, "unable to get parent clock. ret %d\n", rc);
		return rc;
	}
	priv->parent_clk = parent_clk;
	priv->wiz = dev_get_priv(dev->parent);
	return 0;
}

U_BOOT_DRIVER(wiz_div_clk) = {
	.name		= "wiz_div_clk",
	.id		= UCLASS_CLK,
	.priv_auto	= sizeof(struct wiz_div_clk),
	.ops		= &wiz_div_clk_ops,
	.probe		= wiz_div_clk_probe,
};

static int wiz_clk_mux_set_parent(struct clk *clk,  struct clk *parent)
{
	struct udevice *dev = clk->dev;
	struct wiz_mux_clk *priv = dev_get_priv(dev);
	struct wiz_clk_mux_sel *data = dev_get_plat(dev);
	struct wiz *wiz = priv->wiz;
	int i;

	for (i = 0; i < ARRAY_SIZE(priv->parent_clks); i++)
		if (parent->dev == priv->parent_clks[i].dev)
			break;

	if (i == ARRAY_SIZE(priv->parent_clks))
		return -EINVAL;

	regmap_field_write(wiz->mux_sel_field[data->mux_sel], data->table[i]);
	return 0;
}

static int wiz_clk_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	struct udevice *dev = clk->dev;
	struct wiz_mux_clk *priv = dev_get_priv(dev);
	struct wiz *wiz = priv->wiz;

	clk->id = wiz->id;

	return 0;
}

static const struct clk_ops wiz_clk_mux_ops = {
	.set_parent = wiz_clk_mux_set_parent,
	.of_xlate = wiz_clk_xlate,
};

int wiz_mux_clk_probe(struct udevice *dev)
{
	struct wiz_mux_clk *priv = dev_get_priv(dev);
	int rc;
	int i;

	for (i = 0; i < ARRAY_SIZE(priv->parent_clks); i++) {
		rc = clk_get_by_index(dev, i, &priv->parent_clks[i]);
		if (rc)
			priv->parent_clks[i].dev = NULL;
	}
	priv->wiz = dev_get_priv(dev->parent);
	return 0;
}

U_BOOT_DRIVER(wiz_mux_clk) = {
	.name		= "wiz_mux_clk",
	.id		= UCLASS_CLK,
	.priv_auto	= sizeof(struct wiz_mux_clk),
	.ops		= &wiz_clk_mux_ops,
	.probe		= wiz_mux_clk_probe,
};

static int wiz_clk_set_parent(struct clk *clk,  struct clk *parent)
{
	struct udevice *dev = clk->dev;
	struct wiz_clk *priv = dev_get_priv(dev);
	const struct wiz_clk_mux_sel *mux_sel;
	struct wiz *wiz = priv->wiz;
	int num_parents;
	int i, j, id;

	id = clk->id >> 10;

	/* set_parent is applicable only for MUX clocks */
	if (id > TI_WIZ_REFCLK_DIG)
		return 0;

	for (i = 0; i < WIZ_MAX_INPUT_CLOCKS; i++)
		if (wiz->input_clks[i]->dev == parent->dev)
			break;

	if (i == WIZ_MAX_INPUT_CLOCKS)
		return -EINVAL;

	mux_sel = &wiz->clk_mux_sel[id];
	num_parents = mux_sel->num_parents;
	for (j = 0; j < num_parents; j++)
		if (mux_sel->parents[j] == i)
			break;

	if (j == num_parents)
		return -EINVAL;

	regmap_field_write(wiz->mux_sel_field[id], mux_sel->table[j]);

	return 0;
}

static int wiz_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	struct udevice *dev = clk->dev;
	struct wiz_clk *priv = dev_get_priv(dev);
	struct wiz *wiz = priv->wiz;

	clk->id = args->args[0] << 10 | wiz->id;

	return 0;
}

static const struct clk_ops wiz_clk_ops = {
	.set_parent = wiz_clk_set_parent,
	.of_xlate = wiz_clk_of_xlate,
};

int wiz_clk_probe(struct udevice *dev)
{
	struct wiz_clk *priv = dev_get_priv(dev);

	priv->wiz = dev_get_priv(dev->parent);

	return 0;
}

U_BOOT_DRIVER(wiz_clk) = {
	.name		= "wiz_clk",
	.id		= UCLASS_CLK,
	.priv_auto	= sizeof(struct wiz_clk),
	.ops		= &wiz_clk_ops,
	.probe		= wiz_clk_probe,
};

static int wiz_reset_request(struct reset_ctl *reset_ctl)
{
	return 0;
}

static int wiz_reset_free(struct reset_ctl *reset_ctl)
{
	return 0;
}

static int wiz_reset_assert(struct reset_ctl *reset_ctl)
{
	struct wiz_reset *priv = dev_get_priv(reset_ctl->dev);
	struct wiz *wiz = priv->wiz;
	int ret;
	int id = reset_ctl->id;

	if (id == 0) {
		ret = regmap_field_write(wiz->phy_reset_n, false);
		return ret;
	}

	ret = regmap_field_write(wiz->p_enable[id - 1], P_ENABLE_DISABLE);
	return ret;
}

static int wiz_phy_fullrt_div(struct wiz *wiz, int lane)
{
	switch (wiz->type) {
	case AM64_WIZ_10G:
		if (wiz->lane_phy_type[lane] == PHY_TYPE_PCIE)
			return regmap_field_write(wiz->p0_fullrt_div[lane], 0x1);
		break;

	case J721E_WIZ_16G:
	case J721E_WIZ_10G:
		if (wiz->lane_phy_type[lane] == PHY_TYPE_SGMII)
			return regmap_field_write(wiz->p0_fullrt_div[lane], 0x2);
		break;
	default:
		return 0;
	}
	return 0;
}

static int wiz_reset_deassert(struct reset_ctl *reset_ctl)
{
	struct wiz_reset *priv = dev_get_priv(reset_ctl->dev);
	struct wiz *wiz = priv->wiz;
	int ret;
	int id = reset_ctl->id;

	ret = wiz_phy_fullrt_div(wiz, id - 1);
	if (ret)
		return ret;

	/* if typec-dir gpio was specified, set LN10 SWAP bit based on that */
	if (id == 0) {
		if (wiz->gpio_typec_dir) {
			if (dm_gpio_get_value(wiz->gpio_typec_dir)) {
				regmap_update_bits(wiz->regmap, WIZ_SERDES_TYPEC,
						WIZ_SERDES_TYPEC_LN10_SWAP,
						WIZ_SERDES_TYPEC_LN10_SWAP);
			} else {
				regmap_update_bits(wiz->regmap, WIZ_SERDES_TYPEC,
						WIZ_SERDES_TYPEC_LN10_SWAP, 0);
			}
		}
	} else {
		/* if no typec-dir gpio was specified and PHY type is
		 * USB3 with master lane number is '0', set LN10 SWAP
		 * bit to '1'
		 */
		u32 num_lanes = wiz->num_lanes;
		int i;

		for (i = 0; i < num_lanes; i++) {
			if (wiz->lane_phy_type[i] == PHY_TYPE_USB3) {
				switch (wiz->master_lane_num[i]) {
				case LANE0:
					regmap_update_bits(wiz->regmap, WIZ_SERDES_TYPEC,
							WIZ_SERDES_TYPEC_LN10_SWAP,
							WIZ_SERDES_TYPEC_LN10_SWAP);
					break;
				case LANE2:
					 regmap_update_bits(wiz->regmap, WIZ_SERDES_TYPEC,
							WIZ_SERDES_TYPEC_LN23_SWAP,
							WIZ_SERDES_TYPEC_LN23_SWAP);
					break;
				default:
					break;
				}
			}
		}
	}

	if (id == 0) {
		ret = regmap_field_write(wiz->phy_reset_n, true);
		return ret;
	}

	if (wiz->lane_phy_type[id - 1] == PHY_TYPE_DP)
		ret = regmap_field_write(wiz->p_enable[id - 1], P_ENABLE);
	else
		ret = regmap_field_write(wiz->p_enable[id - 1], P_ENABLE_FORCE);

	return ret;
}

static struct reset_ops wiz_reset_ops = {
	.request = wiz_reset_request,
	.rfree = wiz_reset_free,
	.rst_assert = wiz_reset_assert,
	.rst_deassert = wiz_reset_deassert,
};

int wiz_reset_probe(struct udevice *dev)
{
	struct wiz_reset *priv = dev_get_priv(dev);

	priv->wiz = dev_get_priv(dev->parent);

	return 0;
}

U_BOOT_DRIVER(wiz_reset) = {
	.name = "wiz-reset",
	.id = UCLASS_RESET,
	.probe = wiz_reset_probe,
	.ops = &wiz_reset_ops,
	.flags = DM_FLAG_LEAVE_PD_ON,
};

static int wiz_reset(struct wiz *wiz)
{
	int ret;

	ret = regmap_field_write(wiz->por_en, 0x1);
	if (ret)
		return ret;

	mdelay(1);

	ret = regmap_field_write(wiz->por_en, 0x0);
	if (ret)
		return ret;

	return 0;
}

static int wiz_p_mac_div_sel(struct wiz *wiz)
{
	u32 num_lanes = wiz->num_lanes;
	int ret;
	int i;

	for (i = 0; i < num_lanes; i++) {
		if (wiz->lane_phy_type[i] == PHY_TYPE_SGMII ||
		    wiz->lane_phy_type[i] == PHY_TYPE_QSGMII) {
			ret = regmap_field_write(wiz->p_mac_div_sel0[i], 1);
			if (ret)
				return ret;

			ret = regmap_field_write(wiz->p_mac_div_sel1[i], 2);
			if (ret)
				return ret;
		}
	}

	return 0;
}

static int wiz_mode_select(struct wiz *wiz)
{
	u32 num_lanes = wiz->num_lanes;
	int ret;
	int i;

	for (i = 0; i < num_lanes; i++) {
		if (wiz->lane_phy_type[i] == PHY_TYPE_QSGMII) {
			ret = regmap_field_write(wiz->p_standard_mode[i],
						 LANE_MODE_GEN2);
			if (ret)
				return ret;
		}
	}

	return 0;
}

static int wiz_init_raw_interface(struct wiz *wiz, bool enable)
{
	u32 num_lanes = wiz->num_lanes;
	int i;
	int ret;

	for (i = 0; i < num_lanes; i++) {
		ret = regmap_field_write(wiz->p_align[i], enable);
		if (ret)
			return ret;

		ret = regmap_field_write(wiz->p_raw_auto_start[i], enable);
		if (ret)
			return ret;
	}

	return 0;
}

static int wiz_init(struct wiz *wiz)
{
	struct udevice *dev = wiz->dev;
	int ret;

	ret = wiz_reset(wiz);
	if (ret) {
		dev_err(dev, "WIZ reset failed\n");
		return ret;
	}

	ret = wiz_mode_select(wiz);
	if (ret) {
		dev_err(dev, "WIZ mode select failed\n");
		return ret;
	}

	ret = wiz_p_mac_div_sel(wiz);
	if (ret) {
		dev_err(dev, "Configuring P0 MAC DIV SEL failed\n");
		return ret;
	}

	ret = wiz_init_raw_interface(wiz, true);
	if (ret) {
		dev_err(dev, "WIZ interface initialization failed\n");
		return ret;
	}

	return 0;
}

static int wiz_regfield_init(struct wiz *wiz)
{
	struct regmap *regmap = wiz->regmap;
	int num_lanes = wiz->num_lanes;
	struct udevice *dev = wiz->dev;
	const struct wiz_data *data = wiz->data;
	int i;

	wiz->por_en = devm_regmap_field_alloc(dev, regmap, por_en);
	if (IS_ERR(wiz->por_en)) {
		dev_err(dev, "POR_EN reg field init failed\n");
		return PTR_ERR(wiz->por_en);
	}

	wiz->phy_reset_n = devm_regmap_field_alloc(dev, regmap,
						   phy_reset_n);
	if (IS_ERR(wiz->phy_reset_n)) {
		dev_err(dev, "PHY_RESET_N reg field init failed\n");
		return PTR_ERR(wiz->phy_reset_n);
	}

	wiz->pma_cmn_refclk_int_mode =
		devm_regmap_field_alloc(dev, regmap, pma_cmn_refclk_int_mode);
	if (IS_ERR(wiz->pma_cmn_refclk_int_mode)) {
		dev_err(dev, "PMA_CMN_REFCLK_INT_MODE reg field init failed\n");
		return PTR_ERR(wiz->pma_cmn_refclk_int_mode);
	}

	if (data->pma_cmn_refclk1_int_mode) {
		wiz->pma_cmn_refclk1_int_mode =
			devm_regmap_field_alloc(dev, regmap, *data->pma_cmn_refclk1_int_mode);
		if (IS_ERR(wiz->pma_cmn_refclk1_int_mode)) {
			dev_err(dev, "PMA_CMN_REFCLK1_INT_MODE reg field init failed\n");
			return PTR_ERR(wiz->pma_cmn_refclk1_int_mode);
		}
	}

	wiz->pma_cmn_refclk_mode =
		devm_regmap_field_alloc(dev, regmap, pma_cmn_refclk_mode);
	if (IS_ERR(wiz->pma_cmn_refclk_mode)) {
		dev_err(dev, "PMA_CMN_REFCLK_MODE reg field init failed\n");
		return PTR_ERR(wiz->pma_cmn_refclk_mode);
	}

	wiz->div_sel_field[CMN_REFCLK] =
		devm_regmap_field_alloc(dev, regmap, pma_cmn_refclk_dig_div);
	if (IS_ERR(wiz->div_sel_field[CMN_REFCLK])) {
		dev_err(dev, "PMA_CMN_REFCLK_DIG_DIV reg field init failed\n");
		return PTR_ERR(wiz->div_sel_field[CMN_REFCLK]);
	}

	if (data->pma_cmn_refclk1_dig_div) {
		wiz->div_sel_field[CMN_REFCLK1] =
			devm_regmap_field_alloc(dev, regmap, *data->pma_cmn_refclk1_dig_div);
		if (IS_ERR(wiz->div_sel_field[CMN_REFCLK1])) {
			dev_err(dev, "PMA_CMN_REFCLK1_DIG_DIV reg field init failed\n");
			return PTR_ERR(wiz->div_sel_field[CMN_REFCLK1]);
		}
	}

	wiz->mux_sel_field[PLL0_REFCLK] =
		devm_regmap_field_alloc(dev, regmap, *data->pll0_refclk_mux_sel);
	if (IS_ERR(wiz->mux_sel_field[PLL0_REFCLK])) {
		dev_err(dev, "PLL0_REFCLK_SEL reg field init failed\n");
		return PTR_ERR(wiz->mux_sel_field[PLL0_REFCLK]);
	}

	wiz->mux_sel_field[PLL1_REFCLK] =
		devm_regmap_field_alloc(dev, regmap, *data->pll1_refclk_mux_sel);
	if (IS_ERR(wiz->mux_sel_field[PLL1_REFCLK])) {
		dev_err(dev, "PLL1_REFCLK_SEL reg field init failed\n");
		return PTR_ERR(wiz->mux_sel_field[PLL1_REFCLK]);
	}

	wiz->mux_sel_field[REFCLK_DIG] =
		devm_regmap_field_alloc(dev, regmap, *data->refclk_dig_sel);
	if (IS_ERR(wiz->mux_sel_field[REFCLK_DIG])) {
		dev_err(dev, "REFCLK_DIG_SEL reg field init failed\n");
		return PTR_ERR(wiz->mux_sel_field[REFCLK_DIG]);
	}

	for (i = 0; i < num_lanes; i++) {
		wiz->p_enable[i] = devm_regmap_field_alloc(dev, regmap,
							   p_enable[i]);
		if (IS_ERR(wiz->p_enable[i])) {
			dev_err(dev, "P%d_ENABLE reg field init failed\n", i);
			return PTR_ERR(wiz->p_enable[i]);
		}

		wiz->p_align[i] = devm_regmap_field_alloc(dev, regmap,
							  p_align[i]);
		if (IS_ERR(wiz->p_align[i])) {
			dev_err(dev, "P%d_ALIGN reg field init failed\n", i);
			return PTR_ERR(wiz->p_align[i]);
		}

		wiz->p_raw_auto_start[i] =
		  devm_regmap_field_alloc(dev, regmap, p_raw_auto_start[i]);
		if (IS_ERR(wiz->p_raw_auto_start[i])) {
			dev_err(dev, "P%d_RAW_AUTO_START reg field init fail\n",
				i);
			return PTR_ERR(wiz->p_raw_auto_start[i]);
		}

		wiz->p_standard_mode[i] =
		  devm_regmap_field_alloc(dev, regmap, p_standard_mode[i]);
		if (IS_ERR(wiz->p_standard_mode[i])) {
			dev_err(dev, "P%d_STANDARD_MODE reg field init fail\n",
				i);
			return PTR_ERR(wiz->p_standard_mode[i]);
		}

		wiz->p0_fullrt_div[i] = devm_regmap_field_alloc(dev, regmap, p0_fullrt_div[i]);
		if (IS_ERR(wiz->p0_fullrt_div[i])) {
			dev_err(dev, "P%d_FULLRT_DIV reg field init failed\n", i);
			return PTR_ERR(wiz->p0_fullrt_div[i]);
		}

		wiz->p_mac_div_sel0[i] =
		  devm_regmap_field_alloc(dev, regmap, p_mac_div_sel0[i]);
		if (IS_ERR(wiz->p_mac_div_sel0[i])) {
			dev_err(dev, "P%d_MAC_DIV_SEL0 reg field init fail\n",
				i);
			return PTR_ERR(wiz->p_mac_div_sel0[i]);
		}

		wiz->p_mac_div_sel1[i] =
		  devm_regmap_field_alloc(dev, regmap, p_mac_div_sel1[i]);
		if (IS_ERR(wiz->p_mac_div_sel1[i])) {
			dev_err(dev, "P%d_MAC_DIV_SEL1 reg field init fail\n",
				i);
			return PTR_ERR(wiz->p_mac_div_sel1[i]);
		}
	}

	return 0;
}

static int wiz_clock_init(struct wiz *wiz)
{
	struct udevice *dev = wiz->dev;
	unsigned long rate;
	struct clk *clk;
	int ret;

	clk = devm_clk_get(dev, "core_ref_clk");
	if (IS_ERR(clk)) {
		dev_err(dev, "core_ref_clk clock not found\n");
		ret = PTR_ERR(clk);
		return ret;
	}
	wiz->input_clks[WIZ_CORE_REFCLK] = clk;

	rate = clk_get_rate(clk);
	if (rate >= 100000000)
		regmap_field_write(wiz->pma_cmn_refclk_int_mode, 0x1);
	else
		regmap_field_write(wiz->pma_cmn_refclk_int_mode, 0x3);

	if (wiz->data->pma_cmn_refclk1_int_mode) {
		clk = devm_clk_get(dev, "core_ref1_clk");
		if (IS_ERR(clk)) {
			dev_err(dev, "core_ref1_clk clock not found\n");
			ret = PTR_ERR(clk);
			return ret;
		}
		wiz->input_clks[WIZ_CORE_REFCLK1] = clk;

		rate = clk_get_rate(clk);
		if (rate >= 100000000)
			regmap_field_write(wiz->pma_cmn_refclk1_int_mode, 0x1);
		else
			regmap_field_write(wiz->pma_cmn_refclk1_int_mode, 0x3);
	} else {
		/* Initialize CORE_REFCLK1 to the same clock reference to maintain old DT compatibility */
		wiz->input_clks[WIZ_CORE_REFCLK1] = clk;
	}

	clk = devm_clk_get(dev, "ext_ref_clk");
	if (IS_ERR(clk)) {
		dev_err(dev, "ext_ref_clk clock not found\n");
		ret = PTR_ERR(clk);
		return ret;
	}

	wiz->input_clks[WIZ_EXT_REFCLK] = clk;
	/* Initialize EXT_REFCLK1 to the same clock reference to maintain old DT compatibility */
	wiz->input_clks[WIZ_EXT_REFCLK1] = clk;

	rate = clk_get_rate(clk);
	if (rate >= 100000000)
		regmap_field_write(wiz->pma_cmn_refclk_mode, 0x0);
	else
		regmap_field_write(wiz->pma_cmn_refclk_mode, 0x2);

	return 0;
}

static ofnode get_child_by_name(struct udevice *dev, const char *name)
{
	int l = strlen(name);
	ofnode node = dev_read_first_subnode(dev);

	while (ofnode_valid(node)) {
		const char *child_name = ofnode_get_name(node);

		if (!strncmp(child_name, name, l)) {
			if (child_name[l] == '\0' || child_name[l] == '@')
				return node;
		}
		node = dev_read_next_subnode(node);
	}
	return node;
}

static int j721e_wiz_bind_clocks(struct wiz *wiz)
{
	struct udevice *dev = wiz->dev;
	struct driver *wiz_clk_drv;
	int i, rc;

	wiz_clk_drv = lists_driver_lookup_name("wiz_clk");
	if (!wiz_clk_drv) {
		dev_err(dev, "Cannot find driver 'wiz_clk'\n");
		return -ENOENT;
	}

	for (i = 0; i < WIZ_DIV_NUM_CLOCKS_10G; i++) {
		rc = device_bind(dev, wiz_clk_drv, clk_div_sel[i].node_name,
				 &clk_div_sel[i], dev_ofnode(dev), NULL);
		if (rc) {
			dev_err(dev, "cannot bind driver for clock %s\n",
				clk_div_sel[i].node_name);
		}
	}

	for (i = 0; i < WIZ_MUX_NUM_CLOCKS; i++) {
		rc = device_bind(dev, wiz_clk_drv, clk_mux_sel_10g[i].node_name,
				 &clk_mux_sel_10g[i], dev_ofnode(dev), NULL);
		if (rc) {
			dev_err(dev, "cannot bind driver for clock %s\n",
				clk_mux_sel_10g[i].node_name);
		}
	}

	return 0;
}

static int j721e_wiz_bind_of_clocks(struct wiz *wiz)
{
	struct wiz_clk_mux_sel *clk_mux_sel = wiz->clk_mux_sel;
	struct udevice *dev = wiz->dev;
	enum wiz_type type = wiz->type;
	struct driver *div_clk_drv;
	struct driver *mux_clk_drv;
	ofnode node;
	int i, rc;

	switch (type) {
	case AM64_WIZ_10G:
	case J784S4_WIZ_10G:
	case J721S2_WIZ_10G:
		return j721e_wiz_bind_clocks(wiz);
	default:
		break;
	};

	div_clk_drv = lists_driver_lookup_name("wiz_div_clk");
	if (!div_clk_drv) {
		dev_err(dev, "Cannot find driver 'wiz_div_clk'\n");
		return -ENOENT;
	}

	mux_clk_drv = lists_driver_lookup_name("wiz_mux_clk");
	if (!mux_clk_drv) {
		dev_err(dev, "Cannot find driver 'wiz_mux_clk'\n");
		return -ENOENT;
	}

	for (i = 0; i < wiz->clk_div_sel_num; i++) {
		node = get_child_by_name(dev, clk_div_sel[i].node_name);
		if (!ofnode_valid(node)) {
			dev_err(dev, "cannot find node for clock %s\n",
				clk_div_sel[i].node_name);
			continue;
		}
		rc = device_bind(dev, div_clk_drv, clk_div_sel[i].node_name,
				 &clk_div_sel[i], node, NULL);
		if (rc) {
			dev_err(dev, "cannot bind driver for clock %s\n",
				clk_div_sel[i].node_name);
		}
	}

	for (i = 0; i < WIZ_MUX_NUM_CLOCKS; i++) {
		node = get_child_by_name(dev, clk_mux_sel[i].node_name);
		if (!ofnode_valid(node)) {
			dev_err(dev, "cannot find node for clock %s\n",
				clk_mux_sel[i].node_name);
			continue;
		}
		rc = device_bind(dev, mux_clk_drv, clk_mux_sel[i].node_name,
				 &clk_mux_sel[i], node, NULL);
		if (rc) {
			dev_err(dev, "cannot bind driver for clock %s\n",
				clk_mux_sel[i].node_name);
		}
	}

	return 0;
}

static int j721e_wiz_bind_reset(struct udevice *dev)
{
	int rc;
	struct driver *drv;

	drv = lists_driver_lookup_name("wiz-reset");
	if (!drv) {
		dev_err(dev, "Cannot find driver 'wiz-reset'\n");
		return -ENOENT;
	}

	rc = device_bind(dev, drv, "wiz-reset", NULL, dev_ofnode(dev), NULL);
	if (rc) {
		dev_err(dev, "cannot bind driver for wiz-reset\n");
		return rc;
	}

	return 0;
}

static int j721e_wiz_bind(struct udevice *dev)
{
	dm_scan_fdt_dev(dev);

	return 0;
}

static int wiz_get_lane_phy_types(struct udevice *dev, struct wiz *wiz)
{
	ofnode child, serdes;

	serdes = get_child_by_name(dev, "serdes");
	if (!ofnode_valid(serdes)) {
		dev_err(dev, "%s: Getting \"serdes\"-node failed\n", __func__);
		return -EINVAL;
	}

	ofnode_for_each_subnode(child, serdes) {
		u32 reg, num_lanes = 1, phy_type = PHY_NONE;
		int ret, i;

		ret = ofnode_read_u32(child, "reg", &reg);
		if (ret) {
			dev_err(dev, "%s: Reading \"reg\" from failed: %d\n",
				__func__, ret);
			return ret;
		}
		ofnode_read_u32(child, "cdns,num-lanes", &num_lanes);
		ofnode_read_u32(child, "cdns,phy-type", &phy_type);

		dev_dbg(dev, "%s: Lanes %u-%u have phy-type %u\n", __func__,
			reg, reg + num_lanes - 1, phy_type);

		for (i = reg; i < reg + num_lanes; i++) {
			wiz->lane_phy_type[i] = phy_type;
			wiz->master_lane_num[i] = reg;
		}
	}

	return 0;
}

static int j721e_wiz_probe(struct udevice *dev)
{
	struct wiz *wiz = dev_get_priv(dev);
	struct ofnode_phandle_args args;
	unsigned int val;
	int rc, i;
	ofnode node;
	struct regmap *regmap;
	u32 num_lanes;

	node = get_child_by_name(dev, "serdes");

	if (!ofnode_valid(node)) {
		dev_err(dev, "Failed to get SERDES child DT node\n");
		return -ENODEV;
	}

	rc = regmap_init_mem(node, &regmap);
	if (rc)  {
		dev_err(dev, "Failed to get memory resource\n");
		return rc;
	}
	rc = dev_read_u32(dev, "num-lanes", &num_lanes);
	if (rc) {
		dev_err(dev, "Failed to read num-lanes property\n");
		goto err_addr_to_resource;
	}

	if (num_lanes > WIZ_MAX_LANES) {
		dev_err(dev, "Cannot support %d lanes\n", num_lanes);
		goto err_addr_to_resource;
	}

	wiz->gpio_typec_dir = devm_gpiod_get_optional(dev, "typec-dir",
						      GPIOD_IS_IN);
	if (IS_ERR(wiz->gpio_typec_dir)) {
		rc = PTR_ERR(wiz->gpio_typec_dir);
		dev_err(dev, "Failed to request typec-dir gpio: %d\n", rc);
		goto err_addr_to_resource;
	}

	rc = dev_read_phandle_with_args(dev, "power-domains", "#power-domain-cells", 0, 0, &args);
	if (rc) {
		dev_err(dev, "Failed to get power domain: %d\n", rc);
		goto err_addr_to_resource;
	}

	wiz->id = args.args[0];
	wiz->regmap = regmap;
	wiz->num_lanes = num_lanes;
	wiz->dev = dev;
	wiz->clk_div_sel = clk_div_sel;

	wiz->data = (struct wiz_data *)dev_get_driver_data(dev);
	wiz->type = wiz->data->type;

	wiz->clk_mux_sel = (struct wiz_clk_mux_sel *)wiz->data->clk_mux_sel;
	wiz->clk_div_sel_num = wiz->data->clk_div_sel_num;

	rc = wiz_get_lane_phy_types(dev, wiz);
	if (rc) {
		dev_err(dev, "Failed to get lane PHY types\n");
		goto err_addr_to_resource;
	}

	rc = wiz_regfield_init(wiz);
	if (rc) {
		dev_err(dev, "Failed to initialize regfields\n");
		goto err_addr_to_resource;
	}

	for (i = 0; i < wiz->num_lanes; i++) {
		regmap_field_read(wiz->p_enable[i], &val);
		if (val & (P_ENABLE | P_ENABLE_FORCE)) {
			dev_err(dev, "SERDES already configured\n");
			rc = -EBUSY;
			goto err_addr_to_resource;
		}
	}

	rc = j721e_wiz_bind_of_clocks(wiz);
	if (rc) {
		dev_err(dev, "Failed to bind clocks\n");
		goto err_addr_to_resource;
	}

	rc = j721e_wiz_bind_reset(dev);
	if (rc) {
		dev_err(dev, "Failed to bind reset\n");
		goto err_addr_to_resource;
	}

	rc = wiz_clock_init(wiz);
	if (rc) {
		dev_warn(dev, "Failed to initialize clocks\n");
		goto err_addr_to_resource;
	}

	rc = wiz_init(wiz);
	if (rc) {
		dev_err(dev, "WIZ initialization failed\n");
		goto err_addr_to_resource;
	}

	return 0;

err_addr_to_resource:
	free(regmap);

	return rc;
}

static int j721e_wiz_remove(struct udevice *dev)
{
	struct wiz *wiz = dev_get_priv(dev);

	if (wiz->regmap)
		free(wiz->regmap);

	return 0;
}

static const struct udevice_id j721e_wiz_ids[] = {
	{
		.compatible = "ti,j721e-wiz-16g", .data = (ulong)&j721e_16g_data,
	},
	{
		.compatible = "ti,j721e-wiz-10g", .data = (ulong)&j721e_10g_data,
	},
	{
		.compatible = "ti,am64-wiz-10g", .data = (ulong)&am64_10g_data,
	},
	{
		.compatible = "ti,j784s4-wiz-10g", .data = (ulong)&j784s4_wiz_10g,
	},
	{
		.compatible = "ti,j721s2-wiz-10g", .data = (ulong)&j721s2_10g_data,
	},
	{}
};

U_BOOT_DRIVER(phy_j721e_wiz) = {
	.name		= "phy-j721e-wiz",
	.id		= UCLASS_NOP,
	.of_match	= j721e_wiz_ids,
	.bind		= j721e_wiz_bind,
	.probe		= j721e_wiz_probe,
	.remove		= j721e_wiz_remove,
	.priv_auto	= sizeof(struct wiz),
	.flags		= DM_FLAG_LEAVE_PD_ON,
};
