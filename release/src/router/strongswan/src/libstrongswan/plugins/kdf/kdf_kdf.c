/*
 * Copyright (C) 2022 Tobias Brunner
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

#include "kdf_kdf.h"

typedef struct private_kdf_t private_kdf_t;

/**
 * Private data.
 */
struct private_kdf_t {

	/**
	 * Public interface.
	 */
	kdf_t public;

	/**
	 * KDF type.
	 */
	key_derivation_function_t type;

	/**
	 * Underlying PRF.
	 */
	prf_t *prf;

	/**
	 * Salt value.
	 */
	chunk_t salt;
};

METHOD(kdf_t, get_type, key_derivation_function_t,
	private_kdf_t *this)
{
	return this->type;
}

METHOD(kdf_t, get_length, size_t,
	private_kdf_t *this)
{
	if (this->type == KDF_PRF_PLUS)
	{
		return SIZE_MAX;
	}
	return this->prf->get_block_size(this->prf);
}

METHOD(kdf_t, get_bytes_prf_plus, bool,
	private_kdf_t *this, size_t out_len, uint8_t *buffer)
{
	chunk_t block, previous = chunk_empty;
	uint8_t counter = 1, *out = buffer;
	size_t len;
	bool success = TRUE;

	block = chunk_alloca(this->prf->get_block_size(this->prf));
	if (out_len > block.len * 255)
	{
		return FALSE;
	}

	while (out_len)
	{
		if (!this->prf->get_bytes(this->prf, previous, NULL) ||
			!this->prf->get_bytes(this->prf, this->salt, NULL) ||
			!this->prf->get_bytes(this->prf, chunk_from_thing(counter),
								  block.ptr))
		{
			success = FALSE;
			break;
		}
		len = min(out_len, block.len);
		memcpy(out, block.ptr, len);
		previous = chunk_create(out, block.len);

		out_len -= len;
		out += len;
		counter++;
	}
	memwipe(block.ptr, block.len);
	return success;
}

METHOD(kdf_t, get_bytes, bool,
	private_kdf_t *this, size_t out_len, uint8_t *buffer)
{
	if (out_len != get_length(this))
	{
		return FALSE;
	}
	return this->prf->get_bytes(this->prf, this->salt, buffer);
}

METHOD(kdf_t, allocate_bytes, bool,
	private_kdf_t *this, size_t out_len, chunk_t *chunk)
{
	if (this->type == KDF_PRF)
	{
		out_len = out_len ?: get_length(this);
	}

	*chunk = chunk_alloc(out_len);

	if (!this->public.get_bytes(&this->public, out_len, chunk->ptr))
	{
		chunk_free(chunk);
		return FALSE;
	}
	return TRUE;
}

METHOD(kdf_t, set_param, bool,
	private_kdf_t *this, kdf_param_t param, ...)
{
	chunk_t chunk;
	bool success = FALSE;

	if (this->type == KDF_PRF)
	{	/* IKEv2 uses the nonces etc., which we receive as SALT, as PRF key and
		 * the DH secret as salt */
		switch (param)
		{
			case KDF_PARAM_KEY:
				param = KDF_PARAM_SALT;
				break;
			case KDF_PARAM_SALT:
				param = KDF_PARAM_KEY;
				break;
			default:
				break;
		}
	}

	switch (param)
	{
		case KDF_PARAM_KEY:
			VA_ARGS_GET(param, chunk);
			success = this->prf->set_key(this->prf, chunk);
			break;
		case KDF_PARAM_SALT:
			VA_ARGS_GET(param, chunk);
			chunk_clear(&this->salt);
			this->salt = chunk_clone(chunk);
			success = TRUE;
			break;
	}
	return success;
}

METHOD(kdf_t, destroy, void,
	private_kdf_t *this)
{
	this->prf->destroy(this->prf);
	chunk_clear(&this->salt);
	free(this);
}

/*
 * Described in header
 */
kdf_t *kdf_kdf_create(key_derivation_function_t algo, va_list args)
{
	private_kdf_t *this;
	pseudo_random_function_t prf_alg;
	prf_t *prf;

	if (algo != KDF_PRF && algo != KDF_PRF_PLUS)
	{
		return NULL;
	}

	VA_ARGS_VGET(args, prf_alg);
	prf = lib->crypto->create_prf(lib->crypto, prf_alg);
	if (!prf)
	{
		DBG1(DBG_LIB, "failed to create %N for %N",
			 pseudo_random_function_names, prf_alg,
			 key_derivation_function_names, algo);
		return NULL;
	}

	INIT(this,
		.public = {
			.get_type = _get_type,
			.get_length = _get_length,
			.get_bytes = _get_bytes,
			.allocate_bytes = _allocate_bytes,
			.set_param = _set_param,
			.destroy = _destroy,
		},
		.type = algo,
		.prf = prf,
	);

	if (algo == KDF_PRF_PLUS)
	{
		this->public.get_bytes = _get_bytes_prf_plus;
	}
	return &this->public;
}
