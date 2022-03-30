// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * Written by Jean-Jacques Hiblot  <jjhiblot@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/device.h>
#include <generic-phy.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <syscon.h>
#include <regmap.h>

/* PLLCTRL Registers */
#define PLL_STATUS              0x00000004
#define PLL_GO                  0x00000008
#define PLL_CONFIGURATION1      0x0000000C
#define PLL_CONFIGURATION2      0x00000010
#define PLL_CONFIGURATION3      0x00000014
#define PLL_CONFIGURATION4      0x00000020

#define PLL_REGM_MASK           0x001FFE00
#define PLL_REGM_SHIFT          9
#define PLL_REGM_F_MASK         0x0003FFFF
#define PLL_REGM_F_SHIFT        0
#define PLL_REGN_MASK           0x000001FE
#define PLL_REGN_SHIFT          1
#define PLL_SELFREQDCO_MASK     0x0000000E
#define PLL_SELFREQDCO_SHIFT    1
#define PLL_SD_MASK             0x0003FC00
#define PLL_SD_SHIFT            10
#define SET_PLL_GO              0x1
#define PLL_TICOPWDN            BIT(16)
#define PLL_LDOPWDN             BIT(15)
#define PLL_LOCK                0x2
#define PLL_IDLE                0x1

/* Software rest for the SATA PLL (in CTRL_CORE_SMA_SW_0 register)*/
#define SATA_PLL_SOFT_RESET (1<<18)

/* PHY POWER CONTROL Register */
#define OMAP_CTRL_PIPE3_PHY_PWRCTL_CLK_CMD_MASK         0x003FC000
#define OMAP_CTRL_PIPE3_PHY_PWRCTL_CLK_CMD_SHIFT        0xE

#define OMAP_CTRL_PIPE3_PHY_PWRCTL_CLK_FREQ_MASK        0xFFC00000
#define OMAP_CTRL_PIPE3_PHY_PWRCTL_CLK_FREQ_SHIFT       0x16

#define OMAP_CTRL_PIPE3_PHY_TX_RX_POWERON       0x3
#define OMAP_CTRL_PIPE3_PHY_TX_RX_POWEROFF      0x0


#define PLL_IDLE_TIME   100     /* in milliseconds */
#define PLL_LOCK_TIME   100     /* in milliseconds */

struct omap_pipe3 {
	void __iomem		*pll_ctrl_base;
	void __iomem		*power_reg;
	void __iomem		*pll_reset_reg;
	struct pipe3_dpll_map	*dpll_map;
};


struct pipe3_dpll_params {
	u16     m;
	u8      n;
	u8      freq:3;
	u8      sd;
	u32     mf;
};

struct pipe3_dpll_map {
	unsigned long rate;
	struct pipe3_dpll_params params;
};

static inline u32 omap_pipe3_readl(void __iomem *addr, unsigned offset)
{
	return readl(addr + offset);
}

static inline void omap_pipe3_writel(void __iomem *addr, unsigned offset,
		u32 data)
{
	writel(data, addr + offset);
}

static struct pipe3_dpll_params *omap_pipe3_get_dpll_params(struct omap_pipe3
									*pipe3)
{
	u32 rate;
	struct pipe3_dpll_map *dpll_map = pipe3->dpll_map;

	rate = get_sys_clk_freq();

	for (; dpll_map->rate; dpll_map++) {
		if (rate == dpll_map->rate)
			return &dpll_map->params;
	}

	printf("%s: No DPLL configuration for %u Hz SYS CLK\n",
	       __func__, rate);
	return NULL;
}

static int omap_pipe3_wait_lock(struct omap_pipe3 *pipe3)
{
	u32 val;
	int timeout = PLL_LOCK_TIME;

	do {
		mdelay(1);
		val = omap_pipe3_readl(pipe3->pll_ctrl_base, PLL_STATUS);
		if (val & PLL_LOCK)
			break;
	} while (--timeout);

	if (!(val & PLL_LOCK)) {
		printf("%s: DPLL failed to lock\n", __func__);
		return -EBUSY;
	}

	return 0;
}

static int omap_pipe3_dpll_program(struct omap_pipe3 *pipe3)
{
	u32                     val;
	struct pipe3_dpll_params *dpll_params;

	dpll_params = omap_pipe3_get_dpll_params(pipe3);
	if (!dpll_params) {
		printf("%s: Invalid DPLL parameters\n", __func__);
		return -EINVAL;
	}

	val = omap_pipe3_readl(pipe3->pll_ctrl_base, PLL_CONFIGURATION1);
	val &= ~PLL_REGN_MASK;
	val |= dpll_params->n << PLL_REGN_SHIFT;
	omap_pipe3_writel(pipe3->pll_ctrl_base, PLL_CONFIGURATION1, val);

	val = omap_pipe3_readl(pipe3->pll_ctrl_base, PLL_CONFIGURATION2);
	val &= ~(PLL_SELFREQDCO_MASK | PLL_IDLE);
	val |= dpll_params->freq << PLL_SELFREQDCO_SHIFT;
	omap_pipe3_writel(pipe3->pll_ctrl_base, PLL_CONFIGURATION2, val);

	val = omap_pipe3_readl(pipe3->pll_ctrl_base, PLL_CONFIGURATION1);
	val &= ~PLL_REGM_MASK;
	val |= dpll_params->m << PLL_REGM_SHIFT;
	omap_pipe3_writel(pipe3->pll_ctrl_base, PLL_CONFIGURATION1, val);

	val = omap_pipe3_readl(pipe3->pll_ctrl_base, PLL_CONFIGURATION4);
	val &= ~PLL_REGM_F_MASK;
	val |= dpll_params->mf << PLL_REGM_F_SHIFT;
	omap_pipe3_writel(pipe3->pll_ctrl_base, PLL_CONFIGURATION4, val);

	val = omap_pipe3_readl(pipe3->pll_ctrl_base, PLL_CONFIGURATION3);
	val &= ~PLL_SD_MASK;
	val |= dpll_params->sd << PLL_SD_SHIFT;
	omap_pipe3_writel(pipe3->pll_ctrl_base, PLL_CONFIGURATION3, val);

	omap_pipe3_writel(pipe3->pll_ctrl_base, PLL_GO, SET_PLL_GO);

	return omap_pipe3_wait_lock(pipe3);
}

static void omap_control_pipe3_power(struct omap_pipe3 *pipe3, int on)
{
	u32 val, rate;

	val = readl(pipe3->power_reg);

	rate = get_sys_clk_freq();
	rate = rate/1000000;

	if (on) {
		val &= ~(OMAP_CTRL_PIPE3_PHY_PWRCTL_CLK_CMD_MASK |
				OMAP_CTRL_PIPE3_PHY_PWRCTL_CLK_FREQ_MASK);
		val |= OMAP_CTRL_PIPE3_PHY_TX_RX_POWERON <<
			OMAP_CTRL_PIPE3_PHY_PWRCTL_CLK_CMD_SHIFT;
		val |= rate <<
			OMAP_CTRL_PIPE3_PHY_PWRCTL_CLK_FREQ_SHIFT;
	} else {
		val &= ~OMAP_CTRL_PIPE3_PHY_PWRCTL_CLK_CMD_MASK;
		val |= OMAP_CTRL_PIPE3_PHY_TX_RX_POWEROFF <<
			OMAP_CTRL_PIPE3_PHY_PWRCTL_CLK_CMD_SHIFT;
	}

	writel(val, pipe3->power_reg);
}

static int pipe3_init(struct phy *phy)
{
	int ret;
	u32 val;
	struct omap_pipe3 *pipe3 = dev_get_priv(phy->dev);

	/* Program the DPLL only if not locked */
	val = omap_pipe3_readl(pipe3->pll_ctrl_base, PLL_STATUS);
	if (!(val & PLL_LOCK)) {
		ret = omap_pipe3_dpll_program(pipe3);
		if (ret)
			return ret;
	} else {
		/* else just bring it out of IDLE mode */
		val = omap_pipe3_readl(pipe3->pll_ctrl_base,
				       PLL_CONFIGURATION2);
		if (val & PLL_IDLE) {
			val &= ~PLL_IDLE;
			omap_pipe3_writel(pipe3->pll_ctrl_base,
					  PLL_CONFIGURATION2, val);
			ret = omap_pipe3_wait_lock(pipe3);
			if (ret)
				return ret;
		}
	}
	return 0;
}

static int pipe3_power_on(struct phy *phy)
{
	struct omap_pipe3 *pipe3 = dev_get_priv(phy->dev);

	/* Power up the PHY */
	omap_control_pipe3_power(pipe3, 1);

	return 0;
}

static int pipe3_power_off(struct phy *phy)
{
	struct omap_pipe3 *pipe3 = dev_get_priv(phy->dev);

	/* Power down the PHY */
	omap_control_pipe3_power(pipe3, 0);

	return 0;
}

static int pipe3_exit(struct phy *phy)
{
	u32 val;
	int timeout = PLL_IDLE_TIME;
	struct omap_pipe3 *pipe3 = dev_get_priv(phy->dev);

	pipe3_power_off(phy);

	/* Put DPLL in IDLE mode */
	val = omap_pipe3_readl(pipe3->pll_ctrl_base, PLL_CONFIGURATION2);
	val |= PLL_IDLE;
	omap_pipe3_writel(pipe3->pll_ctrl_base, PLL_CONFIGURATION2, val);

	/* wait for LDO and Oscillator to power down */
	do {
		mdelay(1);
		val = omap_pipe3_readl(pipe3->pll_ctrl_base, PLL_STATUS);
		if ((val & PLL_TICOPWDN) && (val & PLL_LDOPWDN))
			break;
	} while (--timeout);

	if (!(val & PLL_TICOPWDN) || !(val & PLL_LDOPWDN)) {
		pr_err("%s: Failed to power down DPLL: PLL_STATUS 0x%x\n",
		      __func__, val);
		return -EBUSY;
	}

	if (pipe3->pll_reset_reg) {
		val = readl(pipe3->pll_reset_reg);
		writel(val | SATA_PLL_SOFT_RESET, pipe3->pll_reset_reg);
		mdelay(1);
		writel(val & ~SATA_PLL_SOFT_RESET, pipe3->pll_reset_reg);
	}

	return 0;
}

static void *get_reg(struct udevice *dev, const char *name)
{
	struct udevice *syscon;
	struct regmap *regmap;
	const fdt32_t *cell;
	int len, err;
	void *base;

	err = uclass_get_device_by_phandle(UCLASS_SYSCON, dev,
					   name, &syscon);
	if (err) {
		pr_err("unable to find syscon device for %s (%d)\n",
		      name, err);
		return NULL;
	}

	regmap = syscon_get_regmap(syscon);
	if (IS_ERR(regmap)) {
		pr_err("unable to find regmap for %s (%ld)\n",
		      name, PTR_ERR(regmap));
		return NULL;
	}

	cell = fdt_getprop(gd->fdt_blob, dev_of_offset(dev), name,
			   &len);
	if (len < 2*sizeof(fdt32_t)) {
		pr_err("offset not available for %s\n", name);
		return NULL;
	}

	base = regmap_get_range(regmap, 0);
	if (!base)
		return NULL;

	return fdtdec_get_number(cell + 1, 1) + base;
}

static int pipe3_phy_probe(struct udevice *dev)
{
	fdt_addr_t addr;
	fdt_size_t sz;
	struct omap_pipe3 *pipe3 = dev_get_priv(dev);

	addr = devfdt_get_addr_size_index(dev, 2, &sz);
	if (addr == FDT_ADDR_T_NONE) {
		pr_err("missing pll ctrl address\n");
		return -EINVAL;
	}

	pipe3->pll_ctrl_base = map_physmem(addr, sz, MAP_NOCACHE);
	if (!pipe3->pll_ctrl_base) {
		pr_err("unable to remap pll ctrl\n");
		return -EINVAL;
	}

	pipe3->power_reg = get_reg(dev, "syscon-phy-power");
	if (!pipe3->power_reg)
		return -EINVAL;

	if (device_is_compatible(dev, "ti,phy-pipe3-sata")) {
		pipe3->pll_reset_reg = get_reg(dev, "syscon-pllreset");
		if (!pipe3->pll_reset_reg)
			return -EINVAL;
	}

	pipe3->dpll_map = (struct pipe3_dpll_map *)dev_get_driver_data(dev);

	return 0;
}

static struct pipe3_dpll_map dpll_map_sata[] = {
	{12000000, {1000, 7, 4, 6, 0} },        /* 12 MHz */
	{16800000, {714, 7, 4, 6, 0} },         /* 16.8 MHz */
	{19200000, {625, 7, 4, 6, 0} },         /* 19.2 MHz */
	{20000000, {600, 7, 4, 6, 0} },         /* 20 MHz */
	{26000000, {461, 7, 4, 6, 0} },         /* 26 MHz */
	{38400000, {312, 7, 4, 6, 0} },         /* 38.4 MHz */
	{ },                                    /* Terminator */
};

static struct pipe3_dpll_map dpll_map_usb[] = {
	{12000000, {1250, 5, 4, 20, 0} },	/* 12 MHz */
	{16800000, {3125, 20, 4, 20, 0} },	/* 16.8 MHz */
	{19200000, {1172, 8, 4, 20, 65537} },	/* 19.2 MHz */
	{20000000, {1000, 7, 4, 10, 0} },	/* 20 MHz */
	{26000000, {1250, 12, 4, 20, 0} },	/* 26 MHz */
	{38400000, {3125, 47, 4, 20, 92843} },	/* 38.4 MHz */
	{ },					/* Terminator */
};

static const struct udevice_id pipe3_phy_ids[] = {
	{ .compatible = "ti,phy-pipe3-sata", .data = (ulong)&dpll_map_sata },
	{ .compatible = "ti,omap-usb3", .data = (ulong)&dpll_map_usb},
	{ }
};

static struct phy_ops pipe3_phy_ops = {
	.init = pipe3_init,
	.power_on = pipe3_power_on,
	.power_off = pipe3_power_off,
	.exit = pipe3_exit,
};

U_BOOT_DRIVER(pipe3_phy) = {
	.name	= "pipe3_phy",
	.id	= UCLASS_PHY,
	.of_match = pipe3_phy_ids,
	.ops = &pipe3_phy_ops,
	.probe = pipe3_phy_probe,
	.priv_auto_alloc_size = sizeof(struct omap_pipe3),
};
