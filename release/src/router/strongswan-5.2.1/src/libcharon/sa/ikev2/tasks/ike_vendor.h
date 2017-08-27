/*
 * Copyright (C) 2009 Martin Willi
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
 * @defgroup ike_vendor ike_vendor
 * @{ @ingroup tasks_v2
 */

#ifndef IKE_VENDOR_H_
#define IKE_VENDOR_H_

typedef struct ike_vendor_t ike_vendor_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/task.h>

/**
 * Vendor ID processing task.
 */
struct ike_vendor_t {

	/**
	 * Implements task interface.
	 */
	task_t task;
};

/**
 * Create a ike_vendor instance.
 *
 * @param ike_sa		IKE_SA this task works for
 * @param initiator		TRUE if task is the original initiator
 */
ike_vendor_t *ike_vendor_create(ike_sa_t *ike_sa, bool initiator);

#endif /** IKE_VENDOR_H_ @}*/
