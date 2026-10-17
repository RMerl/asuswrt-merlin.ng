/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021-2022 Marek Vasut <marex@denx.de>
 */

#ifndef __IMX8MM_MX8MENLO_H
#define __IMX8MM_MX8MENLO_H

#include <configs/verdin-imx8mm.h>

/* Custom initial environment variables */
#undef CFG_EXTRA_ENV_SETTINGS
#define CFG_EXTRA_ENV_SETTINGS					\
	BOOTENV								\
	MEM_LAYOUT_ENV_SETTINGS						\
	"devtype=mmc\0"							\
	"devnum=1\0"							\
	"distro_bootpart=1\0"						\
	"altbootcmd="							\
		"mmc partconf 0 mmcpart ; "				\
		"if test ${mmcpart} -eq 1 ; then "			\
			"mmc partconf 0 1 2 0 ; "			\
		"else "							\
			"mmc partconf 0 1 1 0 ; "			\
		"fi ; "							\
		"boot\0"						\
	"boot_file=fitImage\0"						\
	"console=ttymxc0\0"						\
	"fdt_addr=0x43000000\0"						\
	"initrd_addr=0x43800000\0"					\
	"kernel_image=fitImage\0"

#endif /* __IMX8MM_MX8MENLO_H */
