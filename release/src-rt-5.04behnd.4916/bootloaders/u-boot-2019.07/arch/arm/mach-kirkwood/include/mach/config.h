/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Lei Wen <leiwen@marvell.com>
 */

/*
 * This file should be included in board config header file.
 *
 * It supports common definitions for Kirkwood platform
 */

#ifndef _KW_CONFIG_H
#define _KW_CONFIG_H

#if defined (CONFIG_KW88F6281)
#include <asm/arch/kw88f6281.h>
#elif defined (CONFIG_KW88F6192)
#include <asm/arch/kw88f6192.h>
#else
#error "SOC Name not defined"
#endif /* CONFIG_KW88F6281 */

#include <asm/arch/soc.h>
#define CONFIG_KIRKWOOD_EGIGA_INIT	/* Enable GbePort0/1 for kernel */
#define CONFIG_KIRKWOOD_RGMII_PAD_1V8	/* Set RGMII Pad voltage to 1.8V */
#define CONFIG_KIRKWOOD_PCIE_INIT       /* Enable PCIE Port0 for kernel */

/*
 * By default kwbimage.cfg from board specific folder is used
 * If for some board, different configuration file need to be used,
 * CONFIG_SYS_KWD_CONFIG should be defined in board specific header file
 */
#ifndef CONFIG_SYS_KWD_CONFIG
#define	CONFIG_SYS_KWD_CONFIG	$(CONFIG_BOARDDIR)/kwbimage.cfg
#endif /* CONFIG_SYS_KWD_CONFIG */

/* Kirkwood has 2k of Security SRAM, use it for SP */
#define CONFIG_SYS_INIT_SP_ADDR		0xC8012000

#define CONFIG_I2C_MVTWSI_BASE0	KW_TWSI_BASE
#define MV_UART_CONSOLE_BASE	KW_UART0_BASE
#define MV_SATA_BASE		KW_SATA_BASE
#define MV_SATA_PORT0_OFFSET	KW_SATA_PORT0_OFFSET
#define MV_SATA_PORT1_OFFSET	KW_SATA_PORT1_OFFSET

/*
 * NAND configuration
 */
#ifdef CONFIG_CMD_NAND
#define CONFIG_NAND_KIRKWOOD
#define CONFIG_SYS_NAND_BASE		0xD8000000	/* MV_DEFADR_NANDF */
#define NAND_ALLOW_ERASE_ALL		1
#endif

/*
 * Ethernet Driver configuration
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_NETCONSOLE	/* include NetConsole support   */
#define CONFIG_SYS_FAULT_ECHO_LINK_DOWN	/* detect link using phy */
#define CONFIG_ENV_OVERWRITE	/* ethaddr can be reprogrammed */
#define CONFIG_RESET_PHY_R	/* use reset_phy() to init mv8831116 PHY */
#endif /* CONFIG_CMD_NET */

/*
 * USB/EHCI
 */
#ifdef CONFIG_CMD_USB
#define CONFIG_EHCI_IS_TDI
#endif /* CONFIG_CMD_USB */

/*
 * IDE Support on SATA ports
 */
#ifdef CONFIG_IDE
#define __io
#define CONFIG_IDE_PREINIT
#define CONFIG_MVSATA_IDE_USE_PORT1
/* Needs byte-swapping for ATA data register */
#define CONFIG_IDE_SWAP_IO
/* Data, registers and alternate blocks are at the same offset */
#define CONFIG_SYS_ATA_DATA_OFFSET	(0x0100)
#define CONFIG_SYS_ATA_REG_OFFSET	(0x0100)
#define CONFIG_SYS_ATA_ALT_OFFSET	(0x0100)
/* Each 8-bit ATA register is aligned to a 4-bytes address */
#define CONFIG_SYS_ATA_STRIDE		4
/* Controller supports 48-bits LBA addressing */
#define CONFIG_LBA48
/* CONFIG_IDE requires some #defines for ATA registers */
#define CONFIG_SYS_IDE_MAXBUS		2
#define CONFIG_SYS_IDE_MAXDEVICE	2
/* ATA registers base is at SATA controller base */
#define CONFIG_SYS_ATA_BASE_ADDR	MV_SATA_BASE
#endif /* CONFIG_IDE */

/*
 * I2C related stuff
 */
#if defined(CONFIG_CMD_I2C) && !defined(CONFIG_DM_I2C)
#ifndef CONFIG_SYS_I2C_SOFT
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MVTWSI
#endif
#define CONFIG_SYS_I2C_SLAVE		0x0
#define CONFIG_SYS_I2C_SPEED		100000
#endif

/* Use common timer */
#define CONFIG_SYS_TIMER_COUNTS_DOWN
#define CONFIG_SYS_TIMER_COUNTER	(MVEBU_TIMER_BASE + 0x14)
#define CONFIG_SYS_TIMER_RATE		CONFIG_SYS_TCLK

#endif /* _KW_CONFIG_H */
