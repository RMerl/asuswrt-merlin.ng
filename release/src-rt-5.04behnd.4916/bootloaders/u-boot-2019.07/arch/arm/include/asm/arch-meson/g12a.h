/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#ifndef __G12A_H__
#define __G12A_H__

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

/* Peripherals registers */
#define G12A_PERIPHS_ADDR(off)	(G12A_PERIPHS_BASE + ((off) << 2))

#define G12A_ETH_REG_0			G12A_PERIPHS_ADDR(0x50)
#define G12A_ETH_REG_1			G12A_PERIPHS_ADDR(0x51)

#define G12A_ETH_REG_0_PHY_INTF_RGMII	BIT(0)
#define G12A_ETH_REG_0_PHY_INTF_RMII	BIT(2)
#define G12A_ETH_REG_0_TX_PHASE(x)	(((x) & 3) << 5)
#define G12A_ETH_REG_0_TX_RATIO(x)	(((x) & 7) << 7)
#define G12A_ETH_REG_0_PHY_CLK_EN	BIT(10)
#define G12A_ETH_REG_0_INVERT_RMII_CLK	BIT(11)
#define G12A_ETH_REG_0_CLK_EN		BIT(12)

#define G12A_ETH_PHY_ADDR(off)	(G12A_ETH_PHY_BASE + ((off) << 2))
#define ETH_PLL_CNTL0			G12A_ETH_PHY_ADDR(0x11)
#define ETH_PLL_CNTL1			G12A_ETH_PHY_ADDR(0x12)
#define ETH_PLL_CNTL2			G12A_ETH_PHY_ADDR(0x13)
#define ETH_PLL_CNTL3			G12A_ETH_PHY_ADDR(0x14)
#define ETH_PLL_CNTL4			G12A_ETH_PHY_ADDR(0x15)
#define ETH_PLL_CNTL5			G12A_ETH_PHY_ADDR(0x16)
#define ETH_PLL_CNTL6			G12A_ETH_PHY_ADDR(0x17)
#define ETH_PLL_CNTL7			G12A_ETH_PHY_ADDR(0x18)
#define ETH_PHY_CNTL0			G12A_ETH_PHY_ADDR(0x20)
#define ETH_PHY_CNTL1			G12A_ETH_PHY_ADDR(0x21)
#define ETH_PHY_CNTL2			G12A_ETH_PHY_ADDR(0x22)

/* HIU registers */
#define G12A_HIU_ADDR(off)	(G12A_HIU_BASE + ((off) << 2))

#define G12A_MEM_PD_REG_0		G12A_HIU_ADDR(0x40)

/* Ethernet memory power domain */
#define G12A_MEM_PD_REG_0_ETH_MASK	(BIT(2) | BIT(3))

#endif /* __G12A_H__ */
