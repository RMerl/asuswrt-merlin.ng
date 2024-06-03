/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * eInfochips Ltd. <www.einfochips.com>
 * Written-by: Ajay Bhargav <contact@8051projects.net>
 *
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 */

#ifndef __MVGPIO_H__
#define __MVGPIO_H__

#include <common.h>

/*
 * GPIO Register map for Marvell SOCs
 */
struct gpio_reg {
	u32 gplr;	/* Pin Level Register - 0x0000 */
	u32 pad0[2];
	u32 gpdr;	/* Pin Direction Register - 0x000C */
	u32 pad1[2];
	u32 gpsr;	/* Pin Output Set Register - 0x0018 */
	u32 pad2[2];
	u32 gpcr;	/* Pin Output Clear Register - 0x0024 */
	u32 pad3[2];
	u32 grer;	/* Rising-Edge Detect Enable Register - 0x0030 */
	u32 pad4[2];
	u32 gfer;	/* Falling-Edge Detect Enable Register - 0x003C */
	u32 pad5[2];
	u32 gedr;	/* Edge Detect Status Register - 0x0048 */
	u32 pad6[2];
	u32 gsdr;	/* Bitwise Set of GPIO Direction Register - 0x0054 */
	u32 pad7[2];
	u32 gcdr;	/* Bitwise Clear of GPIO Direction Register - 0x0060 */
	u32 pad8[2];
	u32 gsrer;	/* Bitwise Set of Rising-Edge Detect Enable
			   Register - 0x006C */
	u32 pad9[2];
	u32 gcrer;	/* Bitwise Clear of Rising-Edge Detect Enable
			   Register - 0x0078 */
	u32 pad10[2];
	u32 gsfer;	/* Bitwise Set of Falling-Edge Detect Enable
			   Register - 0x0084 */
	u32 pad11[2];
	u32 gcfer;	/* Bitwise Clear of Falling-Edge Detect Enable
			   Register - 0x0090 */
	u32 pad12[2];
	u32 apmask;	/* Bitwise Mask of Edge Detect Register - 0x009C */
};

#endif /* __MVGPIO_H__ */
