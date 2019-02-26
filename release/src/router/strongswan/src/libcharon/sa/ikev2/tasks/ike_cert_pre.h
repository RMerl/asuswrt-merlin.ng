/*
 * Copyright (C) 2007-2008 Martin Willi
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
 * @defgroup ike_cert_pre ike_cert_pre
 * @{ @ingroup tasks_v2
 */

#ifndef IKE_CERT_PRE_H_
#define IKE_CERT_PRE_H_

typedef struct ike_cert_pre_t ike_cert_pre_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/task.h>

/**
 * Task of type ike_cert_pre, certificate processing before authentication.
 */
struct ike_cert_pre_t {

	/**
	 * Implements the task_t interface
	 */
	task_t task;
};

/**
 * Create a new ike_cert_pre task.
 *
 * The initiator parameter means the original initiator, not the initiator
 * of the certificate request.
 *
 * @param ike_sa		IKE_SA this task works for
 * @param initiator		TRUE if task is the original initiator
 * @return				ike_cert_pre task to handle by the task_manager
 */
ike_cert_pre_t *ike_cert_pre_create(ike_sa_t *ike_sa, bool initiator);

#endif /** IKE_CERT_PRE_H_ @}*/
