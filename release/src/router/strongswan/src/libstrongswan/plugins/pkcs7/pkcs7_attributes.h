/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2008 Andreas Steffen
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
 * @defgroup pkcs7_attributes pkcs7_attributes
 * @{ @ingroup pkcs7p
 */

#ifndef PKCS7_ATTRIBUTES_H_
#define PKCS7_ATTRIBUTES_H_

typedef struct pkcs7_attributes_t pkcs7_attributes_t;

#include <library.h>

/**
 * PKCS#7 attribute lists, aka PKCS#9.
 */
struct pkcs7_attributes_t {

	/**
	 * Gets ASN.1 encoding of PKCS#9 attribute list.
	 *
	 * @return				ASN.1 encoded PKCSI#9 list
	 */
	chunk_t (*get_encoding) (pkcs7_attributes_t *this);

	/**
	 * Gets a PKCS#9 attribute from the list.
	 *
	 * @param oid			OID of the attribute
	 * @return				value of the attribute (internal data)
	 */
	chunk_t (*get_attribute) (pkcs7_attributes_t *this, int oid);

	/**
	 * Adds a PKCS#9 attribute.
	 *
	 * @param oid			OID of the attribute
	 * @param value			value of the attribute, with ASN1 type (gets owned)
	 */
	void (*add_attribute) (pkcs7_attributes_t *this, int oid, chunk_t value);

	/**
	 * Destroys the PKCS#9 attribute list.
	 */
	void (*destroy) (pkcs7_attributes_t *this);
};

/**
 * Read a PKCS#7 attribute list (aka PKCS#9) from a DER encoded chunk.
 *
 * @param chunk		chunk containing DER encoded data
 * @param level		ASN.1 parsing start level
 * @return 			created pkcs9 attribute list, or NULL if invalid.
 */
pkcs7_attributes_t *pkcs7_attributes_create_from_chunk(chunk_t chunk, u_int level);

/**
 * Create an empty PKCS#7 attribute list, aka PKCS#9.
 *
 * @return 				created pkcs9 attribute list.
 */
pkcs7_attributes_t *pkcs7_attributes_create(void);

#endif /** PKCS9_H_ @}*/
