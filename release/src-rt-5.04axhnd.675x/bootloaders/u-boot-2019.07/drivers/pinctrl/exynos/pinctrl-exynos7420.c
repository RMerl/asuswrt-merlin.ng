// SPDX-License-Identifier: GPL-2.0+
/*
 * Exynos7420 pinctrl driver.
 * Copyright (C) 2016 Samsung Electronics
 * Thomas Abraham <thomas.ab@samsung.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <dm/pinctrl.h>
#include <dm/root.h>
#include <fdtdec.h>
#include <asm/arch/pinmux.h>
#include "pinctrl-exynos.h"

#define	GPD1_OFFSET	0xc0

static struct exynos_pinctrl_config_data serial2_conf[] = {
	{
		.offset	= GPD1_OFFSET + PIN_CON,
		.mask	= 0x00ff0000,
		.value	= 0x00220000,
	}, {
		.offset	= GPD1_OFFSET + PIN_PUD,
		.mask	= 0x00000f00,
		.value	= 0x00000f00,
	},
};

static int exynos7420_pinctrl_request(struct udevice *dev, int peripheral,
						int flags)
{
	struct exynos_pinctrl_priv *priv = dev_get_priv(dev);
	unsigned long base = priv->base;

	switch (PERIPH_ID_UART2) {
	case PERIPH_ID_UART2:
		exynos_pinctrl_setup_peri(serial2_conf,
					  ARRAY_SIZE(serial2_conf), base);
		break;
	default:
		return -ENODEV;
	}

	return 0;
}

static struct pinctrl_ops exynos7420_pinctrl_ops = {
	.set_state	= exynos_pinctrl_set_state,
	.request	= exynos7420_pinctrl_request,
};

/* pin banks of Exynos7420 pin-controller - BUS0 */
static const struct samsung_pin_bank_data exynos7420_pin_banks0[] = {
	EXYNOS_PIN_BANK(5, 0x000, "gpb0"),
	EXYNOS_PIN_BANK(8, 0x020, "gpc0"),
	EXYNOS_PIN_BANK(2, 0x040, "gpc1"),
	EXYNOS_PIN_BANK(6, 0x060, "gpc2"),
	EXYNOS_PIN_BANK(8, 0x080, "gpc3"),
	EXYNOS_PIN_BANK(4, 0x0a0, "gpd0"),
	EXYNOS_PIN_BANK(6, 0x0c0, "gpd1"),
	EXYNOS_PIN_BANK(8, 0x0e0, "gpd2"),
	EXYNOS_PIN_BANK(5, 0x100, "gpd4"),
	EXYNOS_PIN_BANK(4, 0x120, "gpd5"),
	EXYNOS_PIN_BANK(6, 0x140, "gpd6"),
	EXYNOS_PIN_BANK(3, 0x160, "gpd7"),
	EXYNOS_PIN_BANK(2, 0x180, "gpd8"),
	EXYNOS_PIN_BANK(2, 0x1a0, "gpg0"),
	EXYNOS_PIN_BANK(4, 0x1c0, "gpg3"),
};

/* pin banks of Exynos7420 pin-controller - FSYS0 */
static const struct samsung_pin_bank_data exynos7420_pin_banks1[] = {
	EXYNOS_PIN_BANK(7, 0x000, "gpr4"),
};

/* pin banks of Exynos7420 pin-controller - FSYS1 */
static const struct samsung_pin_bank_data exynos7420_pin_banks2[] = {
	EXYNOS_PIN_BANK(4, 0x000, "gpr0"),
	EXYNOS_PIN_BANK(8, 0x020, "gpr1"),
	EXYNOS_PIN_BANK(5, 0x040, "gpr2"),
	EXYNOS_PIN_BANK(8, 0x060, "gpr3"),
};

const struct samsung_pin_ctrl exynos7420_pin_ctrl[] = {
	{
		/* pin-controller instance BUS0 data */
		.pin_banks	= exynos7420_pin_banks0,
		.nr_banks	= ARRAY_SIZE(exynos7420_pin_banks0),
	}, {
		/* pin-controller instance FSYS0 data */
		.pin_banks	= exynos7420_pin_banks1,
		.nr_banks	= ARRAY_SIZE(exynos7420_pin_banks1),
	}, {
		/* pin-controller instance FSYS1 data */
		.pin_banks	= exynos7420_pin_banks2,
		.nr_banks	= ARRAY_SIZE(exynos7420_pin_banks2),
	},
};

static const struct udevice_id exynos7420_pinctrl_ids[] = {
	{ .compatible = "samsung,exynos7420-pinctrl",
		.data = (ulong)exynos7420_pin_ctrl },
	{ }
};

U_BOOT_DRIVER(pinctrl_exynos7420) = {
	.name		= "pinctrl_exynos7420",
	.id		= UCLASS_PINCTRL,
	.of_match	= exynos7420_pinctrl_ids,
	.priv_auto_alloc_size = sizeof(struct exynos_pinctrl_priv),
	.ops		= &exynos7420_pinctrl_ops,
	.probe		= exynos_pinctrl_probe,
};
