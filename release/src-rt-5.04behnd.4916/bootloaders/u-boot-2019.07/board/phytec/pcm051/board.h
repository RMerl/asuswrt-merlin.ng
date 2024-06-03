/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * board.h
 *
 * Phytec phyCORE-AM335x (pcm051) boards information header
 *
 * Copyright (C) 2013, Lemonage Software GmbH
 * Author Lars Poeschel <poeschel@lemonage.de>
 */

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * We have three pin mux functions that must exist.  We must be able to enable
 * uart0, for initial output and i2c0 to read the main EEPROM.  We then have a
 * main pinmux function that can be overridden to enable all other pinmux that
 * is required on the board.
 */
void enable_uart0_pin_mux(void);
void enable_i2c0_pin_mux(void);
void enable_board_pin_mux(void);
void enable_cbmux_pin_mux(void);
#endif
