/*
 * Copyright (C) 2005-2008 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <utils/debug.h>

#include "random_rng.h"
#include "random_plugin.h"

typedef struct private_random_rng_t private_random_rng_t;

/**
 * Private data of an random_rng_t object.
 */
struct private_random_rng_t {

	/**
	 * Public random_rng_t interface.
	 */
	random_rng_t public;

	/**
	 * random device, depends on quality
	 */
	int fd;
};

METHOD(rng_t, get_bytes, bool,
	private_random_rng_t *this, size_t bytes, u_int8_t *buffer)
{
	size_t done;
	ssize_t got;

	done = 0;

	while (done < bytes)
	{
		got = read(this->fd, buffer + done, bytes - done);
		if (got <= 0)
		{
			DBG1(DBG_LIB, "reading from random FD %d failed: %s, retrying...",
				 this->fd, strerror(errno));
			sleep(1);
		}
		done += got;
	}
	return TRUE;
}

METHOD(rng_t, allocate_bytes, bool,
	private_random_rng_t *this, size_t bytes, chunk_t *chunk)
{
	*chunk = chunk_alloc(bytes);
	get_bytes(this, chunk->len, chunk->ptr);
	return TRUE;
}

METHOD(rng_t, destroy, void,
	private_random_rng_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
random_rng_t *random_rng_create(rng_quality_t quality)
{
	private_random_rng_t *this;

	INIT(this,
		.public = {
			.rng = {
				.get_bytes = _get_bytes,
				.allocate_bytes = _allocate_bytes,
				.destroy = _destroy,
			},
		},
	);

	switch (quality)
	{
		case RNG_TRUE:
			this->fd = random_plugin_get_dev_random();
			break;
		case RNG_STRONG:
			this->fd = random_plugin_get_strong_equals_true() ?
							random_plugin_get_dev_random() :
							random_plugin_get_dev_urandom();
			break;
		case RNG_WEAK:
		default:
			this->fd = random_plugin_get_dev_urandom();
			break;
	}

	return &this->public;
}

