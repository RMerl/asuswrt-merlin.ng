/*
 * Copyright (C) 2018 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
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
 * @defgroup botan_ed_private_key botan_ed_private_key
 * @{ @ingroup botan_p
 */

#ifndef BOTAN_ED_PRIVATE_KEY_H_
#define BOTAN_ED_PRIVATE_KEY_H_

#include <botan/ffi.h>

#include <credentials/builder.h>
#include <credentials/keys/private_key.h>

/**
 * Generate an EdDSA private key using Botan.
 *
 * @param type		type of the key, must be KEY_ED25519
 * @param args		builder_part_t argument list
 * @return 			generated key, NULL on failure
 */
private_key_t *botan_ed_private_key_gen(key_type_t type, va_list args);

/**
 * Load an EdDSA private key using Botan.
 *
 * @param type		type of the key, must be KEY_ED25519
 * @param args		builder_part_t argument list
 * @return 			loaded key, NULL on failure
 */
private_key_t *botan_ed_private_key_load(key_type_t type, va_list args);

/**
 * Load an EdDSA private key by adopting a botan_privkey_t object.
 *
 * @param key		private key object (adopted)
 * @return			loaded key, NULL on failure
 */
private_key_t *botan_ed_private_key_adopt(botan_privkey_t key);

#endif /** BOTAN_ED_PRIVATE_KEY_H_ @}*/
