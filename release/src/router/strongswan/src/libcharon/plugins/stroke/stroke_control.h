/*
 * Copyright (C) 2008 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
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
 * @defgroup stroke_control stroke_control
 * @{ @ingroup stroke
 */

#ifndef STROKE_CONTROL_H_
#define STROKE_CONTROL_H_

#include <stroke_msg.h>
#include <library.h>
#include <stdio.h>

typedef struct stroke_control_t stroke_control_t;

/**
 * Process stroke control messages
 */
struct stroke_control_t {

	/**
	 * Initiate a connection.
	 *
	 * @param msg		stroke message
	 */
	void (*initiate)(stroke_control_t *this, stroke_msg_t *msg, FILE *out);

	/**
	 * Terminate a connection.
	 *
	 * @param msg		stroke message
	 */
	void (*terminate)(stroke_control_t *this, stroke_msg_t *msg, FILE *out);

	/**
	 * Terminate a connection by peers virtual IP.
	 *
	 * @param msg		stroke message
	 */
	void (*terminate_srcip)(stroke_control_t *this, stroke_msg_t *msg, FILE *out);

	/**
	 * Rekey a connection.
	 *
	 * @param msg		stroke message
	 */
	void (*rekey)(stroke_control_t *this, stroke_msg_t *msg, FILE *out);

	/**
	 * Delete IKE_SAs without a CHILD_SA.
	 *
	 * @param msg		stroke message
	 */
	void (*purge_ike)(stroke_control_t *this, stroke_msg_t *msg, FILE *out);

	/**
	 * Route a connection.
	 *
	 * @param msg		stroke message
	 */
	void (*route)(stroke_control_t *this, stroke_msg_t *msg, FILE *out);

	/**
	 * Unroute a connection.
	 *
	 * @param msg		stroke message
	 */
	void (*unroute)(stroke_control_t *this, stroke_msg_t *msg, FILE *out);

	/**
	 * Destroy a stroke_control instance.
	 */
	void (*destroy)(stroke_control_t *this);
};

/**
 * Create a stroke_control instance.
 */
stroke_control_t *stroke_control_create();

#endif /** STROKE_CONTROL_H_ @}*/
