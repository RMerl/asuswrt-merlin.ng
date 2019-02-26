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
 * @defgroup certexpire_listener certexpire_listener
 * @{ @ingroup certexpire
 */

#ifndef CERTEXPIRE_LISTENER_H_
#define CERTEXPIRE_LISTENER_H_

#include <bus/listeners/listener.h>

#include "certexpire_export.h"

typedef struct certexpire_listener_t certexpire_listener_t;

/**
 * Listener collecting certificate expire information after authentication.
 */
struct certexpire_listener_t {

	/**
	 * Implements listener_t interface.
	 */
	listener_t listener;

	/**
	 * Destroy a certexpire_listener_t.
	 */
	void (*destroy)(certexpire_listener_t *this);
};

/**
 * Create a certexpire_listener instance.
 *
 * @param export		facility exporting collected trustchains
 * @return				listener instance
 */
certexpire_listener_t *certexpire_listener_create(certexpire_export_t *export);

#endif /** CERTEXPIRE_LISTENER_H_ @}*/
