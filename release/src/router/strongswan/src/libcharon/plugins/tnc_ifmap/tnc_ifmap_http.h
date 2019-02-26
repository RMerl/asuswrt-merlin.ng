/*
 * Copyright (C) 2013 Andreas Steffen
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
 * @defgroup tnc_ifmap_http tnc_ifmap_http
 * @{ @ingroup tnc_ifmap 
 */

#ifndef TNC_IFMAP_HTTP_H_
#define TNC_IFMAP_HTTP_H_

#include <library.h>
#include <tls_socket.h>

#include <libxml/parser.h>

typedef struct tnc_ifmap_http_t tnc_ifmap_http_t;

/**
 * Interface for building and processing HTTP messages
 */
struct tnc_ifmap_http_t {

	/**
	 * Build a HTTP POST message
	 *
	 * @param in			input data
	 * @param out			HTTP POST request
	 * @result				status return code
	 */
	status_t (*build)(tnc_ifmap_http_t *this, chunk_t *in, chunk_t *out);

	/**
	 * Receive a HTTP [chunked] response
	 *
	 * @param in			[chunked] HTTP response
	 * @param out			output data
	 * @result				status return code
	 */
	status_t (*process)(tnc_ifmap_http_t *this, chunk_t *in, chunk_t *out);

	/**
	 * Destroy a tnc_ifmap_http_t object.
	 */
	void (*destroy)(tnc_ifmap_http_t *this);
};

/**
 * Create a tnc_ifmap_http instance.
 *
 * @param uri			HTTPS URI with https:// prefix removed
 * @param user_pass		Optional username:password for HTTP Basic Authentication
 */
tnc_ifmap_http_t *tnc_ifmap_http_create(char *uri, chunk_t user_pass);

#endif /** TNC_IFMAP_HTTP_H_ @}*/
