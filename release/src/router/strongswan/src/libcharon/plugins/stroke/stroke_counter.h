/*
 * Copyright (C) 2017 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
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
 * @defgroup stroke_counter stroke_counter
 * @{ @ingroup stroke
 */

#ifndef STROKE_COUNTER_H_
#define STROKE_COUNTER_H_

#include <library.h>

typedef struct stroke_counter_t stroke_counter_t;

/**
 * Interface for counter values for different IKE events.
 */
struct stroke_counter_t {

	/**
	 * Print counter values to an output stream.
	 *
	 * @param out		output stream to write to
	 * @param name		connection name to get counters for, NULL for global
	 */
	void (*print)(stroke_counter_t *this, FILE *out, char *name);

	/**
	 * Reset global or connection specific counters.
	 *
	 * @param name		name of connection counters to reset, NULL for global
	 */
	void (*reset)(stroke_counter_t *this, char *name);

	/**
	 * Destroy a stroke_counter_t.
	 */
	void (*destroy)(stroke_counter_t *this);
};

/**
 * Create a stroke_counter instance.
 */
stroke_counter_t *stroke_counter_create();

#endif /** STROKE_COUNTER_H_ @}*/
