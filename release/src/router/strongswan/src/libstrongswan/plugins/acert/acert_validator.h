/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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
 * @defgroup acert_validator acert_validator
 * @{ @ingroup acert
 */

#ifndef ACERT_VALIDATOR_H_
#define ACERT_VALIDATOR_H_

#include <credentials/cert_validator.h>

typedef struct acert_validator_t acert_validator_t;

/**
 * Attribute certificate group membership checking
 */
struct acert_validator_t {

	/**
	 * Implements cert_validator_t interface.
	 */
	cert_validator_t validator;

	/**
	 * Destroy a acert_validator_t.
	 */
	void (*destroy)(acert_validator_t *this);
};

/**
 * Create a acert_validator instance.
 */
acert_validator_t *acert_validator_create();

#endif /** ACERT_VALIDATOR_H_ @}*/
