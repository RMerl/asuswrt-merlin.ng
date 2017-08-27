/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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
 * @defgroup pkcs7 pkcs7
 * @{ @ingroup containers
 */

#ifndef PKCS7_H_
#define PKCS7_H_

#include <credentials/containers/container.h>

typedef struct pkcs7_t pkcs7_t;

/**
 * PKCS#7/CMS container type.
 */
struct pkcs7_t {

	/**
	 * Implements container_t.
	 */
	container_t container;

	/**
	 * Get an authenticated PKCS#9 attribute from PKCS#7 signerInfo.
	 *
	 * To select the signerInfo structure to get the attribute from, pass
	 * the enumerator position from container_t.create_signature_enumerator().
	 *
	 * The attribute returned does not contain type information and must be
	 * freed after use.
	 *
	 * @param oid			OID from the attribute to get
	 * @param enumerator	enumerator to select signerInfo
	 * @param value			chunk receiving attribute value, allocated
	 * @return				TRUE if attribute found
	 */
	bool (*get_attribute)(pkcs7_t *this, int oid, enumerator_t *enumerator,
						  chunk_t *value);

	/**
	 * Create an enumerator over attached certificates.
	 *
	 * @return				enumerator over certificate_t
	 */
	enumerator_t* (*create_cert_enumerator)(pkcs7_t *this);
};

#endif /** PKCS7_H_ @}*/
