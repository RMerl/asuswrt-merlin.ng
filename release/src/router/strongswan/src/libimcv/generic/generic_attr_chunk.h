/*
 * Copyright (C) 2015 Andreas Steffen
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
 * @defgroup generic_attr_chunk generic_attr_chunk
 * @{ @ingroup generic_attr
 */

#ifndef GENERIC_ATTR_CHUNK_H_
#define GENERIC_ATTR_CHUNK_H_

typedef struct generic_attr_chunk_t generic_attr_chunk_t;

#include <pen/pen.h>
#include "pa_tnc/pa_tnc_attr.h"

/**
 * Class implementing a generic PA-TNC attribute containing a possibly
 * binary string with either a fixed or variable size
 */
struct generic_attr_chunk_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;
};

/**
 * Creates a generic_attr_chunk_t object
 *
 * @param string			Non-nul terminated string
 * @param type				Vendor ID / Attribute Type
 */
pa_tnc_attr_t* generic_attr_chunk_create(chunk_t string, pen_type_t type);

/**
 * Creates an generic_attr_chunk_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 * @param size				size in bytes if fixed array or 0 if dynamic size
 * @param type				Vendor ID / Attribute Type
 */
pa_tnc_attr_t* generic_attr_chunk_create_from_data(size_t length, chunk_t value,
												   size_t size, pen_type_t type);

#endif /** GENERIC_ATTR_CHUNK_H_ @}*/
