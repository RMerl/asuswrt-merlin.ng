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
 * @defgroup cert_cache cert_cache
 * @{ @ingroup sets
 */

#ifndef CERT_CACHE_H_
#define CERT_CACHE_H_

#include <credentials/credential_set.h>

typedef struct cert_cache_t cert_cache_t;

/**
 * Certificate signature verification and certificate cache.
 *
 * This cache serves all certificates seen in its issued_by method
 * and serves them as untrusted through the credential set interface. Further,
 * it caches valid subject-issuer relationships to speed up the issued_by
 * method.
 */
struct cert_cache_t {

	/**
	 * Implements credential_set_t.
	 */
	credential_set_t set;

	/**
	 * Caching wrapper around certificate_t.issued_by.
	 *
	 * @param subject		certificate to verify
	 * @param issuer		issuing certificate to verify subject
	 * @param scheme		receives used signature scheme and parameters, if
	 *						given (allocated)
	 * @return				TRUE if subject issued by issuer
	 */
	bool (*issued_by)(cert_cache_t *this,
					  certificate_t *subject, certificate_t *issuer,
					  signature_params_t **scheme);

	/**
	 * Flush the certificate cache.
	 *
	 * @param type			type of certificate to flush, or CERT_ANY
	 */
	void (*flush)(cert_cache_t *this, certificate_type_t type);

	/**
	 * Destroy a cert_cache instance.
	 */
	void (*destroy)(cert_cache_t *this);
};

/**
 * Create a cert_cache instance.
 */
cert_cache_t *cert_cache_create();

#endif /** CERT_CACHE_H_ @}*/
