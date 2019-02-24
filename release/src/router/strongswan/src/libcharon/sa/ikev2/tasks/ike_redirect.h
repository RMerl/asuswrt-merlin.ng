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
 * @defgroup ike_redirect ike_redirect
 * @{ @ingroup tasks_v2
 */

#ifndef IKE_REDIRECT_H_
#define IKE_REDIRECT_H_

typedef struct ike_redirect_t ike_redirect_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/task.h>

/**
 * Task that handles redirection requests for established SAs.
 */
struct ike_redirect_t {

	/**
	 * Implements the task_t interface
	 */
	task_t task;
};

/**
 * Create a new ike_redirect_t task.
 *
 * As initiator (i.e. original responder) pass the ID of the target gateway,
 * as responder (i.e. original initiator) this argument is NULL.
 *
 * @param ike_sa		IKE_SA this task works for
 * @param to			gateway ID (gets cloned), or NULL as responder
 * @return				task instance
 */
ike_redirect_t *ike_redirect_create(ike_sa_t *ike_sa,
									identification_t *to);

#endif /** IKE_REDIRECT_H_ @}*/
