/*
 * Copyright (C) 2016 Tobias Brunner
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
/*
 * Copyright (C) 2016 Stephen J. Bevan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @defgroup ike_mid_sync ike_mid_sync
 * @{ @ingroup tasks_v2
 */

#ifndef IKE_MID_SYNC_H_
#define IKE_MID_SYNC_H_

typedef struct ike_mid_sync_t ike_mid_sync_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/task.h>

/**
 * Task of type TASK_IKE_MID_SYNC, implements RFC 6311 responder.
 *
 * This task handles an IKEV2_MESSAGE_ID_SYNC notify sent by a peer
 * and if acceptable updates the SA MIDs and replies with the updated
 * MID values.
 */
struct ike_mid_sync_t {

	/**
	 * Implements the task_t interface
	 */
	task_t task;
};

/**
 * Create a new TASK_IKE_MID_SYNC task.
 *
 * @param ike_sa	IKE_SA this task works for
 * @return			task to handle by the task_manager
 */
ike_mid_sync_t *ike_mid_sync_create(ike_sa_t *ike_sa);

#endif /** IKE_MID_SYNC_H_ @}*/
