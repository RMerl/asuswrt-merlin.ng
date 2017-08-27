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
 * @defgroup ietf_attr_product_infot ietf_attr_product_info
 * @{ @ingroup ietf_attr
 */

#ifndef IETF_ATTR_PRODUCT_INFO_H_
#define IETF_ATTR_PRODUCT_INFO_H_

typedef struct ietf_attr_product_info_t ietf_attr_product_info_t;

#include "ietf_attr.h"
#include "pa_tnc/pa_tnc_attr.h"


/**
 * Class implementing the IETF PA-TNC Product Information attribute.
 *
 */
struct ietf_attr_product_info_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Gets all product info
	 *
	 * @param vendor_id		Product vendor ID
	 * @param id			Product ID
	 * @return				Product Name
	 */
	chunk_t (*get_info)(ietf_attr_product_info_t *this,
						pen_t *vendor_id, u_int16_t *id);

};

/**
 * Creates an ietf_attr_product_info_t object
 *
 */
pa_tnc_attr_t* ietf_attr_product_info_create(pen_t vendor_id, u_int16_t id,
											 chunk_t name);

/**
 * Creates an ietf_attr_product_info_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* ietf_attr_product_info_create_from_data(size_t length,
													   chunk_t value);

#endif /** IETF_ATTR_PRODUCT_INFO_H_ @}*/
