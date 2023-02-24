/*
 * Copyright (C) 2014-2022 Andreas Steffen
 *
 * Copyright (C) secunet Security Networks AG
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
 * @defgroup tcg_seg_attr_seg_env tcg_seg_attr_seg_env
 * @{ @ingroup tcg_attr
 */

#ifndef TCG_SEG_ATTR_SEG_ENV_H_
#define TCG_SEG_ATTR_SEG_ENV_H_

typedef struct tcg_seg_attr_seg_env_t tcg_seg_attr_seg_env_t;

#include "tcg/tcg_attr.h"

#define TCG_SEG_ATTR_SEG_ENV_HEADER		4

/**
 * Class implementing the TCG Segment Envelope Attribute
 */
struct tcg_seg_attr_seg_env_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Get enveloped attribute segment
	 *
	 * @param flags			Segmentation flags
	 * @return				Segment
	 */
	chunk_t (*get_segment)(tcg_seg_attr_seg_env_t *this, uint8_t *flags);

	/**
	 * Get Base Message ID
	 *
	 * @return				Base Message ID
	 */
	uint32_t (*get_base_msg_id)(tcg_seg_attr_seg_env_t *this);

};

/**
 * Creates an tcg_seg_attr_seg_env_t object
 *
 * @param segment			Segment
 * @param flags				Segmentation flags
 * @param base_msg_id		Base Message ID
 */
pa_tnc_attr_t* tcg_seg_attr_seg_env_create(chunk_t segment, uint8_t flags,
								           uint32_t base_msg_id);

/**
 * Creates an tcg_seg_attr_seg_env_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* tcg_seg_attr_seg_env_create_from_data(size_t length,
                                                     chunk_t value);

#endif /** TCG_SEG_ATTR_SEG_ENV_H_ @}*/
