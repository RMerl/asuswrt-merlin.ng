/*
 * Copyright (C) 2012 Tobias Brunner
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
 * @defgroup host_resolver host_resolver
 * @{ @ingroup networking
 */

#ifndef HOST_RESOLVER_H_
#define HOST_RESOLVER_H_

#include "host.h"

typedef struct host_resolver_t host_resolver_t;

/**
 * Resolve hosts by DNS name but do so in a separate thread (calling
 * getaddrinfo(3) directly might block indefinitely, or at least a very long
 * time if no DNS servers are reachable).
 */
struct host_resolver_t {

	/**
	 * Resolve host from the given DNS name.
	 *
	 * @param name		name to lookup
	 * @param family	requested address family
	 * @return			resolved host or NULL if failed or canceled
	 */
	host_t *(*resolve)(host_resolver_t *this, char *name, int family);

	/**
	 * Flush the queue of queries. No new queries will be accepted afterwards.
	 */
	void (*flush)(host_resolver_t *this);

	/**
	 * Destroy a host_resolver_t.
	 */
	void (*destroy)(host_resolver_t *this);
};

/**
 * Create a host_resolver_t instance.
 */
host_resolver_t *host_resolver_create();

#endif /** HOST_RESOLVER_H_ @}*/
