/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Gumstix Pepper and AM335x-based boards information header
 *
 * Copyright (C) 2014, Gumstix, Inc. - http://www.gumstix.com/
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#define GUMSTIX_PEPPER		0x30000200
#define GUMSTIX_PEPPER_DVI	0x31000200

struct pepper_board_id {
	unsigned int device_vendor;
	unsigned char revision;
	unsigned char content;
	char fab_revision[8];
	char env_var[16];
	char en_setting[64];
};

/*
 * We must be able to enable uart0, for initial output. We then have a
 * main pinmux function that can be overridden to enable all other pinmux that
 * is required on the board.
 */
void enable_uart0_pin_mux(void);
void enable_board_pin_mux(void);
void enable_i2c0_pin_mux(void);
#endif
