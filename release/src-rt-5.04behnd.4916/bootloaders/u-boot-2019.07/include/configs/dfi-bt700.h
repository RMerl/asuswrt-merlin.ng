/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/x86-common.h>

#define CONFIG_SYS_MONITOR_LEN		(1 << 20)

#ifndef CONFIG_INTERNAL_UART
/* Use BayTrail internal HS UART which is memory-mapped */
#undef  CONFIG_SYS_NS16550_PORT_MAPPED
#endif

#define CONFIG_STD_DEVICES_SETTINGS     "stdin=serial\0" \
					"stdout=serial\0" \
					"stderr=serial\0"

#define VIDEO_IO_OFFSET				0
#define CONFIG_X86EMU_RAW_IO

#define CONFIG_ENV_SECT_SIZE		0x1000
#define CONFIG_ENV_OFFSET		0x006ef000

#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND	\
	"load scsi 0:1 03000000 /boot/vmlinuz-${kernel-ver}-generic;"	\
	"load scsi 0:1 04000000 /boot/initrd.img-${kernel-ver}-generic;" \
	"run boot"

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS				\
	"kernel-ver=4.4.0-24\0"					\
	"boot=zboot 03000000 0 04000000 ${filesize}\0"		\
	"upd_uboot=usb reset;tftp 100000 dfi/u-boot.rom;"	\
		"sf probe;sf update 100000 0 800000;saveenv\0"

#define CONFIG_PREBOOT

#endif	/* __CONFIG_H */
