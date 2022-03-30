/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2010 Albert ARIBAUD <albert.u.boot@aribaud.net>
 *
 * Based on original Kirkwood support which is
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * Header file for Marvell's Orion SoC with Feroceon CPU core.
 */

#ifndef _ASM_ARCH_ORION5X_H
#define _ASM_ARCH_ORION5X_H

#if defined(CONFIG_FEROCEON)

/* SOC specific definations */
#define ORION5X_REGISTER(x)			(ORION5X_REGS_PHY_BASE + x)

/* Documented registers */
#define ORION5X_DRAM_BASE			(ORION5X_REGISTER(0x01500))
#define ORION5X_TWSI_BASE			(ORION5X_REGISTER(0x11000))
#define ORION5X_UART0_BASE			(ORION5X_REGISTER(0x12000))
#define ORION5X_UART1_BASE			(ORION5X_REGISTER(0x12100))
#define ORION5X_MPP_BASE			(ORION5X_REGISTER(0x10000))
#define ORION5X_GPIO_BASE			(ORION5X_REGISTER(0x10100))
#define ORION5X_CPU_WIN_BASE			(ORION5X_REGISTER(0x20000))
#define ORION5X_CPU_REG_BASE			(ORION5X_REGISTER(0x20100))
#define ORION5X_TIMER_BASE			(ORION5X_REGISTER(0x20300))
#define ORION5X_REG_PCI_BASE			(ORION5X_REGISTER(0x30000))
#define ORION5X_REG_PCIE_BASE			(ORION5X_REGISTER(0x40000))
#define ORION5X_USB20_PORT0_BASE		(ORION5X_REGISTER(0x50000))
#define ORION5X_USB20_PORT1_BASE		(ORION5X_REGISTER(0xA0000))
#define ORION5X_EGIGA_BASE			(ORION5X_REGISTER(0x72000))
#define ORION5X_SATA_BASE			(ORION5X_REGISTER(0x80000))
#define ORION5X_SATA_PORT0_OFFSET		0x2000
#define ORION5X_SATA_PORT1_OFFSET		0x4000

/* Orion5x GbE controller has a single port */
#define MAX_MVGBE_DEVS	1
#define MVGBE0_BASE	ORION5X_EGIGA_BASE

/* Orion5x USB Host controller is port 1 */
#define MVUSB0_BASE			ORION5X_USB20_HOST_PORT_BASE
#define MVUSB0_CPU_ATTR_DRAM_CS0	ORION5X_ATTR_DRAM_CS0
#define MVUSB0_CPU_ATTR_DRAM_CS1	ORION5X_ATTR_DRAM_CS1
#define MVUSB0_CPU_ATTR_DRAM_CS2	ORION5X_ATTR_DRAM_CS2
#define MVUSB0_CPU_ATTR_DRAM_CS3	ORION5X_ATTR_DRAM_CS3

/* Kirkwood CPU memory windows */
#define MVCPU_WIN_CTRL_DATA	ORION5X_CPU_WIN_CTRL_DATA
#define MVCPU_WIN_ENABLE	ORION5X_WIN_ENABLE
#define MVCPU_WIN_DISABLE	ORION5X_WIN_DISABLE

#define CONFIG_MAX_RAM_BANK_SIZE		(64*1024*1024)

/* include here SoC variants. 5181, 5281, 6183 should go here when
   adding support for them, and this comment should then be updated. */
#if defined(CONFIG_88F5182)
#include <asm/arch/mv88f5182.h>
#else
#error "SOC Name not defined"
#endif
#endif /* CONFIG_FEROCEON */
#endif /* _ASM_ARCH_ORION5X_H */
