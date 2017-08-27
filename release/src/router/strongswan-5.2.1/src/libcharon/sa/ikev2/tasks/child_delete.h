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
 * @defgroup child_delete child_delete
 * @{ @ingroup tasks_v2
 */

#ifndef CHILD_DELETE_H_
#define CHILD_DELETE_H_

typedef struct child_delete_t child_delete_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/task.h>
#include <sa/child_sa.h>

/**
 * Task of type child_delete, delete a CHILD_SA.
 */
struct child_delete_t {

	/**
	 * Implements the task_t interface
	 */
	task_t task;

	/**
	 * Get the CHILD_SA to delete by this task.
	 *
	 * @return			child_sa
	 */
	child_sa_t* (*get_child) (child_delete_t *this);
};

/**
 * Create a new child_delete task.
 *
 * @param ike_sa		IKE_SA this task works for
 * @param protocol		protocol of CHILD_SA to delete, PROTO_NONE as responder
 * @param spi			inbound SPI of CHILD_SA to delete
 * @param expired		TRUE if CHILD_SA already expired
 * @return				child_delete task to handle by the task_manager
 */
child_delete_t *child_delete_create(ike_sa_t *ike_sa, protocol_id_t protocol,
									u_int32_t spi, bool expired);

#endif /** CHILD_DELETE_H_ @}*/
