/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * SAMSUNG EXYNOS USB HOST EHCI Controller
 *
 * Copyright (C) 2012 Samsung Electronics Co.Ltd
 *	Vivek Gautam <gautam.vivek@samsung.com>
 */

#ifndef __ASM_ARM_ARCH_EHCI_H__
#define __ASM_ARM_ARCH_EHCI_H__

#define CLK_24MHZ		5

#define PHYPWR_NORMAL_MASK_PHY0                 (0x39 << 0)
#define PHYPWR_NORMAL_MASK_PHY1                 (0x7 << 6)
#define PHYPWR_NORMAL_MASK_HSIC0                (0x7 << 9)
#define PHYPWR_NORMAL_MASK_HSIC1                (0x7 << 12)
#define RSTCON_HOSTPHY_SWRST                    (0xf << 3)
#define RSTCON_SWRST                            (0x1 << 0)

#define HOST_CTRL0_PHYSWRSTALL			(1 << 31)
#define HOST_CTRL0_COMMONON_N			(1 << 9)
#define HOST_CTRL0_SIDDQ			(1 << 6)
#define HOST_CTRL0_FORCESLEEP			(1 << 5)
#define HOST_CTRL0_FORCESUSPEND			(1 << 4)
#define HOST_CTRL0_WORDINTERFACE		(1 << 3)
#define HOST_CTRL0_UTMISWRST			(1 << 2)
#define HOST_CTRL0_LINKSWRST			(1 << 1)
#define HOST_CTRL0_PHYSWRST			(1 << 0)

#define HOST_CTRL0_FSEL_MASK			(7 << 16)

#define EHCICTRL_ENAINCRXALIGN			(1 << 29)
#define EHCICTRL_ENAINCR4			(1 << 28)
#define EHCICTRL_ENAINCR8			(1 << 27)
#define EHCICTRL_ENAINCR16			(1 << 26)

#define HSIC_CTRL_REFCLKSEL                     (0x2)
#define HSIC_CTRL_REFCLKSEL_MASK                (0x3)
#define HSIC_CTRL_REFCLKSEL_SHIFT               (23)

#define HSIC_CTRL_REFCLKDIV_12                  (0x24)
#define HSIC_CTRL_REFCLKDIV_MASK                (0x7f)
#define HSIC_CTRL_REFCLKDIV_SHIFT               (16)

#define HSIC_CTRL_SIDDQ                         (0x1 << 6)
#define HSIC_CTRL_FORCESLEEP                    (0x1 << 5)
#define HSIC_CTRL_FORCESUSPEND                  (0x1 << 4)
#define HSIC_CTRL_UTMISWRST                     (0x1 << 2)
#define HSIC_CTRL_PHYSWRST                      (0x1 << 0)

/* Register map for PHY control */
struct exynos_usb_phy {
	unsigned int usbphyctrl0;
	unsigned int usbphytune0;
	unsigned int reserved1[2];
	unsigned int hsicphyctrl1;
	unsigned int hsicphytune1;
	unsigned int reserved2[2];
	unsigned int hsicphyctrl2;
	unsigned int hsicphytune2;
	unsigned int reserved3[2];
	unsigned int ehcictrl;
	unsigned int ohcictrl;
	unsigned int usbotgsys;
	unsigned int reserved4;
	unsigned int usbotgtune;
};

struct exynos4412_usb_phy {
	unsigned int usbphyctrl;
	unsigned int usbphyclk;
	unsigned int usbphyrstcon;
};

/* Switch on the VBUS power. */
int board_usb_vbus_init(void);

#endif /* __ASM_ARM_ARCH_EHCI_H__ */
