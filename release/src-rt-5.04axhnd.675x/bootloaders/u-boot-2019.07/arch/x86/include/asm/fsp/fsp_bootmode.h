/* SPDX-License-Identifier: Intel */
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __FSP_BOOT_MODE_H__
#define __FSP_BOOT_MODE_H__

/* 0x21 - 0xf..f are reserved */
#define BOOT_FULL_CONFIG		0x00
#define BOOT_MINIMAL_CONFIG		0x01
#define BOOT_NO_CONFIG_CHANGES		0x02
#define BOOT_FULL_CONFIG_PLUS_DIAG	0x03
#define BOOT_DEFAULT_SETTINGS		0x04
#define BOOT_ON_S4_RESUME		0x05
#define BOOT_ON_S5_RESUME		0x06
#define BOOT_ON_S2_RESUME		0x10
#define BOOT_ON_S3_RESUME		0x11
#define BOOT_ON_FLASH_UPDATE		0x12
#define BOOT_IN_RECOVERY_MODE		0x20

#endif
