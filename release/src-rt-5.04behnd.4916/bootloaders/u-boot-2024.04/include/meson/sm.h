/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2023 SberDevices, Inc.
 *
 * Author: Alexey Romanov <avromanov@salutedevices.com>
 */

#ifndef __MESON_SM_CMD_H__
#define __MESON_SM_CMD_H__

enum meson_smc_cmd {
	MESON_SMC_CMD_EFUSE_READ,  /* read efuse memory */
	MESON_SMC_CMD_EFUSE_WRITE, /* write efuse memory */
	MESON_SMC_CMD_CHIP_ID_GET, /* readh chip unique id */
	MESON_SMC_CMD_PWRDM_SET,   /* do command at specified power domain */
	MESON_SMC_CMD_COUNT,
};

#endif
