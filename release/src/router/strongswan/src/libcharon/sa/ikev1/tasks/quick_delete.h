/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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
 * @defgroup quick_delete quick_delete
 * @{ @ingroup tasks_v1
 */

#ifndef QUICK_DELETE_H_
#define QUICK_DELETE_H_

typedef struct quick_delete_t quick_delete_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/task.h>
#include <sa/child_sa.h>

/**
 * Task of type QUICK_DELETE, delete an IKEv1 quick mode SA.
 */
struct quick_delete_t {

	/**
	 * Implements the task_t interface
	 */
	task_t task;
};

/**
 * Create a new quick_delete task.
 *
 * @param ike_sa		IKE_SA this task works for
 * @param protocol		protocol of CHILD_SA to delete, PROTO_NONE as responder
 * @param spi			inbound SPI of CHILD_SA to delete
 * @param force			send delete even if SA does not exist
 * @param expired		TRUE if SA already expired
 * @return				quick_delete task to handle by the task_manager
 */
quick_delete_t *quick_delete_create(ike_sa_t *ike_sa, protocol_id_t protocol,
									uint32_t spi, bool force, bool expired);

#endif /** QUICK_DELETE_H_ @}*/
