/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + SZ_2M)

#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + SZ_2M)

#define CONFIG_SYS_MALLOC_LEN		SZ_8M

#define CONFIG_SYS_BOOTM_LEN		SZ_16M

#define CONFIG_STANDALONE_LOAD_ADDR	0x80200000

/* Environment options */
#define CONFIG_ENV_SIZE			SZ_4K

#define BOOT_TARGET_DEVICES(func) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#define CONFIG_EXTRA_ENV_SETTINGS \
	"fdt_high=0xffffffffffffffff\0" \
	"initrd_high=0xffffffffffffffff\0" \
	"kernel_addr_r=0x80600000\0" \
	"fdt_addr_r=0x82200000\0" \
	"scriptaddr=0x82300000\0" \
	"pxefile_addr_r=0x82400000\0" \
	"ramdisk_addr_r=0x82500000\0" \
	BOOTENV

#endif /* __CONFIG_H */
