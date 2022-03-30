/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#ifndef __AXG_H__
#define __AXG_H__

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

/* Peripherals registers */
#define AXG_PERIPHS_ADDR(off)	(AXG_PERIPHS_BASE + ((off) << 2))

#define AXG_ETH_REG_0		AXG_PERIPHS_ADDR(0x50)
#define AXG_ETH_REG_1		AXG_PERIPHS_ADDR(0x51)

#define AXG_ETH_REG_0_PHY_INTF_RGMII	BIT(0)
#define AXG_ETH_REG_0_PHY_INTF_RMII	BIT(2)
#define AXG_ETH_REG_0_TX_PHASE(x)	(((x) & 3) << 5)
#define AXG_ETH_REG_0_TX_RATIO(x)	(((x) & 7) << 7)
#define AXG_ETH_REG_0_PHY_CLK_EN	BIT(10)
#define AXG_ETH_REG_0_INVERT_RMII_CLK	BIT(11)
#define AXG_ETH_REG_0_CLK_EN		BIT(12)

/* HIU registers */
#define AXG_HIU_ADDR(off)	(AXG_HIU_BASE + ((off) << 2))

#define AXG_MEM_PD_REG_0	AXG_HIU_ADDR(0x40)

/* Ethernet memory power domain */
#define AXG_MEM_PD_REG_0_ETH_MASK	(BIT(2) | BIT(3))

#endif /* __AXG_H__ */
