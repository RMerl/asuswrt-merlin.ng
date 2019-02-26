/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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
 * @defgroup child_sa_manager child_sa_manager
 * @{ @ingroup sa
 */

#ifndef CHILD_SA_MANAGER_H_
#define CHILD_SA_MANAGER_H_

#include <sa/ike_sa.h>
#include <sa/child_sa.h>

typedef struct child_sa_manager_t child_sa_manager_t;

/**
 * Handle CHILD_SA to IKE_SA relations
 */
struct child_sa_manager_t {

	/**
	 * Register a CHILD_SA/IKE_SA relation.
	 *
	 * @param child_sa		CHILD_SA to register
	 * @param ike_sa		IKE_SA owning the CHILD_SA
	 */
	void (*add)(child_sa_manager_t *this, child_sa_t *child_sa, ike_sa_t *ike_sa);

	/**
	 * Unregister a CHILD_SA/IKE_SA relation.
	 *
	 * @param child_sa		CHILD_SA to unregister
	 */
	void (*remove)(child_sa_manager_t *this, child_sa_t *child_sa);

	/**
	 * Find a CHILD_SA and check out the associated IKE_SA by SPI.
	 *
	 * On success, the returned IKE_SA must be checked in after use to
	 * the IKE_SA manager.
	 *
	 * @param protocol		IPsec protocol, AH|ESP
	 * @param spi			SPI of CHILD_SA to check out
	 * @param dst			SA destination host related to SPI
	 * @param child_sa		returns CHILD_SA managed by IKE_SA
	 * @return				IKE_SA, NULL if not found
	 */
	ike_sa_t *(*checkout)(child_sa_manager_t *this,
						  protocol_id_t protocol, uint32_t spi, host_t *dst,
						  child_sa_t **child_sa);

	/**
	 * Find a CHILD_SA and check out the associated IKE_SA by unique_id.
	 *
	 * On success, the returned IKE_SA must be checked in after use to
	 * the IKE_SA manager.
	 *
	 * @param unique_id		unique ID of CHILD_SA to check out
	 * @param child_sa		returns CHILD_SA managed by IKE_SA
	 * @return				IKE_SA, NULL if not found
	 */
	ike_sa_t *(*checkout_by_id)(child_sa_manager_t *this, uint32_t unique_id,
								child_sa_t **child_sa);

	/**
	 * Destroy a child_sa_manager_t.
	 */
	void (*destroy)(child_sa_manager_t *this);
};

/**
 * Create a child_sa_manager instance.
 */
child_sa_manager_t *child_sa_manager_create();

#endif /** CHILD_SA_MANAGER_H_ @}*/
