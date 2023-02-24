/*
 * Copyright (C) 2009-2022 Andreas Steffen
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
 * @defgroup req req
 * @{ @ingroup certificates
 */

#ifndef PKCS10_H_
#define PKCS10_H_

#include "x509.h"

#include <collections/enumerator.h>
#include <credentials/certificates/certificate.h>

typedef struct pkcs10_t pkcs10_t;

/**
 * PKCS#10 certificate request interface.
 *
 * This interface adds additional methods to the certificate_t type to
 * allow further operations on a certificate request.
 */
struct pkcs10_t {

	/**
	 * Implements certificate_t.
	 */
	certificate_t interface;

	/**
	 * Get the challenge password
	 *
	 * @return			challenge password as a chunk_t
	 */
	chunk_t (*get_challengePassword)(pkcs10_t *this);

    /**
     * Get Extended Key Usage (EKU) flags
     *
     * @return          EKU flags
     */
    x509_flag_t (*get_flags)(pkcs10_t *this);

	/**
	 * Get subjectAltNames
	 *
	 * @return			enumerator over subjectAltNames as identification_t*
	 */
	enumerator_t* (*create_subjectAltName_enumerator)(pkcs10_t *this);

    /**
     * Replace the public key and private key signature
     *
     * @param private   new private key to be used
     * @param scheme    signature scheme
     * @param password  optionally set new password
     */
    certificate_t* (*replace_key)(pkcs10_t *this, private_key_t *private,
                    signature_params_t *scheme, chunk_t password);
};

#endif /** PKCS10_H_ @}*/
