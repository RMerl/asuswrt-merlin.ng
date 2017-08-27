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

/**
 * @defgroup cert_validator cert_validator
 * @{ @ingroup credentials
 */

#ifndef CERT_VALIDATOR_H_
#define CERT_VALIDATOR_H_

typedef struct cert_validator_t cert_validator_t;

#include <library.h>

/**
 * Certificate validator interface.
 *
 * A certificate validator checks constraints or revocation in a certificate
 * or its issuing CA certificate. The interface allows plugins to do
 * revocation checking or similar tasks.
 */
struct cert_validator_t {

	/**
	 * Check the lifetime of a certificate.
	 *
	 * If this function returns SUCCESS or FAILED, the certificate lifetime is
	 * considered definitely (in-)valid, without asking other validators.
	 * If all registered validaters return NEED_MORE, the default
	 * lifetime check is performed.
	 *
	 * @param cert			certificate to check lifetime
	 * @param pathlen		the current length of the path bottom-up
	 * @param anchor		is certificate trusted root anchor?
	 * @param auth			container for resulting authentication info
	 * @return				SUCCESS, FAILED or NEED_MORE to ask next validator
	 */
	status_t (*check_lifetime)(cert_validator_t *this, certificate_t *cert,
							   int pathlen, bool anchor, auth_cfg_t *auth);
	/**
	 * Validate a subject certificate in relation to its issuer.
	 *
	 * If FALSE is returned, the validator should call_hook() on the
	 * credential manager with an appropriate type and the certificate.
	 *
	 * @param subject		subject certificate to check
	 * @param issuer		issuer of subject
	 * @param online		whether to do online revocation checking
	 * @param pathlen		the current length of the path bottom-up
	 * @param anchor		is issuer trusted root anchor
	 * @param auth			container for resulting authentication info
	 * @return				TRUE if subject certificate valid
	 */
	bool (*validate)(cert_validator_t *this, certificate_t *subject,
					 certificate_t *issuer, bool online, u_int pathlen,
					 bool anchor, auth_cfg_t *auth);
};

#endif /** CERT_VALIDATOR_H_ @}*/
