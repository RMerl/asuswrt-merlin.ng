// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dm/lists.h>
#include <dt-bindings/clk/mpc83xx-clk.h>
#include <asm/arch/soc.h>

#include "mpc83xx_clk.h"

DECLARE_GLOBAL_DATA_PTR;

/**
 * struct mpc83xx_clk_priv - Private data structure for the MPC83xx clock
 *			     driver
 * @speed: Array containing the speed values of all system clocks (initialized
 *	   once, then only read back)
 */
struct mpc83xx_clk_priv {
	u32 speed[MPC83XX_CLK_COUNT];
};

/**
 * is_clk_valid() - Check if clock ID is valid for given clock device
 * @clk: The clock device for which to check a clock ID
 * @id:  The clock ID to check
 *
 * Return: true if clock ID is valid for clock device, false if not
 */
static inline bool is_clk_valid(struct udevice *clk, int id)
{
	ulong type = dev_get_driver_data(clk);

	switch (id) {
	case MPC83XX_CLK_MEM:
		return true;
	case MPC83XX_CLK_MEM_SEC:
		return type == SOC_MPC8360;
	case MPC83XX_CLK_ENC:
		return (type == SOC_MPC8308) || (type == SOC_MPC8309);
	case MPC83XX_CLK_I2C1:
		return true;
	case MPC83XX_CLK_TDM:
		return type == SOC_MPC8315;
	case MPC83XX_CLK_SDHC:
		return mpc83xx_has_sdhc(type);
	case MPC83XX_CLK_TSEC1:
	case MPC83XX_CLK_TSEC2:
		return mpc83xx_has_tsec(type);
	case MPC83XX_CLK_USBDR:
		return type == SOC_MPC8360;
	case MPC83XX_CLK_USBMPH:
		return type == SOC_MPC8349;
	case MPC83XX_CLK_PCIEXP1:
		return mpc83xx_has_pcie1(type);
	case MPC83XX_CLK_PCIEXP2:
		return mpc83xx_has_pcie2(type);
	case MPC83XX_CLK_SATA:
		return mpc83xx_has_sata(type);
	case MPC83XX_CLK_DMAC:
		return (type == SOC_MPC8308) || (type == SOC_MPC8309);
	case MPC83XX_CLK_PCI:
		return mpc83xx_has_pci(type);
	case MPC83XX_CLK_CSB:
		return true;
	case MPC83XX_CLK_I2C2:
		return mpc83xx_has_second_i2c(type);
	case MPC83XX_CLK_QE:
	case MPC83XX_CLK_BRG:
		return mpc83xx_has_quicc_engine(type) && (type != SOC_MPC8309);
	case MPC83XX_CLK_LCLK:
	case MPC83XX_CLK_LBIU:
	case MPC83XX_CLK_CORE:
		return true;
	}

	return false;
}

/**
 * init_single_clk() - Initialize a clock with a given ID
 * @dev: The clock device for which to initialize the clock
 * @clk: The clock ID
 *
 * The clock speed is read from the hardware's registers, and stored in the
 * private data structure of the driver. From there it is only retrieved, and
 * not set.
 *
 * Return: 0 if OK, -ve on error
 */
static int init_single_clk(struct udevice *dev, int clk)
{
	struct mpc83xx_clk_priv *priv = dev_get_priv(dev);
	immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	ulong type = dev_get_driver_data(dev);
	struct clk_mode mode;
	ulong mask;
	u32 csb_clk = get_csb_clk(im);
	int ret;

	ret = retrieve_mode(clk, type, &mode);
	if (ret) {
		debug("%s: Could not retrieve mode for clk %d (ret = %d)\n",
		      dev->name, clk, ret);
		return ret;
	}

	if (mode.type == TYPE_INVALID) {
		debug("%s: clock %d invalid\n", dev->name, clk);
		return -EINVAL;
	}

	if (mode.type == TYPE_SCCR_STANDARD) {
		mask = GENMASK(31 - mode.low, 31 - mode.high);

		switch (sccr_field(im, mask)) {
		case 0:
			priv->speed[clk] = 0;
			break;
		case 1:
			priv->speed[clk] = csb_clk;
			break;
		case 2:
			priv->speed[clk] = csb_clk / 2;
			break;
		case 3:
			priv->speed[clk] = csb_clk / 3;
			break;
		default:
			priv->speed[clk] = 0;
		}

		return 0;
	}

	if (mode.type == TYPE_SPMR_DIRECT_MULTIPLY) {
		mask = GENMASK(31 - mode.low, 31 - mode.high);

		priv->speed[clk] = csb_clk * (1 + sccr_field(im, mask));
		return 0;
	}

	if (clk == MPC83XX_CLK_CSB || clk == MPC83XX_CLK_I2C2) {
		priv->speed[clk] = csb_clk; /* i2c-2 clk is equal to csb clk */
		return 0;
	}

	if (clk == MPC83XX_CLK_QE || clk == MPC83XX_CLK_BRG) {
		u32 pci_sync_in = get_pci_sync_in(im);
		u32 qepmf = spmr_field(im, SPMR_CEPMF);
		u32 qepdf = spmr_field(im, SPMR_CEPDF);
		u32 qe_clk = (pci_sync_in * qepmf) / (1 + qepdf);

		if (clk == MPC83XX_CLK_QE)
			priv->speed[clk] = qe_clk;
		else
			priv->speed[clk] = qe_clk / 2;

		return 0;
	}

	if (clk == MPC83XX_CLK_LCLK || clk == MPC83XX_CLK_LBIU) {
		u32 lbiu_clk = csb_clk *
			(1 + spmr_field(im, SPMR_LBIUCM));
		u32 clkdiv = lcrr_field(im, LCRR_CLKDIV);

		if (clk == MPC83XX_CLK_LBIU)
			priv->speed[clk] = lbiu_clk;

		switch (clkdiv) {
		case 2:
		case 4:
		case 8:
			priv->speed[clk] = lbiu_clk / clkdiv;
			break;
		default:
			/* unknown lcrr */
			priv->speed[clk] = 0;
		}

		return 0;
	}

	if (clk == MPC83XX_CLK_CORE) {
		u8 corepll = spmr_field(im, SPMR_COREPLL);
		u32 corecnf_tab_index = ((corepll & 0x1F) << 2) |
					((corepll & 0x60) >> 5);

		if (corecnf_tab_index > (ARRAY_SIZE(corecnf_tab))) {
			debug("%s: Core configuration index %02x too high; possible wrong value",
			      dev->name, corecnf_tab_index);
			return -EINVAL;
		}

		switch (corecnf_tab[corecnf_tab_index].core_csb_ratio) {
		case RAT_BYP:
		case RAT_1_TO_1:
			priv->speed[clk] = csb_clk;
			break;
		case RAT_1_5_TO_1:
			priv->speed[clk] = (3 * csb_clk) / 2;
			break;
		case RAT_2_TO_1:
			priv->speed[clk] = 2 * csb_clk;
			break;
		case RAT_2_5_TO_1:
			priv->speed[clk] = (5 * csb_clk) / 2;
			break;
		case RAT_3_TO_1:
			priv->speed[clk] = 3 * csb_clk;
			break;
		default:
			/* unknown core to csb ratio */
			priv->speed[clk] = 0;
		}

		return 0;
	}

	/* Unknown clk value -> error */
	debug("%s: clock %d invalid\n", dev->name, clk);
	return -EINVAL;
}

/**
 * init_all_clks() - Initialize all clocks of a clock device
 * @dev: The clock device whose clocks should be initialized
 *
 * Return: 0 if OK, -ve on error
 */
static inline int init_all_clks(struct udevice *dev)
{
	int i;

	for (i = 0; i < MPC83XX_CLK_COUNT; i++) {
		int ret;

		if (!is_clk_valid(dev, i))
			continue;

		ret = init_single_clk(dev, i);
		if (ret) {
			debug("%s: Failed to initialize %s clock\n",
			      dev->name, names[i]);
			return ret;
		}
	}

	return 0;
}

static int mpc83xx_clk_request(struct clk *clock)
{
	/* Reject requests of clocks that are not available */
	if (is_clk_valid(clock->dev, clock->id))
		return 0;
	else
		return -ENODEV;
}

static ulong mpc83xx_clk_get_rate(struct clk *clk)
{
	struct mpc83xx_clk_priv *priv = dev_get_priv(clk->dev);

	if (clk->id >= MPC83XX_CLK_COUNT) {
		debug("%s: clock index %lu invalid\n", __func__, clk->id);
		return 0;
	}

	return priv->speed[clk->id];
}

static int mpc83xx_clk_enable(struct clk *clk)
{
	/* MPC83xx clocks are always enabled */
	return 0;
}

int get_clocks(void)
{
	/* Empty implementation to keep the prototype in common.h happy */
	return 0;
}

int get_serial_clock(void)
{
	struct mpc83xx_clk_priv *priv;
	struct udevice *clk;
	int ret;

	ret = uclass_first_device_err(UCLASS_CLK, &clk);
	if (ret) {
		debug("%s: Could not get clock device\n", __func__);
		return ret;
	}

	priv = dev_get_priv(clk);

	return priv->speed[MPC83XX_CLK_CSB];
}

const struct clk_ops mpc83xx_clk_ops = {
	.request = mpc83xx_clk_request,
	.get_rate = mpc83xx_clk_get_rate,
	.enable = mpc83xx_clk_enable,
};

static const struct udevice_id mpc83xx_clk_match[] = {
	{ .compatible = "fsl,mpc8308-clk", .data = SOC_MPC8308 },
	{ .compatible = "fsl,mpc8309-clk", .data = SOC_MPC8309 },
	{ .compatible = "fsl,mpc8313-clk", .data = SOC_MPC8313 },
	{ .compatible = "fsl,mpc8315-clk", .data = SOC_MPC8315 },
	{ .compatible = "fsl,mpc832x-clk", .data = SOC_MPC832X },
	{ .compatible = "fsl,mpc8349-clk", .data = SOC_MPC8349 },
	{ .compatible = "fsl,mpc8360-clk", .data = SOC_MPC8360 },
	{ .compatible = "fsl,mpc8379-clk", .data = SOC_MPC8379 },
	{ /* sentinel */ }
};

static int mpc83xx_clk_probe(struct udevice *dev)
{
	struct mpc83xx_clk_priv *priv = dev_get_priv(dev);
	ulong type;
	int ret;

	ret = init_all_clks(dev);
	if (ret) {
		debug("%s: Could not initialize all clocks (ret = %d)\n",
		      dev->name, ret);
		return ret;
	}

	type = dev_get_driver_data(dev);

	if (mpc83xx_has_sdhc(type))
		gd->arch.sdhc_clk = priv->speed[MPC83XX_CLK_SDHC];

	gd->arch.core_clk = priv->speed[MPC83XX_CLK_CORE];
	gd->arch.i2c1_clk = priv->speed[MPC83XX_CLK_I2C1];
	if (mpc83xx_has_second_i2c(type))
		gd->arch.i2c2_clk = priv->speed[MPC83XX_CLK_I2C2];

	gd->mem_clk = priv->speed[MPC83XX_CLK_MEM];

	if (mpc83xx_has_pci(type))
		gd->pci_clk = priv->speed[MPC83XX_CLK_PCI];

	gd->cpu_clk = priv->speed[MPC83XX_CLK_CORE];
	gd->bus_clk = priv->speed[MPC83XX_CLK_CSB];

	return 0;
}

static int mpc83xx_clk_bind(struct udevice *dev)
{
	int ret;
	struct udevice *sys_child;

	/*
	 * Since there is no corresponding device tree entry, and since the
	 * clock driver has to be present in either case, bind the sysreset
	 * driver here.
	 */
	ret = device_bind_driver(dev, "mpc83xx_sysreset", "sysreset",
				 &sys_child);
	if (ret)
		debug("%s: No sysreset driver: ret=%d\n",
		      dev->name, ret);

	return 0;
}

U_BOOT_DRIVER(mpc83xx_clk) = {
	.name = "mpc83xx_clk",
	.id = UCLASS_CLK,
	.of_match = mpc83xx_clk_match,
	.ops = &mpc83xx_clk_ops,
	.probe = mpc83xx_clk_probe,
	.priv_auto_alloc_size	= sizeof(struct mpc83xx_clk_priv),
	.bind = mpc83xx_clk_bind,
};

static int do_clocks(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i;
	char buf[32];
	struct udevice *clk;
	int ret;
	struct mpc83xx_clk_priv *priv;

	ret = uclass_first_device_err(UCLASS_CLK, &clk);
	if (ret) {
		debug("%s: Could not get clock device\n", __func__);
		return ret;
	}

	for (i = 0; i < MPC83XX_CLK_COUNT; i++) {
		if (!is_clk_valid(clk, i))
			continue;

		priv = dev_get_priv(clk);

		printf("%s = %s MHz\n", names[i], strmhz(buf, priv->speed[i]));
	}

	return 0;
}

U_BOOT_CMD(clocks,	1,	1,	do_clocks,
	   "display values of SoC's clocks",
	   ""
);
