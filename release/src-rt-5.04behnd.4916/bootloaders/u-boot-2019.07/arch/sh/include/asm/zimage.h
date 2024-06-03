/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 *   Renesas Solutions Corp.
 *   Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 */

#ifndef _ASM_ZIMAGE_H_
#define _ASM_ZIMAGE_H_

#define MOUNT_ROOT_RDONLY	0x000
#define RAMDISK_FLAGS		0x004
#define ORIG_ROOT_DEV		0x008
#define LOADER_TYPE			0x00c
#define INITRD_START		0x010
#define INITRD_SIZE			0x014
#define COMMAND_LINE		0x100

#define RD_PROMPT			(1<<15)
#define RD_DOLOAD			(1<<14)
#define CMD_ARG_RD_PROMPT	"prompt_ramdisk="
#define CMD_ARG_RD_DOLOAD	"load_ramdisk="

#endif
