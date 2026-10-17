/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019
 * shuyiqi  <shuyiqi@phytium.com.cn>
 * liuhao   <liuhao@phytium.com.cn>
 */

#ifndef __DURIAN_CONFIG_H__
#define __DURIAN_CONFIG_H__

/* Sdram Bank #1 Address */
#define PHYS_SDRAM_1			0x80000000
#define PHYS_SDRAM_1_SIZE		0x7B000000
#define CFG_SYS_SDRAM_BASE   PHYS_SDRAM_1

/* BOOT */

#define CFG_EXTRA_ENV_SETTINGS	\
	"load_kernel=ext4load scsi 0:1 0x90100000 uImage-2004\0"	\
	"load_fdt=ext4load scsi 0:1 0x95000000 ft2004-pci-64.dtb\0"\
	"boot_fdt=bootm 0x90100000 -:- 0x95000000\0"	\
	"distro_bootcmd=run load_kernel; run load_fdt; run boot_fdt"

#endif
