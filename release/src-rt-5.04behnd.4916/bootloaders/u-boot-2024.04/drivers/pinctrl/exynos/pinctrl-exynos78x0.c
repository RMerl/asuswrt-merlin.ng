// SPDX-License-Identifier: GPL-2.0+
/*
 * Exynos78x0 pinctrl driver.
 *
 * Copyright (c) 2020 Dzmitry Sankouski (dsankouski@gmail.com)
 *
 * based on drivers/pinctrl/exynos/pinctrl-exynos7420.c :
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

static const struct pinctrl_ops exynos78x0_pinctrl_ops = {
	.set_state	= exynos_pinctrl_set_state
};

/* pin banks of exynos78x0 pin-controller 0 (ALIVE) */
static const struct samsung_pin_bank_data exynos78x0_pin_banks0[] = {
	EXYNOS_PIN_BANK(6, 0x000, "etc0"),
	EXYNOS_PIN_BANK(3, 0x020, "etc1"),
	EXYNOS_PIN_BANK(8, 0x040, "gpa0"),
	EXYNOS_PIN_BANK(8, 0x060, "gpa1"),
	EXYNOS_PIN_BANK(8, 0x080, "gpa2"),
	EXYNOS_PIN_BANK(5, 0x0a0, "gpa3"),
	EXYNOS_PIN_BANK(2, 0x0c0, "gpq0"),
};

/* pin banks of exynos78x0 pin-controller 1 (CCORE) */
static const struct samsung_pin_bank_data exynos78x0_pin_banks1[] = {
	EXYNOS_PIN_BANK(2, 0x000, "gpm0"),
};

/* pin banks of exynos78x0 pin-controller 2 (DISPAUD) */
static const struct samsung_pin_bank_data exynos78x0_pin_banks2[] = {
	EXYNOS_PIN_BANK(4, 0x000, "gpz0"),
	EXYNOS_PIN_BANK(6, 0x020, "gpz1"),
	EXYNOS_PIN_BANK(4, 0x040, "gpz2"),
};

/* pin banks of exynos78x0 pin-controller 4 (FSYS) */
static const struct samsung_pin_bank_data exynos78x0_pin_banks4[] = {
	EXYNOS_PIN_BANK(3, 0x000, "gpr0"),
	EXYNOS_PIN_BANK(8, 0x020, "gpr1"),
	EXYNOS_PIN_BANK(2, 0x040, "gpr2"),
	EXYNOS_PIN_BANK(4, 0x060, "gpr3"),
	EXYNOS_PIN_BANK(6, 0x080, "gpr4"),
};

/* pin banks of exynos78x0 pin-controller 6 (TOP) */
static const struct samsung_pin_bank_data exynos78x0_pin_banks6[] = {
	EXYNOS_PIN_BANK(4, 0x000, "gpb0"),
	EXYNOS_PIN_BANK(3, 0x020, "gpc0"),
	EXYNOS_PIN_BANK(4, 0x040, "gpc1"),
	EXYNOS_PIN_BANK(4, 0x060, "gpc4"),
	EXYNOS_PIN_BANK(2, 0x080, "gpc5"),
	EXYNOS_PIN_BANK(4, 0x0a0, "gpc6"),
	EXYNOS_PIN_BANK(2, 0x0c0, "gpc8"),
	EXYNOS_PIN_BANK(2, 0x0e0, "gpc9"),
	EXYNOS_PIN_BANK(7, 0x100, "gpd1"),
	EXYNOS_PIN_BANK(6, 0x120, "gpd2"),
	EXYNOS_PIN_BANK(8, 0x140, "gpd3"),
	EXYNOS_PIN_BANK(7, 0x160, "gpd4"),
	EXYNOS_PIN_BANK(5, 0x180, "gpd5"),
	EXYNOS_PIN_BANK(3, 0x1a0, "gpe0"),
	EXYNOS_PIN_BANK(4, 0x1c0, "gpf0"),
	EXYNOS_PIN_BANK(2, 0x1e0, "gpf1"),
	EXYNOS_PIN_BANK(2, 0x200, "gpf2"),
	EXYNOS_PIN_BANK(4, 0x220, "gpf3"),
	EXYNOS_PIN_BANK(5, 0x240, "gpf4"),
};

const struct samsung_pin_ctrl exynos78x0_pin_ctrl[] = {
	{
		/* pin-controller instance 0 Alive data */
		.pin_banks	= exynos78x0_pin_banks0,
		.nr_banks	= ARRAY_SIZE(exynos78x0_pin_banks0),
	}, {
		/* pin-controller instance 1 CCORE data */
		.pin_banks	= exynos78x0_pin_banks1,
		.nr_banks	= ARRAY_SIZE(exynos78x0_pin_banks1),
	}, {
		/* pin-controller instance 2 DISPAUD data */
		.pin_banks	= exynos78x0_pin_banks2,
		.nr_banks	= ARRAY_SIZE(exynos78x0_pin_banks2),
	}, {
		/* pin-controller instance 4 FSYS data */
		.pin_banks	= exynos78x0_pin_banks4,
		.nr_banks	= ARRAY_SIZE(exynos78x0_pin_banks4),
	}, {
		/* pin-controller instance 6 TOP data */
		.pin_banks	= exynos78x0_pin_banks6,
		.nr_banks	= ARRAY_SIZE(exynos78x0_pin_banks6),
	},
	{/* list terminator */}
};

static const struct udevice_id exynos78x0_pinctrl_ids[] = {
	{ .compatible = "samsung,exynos78x0-pinctrl",
		.data = (ulong)exynos78x0_pin_ctrl },
	{ }
};

U_BOOT_DRIVER(pinctrl_exynos78x0) = {
	.name		= "pinctrl_exynos78x0",
	.id		= UCLASS_PINCTRL,
	.of_match	= exynos78x0_pinctrl_ids,
	.priv_auto = sizeof(struct exynos_pinctrl_priv),
	.ops		= &exynos78x0_pinctrl_ops,
	.probe		= exynos_pinctrl_probe,
};
