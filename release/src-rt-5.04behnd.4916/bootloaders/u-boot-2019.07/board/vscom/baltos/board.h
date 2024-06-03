/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * board.h
 *
 * TI AM335x boards information header
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#ifndef _BOARD_H_
#define _BOARD_H_

typedef struct _BSP_VS_HWPARAM    // v1.0
{
	uint32_t Magic;
	uint32_t HwRev;
	uint32_t SerialNumber;
	char PrdDate[11];    // as a string ie. "01.01.2006"
	uint16_t SystemId;
	uint8_t MAC1[6];        // internal EMAC
	uint8_t MAC2[6];        // SMSC9514
	uint8_t MAC3[6];        // WL1271 WLAN
} __attribute__ ((packed)) BSP_VS_HWPARAM;

/*
 * We have three pin mux functions that must exist.  We must be able to enable
 * uart0, for initial output and i2c0 to read the main EEPROM.  We then have a
 * main pinmux function that can be overridden to enable all other pinmux that
 * is required on the board.
 */
void enable_uart0_pin_mux(void);
void enable_i2c1_pin_mux(void);
void enable_board_pin_mux(void);
#endif
