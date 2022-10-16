/*
 * Copyright (C) 2020 secunet Security Networks AG
 * Copyright (C) 2020 Stefan Berghofer
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
 * @defgroup tkm-aead aead
 * @{ @ingroup tkm
 */

#ifndef TKM_AEAD_H_
#define TKM_AEAD_H_

typedef struct tkm_aead_t tkm_aead_t;

#include <crypto/aead.h>
#include <tkm/types.h>

/**
 * Create an AEAD implementation providing encryption and integrity protection
 * using TKM.
 *
 * @param isa_ctx_id		id of ISA context to use for encryption/decryption
 * @param block_len			block length of encryption algorithm
 * @param icv_len			length of integrity check value
 * @param iv_len			length of initialization vector
 * @return					created aead_t object
 */
aead_t *tkm_aead_create(isa_id_type isa_ctx_id, block_len_type block_len,
						icv_len_type icv_len, iv_len_type iv_len);

#endif /** TKM_AEAD_H_ @}*/
