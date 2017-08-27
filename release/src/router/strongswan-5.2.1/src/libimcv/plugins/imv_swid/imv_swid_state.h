/*
 * Copyright (C) 2013-2014 Andreas Steffen
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
 * @defgroup imv_swid imv_swid
 * @ingroup libimcv_plugins
 *
 * @defgroup imv_swid_state_t imv_swid_state
 * @{ @ingroup imv_swid
 */

#ifndef IMV_SWID_STATE_H_
#define IMV_SWID_STATE_H_

#include <imv/imv_state.h>
#include <swid/swid_inventory.h>
#include <library.h>

#include <json.h>

typedef struct imv_swid_state_t imv_swid_state_t;
typedef enum imv_swid_handshake_state_t imv_swid_handshake_state_t;

/**
 * IMV OS Handshake States (state machine)
 */
enum imv_swid_handshake_state_t {
	IMV_SWID_STATE_INIT,
	IMV_SWID_STATE_WORKITEMS,
	IMV_SWID_STATE_END
};

/**
 * Internal state of an imv_swid_t connection instance
 */
struct imv_swid_state_t {

	/**
	 * imv_state_t interface
	 */
	imv_state_t interface;

	/**
	 * Set state of the handshake
	 *
	 * @param new_state			the handshake state of IMV
	 */
	void (*set_handshake_state)(imv_swid_state_t *this,
								imv_swid_handshake_state_t new_state);

	/**
	 * Get state of the handshake
	 *
	 * @return					the handshake state of IMV
	 */
	imv_swid_handshake_state_t (*get_handshake_state)(imv_swid_state_t *this);

	/**
	 * Set the SWID request ID
	 *
	 * @param request_id		SWID request ID to be set
	 */
	void (*set_request_id)(imv_swid_state_t *this, uint32_t request_id);

	/**
	 * Get the SWID request ID
	 *
	 * @return					SWID request ID
	 */
	uint32_t (*get_request_id)(imv_swid_state_t *this);

    /**
     * Set or extend the SWID Tag ID inventory in the state
     *
     * @param inventory			SWID Tags ID inventory to be added
     */
    void (*set_swid_inventory)(imv_swid_state_t *this, swid_inventory_t *inventory);

   /**
     * Get the encoding of the complete SWID Tag ID inventory
     *
     * @return			       SWID Tags ID inventory as a JSON array
     */
    json_object* (*get_swid_inventory)(imv_swid_state_t *this);

	/**
	 * Set the number of still missing SWID Tags or Tag IDs
	 *
	 * @param count				Number of missing SWID Tags or Tag IDs
	 */
	void (*set_missing)(imv_swid_state_t *this, uint32_t count);

	/**
	 * Get the number of still missing SWID Tags or Tag IDs
	 *
	 * @result					Number of missing SWID Tags or Tag IDs
	 */
	uint32_t (*get_missing)(imv_swid_state_t *this);

	/**
	 * Set [or with multiple attributes increment] SWID Tag [ID] counters
	 *
	 * @param tag_id_count		Number of received SWID Tag IDs
	 * @param tag_count			Number of received SWID Tags
	 */
	void (*set_count)(imv_swid_state_t *this, int tag_id_count, int tag_count);

	/**
	 * Set [or with multiple attributes increment] SWID Tag [ID] counters
	 *
	 * @param tag_id_count		Number of received SWID Tag IDs
	 * @param tag_count			Number of received SWID Tags
	 */
	void (*get_count)(imv_swid_state_t *this, int *tag_id_count, int *tag_count);
};

/**
 * Create an imv_swid_state_t instance
 *
 * @param id			connection ID
 */
imv_state_t* imv_swid_state_create(TNC_ConnectionID id);

#endif /** IMV_SWID_STATE_H_ @}*/
