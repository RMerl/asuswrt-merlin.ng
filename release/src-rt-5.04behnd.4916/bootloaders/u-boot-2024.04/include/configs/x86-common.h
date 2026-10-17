/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
 */

#ifndef __CONFIG_X86_COMMON_H
#define __CONFIG_X86_COMMON_H

/*-----------------------------------------------------------------------
 * USB configuration
 */

/* Default environment */
#define CFG_RAMDISK_ADDR	0x4000000
#if defined(CONFIG_GENERATE_ACPI_TABLE) || defined(CONFIG_EFI_STUB)
#define CFG_OTHBOOTARGS	"othbootargs=\0"
#else
#define CFG_OTHBOOTARGS	"othbootargs=acpi=off\0"
#endif

#ifndef SPLASH_SETTINGS
#define SPLASH_SETTINGS
#endif

#define CFG_EXTRA_ENV_SETTINGS			\
	CFG_STD_DEVICES_SETTINGS			\
	SPLASH_SETTINGS					\
	"pciconfighost=1\0"				\
	"netdev=eth0\0"					\
	"consoledev=ttyS0\0"				\
	CFG_OTHBOOTARGS				\
	"scriptaddr=0x7000000\0"			\
	"kernel_addr_r=0x1000000\0"			\
	"ramdisk_addr_r=0x4000000\0"			\
	"ramdiskfile=initramfs.gz\0"


#endif	/* __CONFIG_H */
