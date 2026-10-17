// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016 Marvell International Ltd.
 * https://spdx.org/licenses
 */

#include <common.h>
#include <config.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <log.h>
#include <asm/global_data.h>
#include <dm/pinctrl.h>
#include <dm/root.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/arch-armada8k/soc-info.h>
#include <linux/bitops.h>
#include "pinctrl-mvebu.h"

#define AP_EMMC_PHY_CTRL_REG		0x100
#define CP_EMMC_PHY_CTRL_REG		0x424
#define EMMC_PHY_CTRL_SDPHY_EN		BIT(0)

#define AP806_EMMC_CLK_PIN_ID		0
#define AP806_EMMC_CLK_FUNC		0x1
#define CP110_EMMC_CLK_PIN_ID		56
#define CP110_EMMC_CLK_FUNC		0xe

DECLARE_GLOBAL_DATA_PTR;

/* mvebu_pinctl_emmc_set_mux: configure sd/mmc PHY mux
 * To enable SDIO/eMMC in Armada-APN806/CP110, need to configure PHY mux.
 * eMMC/SD PHY register responsible for muxing between MPPs and SD/eMMC
 * controller:
 * - Bit0 enabled SDIO/eMMC PHY is used as a MPP muxltiplexer,
 * - Bit0 disabled SDIO/eMMC PHY is connected to SDIO/eMMC controller
 * If pin function is set to eMMC/SD, then configure the eMMC/SD PHY
 * muxltiplexer register to be on SDIO/eMMC controller
 */
void mvebu_pinctl_emmc_set_mux(struct udevice *dev, u32 pin, u32 func)
{
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(dev);
	struct mvebu_pinctrl_priv *priv = dev_get_priv(dev);

	if (!fdt_node_check_compatible(blob, node, "marvell,ap806-pinctrl")) {
		if ((pin == AP806_EMMC_CLK_PIN_ID) &&
		    (func == AP806_EMMC_CLK_FUNC)) {
			clrbits_le32(priv->base_reg + AP_EMMC_PHY_CTRL_REG,
				     EMMC_PHY_CTRL_SDPHY_EN);
		}
	} else if (!fdt_node_check_compatible(blob, node,
					"marvell,armada-8k-cpm-pinctrl") ||
		   !fdt_node_check_compatible(blob, node,
					"marvell,armada-7k-pinctrl")) {
		if ((pin == CP110_EMMC_CLK_PIN_ID) &&
		    (func == CP110_EMMC_CLK_FUNC)) {
			clrbits_le32(priv->base_reg + CP_EMMC_PHY_CTRL_REG,
				     EMMC_PHY_CTRL_SDPHY_EN);
		}
	}
}

/*
 * mvebu_pinctrl_set_state: configure pin functions.
 * @dev: the pinctrl device to be configured.
 * @config: the state to be configured.
 * @return: 0 in success
 */
int mvebu_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(config);
	struct mvebu_pinctrl_priv *priv;
	u32 pin_arr[MVEBU_MAX_PINS_PER_BANK];
	u32 function;
	int i, pin_count;

	priv = dev_get_priv(dev);

	pin_count = fdtdec_get_int_array_count(blob, node,
					       "marvell,pins",
					       pin_arr,
					       MVEBU_MAX_PINS_PER_BANK);
	if (pin_count <= 0) {
		debug("Failed reading pins array for pinconfig %s (%d)\n",
		      config->name, pin_count);
		return -EINVAL;
	}

	function = fdtdec_get_int(blob, node, "marvell,function", 0xff);

	/*
	 * Check if setup of PHY mux is needed for this pins group.
	 * Only the first pin id in array is tested, all the rest use the same
	 * pin function.
	 */
	mvebu_pinctl_emmc_set_mux(dev, pin_arr[0], function);

	for (i = 0; i < pin_count; i++) {
		int reg_offset;
		int field_offset;
		int pin = pin_arr[i];

		if (function > priv->max_func) {
			debug("Illegal function %d for pinconfig %s\n",
			      function, config->name);
			return -EINVAL;
		}

		/* Calculate register address and bit in register */
		reg_offset   = priv->reg_direction * 4 *
					(pin >> (PIN_REG_SHIFT));
		field_offset = (BITS_PER_PIN) * (pin & PIN_FIELD_MASK);

		clrsetbits_le32(priv->base_reg + reg_offset,
				PIN_FUNC_MASK << field_offset,
				(function & PIN_FUNC_MASK) << field_offset);
	}

	return 0;
}

/*
 * mvebu_pinctrl_set_state_all: configure the entire bank pin functions.
 * @dev: the pinctrl device to be configured.
 * @config: the state to be configured.
 * @return: 0 in success
 */
static int mvebu_pinctrl_set_state_all(struct udevice *dev,
				       struct udevice *config)
{
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(config);
	struct mvebu_pinctrl_priv *priv;
	u32 func_arr[MVEBU_MAX_PINS_PER_BANK];
	int pin, err;

	priv = dev_get_priv(dev);

	err = fdtdec_get_int_array(blob, node, "pin-func",
				   func_arr, priv->pin_cnt);
	if (err) {
		debug("Failed reading pin functions for bank %s\n",
		      priv->bank_name);
		return -EINVAL;
	}

	/* Check if setup of PHY mux is needed for this pins group. */
	if (priv->pin_cnt < CP110_EMMC_CLK_PIN_ID)
		mvebu_pinctl_emmc_set_mux(dev, AP806_EMMC_CLK_PIN_ID,
					  func_arr[AP806_EMMC_CLK_PIN_ID]);
	else
		mvebu_pinctl_emmc_set_mux(dev, CP110_EMMC_CLK_PIN_ID,
					  func_arr[CP110_EMMC_CLK_PIN_ID]);

	for (pin = 0; pin < priv->pin_cnt; pin++) {
		int reg_offset;
		int field_offset;
		u32 func = func_arr[pin];

		/* Bypass pins with function 0xFF */
		if (func == 0xff) {
			debug("Warning: pin %d value is not modified ", pin);
			debug("(kept as default)\n");
			continue;
		} else if (func > priv->max_func) {
			debug("Illegal function %d for pin %d\n", func, pin);
			return -EINVAL;
		}

		/* Calculate register address and bit in register */
		reg_offset   = priv->reg_direction * 4 *
					(pin >> (PIN_REG_SHIFT));
		field_offset = (BITS_PER_PIN) * (pin & PIN_FIELD_MASK);

		clrsetbits_le32(priv->base_reg + reg_offset,
				PIN_FUNC_MASK << field_offset,
				(func & PIN_FUNC_MASK) << field_offset);
	}

	return 0;
}

int mvebu_pinctl_probe(struct udevice *dev)
{
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(dev);
	struct mvebu_pinctrl_priv *priv;

	priv = dev_get_priv(dev);
	if (!priv) {
		debug("%s: Failed to get private\n", __func__);
		return -EINVAL;
	}

	priv->base_reg = dev_read_addr_ptr(dev);
	if (!priv->base_reg) {
		debug("%s: Failed to get base address\n", __func__);
		return -EINVAL;
	}

	priv->pin_cnt   = fdtdec_get_int(blob, node, "pin-count",
					MVEBU_MAX_PINS_PER_BANK);
	priv->max_func  = fdtdec_get_int(blob, node, "max-func",
					 MVEBU_MAX_FUNC);
	priv->bank_name = fdt_getprop(blob, node, "bank-name", NULL);

	priv->reg_direction = 1;
	if (fdtdec_get_bool(blob, node, "reverse-reg"))
		priv->reg_direction = -1;

	return mvebu_pinctrl_set_state_all(dev, dev);
}

static struct pinctrl_ops mvebu_pinctrl_ops = {
	.set_state	= mvebu_pinctrl_set_state
};

static const struct udevice_id mvebu_pinctrl_ids[] = {
	{ .compatible = "marvell,mvebu-pinctrl" },
	{ .compatible = "marvell,ap806-pinctrl" },
	{ .compatible = "marvell,armada-7k-pinctrl" },
	{ .compatible = "marvell,armada-8k-cpm-pinctrl" },
	{ .compatible = "marvell,armada-8k-cps-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_mvebu) = {
	.name		= "mvebu_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= mvebu_pinctrl_ids,
	.priv_auto	= sizeof(struct mvebu_pinctrl_priv),
	.ops		= &mvebu_pinctrl_ops,
	.probe		= mvebu_pinctl_probe
};
