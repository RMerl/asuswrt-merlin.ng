// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Atmel Corporation
 *               Wenyou.Yang <wenyou.yang@atmel.com>
 */

#include <common.h>
#include <dm.h>

static const struct udevice_id at91_sckc_match[] = {
	{ .compatible = "atmel,at91sam9x5-sckc" },
	{}
};

U_BOOT_DRIVER(at91_sckc) = {
	.name = "at91-sckc",
	.id = UCLASS_SIMPLE_BUS,
	.of_match = at91_sckc_match,
};
