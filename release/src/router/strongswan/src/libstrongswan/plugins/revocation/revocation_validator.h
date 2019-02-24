/*
 * Copyright (C) 2018 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
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
 * @defgroup revocation_validator revocation_validator
 * @{ @ingroup revocation
 */

#ifndef REVOCATION_VALIDATOR_H_
#define REVOCATION_VALIDATOR_H_

#include <credentials/cert_validator.h>

typedef struct revocation_validator_t revocation_validator_t;

/**
 * Certificate validator doing CRL/OCSP checking of X509 certificates.
 */
struct revocation_validator_t {

	/**
	 * Implements cert_validator_t interface.
	 */
	cert_validator_t validator;

	/**
	 * Reload the configuration
	 */
	void (*reload)(revocation_validator_t *this);

	/**
	 * Destroy a revocation_validator_t.
	 */
	void (*destroy)(revocation_validator_t *this);
};

/**
 * Create a revocation_validator instance.
 */
revocation_validator_t *revocation_validator_create();

#endif /** REVOCATION_VALIDATOR_H_ @}*/
