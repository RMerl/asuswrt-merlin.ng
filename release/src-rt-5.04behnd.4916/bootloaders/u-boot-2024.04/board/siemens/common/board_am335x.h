/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Board definitions for draco products
 *
 * (C) Copyright 2013 Siemens Schweiz AG
 * (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * TI AM335x boards information header
 * u-boot:/board/ti/am335x/board.h
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#ifndef _BOARD_AM335X_H_
#define _BOARD_AM335X_H_

#include "eeprom.h"

/* Common functions with product specific implementation */
void spl_draco_board_init(void);
void draco_init_ddr(void);
int draco_read_eeprom(void);

#ifdef CONFIG_SPL_BUILD
/* Mux for init: uart?, i2c0 to read the main EEPROM */
void enable_uart0_pin_mux(void);
void enable_uart1_pin_mux(void);
void enable_uart2_pin_mux(void);
void enable_uart3_pin_mux(void);
void enable_uart4_pin_mux(void);
void enable_uart5_pin_mux(void);
void enable_i2c0_pin_mux(void);

/* Main mux function to enable other pinmux required on the board */
void enable_board_pin_mux(void);
#endif /* CONFIG_SPL_BUILD */

#endif /* _BOARD_AM335X_H_ */
