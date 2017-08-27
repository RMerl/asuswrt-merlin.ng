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
 * @defgroup seg_env seg_env
 * @{ @ingroup libimcv_seg
 */

#ifndef SEG_ENV_H_
#define SEG_ENV_H_

typedef struct seg_env_t seg_env_t;
typedef enum seg_env_flags_t seg_env_flags_t;

#include <library.h>

#include <pa_tnc/pa_tnc_attr.h>

/**
 * Segment Envelope flags
 */
enum seg_env_flags_t {
	SEG_ENV_FLAG_NONE =	  0,
	SEG_ENV_FLAG_MORE =	 (1<<7),
	SEG_ENV_FLAG_START = (1<<6)
};

/**
 * Interface for a PA-TNC attribute segment envelope object
 */
struct seg_env_t {

	/**
	 * Get Base Attribute ID
	 *
	 * @return				Base Attribute ID
	 */
	uint32_t (*get_base_attr_id)(seg_env_t *this);

	/**
	 * Get Base Attribute if it contains processed [incremental] data
	 *
	 * @return				Base Attribute (must be destroyed) or NULL
	 */
	pa_tnc_attr_t* (*get_base_attr)(seg_env_t *this);

	/**
	 * Base Attribute Info to be used by PA-TNC error messages
	 *
	 * @return				Message info string
	 */
	chunk_t (*get_base_attr_info)(seg_env_t *this);

	/**
	 * Generate the first segment envelope of the base attribute
	 *
	 * @return				First attribute segment envelope
	 */
	pa_tnc_attr_t* (*first_segment)(seg_env_t *this);

	/**
	 * Generate the next segment envelope of the base attribute
	 *
	 * @param last			TRUE if last segment
	 * @return				Next attribute segment envelope
	 */
	pa_tnc_attr_t* (*next_segment)(seg_env_t *this, bool *last);

	/**
	 * Generate the first segment envelope of the base attribute
	 *
	 * @param segment		Attribute segment to be added
	 * @param error			Error attribute if a parsing error occurred
	 * return				TRUE if segment was successfully added
	 */
	bool (*add_segment)(seg_env_t *this, chunk_t segment,
						pa_tnc_attr_t** error);

	/**
	 * Destroys a seg_env_t object.
	 */
	void (*destroy)(seg_env_t *this);
};

/**
 * Create a PA-TNC attribute segment envelope object
 *
 * @param base_attr_id		Base Attribute ID
 * @param base_attr			Base Attribute to be segmented
 * @param max_seg_size		Maximum segment size
 */
seg_env_t* seg_env_create(uint32_t base_attr_id, pa_tnc_attr_t *base_attr,
						  uint32_t max_seg_size);

/**
 * Create a PA-TNC attribute segment envelope object
 *
 * @param base_attr_id		Base Attribute ID
 * @param data				First attribute segment
 * @param max_seg_size		Maximum segment size
 * @param error				Error attribute if a parsing error occurred
 */
seg_env_t* seg_env_create_from_data(uint32_t base_attr_id, chunk_t data,
									uint32_t max_seg_size,
									pa_tnc_attr_t** error);

#endif /** SEG_ENV_H_ @}*/
