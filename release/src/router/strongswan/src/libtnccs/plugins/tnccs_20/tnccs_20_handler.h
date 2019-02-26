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
 * @defgroup tnccs_20_handler_h tnccs_20_handler
 * @{ @ingroup tnccs_20
 */

#ifndef TNCCS_20_HANDLER_H_
#define TNCCS_20_HANDLER_H_

#include <library.h>

#include "batch/pb_tnc_batch.h"
#include "messages/pb_tnc_msg.h"

typedef struct tnccs_20_handler_t tnccs_20_handler_t;

/**
 * Interface for an IF-TNCCS 2.0 protocol handler
 */
struct tnccs_20_handler_t {

	/**
	 * Process content of received PB-TNC batch
	 *
	 * @param batch			PB-TNC batch to be processed
	 * @return				status
	 */
	status_t (*process)(tnccs_20_handler_t *this, pb_tnc_batch_t *batch);

	/**
	 * Build PB-TNC batch to be sent
	 *
	 * @param buf			buffer to write PB-TNC batch to
	 * @param buflen		size of buffer, receives bytes written
	 * @param msglen		receives size of all PB-TNCH batch
	 * @return				status
	 */
	status_t (*build)(tnccs_20_handler_t *this, void *buf, size_t *buflen,
														  size_t *msglen);

	/**
	 * Put the IMCs or IMVs into the handshake state
	 *
	 * @param mutual		TRUE if PB-TNC mutual mode is already established
	 */
	void (*begin_handshake)(tnccs_20_handler_t *this, bool mutual);

	/**
	 * Indicates if IMCs or IMVs are allowed to send PA-TNC messages
	 *
	 * @return				TRUE if allowed to send
	 */
	bool (*get_send_flag)(tnccs_20_handler_t *this);

	/**
	 * Indicates if the PB-TNC mutual protocol has been enabled
	 *
	 * @return				TRUE if enabled
	 */
	bool (*get_mutual)(tnccs_20_handler_t *this);

	/**
	 * Get state of the PB-TNC protocol
	 *
	 * @return				PB-TNC state
	 */
	pb_tnc_state_t (*get_state)(tnccs_20_handler_t *this);

	/**
	 * Add a PB-PA message to the handler's message queue
	 *
	 * @param msg			PB-PA message to be added
	 */
	void (*add_msg)(tnccs_20_handler_t *this, pb_tnc_msg_t *msg);

	/**
	 * Handle errors that occurred during PB-TNC batch header processing
	 *
	 * @param batch					batch where a fatal error occurred
	 * @param fatal_header_error	TRUE if fatal error in batch header
	 */
	void (*handle_errors)(tnccs_20_handler_t *this, pb_tnc_batch_t *batch,
						  bool fatal_header_error);

	/**
	 * Destroys a tnccs_20_handler_t object.
	 */
	void (*destroy)(tnccs_20_handler_t *this);
};

#endif /** TNCCS_20_HANDLER_H_ @}*/
