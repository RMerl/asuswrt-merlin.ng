// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 */

#include <common.h>
#include <errno.h>
#include <linux/bitops.h>
#include <asm/io.h>
#include <asm/arch/sci/sci.h>
#include <misc.h>

#include "pinctrl-imx.h"

#define PADRING_IFMUX_EN_SHIFT		31
#define PADRING_IFMUX_EN_MASK		BIT(31)
#define PADRING_GP_EN_SHIFT		30
#define PADRING_GP_EN_MASK		BIT(30)
#define PADRING_IFMUX_SHIFT		27
#define PADRING_IFMUX_MASK		GENMASK(29, 27)

static int imx_pinconf_scu_set(struct imx_pinctrl_soc_info *info, u32 pad,
			       u32 mux, u32 val)
{
	int ret;

	/*
	 * Mux should be done in pmx set, but we do not have a good api
	 * to handle that in scfw, so config it in pad conf func
	 */

	val |= PADRING_IFMUX_EN_MASK;
	val |= PADRING_GP_EN_MASK;
	val |= (mux << PADRING_IFMUX_SHIFT) & PADRING_IFMUX_MASK;

	ret = sc_pad_set(-1, pad, val);
	if (ret)
		printf("%s %d\n", __func__, ret);

	return 0;
}

int imx_pinctrl_scu_conf_pins(struct imx_pinctrl_soc_info *info, u32 *pin_data,
			      int npins)
{
	int pin_id, mux, config_val;
	int i, j = 0;
	int ret;

	/*
	 * Refer to linux documentation for details:
	 * Documentation/devicetree/bindings/pinctrl/fsl,imx-pinctrl.txt
	 */
	for (i = 0; i < npins; i++) {
		pin_id = pin_data[j++];
		mux = pin_data[j++];
		config_val = pin_data[j++];

		ret = imx_pinconf_scu_set(info, pin_id, mux, config_val);
		if (ret)
			printf("Set pin %d, mux %d, val %d, error\n", pin_id,
			       mux, config_val);
	}

	return 0;
}
