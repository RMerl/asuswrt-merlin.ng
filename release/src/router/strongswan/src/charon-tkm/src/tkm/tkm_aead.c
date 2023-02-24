/*
 * Copyright (C) 2020 Tobias Brunner
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

#include <errno.h>

#include <crypto/iv/iv_gen.h>
#include <tkm/constants.h>
#include <tkm/client.h>

#include "tkm_aead.h"
#include "tkm_utils.h"

typedef struct private_aead_t private_aead_t;

/**
 * AEAD implementation using TKM
 */
struct private_aead_t {

	/**
	 * Public interface
	 * */
	aead_t public;

	/**
	 * Internal IV generator for TKM
	 */
	iv_gen_t iv_gen;

	/**
	 * ISA context id
	 */
	isa_id_type isa_ctx_id;

	/**
	 * Block length of encryption algorithm
	 */
	block_len_type block_len;

	/**
	 * Length of integrity check value
	 */
	icv_len_type icv_len;

	/**
	 * Length of initialization vector
	 */
	iv_len_type iv_len;
};

METHOD(iv_gen_t, get_iv, bool,
	iv_gen_t *this, uint64_t seq, size_t size, uint8_t *buffer)
{
	return TRUE;
}

METHOD(iv_gen_t, allocate_iv, bool,
	iv_gen_t *this, uint64_t seq, size_t size, chunk_t *chunk)
{
	*chunk = chunk_alloc(size);
	return get_iv(this, seq, chunk->len, chunk->ptr);
}

METHOD(aead_t, encrypt, bool,
	private_aead_t *this, chunk_t plain, chunk_t assoc,
	chunk_t iv, chunk_t *encrypted)
{
	aad_plain_type aad_plain;
	iv_encrypted_icv_type iv_encrypted_icv;
	result_type res;

	aad_plain = (aad_plain_type){
		.size = assoc.len + plain.len,
	};
	if (aad_plain.size > sizeof(aad_plain.data))
	{
		DBG1(DBG_IKE, "%u exceeds buffer size %u, encryption failed (isa: "
			 "%llu)", aad_plain.size, sizeof(aad_plain.data), this->isa_ctx_id);
		return FALSE;
	}
	memcpy(aad_plain.data, assoc.ptr, assoc.len);
	memcpy(aad_plain.data + assoc.len, plain.ptr, plain.len);

	res = ike_isa_encrypt(this->isa_ctx_id, assoc.len, aad_plain,
						  &iv_encrypted_icv);
	if (res != TKM_OK)
	{
		DBG1(DBG_IKE, "encryption failed (isa: %llu)", this->isa_ctx_id);
		return FALSE;
	}

	if (encrypted)
	{
		sequence_to_chunk(iv_encrypted_icv.data, iv_encrypted_icv.size,
						  encrypted);
	}
	else
	{
		memcpy(plain.ptr, iv_encrypted_icv.data + iv.len,
			   iv_encrypted_icv.size - iv.len);
	}
	memcpy(iv.ptr, iv_encrypted_icv.data, iv.len);
	return TRUE;
}

METHOD(aead_t, decrypt, bool,
	private_aead_t *this, chunk_t encrypted, chunk_t assoc, chunk_t iv,
	chunk_t *plain)
{
	aad_iv_encrypted_icv_type aad_iv_encrypted_icv;
	decrypted_type decrypted;
	result_type res;

	aad_iv_encrypted_icv = (aad_iv_encrypted_icv_type){
		.size = assoc.len + iv.len + encrypted.len,
	};
	if (aad_iv_encrypted_icv.size > sizeof(aad_iv_encrypted_icv.data))
	{
		DBG1(DBG_IKE, "%u exceeds buffer size %u, decryption failed (isa: "
			 "%llu)", aad_iv_encrypted_icv.size,
			 sizeof(aad_iv_encrypted_icv.data), this->isa_ctx_id);
		return FALSE;
	}
	memcpy(aad_iv_encrypted_icv.data, assoc.ptr, assoc.len);
	memcpy(aad_iv_encrypted_icv.data + assoc.len, iv.ptr, iv.len);
	memcpy(aad_iv_encrypted_icv.data + assoc.len + iv.len, encrypted.ptr,
		   encrypted.len);

	res = ike_isa_decrypt(this->isa_ctx_id, assoc.len, aad_iv_encrypted_icv,
						  &decrypted);
	if (res != TKM_OK)
	{
		DBG1(DBG_IKE, "decryption failed (isa: %llu)", this->isa_ctx_id);
		return FALSE;
	}

	if (plain)
	{
		sequence_to_chunk(decrypted.data, decrypted.size, plain);
	}
	else
	{
		memcpy(encrypted.ptr, decrypted.data, decrypted.size);
	}
	return TRUE;
}

METHOD(aead_t, get_block_size, size_t,
	private_aead_t *this)
{
	return this->block_len;
}

METHOD(aead_t, get_icv_size, size_t,
	private_aead_t *this)
{
	return this->icv_len;
}

METHOD(aead_t, get_iv_size, size_t,
	private_aead_t *this)
{
	return this->iv_len;
}

METHOD(aead_t, get_iv_gen, iv_gen_t*,
	private_aead_t *this)
{
	return &this->iv_gen;
}

METHOD(aead_t, get_key_size, size_t,
	private_aead_t *this)
{
	return 1;
}

METHOD(aead_t, set_key, bool,
	private_aead_t *this, chunk_t key)
{
	return TRUE;
}

METHOD(aead_t, destroy, void,
	private_aead_t *this)
{
	free(this);
}

/*
 * Described in header
 */
aead_t *tkm_aead_create(isa_id_type isa_ctx_id, block_len_type block_len,
						icv_len_type icv_len, iv_len_type iv_len)
{
	private_aead_t *aead;

	INIT(aead,
		.public = {
			.encrypt = _encrypt,
			.decrypt = _decrypt,
			.get_block_size = _get_block_size,
			.get_icv_size = _get_icv_size,
			.get_iv_size = _get_iv_size,
			.get_iv_gen = _get_iv_gen,
			.get_key_size = _get_key_size,
			.set_key = _set_key,
			.destroy = _destroy,
		},
		.iv_gen = {
			.get_iv = _get_iv,
			.allocate_iv = _allocate_iv,
			.destroy = (void *)nop,
		},
		.isa_ctx_id = isa_ctx_id,
		.block_len = block_len,
		.icv_len = icv_len,
		.iv_len = iv_len,
	);

	return &aead->public;
}
