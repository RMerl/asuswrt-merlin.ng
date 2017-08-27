/*
 * Copyright (C) 2007 Martin Willi
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup ike_auth ike_auth
 * @{ @ingroup tasks_v2
 */

#ifndef IKE_AUTH_H_
#define IKE_AUTH_H_

typedef struct ike_auth_t ike_auth_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/task.h>

/**
 * Task of type ike_auth, authenticates an IKE_SA using authenticators.
 *
 * The ike_auth task authenticates the IKE_SA using the IKE_AUTH
 * exchange. It processes and build IDi and IDr payloads and also
 * handles AUTH payloads. The AUTH payloads are passed to authenticator_t's,
 * which do the actual authentication process. If the ike_auth task is used
 * with EAP authentication, it stays alive over multiple exchanges until
 * EAP has completed.
 */
struct ike_auth_t {

	/**
	 * Implements the task_t interface
	 */
	task_t task;
};

/**
 * Create a new task of type TASK_IKE_AUTH.
 *
 * @param ike_sa		IKE_SA this task works for
 * @param initiator		TRUE if task is the initiator of an exchange
 * @return				ike_auth task to handle by the task_manager
 */
ike_auth_t *ike_auth_create(ike_sa_t *ike_sa, bool initiator);

#endif /** IKE_AUTH_H_ @}*/
