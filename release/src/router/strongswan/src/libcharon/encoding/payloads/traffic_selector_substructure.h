/*
 * Copyright (C) 2022 Tobias Brunner
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
 *
 * Copyright (C) secunet Security Networks AG
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
#include <selectors/sec_label.h>
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
	 * Get a traffic_selector_t from this substructure if possible.
	 *
	 * @warning the returned object must be destroyed after use
	 *
	 * @return			contained traffic_selector_t (NULL if type mismatch)
	 */
	traffic_selector_t *(*get_traffic_selector)(traffic_selector_substructure_t *this);

	/**
	 * Get a sec_label_t from this substructure if possible.
	 *
	 * @warning the returned object must be destroyed after use
	 *
	 * @return			contained sec_label_t (NULL if type mismatch)
	 */
	sec_label_t *(*get_sec_label)(traffic_selector_substructure_t *this);

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
 * Creates a traffic selector substructure based on a traffic_selector_t.
 *
 * @param traffic_selector	data to use
 * @return					traffic_selector_substructure_t object
 */
traffic_selector_substructure_t *traffic_selector_substructure_create_from_traffic_selector(
										traffic_selector_t *traffic_selector);

/**
 * Creates a traffic selector substructure based on a sec_label_t.
 *
 * @param label				data to use
 * @return					traffic_selector_substructure_t object
 */
traffic_selector_substructure_t *traffic_selector_substructure_create_from_sec_label(
										sec_label_t *label);

#endif /** TRAFFIC_SELECTOR_SUBSTRUCTURE_H_ @}*/
