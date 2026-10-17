/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#ifndef __G12A_H__
#define __G12A_H__

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

#define G12A_AOBUS_BASE			0xff800000
#define G12A_PERIPHS_BASE		0xff634400
#define G12A_HIU_BASE			0xff63c000
#define G12A_ETH_PHY_BASE		0xff64c000
#define G12A_ETH_BASE			0xff3f0000

/* Always-On Peripherals registers */
#define G12A_AO_ADDR(off)	(G12A_AOBUS_BASE + ((off) << 2))

#define G12A_AO_SEC_GP_CFG0		G12A_AO_ADDR(0x90)
#define G12A_AO_SEC_GP_CFG3		G12A_AO_ADDR(0x93)
#define G12A_AO_SEC_GP_CFG4		G12A_AO_ADDR(0x94)
#define G12A_AO_SEC_GP_CFG5		G12A_AO_ADDR(0x95)

#define G12A_AO_BOOT_DEVICE		0xF
#define G12A_AO_MEM_SIZE_MASK		0xFFFF0000
#define G12A_AO_MEM_SIZE_SHIFT		16
#define G12A_AO_BL31_RSVMEM_SIZE_MASK	0xFFFF0000
#define G12A_AO_BL31_RSVMEM_SIZE_SHIFT	16
#define G12A_AO_BL32_RSVMEM_SIZE_MASK	0xFFFF

#endif /* __G12A_H__ */
