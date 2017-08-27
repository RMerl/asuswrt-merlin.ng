/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include "tls_compression.h"

typedef struct private_tls_compression_t private_tls_compression_t;

/**
 * Private data of an tls_compression_t object.
 */
struct private_tls_compression_t {

	/**
	 * Public tls_compression_t interface.
	 */
	tls_compression_t public;

	/**
	 * Upper layer, TLS record fragmentation
	 */
	tls_fragmentation_t *fragmentation;
};

METHOD(tls_compression_t, process, status_t,
	private_tls_compression_t *this, tls_content_type_t type, chunk_t data)
{
	return this->fragmentation->process(this->fragmentation, type, data);
}

METHOD(tls_compression_t, build, status_t,
	private_tls_compression_t *this, tls_content_type_t *type, chunk_t *data)
{
	return this->fragmentation->build(this->fragmentation, type, data);
}

METHOD(tls_compression_t, destroy, void,
	private_tls_compression_t *this)
{
	free(this);
}

/**
 * See header
 */
tls_compression_t *tls_compression_create(tls_fragmentation_t *fragmentation,
										  tls_alert_t *alert)
{
	private_tls_compression_t *this;

	INIT(this,
		.public = {
			.process = _process,
			.build = _build,
			.destroy = _destroy,
		},
		.fragmentation = fragmentation,
	);

	return &this->public;
}
