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
 * @defgroup stroke_socket stroke_socket
 * @{ @ingroup stroke
 */

#ifndef STROKE_SOCKET_H_
#define STROKE_SOCKET_H_

typedef struct stroke_socket_t stroke_socket_t;

/**
 * Stroke socket, opens UNIX communication socket, reads and dispatches.
 */
struct stroke_socket_t {

	/**
	 * Destroy a stroke_socket instance.
	 */
	void (*destroy)(stroke_socket_t *this);
};

/**
 * Create a stroke_socket instance.
 */
stroke_socket_t *stroke_socket_create();

#endif /** STROKE_SOCKET_H_ @}*/
