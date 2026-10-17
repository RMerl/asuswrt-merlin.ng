// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2023 Linaro Ltd.
 * Author: Sam Protsenko <semen.protsenko@linaro.org>
 *
 * Exynos850 pinctrl driver.
 */

#include <dm.h>
#include <dm/pinctrl.h>
#include "pinctrl-exynos.h"

#define EXYNOS850_PIN_BANK(pins, reg, id)		\
	{						\
		.type		= &exynos850_bank_type,	\
		.offset		= reg,			\
		.nr_pins	= pins,			\
		.name		= id			\
	}

/* CON, DAT, PUD, DRV */
static const struct samsung_pin_bank_type exynos850_bank_type = {
	.fld_width = { 4, 1, 4, 4, },
	.reg_offset = { 0x00, 0x04, 0x08, 0x0c, },
};

static const struct pinctrl_ops exynos850_pinctrl_ops = {
	.set_state = exynos_pinctrl_set_state
};

/* pin banks of exynos850 pin-controller 0 (ALIVE) */
static const struct samsung_pin_bank_data exynos850_pin_banks0[] = {
	EXYNOS850_PIN_BANK(8, 0x000, "gpa0"),
	EXYNOS850_PIN_BANK(8, 0x020, "gpa1"),
	EXYNOS850_PIN_BANK(8, 0x040, "gpa2"),
	EXYNOS850_PIN_BANK(8, 0x060, "gpa3"),
	EXYNOS850_PIN_BANK(4, 0x080, "gpa4"),
	EXYNOS850_PIN_BANK(3, 0x0a0, "gpq0"),
};

/* pin banks of exynos850 pin-controller 1 (CMGP) */
static const struct samsung_pin_bank_data exynos850_pin_banks1[] = {
	EXYNOS850_PIN_BANK(1, 0x000, "gpm0"),
	EXYNOS850_PIN_BANK(1, 0x020, "gpm1"),
	EXYNOS850_PIN_BANK(1, 0x040, "gpm2"),
	EXYNOS850_PIN_BANK(1, 0x060, "gpm3"),
	EXYNOS850_PIN_BANK(1, 0x080, "gpm4"),
	EXYNOS850_PIN_BANK(1, 0x0a0, "gpm5"),
	EXYNOS850_PIN_BANK(1, 0x0c0, "gpm6"),
	EXYNOS850_PIN_BANK(1, 0x0e0, "gpm7"),
};

/* pin banks of exynos850 pin-controller 2 (AUD) */
static const struct samsung_pin_bank_data exynos850_pin_banks2[] = {
	EXYNOS850_PIN_BANK(5, 0x000, "gpb0"),
	EXYNOS850_PIN_BANK(5, 0x020, "gpb1"),
};

/* pin banks of exynos850 pin-controller 3 (HSI) */
static const struct samsung_pin_bank_data exynos850_pin_banks3[] = {
	EXYNOS850_PIN_BANK(6, 0x000, "gpf2"),
};

/* pin banks of exynos850 pin-controller 4 (CORE) */
static const struct samsung_pin_bank_data exynos850_pin_banks4[] = {
	EXYNOS850_PIN_BANK(4, 0x000, "gpf0"),
	EXYNOS850_PIN_BANK(8, 0x020, "gpf1"),
};

/* pin banks of exynos850 pin-controller 5 (PERI) */
static const struct samsung_pin_bank_data exynos850_pin_banks5[] = {
	EXYNOS850_PIN_BANK(2, 0x000, "gpg0"),
	EXYNOS850_PIN_BANK(6, 0x020, "gpp0"),
	EXYNOS850_PIN_BANK(4, 0x040, "gpp1"),
	EXYNOS850_PIN_BANK(4, 0x060, "gpp2"),
	EXYNOS850_PIN_BANK(8, 0x080, "gpg1"),
	EXYNOS850_PIN_BANK(8, 0x0a0, "gpg2"),
	EXYNOS850_PIN_BANK(1, 0x0c0, "gpg3"),
	EXYNOS850_PIN_BANK(3, 0x0e0, "gpc0"),
	EXYNOS850_PIN_BANK(6, 0x100, "gpc1"),
};

static const struct samsung_pin_ctrl exynos850_pin_ctrl[] = {
	{
		/* pin-controller instance 0 ALIVE data */
		.pin_banks	= exynos850_pin_banks0,
		.nr_banks	= ARRAY_SIZE(exynos850_pin_banks0),
	}, {
		/* pin-controller instance 1 CMGP data */
		.pin_banks	= exynos850_pin_banks1,
		.nr_banks	= ARRAY_SIZE(exynos850_pin_banks1),
	}, {
		/* pin-controller instance 2 AUD data */
		.pin_banks	= exynos850_pin_banks2,
		.nr_banks	= ARRAY_SIZE(exynos850_pin_banks2),
	}, {
		/* pin-controller instance 3 HSI data */
		.pin_banks	= exynos850_pin_banks3,
		.nr_banks	= ARRAY_SIZE(exynos850_pin_banks3),
	}, {
		/* pin-controller instance 4 CORE data */
		.pin_banks	= exynos850_pin_banks4,
		.nr_banks	= ARRAY_SIZE(exynos850_pin_banks4),
	}, {
		/* pin-controller instance 5 PERI data */
		.pin_banks	= exynos850_pin_banks5,
		.nr_banks	= ARRAY_SIZE(exynos850_pin_banks5),
	},
	{/* list terminator */}
};

static const struct udevice_id exynos850_pinctrl_ids[] = {
	{ .compatible = "samsung,exynos850-pinctrl",
		.data = (ulong)exynos850_pin_ctrl },
	{ }
};

U_BOOT_DRIVER(pinctrl_exynos850) = {
	.name		= "pinctrl_exynos850",
	.id		= UCLASS_PINCTRL,
	.of_match	= exynos850_pinctrl_ids,
	.priv_auto	= sizeof(struct exynos_pinctrl_priv),
	.ops		= &exynos850_pinctrl_ops,
	.probe		= exynos_pinctrl_probe,
};
