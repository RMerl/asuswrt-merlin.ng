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
 * @defgroup tnccs_20_client_h tnccs_20_client
 * @{ @ingroup tnccs_20
 */

#ifndef TNCCS_20_CLIENT_H_
#define TNCCS_20_CLIENT_H_

#include <library.h>

#include <tnc/tnccs/tnccs.h>

#include "tnccs_20_handler.h"

typedef struct tnccs_20_client_t tnccs_20_client_t;

/**
 * Interface for a TNC client
 */
struct tnccs_20_client_t {

	/**
	 * IF-TNCCS 2.0 protocol handler interface
	 */
	tnccs_20_handler_t handler;

	/**
	 * Get PDP server information if available
	 *
	 * @param port			PT-TLS port of the PDP server
	 * @return				FQDN of PDP server
	 */
	chunk_t (*get_pdp_server)(tnccs_20_client_t *this, uint16_t *port);

};

/**
 * Create an instance of the TNC IF-TNCCS 2.0 client-side protocol handler.
 *
 * @param tnccs				TNC IF-TNCCS 2.0 stack
 * @param send_msg			TNF IF-TNCCS 2.0 send message callback function
 * @param max_batch_len		Maximum PB-TNC batch size
 * @param max_msg_len		Maximum PA-TNC message size
 */
tnccs_20_handler_t* tnccs_20_client_create(tnccs_t *tnccs,
										   tnccs_send_message_t send_msg,
										   size_t max_batch_len,
										   size_t max_msg_len);

#endif /** TNCCS_20_CLIENT_H_ @}*/
