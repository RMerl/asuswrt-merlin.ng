/*
 * Copyright (C) 2017 Andreas Steffen
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
 * @defgroup imc_swima imc_swima
 * @ingroup libimcv_plugins
 *
 * @defgroup imc_swima_state_t imc_swima_state
 * @{ @ingroup imc_swima
 */

#ifndef IMC_SWIMA_STATE_H_
#define IMC_SWIMA_STATE_H_

#include <imc/imc_state.h>
#include <swima/swima_inventory.h>
#include <library.h>

typedef struct imc_swima_state_t imc_swima_state_t;
typedef struct imc_swima_subscription_t imc_swima_subscription_t;

/**
 * State information on subscriptions
 */
struct imc_swima_subscription_t {

	/**
	 * IMV which sent the subscription request
	  */
	TNC_IMVID imv_id;

	/**
	 * SWIMA Request ID
	 */
	uint32_t request_id;

	/**
	 * SWIMA Request targets
	 */
	swima_inventory_t *targets;

	/**
	 * Retrieve SW Identifieres only
	 */
	bool sw_id_only;

};

/**
 * Internal state of an imc_swima_t connection instance
 */
struct imc_swima_state_t {

	/**
	 * imc_state_t interface
	 */
	imc_state_t interface;

	/**
	 * Set or clear a subscription
	 *
	 * @param subscription		state information on subscription
	 * @param set				TRUE sets and FALSE clears a subscripton
	 */
	void (*set_subscription)(imc_swima_state_t *this,
							 imc_swima_subscription_t *subscription, bool set);

	/**
	 * Get the subscription status
	 *
	 * @param subscription		state information on subscription
	 * @return					TRUE if subscription is set
	 */
	bool (*get_subscription)(imc_swima_state_t *this,
							 imc_swima_subscription_t**subscription);

	/**
	 * Set the earliest EID for the next subscription round
	 *
	 * @param eid				Earliest EID for events or 0 for inventories
	 */
	void (*set_earliest_eid)(imc_swima_state_t *this, uint32_t eid);

	/**
	 * Get earliest EID for the next subscription round
	 *
	 * @return					Earliest EID for events or 0 for inventories
	 */
	uint32_t (*get_earliest_eid)(imc_swima_state_t *this);
};

/**
 * Create an imc_swima_state_t instance
 *
 * @param id		connection ID
 */
imc_state_t* imc_swima_state_create(TNC_ConnectionID id);

#endif /** IMC_SWIMA_STATE_H_ @}*/
