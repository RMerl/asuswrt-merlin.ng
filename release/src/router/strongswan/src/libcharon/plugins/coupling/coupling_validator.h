/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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
 * @defgroup coupling_validator coupling_validator
 * @{ @ingroup coupling
 */

#ifndef COUPLING_VALIDATOR_H_
#define COUPLING_VALIDATOR_H_

#include <credentials/cert_validator.h>

typedef struct coupling_validator_t coupling_validator_t;

/**
 * Validator that couples authenticated certificates permanently.
 */
struct coupling_validator_t {

	/**
	 * Implements cert_validator_t interface.
	 */
	cert_validator_t validator;

	/**
	 * Destroy a coupling_validator_t.
	 */
	void (*destroy)(coupling_validator_t *this);
};

/**
 * Create a coupling_validator instance.
 */
coupling_validator_t *coupling_validator_create();

#endif /** COUPLING_VALIDATOR_H_ @}*/
