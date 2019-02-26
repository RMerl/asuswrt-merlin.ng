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
 * @defgroup imv_msg_t imv_msg
 * @{ @ingroup libimcv_imv
 */

#ifndef IMV_MSG_H_
#define IMV_MSG_H_

#include <imv/imv_agent.h>

typedef struct imv_msg_t imv_msg_t;

#include <library.h>

/**
 * Interface for a PA-TNC message handled by an IMV.
 *
 */
struct imv_msg_t {

	/**
	 * Get source ID of PA-TNC message
	 *
	 * @return					src ID
	 */
	TNC_UInt32 (*get_src_id)(imv_msg_t *this);

	/**
	 * Get destination ID of PA-TNC message
	 *
	 * @return					destination ID
	 */
	TNC_UInt32 (*get_dst_id)(imv_msg_t *this);

	/**
	 * Set the type of a PA-TNC message
	 *
	 * @param msg_type			message type
	 */
	void (*set_msg_type)(imv_msg_t *this, pen_type_t msg_type);

	/**
	 * Get the type of a PA-TNC message.
	 *
	 * @return					message type
	 */
	pen_type_t (*get_msg_type)(imv_msg_t *this);

	/**
	 * Sends one or multiple PA-TNC messages
	 *
	 * @param excl				set the excl message flag if supported
	 * @return					TNC result code
	 */
	TNC_Result (*send)(imv_msg_t *this, bool excl);

	/**
	 * Send a PA-TNC message containing an IETF Assessment Result attribute
	 *
	 * @return					TNC result code
	 */
	TNC_Result (*send_assessment)(imv_msg_t *this);

	/**
	 * Processes a received PA-TNC message
	 *
	 * @param out_msg			outgoing PA-TN message
	 * @param fatal_error		TRUE if IMC sent a fatal error message
	 * @return					TNC result code
	 */
	TNC_Result (*receive)(imv_msg_t *this, imv_msg_t *out_msg,
						  bool *fatal_error);

	/**
	 * Add a PA-TNC attribute to the send queue
	 *
	 * @param attr				PA-TNC attribute to be added
	 */
	void (*add_attribute)(imv_msg_t *this, pa_tnc_attr_t *attr);

	/**
	 * Get the number of PA-TNC attributes in the send queue
	 *
	 * @return					number of PA-TNC attribute in send queue
	 */
	int (*get_attribute_count)(imv_msg_t *this);

	/**
	 * Enumerator over PA-TNC attributes contained in the PA-TNC message
	 *
	 * @return					PA-TNC attribute enumerator
	 */
	enumerator_t* (*create_attribute_enumerator)(imv_msg_t *this);

	/**
	 * Get the full encoding of an IMV message.
	 *
	 * @return					message encoding, internal data
	 */
	chunk_t (*get_encoding)(imv_msg_t *this);

	/**
	 * Destroys a imv_msg_t object.
	 */
	void (*destroy)(imv_msg_t *this);
};

/**
 * Create a wrapper for an outbound message
 *
 * @param agent					IMV agent responsible for the message
 * @param state					IMV state for the given connection ID
 * @param connection_id			connection ID
 * @param src_id				source IMV ID
 * @param dst_id				destination IMC ID
 * @param msg_type				PA-TNC message type
 */
imv_msg_t* imv_msg_create(imv_agent_t *agent, imv_state_t *state,
						  TNC_ConnectionID connection_id,
						  TNC_UInt32 src_id, TNC_UInt32 dst_id,
						  pen_type_t msg_type);

/**
 * Create a wrapper for an outbound message based on a received message
 *
 * @param msg					received message the reply is based on
 */
imv_msg_t* imv_msg_create_as_reply(imv_msg_t *msg);

/**
 * Create a wrapper around message data received via the legacy IF-IMV interface
 *
 * @param agent					IMV agent responsible for the message
 * @param state					IMV state for the given connection ID
 * @param connection_id			connection ID
 * @param msg_type				PA-TNC message type
 * @param msg					received PA-TNC message blob
 */
imv_msg_t* imv_msg_create_from_data(imv_agent_t *agent, imv_state_t *state,
									TNC_ConnectionID connection_id,
									TNC_MessageType msg_type,
									chunk_t msg);

/**
 * Create a wrapper around message data received via the long IF-IMV interface
 *
 * @param agent					IMV agent responsible for the message
 * @param state					IMV state for the given connection ID
 * @param connection_id			connection ID
 * @param src_id				source IMC ID
 * @param dst_id				destination IMV ID
 * @param msg_vid				PA-TNC message vendor ID
 * @param msg_subtype			PA-TNC subtype
 * @param msg					received PA-TNC message blob
 */
imv_msg_t* imv_msg_create_from_long_data(imv_agent_t *agent, imv_state_t *state,
										 TNC_ConnectionID connection_id,
										 TNC_UInt32 src_id, TNC_UInt32 dst_id,
										 TNC_VendorID msg_vid,
										 TNC_MessageSubtype msg_subtype,
										 chunk_t msg);

#endif /** IMV_MSG_H_ @}*/
