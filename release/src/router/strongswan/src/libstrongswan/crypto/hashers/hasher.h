/*
 * Copyright (C) 2016-2017 Andreas Steffen
 * Copyright (C) 2012-2015 Tobias Brunner
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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
 * @defgroup hasher hasher
 * @{ @ingroup crypto
 */

#ifndef HASHER_H_
#define HASHER_H_

typedef enum hash_algorithm_t hash_algorithm_t;
typedef struct hasher_t hasher_t;

#include <crypto/prfs/prf.h>
#include <crypto/signers/signer.h>
#include <credentials/keys/public_key.h>

/**
 * Hash algorithms as defined for IKEv2
 */
enum hash_algorithm_t {
	/* RFC 7427 */
	HASH_SHA1 			= 1,
	HASH_SHA256			= 2,
	HASH_SHA384			= 3,
	HASH_SHA512			= 4,
	/* RFC 8420 */
	HASH_IDENTITY		= 5,
	/* use private use range for algorithms not defined/permitted by RFC 7427 */
	HASH_UNKNOWN 		= 1024,
	HASH_MD2 			= 1025,
	HASH_MD4			= 1026,
	HASH_MD5 			= 1027,
	HASH_SHA224			= 1028,
	HASH_SHA3_224		= 1029,
	HASH_SHA3_256		= 1030,
	HASH_SHA3_384		= 1031,
	HASH_SHA3_512		= 1032
};

#define HASH_SIZE_MD2		16
#define HASH_SIZE_MD4		16
#define HASH_SIZE_MD5		16
#define HASH_SIZE_SHA1		20
#define HASH_SIZE_SHA224	28
#define HASH_SIZE_SHA256	32
#define HASH_SIZE_SHA384	48
#define HASH_SIZE_SHA512	64

/**
 * enum names for hash_algorithm_t.
 */
extern enum_name_t *hash_algorithm_names;

/**
 * Short names for hash_algorithm_names
 */
extern enum_name_t *hash_algorithm_short_names;

/**
 * Uppercase short names for hash_algorithm_names
 */
extern enum_name_t *hash_algorithm_short_names_upper;

/**
 * Generic interface for all hash functions.
 */
struct hasher_t {

	/**
	 * Hash data and write it in the buffer.
	 *
	 * If the parameter hash is NULL, no result is written back
	 * and more data can be appended to already hashed data.
	 * If not, the result is written back and the hasher is reset.
	 *
	 * The hash output parameter must hold at least
	 * hash_t.get_block_size() bytes.
	 *
	 * @param data		data to hash
	 * @param hash		pointer where the hash will be written
	 * @return			TRUE if hash created successfully
	 */
	bool (*get_hash)(hasher_t *this, chunk_t data,
					 uint8_t *hash) __attribute__((warn_unused_result));

	/**
	 * Hash data and allocate space for the hash.
	 *
	 * If the parameter hash is NULL, no result is written back
	 * and more data can be appended to already hashed data.
	 * If not, the result is written back and the hasher is reset.
	 *
	 * @param data		chunk with data to hash
	 * @param hash		chunk which will hold allocated hash
	 * @return			TRUE if hash allocated successfully
	 */
	bool (*allocate_hash)(hasher_t *this, chunk_t data,
						  chunk_t *hash) __attribute__((warn_unused_result));

	/**
	 * Get the size of the resulting hash.
	 *
	 * @return			hash size in bytes
	 */
	size_t (*get_hash_size)(hasher_t *this);

	/**
	 * Resets the hasher's state.
	 *
	 * @return			TRUE if hasher reset successfully
	 */
	bool (*reset)(hasher_t *this) __attribute__((warn_unused_result));

	/**
	 * Destroys a hasher object.
	 */
	void (*destroy)(hasher_t *this);
};

/**
 * Returns the size of the hash for the given algorithm.
 *
 * @param alg			hash algorithm
 * @return				size of hash or 0 if unknown
 */
size_t hasher_hash_size(hash_algorithm_t alg);

/**
 * Conversion of ASN.1 OID to hash algorithm.
 *
 * @param oid			ASN.1 OID
 * @return				hash algorithm, HASH_UNKNOWN if OID unsupported
 */
hash_algorithm_t hasher_algorithm_from_oid(int oid);

/**
 * Conversion of PRF algorithm to hash algorithm (if based on one).
 *
 * @param alg			prf algorithm
 * @return				hash algorithm, HASH_UNKNOWN if not based on a hash
 */
hash_algorithm_t hasher_algorithm_from_prf(pseudo_random_function_t alg);

/**
 * Conversion of integrity algorithm to hash algorithm (if based on one).
 *
 * If length is not NULL the length of the resulting signature is returned,
 * which might be smaller than the output size of the underlying hash.
 *
 * @param alg			integrity algorithm
 * @param length		returns signature length, if not NULL
 * @return				hash algorithm, HASH_UNKNOWN if not based on a hash
 */
hash_algorithm_t hasher_algorithm_from_integrity(integrity_algorithm_t alg,
												 size_t *length);

/**
 * Conversion of hash algorithm to integrity algorithm (if based on a hash).
 *
 * @param alg			hash algorithm
 * @param length		length of the signature
 * @return				integrity algorithm, AUTH_UNDEFINED if none is known
 * 						based on the given hash function
 */
integrity_algorithm_t hasher_algorithm_to_integrity(hash_algorithm_t alg,
													size_t length);

/**
 * Check if the given algorithm may be used for IKEv2 signature authentication.
 *
 * @param alg			hash algorithm
 * @return				TRUE if algorithm may be used, FALSE otherwise
 */
bool hasher_algorithm_for_ikev2(hash_algorithm_t alg);

/**
 * Conversion of hash algorithm into ASN.1 OID.
 *
 * @param alg			hash algorithm
 * @return				ASN.1 OID, or OID_UNKNOW
 */
int hasher_algorithm_to_oid(hash_algorithm_t alg);

/**
 * Conversion of hash signature algorithm into ASN.1 OID.
 *
 * @param alg			hash algorithm
 * @param key			public key type
 * @return				ASN.1 OID if, or OID_UNKNOW
 */
int hasher_signature_algorithm_to_oid(hash_algorithm_t alg, key_type_t key);

/**
 * Determine the hash algorithm associated with a given signature scheme.
 *
 * @param scheme		signature scheme
 * @param params		optional parameters
 * @return				hash algorithm (could be HASH_UNKNOWN)
 */
hash_algorithm_t hasher_from_signature_scheme(signature_scheme_t scheme,
											  void *params);

#endif /** HASHER_H_ @}*/
