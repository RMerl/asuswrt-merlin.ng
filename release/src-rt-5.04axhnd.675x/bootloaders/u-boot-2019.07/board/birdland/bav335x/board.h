/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * board.c
 *
 * Board functions for Birdland Audio BAV335x Network Processor
 *
 * Copyright (c) 2012-2014, Birdland Audio - http://birdland.com/oem
 *
 */

#ifndef _BOARD_H_
#define _BOARD_H_

/* Serial MagicE: AA 55 BA BE */
#define BOARD_MAGIC  0xBEBA55AA
enum board_type {UNKNOWN, BAV335A, BAV335B};


/*
 * The BAV335x may use a built-in read-only serial EEProm.
 * The Evaluation board, disables the write-protect so the Serial-EE
 * Can be programmed during manufacturing to store fields such as
 * a board serial number, ethernet mac address and other user fields.
 * Additionally, the Serial-EE can store the specific version of the
 * board it runs on, and overwrite the defaults in _defconfig
 */
#define HDR_NO_OF_MAC_ADDR	3
#define HDR_ETH_ALEN		6
#define HDR_NAME_LEN		8

struct board_eeconfig {
	unsigned int  magic;
	char name[HDR_NAME_LEN];	/* BAV3354 */
	char version[4];		/* 0B20 - Rev.B2 */
	char serial[16];
	char config[32];
	char mac_addr[HDR_NO_OF_MAC_ADDR][HDR_ETH_ALEN];
};

enum board_type get_board_type(bool verbose_debug_output);


/*
 * We have three pin mux functions that must exist.  We must be able to enable
 * uart0, for initial output and i2c0 to read the main EEPROM.  We then have a
 * main pinmux function that can be overridden to enable all other pinmux that
 * is required on the board.
 */
void enable_uart0_pin_mux(void);
void enable_uart1_pin_mux(void);
void enable_uart2_pin_mux(void);
void enable_uart3_pin_mux(void);
void enable_uart4_pin_mux(void);
void enable_uart5_pin_mux(void);
void enable_i2c0_pin_mux(void);
void enable_board_pin_mux(enum board_type board);

#endif
