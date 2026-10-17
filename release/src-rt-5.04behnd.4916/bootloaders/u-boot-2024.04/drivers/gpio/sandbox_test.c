// SPDX-License-Identifier: GPL-2.0
/*
 * Sandbox driver for testing GPIOs with of-platdata
 *
 * Copyright 2021 Google LLC
 */

#include <common.h>
#include <dm.h>
#include <asm-generic/gpio.h>

static const struct udevice_id sandbox_gpio_test_ids[] = {
	{ .compatible = "sandbox,gpio-test" },
	{ }
};

U_BOOT_DRIVER(sandbox_gpio_test) = {
	.name = "sandbox_gpio_test",
	.id = UCLASS_MISC,
	.of_match = sandbox_gpio_test_ids,
};
