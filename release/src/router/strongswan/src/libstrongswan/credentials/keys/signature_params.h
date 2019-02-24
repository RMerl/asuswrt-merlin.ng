/*
 * Copyright (C) 2017-2018 Tobias Brunner
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
 * @defgroup signature_params signature_params
 * @{ @ingroup keys
 */

#ifndef SIGNATURE_PARAMS_H_
#define SIGNATURE_PARAMS_H_

typedef struct signature_params_t signature_params_t;
typedef struct rsa_pss_params_t rsa_pss_params_t;

#include <crypto/hashers/hasher.h>

/**
 * Signature scheme with parameters
 */
struct signature_params_t {
	/** Signature scheme */
	signature_scheme_t scheme;
	/** Parameters, depending on scheme */
	void *params;
};

/**
 * Compare two signature schemes and their parameters
 *
 * @param a			first scheme
 * @param b			second scheme
 * @return			TRUE if schemes and parameters are equal
 */
bool signature_params_equal(signature_params_t *a, signature_params_t *b);

/**
 * Compare two signature schemes and their parameters
 *
 * @param c			constraint
 * @param s			scheme
 * @return			TRUE if scheme complies to constraint
 */
bool signature_params_comply(signature_params_t *c, signature_params_t *s);

/**
 * Clone the given scheme and parameters, if any
 *
 * @return			cloned object
 */
signature_params_t *signature_params_clone(signature_params_t *this);

/**
 * Destroy the given scheme and parameters, if any
 */
void signature_params_destroy(signature_params_t *this);

/**
 * Clear the given parameters, if any, sets the scheme to SIGN_UNKNOWN
 */
void signature_params_clear(signature_params_t *this);

/**
 * Parse an ASN.1 algorithmIdentifier with parameters denoting a signature
 * scheme.
 *
 * @param asn1		ASN.1 encoded RSASSA-PSS-params
 * @param level0	current level of the ASN.1 parser
 * @param params	parsed parameters
 * @return			TRUE if successfully parsed
 */
bool signature_params_parse(chunk_t asn1, int level0,
							signature_params_t *params);

/**
 * Build ASN.1 algorithmIdentifier with parameters denoting a signature scheme.
 *
 * @param params	signature scheme and parameters to encode
 * @param asn1		ASN.1 encoded algorithmIdentifier (allocated)
 * @return			TRUE if successfully built
 */
bool signature_params_build(signature_params_t *params, chunk_t *asn1);

/**
 * Parameters for SIGN_RSA_EMSA_PSS signature scheme
 */
struct rsa_pss_params_t {
	/** Hash algorithm */
	hash_algorithm_t hash;
	/** Hash for the MGF1 function */
	hash_algorithm_t mgf1_hash;
	/** Salt length, use the constants below for special lengths resolved
	 * via rsa_pss_params_set_salt_len() */
	ssize_t salt_len;
	/** Salt value, for unit tests (not all implementations support this) */
	chunk_t salt;
/** Use a salt length equal to the length of the hash */
#define RSA_PSS_SALT_LEN_DEFAULT -1
/** Use the maximum salt length depending on the hash and key length */
#define RSA_PSS_SALT_LEN_MAX -2
};

/**
 * Parse the given ASN.1 algorithm identifier params
 *
 * @param asn1		ASN.1 encoded RSASSA-PSS-params
 * @param level0	current level of the ASN.1 parser
 * @param params	parsed parameters
 * @return			TRUE if successfully parsed
 */
bool rsa_pss_params_parse(chunk_t asn1, int level0, rsa_pss_params_t *params);

/**
 * Build ASN.1 algorithm identifier params
 *
 * @param params	parameters to encode
 * @param asn1		ASN.1 encoded RSASSA-PSS-params (allocated)
 * @return			TRUE if successfully built
 */
bool rsa_pss_params_build(rsa_pss_params_t *params, chunk_t *asn1);

/**
 * Determine and set the salt length for the given params in case constants
 * are used
 *
 * @param params	parameters to update
 * @param modbits	RSA modulus length in bits (required if RSA_PSS_SALT_LEN_MAX
 *					is used)
 * @return			salt length to use, negative on error
 */
bool rsa_pss_params_set_salt_len(rsa_pss_params_t *params, size_t modbits);

#endif /** SIGNATURE_PARAMS_H_ @}*/
