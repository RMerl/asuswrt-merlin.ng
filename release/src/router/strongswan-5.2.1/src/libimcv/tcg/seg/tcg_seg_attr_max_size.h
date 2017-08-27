/*
 * Copyright (C) 2014 Andreas Steffen
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
 * @defgroup tcg_seg_attr_max_size tcg_seg_attr_max_size
 * @{ @ingroup tcg_attr
 */

#ifndef TCG_SEG_ATTR_MAX_SIZE_H_
#define TCG_SEG_ATTR_MAX_SIZE_H_

typedef struct tcg_seg_attr_max_size_t tcg_seg_attr_max_size_t;

#include "tcg/tcg_attr.h"

#define TCG_SEG_ATTR_MAX_SIZE_SIZE		8

/**
 * Class implementing the TCG Segmentation Maximum Attribute Size Attribute
 */
struct tcg_seg_attr_max_size_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Get maximum IF-M attribute and segment size in octets
	 *
	 * @param max_attr_size		Maximum IF-M attribute size in octets
	 * @param max_seg_size		Maximum IF-M attribute segment size in octets
 	 */
	void (*get_attr_size)(tcg_seg_attr_max_size_t *this,
						  uint32_t *max_attr_size, uint32_t *max_seg_size);

};

/**
 * Creates an tcg_seg_attr_max_size_t object
 *
 * @param max_attr_size		Maximum IF-M attribute size in octets
 * @param max_seg_size		Maximum IF-M attribute segment size in octets
 * @param request			TRUE for a request, FALSE for a response
 */
pa_tnc_attr_t* tcg_seg_attr_max_size_create(uint32_t max_attr_size,
											uint32_t max_seg_size,
											bool request);

/**
 * Creates an tcg_seg_attr_max_size_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 * @param request			TRUE for a request, FALSE for a response
 */
pa_tnc_attr_t* tcg_seg_attr_max_size_create_from_data(size_t length,
													  chunk_t value,
													  bool request);

#endif /** TCG_SEG_ATTR_MAX_SIZE_H_ @}*/
