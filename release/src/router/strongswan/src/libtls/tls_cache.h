/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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
 * @defgroup tls_cache tls_cache
 * @{ @ingroup libtls
 */

#ifndef TLS_CACHE_H_
#define TLS_CACHE_H_

typedef struct tls_cache_t tls_cache_t;

#include "tls_crypto.h"

/**
 * TLS session cache facility.
 */
struct tls_cache_t {

	/**
	 * Create a new TLS session entry.
	 *
	 * @param session		session identifier
	 * @param id			identity the session is bound to
	 * @param master		TLS master secret
	 * @param suite			TLS cipher suite of the session
	 */
	void (*create)(tls_cache_t *this, chunk_t session, identification_t *id,
				   chunk_t master, tls_cipher_suite_t suite);

	/**
	 * Look up a TLS session entry.
	 *
	 * @param session		session ID to find
	 * @param id			identity the session is bound to
	 * @param master		gets allocated master secret, if session found
	 * @return				TLS suite of session, 0 if none found
	 */
	tls_cipher_suite_t (*lookup)(tls_cache_t *this, chunk_t session,
								 identification_t *id, chunk_t* master);

	/**
	 * Check if we have a session for a given identity.
	 *
	 * @param id			identity to check
	 * @return				allocated session ID, or chunk_empty
	 */
	chunk_t (*check)(tls_cache_t *this, identification_t *id);

	/**
	 * Destroy a tls_cache_t.
	 */
	void (*destroy)(tls_cache_t *this);
};

/**
 * Create a tls_cache instance.
 *
 * @param max_sessions		maximum number of sessions to store
 * @param max_age			maximum age of a session, in seconds
 * @return					tls cache
 */
tls_cache_t *tls_cache_create(u_int max_sessions, u_int max_age);

#endif /** TLS_CACHE_H_ @}*/
