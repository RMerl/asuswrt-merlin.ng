/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015
 * Kamil Lulko, <kamil.lulko@gmail.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CFG_SYS_FLASH_BASE		0x08000000

/*
 * Configuration of the external SDRAM memory
 */

#define CFG_SYS_HZ_CLOCK		1000000	/* Timer is clocked at 1MHz */

#define CFG_EXTRA_ENV_SETTINGS \
	"bootargs_romfs=uclinux.physaddr=0x08180000 root=/dev/mtdblock0\0" \
	"bootcmd_romfs=setenv bootargs ${bootargs} ${bootargs_romfs};" \
	"bootm 0x08044000 - 0x08042000\0"

#endif /* __CONFIG_H */
