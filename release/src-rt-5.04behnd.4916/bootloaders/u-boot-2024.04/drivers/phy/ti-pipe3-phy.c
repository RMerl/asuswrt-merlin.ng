// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Texas Instruments Incorporated - https://www.ti.com/
 * Written by Jean-Jacques Hiblot  <jjhiblot@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/device.h>
#include <generic-phy.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <syscon.h>
#include <regmap.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/printk.h>

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
#define PIPE3_PHY_PWRCTL_CLK_CMD_MASK	GENMASK(21, 14)
#define PIPE3_PHY_PWRCTL_CLK_CMD_SHIFT	14

#define PIPE3_PHY_PWRCTL_CLK_FREQ_MASK	GENMASK(31, 22)
#define PIPE3_PHY_PWRCTL_CLK_FREQ_SHIFT	22

#define PIPE3_PHY_RX_POWERON       (0x1 << PIPE3_PHY_PWRCTL_CLK_CMD_SHIFT)
#define PIPE3_PHY_TX_POWERON       (0x2 << PIPE3_PHY_PWRCTL_CLK_CMD_SHIFT)

/* PHY RX Registers */
#define PIPE3_PHY_RX_ANA_PROGRAMMABILITY	0x0000000C
#define INTERFACE_MASK			GENMASK(31, 27)
#define INTERFACE_SHIFT			27
#define INTERFACE_MODE_USBSS		BIT(4)
#define INTERFACE_MODE_SATA_1P5		BIT(3)
#define INTERFACE_MODE_SATA_3P0		BIT(2)
#define INTERFACE_MODE_PCIE		BIT(0)

#define LOSD_MASK			GENMASK(17, 14)
#define LOSD_SHIFT			14
#define MEM_PLLDIV			GENMASK(6, 5)

#define PIPE3_PHY_RX_TRIM		0x0000001C
#define MEM_DLL_TRIM_SEL_MASK		GENMASK(31, 30)
#define MEM_DLL_TRIM_SHIFT		30

#define PIPE3_PHY_RX_DLL		0x00000024
#define MEM_DLL_PHINT_RATE_MASK		GENMASK(31, 30)
#define MEM_DLL_PHINT_RATE_SHIFT	30

#define PIPE3_PHY_RX_DIGITAL_MODES		0x00000028
#define MEM_HS_RATE_MASK		GENMASK(28, 27)
#define MEM_HS_RATE_SHIFT		27
#define MEM_OVRD_HS_RATE		BIT(26)
#define MEM_OVRD_HS_RATE_SHIFT		26
#define MEM_CDR_FASTLOCK		BIT(23)
#define MEM_CDR_FASTLOCK_SHIFT		23
#define MEM_CDR_LBW_MASK		GENMASK(22, 21)
#define MEM_CDR_LBW_SHIFT		21
#define MEM_CDR_STEPCNT_MASK		GENMASK(20, 19)
#define MEM_CDR_STEPCNT_SHIFT		19
#define MEM_CDR_STL_MASK		GENMASK(18, 16)
#define MEM_CDR_STL_SHIFT		16
#define MEM_CDR_THR_MASK		GENMASK(15, 13)
#define MEM_CDR_THR_SHIFT		13
#define MEM_CDR_THR_MODE		BIT(12)
#define MEM_CDR_THR_MODE_SHIFT		12
#define MEM_CDR_2NDO_SDM_MODE		BIT(11)
#define MEM_CDR_2NDO_SDM_MODE_SHIFT	11

#define PIPE3_PHY_RX_EQUALIZER		0x00000038
#define MEM_EQLEV_MASK			GENMASK(31, 16)
#define MEM_EQLEV_SHIFT			16
#define MEM_EQFTC_MASK			GENMASK(15, 11)
#define MEM_EQFTC_SHIFT			11
#define MEM_EQCTL_MASK			GENMASK(10, 7)
#define MEM_EQCTL_SHIFT			7
#define MEM_OVRD_EQLEV			BIT(2)
#define MEM_OVRD_EQLEV_SHIFT		2
#define MEM_OVRD_EQFTC			BIT(1)
#define MEM_OVRD_EQFTC_SHIFT		1

#define SATA_PHY_RX_IO_AND_A2D_OVERRIDES	0x44
#define MEM_CDR_LOS_SOURCE_MASK		GENMASK(10, 9)
#define MEM_CDR_LOS_SOURCE_SHIFT	9

#define PLL_IDLE_TIME   100     /* in milliseconds */
#define PLL_LOCK_TIME   100     /* in milliseconds */

enum pipe3_mode { PIPE3_MODE_PCIE = 1,
		  PIPE3_MODE_SATA,
		  PIPE3_MODE_USBSS };

struct pipe3_settings {
	u8 ana_interface;
	u8 ana_losd;
	u8 dig_fastlock;
	u8 dig_lbw;
	u8 dig_stepcnt;
	u8 dig_stl;
	u8 dig_thr;
	u8 dig_thr_mode;
	u8 dig_2ndo_sdm_mode;
	u8 dig_hs_rate;
	u8 dig_ovrd_hs_rate;
	u8 dll_trim_sel;
	u8 dll_phint_rate;
	u8 eq_lev;
	u8 eq_ftc;
	u8 eq_ctl;
	u8 eq_ovrd_lev;
	u8 eq_ovrd_ftc;
};

struct omap_pipe3 {
	void __iomem		*pll_ctrl_base;
	void __iomem		*phy_rx;
	void __iomem		*power_reg;
	void __iomem		*pll_reset_reg;
	struct pipe3_dpll_map	*dpll_map;
	enum pipe3_mode		mode;
	struct pipe3_settings	settings;
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

struct pipe3_data {
	enum pipe3_mode mode;
	struct pipe3_dpll_map *dpll_map;
	struct pipe3_settings settings;
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
		val &= ~(PIPE3_PHY_PWRCTL_CLK_CMD_MASK |
			 PIPE3_PHY_PWRCTL_CLK_FREQ_MASK);
		val |= rate << PIPE3_PHY_PWRCTL_CLK_FREQ_SHIFT;
		writel(val, pipe3->power_reg);

		/* Power up TX before RX for SATA & USB */
		val |= PIPE3_PHY_TX_POWERON;
		writel(val, pipe3->power_reg);

		val |= PIPE3_PHY_RX_POWERON;
		writel(val, pipe3->power_reg);
	} else {
		val &= ~PIPE3_PHY_PWRCTL_CLK_CMD_MASK;
		writel(val, pipe3->power_reg);
	}
}

static void ti_pipe3_calibrate(struct omap_pipe3 *phy)
{
	u32 val;
	struct pipe3_settings *s = &phy->settings;

	val = omap_pipe3_readl(phy->phy_rx, PIPE3_PHY_RX_ANA_PROGRAMMABILITY);
	val &= ~(INTERFACE_MASK | LOSD_MASK | MEM_PLLDIV);
	val = (s->ana_interface << INTERFACE_SHIFT | s->ana_losd << LOSD_SHIFT);
	omap_pipe3_writel(phy->phy_rx, PIPE3_PHY_RX_ANA_PROGRAMMABILITY, val);

	val = omap_pipe3_readl(phy->phy_rx, PIPE3_PHY_RX_DIGITAL_MODES);
	val &= ~(MEM_HS_RATE_MASK | MEM_OVRD_HS_RATE | MEM_CDR_FASTLOCK |
		 MEM_CDR_LBW_MASK | MEM_CDR_STEPCNT_MASK | MEM_CDR_STL_MASK |
		 MEM_CDR_THR_MASK | MEM_CDR_THR_MODE | MEM_CDR_2NDO_SDM_MODE);
	val |= s->dig_hs_rate << MEM_HS_RATE_SHIFT |
		s->dig_ovrd_hs_rate << MEM_OVRD_HS_RATE_SHIFT |
		s->dig_fastlock << MEM_CDR_FASTLOCK_SHIFT |
		s->dig_lbw << MEM_CDR_LBW_SHIFT |
		s->dig_stepcnt << MEM_CDR_STEPCNT_SHIFT |
		s->dig_stl << MEM_CDR_STL_SHIFT |
		s->dig_thr << MEM_CDR_THR_SHIFT |
		s->dig_thr_mode << MEM_CDR_THR_MODE_SHIFT |
		s->dig_2ndo_sdm_mode << MEM_CDR_2NDO_SDM_MODE_SHIFT;
	omap_pipe3_writel(phy->phy_rx, PIPE3_PHY_RX_DIGITAL_MODES, val);

	val = omap_pipe3_readl(phy->phy_rx, PIPE3_PHY_RX_TRIM);
	val &= ~MEM_DLL_TRIM_SEL_MASK;
	val |= s->dll_trim_sel << MEM_DLL_TRIM_SHIFT;
	omap_pipe3_writel(phy->phy_rx, PIPE3_PHY_RX_TRIM, val);

	val = omap_pipe3_readl(phy->phy_rx, PIPE3_PHY_RX_DLL);
	val &= ~MEM_DLL_PHINT_RATE_MASK;
	val |= s->dll_phint_rate << MEM_DLL_PHINT_RATE_SHIFT;
	omap_pipe3_writel(phy->phy_rx, PIPE3_PHY_RX_DLL, val);

	val = omap_pipe3_readl(phy->phy_rx, PIPE3_PHY_RX_EQUALIZER);
	val &= ~(MEM_EQLEV_MASK | MEM_EQFTC_MASK | MEM_EQCTL_MASK |
		 MEM_OVRD_EQLEV | MEM_OVRD_EQFTC);
	val |= s->eq_lev << MEM_EQLEV_SHIFT |
		s->eq_ftc << MEM_EQFTC_SHIFT |
		s->eq_ctl << MEM_EQCTL_SHIFT |
		s->eq_ovrd_lev << MEM_OVRD_EQLEV_SHIFT |
		s->eq_ovrd_ftc << MEM_OVRD_EQFTC_SHIFT;
	omap_pipe3_writel(phy->phy_rx, PIPE3_PHY_RX_EQUALIZER, val);

	if (phy->mode == PIPE3_MODE_SATA) {
		val = omap_pipe3_readl(phy->phy_rx,
				       SATA_PHY_RX_IO_AND_A2D_OVERRIDES);
		val &= ~MEM_CDR_LOS_SOURCE_MASK;
		omap_pipe3_writel(phy->phy_rx, SATA_PHY_RX_IO_AND_A2D_OVERRIDES,
				  val);
	}
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

		ti_pipe3_calibrate(pipe3);
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
	struct pipe3_data *data;

	/* PHY_RX */
	addr = devfdt_get_addr_size_index(dev, 0, &sz);
	if (addr == FDT_ADDR_T_NONE) {
		pr_err("missing phy_rx address\n");
		return -EINVAL;
	}

	pipe3->phy_rx = map_physmem(addr, sz, MAP_NOCACHE);
	if (!pipe3->phy_rx) {
		pr_err("unable to remap phy_rx\n");
		return -EINVAL;
	}

	/* PLLCTRL */
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

	data = (struct pipe3_data *)dev_get_driver_data(dev);
	pipe3->mode = data->mode;
	pipe3->dpll_map = data->dpll_map;
	pipe3->settings = data->settings;

	if (pipe3->mode == PIPE3_MODE_SATA) {
		pipe3->pll_reset_reg = get_reg(dev, "syscon-pllreset");
		if (!pipe3->pll_reset_reg)
			return -EINVAL;
	}

	return 0;
}

static struct pipe3_dpll_map dpll_map_sata[] = {
	{12000000, {625, 4, 4, 6, 0} },	/* 12 MHz */
	{16800000, {625, 6, 4, 7, 0} },		/* 16.8 MHz */
	{19200000, {625, 7, 4, 6, 0} },		/* 19.2 MHz */
	{20000000, {750, 9, 4, 6, 0} },		/* 20 MHz */
	{26000000, {750, 12, 4, 6, 0} },	/* 26 MHz */
	{38400000, {625, 15, 4, 6, 0} },	/* 38.4 MHz */
	{ },					/* Terminator */
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

static struct pipe3_data data_usb = {
	.mode = PIPE3_MODE_USBSS,
	.dpll_map = dpll_map_usb,
	.settings = {
	/* DRA75x TRM Table 26-17. Preferred USB3_PHY_RX SCP Register Settings */
		.ana_interface = INTERFACE_MODE_USBSS,
		.ana_losd = 0xa,
		.dig_fastlock = 1,
		.dig_lbw = 3,
		.dig_stepcnt = 0,
		.dig_stl = 0x3,
		.dig_thr = 1,
		.dig_thr_mode = 1,
		.dig_2ndo_sdm_mode = 0,
		.dig_hs_rate = 0,
		.dig_ovrd_hs_rate = 1,
		.dll_trim_sel = 0x2,
		.dll_phint_rate = 0x3,
		.eq_lev = 0,
		.eq_ftc = 0,
		.eq_ctl = 0x9,
		.eq_ovrd_lev = 0,
		.eq_ovrd_ftc = 0,
	},
};

static struct pipe3_data data_sata = {
	.mode = PIPE3_MODE_SATA,
	.dpll_map = dpll_map_sata,
	.settings = {
	/* DRA75x TRM Table 26-9. Preferred SATA_PHY_RX SCP Register Settings */
		.ana_interface = INTERFACE_MODE_SATA_3P0,
		.ana_losd = 0x5,
		.dig_fastlock = 1,
		.dig_lbw = 3,
		.dig_stepcnt = 0,
		.dig_stl = 0x3,
		.dig_thr = 1,
		.dig_thr_mode = 1,
		.dig_2ndo_sdm_mode = 0,
		.dig_hs_rate = 0,	/* Not in TRM preferred settings */
		.dig_ovrd_hs_rate = 0,	/* Not in TRM preferred settings */
		.dll_trim_sel = 0x1,
		.dll_phint_rate = 0x2,	/* for 1.5 GHz DPLL clock */
		.eq_lev = 0,
		.eq_ftc = 0x1f,
		.eq_ctl = 0,
		.eq_ovrd_lev = 1,
		.eq_ovrd_ftc = 1,
	},
};

static const struct udevice_id pipe3_phy_ids[] = {
	{ .compatible = "ti,phy-pipe3-sata", .data = (ulong)&data_sata },
	{ .compatible = "ti,omap-usb3", .data = (ulong)&data_usb},
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
	.priv_auto	= sizeof(struct omap_pipe3),
};
