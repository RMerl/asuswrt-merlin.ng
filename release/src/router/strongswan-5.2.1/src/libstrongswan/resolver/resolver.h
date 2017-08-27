/*
 * Copyright (C) 2011-2012 Reto Guadagnini
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
 * @defgroup resolveri resolver
 * @{ @ingroup resolver
 */

#ifndef RESOLVER_H_
#define RESOLVER_H_

typedef struct resolver_t resolver_t;

/**
 * Constructor function which creates DNS resolver instances.
 */
typedef resolver_t* (*resolver_constructor_t)(void);

#include <resolver/resolver_response.h>
#include <resolver/rr_set.h>
#include <resolver/rr.h>

/**
 * Interface of a security-aware DNS resolver.
 *
 */
struct resolver_t {

	/**
	 * Perform a DNS query.
	 *
	 * @param domain		domain (FQDN) to query
	 * @param rr_class		class of the desired RRs
	 * @param rr_type		type of the desired RRs
	 * @return				response to the query, NULL on failure
	 */
	resolver_response_t *(*query)(resolver_t *this, char *domain,
								  rr_class_t rr_class, rr_type_t rr_type);

	/**
	 * Destroy the resolver instance.
	 */
	void (*destroy)(resolver_t *this);
};

#endif /** RESOLVER_H_ @}*/
