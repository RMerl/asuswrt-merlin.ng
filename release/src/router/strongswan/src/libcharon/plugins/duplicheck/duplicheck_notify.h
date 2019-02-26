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
 * @defgroup duplicheck_notify duplicheck_notify
 * @{ @ingroup duplicheck
 */

#ifndef DUPLICHECK_NOTIFY_H_
#define DUPLICHECK_NOTIFY_H_

#include <utils/identification.h>

typedef struct duplicheck_notify_t duplicheck_notify_t;

/**
 * Sends notifications over a unix socket when duplicates are detected.
 */
struct duplicheck_notify_t {

	/**
	 * Send a notification message if duplicate IKE_SA detected.
	 *
	 * @param id		identity a duplicate tunnel has been detected
	 */
	void (*send)(duplicheck_notify_t *this, identification_t *id);

	/**
	 * Destroy a duplicheck_notify_t.
	 */
	void (*destroy)(duplicheck_notify_t *this);
};

/**
 * Create a duplicheck_notify instance.
 */
duplicheck_notify_t *duplicheck_notify_create();

#endif /** DUPLICHECK_NOTIFY_H_ @}*/
