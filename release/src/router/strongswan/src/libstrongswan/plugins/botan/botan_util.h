/*
 * Copyright (C) 2018 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2018 René Korthaus
 * Rohde & Schwarz Cybersecurity GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @defgroup botan_util botan_util
 * @{ @ingroup botan_p
 */

#ifndef BOTAN_UTIL_H_
#define BOTAN_UTIL_H_

#include <library.h>

#include <botan/ffi.h>

/**
 * Converts chunk_t to botan_mp_t.
 *
 * @param value		chunk to convert
 * @param mp		allocated botan_mp_t
 * @return			TRUE if conversion successful
 */
bool chunk_to_botan_mp(chunk_t value, botan_mp_t *mp);

/**
 * Get the Botan string identifier for the given hash algorithm.
 *
 * @param hash		hash algorithm
 * @return			Botan string identifier, NULL if not found
 */
const char *botan_get_hash(hash_algorithm_t hash);

/**
 * Get the encoding of a botan_pubkey_t.
 *
 * @param pubkey	public key object
 * @param type		encoding type
 * @param encoding	allocated encoding
 * @return			TRUE if encoding successful
 */
bool botan_get_encoding(botan_pubkey_t pubkey, cred_encoding_type_t type,
						chunk_t *encoding);

/**
 * Get the encoding of a botan_privkey_t.
 *
 * @param key		private key object
 * @param type		encoding type
 * @param encoding	allocated encoding
 * @return			TRUE if encoding successful
 */
bool botan_get_privkey_encoding(botan_privkey_t key, cred_encoding_type_t type,
								chunk_t *encoding);

/**
 * Get the fingerprint of a botan_pubkey_t.
 *
 * @param pubkey	public key object
 * @param cache		key to use for caching, NULL to not cache
 * @param type		fingerprint type
 * @param fp		allocated fingerprint
 * @return			TRUE if fingerprinting successful
 */
bool botan_get_fingerprint(botan_pubkey_t pubkey, void *cache,
						   cred_encoding_type_t type, chunk_t *fp);

/**
 * Sign the given data using the provided key with the specified signature
 * scheme (hash/padding).
 *
 * @param key		private key object
 * @param scheme	hash/padding algorithm
 * @param data		data to sign
 * @param signature	allocated signature
 * @return			TRUE if signature successfully created
 */
bool botan_get_signature(botan_privkey_t key, const char *scheme,
						 chunk_t data, chunk_t *signature);

/**
 * Verify the given signature using the provided data and key with the specified
 * signature scheme (hash/padding).
 *
 * @param key		private key object
 * @param scheme	hash/padding algorithm
 * @param data		signed data
 * @param signature	signature to verify
 */
bool botan_verify_signature(botan_pubkey_t key, const char* scheme,
							chunk_t data, chunk_t signature);

/**
 * Do the Diffie-Hellman key derivation using the given private key and public
 * value.
 *
 * Note that the public value is not verified in this function.
 *
 * @param key		DH private key
 * @param pub		other's public value
 * @param secret	the derived secret (allocated on success)
 * @return			TRUE if derivation was successful
 */
bool botan_dh_key_derivation(botan_privkey_t key, chunk_t pub, chunk_t *secret);

#endif /** BOTAN_UTIL_H_ @}*/
