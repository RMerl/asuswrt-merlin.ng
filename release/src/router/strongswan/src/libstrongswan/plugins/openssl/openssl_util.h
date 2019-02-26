/*
 * Copyright (C) 2008 Tobias Brunner
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
 * @defgroup openssl_util openssl_util
 * @{ @ingroup openssl_p
 */

#ifndef OPENSSL_UTIL_H_
#define OPENSSL_UTIL_H_

#include <library.h>

#ifdef X509_NAME
/* from <wincrypt.h> */
# undef X509_NAME
#endif

#include <openssl/bn.h>
#include <openssl/asn1.h>

/**
 * Returns the length in bytes of a field element
 */
#define EC_FIELD_ELEMENT_LEN(group) ((EC_GROUP_get_degree(group) + 7) / 8)

/**
 * Creates a hash of a given type of a chunk of data.
 *
 * Note: this function allocates memory for the hash
 *
 * @param hash_type	NID of the hash
 * @param data		the chunk of data to hash
 * @param hash		chunk that contains the hash
 * @return			TRUE on success, FALSE otherwise
 */
bool openssl_hash_chunk(int hash_type, chunk_t data, chunk_t *hash);

/**
 * Concatenates two bignums into a chunk, thereby enfocing the length of
 * a single BIGNUM, if necessary, by pre-pending it with zeros.
 *
 * Note: this function allocates memory for the chunk
 *
 * @param len		the length of a single BIGNUM
 * @param a			first BIGNUM
 * @param b			second BIGNUM
 * @param chunk		resulting chunk
 * @return			TRUE on success, FALSE otherwise
 */
bool openssl_bn_cat(const int len, const BIGNUM *a, const BIGNUM *b,
					chunk_t *chunk);

/**
 * Splits a chunk into two bignums of equal binary length.
 *
 * @param chunk		a chunk that contains the two BIGNUMs
 * @param a			first BIGNUM
 * @param b			second BIGNUM
 * @return			TRUE on success, FALSE otherwise
 */
bool openssl_bn_split(chunk_t chunk, BIGNUM *a, BIGNUM *b);

/**
 * Exports the given bignum (assumed to be a positive number) to a chunk in
 * two's complement format (i.e. a zero byte is added if the MSB is set).
 *
 * @param bn		the BIGNUM to export
 * @param chunk		the chunk (data gets allocated)
 * @return			TRUE on success, FALSE otherwise
 */
bool openssl_bn2chunk(const BIGNUM *bn, chunk_t *chunk);

/**
 * Allocate a chunk using the i2d function of a given object
 *
 * @param type		type of the object
 * @param obj		object to convert to DER
 * @returns			allocated chunk of the object, or chunk_empty
 */
#define openssl_i2chunk(type, obj) ({ \
					unsigned char *ptr = NULL; \
					int len = i2d_##type(obj, &ptr); \
					len < 0 ? chunk_empty : chunk_create(ptr, len);})

/**
 * Convert an OpenSSL ASN1_OBJECT to a chunk.
 *
 * @param asn1		asn1 object to convert
 * @return			chunk, pointing into asn1 object
 */
chunk_t openssl_asn1_obj2chunk(ASN1_OBJECT *asn1);

/**
 * Convert an OpenSSL ASN1_STRING to a chunk.
 *
 * @param asn1		asn1 string to convert
 * @return			chunk, pointing into asn1 string
 */
chunk_t openssl_asn1_str2chunk(const ASN1_STRING *asn1);

/**
 * Convert an openssl X509_NAME to a identification_t of type ID_DER_ASN1_DN.
 *
 * @param name		name to convert
 * @return			identification_t, NULL on error
 */
identification_t *openssl_x509_name2id(X509_NAME *name);

/**
 * Check if an ASN1 oid is a an OID known by libstrongswan.
 *
 * @param obj		openssl ASN1 object
 * @returns			OID, as defined in <asn1/oid.h>
 */
int openssl_asn1_known_oid(ASN1_OBJECT *obj);

/**
 * Convert an OpenSSL ASN1_TIME to a time_t.
 *
 * @param time		openssl ASN1_TIME
 * @returns			time_t, 0 on error
 */
time_t openssl_asn1_to_time(const ASN1_TIME *time);

/**
 * Compatibility macros
 */
#ifdef OPENSSL_IS_BORINGSSL
#define EVP_PKEY_base_id(p) EVP_PKEY_type(p->type)
#endif

/**
 * Macros to define fallback getters/setters to access keys (BIGNUM*) for types
 * that were made opaque with OpenSSL 1.1.0.
 */
#define OPENSSL_KEY_FALLBACK(...) VA_ARGS_DISPATCH(OPENSSL_KEY_FALLBACK, __VA_ARGS__)(__VA_ARGS__)
#define OPENSSL_KEY_FALLBACK3(type, k1, k2) \
__attribute__((unused)) \
static inline void type##_get0(const type *o, const BIGNUM **k1, const BIGNUM **k2) { \
	if (k1) *k1 = o->k1; \
	if (k2) *k2 = o->k2; } \
__attribute__((unused)) \
static inline int type##_set0(type *o, BIGNUM *k1, BIGNUM *k2) { \
	if (k1) { BN_clear_free(o->k1); o->k1 = k1; } \
	if (k2) { BN_clear_free(o->k2); o->k2 = k2; } \
	return 1; }
#define OPENSSL_KEY_FALLBACK4(type, name, k1, k2) \
__attribute__((unused)) \
static inline void type##_get0_##name(const type *o, const BIGNUM **k1, const BIGNUM **k2) { \
	if (k1) *k1 = o->k1; \
	if (k2) *k2 = o->k2; } \
__attribute__((unused)) \
static inline int type##_set0_##name(type *o, BIGNUM *k1, BIGNUM *k2) { \
	if (k1) { BN_clear_free(o->k1); o->k1 = k1; } \
	if (k2) { BN_clear_free(o->k2); o->k2 = k2; } \
	return 1; }
#define OPENSSL_KEY_FALLBACK5(type, name, k1, k2, k3) \
__attribute__((unused)) \
static inline void type##_get0_##name(const type *o, const BIGNUM **k1, const BIGNUM **k2, const BIGNUM **k3) { \
	if (k1) *k1 = o->k1; \
	if (k2) *k2 = o->k2; \
	if (k3) *k3 = o->k3; } \
__attribute__((unused)) \
static inline int type##_set0_##name(type *o, BIGNUM *k1, BIGNUM *k2, BIGNUM *k3) { \
	if (k1) { BN_clear_free(o->k1); o->k1 = k1; } \
	if (k2) { BN_clear_free(o->k2); o->k2 = k2; } \
	if (k3) { BN_clear_free(o->k3); o->k3 = k3; } \
	return 1; }

#endif /** OPENSSL_UTIL_H_ @}*/
