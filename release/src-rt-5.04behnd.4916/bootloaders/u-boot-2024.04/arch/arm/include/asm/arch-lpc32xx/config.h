/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common definitions for LPC32XX board configurations
 *
 * Copyright (C) 2011-2015 Vladimir Zapolskiy <vz@mleia.com>
 */

#ifndef _LPC32XX_CONFIG_H
#define _LPC32XX_CONFIG_H


/* Basic CPU architecture */

#if !defined(CFG_SYS_NS16550_CLK)
#define CFG_SYS_NS16550_CLK		13000000
#endif

#define CFG_SYS_BAUDRATE_TABLE	\
		{ 9600, 19200, 38400, 57600, 115200, 230400, 460800 }

/* NAND */
#if defined(CONFIG_NAND_LPC32XX_SLC)
#define NAND_LARGE_BLOCK_PAGE_SIZE	0x800
#define NAND_SMALL_BLOCK_PAGE_SIZE	0x200

#if (CONFIG_SYS_NAND_PAGE_SIZE == NAND_LARGE_BLOCK_PAGE_SIZE)
#define CFG_SYS_NAND_ECCPOS		{ 40, 41, 42, 43, 44, 45, 46, 47, \
					  48, 49, 50, 51, 52, 53, 54, 55, \
					  56, 57, 58, 59, 60, 61, 62, 63, }
#elif (CONFIG_SYS_NAND_PAGE_SIZE == NAND_SMALL_BLOCK_PAGE_SIZE)
#define CFG_SYS_NAND_ECCPOS		{ 10, 11, 12, 13, 14, 15, }
#else
#error "CONFIG_SYS_NAND_PAGE_SIZE set to an invalid value"
#endif

#define CFG_SYS_NAND_ECCSIZE		0x100
#define CFG_SYS_NAND_ECCBYTES	3
#endif	/* CONFIG_NAND_LPC32XX_SLC */

/* NOR Flash */

/* USB OHCI */
#if defined(CONFIG_USB_OHCI_LPC32XX)
#define CFG_SYS_USB_OHCI_REGS_BASE		USB_BASE
#endif

#endif /* _LPC32XX_CONFIG_H */
