/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2010-2018 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the virtual mx6memcal board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* SPL */

#include "mx6_common.h"

#ifdef CONFIG_SERIAL_CONSOLE_UART1
#if defined(CONFIG_MX6SL)
#define CFG_MXC_UART_BASE		UART1_IPS_BASE_ADDR
#else
#define CFG_MXC_UART_BASE		UART1_BASE
#endif
#elif defined(CONFIG_SERIAL_CONSOLE_UART2)
#define CFG_MXC_UART_BASE		UART2_BASE
#else
#error please define serial console (CONFIG_SERIAL_CONSOLE_UARTx)
#endif

/* Physical Memory Map */
#define PHYS_SDRAM		       MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE	       PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE       IRAM_SIZE

#define CFG_MXC_USB_PORTSC	PORT_PTS_UTMI

#endif	       /* __CONFIG_H */
