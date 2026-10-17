/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Modified from coreboot bochs.c
 */

#ifndef __BOCHS_H
#define __BOCHS_H

#define VGA_INDEX		0x3c0

#define VGA_ATT_W		0x3c0
#define VGA_AR_ENABLE_DISPLAY	0x20

enum {
	INDEX_ID,
	INDEX_XRES,
	INDEX_YRES,
	INDEX_BPP,
	INDEX_ENABLE,
	INDEX_BANK,
	INDEX_VIRT_WIDTH,
	INDEX_VIRT_HEIGHT,
	INDEX_X_OFFSET,
	INDEX_Y_OFFSET,
	INDEX_VIDEO_MEMORY_64K
};

#define ID0		0xb0c0

#define ENABLED		BIT(0)
#define LFB_ENABLED	BIT(6)
#define NOCLEARMEM	BIT(7)

#define VGA_BASE	0x400
#define MMIO_BASE	0x500

#endif
