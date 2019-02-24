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
 * @defgroup imv_swima imv_swima
 * @ingroup libimcv_plugins
 *
 * @defgroup imv_swima_state_t imv_swima_state
 * @{ @ingroup imv_swima
 */

#ifndef IMV_SWIMA_STATE_H_
#define IMV_SWIMA_STATE_H_

#include <imv/imv_state.h>
#include <swima/swima_inventory.h>
#include <swima/swima_events.h>
#include <library.h>

#include <json.h>

typedef struct imv_swima_state_t imv_swima_state_t;
typedef enum imv_swima_handshake_state_t imv_swima_handshake_state_t;

/**
 * IMV OS Handshake States (state machine)
 */
enum imv_swima_handshake_state_t {
	IMV_SWIMA_STATE_INIT,
	IMV_SWIMA_STATE_WORKITEMS,
	IMV_SWIMA_STATE_END
};

/**
 * Internal state of an imv_swima_t connection instance
 */
struct imv_swima_state_t {

	/**
	 * imv_state_t interface
	 */
	imv_state_t interface;

	/**
	 * Set state of the handshake
	 *
	 * @param new_state			the handshake state of IMV
	 */
	void (*set_handshake_state)(imv_swima_state_t *this,
								imv_swima_handshake_state_t new_state);

	/**
	 * Get state of the handshake
	 *
	 * @return					the handshake state of IMV
	 */
	imv_swima_handshake_state_t (*get_handshake_state)(imv_swima_state_t *this);

	/**
	 * Set the SWID request ID
	 *
	 * @param request_id		SWID request ID to be set
	 */
	void (*set_request_id)(imv_swima_state_t *this, uint32_t request_id);

	/**
	 * Get the SWID request ID
	 *
	 * @return					SWID request ID
	 */
	uint32_t (*get_request_id)(imv_swima_state_t *this);

	/**
	 * Set or extend the SW ID inventory in the state
	 *
	 * @param inventory			SW ID inventory to be added
	 */
	void (*set_inventory)(imv_swima_state_t *this, swima_inventory_t *inventory);

	/**
	 * Set or extend the SW ID events in the state
	 *
	 * @param events			SW ID events to be added
	 */
	void (*set_events)(imv_swima_state_t *this, swima_events_t *events);

	/**
	 * Get the JSON encoding of the complete SW ID inventory or SW ID events
	 *
	 * @return			       JSON encoding
	 */
	json_object* (*get_jrequest)(imv_swima_state_t *this);

	/**
	 * Set the number of still missing SW [ID] records or envents
	 *
	 * @param count				Number of missing SW [ID] records or envents
	 */
	void (*set_missing)(imv_swima_state_t *this, uint32_t count);

	/**
	 * Get the number of still missing SWID Tags or Tag IDs
	 *
	 * @result					Number of missing SWID Tags or Tag IDs
	 */
	uint32_t (*get_missing)(imv_swima_state_t *this);

	/**
	 * Set [or with multiple attributes increment] SWID Tag [ID] counters
	 *
	 * @param tag_id_count		Number of received SWID Tag IDs
	 * @param tag_count			Number of received SWID Tags
	 * @param imc_id			SWID IMC ID
	 */
	void (*set_count)(imv_swima_state_t *this, int tag_id_count, int tag_count,
					  TNC_UInt32 imc_id);

	/**
	 * Set [or with multiple attributes increment] SWID Tag [ID] counters
	 *
	 * @param tag_id_count		Number of received SWID Tag IDs
	 * @param tag_count			Number of received SWID Tags
	 */
	void (*get_count)(imv_swima_state_t *this, int *tag_id_count, int *tag_count);

	/**
	 * Get SWID IMC ID
	 *
	 * @return					SWID IMC ID
	 */
	TNC_UInt32 (*get_imc_id)(imv_swima_state_t *this);

	/**
	 * Set or clear a subscription
	 *
	 * @param set				TRUE sets and FALSE clears a subscripton
	 */
	void (*set_subscription)(imv_swima_state_t *this, bool set);

	/**
	 * Get the subscription status
	 *
	 * @return					TRUE if subscription is set
	 */
	bool (*get_subscription)(imv_swima_state_t *this);
};

/**
 * Create an imv_swima_state_t instance
 *
 * @param id			connection ID
 */
imv_state_t* imv_swima_state_create(TNC_ConnectionID id);

#endif /** IMV_SWIMA_STATE_H_ @}*/
