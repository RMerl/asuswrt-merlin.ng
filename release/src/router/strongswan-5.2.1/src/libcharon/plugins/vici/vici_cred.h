/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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
 * @defgroup vici_cred vici_cred
 * @{ @ingroup vici
 */

#ifndef VICI_CRED_H_
#define VICI_CRED_H_

#include "vici_dispatcher.h"

typedef struct vici_cred_t vici_cred_t;

/**
 * In-memory credential backend, managed by VICI.
 */
struct vici_cred_t {

	/**
	 * Destroy a vici_cred_t.
	 */
	void (*destroy)(vici_cred_t *this);
};

/**
 * Create a vici_cred instance.
 *
 * @param dispatcher		dispatcher to receive requests from
 * @return					credential backend
 */
vici_cred_t *vici_cred_create(vici_dispatcher_t *dispatcher);

#endif /** VICI_CRED_H_ @}*/
