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
 * @defgroup imc_agent_t imc_agent
 * @{ @ingroup libimcv_imc
 */

#ifndef IMC_AGENT_H_
#define IMC_AGENT_H_

#include "imc_state.h"
#include "pa_tnc/pa_tnc_msg.h"

#include <tncifimc.h>
#include <pen/pen.h>
#include <collections/linked_list.h>

#include <library.h>

typedef struct imc_agent_t imc_agent_t;

/**
 * Core functions of an Integrity Measurement Verifier (IMC)
 */
struct imc_agent_t {

	/**
	 * Ask a TNCC to retry an Integrity Check Handshake
	 *
	 * @param imc_id			IMC ID assigned by TNCC
	 * @param connection_id		network connection ID assigned by TNCC
	 * @param reason			IMC retry reason
	 * @return					TNC result code
	 */
	TNC_Result (*request_handshake_retry)(TNC_IMCID imc_id,
										  TNC_ConnectionID connection_id,
										  TNC_RetryReason reason);

	/**
	 * Call when an IMC-IMC message is to be sent
	 *
	 * @param imc_id			IMC ID assigned by TNCC
	 * @param connection_id		network connection ID assigned by TNCC
	 * @param msg				message to send
	 * @param msg_len			message length in bytes
	 * @param msg_type			message type
	 * @return					TNC result code
	 */
	TNC_Result (*send_message)(TNC_IMCID imc_id,
							   TNC_ConnectionID connection_id,
							   TNC_BufferReference msg,
							   TNC_UInt32 msg_len,
							   TNC_MessageType msg_type);

	/**
	 * Call when an IMC-IMC message is to be sent with long message types
	 *
	 * @param imc_id			IMC ID assigned by TNCC
	 * @param connection_id		network connection ID assigned by TNCC
	 * @param msg_flags			message flags
	 * @param msg				message to send
	 * @param msg_len			message length in bytes
	 * @param msg_vid			message vendor ID
	 * @param msg_subtype		message subtype
	 * @param dst_imc_id		destination IMV ID
	 * @return					TNC result code
	 */
	TNC_Result (*send_message_long)(TNC_IMCID imc_id,
									TNC_ConnectionID connection_id,
									TNC_UInt32 msg_flags,
									TNC_BufferReference msg,
									TNC_UInt32 msg_len,
									TNC_VendorID msg_vid,
									TNC_MessageSubtype msg_subtype,
									TNC_UInt32 dst_imv_id);

	/**
	 * Bind TNCC functions
	 *
	 * @param bind_function		function offered by the TNCC
	 * @return					TNC result code
	 */
	TNC_Result (*bind_functions)(imc_agent_t *this,
								 TNC_TNCC_BindFunctionPointer bind_function);

	/**
	 * Create the IMC state for a TNCCS connection instance
	 *
	 * @param state				internal IMC state instance
	 * @return					TNC result code
	 */
	TNC_Result (*create_state)(imc_agent_t *this, imc_state_t *state);

	/**
	 * Delete the IMC state for a TNCCS connection instance
	 *
	 * @param connection_id		network connection ID assigned by TNCS
	 * @return					TNC result code
	 */
	TNC_Result (*delete_state)(imc_agent_t *this,
							   TNC_ConnectionID connection_id);

	/**
	 * Change the current state of a TNCCS connection
	 *
	 * @param connection_id		network connection ID assigned by TNCS
	 * @param new_state			new state of TNCCS connection
	 * @param state_p			internal IMC state instance [optional argument]
	 * @return					TNC result code
	 */
	TNC_Result (*change_state)(imc_agent_t *this,
							   TNC_ConnectionID connection_id,
							   TNC_ConnectionState new_state,
							   imc_state_t **state_p);

	/**
	 * Get the IMC state for a TNCCS connection instance
	 *
	 * @param connection_id		network connection ID assigned by TNCS
	 * @param state				internal IMC state instance
	 * @return					TRUE if the state was found
	 */
	bool (*get_state)(imc_agent_t *this,
					  TNC_ConnectionID connection_id, imc_state_t **state);

	/**
	 * Get IMC name
	 *
	 * return					IMC name
	 */
	const char* (*get_name)(imc_agent_t *this);

	/**
	 * Get base IMC ID
	 *
	 * return					base IMC ID
	 */
	TNC_IMCID (*get_id)(imc_agent_t *this);

	/**
	 * Reserve additional IMC IDs from TNCC
	 *
	 * @param count				number of additional IMC IDs to be assigned
	 * @return					TNC result code
	 */
	TNC_Result (*reserve_additional_ids)(imc_agent_t *this, int count);

	/**
	 * Return the number of additional IMC IDs assigned by the TNCC
	 *
	 * @return					number of additional IMC IDs
	 */
	int (*count_additional_ids)(imc_agent_t *this);

	/**
	 * Create an enumerator for the additional IMC IDs
	 */
	enumerator_t* (*create_id_enumerator)(imc_agent_t *this);

	/**
	 * Add an item to the list of non-fatal unsupported PA-TNC attribute types
	 */
	void (*add_non_fatal_attr_type)(imc_agent_t *this, pen_type_t type);

	/**
	 * Get a list of non-fatal unsupported PA-TNC attribute types
	 */
	linked_list_t* (*get_non_fatal_attr_types)(imc_agent_t *this);

	/**
	 * Is the transport protocol PT-TLS?
	 *
	 * return					TRUE if PT-TLS
	 */
	bool (*has_pt_tls)(imc_agent_t *this);

	/**
	 * Destroys an imc_agent_t object
	 */
	void (*destroy)(imc_agent_t *this);
};

/**
 * Create an imc_agent_t object
 *
 * @param name				name of the IMC
 * @param supported_types	list of message types registered by the IMC
 * @param type_count		number of registered message types
 * @param id				ID of the IMC as assigned by the TNCS
 * @param actual_version	actual version of the IF-IMC API
 *
 */
imc_agent_t *imc_agent_create(const char *name,
							  pen_type_t *supported_types, uint32_t type_count,
							  TNC_IMCID id, TNC_Version *actual_version);

#endif /** IMC_AGENT_H_ @}*/
