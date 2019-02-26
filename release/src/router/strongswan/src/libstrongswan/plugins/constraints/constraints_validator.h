/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
 * @defgroup constraints_validator constraints_validator
 * @{ @ingroup constraints
 */

#ifndef CONSTRAINTS_VALIDATOR_H_
#define CONSTRAINTS_VALIDATOR_H_

#include <credentials/cert_validator.h>

typedef struct constraints_validator_t constraints_validator_t;

/**
 * Certificate validator doing advanced X509 constraint checking.
 */
struct constraints_validator_t {

	/**
	 * Implements cert_validator_t interface.
	 */
	cert_validator_t validator;

	/**
	 * Destroy a constraints_validator_t.
	 */
	void (*destroy)(constraints_validator_t *this);
};

/**
 * Create a constraints_validator instance.
 */
constraints_validator_t *constraints_validator_create();

#endif /** CONSTRAINTS_VALIDATOR_H_ @}*/
