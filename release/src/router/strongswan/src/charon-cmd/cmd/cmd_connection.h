/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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
 * @defgroup charon-cmd charon-cmd
 *
 * @defgroup cmd cmd
 * @ingroup charon-cmd
 *
 * @defgroup cmd_connection cmd_connection
 * @{ @ingroup cmd
 */

#ifndef CMD_CONNECTION_H_
#define CMD_CONNECTION_H_

#include <library.h>

#include "cmd_options.h"

typedef struct cmd_connection_t cmd_connection_t;

/**
 * Connection definition to construct and initiate.
 */
struct cmd_connection_t {

	/**
	 * Handle a command line option.
	 *
	 * @param opt		option to handle
	 * @param arg		option argument
	 * @return			TRUE if option handled
	 */
	bool (*handle)(cmd_connection_t *this, cmd_option_type_t opt, char *arg);

	/**
	 * Destroy a cmd_connection_t.
	 */
	void (*destroy)(cmd_connection_t *this);
};

/**
 * Create a cmd_connection instance.
 */
cmd_connection_t *cmd_connection_create();

#endif /** CMD_CONNECTION_H_ @}*/
