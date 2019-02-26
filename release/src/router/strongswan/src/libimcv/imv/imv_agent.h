/*
 * Copyright (C) 2011-2014 Andreas Steffen
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
 *
 * @defgroup imv_agent_t imv_agent
 * @{ @ingroup libimcv_imv
 */

#ifndef IMV_AGENT_H_
#define IMV_AGENT_H_

#include "imv_state.h"
#include "imv_database.h"
#include "pa_tnc/pa_tnc_msg.h"

#include <tncifimv.h>
#include <pen/pen.h>
#include <collections/linked_list.h>

#include <library.h>

typedef struct imv_agent_t imv_agent_t;

/**
 * Core functions of an Integrity Measurement Verifier (IMV)
 */
struct imv_agent_t {

	/**
	 * Ask a TNCS to retry an Integrity Check Handshake
	 *
	 * @param imv_id			IMV ID assigned by TNCS
	 * @param connection_id		network connection ID assigned by TNCS
	 * @param reason			IMV retry reason
	 * @return					TNC result code
	 */
	TNC_Result (*request_handshake_retry)(TNC_IMVID imv_id,
										  TNC_ConnectionID connection_id,
										  TNC_RetryReason reason);

	/**
	 * Call when an IMV-IMC message is to be sent
	 *
	 * @param imv_id			IMV ID assigned by TNCS
	 * @param connection_id		network connection ID assigned by TNCS
	 * @param msg				message to send
	 * @param msg_len			message length in bytes
	 * @param msg_type			message type
	 * @return					TNC result code
	 */
	TNC_Result (*send_message)(TNC_IMVID imv_id,
							   TNC_ConnectionID connection_id,
							   TNC_BufferReference msg,
							   TNC_UInt32 msg_len,
							   TNC_MessageType msg_type);

	/**
	 * Call when an IMV-IMC message is to be sent with long message types
	 *
	 * @param imv_id			IMV ID assigned by TNCS
	 * @param connection_id		network connection ID assigned by TNCS
	 * @param msg_flags			message flags
	 * @param msg				message to send
	 * @param msg_len			message length in bytes
	 * @param msg_vid			message vendor ID
	 * @param msg_subtype		message subtype
	 * @param dst_imc_id		destination IMC ID
	 * @return					TNC result code
	 */
	TNC_Result (*send_message_long)(TNC_IMVID imv_id,
									TNC_ConnectionID connection_id,
									TNC_UInt32 msg_flags,
									TNC_BufferReference msg,
									TNC_UInt32 msg_len,
									TNC_VendorID msg_vid,
									TNC_MessageSubtype msg_subtype,
									TNC_UInt32 dst_imc_id);

	/**
	 * Bind TNCS functions
	 *
	 * @param bind_function		function offered by the TNCS
	 * @return					TNC result code
	 */
	TNC_Result (*bind_functions)(imv_agent_t *this,
								 TNC_TNCS_BindFunctionPointer bind_function);

	/**
	 * Create the IMV state for a TNCCS connection instance
	 *
	 * @param state				internal IMV state instance
	 * @return					TNC result code
	 */
	TNC_Result (*create_state)(imv_agent_t *this, imv_state_t *state);

	/**
	 * Delete the IMV state for a TNCCS connection instance
	 *
	 * @param connection_id		network connection ID assigned by TNCS
	 * @return					TNC result code
	 */
	TNC_Result (*delete_state)(imv_agent_t *this,
							   TNC_ConnectionID connection_id);

	/**
	 * Change the current state of a TNCCS connection
	 *
	 * @param connection_id		network connection ID assigned by TNCS
	 * @param new_state			new state of TNCCS connection
	 * @param state_p			internal IMV state instance [optional argument]
	 * @return					TNC result code
	 */
	TNC_Result (*change_state)(imv_agent_t *this,
							   TNC_ConnectionID connection_id,
							   TNC_ConnectionState new_state,
							   imv_state_t **state_p);

	/**
	 * Get the IMV state for a TNCCS connection instance
	 *
	 * @param connection_id		network connection ID assigned by TNCS
	 * @param state				internal IMV state instance
	 * @return					TRUE if the state was found
	 */
	bool (*get_state)(imv_agent_t *this,
					  TNC_ConnectionID connection_id, imv_state_t **state);

	/**
	 * Get IMV name
	 *
	 * return					IMV name
	 */
	const char* (*get_name)(imv_agent_t *this);

	/**
	 * Get base IMV ID
	 *
	 * return					base IMV ID
	 */
	TNC_IMVID (*get_id)(imv_agent_t *this);

	/**
	 * Reserve additional IMV IDs from TNCS
	 *
	 * @param count				number of additional IMV IDs to be assigned
	 * @return					TNC result code
	 */
	TNC_Result (*reserve_additional_ids)(imv_agent_t *this, int count);

	/**
	 * Return the number of additional IMV IDs assigned by the TNCS
	 *
	 * @return					number of additional IMV IDs
	 */
	int (*count_additional_ids)(imv_agent_t *this);

	/**
	 * Create an enumerator for the additional IMV IDs
	 */
	enumerator_t* (*create_id_enumerator)(imv_agent_t *this);

	/**
	 * Create a preferred languages enumerator
	 *
	 * @param					state of TNCCS connection
	 */
	enumerator_t* (*create_language_enumerator)(imv_agent_t *this,
				   imv_state_t *state);

	/**
	 * Deliver IMV Action Recommendation and IMV Evaluation Result to the TNCS
	 *
	 * @param state				state bound to a connection ID
	 * @return					TNC result code
	 */
	TNC_Result (*provide_recommendation)(imv_agent_t *this, imv_state_t* state);

	/**
	 * Add an item to the list of non-fatal unsupported PA-TNC attribute types
	 */
	void (*add_non_fatal_attr_type)(imv_agent_t *this, pen_type_t type);

	/**
	 * Get a list of non-fatal unsupported PA-TNC attribute types
	 */
	linked_list_t* (*get_non_fatal_attr_types)(imv_agent_t *this);

	/**
	 * Destroys an imv_agent_t object
	 */
	void (*destroy)(imv_agent_t *this);
};

/**
 * Create an imv_agent_t object
 *
 * @param name				name of the IMV
 * @param supported_types	list of message types registered by the IMV
 * @param type_count		number of registered message types
 * @param id				ID of the IMV as assigned by the TNCS
 * @param actual_version	actual version of the IF-IMV API
 *
 */
imv_agent_t *imv_agent_create(const char *name,
							  pen_type_t *supported_types, uint32_t type_count,
							  TNC_IMVID id, TNC_Version *actual_version);

#endif /** IMV_AGENT_H_ @}*/
