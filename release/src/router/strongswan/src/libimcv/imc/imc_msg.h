/*
 * Copyright (C) 2012-2014 Andreas Steffen
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
 * @defgroup imc_msg imc_msg
 * @{ @ingroup libimcv_imc
 */

#ifndef IMC_MSG_H_
#define IMC_MSG_H_

#include <imc/imc_agent.h>

typedef struct imc_msg_t imc_msg_t;

#include <library.h>

/**
 * Interface for a PA-TNC message handled by an IMC.
 *
 */
struct imc_msg_t {

	/**
	 * Get source ID of PA-TNC message
	 *
	 * @return					src ID
	 */
	TNC_UInt32 (*get_src_id)(imc_msg_t *this);

	/**
	 * Get destination ID of PA-TNC message
	 *
	 * @return					destination ID
	 */
	TNC_UInt32 (*get_dst_id)(imc_msg_t *this);

	/**
	 * Get the PA-TNC message type.
	 *
	 * @return					message type
	 */
	pen_type_t (*get_msg_type)(imc_msg_t *this);

	/**
	 * Sends one or multiple PA-TNC messages
	 *
	 * @param excl				set the excl message flag if supported
	 * @return					TNC result code
	 */
	TNC_Result (*send)(imc_msg_t *this, bool excl);

	/**
	 * Processes a received PA-TNC message
	 *
	 * @param out_msg			outgoing PA-TN message
	 * @param fatal_error		TRUE if IMV sent a fatal error message
	 * @return					TNC result code
	 */
	TNC_Result (*receive)(imc_msg_t *this, imc_msg_t *out_msg,
						  bool *fatal_error);

	/**
	 * Add a PA-TNC attribute to the send queue
	 *
	 * @param attr				PA-TNC attribute to be added
	 */
	void (*add_attribute)(imc_msg_t *this, pa_tnc_attr_t *attr);

	/**
	 * Enumerator over PA-TNC attributes contained in the PA-TNC message
	 *
	 * @return					PA-TNC attribute enumerator
	 */
	enumerator_t* (*create_attribute_enumerator)(imc_msg_t *this);

	/**
	 * Get the encoding of the IMC message.
	 *
	 * @return					message encoding, internal data
	 */
	chunk_t (*get_encoding)(imc_msg_t *this);

	/**
	 * Destroys a imc_msg_t object.
	 */
	void (*destroy)(imc_msg_t *this);
};

/**
 * Create a wrapper for an outbound message
 *
 * @param agent					IMC agent responsible for the message
 * @param state					IMC state for the given connection ID
 * @param connection_id			connection ID
 * @param src_id				source IMC ID
 * @param dst_id				destination IMV ID
 * @param msg_type				PA-TNC message type
 */
imc_msg_t* imc_msg_create(imc_agent_t *agent, imc_state_t *state,
						  TNC_ConnectionID connection_id,
						  TNC_UInt32 src_id, TNC_UInt32 dst_id,
						  pen_type_t msg_type);

/**
 * Create a wrapper for an outbound message based on a received message
 *
 * @param msg					received message the reply is based on
 */
imc_msg_t* imc_msg_create_as_reply(imc_msg_t *msg);

/**
 * Create a wrapper around message data received via the legacy IF-IMC interface
 *
 * @param agent					IMC agent responsible for the message
 * @param state					IMC state for the given connection ID
 * @param connection_id			connection ID
 * @param msg_type				PA-TNC message type
 * @param msg					received PA-TNC message blob
 */
imc_msg_t* imc_msg_create_from_data(imc_agent_t *agent, imc_state_t *state,
									TNC_ConnectionID connection_id,
									TNC_MessageType msg_type,
									chunk_t msg);

/**
 * Create a wrapper around message data received via the long IF-IMC interface
 *
 * @param agent					IMC agent responsible for the message
 * @param state					IMC state for the given connection ID
 * @param connection_id			connection ID
 * @param src_id				source IMV ID
 * @param dst_id				destination IMC ID
 * @param msg_vid				PA-TNC message vendor ID
 * @param msg_subtype			PA-TNC subtype
 * @param msg					received PA-TNC message blob
 */
imc_msg_t* imc_msg_create_from_long_data(imc_agent_t *agent, imc_state_t *state,
										 TNC_ConnectionID connection_id,
										 TNC_UInt32 src_id, TNC_UInt32 dst_id,
										 TNC_VendorID msg_vid,
										 TNC_MessageSubtype msg_subtype,
										 chunk_t msg);

#endif /** IMC_MSG_H_ @}*/
