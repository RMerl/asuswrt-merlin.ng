// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Peng Fan <van.freenix@gmail.com>
 */

#include <common.h>
#include <mapmem.h>
#include <linux/io.h>
#include <linux/err.h>
#include <dm.h>
#include <dm/pinctrl.h>

#include "pinctrl-imx.h"

DECLARE_GLOBAL_DATA_PTR;

static int imx_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	struct imx_pinctrl_priv *priv = dev_get_priv(dev);
	struct imx_pinctrl_soc_info *info = priv->info;
	int node = dev_of_offset(config);
	const struct fdt_property *prop;
	u32 *pin_data;
	int npins, size, pin_size;
	int mux_reg, conf_reg, input_reg;
	u32 input_val, mux_mode, config_val;
	u32 mux_shift = info->mux_mask ? ffs(info->mux_mask) - 1 : 0;
	int i, j = 0;

	dev_dbg(dev, "%s: %s\n", __func__, config->name);

	if (info->flags & IMX8_USE_SCU)
		pin_size = SHARE_IMX8_PIN_SIZE;
	else if (info->flags & SHARE_MUX_CONF_REG)
		pin_size = SHARE_FSL_PIN_SIZE;
	else
		pin_size = FSL_PIN_SIZE;

	prop = fdt_getprop(gd->fdt_blob, node, "fsl,pins", &size);
	if (!prop) {
		dev_err(dev, "No fsl,pins property in node %s\n", config->name);
		return -EINVAL;
	}

	if (!size || size % pin_size) {
		dev_err(dev, "Invalid fsl,pins property in node %s\n",
			config->name);
		return -EINVAL;
	}

	pin_data = devm_kzalloc(dev, size, 0);
	if (!pin_data)
		return -ENOMEM;

	if (fdtdec_get_int_array(gd->fdt_blob, node, "fsl,pins",
				 pin_data, size >> 2)) {
		dev_err(dev, "Error reading pin data.\n");
		devm_kfree(dev, pin_data);
		return -EINVAL;
	}

	npins = size / pin_size;

	if (info->flags & IMX8_USE_SCU) {
		imx_pinctrl_scu_conf_pins(info, pin_data, npins);
	} else {
		/*
		 * Refer to linux documentation for details:
		 * Documentation/devicetree/bindings/pinctrl/fsl,imx-pinctrl.txt
		 */
		for (i = 0; i < npins; i++) {
			mux_reg = pin_data[j++];

			if (!(info->flags & ZERO_OFFSET_VALID) && !mux_reg)
				mux_reg = -1;

			if (info->flags & SHARE_MUX_CONF_REG) {
				conf_reg = mux_reg;
			} else {
				conf_reg = pin_data[j++];
				if (!(info->flags & ZERO_OFFSET_VALID) &&
				    !conf_reg)
					conf_reg = -1;
			}

			if ((mux_reg == -1) || (conf_reg == -1)) {
				dev_err(dev, "Error mux_reg or conf_reg\n");
				devm_kfree(dev, pin_data);
				return -EINVAL;
			}

			input_reg = pin_data[j++];
			mux_mode = pin_data[j++];
			input_val = pin_data[j++];
			config_val = pin_data[j++];

			dev_dbg(dev, "mux_reg 0x%x, conf_reg 0x%x, "
				"input_reg 0x%x, mux_mode 0x%x, "
				"input_val 0x%x, config_val 0x%x\n",
				mux_reg, conf_reg, input_reg, mux_mode,
				input_val, config_val);

			if (config_val & IMX_PAD_SION)
				mux_mode |= IOMUXC_CONFIG_SION;

			config_val &= ~IMX_PAD_SION;

			/* Set Mux */
			if (info->flags & SHARE_MUX_CONF_REG) {
				clrsetbits_le32(info->base + mux_reg,
						info->mux_mask,
						mux_mode << mux_shift);
			} else {
				writel(mux_mode, info->base + mux_reg);
			}

			dev_dbg(dev, "write mux: offset 0x%x val 0x%x\n",
				mux_reg, mux_mode);

			/*
			 * Set select input
			 *
			 * If the select input value begins with 0xff,
			 * it's a quirky select input and the value should
			 * be interpreted as below.
			 *     31     23      15      7        0
			 *     | 0xff | shift | width | select |
			 * It's used to work around the problem that the
			 * select input for some pin is not implemented in
			 * the select input register but in some general
			 * purpose register. We encode the select input
			 * value, width and shift of the bit field into
			 * input_val cell of pin function ID in device tree,
			 * and then decode them here for setting up the select
			 * input bits in general purpose register.
			 */

			if (input_val >> 24 == 0xff) {
				u32 val = input_val;
				u8 select = val & 0xff;
				u8 width = (val >> 8) & 0xff;
				u8 shift = (val >> 16) & 0xff;
				u32 mask = ((1 << width) - 1) << shift;
				/*
				 * The input_reg[i] here is actually some
				 * IOMUXC general purpose register, not
				 * regular select input register.
				 */
				val = readl(info->base + input_reg);
				val &= ~mask;
				val |= select << shift;
				writel(val, info->base + input_reg);
			} else if (input_reg) {
				/*
				 * Regular select input register can never be
				 * at offset 0, and we only print register
				 * value for regular case.
				 */
				if (info->input_sel_base)
					writel(input_val,
					       info->input_sel_base +
					       input_reg);
				else
					writel(input_val,
					       info->base + input_reg);

				dev_dbg(dev, "select_input: offset 0x%x val "
					"0x%x\n", input_reg, input_val);
			}

			/* Set config */
			if (!(config_val & IMX_NO_PAD_CTL)) {
				if (info->flags & SHARE_MUX_CONF_REG) {
					clrsetbits_le32(info->base + conf_reg,
							~info->mux_mask,
							config_val);
				} else {
					writel(config_val,
					       info->base + conf_reg);
				}

				dev_dbg(dev, "write config: offset 0x%x val "
					"0x%x\n", conf_reg, config_val);
			}
		}
	}

	devm_kfree(dev, pin_data);

	return 0;
}

const struct pinctrl_ops imx_pinctrl_ops  = {
	.set_state = imx_pinctrl_set_state,
};

int imx_pinctrl_probe(struct udevice *dev,
		      struct imx_pinctrl_soc_info *info)
{
	struct imx_pinctrl_priv *priv = dev_get_priv(dev);
	int node = dev_of_offset(dev), ret;
	struct fdtdec_phandle_args arg;
	fdt_addr_t addr;
	fdt_size_t size;

	if (!info) {
		dev_err(dev, "wrong pinctrl info\n");
		return -EINVAL;
	}

	priv->dev = dev;
	priv->info = info;

	if (info->flags & IMX8_USE_SCU)
		return 0;

	addr = fdtdec_get_addr_size(gd->fdt_blob, dev_of_offset(dev), "reg",
				    &size);

	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	info->base = map_sysmem(addr, size);
	if (!info->base)
		return -ENOMEM;
	priv->info = info;

	info->mux_mask = fdtdec_get_int(gd->fdt_blob, node, "fsl,mux_mask", 0);
	/*
	 * Refer to linux documentation for details:
	 * Documentation/devicetree/bindings/pinctrl/fsl,imx7d-pinctrl.txt
	 */
	if (fdtdec_get_bool(gd->fdt_blob, node, "fsl,input-sel")) {
		ret = fdtdec_parse_phandle_with_args(gd->fdt_blob,
						     node, "fsl,input-sel",
						     NULL, 0, 0, &arg);
		if (ret) {
			dev_err(dev, "iomuxc fsl,input-sel property not found\n");
			return -EINVAL;
		}

		addr = fdtdec_get_addr_size(gd->fdt_blob, arg.node, "reg",
					    &size);
		if (addr == FDT_ADDR_T_NONE)
			return -EINVAL;

		info->input_sel_base = map_sysmem(addr, size);
		if (!info->input_sel_base)
			return -ENOMEM;
	}

	dev_dbg(dev, "initialized IMX pinctrl driver\n");

	return 0;
}

int imx_pinctrl_remove(struct udevice *dev)
{
	struct imx_pinctrl_priv *priv = dev_get_priv(dev);
	struct imx_pinctrl_soc_info *info = priv->info;

	if (info->flags & IMX8_USE_SCU)
		return 0;

	if (info->input_sel_base)
		unmap_sysmem(info->input_sel_base);
	if (info->base)
		unmap_sysmem(info->base);

	return 0;
}
