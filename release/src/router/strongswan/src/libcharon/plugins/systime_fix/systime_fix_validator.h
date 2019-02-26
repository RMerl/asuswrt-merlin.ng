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
 * @defgroup systime_fix_validator systime_fix_validator
 * @{ @ingroup systime_fix
 */

#ifndef SYSTIME_FIX_VALIDATOR_H_
#define SYSTIME_FIX_VALIDATOR_H_

#include <credentials/cert_validator.h>

typedef struct systime_fix_validator_t systime_fix_validator_t;

/**
 * Validator that accepts cert lifetimes if system time is out of sync.
 */
struct systime_fix_validator_t {

	/**
	 * Implements cert_validator_t interface.
	 */
	cert_validator_t validator;

	/**
	 * Destroy a systime_fix_validator_t.
	 */
	void (*destroy)(systime_fix_validator_t *this);
};

/**
 * Create a systime_fix_validator instance.
 */
systime_fix_validator_t *systime_fix_validator_create();

#endif /** SYSTIME_FIX_VALIDATOR_H_ @}*/
