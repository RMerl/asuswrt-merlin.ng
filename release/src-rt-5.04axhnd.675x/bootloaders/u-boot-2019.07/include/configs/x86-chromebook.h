/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 */

#ifndef _X86_CHROMEBOOK_H
#define _X86_CHROMEBOOK_H

#define CONFIG_SYS_MONITOR_LEN			(1 << 20)

#define CONFIG_X86_MRC_ADDR			0xfffa0000
#define CONFIG_X86_REFCODE_ADDR			0xffea0000
#define CONFIG_X86_REFCODE_RUN_ADDR		0

#define CONFIG_PCI_MEM_BUS	0xe0000000
#define CONFIG_PCI_MEM_PHYS	CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE	0x10000000

#define CONFIG_PCI_PREF_BUS	0xd0000000
#define CONFIG_PCI_PREF_PHYS	CONFIG_PCI_PREF_BUS
#define CONFIG_PCI_PREF_SIZE	0x10000000

#define CONFIG_PCI_IO_BUS	0x1000
#define CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE	0xefff

#define CONFIG_BIOSEMU
#define VIDEO_IO_OFFSET				0
#define CONFIG_X86EMU_RAW_IO

#undef CONFIG_ENV_SIZE
#define CONFIG_ENV_SIZE			0x1000
#define CONFIG_ENV_SECT_SIZE		0x1000
#define CONFIG_ENV_OFFSET		0x003f8000

#define CONFIG_STD_DEVICES_SETTINGS	"stdin=usbkbd,i8042-kbd,serial\0" \
					"stdout=vidconsole,serial\0" \
					"stderr=vidconsole,serial\0"

#endif
