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
 * @defgroup unity_narrow unity_narrow
 * @{ @ingroup unity
 */

#ifndef UNITY_NARROW_H_
#define UNITY_NARROW_H_

#include <bus/listeners/listener.h>

#include "unity_handler.h"

typedef struct unity_narrow_t unity_narrow_t;

/**
 * Listener that narrows Quick Modes to the Unity Split-Include subnets.
 */
struct unity_narrow_t {

	/**
	 * Implements listener_t.
	 */
	listener_t listener;

	/**
	 * Destroy a unity_narrow_t.
	 */
	void (*destroy)(unity_narrow_t *this);
};

/**
 * Create a unity_narrow instance.
 */
unity_narrow_t *unity_narrow_create(unity_handler_t *handler);

#endif /** UNITY_NARROW_H_ @}*/
