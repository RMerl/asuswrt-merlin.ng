/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
 * @defgroup hook_t hook
 * @{ @ingroup conftest
 */

#ifndef HOOK_H_
#define HOOK_H_

typedef struct hook_t hook_t;

#include <daemon.h>
#include <conftest.h>

/**
 * Hook providing interface.
 */
struct hook_t {

	/**
	 * Implements listener_t.
	 */
	listener_t listener;

	/**
	 * Destroy a hook_t.
	 */
	void (*destroy)(hook_t *this);
};

#endif /** HOOK_H_ @}*/
