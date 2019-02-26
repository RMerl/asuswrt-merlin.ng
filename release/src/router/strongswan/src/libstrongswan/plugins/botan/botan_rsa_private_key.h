/*
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
 * @defgroup botan_rsa_private_key botan_rsa_private_key
 * @{ @ingroup botan_p
 */

#ifndef BOTAN_RSA_PRIVATE_KEY_H_
#define BOTAN_RSA_PRIVATE_KEY_H_

#include <botan/ffi.h>

#include <credentials/builder.h>
#include <credentials/keys/private_key.h>

typedef struct botan_rsa_private_key_t botan_rsa_private_key_t;

/**
 * private_key_t implementation of RSA algorithm using Botan.
 */
struct botan_rsa_private_key_t {

	/**
	 * Implements private_key_t interface
	 */
	private_key_t key;
};

/**
 * Generate a RSA private key using Botan.
 *
 * Accepts the BUILD_KEY_SIZE argument.
 *
 * @param type		type of the key, must be KEY_RSA
 * @param args		builder_part_t argument list
 * @return 			generated key, NULL on failure
 */
botan_rsa_private_key_t *botan_rsa_private_key_gen(key_type_t type,
												   va_list args);

/**
 * Load a RSA private key using Botan.
 *
 * Accepts a BUILD_BLOB_ASN1_DER argument.
 *
 * @param type		type of the key, must be KEY_RSA
 * @param args		builder_part_t argument list
 * @return			loaded key, NULL on failure
 */
botan_rsa_private_key_t *botan_rsa_private_key_load(key_type_t type,
													va_list args);

/**
 * Load a RSA private key by adopting a botan_privkey_t object.
 *
 * @param key		private key object (adopted)
 * @return			loaded key, NULL on failure
 */
botan_rsa_private_key_t *botan_rsa_private_key_adopt(botan_privkey_t key);

#endif /** BOTAN_RSA_PRIVATE_KEY_H_ @}*/
