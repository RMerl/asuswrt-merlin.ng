/*
 * Copyright (C) 2025 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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
 * @defgroup revocation_fetcher revocation_fetcher
 * @{ @ingroup revocation
 */

#ifndef REVOCATION_FETCHER_H_
#define REVOCATION_FETCHER_H_

#include <credentials/certificates/certificate.h>

typedef struct revocation_fetcher_t revocation_fetcher_t;

/**
 * Certificate fetcher performing the CRL/OCSP transfer.
 */
struct revocation_fetcher_t {

	/**
	 * Fetch a CRL from given URL.
	 *
	 * @param this		revocation fetcher
	 * @param url		URL to retrieve the CRL from
	 * @param timeout	timeout in seconds for the fetch operation
	 * @return			fetched CRL or NULL on error
	 */
	certificate_t *(*fetch_crl)(revocation_fetcher_t *this, char *url,
								u_int timeout);

	/**
	 * Fetch an OCSP response from given URL.
	 *
	 * @param this		revocation fetcher
	 * @param url		URL to retrieve the OCSP response from
	 * @param subject	subject to request OSCP status for
	 * @param issuer	issuer of the subject
	 * @param timeout	timeout in seconds for the fetch operation
	 * @return			fetched OCSP response or NULL on error
	 */
	certificate_t *(*fetch_ocsp)(revocation_fetcher_t *this, char *url,
								 certificate_t *subject, certificate_t *issuer,
								 u_int timeout);

	/**
	 * Destroy a revocation_fetcher_t.
	 */
	void (*destroy)(revocation_fetcher_t *this);
};

/**
 * Create a revocation_fetcher instance.
 */
revocation_fetcher_t *revocation_fetcher_create();

#endif /** REVOCATION_FETCHER_H_ @}*/
