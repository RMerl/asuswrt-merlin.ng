/*
 * Copyright (C) 2011-2013 Andreas Steffen
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
 * @defgroup pb_remediation_parameters_msg pb_remediation_parameters_msg
 * @{ @ingroup tnccs_20
 */

#ifndef PB_REMEDIATION_PARAMETERS_MSG_H_
#define PB_REMEDIATION_PARAMETERS_MSG_H_

typedef enum pb_tnc_remed_param_type_t pb_tnc_remed_param_type_t;
typedef struct pb_remediation_parameters_msg_t pb_remediation_parameters_msg_t;

#include "messages/pb_tnc_msg.h"

#include <pen/pen.h>

/**
 * PB-TNC Remediation Parameter Types as defined in section 4.8.1 of RFC 5793
 */
enum pb_tnc_remed_param_type_t {
	PB_REMEDIATION_URI =			1,
	PB_REMEDIATION_STRING =			2,
};

/**
 * enum name for pb_tnc_remed_param_type_t.
 */
extern enum_name_t *pb_tnc_remed_param_type_names;

/**
 * Class representing the PB-Remediation-Parameters message type.
 */
struct pb_remediation_parameters_msg_t {

	/**
	 * PB-TNC Message interface
	 */
	pb_tnc_msg_t pb_interface;

	/**
	 * Get the Remediation Parameters Type (Vendor ID and Type)
	 *
	 * @return				Remediation Parameters Type
	 */
	pen_type_t (*get_parameters_type)(pb_remediation_parameters_msg_t *this);

	/**
	 * Get the Remediation Parameters
	 *
	 * @return				Remediation Parameters
	 */
	chunk_t (*get_parameters)(pb_remediation_parameters_msg_t *this);

	/**
	 * Get the Remediation URI
	 *
	 * @return				Remediation URI
	 */
	chunk_t (*get_uri)(pb_remediation_parameters_msg_t *this);

	/**
	 * Get the Remediation String
	 *
	 * @param lang_code		Optional Language Code
	 * @return				Remediation String
	 */
	chunk_t (*get_string)(pb_remediation_parameters_msg_t *this,
						  chunk_t *lang_code);

};

/**
 * Create a general PB-Remediation-Parameters message
 *
 * @param parameters_type	Remediation Parameters Type
 * @param parameters		Remediation Parameters
 */
pb_tnc_msg_t* pb_remediation_parameters_msg_create(pen_type_t parameters_type,
												   chunk_t parameters);

/**
 * Create a PB-Remediation-Parameters message of IETF Type Remediation URI
 *
 * @param uri				Remediation URI
 */
pb_tnc_msg_t* pb_remediation_parameters_msg_create_from_uri(chunk_t uri);

/**
 * Create a PB-Remediation-Parameters message of IETF Type Remediation String
 *
 * @param string			Remediation String
 * @param lang_code			Remediation String Language Code
 */
pb_tnc_msg_t* pb_remediation_parameters_msg_create_from_string(chunk_t string,
															   chunk_t lang_code);

/**
 * Create an unprocessed PB-Remediation-Parameters message from raw data
 *
  * @param data		PB-Remediation-Parameters message data
 */
pb_tnc_msg_t* pb_remediation_parameters_msg_create_from_data(chunk_t data);

#endif /** PB_PA_MSG_H_ @}*/
