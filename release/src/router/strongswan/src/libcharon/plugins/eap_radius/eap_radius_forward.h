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
 * @defgroup eap_radius_forward eap_radius_forward
 * @{ @ingroup eap_radius
 */

#ifndef EAP_RADIUS_FORWARD_H_
#define EAP_RADIUS_FORWARD_H_

#include <radius_message.h>

#include <bus/listeners/listener.h>

typedef struct eap_radius_forward_t eap_radius_forward_t;

/**
 * Forward RADIUS attributes in Notifies between client and AAA backend.
 */
struct eap_radius_forward_t {

	/**
	 * Implements a listener.
	 */
	listener_t listener;

	/**
	 * Destroy a eap_radius_forward_t.
	 */
	void (*destroy)(eap_radius_forward_t *this);
};

/**
 * Create a eap_radius_forward instance.
 */
eap_radius_forward_t *eap_radius_forward_create();

/**
 * Forward RADIUS attributes from IKE notifies to a RADIUS request.
 *
 * @param request		RADIUS request message to add attributes to
 */
void eap_radius_forward_from_ike(radius_message_t *request);

/**
 * Forward RADIUS attributes from a RADIUS response to IKE notifies.
 *
 * @param response		RADIUS response to read notifies from
 */
void eap_radius_forward_to_ike(radius_message_t *response);

#endif /** EAP_RADIUS_FORWARD_H_ @}*/
