/*
 * Copyright (C) 2013-2014 Andreas Steffen
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
 * @defgroup ita_attr_device_id ita_attr_device_id
 * @{ @ingroup ita_attr
 */

#ifndef ITA_ATTR_DEVICE_ID_H_
#define ITA_ATTR_DEVICE_ID_H_

typedef struct ita_attr_device_id_t ita_attr_device_id_t;

#include "pa_tnc/pa_tnc_attr.h"

/**
 * Class implementing the ITA Device ID PA-TNC attribute.
 *
 */
struct ita_attr_device_id_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

};

/**
 * Creates an ita_attr_device_id_t object
 *
 * @param value				ITA Device ID attribute value
 */
pa_tnc_attr_t* ita_attr_device_id_create(chunk_t value);

/**
 * Creates an ita_attr_device_id_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* ita_attr_device_id_create_from_data(size_t length, chunk_t value);

#endif /** ITA_ATTR_DEVICE_ID_H_ @}*/
