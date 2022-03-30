/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Peng Fan <van.freenix@gmail.com>
 */

#ifndef __DRIVERS_PINCTRL_IMX_H
#define __DRIVERS_PINCTRL_IMX_H

/**
 * @base: the address to the controller in virtual memory
 * @input_sel_base: the address of the select input in virtual memory.
 * @flags: flags specific for each soc
 * @mux_mask: Used when SHARE_MUX_CONF_REG flag is added
 */
struct imx_pinctrl_soc_info {
	void __iomem *base;
	void __iomem *input_sel_base;
	unsigned int flags;
	unsigned int mux_mask;
};

/**
 * @dev: a pointer back to containing device
 * @info: the soc info
 */
struct imx_pinctrl_priv {
	struct udevice *dev;
	struct imx_pinctrl_soc_info *info;
};

extern const struct pinctrl_ops imx_pinctrl_ops;

#define IMX_NO_PAD_CTL	0x80000000	/* no pin config need */
#define IMX_PAD_SION	0x40000000	/* set SION */

/*
 * Each pin represented in fsl,pins consists of 5 u32 PIN_FUNC_ID and
 * 1 u32 CONFIG, so 24 types in total for each pin.
 */
#define FSL_PIN_SIZE		24
#define SHARE_FSL_PIN_SIZE	20

/* Each pin on imx8qm/qxp consists of 2 u32 PIN_FUNC_ID and 1 u32 CONFIG */
#define SHARE_IMX8_PIN_SIZE	12

#define SHARE_MUX_CONF_REG	0x1
#define ZERO_OFFSET_VALID	0x2
#define CONFIG_IBE_OBE		0x4
#define IMX8_USE_SCU		0x8

#define IOMUXC_CONFIG_SION	(0x1 << 4)

int imx_pinctrl_probe(struct udevice *dev, struct imx_pinctrl_soc_info *info);

int imx_pinctrl_remove(struct udevice *dev);

#ifdef CONFIG_PINCTRL_IMX_SCU
int imx_pinctrl_scu_conf_pins(struct imx_pinctrl_soc_info *info,
			      u32 *pin_data, int npins);
#else
static inline int imx_pinctrl_scu_conf_pins(struct imx_pinctrl_soc_info *info,
					    u32 *pin_data, int npins)
{
	return 0;
}
#endif

#endif /* __DRIVERS_PINCTRL_IMX_H */
