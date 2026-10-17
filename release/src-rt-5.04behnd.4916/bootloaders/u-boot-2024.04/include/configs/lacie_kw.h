/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Simon Guinot <sguinot@lacie.com>
 */

#ifndef _CONFIG_LACIE_KW_H
#define _CONFIG_LACIE_KW_H

#include "mv-common.h"

/* Remove or override few declarations from mv-common.h */

/*
 * Enable platform initialisation via misc_init_r() function
 */

/*
 * Enable GPI0 support
 */

/*
 * Enable I2C support
 */
#ifdef CONFIG_CMD_I2C
/* I2C EEPROM HT24LC04 (512B - 32 pages of 16 Bytes) */
#if defined(CONFIG_NET2BIG_V2)
#define CFG_SYS_I2C_G762_ADDR		0x3e
#endif
#endif /* CONFIG_CMD_I2C */

/*
 * Partition support
 */

/*
 * File systems support
 */

/*
 * Environment variables configurations
 */

/*
 * Default environment variables
 */

#define CFG_EXTRA_ENV_SETTINGS				\
	"stdin=serial\0"					\
	"stdout=serial\0"					\
	"stderr=serial\0"					\
	"bootfile=uImage\0"					\
	"loadaddr=0x800000\0"					\
	"netconsole="						\
		"set stdin $stdin,nc; "				\
		"set stdout $stdout,nc; "			\
		"set stderr $stderr,nc;\0"			\
	"diskload=sata init && "				\
		"ext2load sata 0:1 $loadaddr /boot/$bootfile\0"	\
	"usbload=usb start && "					\
		"fatload usb 0:1 $loadaddr /boot/$bootfile\0"

#endif /* _CONFIG_LACIE_KW_H */
