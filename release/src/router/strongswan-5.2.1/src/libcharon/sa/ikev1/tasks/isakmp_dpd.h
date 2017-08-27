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
 * @defgroup isakmp_dpd isakmp_dpd
 * @{ @ingroup tasks_v1
 */

#ifndef ISAKMP_DPD_H_
#define ISAKMP_DPD_H_

typedef struct isakmp_dpd_t isakmp_dpd_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/task.h>

/**
 * IKEv1 dead peer detection task.
 */
struct isakmp_dpd_t {

	/**
	 * Implements the task_t interface
	 */
	task_t task;
};

/**
 * Create a new ISAKMP_DPD task.
 *
 * @param ike_sa		associated IKE_SA
 * @param type			DPD notify to use, DPD_R_U_THERE | DPD_R_U_THERE_ACK
 * @param seqnr			DPD sequence number to use/expect
 * @return				ISAKMP_DPD task to handle by the task_manager
 */
isakmp_dpd_t *isakmp_dpd_create(ike_sa_t *ike_sa, notify_type_t type,
								u_int32_t seqnr);

#endif /** ISAKMP_DPD_H_ @}*/
