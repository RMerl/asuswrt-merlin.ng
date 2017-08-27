/*
 * Copyright (C) 2005-2006 Martin Willi
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

/**
 * @defgroup certreq_payload certreq_payload
 * @{ @ingroup payloads
 */

#ifndef CERTREQ_PAYLOAD_H_
#define CERTREQ_PAYLOAD_H_

typedef struct certreq_payload_t certreq_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>
#include <encoding/payloads/cert_payload.h>
#include <utils/identification.h>

/**
 * Class representing an IKEv1/IKEv2 CERTREQ payload.
 */
struct certreq_payload_t {

	/**
	 * The payload_t interface.
	 */
	payload_t payload_interface;

	/**
	 * Create an enumerator over contained keyids (IKEv2 only).
	 *
	 * @return			enumerator over chunk_t's.
	 */
	enumerator_t* (*create_keyid_enumerator)(certreq_payload_t *this);

	/**
	 * Get the type of contained certificate keyids.
	 *
	 * @return			certificate keyid type
	 */
	certificate_type_t (*get_cert_type)(certreq_payload_t *this);

	/**
	 * Add a certificates keyid to the payload (IKEv2 only).
	 *
	 * @param keyid		keyid of the trusted certificate
	 * @return
	 */
	void (*add_keyid)(certreq_payload_t *this, chunk_t keyid);

	/**
	 * Get the distinguished name of the payload (IKEv1 only).
	 *
	 * @return			DN as identity, must be destroyed
	 */
	identification_t* (*get_dn)(certreq_payload_t *this);

	/**
	 * Destroys an certreq_payload_t object.
	 */
	void (*destroy) (certreq_payload_t *this);
};

/**
 * Creates an empty certreq_payload_t object.
 *
 * @return 				certreq payload
 */
certreq_payload_t *certreq_payload_create(payload_type_t payload_type);

/**
 * Creates an empty IKEv2 certreq_payload_t for a kind of certificates.
 *
 * @param type			type of the added keyids
 * @return 				certreq payload
 */
certreq_payload_t *certreq_payload_create_type(certificate_type_t type);

/**
 * Creates a IKEv1 certreq_payload_t for a given distinguished name.
 *
 * @param id			distinguished name, does not get owned
 * @return 				certreq payload
 */
certreq_payload_t *certreq_payload_create_dn(identification_t *id);

#endif /** CERTREQ_PAYLOAD_H_ @}*/
