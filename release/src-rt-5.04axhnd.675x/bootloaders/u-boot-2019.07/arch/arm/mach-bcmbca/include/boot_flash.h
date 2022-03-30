/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _BOOT_FLASH_H
#define _BOOT_FLASH_H

#define NOR_XIP_BASE_ADDR 0xffd00000

#define FLASH_DEV_STR_NAND	 "NAND"
#define FLASH_DEV_STR_SPINOR "SPINOR"
#define FLASH_DEV_STR_EMMC	 "EMMC"

#define SPIFLASH_MTDNAME "nor0"

int boot_flash_init(void);
int read_boot_device(uint32_t address, void *data, int len);

#endif
