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
 * @defgroup addrblock_narrow addrblock_narrow
 * @{ @ingroup addrblock
 */

#ifndef ADDRBLOCK_NARROW_H_
#define ADDRBLOCK_NARROW_H_

#include <bus/listeners/listener.h>

typedef struct addrblock_narrow_t addrblock_narrow_t;

/**
 * Listener that checks traffic selectors against addrblock constraints.
 */
struct addrblock_narrow_t {

	/**
	 * Implements listener_t.
	 */
	listener_t listener;

	/**
	 * Destroy a addrblock_narrow_t.
	 */
	void (*destroy)(addrblock_narrow_t *this);
};

/**
 * Create a addrblock_narrow instance.
 */
addrblock_narrow_t *addrblock_narrow_create();

#endif /** ADDRBLOCK_NARROW_H_ @}*/
