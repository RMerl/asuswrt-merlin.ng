/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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
 * @defgroup vici_query vici_query
 * @{ @ingroup vici
 */

#include <bus/listeners/listener.h>
#include "vici_dispatcher.h"

#ifndef VICI_QUERY_H_
#define VICI_QUERY_H_

typedef struct vici_query_t vici_query_t;

/**
 * Query helper, provides various commands to query/list daemon info.
 */
struct vici_query_t {

	/**
	 * Implements listener_t.
	 */
	listener_t listener;

	/**
	 * Destroy a vici_query_t.
	 */
	void (*destroy)(vici_query_t *this);
};

/**
 * Create a vici_query instance.
 *
 * @param dispatcher		dispatcher to receive requests from
 * @return					query handler
 */
vici_query_t *vici_query_create(vici_dispatcher_t *dispatcher);

#endif /** VICI_QUERY_H_ @}*/
