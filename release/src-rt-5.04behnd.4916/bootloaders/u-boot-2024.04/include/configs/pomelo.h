/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2021
 * lixinde         <lixinde@phytium.com.cn>
 * weichangzheng   <weichangzheng@phytium.com.cn>
 */

#ifndef __POMELO_CONFIG_H__
#define __POMELO_CONFIG_H__

/* SDRAM Bank #1 start address */
#define CFG_SYS_SDRAM_BASE		0x80000000

/* SIZE of malloc pool */

/*BOOT*/

#define BOOT_TARGET_DEVICES(func) \
	func(SCSI, scsi, 0) \

#include <config_distro_bootcmd.h>

/* Initial environment variables */
#define CFG_EXTRA_ENV_SETTINGS		\
	"image=Image\0" \
	BOOTENV \
	"scriptaddr=0x90100000\0" \
	"kernel_addr_r=0x90200000\0" \
	"fdt_addr_r=0x95000000\0"			\
	"boot_fit=no\0" \
	"fdtfile=phytium-pomelo.dtb\0" \

#endif

