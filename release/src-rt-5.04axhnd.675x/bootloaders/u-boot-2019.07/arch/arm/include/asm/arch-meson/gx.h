/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 - Beniamino Galvani <b.galvani@gmail.com>
 */

#ifndef __GX_H__
#define __GX_H__

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

#define GX_ETH_REG_0		GX_PERIPHS_ADDR(0x50)
#define GX_ETH_REG_1		GX_PERIPHS_ADDR(0x51)
#define GX_ETH_REG_2		GX_PERIPHS_ADDR(0x56)
#define GX_ETH_REG_3		GX_PERIPHS_ADDR(0x57)

#define GX_ETH_REG_0_PHY_INTF		BIT(0)
#define GX_ETH_REG_0_TX_PHASE(x)	(((x) & 3) << 5)
#define GX_ETH_REG_0_TX_RATIO(x)	(((x) & 7) << 7)
#define GX_ETH_REG_0_PHY_CLK_EN	BIT(10)
#define GX_ETH_REG_0_INVERT_RMII_CLK	BIT(11)
#define GX_ETH_REG_0_CLK_EN		BIT(12)

/* HIU registers */
#define GX_HIU_ADDR(off)	(GX_HIU_BASE + ((off) << 2))

#define GX_MEM_PD_REG_0	GX_HIU_ADDR(0x40)

/* Ethernet memory power domain */
#define GX_MEM_PD_REG_0_ETH_MASK	(BIT(2) | BIT(3))

#endif /* __GX_H__ */
