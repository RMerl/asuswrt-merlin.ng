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
 * @defgroup ietf_attr_attr_requestt ietf_attr_attr_request
 * @{ @ingroup ietf_attr
 */

#ifndef IETF_ATTR_ATTR_REQUEST_H_
#define IETF_ATTR_ATTR_REQUEST_H_

typedef struct ietf_attr_attr_request_t ietf_attr_attr_request_t;

#include "ietf_attr.h"
#include "pa_tnc/pa_tnc_attr.h"


/**
 * Class implementing the IETF PA-TNC Attribute Request attribute.
 *
 */
struct ietf_attr_attr_request_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Adds another attribute type to the attribute request
	 *
	 * @param vendor_id		Attribute Vendor ID
	 * @param type			Attribute Type
	 */
	void (*add)(ietf_attr_attr_request_t *this, pen_t vendor_id, uint32_t type);

	/**
	 * Creates an enumerator over all attribute types contained
	 * in the attribute request
	 *
	 * @return			Attribute Type enumerator returns { vendor ID, type }
	 */
	enumerator_t* (*create_enumerator)(ietf_attr_attr_request_t *this);
};

/**
 * Creates an ietf_attr_attr_request_t object
 *
 */
pa_tnc_attr_t* ietf_attr_attr_request_create(pen_t vendor_id, uint32_t type);

/**
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* ietf_attr_attr_request_create_from_data(size_t length,
													   chunk_t value);

#endif /** IETF_ATTR_ATTR_REQUEST_H_ @}*/
