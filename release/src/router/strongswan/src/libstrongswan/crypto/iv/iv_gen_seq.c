/*
 * Copyright (C) 2013 Tobias Brunner
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

#include "iv_gen_seq.h"

/**
 * Magic value for the initial IV state
 */
#define SEQ_IV_INIT_STATE (~(uint64_t)0)
#define SEQ_IV_HIGH_MASK (1ULL << 63)

typedef struct private_iv_gen_t private_iv_gen_t;

/**
 * Private data of an iv_gen_t object.
 */
struct private_iv_gen_t {

	/**
	 * Public iv_gen_t interface.
	 */
	iv_gen_t public;

	/**
	 * Previously passed sequence number in lower space to enforce uniqueness
	 */
	uint64_t prevl;

	/**
	 * Previously passed sequence number in upper space to enforce uniqueness
	 */
	uint64_t prevh;

	/**
	 * Salt to mask counter
	 */
	uint8_t *salt;
};

METHOD(iv_gen_t, get_iv, bool,
	private_iv_gen_t *this, uint64_t seq, size_t size, uint8_t *buffer)
{
	uint8_t iv[sizeof(uint64_t)];
	size_t len = size;

	if (!this->salt)
	{
		return FALSE;
	}
	if (size < sizeof(uint64_t))
	{
		return FALSE;
	}
	if (this->prevl != SEQ_IV_INIT_STATE && seq <= this->prevl)
	{
		seq |= SEQ_IV_HIGH_MASK;
		if (this->prevh != SEQ_IV_INIT_STATE && seq <= this->prevh)
		{
			return FALSE;
		}
	}
	if ((seq | SEQ_IV_HIGH_MASK) == SEQ_IV_INIT_STATE)
	{
		return FALSE;
	}
	if (seq & SEQ_IV_HIGH_MASK)
	{
		this->prevh = seq;
	}
	else
	{
		this->prevl = seq;
	}
	if (len > sizeof(uint64_t))
	{
		len = sizeof(uint64_t);
		memset(buffer, 0, size - len);
	}
	htoun64(iv, seq);
	memxor(iv, this->salt, sizeof(uint64_t));
	memcpy(buffer + size - len, iv + sizeof(uint64_t) - len, len);
	return TRUE;
}

METHOD(iv_gen_t, allocate_iv, bool,
	private_iv_gen_t *this, uint64_t seq, size_t size, chunk_t *chunk)
{
	*chunk = chunk_alloc(size);
	if (!get_iv(this, seq, chunk->len, chunk->ptr))
	{
		chunk_free(chunk);
		return FALSE;
	}
	return TRUE;
}

METHOD(iv_gen_t, destroy, void,
	private_iv_gen_t *this)
{
	free(this->salt);
	free(this);
}

iv_gen_t *iv_gen_seq_create()
{
	private_iv_gen_t *this;
	rng_t *rng;

	INIT(this,
		.public = {
			.get_iv = _get_iv,
			.allocate_iv = _allocate_iv,
			.destroy = _destroy,
		},
		.prevl = SEQ_IV_INIT_STATE,
		.prevh = SEQ_IV_INIT_STATE,
	);

	rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
	if (rng)
	{
		this->salt = malloc(sizeof(uint64_t));
		if (!rng->get_bytes(rng, sizeof(uint64_t), this->salt))
		{
			free(this->salt);
			this->salt = NULL;
		}
		rng->destroy(rng);
	}

	return &this->public;
}
