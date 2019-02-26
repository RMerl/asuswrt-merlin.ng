/*
 * Copyright (C) 2015-2017 Tobias Brunner
 * Copyright (C) 2014-2017 Andreas Steffen
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
 * @defgroup public_key public_key
 * @{ @ingroup keys
 */

#ifndef PUBLIC_KEY_H_
#define PUBLIC_KEY_H_

typedef struct public_key_t public_key_t;
typedef enum key_type_t key_type_t;
typedef enum signature_scheme_t signature_scheme_t;
typedef enum encryption_scheme_t encryption_scheme_t;

#include <utils/identification.h>
#include <credentials/cred_encoding.h>

/**
 * Type of a key pair, the used crypto system
 */
enum key_type_t {
	/** key type wildcard */
	KEY_ANY     = 0,
	/** RSA crypto system as in PKCS#1 */
	KEY_RSA     = 1,
	/** ECDSA as in ANSI X9.62 */
	KEY_ECDSA   = 2,
	/** DSA */
	KEY_DSA     = 3,
	/** Ed25519 PureEdDSA instance as in RFC 8032 */
	KEY_ED25519 = 4,
	/** Ed448   PureEdDSA instance as in RFC 8032 */
	KEY_ED448   = 5,
	/** BLISS */
	KEY_BLISS = 6,
};

/**
 * Enum names for key_type_t
 */
extern enum_name_t *key_type_names;

/**
 * Signature scheme for signature creation
 *
 * EMSA-PKCS1 signatures are defined in PKCS#1 standard.
 * A prepended ASN.1 encoded digestInfo field contains the
 * OID of the used hash algorithm.
 */
enum signature_scheme_t {
	/** Unknown signature scheme                                       */
	SIGN_UNKNOWN,
	/** EMSA-PKCS1_v1.5 signature over digest without digestInfo       */
	SIGN_RSA_EMSA_PKCS1_NULL,
	/** EMSA-PKCS1_v1.5 signature as in PKCS#1 using RSA and MD5       */
	SIGN_RSA_EMSA_PKCS1_MD5,
	/** EMSA-PKCS1_v1.5 signature as in PKCS#1 using RSA and SHA-1     */
	SIGN_RSA_EMSA_PKCS1_SHA1,
	/** EMSA-PKCS1_v1.5 signature as in PKCS#1 using RSA and SHA-2_224 */
	SIGN_RSA_EMSA_PKCS1_SHA2_224,
	/** EMSA-PKCS1_v1.5 signature as in PKCS#1 using RSA and SHA-2_256 */
	SIGN_RSA_EMSA_PKCS1_SHA2_256,
	/** EMSA-PKCS1_v1.5 signature as in PKCS#1 using RSA and SHA-2_384 */
	SIGN_RSA_EMSA_PKCS1_SHA2_384,
	/** EMSA-PKCS1_v1.5 signature as in PKCS#1 using RSA and SHA-2_512 */
	SIGN_RSA_EMSA_PKCS1_SHA2_512,
	/** EMSA-PKCS1_v1.5 signature as in PKCS#1 using RSA and SHA-3_224 */
	SIGN_RSA_EMSA_PKCS1_SHA3_224,
	/** EMSA-PKCS1_v1.5 signature as in PKCS#1 using RSA and SHA-3_256 */
	SIGN_RSA_EMSA_PKCS1_SHA3_256,
	/** EMSA-PKCS1_v1.5 signature as in PKCS#1 using RSA and SHA-3_384 */
	SIGN_RSA_EMSA_PKCS1_SHA3_384,
	/** EMSA-PKCS1_v1.5 signature as in PKCS#1 using RSA and SHA-3_512 */
	SIGN_RSA_EMSA_PKCS1_SHA3_512,
	/** EMSA-PSS signature as in PKCS#1 using RSA                      */
	SIGN_RSA_EMSA_PSS,
	/** ECDSA with SHA-1 using DER encoding as in RFC 3279             */
	SIGN_ECDSA_WITH_SHA1_DER,
	/** ECDSA with SHA-256 using DER encoding as in RFC 3279           */
	SIGN_ECDSA_WITH_SHA256_DER,
	/** ECDSA with SHA-384 using DER encoding as in RFC 3279           */
	SIGN_ECDSA_WITH_SHA384_DER,
	/** ECDSA with SHA-1 using DER encoding as in RFC 3279             */
	SIGN_ECDSA_WITH_SHA512_DER,
	/** ECDSA over precomputed digest, signature as in RFC 4754        */
	SIGN_ECDSA_WITH_NULL,
	/** ECDSA on the P-256 curve with SHA-256 as in RFC 4754           */
	SIGN_ECDSA_256,
	/** ECDSA on the P-384 curve with SHA-384 as in RFC 4754           */
	SIGN_ECDSA_384,
	/** ECDSA on the P-521 curve with SHA-512 as in RFC 4754           */
	SIGN_ECDSA_521,
	/** PureEdDSA on Curve25519 as in RFC 8410                         */
	SIGN_ED25519,
	/** PureEdDSA on Curve448 as in RFC 8410                           */
	SIGN_ED448,
	/** BLISS with SHA-2_256                                           */
	SIGN_BLISS_WITH_SHA2_256,
	/** BLISS with SHA-2_384                                           */
	SIGN_BLISS_WITH_SHA2_384,
	/** BLISS with SHA-2_512                                           */
	SIGN_BLISS_WITH_SHA2_512,
	/** BLISS with SHA-3_256                                           */
	SIGN_BLISS_WITH_SHA3_256,
	/** BLISS with SHA-3_384                                           */
	SIGN_BLISS_WITH_SHA3_384,
	/** BLISS with SHA-3_512                                           */
	SIGN_BLISS_WITH_SHA3_512,
};

/**
 * Enum names for signature_scheme_t
 */
extern enum_name_t *signature_scheme_names;

/**
 * Encryption scheme for public key data encryption.
 */
enum encryption_scheme_t {
	/** Unknown encryption scheme                                      */
	ENCRYPT_UNKNOWN,
	/** RSAES-PKCS1-v1_5 as in PKCS#1                                  */
	ENCRYPT_RSA_PKCS1,
	/** RSAES-OAEP as in PKCS#1, using SHA1 as hash, no label          */
	ENCRYPT_RSA_OAEP_SHA1,
	/** RSAES-OAEP as in PKCS#1, using SHA-224 as hash, no label       */
	ENCRYPT_RSA_OAEP_SHA224,
	/** RSAES-OAEP as in PKCS#1, using SHA-256 as hash, no label       */
	ENCRYPT_RSA_OAEP_SHA256,
	/** RSAES-OAEP as in PKCS#1, using SHA-384 as hash, no label       */
	ENCRYPT_RSA_OAEP_SHA384,
	/** RSAES-OAEP as in PKCS#1, using SHA-512 as hash, no label       */
	ENCRYPT_RSA_OAEP_SHA512,
};

/**
 * Enum names for encryption_scheme_t
 */
extern enum_name_t *encryption_scheme_names;

/**
 * Abstract interface of a public key.
 */
struct public_key_t {

	/**
	 * Get the key type.
	 *
	 * @return			type of the key
	 */
	key_type_t (*get_type)(public_key_t *this);

	/**
	 * Verifies a signature against a chunk of data.
	 *
	 * @param scheme	signature scheme to use for verification
	 * @param params	optional parameters required by the specified scheme
	 * @param data		data to check signature against
	 * @param signature	signature to check
	 * @return			TRUE if signature matches
	 */
	bool (*verify)(public_key_t *this, signature_scheme_t scheme, void *params,
				   chunk_t data, chunk_t signature);

	/**
	 * Encrypt a chunk of data.
	 *
	 * @param scheme	encryption scheme to use
	 * @param plain		chunk containing plaintext data
	 * @param crypto	where to allocate encrypted data
	 * @return			TRUE if data successfully encrypted
	 */
	bool (*encrypt)(public_key_t *this, encryption_scheme_t scheme,
					chunk_t plain, chunk_t *crypto);

	/**
	 * Check if two public keys are equal.
	 *
	 * @param other		other public key
	 * @return			TRUE, if equality
	 */
	bool (*equals)(public_key_t *this, public_key_t *other);

	/**
	 * Get the strength of the key in bits.
	 *
	 * @return			strength of the key in bits
	 */
	int (*get_keysize) (public_key_t *this);

	/**
	 * Get the fingerprint of the key.
	 *
	 * @param type		type of fingerprint, one of KEYID_*
	 * @param fp		fingerprint, points to internal data
	 * @return			TRUE if fingerprint type supported
	 */
	bool (*get_fingerprint)(public_key_t *this, cred_encoding_type_t type,
							chunk_t *fp);

	/**
	 * Check if a key has a given fingerprint of any kind.
	 *
	 * @param fp		fingerprint to check
	 * @return			TRUE if key has given fingerprint
	 */
	bool (*has_fingerprint)(public_key_t *this, chunk_t fp);

	/**
	 * Get the key in an encoded form as a chunk.
	 *
	 * @param type		type of the encoding, one of PUBKEY_*
	 * @param encoding	encoding of the key, allocated
	 * @return			TRUE if encoding supported
	 */
	bool (*get_encoding)(public_key_t *this, cred_encoding_type_t type,
						 chunk_t *encoding);

	/**
	 * Increase the refcount of the key.
	 *
	 * @return			this with an increased refcount
	 */
	public_key_t* (*get_ref)(public_key_t *this);

	/**
	 * Destroy a public_key instance.
	 */
	void (*destroy)(public_key_t *this);
};

/**
 * Generic public key equals() implementation, usable by implementors.
 *
 * @param public		public key to check
 * @param other			key to compare
 * @return				TRUE if this is equal to other
 */
bool public_key_equals(public_key_t *public, public_key_t *other);

/**
 * Generic public key has_fingerprint() implementation, usable by implementors.
 *
 * @param public		public key to check
 * @param fingerprint	fingerprint to check
 * @return				TRUE if key has given fingerprint
 */
bool public_key_has_fingerprint(public_key_t *public, chunk_t fingerprint);

/**
 * Conversion of ASN.1 signature or hash OID to signature scheme.
 *
 * @param oid			ASN.1 OID
 * @return				signature scheme, SIGN_UNKNOWN if OID is unsupported
 */
signature_scheme_t signature_scheme_from_oid(int oid);

/**
 * Conversion of signature scheme to ASN.1 signature OID.
 *
 * @param scheme		signature scheme
 * @return				ASN.1 OID, OID_UNKNOWN if not supported
 */
int signature_scheme_to_oid(signature_scheme_t scheme);

/**
 * Enumerate signature schemes that are appropriate for a key of the given type
 * and size|strength ordered by increasing strength.
 *
 * @param type			type of the key
 * @param size			size or strength of the key
 * @return				enumerator over signature_params_t* (by strength)
 */
enumerator_t *signature_schemes_for_key(key_type_t type, int size);

/**
 * Determine the type of key associated with a given signature scheme.
 *
 * @param scheme		signature scheme
 * @return				key type (could be KEY_ANY)
 */
key_type_t key_type_from_signature_scheme(signature_scheme_t scheme);


#endif /** PUBLIC_KEY_H_ @}*/
