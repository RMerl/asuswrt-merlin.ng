/*
 * Copyright (C) 2015 Tobias Brunner
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
 * @defgroup ike_verify_peer_cert ike_verify_peer_cert
 * @{ @ingroup tasks_v2
 */

#ifndef IKE_VERIFY_PEER_CERT_H_
#define IKE_VERIFY_PEER_CERT_H_

typedef struct ike_verify_peer_cert_t ike_verify_peer_cert_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/task.h>

/**
 * Task of type ike_verify_peer_cert, verifies a peer's certificate.
 *
 * This task (re-)verifies the peer's certificate explicitly including online
 * OCSP and CRL checks.
 */
struct ike_verify_peer_cert_t {

	/**
	 * Implements the task_t interface
	 */
	task_t task;
};

/**
 * Create a new ike_verify_peer_cert task.
 *
 * This task is initiator only.
 *
 * @param ike_sa		IKE_SA this task works for
 * @return				ike_verify_peer_cert task to handle by the task_manager
 */
ike_verify_peer_cert_t *ike_verify_peer_cert_create(ike_sa_t *ike_sa);

#endif /** IKE_VERIFY_PEER_CERT_H_ @}*/
