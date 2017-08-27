/*
 * Copyright (C) 2007 Tobias Brunner
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
 * @defgroup endpoint_notify endpoint_notify
 * @{ @ingroup payloads
 */

#ifndef ENDPOINT_NOTIFY_H_
#define ENDPOINT_NOTIFY_H_

#define ME_PRIO_HOST   255
#define ME_PRIO_PEER   128
#define ME_PRIO_SERVER 64
#define ME_PRIO_RELAY  0

typedef enum me_endpoint_family_t me_endpoint_family_t;
typedef enum me_endpoint_type_t me_endpoint_type_t;
typedef struct endpoint_notify_t endpoint_notify_t;

#include <encoding/payloads/notify_payload.h>

/**
 * ME endpoint families.
 */
enum me_endpoint_family_t {

	NO_FAMILY = 0,

	IPv4 = 1,

	IPv6 = 2,

	MAX_FAMILY = 3

};

/**
 * ME endpoint types.
 */
enum me_endpoint_type_t {

	NO_TYPE = 0,

	HOST = 1,

	PEER_REFLEXIVE = 2,

	SERVER_REFLEXIVE = 3,

	RELAYED = 4,

	MAX_TYPE = 5

};

/**
 * enum name for me_endpoint_type_t.
 */
extern enum_name_t *me_endpoint_type_names;

/**
 * Class representing a ME_ENDPOINT Notify payload. In fact it's not
 * the notify per se, but the notification data of that notify that is
 * handled with this class.
 */
struct endpoint_notify_t {
	/**
	 * Returns the priority of this endpoint.
	 *
	 * @return			priority
	 */
	u_int32_t (*get_priority) (endpoint_notify_t *this);

	/**
	 * Sets the priority of this endpoint.
	 *
	 * @param priority	priority
	 */
	void (*set_priority) (endpoint_notify_t *this, u_int32_t priority);

	/**
	 * Returns the endpoint type of this endpoint.
	 *
	 * @return			endpoint type
	 */
	me_endpoint_type_t (*get_type) (endpoint_notify_t *this);

	/**
	 * Returns the endpoint family of this endpoint.
	 *
	 * @return			endpoint family
	 */
	me_endpoint_family_t (*get_family) (endpoint_notify_t *this);

	/**
	 * Returns the host of this endpoint.
	 *
	 * @return			host
	 */
	host_t *(*get_host) (endpoint_notify_t *this);

	/**
	 * Returns the base of this endpoint.
	 *
	 * If this is not a SERVER_REFLEXIVE endpoint, the returned host is the same
	 * as the one returned by get_host.
	 *
	 * @return			host
	 */
	host_t *(*get_base) (endpoint_notify_t *this);

	/**
	 * Generates a notification payload from this endpoint.
	 *
	 * @return			built notify_payload_t
	 */
	notify_payload_t *(*build_notify) (endpoint_notify_t *this);

	/**
	 * Clones an endpoint_notify_t object.
	 *
	 * @return			cloned object
	 */
	endpoint_notify_t *(*clone) (endpoint_notify_t *this);

	/**
	 * Destroys an endpoint_notify_t object.
	 */
	void (*destroy) (endpoint_notify_t *this);
};

/**
 * Creates an endpoint_notify_t object from a host.
 *
 * @param type		the endpoint type
 * @param host		host to base the notify on (gets cloned)
 * @param base		base of the endpoint, applies only to reflexive
 *					endpoints (gets cloned)
 * @return			created endpoint_notify_t object
 */
endpoint_notify_t *endpoint_notify_create_from_host(me_endpoint_type_t type,
													host_t *host, host_t *base);

/**
 * Creates an endpoint_notify_t object from a notify payload.
 *
 * @param notify	the notify payload
 * @return			- created endpoint_notify_t object
 *					- NULL if invalid payload
 */
endpoint_notify_t *endpoint_notify_create_from_payload(notify_payload_t *notify);

#endif /** ENDPOINT_NOTIFY_H_ @}*/
