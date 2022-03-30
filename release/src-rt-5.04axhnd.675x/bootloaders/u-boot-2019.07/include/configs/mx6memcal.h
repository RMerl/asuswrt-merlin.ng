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
#include "imx6_spl.h"

#undef CONFIG_MMC
#undef CONFIG_SPL_MMC_SUPPORT
#undef CONFIG_GENERIC_MMC
#undef CONFIG_CMD_FUSE

#define CONFIG_SYS_MEMTEST_START	0x10000000
#define CONFIG_SYS_MEMTEST_END		0x20000000
#define CONFIG_SYS_MALLOC_LEN		(64 * 1024 * 1024)

#define CONFIG_MXC_UART
#ifdef CONFIG_SERIAL_CONSOLE_UART1
#if defined(CONFIG_MX6SL)
#define CONFIG_MXC_UART_BASE		UART1_IPS_BASE_ADDR
#else
#define CONFIG_MXC_UART_BASE		UART1_BASE
#endif
#elif defined(CONFIG_SERIAL_CONSOLE_UART2)
#define CONFIG_MXC_UART_BASE		UART2_BASE
#else
#error please define serial console (CONFIG_SERIAL_CONSOLE_UARTx)
#endif
#define CONFIG_BAUDRATE			115200

#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + 16)

/* Physical Memory Map */
#define PHYS_SDRAM		       MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE	       PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE       IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_ENV_SIZE			(8 * 1024)

#define CONFIG_MXC_USB_PORTSC	PORT_PTS_UTMI

#endif	       /* __CONFIG_H */
