/*
 * Copyright (C) 2007 Martin Willi
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
 * @defgroup shared_key shared_key
 * @{ @ingroup keys
 */

#ifndef SHARED_KEY_H_
#define SHARED_KEY_H_

#include <collections/enumerator.h>
#include <utils/identification.h>

typedef struct shared_key_t shared_key_t;
typedef enum shared_key_type_t shared_key_type_t;

/**
 * Type of a shared key.
 */
enum shared_key_type_t {
	/** wildcard for all keys */
	SHARED_ANY,
	/** PSK for IKE authentication */
	SHARED_IKE,
	/** key for a EAP authentication method */
	SHARED_EAP,
	/** key to decrypt encrypted private keys */
	SHARED_PRIVATE_KEY_PASS,
	/** PIN to unlock a smartcard */
	SHARED_PIN,
	/** Calculated NT Hash = MD4(UTF-16LE(password)) */
	SHARED_NT_HASH,
	/** Postquantum Preshared Key */
	SHARED_PPK,
};

/**
 * enum names for shared_key_type_t
 */
extern enum_name_t *shared_key_type_names;

/**
 * A symmetric key shared between multiple owners.
 *
 * This class is not thread save, do not add owners while others might be
 * reading.
 */
struct shared_key_t {

	/**
	 * Get the kind of this key.
	 *
	 * @return			type of the key
	 */
	shared_key_type_t (*get_type)(shared_key_t *this);

	/**
	 * Get the shared key data.
	 *
	 * @return			chunk pointing to the internal key
	 */
	chunk_t (*get_key)(shared_key_t *this);

	/**
	 * Increase refcount of the key.
	 *
	 * @return			this with an increased refcount
	 */
	shared_key_t* (*get_ref)(shared_key_t *this);

	/**
	 * Destroy a shared_key instance if all references are gone.
	 */
	void (*destroy)(shared_key_t *this);
};

/**
 * A simple private key implementation
 *
 * @param type		type of the shared key
 * @param key		key data, gets owned by instance
 * @return			simple shared key instance
 */
shared_key_t *shared_key_create(shared_key_type_t type, chunk_t key);

#endif /** SHARED_KEY_H_ @} */
