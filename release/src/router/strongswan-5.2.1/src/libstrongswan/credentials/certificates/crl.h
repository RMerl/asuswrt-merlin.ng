/*
 * Copyright (C) 2008 Martin Willi
 * Copyright (C) 2006 Andreas Steffen
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
 * @defgroup crl crl
 * @{ @ingroup certificates
 */

#ifndef CRL_H_
#define CRL_H_

typedef struct crl_t crl_t;
typedef enum crl_reason_t crl_reason_t;

#include <library.h>
#include <credentials/certificates/certificate.h>

/* <wincrypt.h> comes with CRL_REASON clashing with ours. Even if the values
 * are identical, we undef them here to use our enum instead of defines. */
#ifdef WIN32
# undef CRL_REASON_UNSPECIFIED
# undef CRL_REASON_KEY_COMPROMISE
# undef CRL_REASON_CA_COMPROMISE
# undef CRL_REASON_AFFILIATION_CHANGED
# undef CRL_REASON_SUPERSEDED
# undef CRL_REASON_CERTIFICATE_HOLD
# undef CRL_REASON_REMOVE_FROM_CRL
#endif

/**
 * RFC 2459 CRL reason codes
 */
enum crl_reason_t {
	CRL_REASON_UNSPECIFIED				= 0,
	CRL_REASON_KEY_COMPROMISE			= 1,
	CRL_REASON_CA_COMPROMISE			= 2,
	CRL_REASON_AFFILIATION_CHANGED		= 3,
	CRL_REASON_SUPERSEDED				= 4,
	CRL_REASON_CESSATION_OF_OPERATON	= 5,
	CRL_REASON_CERTIFICATE_HOLD			= 6,
	CRL_REASON_REMOVE_FROM_CRL			= 8,
};

/**
 * enum names for crl_reason_t
 */
extern enum_name_t *crl_reason_names;

/**
 * X509 certificate revocation list (CRL) interface definition.
 */
struct crl_t {

	/**
	 * Implements (parts of) the certificate_t interface
	 */
	certificate_t certificate;

	/**
	 * Get the CRL serial number.
	 *
	 * @return			chunk pointing to internal crlNumber
	 */
	chunk_t (*get_serial)(crl_t *this);

	/**
	 * Get the the authorityKeyIdentifier.
	 *
	 * @return			authKeyIdentifier chunk, point to internal data
	 */
	chunk_t (*get_authKeyIdentifier)(crl_t *this);

	/**
	 * Is this CRL a delta CRL?
	 *
	 * @param base_crl	gets to baseCrlNumber, if this is a delta CRL
	 * @return			TRUE if delta CRL
	 */
	bool (*is_delta_crl)(crl_t *this, chunk_t *base_crl);

	/**
	 * Create an enumerator over Freshest CRL distribution points and issuers.
	 *
	 * @return			enumerator over x509_cdp_t
	 */
	enumerator_t* (*create_delta_crl_uri_enumerator)(crl_t *this);

	/**
	 * Create an enumerator over all revoked certificates.
	 *
	 * The enumerator takes 3 pointer arguments:
	 * chunk_t serial, time_t revocation_date, crl_reason_t reason
	 *
	 * @return			enumerator over revoked certificates.
	 */
	enumerator_t* (*create_enumerator)(crl_t *this);
};

/**
 * Generic check if a given CRL is newer than another.
 *
 * @param crl			CRL
 * @param other			CRL to compare to
 * @return				TRUE if this newer than other
 */
bool crl_is_newer(crl_t *crl, crl_t *other);

#endif /** CRL_H_ @}*/
