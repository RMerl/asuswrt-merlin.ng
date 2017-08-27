/*
 * Copyright (C) 2007 Martin Willi
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

#ifndef MCONSOLE_H
#define MCONSOLE_H

#include <library.h>

typedef struct mconsole_t mconsole_t;

/**
 * UML mconsole, change running UML configuration using mconsole.
 */
struct mconsole_t {

	/**
	 * Create a guest interface and connect it to tap host interface.
	 *
	 * @param guest			name of the interface to create in the guest
	 * @param host			name of the tap device to connect guest to
	 * @return				TRUE if interface created
	 */
	bool (*add_iface)(mconsole_t *this, char *guest, char *host);

	/**
	 * Delete a guest interface.
	 *
	 * @param guest			name of the interface to delete on the guest
	 * @return				TRUE if interface deleted
	 */
	bool (*del_iface)(mconsole_t *this, char *guest);

	/**
	 * Execute a command on the mconsole.
	 *
	 * @param cb			callback function to invoke for each line
	 * @param data			data to pass to callback
	 * @param cmd			command to invoke
	 * @return				return value of command
	 */
	int (*exec)(mconsole_t *this, void(*cb)(void*,char*,size_t), void *data,
				char *cmd);

	/**
	 * Destroy the mconsole instance
	 */
	void (*destroy) (mconsole_t *this);
};

/**
 * Create a new mconsole connection to a guest.
 *
 * Waits for a notification from the guest through the notify socket and tries
 * to connect to the mconsole socket supplied in the received notification.
 *
 * @param notify			unix notify socket path
 * @param idle				idle function to call while waiting for responses
 * @return					mconsole instance, or NULL if failed
 */
mconsole_t *mconsole_create(char *notify, void(*idle)(void));

#endif /* MCONSOLE_H */

