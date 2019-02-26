/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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
 * Copyright (C) 2014 Timo Teräs <timo.teras@iki.fi>
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
 * @defgroup vici_dispatcher vici_dispatcher
 * @{ @ingroup vici
 */

#ifndef VICI_DISPATCHER_H_
#define VICI_DISPATCHER_H_

#include "vici_message.h"

typedef struct vici_dispatcher_t vici_dispatcher_t;
typedef enum vici_operation_t vici_operation_t;

/**
 * Default socket URI of vici service
 */
#ifdef WIN32
# define VICI_DEFAULT_URI "tcp://127.0.0.1:4502"
#else
# define VICI_DEFAULT_URI "unix://" IPSEC_PIDDIR "/charon.vici"
#endif

/**
 * Kind of vici operation
 */
enum vici_operation_t {
	/** a named request message */
	VICI_CMD_REQUEST,
	/** an unnamed response message to a request */
	VICI_CMD_RESPONSE,
	/** unnamed response if requested command is unknown */
	VICI_CMD_UNKNOWN,
	/** a named event registration request */
	VICI_EVENT_REGISTER,
	/** a named event unregistration request */
	VICI_EVENT_UNREGISTER,
	/** unnamed response for successful event (un-)registration */
	VICI_EVENT_CONFIRM,
	/** unnamed response if event (un-)registration failed */
	VICI_EVENT_UNKNOWN,
	/** a named event message */
	VICI_EVENT,
};

/**
 * Vici command callback function
 *
 * @param user			user data, as supplied during registration
 * @param name			name of the command it has been registered under
 * @param id			client connection identifier
 * @param request		request message data
 * @return				response message
 */
typedef vici_message_t* (*vici_command_cb_t)(void *user, char *name, u_int id,
											 vici_message_t *request);

/**
 * Vici command dispatcher.
 */
struct vici_dispatcher_t {

	/**
	 * Register/Unregister a callback invoked for a specific command request.
	 *
	 * @param name			name of the command
	 * @param cb			callback function to register, NULL to unregister
	 * @param user			user data to pass to callback
	 */
	void (*manage_command)(vici_dispatcher_t *this, char *name,
						   vici_command_cb_t cb, void *user);

	/**
	 * Register/Unregister an event type to send.
	 *
	 * The dispatcher internally manages event subscriptions. Clients registered
	 * for an event will receive such messages when the event is raised.
	 *
	 * @param name			event name to manager
	 * @param reg			TRUE to register, FALSE to unregister
	 */
	void (*manage_event)(vici_dispatcher_t *this, char *name, bool reg);

	/**
	 * Check if an event has listeners.
	 *
	 * This can be used to check if a vici message needs to be generated or not,
	 * as in some cases the generation can be a heavy operation.
	 *
	 * @param name			event name to check
	 * @return				TRUE if event has listeners
	 */
	bool (*has_event_listeners)(vici_dispatcher_t *this, char *name);

	/**
	 * Raise an event to a specific or all clients registered to that event.
	 *
	 * @param name			event name to raise
	 * @param id			client connection ID, 0 for all
	 * @param message		event message to send, gets destroyed
	 */
	void (*raise_event)(vici_dispatcher_t *this, char *name, u_int id,
						vici_message_t *message);

	/**
	 * Destroy a vici_dispatcher_t.
	 */
	void (*destroy)(vici_dispatcher_t *this);
};

/**
 * Create a vici_dispatcher instance.
 *
 * @param uri		uri for listening stream service
 * @return			dispatcher instance
 */
vici_dispatcher_t *vici_dispatcher_create(char *uri);

#endif /** VICI_DISPATCHER_H_ @}*/
