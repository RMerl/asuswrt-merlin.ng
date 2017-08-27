/*
 * Copyright (C) 2008 Thomas Kallenberg
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup uci_control_t uci_control
 * @{ @ingroup uci
 */

#ifndef UCI_CONTROL_H_
#define UCI_CONTROL_H_

typedef struct uci_control_t uci_control_t;

/**
 * UCI control interface, uses a simple FIFO file
 */
struct uci_control_t {

	/**
	 * Destroy the controller
	 */
	void (*destroy)(uci_control_t *this);
};

/**
 * Create a UCI based configuration backend.
 */
uci_control_t *uci_control_create();

#endif /** UCI_CONTROL_H_ @}*/
