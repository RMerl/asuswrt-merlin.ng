/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuation settings for the SAM9X60EK board.
 *
 * Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Sandeep Sheriker M <sandeep.sheriker@microchip.com>
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

/* ARM asynchronous clock */
#define CFG_SYS_AT91_SLOW_CLOCK	32768
#define CFG_SYS_AT91_MAIN_CLOCK	24000000	/* 24 MHz crystal */

#define CFG_USART_BASE   ATMEL_BASE_DBGU
#define CFG_USART_ID     0 /* ignored in arm */

/*
 * define CONFIG_USB_EHCI_HCD to enable USB Hi-Speed (aka 2.0)
 * NB: in this case, USB 1.1 devices won't be recognized.
 */

/* SDRAM */
#define CFG_SYS_SDRAM_BASE		0x20000000
#define CFG_SYS_SDRAM_SIZE		0x10000000	/* 256 megs */

#endif
