/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Synopsys, Inc. All rights reserved.
 */

#ifndef _CONFIG_EMSDP_H_
#define _CONFIG_EMSDP_H_

#include <linux/sizes.h>

#define CFG_SYS_SDRAM_BASE		0x10000000
#define CFG_SYS_SDRAM_SIZE		SZ_16M

/*
 * Environment
 */

#define CFG_EXTRA_ENV_SETTINGS \
	"upgrade_image=u-boot.bin\0" \
	"upgrade=emsdp rom unlock && " \
		"fatload mmc 0 ${loadaddr} ${upgrade_image} && " \
		"cp.b ${loadaddr} 0 ${filesize} && " \
		"dcache flush && " \
		"emsdp rom lock\0"

#endif /* _CONFIG_EMSDP_H_ */
