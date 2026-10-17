/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 NXP
 */

#ifndef __LX2160_H
#define __LX2160_H

#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS)
/* SYSCLK */
#define QIXIS_SYSCLK_100		0x0
#define QIXIS_SYSCLK_125		0x1
#define QIXIS_SYSCLK_133		0x2

/* DDRCLK */
#define QIXIS_DDRCLK_100		0x0
#define QIXIS_DDRCLK_125		0x1
#define QIXIS_DDRCLK_133		0x2

#define BRDCFG4_EMI1SEL_MASK		0xF8
#define BRDCFG4_EMI1SEL_SHIFT		3
#define BRDCFG4_EMI2SEL_MASK		0x07
#define BRDCFG4_EMI2SEL_SHIFT		0
#endif

#define QIXIS_XMAP_SHIFT		5

/* RTC */
#define I2C_MUX_CH_RTC			0xB

/* MAC/PHY configuration */
#if defined(CONFIG_FSL_MC_ENET)
#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS)
#define AQ_PHY_ADDR1			0x00
#define AQ_PHY_ADDR2			0x01
#define AQ_PHY_ADDR3			0x02
#define AQ_PHY_ADDR4			0x03
#endif

#ifdef CONFIG_TARGET_LX2160ARDB
#define AQR107_PHY_ADDR1		0x04
#define AQR107_PHY_ADDR2		0x05
#define AQR107_IRQ_MASK			0x0C
#endif

#define CORTINA_PHY_ADDR1		0x0
#define INPHI_PHY_ADDR1			0x0

#define RGMII_PHY_ADDR1			0x01
#define RGMII_PHY_ADDR2			0x02

#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS)
#define INPHI_PHY_ADDR2			0x1
#define SGMII_CARD_PORT1_PHY_ADDR	0x1C
#define SGMII_CARD_PORT2_PHY_ADDR	0x1D
#define SGMII_CARD_PORT3_PHY_ADDR	0x1E
#define SGMII_CARD_PORT4_PHY_ADDR	0x1F
#endif
#endif

#if IS_ENABLED(CONFIG_TARGET_LX2160ARDB)
u8 get_board_rev(void);
int fdt_fixup_board_phy_revc(void *fdt);
#else
static inline u8 get_board_rev(void)
{
	return 0;
}

static inline int fdt_fixup_board_phy_revc(void *fdt)
{
	return 0;
}
#endif

#endif /* __LX2160_H */
