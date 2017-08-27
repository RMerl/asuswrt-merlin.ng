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
 * @defgroup imc_manager imc_manager
 * @{ @ingroup imc
 */

#ifndef IMC_MANAGER_H_
#define IMC_MANAGER_H_

typedef struct imc_manager_t imc_manager_t;

#include "imc.h"

#include <library.h>

/**
 * The IMC manager controls all IMC instances.
 */
struct imc_manager_t {

	/**
	 * Add an IMC instance
	 *
	 * @param imc				IMC instance
	 * @return					TRUE if initialization successful
	 */
	 bool (*add)(imc_manager_t *this, imc_t *imc);

	/**
	 * Remove an IMC instance from the list and return it
	 *
	 * @param id				ID of IMC instance
	 * @return					removed IMC instance
	 */
	imc_t* (*remove)(imc_manager_t *this, TNC_IMCID id);

	/**
	 * Load and initialize an IMC as a dynamic library and add it to the list
	 *
	 * @param name				name of the IMC to be loaded
	 * @param path				path of the IMC dynamic library file
	 * @return					TRUE if loading succeeded
	 */
	bool (*load)(imc_manager_t *this, char *name, char *path);

	/**
	 * Load and initialize an IMC from a set of TNC IMC functions.
	 *
	 * @param name						name of the IMC
	 * @param initialize				TNC_IMC_InitializePointer
	 * @param notify_connection_change	TNC_IMC_NotifyConnectionChangePointer
	 * @param begin_handshake			TNC_IMC_BeginHandshakePointer
	 * @param receive_message			TNC_IMC_ReceiveMessagePointer
	 * @param receive_message_long		TNC_IMC_ReceiveMessageLongPointer
	 * @param batch_ending				TNC_IMC_BatchEndingPointer
	 * @param terminate					TNC_IMC_TerminatePointer
	 * @param provide_bind_function		TNC_IMC_ProvideBindFunctionPointer
	 * @return							TRUE if loading succeeded
	 */
	bool (*load_from_functions)(imc_manager_t *this, char *name,
				TNC_IMC_InitializePointer initialize,
				TNC_IMC_NotifyConnectionChangePointer notify_connection_change,
				TNC_IMC_BeginHandshakePointer begin_handshake,
				TNC_IMC_ReceiveMessagePointer receive_message,
				TNC_IMC_ReceiveMessageLongPointer receive_message_long,
				TNC_IMC_BatchEndingPointer batch_ending,
				TNC_IMC_TerminatePointer terminate,
				TNC_IMC_ProvideBindFunctionPointer provide_bind_function);

	/**
	 * Check if an IMC with a given ID is registered with the IMC manager
	 *
	 * @param id				ID of IMC instance
	 * @return					TRUE if registered
	 */
	bool (*is_registered)(imc_manager_t *this, TNC_IMCID id);

	/**
	 * Reserve an additional ID for an IMC
	 *
	 * @param id				ID of IMC instance
	 * @param new_id			reserved ID assigned to IMC
	 * @return					TRUE if primary IMC ID was used
	 */
	bool (*reserve_id)(imc_manager_t *this, TNC_IMCID id, TNC_UInt32 *new_id);

	/**
	 * Return the preferred language for recommendations
	 *
	 * @return					preferred language string
	 */
	char* (*get_preferred_language)(imc_manager_t *this);

	/**
	 * Notify all IMC instances
	 *
	 * @param state			communicate the state a connection has reached
	 */
	void (*notify_connection_change)(imc_manager_t *this,
									 TNC_ConnectionID id,
									 TNC_ConnectionState state);

	/**
	 * Begin a handshake between the IMCs and a connection
	 *
	 * @param id					connection ID
	 */
	void (*begin_handshake)(imc_manager_t *this, TNC_ConnectionID id);

	/**
	 * Sets the supported message types reported by a given IMC
	 *
	 * @param id					ID of reporting IMC
	 * @param supported_types		list of messages type supported by IMC
	 * @param type_count			number of supported message types
	 * @return						TNC result code
	 */
	TNC_Result (*set_message_types)(imc_manager_t *this,
									TNC_IMCID id,
									TNC_MessageTypeList supported_types,
									TNC_UInt32 type_count);

	/**
	 * Sets the supported long message types reported by a given IMC
	 *
	 * @param id					ID of reporting IMC
	 * @param supported_vids		list of vendor IDs supported by IMC
	 * @param supported_subtypes	list of messages type supported by IMC
	 * @param type_count			number of supported message types
	 * @return						TNC result code
	 */
	TNC_Result (*set_message_types_long)(imc_manager_t *this,
									TNC_IMCID id,
									TNC_VendorIDList supported_vids,
									TNC_MessageSubtypeList supported_subtypes,
									TNC_UInt32 type_count);

	/**
	 * Delivers a message to interested IMCs.
	 *
	 * @param connection_id			connection ID
	 * @param excl					exclusive message flag
	 * @param msg					message
	 * @param msg_len				message length
	 * @param msg_vid				message Vendor ID
	 * @param msg_subtype			message subtype
	 * @param src_imv_id			source IMV ID
	 * @param dst_imc_id			destination IMC ID
	 */
	void (*receive_message)(imc_manager_t *this,
							TNC_ConnectionID connection_id,
							bool excl,
							TNC_BufferReference msg,
							TNC_UInt32 msg_len,
							TNC_VendorID msg_vid,
							TNC_MessageSubtype msg_subtype,
							TNC_UInt32 src_imv_id,
							TNC_UInt32 dst_imc_id);

	/**
	 * Notify all IMCs that all IMV messages received in a batch have been
	 * delivered and this is the IMCs last chance to send a message in the
	 * batch of IMC messages currently being collected.
	 *
	 * @param id				connection ID
	 */
	void (*batch_ending)(imc_manager_t *this, TNC_ConnectionID id);

	/**
	 * Destroy an IMC manager and all its controlled instances.
	 */
	void (*destroy)(imc_manager_t *this);
};

#endif /** IMC_MANAGER_H_ @}*/
