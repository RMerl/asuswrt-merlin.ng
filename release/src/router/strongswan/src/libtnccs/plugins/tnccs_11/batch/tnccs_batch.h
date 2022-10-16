/*
 * Copyright (C) 2010 Andreas Steffen
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
 * @defgroup tnccs_batch tnccs_batch
 * @{ @ingroup tnccs_11
 */

#ifndef TNCCS_BATCH_H_
#define TNCCS_BATCH_H_

typedef enum tnccs_batch_type_t tnccs_batch_type_t;
typedef struct tnccs_batch_t tnccs_batch_t;

#include "messages/tnccs_msg.h"

#include <library.h>

/**
 * Interface for a TNCCS 1.x Batch.
 */
struct tnccs_batch_t {

	/**
	 * Get the encoding of the TNCCS 1.x Batch
	 *
	 * @return				encoded TNCCS 1.x batch
	 */
	chunk_t (*get_encoding)(tnccs_batch_t *this);

	/**
	 * Add TNCCS message
	 *
	 * @param msg			TNCCS message to be added
	 */
	void (*add_msg)(tnccs_batch_t *this, tnccs_msg_t* msg);

	/**
	 * Build the TNCCS 1.x Batch
	 */
	void (*build)(tnccs_batch_t *this);

	/**
	 * Process the TNCCS 1.x Batch
	 *
	 * @return				return processing status
	 */
	status_t (*process)(tnccs_batch_t *this);

	/**
	 * Enumerates over all TNCCS Messages
	 *
	 * @return				return message enumerator
	 */
	enumerator_t* (*create_msg_enumerator)(tnccs_batch_t *this);

	/**
	 * Enumerates over all parsing errors
	 *
	 * @return				return error enumerator
	 */
	enumerator_t* (*create_error_enumerator)(tnccs_batch_t *this);

	/**
	 * Destroys a tnccs_batch_t object.
	 */
	void (*destroy)(tnccs_batch_t *this);
};

/**
 * Create an empty TNCCS 1.x Batch
 *
 * @param is_server			TRUE if server, FALSE if client
 * @param batch_id			number of the batch to be sent
 */
tnccs_batch_t* tnccs_batch_create(bool is_server, int batch_id);

/**
 * Create an unprocessed TNCCS 1.x Batch from data
 *
 * @param  is_server		TRUE if server, FALSE if client
 * @param batch_id			current Batch ID
 * @param  data				encoded PB-TNC batch
 */
tnccs_batch_t* tnccs_batch_create_from_data(bool is_server, int batch_id,
											chunk_t data);

#endif /** TNCCS_BATCH_H_ @}*/
