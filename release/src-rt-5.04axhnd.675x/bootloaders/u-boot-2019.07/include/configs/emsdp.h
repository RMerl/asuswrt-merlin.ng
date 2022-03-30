/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Synopsys, Inc. All rights reserved.
 */

#ifndef _CONFIG_EMSDP_H_
#define _CONFIG_EMSDP_H_

#include <linux/sizes.h>

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_SDRAM_BASE		0x10000000
#define CONFIG_SYS_SDRAM_SIZE		SZ_16M

#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + SZ_1M)

#define CONFIG_SYS_MALLOC_LEN		SZ_64K
#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SDRAM_BASE

/*
 * Environment
 */
#define CONFIG_BOOTFILE			"app.bin"
#define CONFIG_LOADADDR			CONFIG_SYS_LOAD_ADDR

#define CONFIG_EXTRA_ENV_SETTINGS \
	"upgrade_image=u-boot.bin\0" \
	"upgrade=emsdp rom unlock && " \
		"fatload mmc 0 ${loadaddr} ${upgrade_image} && " \
		"cp.b ${loadaddr} 0 ${filesize} && " \
		"dcache flush && " \
		"emsdp rom lock\0"

#endif /* _CONFIG_EMSDP_H_ */

