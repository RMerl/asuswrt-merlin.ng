/*
 * Copyright (C) 2009 Martin Willi
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
 * @defgroup cred_encoding cred_encoding
 * @{ @ingroup credentials
 */

#ifndef CRED_ENCODING_H_
#define CRED_ENCODING_H_

typedef struct cred_encoding_t cred_encoding_t;
typedef enum cred_encoding_type_t cred_encoding_type_t;
typedef enum cred_encoding_part_t cred_encoding_part_t;

#include <utils/chunk.h>

/**
 * Credential encoder function implementing encoding/fingerprinting.
 *
 * The variable argument list takes cred_encoding_part_t, followed by part
 * specific arguments, terminated by KEY_PART_END.
 *
 * @param type		format to encode the credential to
 * @param args		list of (cred_encoding_part_t, data)
 * @param encoding	encoding result, allocated
 * @return			TRUE if encoding successful
 */
typedef bool (*cred_encoder_t)(cred_encoding_type_t type, chunk_t *encoding,
							   va_list args);

/**
 * Helper function for cred_encoder_t implementations to parse argument list.
 *
 * Credential encoder functions get a variable argument list to parse. To
 * simplify the job, this function reads the arguments and returns chunks for
 * each part.
 * The argument list of this function takes a cred_encoding_part_t, followed
 * by a data pointer receiving the value, terminated by CRED_PART_END.
 *
 * @param args		argument list passed to credential encoder function
 * @param ...		list of (cred_encoding_part_t, data*)
 * @return			TRUE if all parts found, FALSE otherwise
 */
bool cred_encoding_args(va_list args, ...);

/**
 * Encoding type of a fingerprint/credential.
 *
 * Fingerprints have the KEYID_*, public keys the PUBKEY_* and
 * private keys the PRIVKEY_* prefix.
 */
enum cred_encoding_type_t {
	/** SHA1 fingerprint over subjectPublicKeyInfo */
	KEYID_PUBKEY_INFO_SHA1 = 0,
	/** SHA1 fingerprint over subjectPublicKey */
	KEYID_PUBKEY_SHA1,
	/** PGPv3 fingerprint */
	KEYID_PGPV3,
	/** PGPv4 fingerprint */
	KEYID_PGPV4,

	KEYID_MAX,

	/** PKCS#1 and similar ASN.1 key encoding */
	PUBKEY_ASN1_DER,
	PRIVKEY_ASN1_DER,
	/** subjectPublicKeyInfo encoding */
	PUBKEY_SPKI_ASN1_DER,
	/** PEM encoded PKCS#1 key */
	PUBKEY_PEM,
	PRIVKEY_PEM,
	/** PGP key encoding */
	PUBKEY_PGP,
	PRIVKEY_PGP,
	/** DNSKEY encoding */
	PUBKEY_DNSKEY,
	/** SSHKEY encoding (Base64) */
	PUBKEY_SSHKEY,
	/** RSA modulus only */
	PUBKEY_RSA_MODULUS,

	/** ASN.1 DER encoded certificate */
	CERT_ASN1_DER,
	/** PEM encoded certificate */
	CERT_PEM,
	/** PGP Packet encoded certificate */
	CERT_PGP_PKT,

	CRED_ENCODING_MAX,
};

/**
 * Parts of a credential to encode.
 */
enum cred_encoding_part_t {
	/** modulus of a RSA key, n */
	CRED_PART_RSA_MODULUS,
	/** public exponent of a RSA key, e */
	CRED_PART_RSA_PUB_EXP,
	/** private exponent of a RSA key, d */
	CRED_PART_RSA_PRIV_EXP,
	/** prime1 a RSA key, p */
	CRED_PART_RSA_PRIME1,
	/** prime2 a RSA key, q */
	CRED_PART_RSA_PRIME2,
	/** exponent1 a RSA key, exp1 */
	CRED_PART_RSA_EXP1,
	/** exponent1 a RSA key, exp2 */
	CRED_PART_RSA_EXP2,
	/** coefficient of RSA key, coeff */
	CRED_PART_RSA_COEFF,
	/** a DER encoded RSA public key */
	CRED_PART_RSA_PUB_ASN1_DER,
	/** a DER encoded RSA private key */
	CRED_PART_RSA_PRIV_ASN1_DER,
	/** a DER encoded ECDSA public key */
	CRED_PART_ECDSA_PUB_ASN1_DER,
	/** a DER encoded ECDSA private key */
	CRED_PART_ECDSA_PRIV_ASN1_DER,
	/** a DER encoded X509 certificate */
	CRED_PART_X509_ASN1_DER,
	/** a DER encoded X509 CRL */
	CRED_PART_X509_CRL_ASN1_DER,
	/** a DER encoded X509 OCSP request */
	CRED_PART_X509_OCSP_REQ_ASN1_DER,
	/** a DER encoded X509 OCSP response */
	CRED_PART_X509_OCSP_RES_ASN1_DER,
	/** a DER encoded X509 attribute certificate */
	CRED_PART_X509_AC_ASN1_DER,
	/** a DER encoded PKCS10 certificate request */
	CRED_PART_PKCS10_ASN1_DER,
	/** a PGP encoded certificate */
	CRED_PART_PGP_CERT,
	/** a DER encoded EdDSA public key */
	CRED_PART_EDDSA_PUB_ASN1_DER,
	/** a DER encoded EdDSA private key */
	CRED_PART_EDDSA_PRIV_ASN1_DER,
	/** a DER encoded BLISS public key */
	CRED_PART_BLISS_PUB_ASN1_DER,
	/** a DER encoded BLISS private key */
	CRED_PART_BLISS_PRIV_ASN1_DER,

	CRED_PART_END,
};

/**
 * Credential encoding and fingerprinting facility.
 */
struct cred_encoding_t {

	/**
	 * Encode a credential in a format using several parts, optional caching.
	 *
	 * The variable argument list takes cred_encoding_part_t, followed by part
	 * specific arguments, terminated by CRED_PART_END.
	 * If a cache key is given, the returned encoding points to internal data:
	 * do not free or modify. If no cache key is given, the encoding is
	 * allocated and must be freed by the caller.
	 *
	 * @param type			format the credential should be encoded to
	 * @param cache			key to use for caching, NULL to not cache
	 * @param encoding		encoding result, allocated if caching disabled
	 * @param ...			list of (cred_encoding_part_t, data)
	 * @return				TRUE if encoding successful
	 */
	bool (*encode)(cred_encoding_t *this, cred_encoding_type_t type, void *cache,
				   chunk_t *encoding, ...);

	/**
	 * Clear all cached encodings of a given cache key.
	 *
	 * @param cache			key used in encode() for caching
	 */
	void (*clear_cache)(cred_encoding_t *this, void *cache);

	/**
	 * Check for a cached encoding.
	 *
	 * @param type			format of the credential encoding
	 * @param cache			key to use for caching, as given to encode()
	 * @param encoding		encoding result, internal data
	 * @return				TRUE if cache entry found
	 */
	bool (*get_cache)(cred_encoding_t *this, cred_encoding_type_t type,
					  void *cache, chunk_t *encoding);

	/**
	 * Cache a credential encoding created externally.
	 *
	 * After calling cache(), the passed encoding is owned by the cred encoding
	 * facility.
	 *
	 * @param type			format of the credential encoding
	 * @param cache			key to use for caching, as given to encode()
	 * @param encoding		encoding to cache, gets owned by this
	 */
	void (*cache)(cred_encoding_t *this, cred_encoding_type_t type, void *cache,
				  chunk_t encoding);

	/**
	 * Register a credential encoder function.
	 *
	 * @param encoder		credential encoder function to add
	 */
	void (*add_encoder)(cred_encoding_t *this, cred_encoder_t encoder);

	/**
	 * Unregister a previously registered credential encoder function.
	 *
	 * @param encoder		credential encoder function to remove
	 */
	void (*remove_encoder)(cred_encoding_t *this, cred_encoder_t encoder);

	/**
	 * Destroy a cred_encoding_t.
	 */
	void (*destroy)(cred_encoding_t *this);
};

/**
 * Create a cred_encoding instance.
 */
cred_encoding_t *cred_encoding_create();

#endif /** CRED_ENCODING_H_ @}*/
