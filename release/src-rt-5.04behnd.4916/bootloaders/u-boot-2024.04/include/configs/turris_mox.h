/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Marek Behún <kabel@kernel.org>
 *
 * Based on mvebu_armada-37xx.h by Stefan Roese <sr@denx.de>
 */

#ifndef _CONFIG_TURRIS_MOX_H
#define _CONFIG_TURRIS_MOX_H

#define CFG_SYS_SDRAM_BASE		0x00000000
#define CFG_SYS_BAUDRATE_TABLE	{ 300, 600, 1200, 1800, 2400, 4800, \
					  9600, 19200, 38400, 57600, 115200, \
					  230400, 460800, 500000, 576000, \
					  921600, 1000000, 1152000, 1500000, \
					  2000000, 2500000, 3000000, 3500000, \
					  4000000, 4500000, 5000000, 5500000, \
					  6000000 }

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(NVME, nvme, 0) \
	func(SCSI, scsi, 0) \
	func(USB, usb, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#define TURRIS_MOX_BOOTCMD_RESCUE					\
	"setenv bootargs \"console=ttyMV0,115200 "			\
			  "earlycon=ar3700_uart,0xd0012000\" && "	\
	"sf probe && "							\
	"sf read 0x5000000 0x190000 && "				\
	"lzmadec 0x5000000 0x5800000 && "				\
	"bootm 0x5800000"

#define CFG_EXTRA_ENV_SETTINGS				\
	"fdt_addr=0x4c00000\0"					\
	"scriptaddr=0x4d00000\0"				\
	"pxefile_addr_r=0x4e00000\0"				\
	"fdt_addr_r=0x4f00000\0"				\
	"kernel_addr_r=0x5000000\0"				\
	"ramdisk_addr_r=0x8000000\0"				\
	"fdtfile=marvell/" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0"	\
	"bootcmd_rescue=" TURRIS_MOX_BOOTCMD_RESCUE "\0"	\
	BOOTENV

#endif /* _CONFIG_TURRIS_MOX_H */
