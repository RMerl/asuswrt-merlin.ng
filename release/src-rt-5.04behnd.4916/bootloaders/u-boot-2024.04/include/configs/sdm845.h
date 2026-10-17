/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration file for boards, based on Qualcomm SDM845 chip
 *
 * (C) Copyright 2021 Dzmitry Sankouski <dsankouski@gmail.com>
 */

#ifndef __CONFIGS_SDM845_H
#define __CONFIGS_SDM845_H

#include <linux/sizes.h>

#define CFG_SYS_BAUDRATE_TABLE	{ 115200, 230400, 460800, 921600 }

#define CFG_EXTRA_ENV_SETTINGS \
	"bootm_size=0x4000000\0"	\
	"bootm_low=0x80000000\0"	\
	"stdin=serial,button-kbd\0"	\
	"stdout=serial,vidconsole\0"	\
	"stderr=serial,vidconsole\0"	\
	"preboot=source $prevbl_initrd_start_addr:prebootscript\0" \
	"bootcmd=source $prevbl_initrd_start_addr:bootscript\0"

/* Size of malloc() pool */

#endif
