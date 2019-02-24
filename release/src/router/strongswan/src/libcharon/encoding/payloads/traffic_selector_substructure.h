/*
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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
 * @defgroup traffic_selector_substructure traffic_selector_substructure
 * @{ @ingroup payloads
 */

#ifndef TRAFFIC_SELECTOR_SUBSTRUCTURE_H_
#define TRAFFIC_SELECTOR_SUBSTRUCTURE_H_

typedef struct traffic_selector_substructure_t traffic_selector_substructure_t;

#include <library.h>
#include <networking/host.h>
#include <selectors/traffic_selector.h>
#include <encoding/payloads/payload.h>

/**
 * Class representing an IKEv2 TRAFFIC SELECTOR.
 *
 * The TRAFFIC SELECTOR format is described in RFC section 3.13.1.
 */
struct traffic_selector_substructure_t {
	/**
	 * The payload_t interface.
	 */
	payload_t payload_interface;

	/**
	 * Get the type of Traffic selector.
	 *
	 * @return			type of traffic selector
	 *
	 */
	ts_type_t (*get_ts_type) (traffic_selector_substructure_t *this);

	/**
	 * Set the type of Traffic selector.
	 *
	 * @param ts_type	type of traffic selector
	 */
	void (*set_ts_type) (traffic_selector_substructure_t *this,
						 ts_type_t ts_type);

	/**
	 * Get the IP protocol ID of Traffic selector.
	 *
	 * @return			type of traffic selector
	 *
	 */
	uint8_t (*get_protocol_id) (traffic_selector_substructure_t *this);

	/**
	 * Set the IP protocol ID of Traffic selector
	 *
	 * @param protocol_id	protocol ID of traffic selector
	 */
	void (*set_protocol_id) (traffic_selector_substructure_t *this,
							  uint8_t protocol_id);

	/**
	 * Get the start port and address as host_t object.
	 *
	 * Returned host_t object has to get destroyed by the caller.
	 *
	 * @return			start host as host_t object
	 *
	 */
	host_t *(*get_start_host) (traffic_selector_substructure_t *this);

	/**
	 * Set the start port and address as host_t object.
	 *
	 * @param start_host	start host as host_t object
	 */
	void (*set_start_host) (traffic_selector_substructure_t *this,
							host_t *start_host);

	/**
	 * Get the end port and address as host_t object.
	 *
	 * Returned host_t object has to get destroyed by the caller.
	 *
	 * @return			end host as host_t object
	 *
	 */
	host_t *(*get_end_host) (traffic_selector_substructure_t *this);

	/**
	 * Set the end port and address as host_t object.
	 *
	 * @param end_host	end host as host_t object
	 */
	void (*set_end_host) (traffic_selector_substructure_t *this,
						  host_t *end_host);

	/**
	 * Get a traffic_selector_t from this substructure.
	 *
	 * @warning traffic_selector_t must be destroyed after usage.
	 *
	 * @return			contained traffic_selector_t
	 */
	traffic_selector_t *(*get_traffic_selector) (
										traffic_selector_substructure_t *this);

	/**
	 * Destroys an traffic_selector_substructure_t object.
	 */
	void (*destroy) (traffic_selector_substructure_t *this);
};

/**
 * Creates an empty traffic_selector_substructure_t object.
 *
 * TS type is set to default TS_IPV4_ADDR_RANGE!
 *
 * @return 					traffic_selector_substructure_t object
 */
traffic_selector_substructure_t *traffic_selector_substructure_create(void);

/**
 * Creates an initialized traffif selector substructure using
 * the values from a traffic_selector_t.
 *
 * @param traffic_selector	traffic_selector_t to use for initialization
 * @return					traffic_selector_substructure_t object
 */
traffic_selector_substructure_t *traffic_selector_substructure_create_from_traffic_selector(
										traffic_selector_t *traffic_selector);

#endif /** TRAFFIC_SELECTOR_SUBSTRUCTURE_H_ @}*/
