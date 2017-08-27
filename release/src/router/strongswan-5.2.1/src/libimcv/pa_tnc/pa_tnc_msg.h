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
 * @defgroup pa_tnc_msg pa_tnc_msg
 * @{ @ingroup pa_tnc
 */

#ifndef PA_TNC_MSG_H_
#define PA_TNC_MSG_H_

typedef struct pa_tnc_msg_t pa_tnc_msg_t;

#define PA_TNC_VERSION		0x01
#define PA_TNC_HEADER_SIZE	8

#include "pa_tnc_attr.h"

#include <library.h>

/**
 * Interface for the RFC 5792 PA-TNC Posture Attribute protocol.
 *
 */
struct pa_tnc_msg_t {

	/**
	 * Get the encoding of the PA-TNC message
	 *
	 * @return					encoded PA-TNC message
	 */
	chunk_t (*get_encoding)(pa_tnc_msg_t *this);

	/**
	 * Add a PA-TNC attribute
	 *
	 * @param attr				PA-TNC attribute to be addedd
	 * @return					TRUE if attribute fit into message and was added
	 */
	bool (*add_attribute)(pa_tnc_msg_t *this, pa_tnc_attr_t* attr);

	/**
	 * Build the PA-TNC message
	 *
	 * @return					TRUE if PA-TNC message was built successfully
	 */
	bool (*build)(pa_tnc_msg_t *this);

	/**
	 * Process the PA-TNC message
	 *
	 * @return					return processing status
	 */
	status_t (*process)(pa_tnc_msg_t *this);

	/**
	 * Process all IETF standard error PA-TNC attributes
	 *
	 * @param non_fatal_types	list of non fatal unsupported attribute types
	 * @return					TRUE if at least one fatal error processed
	 */
	bool (*process_ietf_std_errors)(pa_tnc_msg_t *this,
								    linked_list_t *non_fatal_types);

	/**
	 * Enumerates over all PA-TNC attributes
	 *
	 * @return				return attribute enumerator
	 */
	enumerator_t* (*create_attribute_enumerator)(pa_tnc_msg_t *this);

	/**
	 * Enumerates over all parsing errors
	 *
	 * @return				return error enumerator
	 */
	enumerator_t* (*create_error_enumerator)(pa_tnc_msg_t *this);

	/**
	 * Destroys a pa_tnc_msg_t object.
	 */
	void (*destroy)(pa_tnc_msg_t *this);
};

/**
 * Create an empty PA-TNC message
 */
pa_tnc_msg_t* pa_tnc_msg_create(size_t max_msg_len);

/**
 * Create an unprocessed PA-TNC message from received data
 *
 * @param data					PA-TNC message data
 */
pa_tnc_msg_t* pa_tnc_msg_create_from_data(chunk_t data);

#endif /** PA_TNC_MSG_H_ @}*/
