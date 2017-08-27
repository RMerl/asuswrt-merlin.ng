/*
 * Copyright (C) 2013 Andreas Steffen
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
 * @defgroup swid_error swid_error
 * @{ @ingroup libimcv_swid
 */

#ifndef SWID_ERROR_H_
#define SWID_ERROR_H_

typedef enum swid_error_code_t swid_error_code_t;

#include "pa_tnc/pa_tnc_attr.h"

#include <library.h>


/**
 * SWID Error Codes
 * see section 3.14.2 of PTS Protocol: Binding to TNC IF-M Specification
 */
enum swid_error_code_t {
	TCG_SWID_ERROR = 		        0x20,
	TCG_SWID_SUBSCRIPTION_DENIED =  0x21,
	TCG_SWID_RESPONSE_TOO_LARGE =   0x22
};

/**
 * enum name for swid_error_code_t.
 */
extern enum_name_t *swid_error_code_names;

/**
 * Creates a SWID Error Attribute
 * see section 4.12 of TNC SWID Message and Attributes for IF-M
 *
 * @param code				SWID error code
 * @param request			SWID request ID
 * @param max_attr_size		Maximum IF-M attribute size (if applicable)
 * @param description		Optional description string or NULL
 */
pa_tnc_attr_t* swid_error_create(swid_error_code_t code, u_int32_t request,
								 u_int32_t max_attr_size, char *description);

#endif /** SWID_ERROR_H_ @}*/
