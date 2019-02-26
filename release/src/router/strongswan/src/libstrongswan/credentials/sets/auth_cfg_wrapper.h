/*
 * Copyright (C) 2008-2009 Martin Willi
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
 * @defgroup auth_cfg_wrapper auth_cfg_wrapper
 * @{ @ingroup sets
 */

#ifndef AUTH_CFG_WRAPPER_H_
#define AUTH_CFG_WRAPPER_H_

#include <credentials/auth_cfg.h>
#include <credentials/credential_set.h>

typedef struct auth_cfg_wrapper_t auth_cfg_wrapper_t;

/**
 * A wrapper around auth_cfg_t to handle it as a credential set.
 */
struct auth_cfg_wrapper_t {

	/**
	 * implements credential_set_t
	 */
	credential_set_t set;

	/**
	 * Destroy a auth_cfg_wrapper instance.
	 */
	void (*destroy)(auth_cfg_wrapper_t *this);
};

/**
 * Create a auth_cfg_wrapper instance.
 *
 * @param auth		the wrapped auth info
 * @return			wrapper around auth
 */
auth_cfg_wrapper_t *auth_cfg_wrapper_create(auth_cfg_t *auth);

#endif /** AUTH_CFG_WRAPPER_H_ @}*/
