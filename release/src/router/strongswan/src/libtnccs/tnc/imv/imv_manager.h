/*
 * Copyright (C) 2010-2013 Andreas Steffen
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
 * @defgroup imv_manager imv_manager
 * @{ @ingroup imv
 */

#ifndef IMV_MANAGER_H_
#define IMV_MANAGER_H_

typedef struct imv_manager_t imv_manager_t;

#include "imv.h"
#include "imv_recommendations.h"

#include <library.h>

/**
 * The IMV manager controls all IMV instances.
 */
struct imv_manager_t {

	/**
	 * Add an IMV instance
	 *
	 * @param imv				IMV instance
	 * @return					TRUE if initialization successful
	 */
	bool (*add)(imv_manager_t *this, imv_t *imv);

	/**
	 * Remove an IMV instance from the list and return it
	 *
	 * @param id				ID of IMV instance
	 * @return					removed IMC instance
	 */
	imv_t* (*remove)(imv_manager_t *this, TNC_IMVID id);

	/**
	 * Load and initialize an IMV as a dynamic library and add it to the list
	 *
	 * @param name				name of the IMV to be loaded
	 * @param path				path of the IMV dynamic library file
	 * @return					TRUE if loading succeeded
	 */
	bool (*load)(imv_manager_t *this, char *name, char *path);

	/**
	 * Load and initialize an IMV from a set of TNC IMC functions.
	 *
	 * @param name						name of the IMV
	 * @param initialize				TNC_IMV_InitializePointer
	 * @param notify_connection_change	TNC_IMV_NotifyConnectionChangePointer
	 * @param receive_message			TNC_IMV_ReceiveMessagePointer
	 * @param receive_message_long		TNC_IMV_ReceiveMessageLongPointer
	 * @param solicit_recommendation	TNC_IMV_SolicitRecommendationPointer
	 * @param batch_ending				TNC_IMV_BatchEndingPointer
	 * @param terminate					TNC_IMV_TerminatePointer
	 * @param provide_bind_function		TNC_IMV_ProvideBindFunctionPointer
	 * @return							TRUE if loading succeeded
	 */
	bool (*load_from_functions)(imv_manager_t *this, char *name,
				TNC_IMV_InitializePointer initialize,
				TNC_IMV_NotifyConnectionChangePointer notify_connection_change,
				TNC_IMV_ReceiveMessagePointer receive_message,
				TNC_IMV_ReceiveMessageLongPointer receive_message_long,
				TNC_IMV_SolicitRecommendationPointer solicit_recommendation,
				TNC_IMV_BatchEndingPointer batch_ending,
				TNC_IMV_TerminatePointer terminate,
				TNC_IMV_ProvideBindFunctionPointer provide_bind_function);

	/**
	 * Check if an IMV with a given ID is registered with the IMV manager
	 *
	 * @param id				ID of IMV instance
	 * @return					TRUE if registered
	 */
	bool (*is_registered)(imv_manager_t *this, TNC_IMVID id);

	/**
	 * Reserve an additional ID for an IMV
	 *
	 * @param id				ID of IMV instance
	 * @param new_id			reserved ID assigned to IMV
	 * @return					TRUE if primary IMV ID was used
	 */
	bool (*reserve_id)(imv_manager_t *this, TNC_IMVID id, TNC_UInt32 *new_id);

	/**
	 * Get the configured recommendation policy
	 *
	 * @return					configured recommendation policy
	 */
	recommendation_policy_t (*get_recommendation_policy)(imv_manager_t *this);

	/**
	 * Create an empty set of IMV recommendations and evaluations
	 *
	 * @return					instance of a recommendations_t list
	 */
	recommendations_t* (*create_recommendations)(imv_manager_t *this);

	/**
	 * Notify all IMV instances
	 *
	 * @param state			communicate the state a connection has reached
	 */
	void (*notify_connection_change)(imv_manager_t *this,
									 TNC_ConnectionID id,
									 TNC_ConnectionState state);

	/**
	 * Sets the supported message types reported by a given IMV
	 *
	 * @param id					ID of reporting IMV
	 * @param supported_types		list of messages type supported by IMV
	 * @param type_count			number of supported message types
	 * @return						TNC result code
	 */
	TNC_Result (*set_message_types)(imv_manager_t *this,
									TNC_IMVID id,
									TNC_MessageTypeList supported_types,
									TNC_UInt32 type_count);

	/**
	 * Sets the supported long message types reported by a given IMV
	 *
	 * @param id					ID of reporting IMV
	 * @param supported_vids		list of vendor IDs supported by IMV
	 * @param supported_subtypes	list of messages type supported by IMV
	 * @param type_count			number of supported message types
	 * @return						TNC result code
	 */
	TNC_Result (*set_message_types_long)(imv_manager_t *this,
									TNC_IMVID id,
									TNC_VendorIDList supported_vids,
									TNC_MessageSubtypeList supported_subtypes,
									TNC_UInt32 type_count);

	/**
	 * Solicit recommendations from IMVs that have not yet provided one
	 *
	 * @param id					connection ID
	 */
	void (*solicit_recommendation)(imv_manager_t *this, TNC_ConnectionID id);

	/**
	 * Delivers a message to interested IMVs.
	 *
	 * @param connection_id			connection ID
	 * @param excl					exclusive message flag
	 * @param msg					message
	 * @param msg_len				message length
	 * @param msg_vid				message Vendor ID
	 * @param msg_subtype			message subtype
	 * @param src_imc_id			source IMC ID
	 * @param dst_imv_id			destination IMV ID
	 */
	void (*receive_message)(imv_manager_t *this,
							TNC_ConnectionID connection_id,
							bool excl,
							TNC_BufferReference msg,
							TNC_UInt32 msg_len,
							TNC_VendorID msg_vid,
							TNC_MessageSubtype msg_subtype,
							TNC_UInt32 src_imc_id,
							TNC_UInt32 dst_imv_id);

	/**
	 * Notify all IMVs that all IMC messages received in a batch have been
	 * delivered and this is the IMVs last chance to send a message in the
	 * batch of IMV messages currently being collected.
	 *
	 * @param id				connection ID
	 */
	void (*batch_ending)(imv_manager_t *this, TNC_ConnectionID id);

	/**
	 * Destroy an IMV manager and all its controlled instances.
	 */
	void (*destroy)(imv_manager_t *this);
};

#endif /** IMV_MANAGER_H_ @}*/
