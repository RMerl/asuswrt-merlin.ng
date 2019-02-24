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
 * @defgroup tcg_seg_attr_next_seg tcg_seg_attr_next_seg
 * @{ @ingroup tcg_attr
 */

#ifndef TCG_SEG_ATTR_NEXT_SEG_H_
#define TCG_SEG_ATTR_NEXT_SEG_H_

typedef struct tcg_seg_attr_next_seg_t tcg_seg_attr_next_seg_t;

#include "tcg/tcg_attr.h"

#define TCG_SEG_ATTR_NEXT_SEG_SIZE		4

/**
 * Class implementing the TCG Segmentation Next Segment Attribute
 */
struct tcg_seg_attr_next_seg_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Get Base Attribute ID
	 *
	 * @return				Base Attribute ID
	 */
	uint32_t (*get_base_attr_id)(tcg_seg_attr_next_seg_t *this);

	/**
	 * Get the Cancel flag
	 *
	 * @return				Cancel flag
	 */
	bool (*get_cancel_flag)(tcg_seg_attr_next_seg_t *this);

};

/**
 * Creates an tcg_seg_attr_next_seg_t object
 *
 * @param base_attr_id		Base Attribute ID
 * @param cancel			If TRUE set Cancel flag
 */
pa_tnc_attr_t* tcg_seg_attr_next_seg_create(uint32_t base_attr_id, bool cancel);

/**
 * Creates an tcg_seg_attr_next_seg_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* tcg_seg_attr_next_seg_create_from_data(size_t length,
													  chunk_t value);

#endif /** TCG_SEG_ATTR_NEXT_SEG_H_ @}*/
