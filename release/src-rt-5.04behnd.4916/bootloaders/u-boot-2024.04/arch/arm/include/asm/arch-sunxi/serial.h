/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  hardcoded UART base addresses for early SPL use
 *
 *  Copyright (c) 2022  Arm Ltd.
 */

#ifndef SUNXI_SERIAL_MEMMAP_H
#define SUNXI_SERIAL_MEMMAP_H

#if defined(CONFIG_MACH_SUN9I)
#define SUNXI_UART0_BASE		0x07000000
#define SUNXI_R_UART_BASE		0x08002800
#elif defined(CONFIG_SUN50I_GEN_H6)
#define SUNXI_UART0_BASE		0x05000000
#define SUNXI_R_UART_BASE		0x07080000
#elif defined(CONFIG_MACH_SUNIV)
#define SUNXI_UART0_BASE		0x01c25000
#define SUNXI_R_UART_BASE		0
#elif defined(CONFIG_SUNXI_GEN_NCAT2)
#define SUNXI_UART0_BASE		0x02500000
#define SUNXI_R_UART_BASE		0		// 0x07080000 (?>
#else
#define SUNXI_UART0_BASE		0x01c28000
#define SUNXI_R_UART_BASE		0x01f02800
#endif

#define SUNXI_UART1_BASE		(SUNXI_UART0_BASE + 0x400)
#define SUNXI_UART2_BASE		(SUNXI_UART0_BASE + 0x800)
#define SUNXI_UART3_BASE		(SUNXI_UART0_BASE + 0xc00)

#endif /* SUNXI_SERIAL_MEMMAP_H */
