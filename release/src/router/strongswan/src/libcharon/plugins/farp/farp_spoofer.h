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
 * @defgroup farp_spoofer farp_spoofer
 * @{ @ingroup farp
 */

#ifndef FARP_SPOOFER_H_
#define FARP_SPOOFER_H_

#include "farp_listener.h"

typedef struct farp_spoofer_t farp_spoofer_t;

/**
 * Listen to ARP requests and spoof responses, if required.
 */
struct farp_spoofer_t {

	/**
	 * Destroy a farp_spoofer_t.
	 */
	void (*destroy)(farp_spoofer_t *this);
};

/**
 * Create a farp_spoofer instance.
 *
 * @param listener		listener to check for addresses to spoof
 * @return				spoofer instance
 */
farp_spoofer_t *farp_spoofer_create(farp_listener_t *listener);

#endif /** FARP_SPOOFER_H_ @}*/
