/*
 * Copyright (C) 2008-2009 Martin Willi
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
 * @defgroup agent_private_key agent_private_key
 * @{ @ingroup agent_p
 */

#ifndef AGENT_PRIVATE_KEY_H_
#define AGENT_PRIVATE_KEY_H_

#include <credentials/builder.h>
#include <credentials/keys/private_key.h>

typedef struct agent_private_key_t agent_private_key_t;

/**
 * private_key_t implementation using an ssh-agent.
 */
struct agent_private_key_t {

	/**
	 * Implements private_key_t interface
	 */
	private_key_t key;
};

/**
 * Open connection to a private key stored in a SSH agent.
 *
 * The function takes BUILD_AGENT_SOCKET and optionally a BUILD_PUBLIC_KEY
 * to select a specific key loaded in ssh-agent.
 *
 * @param type		type of the key, must be KEY_RSA
 * @param args		builder_part_t argument list
 * @return 			built key, NULL on failure
 */
agent_private_key_t *agent_private_key_open(key_type_t type, va_list args);

#endif /** AGENT_PRIVATE_KEY_H_ @}*/

