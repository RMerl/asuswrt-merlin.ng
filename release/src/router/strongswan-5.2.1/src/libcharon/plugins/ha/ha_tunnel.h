/*
 * Copyright (C) 2009 Martin Willi
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
 * @defgroup ha_ ha_tunnel
 * @{ @ingroup ha
 */

#ifndef HA_TUNNEL_H_
#define HA_TUNNEL_H_

#include <sa/ike_sa.h>

typedef struct ha_tunnel_t ha_tunnel_t;

/**
 * Socket to send/received SA synchronization data
 */
struct ha_tunnel_t {

	/**
	 * Check if an IKE_SA is used for exchanging HA messages.
	 *
	 * @param ike_Sa	ike_sa to check
	 * @return			TRUE if IKE_SA is used to secure HA messages
	 */
	bool (*is_sa)(ha_tunnel_t *this, ike_sa_t *ike_sa);

	/**
	 * Destroy a ha_tunnel_t.
	 */
	void (*destroy)(ha_tunnel_t *this);
};

/**
 * Create a ha_tunnel instance.
 *
 * @param local		local address of HA tunnel
 * @param remote	remote address of HA tunnel
 * @param secret	PSK tunnel authentication secret
 * @return			HA tunnel instance
 */
ha_tunnel_t *ha_tunnel_create(char *local, char *remote, char *secret);

#endif /** HA_TUNNEL_H_ @}*/
