/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Evgeni Dobrev <evgeni@studio-punkt.com>
 *
 * based on work from:
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#ifndef _CONFIG_NAS220_H
#define _CONFIG_NAS220_H

/*
 * Machine type ID
 */
#define CONFIG_MACH_TYPE		MACH_TYPE_RD88F6192_NAS

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_FEROCEON_88FR131		/* #define CPU Core subversion */
#define CONFIG_KW88F6192		/* SOC Name */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */

/* power-on led, regulator, sata0, sata1 */
#define NAS220_GE_OE_VAL_LOW ((1 << 12)|(1 << 14)|(1 << 24)|(1 << 28))
#define NAS220_GE_OE_VAL_HIGH (0)
#define NAS220_GE_OE_LOW (~((1 << 12)|(1 << 14)|(1 << 24)|(1 << 28)))
#define NAS220_GE_OE_HIGH (~(0))

/* PHY related */
#define MV88E1116_LED_FCTRL_REG		10
#define MV88E1116_CPRSP_CR3_REG		21
#define MV88E1116_MAC_CTRL_REG		21
#define MV88E1116_PGADR_REG		22
#define MV88E1116_RGMII_TXTM_CTRL	(1 << 4)
#define MV88E1116_RGMII_RXTM_CTRL	(1 << 5)

/*
 * Commands configuration
 */

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/*
 *  Environment variables configurations
 */
#ifdef CONFIG_CMD_NAND
#define CONFIG_ENV_SECT_SIZE 0x10000
#endif

#define CONFIG_ENV_SIZE	0x10000
#define CONFIG_ENV_OFFSET 0xa0000

/*
 * Default environment variables
 */
#define CONFIG_BOOTCOMMAND ""

#define CONFIG_EXTRA_ENV_SETTINGS \
	"bootargs=console=ttyS0,115200\0" \
	"mtdparts=mtdparts=orion_nand:0xa0000@0x0(uboot),"\
	"0x010000@0xa0000(env),"\
	"0x500000@0xc0000(uimage),"\
	"0x1a40000@0x5c0000(rootfs)\0" \
	"mtdids=nand0=orion_nand\0"\
	"autostart=no\0"\
	"autoload=no\0"

/*
 * Ethernet Driver configuration
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_MVGBE_PORTS {1, 0}	/* enable port 0 only */
#define CONFIG_PHY_BASE_ADR 8
#endif /* CONFIG_CMD_NET */

/*
 * USB/EHCI
 */
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_EHCI_KIRKWOOD	/* on Kirkwood platform	*/
#define CONFIG_EHCI_IS_TDI
#endif /* CONFIG_CMD_USB */

/*
 * File system
 */
#define CONFIG_JFFS2_NAND
#define CONFIG_JFFS2_LZO

/*
 * SATA
 */
#ifdef CONFIG_MVSATA_IDE
#define CONFIG_SYS_ATA_IDE0_OFFSET      MV_SATA_PORT0_OFFSET
#define CONFIG_SYS_ATA_IDE1_OFFSET      MV_SATA_PORT1_OFFSET
#endif

/*
 * EFI partition
 */

#define CONFIG_KIRKWOOD_GPIO

#endif /* _CONFIG_NAS220_H */

