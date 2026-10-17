/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration file for Dragonboard 845c, based on Qualcomm SDA845 chip
 *
 * (C) Copyright 2022 Sumit Garg <sumit.garg@linaro.org>
 */

#ifndef __CONFIGS_SDM845_H
#define __CONFIGS_SDM845_H

#include <linux/sizes.h>

#define CFG_SYS_BAUDRATE_TABLE	{ 115200, 230400, 460800, 921600 }

#define CFG_EXTRA_ENV_SETTINGS \
	"bootm_size=0x5000000\0"	\
	"bootm_low=0x80000000\0"	\
	"bootcmd=bootm $prevbl_initrd_start_addr\0"

#endif
