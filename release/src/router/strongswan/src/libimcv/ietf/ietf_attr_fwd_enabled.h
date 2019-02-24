/*
 * Copyright (C) 2012-2015 Andreas Steffen
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
 * @defgroup ietf_attr_fwd_enabled ietf_attr_fwd_enabled
 * @{ @ingroup ietf_attr
 */

#ifndef IETF_ATTR_FWD_ENABLED_H_
#define IETF_ATTR_FWD_ENABLED_H_

typedef struct ietf_attr_fwd_enabled_t ietf_attr_fwd_enabled_t;

#include "ietf_attr.h"
#include "pa_tnc/pa_tnc_attr.h"
#include "os_info/os_info.h"

/**
 * Class implementing the IETF PA-TNC Forwarding Enabled attribute.
 *
 */
struct ietf_attr_fwd_enabled_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Gets the Forwarding Enabled status
	 *
	 * @return				Forwarding Enabled status
	 */
	os_fwd_status_t (*get_status)(ietf_attr_fwd_enabled_t *this);

};

/**
 * Creates an ietf_attr_fwd_enabled_t object
 *
 * @param fwd_status		Forwarding Enabled status
 * @param type				Vendor ID / Attribute Type
 */
pa_tnc_attr_t* ietf_attr_fwd_enabled_create(os_fwd_status_t fwd_status,
											pen_type_t type);

/**
 * Creates an ietf_attr_fwd_enabled_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 * @param type				Vendor ID / Attribute Type
 */
pa_tnc_attr_t* ietf_attr_fwd_enabled_create_from_data(size_t length,
										chunk_t value, pen_type_t type);

#endif /** IETF_ATTR_FWD_ENABLED_H_ @}*/
