// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#include <common.h>
#include <dm.h>
#include <env_internal.h>
#include <init.h>
#include <net.h>
#include <asm/io.h>
#include <asm/arch/eth.h>

int misc_init_r(void)
{
	meson_generate_serial_ethaddr();

	return 0;
}
