/*
 * Copyright (C) 2025 Tobias Brunner
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
 * @defgroup traffic_selector_list traffic_selector_list
 * @{ @ingroup selectors
 */

#ifndef TRAFFIC_SELECTOR_LIST_H_
#define TRAFFIC_SELECTOR_LIST_H_

typedef struct traffic_selector_list_t traffic_selector_list_t;

#include "traffic_selector.h"

/**
 * Collection of traffic selectors that can be narrowed to a new set of
 * traffic selectors.
 */
struct traffic_selector_list_t {

	/**
	 * Add a traffic selector to the collection.
	 *
	 * @param ts			traffic_selector to add (adopted)
	 */
	void (*add)(traffic_selector_list_t *this, traffic_selector_t *ts);

	/**
	 * Enumerate all traffic selectors in the collection.
	 *
	 * Similar to calling get() without \p hosts, but does not clone the traffic
	 * selectors and duplicates are not removed.
	 *
	 * @return				enumerator over traffic_selector_t*
	 */
	enumerator_t *(*create_enumerator)(traffic_selector_list_t *this);

	/**
	 * Get a list of traffic selectors contained in the collection.
	 *
	 * Some traffic selectors may be "dynamic", meaning they are narrowed down
	 * to a specific address (host-to-host or virtual-IP setups). Use the
	 * \p hosts parameter to narrow such traffic selectors to an address. If
	 * \p force_dynamic is also passed, even non-dynamic traffic selectors that
	 * match are replaced using the IPs in \p hosts (useful as initiator with
	 * transport mode).
	 *
	 * If \p hosts is not passed, the list of traffic selectors is returned as
	 * configured, except that exact duplicates are removed. However, note that
	 * "dynamic" traffic selectors are not considered duplicates.
	 *
	 * Returned list and its traffic selectors must be destroyed after use.
	 *
	 * Note that this method does not log anything. If logging is required, use
	 * select() without passing supplied traffic selectors.
	 *
	 * @param hosts			addresses to use for narrowing "dynamic" TS, host_t
	 * @param force_dynamic	TRUE to replace non-"dynamic" TS with \p hosts as
	 *						initiator in transport mode
	 * @return				list containing the traffic selectors
	 */
	linked_list_t *(*get)(traffic_selector_list_t *this, linked_list_t *hosts,
						  bool force_dynamic);

	/**
	 * Select a list of traffic selectors contained in the collection.
	 *
	 * If a list with traffic selectors is supplied, these are used to narrow
	 * down the traffic selectors to the greatest common subset.
	 *
	 * Some traffic selectors may be "dynamic", meaning they are narrowed down
	 * to a specific address (host-to-host or virtual-IP setups). Use the
	 * \p hosts parameter to narrow such traffic selectors to an address. If
	 * \p force_dynamic is also passed, even non-dynamic traffic selectors that
	 * match are replaced using the IPs in \p hosts (useful as initiator with
	 * transport mode).
	 *
	 * Details about the selection of each individual traffic selector are
	 * logged.
	 *
	 * Returned list and its traffic selectors must be destroyed after use.
	 *
	 * @param supplied		list with TS to select from, or NULL
	 * @param hosts			addresses to use for narrowing "dynamic" TS', host_t
	 * @param force_dynamic	TRUE to replace non-"dynamic" TS with \p hosts as
	 *						initiator in transport mode
	 * @param narrowed[out]	optional flag that indicates if any TS were narrowed
	 * @return				list containing the traffic selectors
	 */
	linked_list_t *(*select)(traffic_selector_list_t *this,
							 linked_list_t *supplied, linked_list_t *hosts,
							 bool force_dynamic, bool *narrowed);

	/**
	 * Compare two collections of traffic selectors.
	 *
	 * @param other			collection to compare with this
	 * @return				TRUE if equal, FALSE otherwise
	 */
	bool (*equals)(traffic_selector_list_t *this, traffic_selector_list_t *other);

	/**
	 * Clone this collection of traffic selectors.
	 *
	 * @return				cloned collection
	 */
	traffic_selector_list_t *(*clone)(traffic_selector_list_t *this);

	/**
	 * Destroys this collection.
	 */
	void (*destroy)(traffic_selector_list_t *this);
};

/**
 * Create an empty traffic selector collection.
 *
 * @return					created object
 */
traffic_selector_list_t *traffic_selector_list_create();

/**
 * Create a collection with traffic selectors from the given list (adopted).
 *
 * @param list				list of traffic_selector_t (adopted)
 * @return					created object
 */
traffic_selector_list_t *traffic_selector_list_create_from_list(
														linked_list_t *list);

/**
 * Create a collection with traffic selectors from the given enumerator (objects
 * are cloned, the enumerator is destroyed).
 *
 * @param enumerator		enumerator over traffic_selector_t (cloned/destroyed)
 * @return					created object
 */
traffic_selector_list_t *traffic_selector_list_create_from_enumerator(
													enumerator_t *enumerator);

#endif /** TRAFFIC_SELECTOR_LIST_H_ @}*/
