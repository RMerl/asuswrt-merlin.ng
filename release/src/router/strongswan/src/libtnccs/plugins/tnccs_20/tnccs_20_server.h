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
 * @defgroup tnccs_20_server_h tnccs_20_server
 * @{ @ingroup tnccs_20
 */

#ifndef TNCCS_20_SERVER_H_
#define TNCCS_20_SERVER_H_

#include <library.h>

#include <tnc/tnccs/tnccs.h>

#include "tnccs_20_handler.h"

typedef struct tnccs_20_server_t tnccs_20_server_t;

/**
 * Interface for a TNC server
 */
struct tnccs_20_server_t {

	/**
	 * IF-TNCCS 2.0 protocol handler interface
	 */
	tnccs_20_handler_t handler;

	/**
	 * Check if an Action Recommendation is already available
	 *
	 * @param rec			TNC Action Recommendation
	 * @param eval			TNC Evaluation Result
	 * @return				TRUE if Action Recommendation is
	 */
	bool (*have_recommendation)(tnccs_20_server_t *this,
								TNC_IMV_Action_Recommendation *rec,
								TNC_IMV_Evaluation_Result *eval);

};

/**
 * Create an instance of the TNC IF-TNCCS 2.0 server-side protocol handler.
 *
 * @param tnccs				TNC IF-TNCCS 2.0 stack
 * @param send_msg			TNF IF-TNCCS 2.0 send message callback function
 * @param max_batch_len		Maximum PB-TNC batch size
 * @param max_msg_len		Maximum PA-TNC message size
 * @param eap_transport		TRUE if IF-T for EAP methods
 */
tnccs_20_handler_t* tnccs_20_server_create(tnccs_t *tnccs,
										   tnccs_send_message_t send_msg,
										   size_t max_batch_len,
										   size_t max_msg_len,
										   bool eap_transport);


#endif /** TNCCS_20_SERVER_H_ @}*/
