/*
 * Copyright (C) 2010 Andreas Steffen
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
 * @defgroup pb_tnc_state_machine pb_tnc_state_machine
 * @{ @ingroup tnccs_20
 */

#ifndef PB_TNC_STATE_MACHINE_H_
#define PB_TNC_STATE_MACHINE_H_

typedef struct pb_tnc_state_machine_t pb_tnc_state_machine_t;
typedef enum pb_tnc_state_t pb_tnc_state_t;

#include "batch/pb_tnc_batch.h"

#include <library.h>

/**
 * PB-TNC States (state machine) as defined in section 3.2 of RFC 5793
 */
enum pb_tnc_state_t {
	PB_STATE_INIT,
	PB_STATE_SERVER_WORKING,
	PB_STATE_CLIENT_WORKING,
	PB_STATE_DECIDED,
	PB_STATE_END,
};

/**
 * enum name for pb_tnc_state_t.
 */
extern enum_name_t *pb_tnc_state_names;

/**
 * Interface for the PB-TNC state machine.
 */
struct pb_tnc_state_machine_t {

	/**
	 * Get the current PB-TNC STATE
	 *
	 * @return				current state
	 */
	pb_tnc_state_t (*get_state)(pb_tnc_state_machine_t *this);

	/**
	 * Compute state transition due to received PB-TNC Batch
	 *
	 * @param type			type of received batch
	 * @result				TRUE if a valid transition was found, FALSE otherwise
	 */
	bool (*receive_batch)(pb_tnc_state_machine_t *this, pb_tnc_batch_type_t type);

	/**
	 * Compute state transition due to sent PB-TNC Batch
	 *
	 * @param type			type of sent batch
	 * @result				TRUE if a valid transition was found, FALSE otherwise
	 */
	bool (*send_batch)(pb_tnc_state_machine_t *this, pb_tnc_batch_type_t type);

	/**
	 * Informs whether the last received PB-TNC CDATA Batch was empty
	 *
	 * @result				TRUE if last received PB-TNC CDATA Batch was empty
	 */
	bool (*get_empty_cdata)(pb_tnc_state_machine_t *this);

	/**
	 * Store information whether the received PB-TNC CDATA Batch was empty
	 *
	 * @param empty			set to TRUE if received PB-TNC CDATA Batch was empty
	 */
	void (*set_empty_cdata)(pb_tnc_state_machine_t *this, bool empty);

	/**
	 * Destroys a pb_tnc_state_machine_t object.
	 */
	void (*destroy)(pb_tnc_state_machine_t *this);
};

/**
 * Create and initialize a PB-TNC state machine
 *
 * @param is_server		TRUE if PB-TNC server, FALSE if PB-TNC client
 */
pb_tnc_state_machine_t* pb_tnc_state_machine_create(bool is_server);

#endif /** PB_TNC_STATE_MACHINE_H_ @}*/
