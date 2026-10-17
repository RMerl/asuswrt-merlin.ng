/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 - Beniamino Galvani <b.galvani@gmail.com>
 */

#ifndef __GX_H__
#define __GX_H__

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

#define GX_FIRMWARE_MEM_SIZE	0x1000000

#define GX_AOBUS_BASE		0xc8100000
#define GX_PERIPHS_BASE	0xc8834400
#define GX_HIU_BASE		0xc883c000
#define GX_ETH_BASE		0xc9410000

/* Always-On Peripherals registers */
#define GX_AO_ADDR(off)	(GX_AOBUS_BASE + ((off) << 2))

#define GX_AO_SEC_GP_CFG0	GX_AO_ADDR(0x90)
#define GX_AO_SEC_GP_CFG3	GX_AO_ADDR(0x93)
#define GX_AO_SEC_GP_CFG4	GX_AO_ADDR(0x94)
#define GX_AO_SEC_GP_CFG5	GX_AO_ADDR(0x95)

#define GX_AO_BOOT_DEVICE	0xF
#define GX_AO_MEM_SIZE_MASK	0xFFFF0000
#define GX_AO_MEM_SIZE_SHIFT	16
#define GX_AO_BL31_RSVMEM_SIZE_MASK	0xFFFF0000
#define GX_AO_BL31_RSVMEM_SIZE_SHIFT	16
#define GX_AO_BL32_RSVMEM_SIZE_MASK	0xFFFF

/* Peripherals registers */
#define GX_PERIPHS_ADDR(off)	(GX_PERIPHS_BASE + ((off) << 2))

/* GPIO registers 0 to 6 */
#define _GX_GPIO_OFF(n)	((n) == 6 ? 0x08 : 0x0c + 3 * (n))
#define GX_GPIO_EN(n)		GX_PERIPHS_ADDR(_GX_GPIO_OFF(n) + 0)
#define GX_GPIO_IN(n)		GX_PERIPHS_ADDR(_GX_GPIO_OFF(n) + 1)
#define GX_GPIO_OUT(n)	GX_PERIPHS_ADDR(_GX_GPIO_OFF(n) + 2)

#endif /* __GX_H__ */
