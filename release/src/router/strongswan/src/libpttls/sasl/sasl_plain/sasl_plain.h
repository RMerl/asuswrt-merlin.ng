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
 * @defgroup sasl_plain sasl_plain
 * @{ @ingroup sasl
 */

#ifndef SASL_PLAIN_H_
#define SASL_PLAIN_H_

#include <sasl/sasl_mechanism.h>

typedef struct sasl_plain_t sasl_plain_t;

/**
 * SASL Mechanism implementing PLAIN.
 */
struct sasl_plain_t {

	/**
	 * Implements sasl_mechanism_t
	 */
	sasl_mechanism_t sasl;
};

/**
 * Create a sasl_plain instance.
 *
 * @param name			name of mechanism, must be "PLAIN"
 * @param client		client identity, NULL to act as server
 * @return				mechanism implementing PLAIN, NULL on error
 */
sasl_plain_t *sasl_plain_create(char *name, identification_t *client);

#endif /** SASL_PLAIN_H_ @}*/
