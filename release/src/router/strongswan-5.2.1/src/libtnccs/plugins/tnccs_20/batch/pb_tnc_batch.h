/*
 * Copyright (C) 2010-2012 Andreas Steffen
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
 * @defgroup pb_tnc_batch pb_tnc_batch
 * @{ @ingroup tnccs_20
 */

#ifndef PB_TNC_BATCH_H_
#define PB_TNC_BATCH_H_

typedef enum pb_tnc_batch_type_t pb_tnc_batch_type_t;
typedef struct pb_tnc_batch_t pb_tnc_batch_t;

#include "messages/pb_tnc_msg.h"
#include "state_machine/pb_tnc_state_machine.h"

#include <library.h>

#define PB_TNC_BATCH_HEADER_SIZE	 8
#define PB_TNC_MSG_HEADER_SIZE		12

/**
  * PB-TNC Batch Types as defined in section 4.1 of RFC 5793
 */
enum pb_tnc_batch_type_t {
	PB_BATCH_NONE =		0,	/* for internal use only */
	PB_BATCH_CDATA =	1,
	PB_BATCH_SDATA =	2,
	PB_BATCH_RESULT =	3,
	PB_BATCH_CRETRY =	4,
	PB_BATCH_SRETRY =	5,
	PB_BATCH_CLOSE =	6,
	PB_BATCH_ROOF =		6
};

/**
 * enum name for pb_tnc_batch_type_t.
 */
extern enum_name_t *pb_tnc_batch_type_names;

/**
 * Interface for all PB-TNC Batch Types.
 */
struct pb_tnc_batch_t {

	/**
	 * Get the PB-TNC Message Type
	 *
	 * @return				PB-TNC batch type
	 */
	pb_tnc_batch_type_t (*get_type)(pb_tnc_batch_t *this);

	/**
	 * Get the encoding of the PB-TNC Batch
	 *
	 * @return				encoded PB-TNC batch
	 */
	chunk_t (*get_encoding)(pb_tnc_batch_t *this);

	/**
	 * Add a PB-TNC Message
	 *
	 * @param msg			PB-TNC message to be addedd
	 * @return				TRUE if message fit into batch and was added
	 */
	bool (*add_msg)(pb_tnc_batch_t *this, pb_tnc_msg_t* msg);

	/**
	 * Build the PB-TNC Batch
	 */
	void (*build)(pb_tnc_batch_t *this);

	/**
	 * Process the PB-TNC Batch
	 *
	 * @param				PB-TNC state machine
	 * @return				return processing status
	 */
	status_t (*process)(pb_tnc_batch_t *this,
						pb_tnc_state_machine_t *state_machine);

	/**
	 * Enumerates over all PB-TNC Messages
	 *
	 * @return				return message enumerator
	 */
	enumerator_t* (*create_msg_enumerator)(pb_tnc_batch_t *this);

	/**
	 * Enumerates over all parsing errors
	 *
	 * @return				return error enumerator
	 */
	enumerator_t* (*create_error_enumerator)(pb_tnc_batch_t *this);

	/**
	 * Destroys a pb_tnc_batch_t object.
	 */
	void (*destroy)(pb_tnc_batch_t *this);
};

/**
 * Create an empty PB-TNC Batch of a given type
 *
 * @param is_server			TRUE if server, FALSE if client
 * @param type				PB-TNC batch type
 * @param max_batch_len		maximum size the PB-TNC batch
 */
pb_tnc_batch_t* pb_tnc_batch_create(bool is_server, pb_tnc_batch_type_t type,
									size_t max_batch_len);

/**
 * Create an unprocessed PB-TNC Batch from data
 *
 * @param is_server			TRUE if server, FALSE if client
 * @param  data				encoded PB-TNC batch
 */
pb_tnc_batch_t* pb_tnc_batch_create_from_data(bool is_server, chunk_t data);

#endif /** PB_TNC_BATCH_H_ @}*/
