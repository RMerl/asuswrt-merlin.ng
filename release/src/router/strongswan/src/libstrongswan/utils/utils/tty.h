/*
 * Copyright (C) 2008-2014 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup tty_i tty
 * @{ @ingroup utils_i
 */

#ifndef TTY_H_
#define TTY_H_

typedef enum tty_escape_t tty_escape_t;

/**
 * Excape codes for tty colors
 */
enum tty_escape_t {
	/** text properties */
	TTY_RESET,
	TTY_BOLD,
	TTY_UNDERLINE,
	TTY_BLINKING,

	/** foreground colors */
	TTY_FG_BLACK,
	TTY_FG_RED,
	TTY_FG_GREEN,
	TTY_FG_YELLOW,
	TTY_FG_BLUE,
	TTY_FG_MAGENTA,
	TTY_FG_CYAN,
	TTY_FG_WHITE,
	TTY_FG_DEF,

	/** background colors */
	TTY_BG_BLACK,
	TTY_BG_RED,
	TTY_BG_GREEN,
	TTY_BG_YELLOW,
	TTY_BG_BLUE,
	TTY_BG_MAGENTA,
	TTY_BG_CYAN,
	TTY_BG_WHITE,
	TTY_BG_DEF,
};

/**
 * Get the escape string for a given TTY color, empty string on non-tty fd
 */
char* tty_escape_get(int fd, tty_escape_t escape);

#endif /** TTY_H_ @} */
