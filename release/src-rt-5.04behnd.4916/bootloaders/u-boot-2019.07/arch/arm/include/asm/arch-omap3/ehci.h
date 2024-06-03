/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * Alexander Holler <holler@ahsoftware.de>
 *
 * Based on "drivers/usb/host/ehci-omap.c" from Linux 2.6.37
 *
 * See there for additional Copyrights.
 */
#ifndef _OMAP3_EHCI_H_
#define _OMAP3_EHCI_H_

/* USB/EHCI registers */
#define OMAP_USBTLL_BASE				0x48062000UL
#define OMAP_UHH_BASE					0x48064000UL
#define OMAP_EHCI_BASE					0x48064800UL

/* TLL Register Set */
#define OMAP_USBTLL_SYSCONFIG_SOFTRESET			(1 << 1)
#define OMAP_USBTLL_SYSCONFIG_ENAWAKEUP			(1 << 2)
#define OMAP_USBTLL_SYSCONFIG_SIDLEMODE			(1 << 3)
#define OMAP_USBTLL_SYSCONFIG_CACTIVITY			(1 << 8)
#define OMAP_USBTLL_SYSSTATUS_RESETDONE			1

/* UHH Register Set */
#define OMAP_UHH_SYSCONFIG_SOFTRESET			(1 << 1)
#define OMAP_UHH_SYSCONFIG_CACTIVITY			(1 << 8)
#define OMAP_UHH_SYSCONFIG_SIDLEMODE			(1 << 3)
#define OMAP_UHH_SYSCONFIG_ENAWAKEUP			(1 << 2)
#define OMAP_UHH_SYSCONFIG_MIDLEMODE			(1 << 12)
#define OMAP_UHH_SYSSTATUS_EHCI_RESETDONE		(1 << 2)

#define OMAP_UHH_SYSCONFIG_VAL		(OMAP_UHH_SYSCONFIG_CACTIVITY | \
					OMAP_UHH_SYSCONFIG_SIDLEMODE | \
					OMAP_UHH_SYSCONFIG_ENAWAKEUP | \
					OMAP_UHH_SYSCONFIG_MIDLEMODE)

#endif /* _OMAP3_EHCI_H_ */
