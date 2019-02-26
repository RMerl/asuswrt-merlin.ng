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
 * @defgroup addrblock_validator addrblock_validator
 * @{ @ingroup addrblock
 */

#ifndef ADDRBLOCK_VALIDATOR_H_
#define ADDRBLOCK_VALIDATOR_H_

#include <credentials/cert_validator.h>

typedef struct addrblock_validator_t addrblock_validator_t;

/**
 * RFC 3779 address block X509 certificate validator.
 */
struct addrblock_validator_t {

	/**
	 * Implements cert_validator_t interface.
	 */
	cert_validator_t validator;

	/**
	 * Destroy a addrblock_validator_t.
	 */
	void (*destroy)(addrblock_validator_t *this);
};

/**
 * Create a addrblock_validator instance.
 */
addrblock_validator_t *addrblock_validator_create();

#endif /** ADDRBLOCK_VALIDATOR_H_ @}*/
