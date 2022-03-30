/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Freescale USB PHY Register Definitions
 *
 */

#ifndef __REGS_USBPHY_H__
#define __REGS_USBPHY_H__

#define USBPHY_CTRL						0x00000030
#define USBPHY_CTRL_SET					0x00000034
#define USBPHY_CTRL_CLR					0x00000038
#define USBPHY_CTRL_TOG					0x0000003C
#define USBPHY_PWD						0x00000000
#define USBPHY_TX						0x00000010
#define USBPHY_RX						0x00000020
#define USBPHY_DEBUG					0x00000050

#define USBPHY_CTRL_ENUTMILEVEL2		(1 << 14)
#define USBPHY_CTRL_ENUTMILEVEL3		(1 << 15)
#define USBPHY_CTRL_OTG_ID				(1 << 27)
#define USBPHY_CTRL_CLKGATE				(1 << 30)
#define USBPHY_CTRL_SFTRST				(1 << 31)

#endif /* __REGS_USBPHY_H__ */
