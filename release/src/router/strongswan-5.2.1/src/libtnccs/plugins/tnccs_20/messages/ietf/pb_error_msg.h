/*
 * Copyright (C) 2010 Sansar Choinyambuu
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
 * @defgroup pb_error_msg pb_error_msg
 * @{ @ingroup tnccs_20
 */

#ifndef PB_ERROR_MSG_H_
#define PB_ERROR_MSG_H_

typedef enum pb_tnc_error_code_t pb_tnc_error_code_t;
typedef struct pb_error_msg_t pb_error_msg_t;

#include "messages/pb_tnc_msg.h"

/**
 * PB-TNC Error Codes as defined in section 4.9.1 of RFC 5793
 */
enum  pb_tnc_error_code_t {
	PB_ERROR_UNEXPECTED_BATCH_TYPE =		0,
	PB_ERROR_INVALID_PARAMETER =			1,
	PB_ERROR_LOCAL_ERROR =					2,
	PB_ERROR_UNSUPPORTED_MANDATORY_MSG =	3,
	PB_ERROR_VERSION_NOT_SUPPORTED =		4
};

/**
 * enum name for pb_tnc_error_code_t.
 */
extern enum_name_t *pb_tnc_error_code_names;

/**
 * Class representing the PB-Error message type.
 */
struct pb_error_msg_t {

	/**
	 * PB-TNC Message interface
	 */
	pb_tnc_msg_t pb_interface;

	/**
	 * Get the fatal flag
	 *
	 * @return				fatal flag
	 */
	bool (*get_fatal_flag)(pb_error_msg_t *this);

	/**
	 * Get PB Error code Vendor ID
	 *
	 * @return				PB Error Code Vendor ID
	 */
	u_int32_t (*get_vendor_id)(pb_error_msg_t *this);

	/**
	 * Get PB Error Code
	 *
	 * @return				PB Error Code
	 */
	u_int16_t (*get_error_code)(pb_error_msg_t *this);

	/**
	 * Get the PB Error Offset
	 *
	 * @return				PB Error Offset
	 */
	u_int32_t (*get_offset)(pb_error_msg_t *this);

	/**
	 * Get the PB Bad Version
	 *
	 * @return				PB Bad Version
	 */
	u_int8_t (*get_bad_version)(pb_error_msg_t *this);

	/**
	 * Set the PB Bad Version
	 *
	 * @param version		PB Bad Version
	 */
	void (*set_bad_version)(pb_error_msg_t *this, u_int8_t version);
};

/**
 * Create a PB-Error message from parameters
 *
 * @param fatal				fatal flag
 * @param vendor_id			Error Code Vendor ID
 * @param error_code		Error Code
 */
pb_tnc_msg_t* pb_error_msg_create(bool fatal, u_int32_t vendor_id,
								  pb_tnc_error_code_t error_code);
	
/**
 * Create a PB-Error message from parameters with offset field
 *
 * @param fatal				fatal flag
 * @param vendor_id			Error Code Vendor ID
 * @param error_code		Error Code
 * @param error_offset		Error Offset
 */
pb_tnc_msg_t* pb_error_msg_create_with_offset(bool fatal, u_int32_t vendor_id,
											  pb_tnc_error_code_t error_code,
											  u_int32_t error_offset);

/**
 * Create an unprocessed PB-Error message from raw data
 *
 * @param data				PB-Error message data
 */
pb_tnc_msg_t* pb_error_msg_create_from_data(chunk_t data);

#endif /** PB_PA_MSG_H_ @}*/
