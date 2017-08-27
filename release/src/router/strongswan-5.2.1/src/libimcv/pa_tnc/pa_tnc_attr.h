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
 * @defgroup pa_tnc_attr pa_tnc_attr
 * @{ @ingroup pa_tnc
 */

#ifndef PA_TNC_ATTR_H_
#define PA_TNC_ATTR_H_

typedef struct pa_tnc_attr_t pa_tnc_attr_t;

#include <library.h>
#include <pen/pen.h>

#define PA_TNC_ATTR_INFO_SIZE		 8
#define PA_TNC_ATTR_HEADER_SIZE		12

#define PA_TNC_ATTR_FLAG_NONE		0x00
#define PA_TNC_ATTR_FLAG_NOSKIP		(1<<7)

/**
 * Interface for an RFC 5792 PA-TNC Posture Attribute.
 *
 */
struct pa_tnc_attr_t {

	/**
	 * Get the vendor ID/type of an PA-TNC attribute
	 *
	 * @return					vendor-specific attribute type
	 */
	pen_type_t (*get_type)(pa_tnc_attr_t *this);

	/**
	 * Get the value of an PA-TNC attribute
	 *
	 * @return					attribute value
	 */
	chunk_t (*get_value)(pa_tnc_attr_t *this);

	/**
	 * Get the noskip flag
	 *
	 * @return					TRUE if the noskip flag is set
	 */
	bool (*get_noskip_flag)(pa_tnc_attr_t *this);

	/**
	 * Set the noskip flag
	 *
	 * @param noskip_flag		TRUE if the noskip flag is to be set
	 */
	void (*set_noskip_flag)(pa_tnc_attr_t *this, bool noskip);

	/**
	 * Build value of an PA-TNC attribute from its parameters
	 */
	void (*build)(pa_tnc_attr_t *this);

	/**
	 * Process the value of an PA-TNC attribute to extract its parameters
	 *
	 * @param offset			relative error offset within attribute body
	 * @return					result status
	 */
	status_t (*process)(pa_tnc_attr_t *this, uint32_t *offset);

	/**
	 * Add a data segment to an attribute allowing incremental processing
	 *
	 * @param segment			data segment to be appended
	 */
	void (*add_segment)(pa_tnc_attr_t *this, chunk_t segment);

	/**
	 * Get a new reference to the PA-TNC attribute
	 *
	 * @return			this, with an increased refcount
	 */
	pa_tnc_attr_t* (*get_ref)(pa_tnc_attr_t *this);

	/**
	 * Destroys a pa_tnc_attr_t object.
	 */
	void (*destroy)(pa_tnc_attr_t *this);
};

#endif /** PA_TNC_ATTR_H_ @}*/
