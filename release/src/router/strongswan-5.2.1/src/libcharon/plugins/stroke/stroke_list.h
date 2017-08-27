/*
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup stroke_list stroke_list
 * @{ @ingroup stroke
 */

#ifndef STROKE_LIST_H_
#define STROKE_LIST_H_

#include "stroke_attribute.h"

#include <stroke_msg.h>
#include <library.h>

typedef struct stroke_list_t stroke_list_t;

/**
 * Log status information to stroke console
 */
struct stroke_list_t {

	/**
	 * List certificate information to stroke console.
	 *
	 * @param msg		stroke message
	 * @param out		stroke console stream
	 */
	void (*list)(stroke_list_t *this, stroke_msg_t *msg, FILE *out);

	/**
	 * Log status information to stroke console.
	 *
	 * @param msg		stroke message
	 * @param out		stroke console stream
	 * @param all		TRUE for "statusall"
	 * @param wait		TRUE to wait for IKE_SA entries, FALSE to skip if locked
	 */
	void (*status)(stroke_list_t *this, stroke_msg_t *msg, FILE *out,
				   bool all, bool wait);

	/**
	 * Log pool leases to stroke console.
	 *
	 * @param msg		stroke message
	 * @param out		stroke console stream
	 */
	void (*leases)(stroke_list_t *this, stroke_msg_t *msg, FILE *out);

	/**
	 * Destroy a stroke_list instance.
	 */
	void (*destroy)(stroke_list_t *this);
};

/**
 * Create a stroke_list instance.
 *
 * @param attribute		strokes attribute provider
 */
stroke_list_t *stroke_list_create(stroke_attribute_t *attribute);

#endif /** STROKE_LIST_H_ @}*/
