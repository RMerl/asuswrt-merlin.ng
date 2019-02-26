/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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
 * @defgroup pt_tls_client pt_tls_client
 * @{ @ingroup pt_tls
 */

#ifndef PT_TLS_CLIENT_H_
#define PT_TLS_CLIENT_H_

#include <networking/host.h>
#include <utils/identification.h>

#include <tnc/tnccs/tnccs.h>

typedef struct pt_tls_client_t pt_tls_client_t;

/**
 * IF-T for TLS aka PT-TLS transport client.
 */
struct pt_tls_client_t {

	/**
	 * Perform an assessment.
	 *
	 * @param tnccs		upper layer TNC client used for assessment
	 * @return			status of assessment
	 */
	status_t (*run_assessment)(pt_tls_client_t *this, tnccs_t *tnccs);

	/**
	 * Destroy a pt_tls_client_t.
	 */
	void (*destroy)(pt_tls_client_t *this);
};

/**
 * Create a pt_tls_client instance.
 *
 * The client identity is used for:
 * - TLS authentication if an appropirate certificate is found
 * - SASL authentication if requested from the server
 *
 * @param address		address/port to run assessments against, gets owned
 * @param server		server identity to use for authentication, gets owned
 * @param client		client identity to use for authentication, gets owned
 * @return				PT-TLS context
 */
pt_tls_client_t *pt_tls_client_create(host_t *address, identification_t *server,
									  identification_t *client);

#endif /** PT_TLS_CLIENT_H_ @}*/
