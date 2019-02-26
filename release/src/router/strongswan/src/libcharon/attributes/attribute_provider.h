/*
 * Copyright (C) 2008 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
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
 * @defgroup attribute_provider attribute_provider
 * @{ @ingroup attributes
 */

#ifndef ATTRIBUTE_PROVIDER_H_
#define ATTRIBUTE_PROVIDER_H_

#include <sa/ike_sa.h>
#include <networking/host.h>
#include <collections/linked_list.h>

typedef struct attribute_provider_t attribute_provider_t;

/**
 * Interface to provide attributes to peers through attribute manager.
 */
struct attribute_provider_t {

	/**
	 * Acquire a virtual IP address to assign to a peer.
	 *
	 * @param pools			list of pool names (char*) to acquire from
	 * @param ike_sa		associated IKE_SA to assign address over
	 * @param requested		IP in configuration request
	 * @return				allocated address, NULL to serve none
	 */
	host_t* (*acquire_address)(attribute_provider_t *this,
							   linked_list_t *pools, ike_sa_t *ike_sa,
							   host_t *requested);
	/**
	 * Release a previously acquired address.
	 *
	 * @param pools			list of pool names (char*) to release to
	 * @param address		address to release
	 * @param ike_sa		IKE_SA to release address for
	 * @return				TRUE if the address has been released by the provider
	 */
	bool (*release_address)(attribute_provider_t *this,
							linked_list_t *pools, host_t *address,
							ike_sa_t *ike_sa);

	/**
	 * Create an enumerator over attributes to hand out to a peer.
	 *
	 * @param pool			list of pools names (char*) to query attributes from
	 * @param ike_sa		IKE_SA to request attributes for
	 * @param vip			list of virtual IPs (host_t*) to assign to peer
	 * @return				enumerator (configuration_attribute_type_t, chunk_t)
	 */
	enumerator_t* (*create_attribute_enumerator)(attribute_provider_t *this,
									linked_list_t *pools, ike_sa_t *ike_sa,
									linked_list_t *vips);
};

#endif /** ATTRIBUTE_PROVIDER_H_ @}*/
