/*
 * Copyright (C) 2020 Stefan Berghofer
 *
 * Copyright (C) secunet Security Networks AG
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
