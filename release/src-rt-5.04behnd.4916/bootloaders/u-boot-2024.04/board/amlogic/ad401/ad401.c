// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2023 SberDevices, Inc.
 * Author: Igor Prusov <ivprusov@sberdevices.ru>
 */

#include <init.h>
#include <asm/arch/eth.h>

int misc_init_r(void)
{
	meson_generate_serial_ethaddr();

	return 0;
}
