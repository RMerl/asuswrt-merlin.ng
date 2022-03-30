/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Stefan Roese <sr@denx.de>
 */

#ifndef _CONFIG_CRS305_1G_4S_H
#define _CONFIG_CRS305_1G_4S_H

/*
 * High Level Configuration Options (easy to change)
 */

#define CONFIG_SYS_KWD_CONFIG	$(CONFIG_BOARDDIR)/kwbimage.cfg
#define CONFIG_SYS_TCLK		200000000	/* 200MHz */

/* USB/EHCI configuration */
#define CONFIG_EHCI_IS_TDI

/* Environment in SPI NOR flash */
#define CONFIG_ENV_OFFSET		(1 << 20) /* 1MiB in */
#define CONFIG_ENV_SIZE			(64 << 10) /* 64KiB */
#define CONFIG_ENV_SECT_SIZE		(256 << 10) /* 256KiB sectors */

/* Keep device tree and initrd in lower memory so the kernel can access them */
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"fdt_high=0x10000000\0"		\
	"initrd_high=0x10000000\0"

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"
#undef CONFIG_SYS_MAXARGS
#define CONFIG_SYS_MAXARGS 96

#endif /* _CONFIG_CRS305_1G_4S_H */
