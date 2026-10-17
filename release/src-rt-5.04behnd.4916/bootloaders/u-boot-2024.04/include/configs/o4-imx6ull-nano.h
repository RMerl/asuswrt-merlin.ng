/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright (C) 2021 Oleh Kravchenko <oleg@kaa.org.ua> */

#ifndef __O4_IMX6ULL_NANO_CONFIG_H
#define __O4_IMX6ULL_NANO_CONFIG_H

#include "mx6_common.h"

#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR
#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#if IS_ENABLED(CONFIG_CMD_USB)
#	define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#endif /* CONFIG_CMD_USB */

#define CFG_EXTRA_ENV_SETTINGS \
	"mmcdev=0\0" \
	"mmcpart=2\0" \
	"mmcargs=setenv bootargs root=/dev/mmcblk${mmcdev}p${mmcpart} console=ttymxc0,${baudrate} panic=30\0" \
	"mmcboot=run mmcargs && ext4load mmc ${mmcdev}:${mmcpart} $loadaddr /boot/zImage && bootz $loadaddr - $fdtcontroladdr\0" \
	"bootcmd=run mmcboot\0" \
	"bootcmd_mfg=fastboot usb 0\0"

#endif /* __O4_IMX6ULL_NANO_CONFIG_H */
