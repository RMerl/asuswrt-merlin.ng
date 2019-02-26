/*
 * Copyright (C) 2011 Tobias Brunner
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
 * @defgroup isakmp_natd isakmp_natd
 * @{ @ingroup tasks_v1
 */

#ifndef ISAKMP_NATD_H_
#define ISAKMP_NATD_H_

typedef struct isakmp_natd_t isakmp_natd_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/task.h>

/**
 * Task of type ISAKMP_NATD, detects NAT situation in IKEv1 Phase 1.
 */
struct isakmp_natd_t {

	/**
	 * Implements the task_t interface
	 */
	task_t task;
};

/**
 * Create a new ISAKMP_NATD task.
 *
 * @param ike_sa		IKE_SA this task works for
 * @param initiator		TRUE if task is the original initiator
 * @return				isakmp_natd task to handle by the task_manager
 */
isakmp_natd_t *isakmp_natd_create(ike_sa_t *ike_sa, bool initiator);

#endif /** ISAKMP_NATD_H_ @}*/
