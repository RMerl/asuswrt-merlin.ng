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
 * @defgroup isakmp_cert_pre isakmp_cert_pre
 * @{ @ingroup tasks_v1
 */

#ifndef ISAKMP_CERT_PRE_H_
#define ISAKMP_CERT_PRE_H_

typedef struct isakmp_cert_pre_t isakmp_cert_pre_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/task.h>

/**
 * ISAKMP_CERT_PRE task, IKEv1 certificate processing before authentication.
 */
struct isakmp_cert_pre_t {

	/**
	 * Implements the task_t interface
	 */
	task_t task;
};

/**
 * Create a new ISAKMP_CERT_PRE task.
 *
 * The initiator parameter means the original initiator, not the initiator
 * of the certificate request.
 *
 * @param ike_sa		IKE_SA this task works for
 * @param initiator		TRUE if task is the original initiator
 * @return				isakmp_cert_pre task to handle by the task_manager
 */
isakmp_cert_pre_t *isakmp_cert_pre_create(ike_sa_t *ike_sa, bool initiator);

#endif /** ISAKMP_CERT_PRE_H_ @}*/
