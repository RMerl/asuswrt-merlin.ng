/*
 * Copyright (C) 2017 Andreas Steffen
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
 * @defgroup swima_error swima_error
 * @{ @ingroup libimcv_swima
 */

#ifndef SWIMA_ERROR_H_
#define SWIMA_ERROR_H_

typedef enum swima_error_code_t swima_error_code_t;

#include "pa_tnc/pa_tnc_attr.h"
#include "ietf/ietf_attr_pa_tnc_error.h"

#include <library.h>

/**
 * Creates a SWIMA Error Attribute
 * see section 5.16 of IETF SW Inventory Message and Attributes for PA-TNC
 *
 * @param code				PA-TNC error code
 * @param request			SWID request ID
 * @param max_attr_size		Maximum PA-TNC attribute size (if applicable)
 * @param description		Optional description string or NULL
 */
pa_tnc_attr_t* swima_error_create(pa_tnc_error_code_t code, uint32_t request,
								  uint32_t max_attr_size, char *description);

#endif /** SWIMA_ERROR_H_ @}*/
