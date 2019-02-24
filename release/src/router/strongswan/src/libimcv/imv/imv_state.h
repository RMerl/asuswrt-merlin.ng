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
 * @defgroup imv_state_t imv_state
 * @{ @ingroup libimcv_imv
 */

#ifndef IMV_STATE_H_
#define IMV_STATE_H_

#include "imv_session.h"
#include "seg/seg_contract_manager.h"

#include <tncifimv.h>

#include <library.h>

typedef struct imv_state_t imv_state_t;

/**
 * Internal state of an IMV connection instance
 */
struct imv_state_t {

	/**
	 * Get the TNCCS connection ID attached to the state
	 *
	 * @return				TNCCS connection ID of the state
	 */
	 TNC_ConnectionID (*get_connection_id)(imv_state_t *this);

	/**
	 * Checks if long message types are supported for this TNCCS connection
	 *
	 * @return				TRUE if set, FALSE otherwise
	 */
	bool (*has_long)(imv_state_t *this);

	/**
	 * Checks if the exclusive delivery is supported for this TNCCS connection
	 *
	 * @return				TRUE if set, FALSE otherwise
	 */
	bool (*has_excl)(imv_state_t *this);

	/**
	 * Sets the long message types and exclusive flags for this TNCCS connection
	 *
	 * @param has_long		TNCCS connection supports long message types
	 * @param has_excl		TNCCS connection supports exclusive delivery
	 * @return				TRUE if set, FALSE otherwise
	 */
	void (*set_flags)(imv_state_t *this, bool has_long, bool has_excl);

	/**
	 * Set the maximum size of a PA-TNC message for this TNCCS connection
	 *
	 * @param max_msg_len	maximum size of a PA-TNC message
	 */
	void (*set_max_msg_len)(imv_state_t *this, uint32_t max_msg_len);

	/**
	 * Get the maximum size of a PA-TNC message for this TNCCS connection
	 *
	 * @return				maximum size of a PA-TNC message
	 */
	uint32_t (*get_max_msg_len)(imv_state_t *this);

	/**
	 * Set flags for completed actions
	 *
	 * @param flags			Flags to be set
	 */
	void (*set_action_flags)(imv_state_t *this, uint32_t flags);

	/**
	 * Get flags set for completed actions
	 *
	 * @return				Flags set for completed actions
	 */
	uint32_t (*get_action_flags)(imv_state_t *this);

	/**
	 * Set session associated with TNCCS Connection
	 *
	 * @param session		Session associated with TNCCS Connection
	 */
	void (*set_session)(imv_state_t *this, imv_session_t *session);

	/**
	 * Get session associated with TNCCS Connection
	 *
	 * @return				Session associated with TNCCS Connection
	 */
	imv_session_t* (*get_session)(imv_state_t *this);

	/**
	 * Get attribute segmentation contracts associated with TNCCS Connection
	 *
	 * @return				Contracts associated with TNCCS Connection
	 */
	seg_contract_manager_t* (*get_contracts)(imv_state_t *this);

	/**
	 * Change the connection state
	 *
	 * @param new_state		new connection state
	 * @return				old connection state
	 */
	TNC_ConnectionState (*change_state)(imv_state_t *this,
						 TNC_ConnectionState new_state);

	/**
	 * Get IMV action recommendation and evaluation result
	 *
	 * @param rec			IMV action recommendation
	 * @param eval			IMV evaluation result
	 *
	 */
	void (*get_recommendation)(imv_state_t *this,
							   TNC_IMV_Action_Recommendation *rec,
							   TNC_IMV_Evaluation_Result *eval);

	/**
	 * Set IMV action recommendation and evaluation result
	 *
	 * @param rec			IMV action recommendation
	 * @param eval			IMV evaluation result
	 *
	 */
	void (*set_recommendation)(imv_state_t *this,
							   TNC_IMV_Action_Recommendation rec,
							   TNC_IMV_Evaluation_Result eval);

	/**
	 * Update IMV action recommendation and evaluation result
	 *
	 * @param rec			IMV action recommendation
	 * @param eval			IMV evaluation result
	 *
	 */
	void (*update_recommendation)(imv_state_t *this,
								  TNC_IMV_Action_Recommendation rec,
								  TNC_IMV_Evaluation_Result eval);

	/**
	 * Get reason string based on the preferred language
	 *
	 * @param language_enumerator	language enumerator
	 * @param reason_string			reason string
	 * @param reason_language		language of the returned reason string
	 * @return						TRUE if a reason string was found
	 */
	bool (*get_reason_string)(imv_state_t *this,
							  enumerator_t *language_enumerator,
							  chunk_t *reason_string, char **reason_language);

	/**
	 * Get remediation instructions based on the preferred language
	 *
	 * @param language_enumerator	language enumerator
	 * @param string				remediation instruction string
	 * @param lang_code				language of the remediation instructions
	 * @param uri					remediation URI
	 * @return						TRUE if remediation instructions were found
	 */
	bool (*get_remediation_instructions)(imv_state_t *this,
										 enumerator_t *language_enumerator,
										 chunk_t *string, char **lang_code,
										 char **uri);

	/**
	 * Resets the state for a new measurement cycle triggered by a SRETRY batch
	 */
	void (*reset)(imv_state_t *this);

	/**
	 * Destroys an imv_state_t object
	 */
	void (*destroy)(imv_state_t *this);
};

#endif /** IMV_STATE_H_ @}*/
