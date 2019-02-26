/*
 * Copyright (C) 2013 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
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
 * @defgroup container container
 * @{ @ingroup containers
 */

#ifndef CONTAINER_H_
#define CONTAINER_H_

typedef struct container_t container_t;
typedef enum container_type_t container_type_t;

#include <utils/chunk.h>
#include <collections/enumerator.h>

/**
 * Type of the container.
 */
enum container_type_t {
	/** Any kind of PKCS#7/CMS container */
	CONTAINER_PKCS7,
	/** PKCS#7/CMS plain "data" */
	CONTAINER_PKCS7_DATA,
	/** PKCS#7/CMS "signed-data" */
	CONTAINER_PKCS7_SIGNED_DATA,
	/** PKCS#7/CMS "enveloped-data" */
	CONTAINER_PKCS7_ENVELOPED_DATA,
	/** PKCS#7/CMS "encrypted-data" */
	CONTAINER_PKCS7_ENCRYPTED_DATA,
	/** A PKCS#12 container */
	CONTAINER_PKCS12,
};

/**
 * Enum names for container_type_t
 */
extern enum_name_t *container_type_names;

/**
 * Generic interface for cryptographic containers.
 */
struct container_t {

	/**
	 * Get the type of the container.
	 *
	 * @return		container type
	 */
	container_type_t (*get_type)(container_t *this);

	/**
	 * Create an enumerator over trustchains for valid container signatures.
	 *
	 * @return		enumerator over auth_cfg_t*
	 */
	enumerator_t* (*create_signature_enumerator)(container_t *this);

	/**
	 * Get signed/decrypted data wrapped in this container.
	 *
	 * This function does not verify any associated signatures, use
	 * create_signature_enumerator() to verify them.
	 *
	 * @param data	allocated data wrapped in this container
	 * @return		TRUE if data decrypted successfully
	 */
	bool (*get_data)(container_t *this, chunk_t *data);

	/**
	 * Get the encoding of the full signed/encrypted container.
	 *
	 * @param data	allocated container encoding
	 * @return		TRUE if encodign successful
	 */
	bool (*get_encoding)(container_t *this, chunk_t *encoding);

	/**
	 * Destroy a container_t.
	 */
	void (*destroy)(container_t *this);
};

#endif /** CONTAINER_H_ @}*/
