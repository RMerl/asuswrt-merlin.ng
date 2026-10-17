/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Stefan Roese <sr@denx.de>
 */

#ifndef _CONFIG_SYNOLOGY_DS414_H
#define _CONFIG_SYNOLOGY_DS414_H

/*
 * High Level Configuration Options (easy to change)
 */

/*
 * TEXT_BASE needs to be below 16MiB, since this area is scrubbed
 * for DDR ECC byte filling in the SPL before loading the main
 * U-Boot into it.
 */

/* I2C */
#define CFG_I2C_MVTWSI_BASE0		MVEBU_TWSI_BASE

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/*
 * Memory layout while starting into the bin_hdr via the
 * BootROM:
 *
 * 0x4000.4000 - 0x4003.4000	headers space (192KiB)
 * 0x4000.4030			bin_hdr start address
 * 0x4003.4000 - 0x4004.7c00	BootROM memory allocations (15KiB)
 * 0x4007.fffc			BootROM stack top
 *
 * The address space between 0x4007.fffc and 0x400f.fff is not locked in
 * L2 cache thus cannot be used.
 */

/* SPL */
/* Defines for SPL */

/* Default Environment */

#define CFG_EXTRA_ENV_SETTINGS				\
	"initrd_high=0xffffffff\0"				\
	"ramdisk_addr_r=0x8000000\0"				\
	"usb0Mode=host\0usb1Mode=host\0usb2Mode=device\0"	\
	"ethmtu=1500\0eth1mtu=1500\0"				\
	"update_uboot=sf probe; dhcp; "				\
		"mw.b ${loadaddr} 0x0 0xd0000; "		\
		"tftpboot ${loadaddr} u-boot-with-spl.kwb; "	\
		"sf update ${loadaddr} 0x0 0xd0000\0"


/* increase autoneg timeout, my NIC sucks */
#define PHY_ANEG_TIMEOUT	16000

#endif /* _CONFIG_SYNOLOGY_DS414_H */
