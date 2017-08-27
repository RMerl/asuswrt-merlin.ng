/*
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup load_tester_creds_t load_tester_creds
 * @{ @ingroup load_tester
 */

#ifndef LOAD_TESTER_CREDS_H_
#define LOAD_TESTER_CREDS_H_

#include <credentials/credential_set.h>

typedef struct load_tester_creds_t load_tester_creds_t;

/**
 * Provide hard-coded credentials for load testing.
 */
struct load_tester_creds_t {

	/**
	 * Implements credential set interface.
	 */
	credential_set_t credential_set;

	/**
	 * Destroy the backend.
	 */
	void (*destroy)(load_tester_creds_t *this);
};

/**
 * Create a credential set for load testing.
 *
 * @return			credential set
 */
load_tester_creds_t *load_tester_creds_create();

#endif /** LOAD_TESTER_CREDS_H_ @}*/
