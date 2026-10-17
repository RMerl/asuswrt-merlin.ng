/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#ifndef __AXG_H__
#define __AXG_H__

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

#define AXG_AOBUS_BASE		0xff800000
#define AXG_PERIPHS_BASE	0xff634400
#define AXG_HIU_BASE		0xff63c000
#define AXG_ETH_BASE		0xff3f0000

/* Always-On Peripherals registers */
#define AXG_AO_ADDR(off)	(AXG_AOBUS_BASE + ((off) << 2))

#define AXG_AO_SEC_GP_CFG0	AXG_AO_ADDR(0x90)
#define AXG_AO_SEC_GP_CFG3	AXG_AO_ADDR(0x93)
#define AXG_AO_SEC_GP_CFG4	AXG_AO_ADDR(0x94)
#define AXG_AO_SEC_GP_CFG5	AXG_AO_ADDR(0x95)

#define AXG_AO_BOOT_DEVICE	0xF
#define AXG_AO_MEM_SIZE_MASK	0xFFFF0000
#define AXG_AO_MEM_SIZE_SHIFT	16
#define AXG_AO_BL31_RSVMEM_SIZE_MASK	0xFFFF0000
#define AXG_AO_BL31_RSVMEM_SIZE_SHIFT	16
#define AXG_AO_BL32_RSVMEM_SIZE_MASK	0xFFFF

#endif /* __AXG_H__ */
