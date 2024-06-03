/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 by Vladimir Zapolskiy <vz@mleia.com>
 */

#ifndef _LPC32XX_CPU_H
#define _LPC32XX_CPU_H

/* LPC32XX Memory map */

/* AHB physical base addresses */
#define SLC_NAND_BASE	0x20020000	/* SLC NAND Flash registers base    */
#define SSP0_BASE	0x20084000	/* SSP0 registers base              */
#define SD_CARD_BASE	0x20098000	/* SD card interface registers base */
#define MLC_NAND_BASE	0x200A8000	/* MLC NAND Flash registers base    */
#define DMA_BASE	0x31000000	/* DMA controller registers base    */
#define USB_BASE	0x31020000	/* USB registers base               */
#define LCD_BASE	0x31040000	/* LCD registers base               */
#define ETHERNET_BASE	0x31060000	/* Ethernet registers base          */
#define EMC_BASE	0x31080000	/* EMC configuration registers base */

/* FAB peripherals base addresses */
#define CLK_PM_BASE	0x40004000	/* System control registers base    */
#define HS_UART1_BASE	0x40014000	/* High speed UART 1 registers base */
#define HS_UART2_BASE	0x40018000	/* High speed UART 2 registers base */
#define HS_UART7_BASE	0x4001C000	/* High speed UART 7 registers base */
#define RTC_BASE	0x40024000	/* RTC registers base               */
#define GPIO_BASE	0x40028000	/* GPIO registers base              */
#define MUX_BASE	0x40028000	/* MUX registers base               */
#define WDT_BASE	0x4003C000	/* Watchdog timer registers base    */
#define TIMER0_BASE	0x40044000	/* Timer0 registers base            */
#define TIMER1_BASE	0x4004C000	/* Timer1 registers base            */
#define UART_CTRL_BASE	0x40054000	/* UART control regsisters base     */

/* APB peripherals base addresses */
#define UART3_BASE	0x40080000	/* UART 3 registers base            */
#define UART4_BASE	0x40088000	/* UART 4 registers base            */
#define UART5_BASE	0x40090000	/* UART 5 registers base            */
#define UART6_BASE	0x40098000	/* UART 6 registers base            */
#define I2C1_BASE	0x400A0000	/* I2C  1 registers base            */
#define I2C2_BASE	0x400A8000	/* I2C  2 registers base            */

/* External SDRAM Memory Bank base addresses */
#define EMC_DYCS0_BASE	0x80000000	/* SDRAM DYCS0 base address         */
#define EMC_DYCS1_BASE	0xA0000000	/* SDRAM DYCS1 base address         */

/* External Static Memory Bank base addresses */
#define EMC_CS0_BASE	0xE0000000
#define EMC_CS1_BASE	0xE1000000
#define EMC_CS2_BASE	0xE2000000
#define EMC_CS3_BASE	0xE3000000

#endif /* _LPC32XX_CPU_H */
