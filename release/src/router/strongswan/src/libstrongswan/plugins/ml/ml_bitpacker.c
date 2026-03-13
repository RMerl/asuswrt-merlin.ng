/*
 * Copyright (C) 2024 Tobias Brunner
 * Copyright (C) 2014 Andreas Steffen
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

#include "ml_bitpacker.h"
#include "ml_utils.h"

typedef struct private_ml_bitpacker_t private_ml_bitpacker_t;

/**
 * Private data.
 */
struct private_ml_bitpacker_t {

	/**
	 * Public interface.
	 */
	ml_bitpacker_t public;

	/**
	 * Bit buffer for up to 32 bits.
	 */
	uint32_t bits_buf;

	/**
	 * Bits left in the bit buffer.
	 */
	size_t bits_left;

	/**
	 * Target buffer.
	 */
	chunk_t buf;

	/**
	 * Read/Write pointer into buffer.
	 */
	chunk_t pos;
};

/**
 * Write the bytes in the bit buffer to the output buffer.
 */
static void flush_buffer(private_ml_bitpacker_t *this)
{
	size_t to_write = min(4, this->pos.len);

	ml_write_bytes_le(this->pos.ptr, to_write, this->bits_buf);
	this->pos = chunk_skip(this->pos, to_write);
}

METHOD(ml_bitpacker_t, write_bits, bool,
	private_ml_bitpacker_t *this, uint32_t value, size_t bits)
{
	if (!bits)
	{
		return TRUE;
	}
	if (bits > 32)
	{
		return FALSE;
	}
	if (bits < 32)
	{
		value &= (1 << bits) - 1;
	}

	while (TRUE)
	{
		if (!this->pos.len)
		{
			return FALSE;
		}

		this->bits_buf |= value << (32 - this->bits_left);

		if (bits < this->bits_left)
		{
			this->bits_left -= bits;
			return TRUE;
		}
		value >>= this->bits_left;
		bits -= this->bits_left;

		flush_buffer(this);
		this->bits_buf = 0;
		this->bits_left = 32;
	}
}

METHOD(ml_bitpacker_t, read_bits, bool,
	private_ml_bitpacker_t *this, uint32_t *value, size_t bits)
{
	size_t to_read, written = 0;

	if (bits > 32)
	{
		return FALSE;
	}
	*value = 0;

	while (TRUE)
	{
		if (!this->bits_left)
		{
			if (!this->pos.len)
			{
				return FALSE;
			}
			to_read = min(4, this->pos.len);
			this->bits_buf = ml_read_bytes_le(this->pos.ptr, to_read);
			this->pos = chunk_skip(this->pos, to_read);
			this->bits_left = 8 * to_read;
		}
		if (bits <= this->bits_left)
		{
			*value |= (this->bits_buf & ((1 << bits) - 1)) << written;
			this->bits_buf >>= bits;
			this->bits_left -= bits;
			return TRUE;
		}
		*value |= this->bits_buf;
		written = this->bits_left;
		bits -= this->bits_left;
		this->bits_left = 0;
	}
}

METHOD(ml_bitpacker_t, destroy, void,
	private_ml_bitpacker_t *this)
{
	if (this->public.write_bits == _write_bits &&
		this->bits_left < 32)
	{
		flush_buffer(this);
	}
	free(this);
}

/*
 * Described in header
 */
ml_bitpacker_t *ml_bitpacker_create(chunk_t dst)
{
	private_ml_bitpacker_t *this;

	INIT(this,
		.public = {
			.write_bits = _write_bits,
			.read_bits = (void*)return_false,
			.destroy = _destroy,
		},
		.bits_left = 32,
		.buf = dst,
	);

	this->pos = this->buf;

	return &this->public;
}

/*
 * Described in header
 */
ml_bitpacker_t *ml_bitpacker_create_from_data(chunk_t data)
{
	private_ml_bitpacker_t *this;

	INIT(this,
		.public = {
			.write_bits = (void*)return_false,
			.read_bits = _read_bits,
			.destroy = _destroy,
		},
		.buf = data,
	);

	this->pos = this->buf;

	return &this->public;
}
