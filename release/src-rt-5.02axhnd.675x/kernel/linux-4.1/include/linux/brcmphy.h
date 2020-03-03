#ifndef _LINUX_BRCMPHY_H
#define _LINUX_BRCMPHY_H

#define PHY_ID_BCM50610			0x0143bd60
#define PHY_ID_BCM50610M		0x0143bd70
#define PHY_ID_BCM5241			0x0143bc30
#define PHY_ID_BCMAC131			0x0143bc70
#define PHY_ID_BCM5481			0x0143bca0
#define PHY_ID_BCM5482			0x0143bcb0
#define PHY_ID_BCM5411			0x00206070
#define PHY_ID_BCM5421			0x002060e0
#define PHY_ID_BCM5464			0x002060b0
#define PHY_ID_BCM5461			0x002060c0
#define PHY_ID_BCM54616S		0x03625d10
#define PHY_ID_BCM57780			0x03625d90

#define PHY_ID_BCM7250			0xae025280
#define PHY_ID_BCM7364			0xae025260
#define PHY_ID_BCM7366			0x600d8490
#define PHY_ID_BCM7425			0x600d86b0
#define PHY_ID_BCM7429			0x600d8730
#define PHY_ID_BCM7439			0x600d8480
#define PHY_ID_BCM7439_2		0xae025080
#define PHY_ID_BCM7445			0x600d8510

#define PHY_BCM_OUI_MASK		0xfffffc00
#define PHY_BCM_OUI_1			0x00206000
#define PHY_BCM_OUI_2			0x0143bc00
#define PHY_BCM_OUI_3			0x03625c00
#define PHY_BCM_OUI_4			0x600d8400
#define PHY_BCM_OUI_5			0x03625e00
#define PHY_BCM_OUI_6			0xae025000

#define PHY_BCM_FLAGS_MODE_COPPER	0x00000001
#define PHY_BCM_FLAGS_MODE_1000BX	0x00000002
#define PHY_BCM_FLAGS_INTF_SGMII	0x00000010
#define PHY_BCM_FLAGS_INTF_XAUI		0x00000020
#define PHY_BRCM_WIRESPEED_ENABLE	0x00000100
#define PHY_BRCM_AUTO_PWRDWN_ENABLE	0x00000200
#define PHY_BRCM_RX_REFCLK_UNUSED	0x00000400
#define PHY_BRCM_STD_IBND_DISABLE	0x00000800
#define PHY_BRCM_EXT_IBND_RX_ENABLE	0x00001000
#define PHY_BRCM_EXT_IBND_TX_ENABLE	0x00002000
#define PHY_BRCM_CLEAR_RGMII_MODE	0x00004000
#define PHY_BRCM_DIS_TXCRXC_NOENRGY	0x00008000
/* Broadcom BCM7xxx specific workarounds */
#define PHY_BRCM_7XXX_REV(x)		(((x) >> 8) & 0xff)
#define PHY_BRCM_7XXX_PATCH(x)		((x) & 0xff)
#define PHY_BCM_FLAGS_VALID		0x80000000

/* Broadcom BCM54XX register definitions, common to most Broadcom PHYs */
#define MII_BCM54XX_ECR		0x10	/* BCM54xx extended control register */
#define MII_BCM54XX_ECR_IM	0x1000	/* Interrupt mask */
#define MII_BCM54XX_ECR_IF	0x0800	/* Interrupt force */

#define MII_BCM54XX_ESR		0x11	/* BCM54xx extended status register */
#define MII_BCM54XX_ESR_IS	0x1000	/* Interrupt status */

#define MII_BCM54XX_EXP_DATA	0x15	/* Expansion register data */
#define MII_BCM54XX_EXP_SEL	0x17	/* Expansion register select */
#define MII_BCM54XX_EXP_SEL_SSD	0x0e00	/* Secondary SerDes select */
#define MII_BCM54XX_EXP_SEL_ER	0x0f00	/* Expansion register select */

#define MII_BCM54XX_AUX_CTL	0x18	/* Auxiliary control register */
#define MII_BCM54XX_ISR		0x1a	/* BCM54xx interrupt status register */
#define MII_BCM54XX_IMR		0x1b	/* BCM54xx interrupt mask register */
#define MII_BCM54XX_INT_CRCERR	0x0001	/* CRC error */
#define MII_BCM54XX_INT_LINK	0x0002	/* Link status changed */
#define MII_BCM54XX_INT_SPEED	0x0004	/* Link speed change */
#define MII_BCM54XX_INT_DUPLEX	0x0008	/* Duplex mode changed */
#define MII_BCM54XX_INT_LRS	0x0010	/* Local receiver status changed */
#define MII_BCM54XX_INT_RRS	0x0020	/* Remote receiver status changed */
#define MII_BCM54XX_INT_SSERR	0x0040	/* Scrambler synchronization error */
#define MII_BCM54XX_INT_UHCD	0x0080	/* Unsupported HCD negotiated */
#define MII_BCM54XX_INT_NHCD	0x0100	/* No HCD */
#define MII_BCM54XX_INT_NHCDL	0x0200	/* No HCD link */
#define MII_BCM54XX_INT_ANPR	0x0400	/* Auto-negotiation page received */
#define MII_BCM54XX_INT_LC	0x0800	/* All counters below 128 */
#define MII_BCM54XX_INT_HC	0x1000	/* Counter above 32768 */
#define MII_BCM54XX_INT_MDIX	0x2000	/* MDIX status change */
#define MII_BCM54XX_INT_PSERR	0x4000	/* Pair swap error */

#define MII_BCM54XX_SHD		0x1c	/* 0x1c shadow registers */
#define MII_BCM54XX_SHD_WRITE	0x8000
#define MII_BCM54XX_SHD_VAL(x)	((x & 0x1f) << 10)
#define MII_BCM54XX_SHD_DATA(x)	((x & 0x3ff) << 0)

/*
 * AUXILIARY CONTROL SHADOW ACCESS REGISTERS.  (PHY REG 0x18)
 */
#define MII_BCM54XX_AUXCTL_SHDWSEL_AUXCTL	0x0000
#define MII_BCM54XX_AUXCTL_ACTL_TX_6DB		0x0400
#define MII_BCM54XX_AUXCTL_ACTL_SMDSP_ENA	0x0800

#define MII_BCM54XX_AUXCTL_MISC_WREN	0x8000
#define MII_BCM54XX_AUXCTL_MISC_FORCE_AMDIX	0x0200
#define MII_BCM54XX_AUXCTL_MISC_RDSEL_MISC	0x7000
#define MII_BCM54XX_AUXCTL_SHDWSEL_MISC	0x0007

#define MII_BCM54XX_AUXCTL_SHDWSEL_AUXCTL	0x0000

/*
 * Broadcom LED source encodings.  These are used in BCM5461, BCM5481,
 * BCM5482, and possibly some others.
 */
#define BCM_LED_SRC_LINKSPD1	0x0
#define BCM_LED_SRC_LINKSPD2	0x1
#define BCM_LED_SRC_XMITLED	0x2
#define BCM_LED_SRC_ACTIVITYLED	0x3
#define BCM_LED_SRC_FDXLED	0x4
#define BCM_LED_SRC_SLAVE	0x5
#define BCM_LED_SRC_INTR	0x6
#define BCM_LED_SRC_QUALITY	0x7
#define BCM_LED_SRC_RCVLED	0x8
#define BCM_LED_SRC_MULTICOLOR1	0xa
#define BCM_LED_SRC_OPENSHORT	0xb
#define BCM_LED_SRC_OFF		0xe	/* Tied high */
#define BCM_LED_SRC_ON		0xf	/* Tied low */


/*
 * BCM5482: Shadow registers
 * Shadow values go into bits [14:10] of register 0x1c to select a shadow
 * register to access.
 */
/* 00101: Spare Control Register 3 */
#define BCM54XX_SHD_SCR3		0x05
#define  BCM54XX_SHD_SCR3_DEF_CLK125	0x0001
#define  BCM54XX_SHD_SCR3_DLLAPD_DIS	0x0002
#define  BCM54XX_SHD_SCR3_TRDDAPD	0x0004

/* 01010: Auto Power-Down */
#define BCM54XX_SHD_APD			0x0a
#define  BCM54XX_SHD_APD_EN		0x0020

#define BCM5482_SHD_LEDS1	0x0d	/* 01101: LED Selector 1 */
					/* LED3 / ~LINKSPD[2] selector */
#define BCM5482_SHD_LEDS1_LED3(src)	((src & 0xf) << 4)
					/* LED1 / ~LINKSPD[1] selector */
#define BCM5482_SHD_LEDS1_LED1(src)	((src & 0xf) << 0)
#define BCM54XX_SHD_RGMII_MODE	0x0b	/* 01011: RGMII Mode Selector */
#define BCM5482_SHD_SSD		0x14	/* 10100: Secondary SerDes control */
#define BCM5482_SHD_SSD_LEDM	0x0008	/* SSD LED Mode enable */
#define BCM5482_SHD_SSD_EN	0x0001	/* SSD enable */
#define BCM5482_SHD_MODE	0x1f	/* 11111: Mode Control Register */
#define BCM5482_SHD_MODE_1000BX	0x0001	/* Enable 1000BASE-X registers */


/*
 * EXPANSION SHADOW ACCESS REGISTERS.  (PHY REG 0x15, 0x16, and 0x17)
 */
#define MII_BCM54XX_EXP_AADJ1CH0		0x001f
#define  MII_BCM54XX_EXP_AADJ1CH0_SWP_ABCD_OEN	0x0200
#define  MII_BCM54XX_EXP_AADJ1CH0_SWSEL_THPF	0x0100
#define MII_BCM54XX_EXP_AADJ1CH3		0x601f
#define  MII_BCM54XX_EXP_AADJ1CH3_ADCCKADJ	0x0002
#define MII_BCM54XX_EXP_EXP08			0x0F08
#define  MII_BCM54XX_EXP_EXP08_RJCT_2MHZ	0x0001
#define  MII_BCM54XX_EXP_EXP08_EARLY_DAC_WAKE	0x0200
#define MII_BCM54XX_EXP_EXP75			0x0f75
#define  MII_BCM54XX_EXP_EXP75_VDACCTRL		0x003c
#define  MII_BCM54XX_EXP_EXP75_CM_OSC		0x0001
#define MII_BCM54XX_EXP_EXP96			0x0f96
#define  MII_BCM54XX_EXP_EXP96_MYST		0x0010
#define MII_BCM54XX_EXP_EXP97			0x0f97
#define  MII_BCM54XX_EXP_EXP97_MYST		0x0c0c

/*
 * BCM5482: Secondary SerDes registers
 */
#define BCM5482_SSD_1000BX_CTL		0x00	/* 1000BASE-X Control */
#define BCM5482_SSD_1000BX_CTL_PWRDOWN	0x0800	/* Power-down SSD */
#define BCM5482_SSD_SGMII_SLAVE		0x15	/* SGMII Slave Register */
#define BCM5482_SSD_SGMII_SLAVE_EN	0x0002	/* Slave mode enable */
#define BCM5482_SSD_SGMII_SLAVE_AD	0x0001	/* Slave auto-detection */


/*****************************************************************************/
/* Fast Ethernet Transceiver definitions. */
/*****************************************************************************/

#define MII_BRCM_FET_INTREG		0x1a	/* Interrupt register */
#define MII_BRCM_FET_IR_MASK		0x0100	/* Mask all interrupts */
#define MII_BRCM_FET_IR_LINK_EN		0x0200	/* Link status change enable */
#define MII_BRCM_FET_IR_SPEED_EN	0x0400	/* Link speed change enable */
#define MII_BRCM_FET_IR_DUPLEX_EN	0x0800	/* Duplex mode change enable */
#define MII_BRCM_FET_IR_ENABLE		0x4000	/* Interrupt enable */

#define MII_BRCM_FET_BRCMTEST		0x1f	/* Brcm test register */
#define MII_BRCM_FET_BT_SRE		0x0080	/* Shadow register enable */


/*** Shadow register definitions ***/

#define MII_BRCM_FET_SHDW_MISCCTRL	0x10	/* Shadow misc ctrl */
#define MII_BRCM_FET_SHDW_MC_FAME	0x4000	/* Force Auto MDIX enable */

#define MII_BRCM_FET_SHDW_AUXMODE4	0x1a	/* Auxiliary mode 4 */
#define MII_BRCM_FET_SHDW_AM4_LED_MASK	0x0003
#define MII_BRCM_FET_SHDW_AM4_LED_MODE1 0x0001

#define MII_BRCM_FET_SHDW_AUXSTAT2	0x1b	/* Auxiliary status 2 */
#define MII_BRCM_FET_SHDW_AS2_APDE	0x0020	/* Auto power down enable */

/*
 * Indirect register access functions for the 1000BASE-T/100BASE-TX/10BASE-T
 * 0x1c shadow registers.
 */
static inline int bcm54xx_shadow_read(struct phy_device *phydev, u16 shadow)
{
	phy_write(phydev, MII_BCM54XX_SHD, MII_BCM54XX_SHD_VAL(shadow));
	return MII_BCM54XX_SHD_DATA(phy_read(phydev, MII_BCM54XX_SHD));
}

static inline int bcm54xx_shadow_write(struct phy_device *phydev, u16 shadow,
				       u16 val)
{
	return phy_write(phydev, MII_BCM54XX_SHD,
			 MII_BCM54XX_SHD_WRITE |
			 MII_BCM54XX_SHD_VAL(shadow) |
			 MII_BCM54XX_SHD_DATA(val));
}

#define BRCM_CL45VEN_EEE_CONTROL	0x803d
#define LPI_FEATURE_EN			0x8000
#define LPI_FEATURE_EN_DIG1000X		0x4000

#endif /* _LINUX_BRCMPHY_H */
