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
 * @defgroup ietf_attr_pa_tnc_errort ietf_attr_pa_tnc_error
 * @{ @ingroup ietf_attr
 */

#ifndef IETF_ATTR_PA_TNC_ERROR_H_
#define IETF_ATTR_PA_TNC_ERROR_H_

typedef struct ietf_attr_pa_tnc_error_t ietf_attr_pa_tnc_error_t;
typedef enum pa_tnc_error_code_t pa_tnc_error_code_t;

#include "ietf_attr.h"
#include "pa_tnc/pa_tnc_attr.h"


/**
 * IETF Standard PA-TNC Error Codes as defined in section 4.2.8 of RFC 5792
 */
enum  pa_tnc_error_code_t {
    PA_ERROR_RESERVED =                 0,
	PA_ERROR_INVALID_PARAMETER =        1,
	PA_ERROR_VERSION_NOT_SUPPORTED =    2,
	PA_ERROR_ATTR_TYPE_NOT_SUPPORTED =  3,
};

/**
 * enum name for pa_tnc_error_code_t.
 */
extern enum_name_t *pa_tnc_error_code_names;

/**
 * Class implementing the IETF PA-TNC Error attribute.
 *
 */
struct ietf_attr_pa_tnc_error_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Get Vendor-specific PA-TNC error code
	 *
	 * @return				error code
	 */
	pen_type_t (*get_error_code)(ietf_attr_pa_tnc_error_t *this);

	/**
	 * Get first 8 bytes of erroneous PA-TNC message
	 *
	 * @return				PA-TNC message info
	 */
	chunk_t (*get_msg_info)(ietf_attr_pa_tnc_error_t *this);

	/**
	 * Get flags, vendor ID and type of unsupported PA-TNC attribute
	 *
	 * @param flags			PA-TNC attribute flags
	 * @return				PA-TNC attribute vendor ID and type
	 */
	pen_type_t (*get_unsupported_attr)(ietf_attr_pa_tnc_error_t *this,
									   uint8_t *flags);

	/**
	 * Set flags, vendor ID and type of unsupported PA-TNC attribute
	 *
	 * @param flags			PA-TNC attribute flags
	 * @param attr_info		PA-TNC attribute vendor ID and type
	 */
	void (*set_unsupported_attr)(ietf_attr_pa_tnc_error_t *this, uint8_t flags,
								 pen_type_t type);

	/**
	 * Get the PA-TNC error offset
	 *
	 * @return				PA-TNC error offset
	 */
	uint32_t (*get_offset)(ietf_attr_pa_tnc_error_t *this);

};

/**
 * Creates an ietf_attr_pa_tnc_error_t object from an error code
 *
 * @param error_code		Vendor-specific PA-TNC error code
 * @param header			PA-TNC message header (first 8 bytes)
 *
 */
pa_tnc_attr_t* ietf_attr_pa_tnc_error_create(pen_type_t error_code,
											 chunk_t header);

/**
 * Creates an ietf_attr_pa_tnc_error_t object from an error code with offset
 *
 * @param error_code		Vendor-specifica PA-TNC error code
 * @param header			PA-TNC message header (first 8 bytes)
 * @param error_offset		PA-TNC error offset in bytes
 *
 */
pa_tnc_attr_t* ietf_attr_pa_tnc_error_create_with_offset(pen_type_t error_code,
														 chunk_t header,
														 uint32_t error_offset);

/**
 * Creates an ietf_attr_pa_tnc_error_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* ietf_attr_pa_tnc_error_create_from_data(size_t length,
													   chunk_t value);

#endif /** IETF_ATTR_PA_TNC_ERROR_H_ @}*/
