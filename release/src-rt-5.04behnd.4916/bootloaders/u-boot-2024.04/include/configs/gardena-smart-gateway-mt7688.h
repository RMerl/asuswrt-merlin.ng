/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Stefan Roese <sr@denx.de>
 */

#ifndef __CONFIG_GARDENA_SMART_GATEWAY_H
#define __CONFIG_GARDENA_SMART_GATEWAY_H

/* RAM */
#define CFG_SYS_SDRAM_BASE		0x80000000

#define CFG_SYS_INIT_SP_OFFSET	0x400000

/* Dummy value */
#define CFG_SYS_UBOOT_BASE		0

/* Serial SPL */
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_SERIAL)
#define CFG_SYS_NS16550_CLK		40000000
#define CFG_SYS_NS16550_COM1		0xb0000c00
#endif

/* UART */
#define CFG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, \
					  230400, 460800, 921600 }

/* RAM */

/* Environment settings */

#endif /* __CONFIG_GARDENA_SMART_GATEWAY_H */
