/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Toradex Colibri PXA270 configuration file
 *
 * Copyright (C) 2010 Marek Vasut <marek.vasut@gmail.com>
 */

#ifndef	__CONFIG_PXA_COMMON_H__
#define	__CONFIG_PXA_COMMON_H__

#define	CONFIG_SYS_ARM_CACHE_WRITETHROUGH

/*
 * KGDB
 */
#ifdef	CONFIG_CMD_KGDB
#define	CONFIG_KGDB_BAUDRATE		230400
#endif

/*
 * MMC Card Configuration
 */
#ifdef	CONFIG_CMD_MMC
#define	CONFIG_PXA_MMC_GENERIC
#endif

/*
 * OHCI USB
 */
#ifdef	CONFIG_CMD_USB
#define	CONFIG_USB_OHCI_NEW
#define	CONFIG_SYS_USB_OHCI_CPU_INIT
#define	CONFIG_SYS_USB_OHCI_BOARD_INIT
#ifdef CONFIG_CPU_PXA27X
#define	CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	3
#else
#define	CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2
#endif
#define	CONFIG_SYS_USB_OHCI_REGS_BASE		0x4c000000
#define	CONFIG_SYS_USB_OHCI_SLOT_NAME		"pxa-ohci"
#endif

#endif	/* __CONFIG_PXA_COMMON_H__ */
