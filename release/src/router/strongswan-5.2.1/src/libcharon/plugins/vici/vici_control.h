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
 * @defgroup vici_control vici_control
 * @{ @ingroup vici
 */

#include "vici_dispatcher.h"

#ifndef VICI_CONTROL_H_
#define VICI_CONTROL_H_

typedef struct vici_control_t vici_control_t;

/**
 * Control helper, provides initiate/terminate and other commands.
 */
struct vici_control_t {

	/**
	 * Destroy a vici_control_t.
	 */
	void (*destroy)(vici_control_t *this);
};

/**
 * Create a vici_control instance.
 *
 * @param dispatcher		dispatcher to receive requests from
 * @return					query handler
 */
vici_control_t *vici_control_create(vici_dispatcher_t *dispatcher);

#endif /** VICI_CONTROL_H_ @}*/
