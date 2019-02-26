/*
 * Copyright (C) 2016 Andreas Steffen
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

#include "chapoly_xof.h"
#include "chapoly_drv.h"

typedef struct private_chapoly_xof_t private_chapoly_xof_t;

/**
 * Private data of an chapoly_xof_t object.
 */
struct private_chapoly_xof_t {

	/**
	 * Public chapoly_xof_t interface.
	 */
	chapoly_xof_t public;

	/**
	 * Latest block of the ChaCha20 stream.
	 */
	uint8_t stream[CHACHA_BLOCK_SIZE];

	/**
	 * Index pointing to the current position in the stream
	 */
	u_int stream_index;

	/**
	 * Driver backend
	 */
	chapoly_drv_t *drv;
};

METHOD(xof_t, get_type, ext_out_function_t,
	private_chapoly_xof_t *this)
{
	return XOF_CHACHA20;
}

METHOD(xof_t, get_bytes, bool,
	private_chapoly_xof_t *this, size_t out_len, uint8_t *buffer)
{
	size_t index = 0, len, blocks;

	/* empty the stream buffer first */
	len = min(out_len, CHACHA_BLOCK_SIZE - this->stream_index);
	if (len)
	{
		memcpy(buffer, this->stream + this->stream_index, len);
		index += len;
		this->stream_index += len;
	}

	/* copy whole stream blocks directly to output buffer */
	blocks = (out_len - index) / CHACHA_BLOCK_SIZE;	
	while (blocks--)
	{
		if (!this->drv->chacha(this->drv, buffer + index))
		{
			return FALSE;
		}
		index += CHACHA_BLOCK_SIZE;
	}	
	
	/* refill the stream buffer if some more output bytes are needed */
	len = out_len - index;
	if (len)
	{
		if (!this->drv->chacha(this->drv, this->stream))
		{
			return FALSE;
		}
		memcpy(buffer + index, this->stream, len);
		this->stream_index = len;
	}		
	
	return TRUE;
}

METHOD(xof_t, allocate_bytes, bool,
	private_chapoly_xof_t *this, size_t out_len, chunk_t *chunk)
{
	*chunk = chunk_alloc(out_len);

	if (!get_bytes(this, out_len, chunk->ptr))
	{
		chunk_free(chunk);
		return FALSE;
	}

	return TRUE;
}

METHOD(xof_t, get_block_size, size_t,
	private_chapoly_xof_t *this)
{
	return CHACHA_BLOCK_SIZE;
}

METHOD(xof_t, get_seed_size, size_t,
	private_chapoly_xof_t *this)
{
	return CHACHA_KEY_SIZE + CHACHA_SALT_SIZE + CHACHA_IV_SIZE;
}

METHOD(xof_t, set_seed, bool,
	private_chapoly_xof_t *this, chunk_t seed)
{
	this->stream_index = CHACHA_BLOCK_SIZE;

	return  seed.len == get_seed_size(this) &&
			this->drv->set_key(this->drv, "expand 32-byte k",
							seed.ptr, seed.ptr + CHACHA_KEY_SIZE) &&
			this->drv->init(this->drv,
						 	seed.ptr + CHACHA_KEY_SIZE + CHACHA_SALT_SIZE);
}

METHOD(xof_t, destroy, void,
	private_chapoly_xof_t *this)
{
	this->drv->destroy(this->drv);
	free(this);
}

/**
 * See header
 */
chapoly_xof_t *chapoly_xof_create(ext_out_function_t algorithm)
{
	private_chapoly_xof_t *this;
	chapoly_drv_t *drv;

	if (algorithm != XOF_CHACHA20)
	{
		return NULL;
	}

	drv = chapoly_drv_probe();
	if (!drv)
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.xof_interface = {
				.get_type = _get_type,
				.get_bytes = _get_bytes,
				.allocate_bytes = _allocate_bytes,
				.get_block_size = _get_block_size,
				.get_seed_size = _get_seed_size,
				.set_seed = _set_seed,
				.destroy = _destroy,
			},
		},
		.drv = drv,
	);

	return &this->public;
}
