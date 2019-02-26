/*
 * Copyright (C) 2014 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY;https://www.hsr.ch/HSR-intern-Anmeldung.4409.0.html?&no_cache=1 without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "bliss_bitpacker.h"

typedef struct private_bliss_bitpacker_t private_bliss_bitpacker_t;

/**
 * Private data structure for bliss_bitpacker_t object
 */
struct private_bliss_bitpacker_t {
	/**
	 * Public interface.
	 */
	bliss_bitpacker_t public;

	/**
	 * Current number of bits written to buffer
	 */
	size_t bits;

	/**
	 * Bit buffer for up to 32 bits
	 */
	uint32_t bits_buf;

	/**
	 * Bits left in the bit buffer
	 */
	size_t bits_left;

	/**
	 * Buffer
	 */
	chunk_t buf;

	/**
	 * Read/Write pointer into buffer
	 */
	chunk_t pos;

};

METHOD(bliss_bitpacker_t, get_bits, size_t,
	private_bliss_bitpacker_t *this)
{
	return this->bits;
}

METHOD(bliss_bitpacker_t, write_bits, bool,
	private_bliss_bitpacker_t *this, uint32_t value, size_t bits)
{
	if (bits == 0)
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
	this->bits += bits;

	while (TRUE)
	{
		if (bits <= this->bits_left)
		{
			this->bits_buf |= value << (this->bits_left - bits);
			this->bits_left -= bits;
			return TRUE;
		}

		this->bits_buf |= value >> (bits - this->bits_left);
		value &= (1 << (bits - this->bits_left)) - 1;
		bits -= this->bits_left;

		if (this->pos.len < 8)
		{
			return FALSE;
		}
		htoun32(this->pos.ptr, this->bits_buf);
		this->pos = chunk_skip(this->pos, 4);
		this->bits_buf = 0;
		this->bits_left = 32;
	}
}

METHOD(bliss_bitpacker_t, read_bits, bool,
	private_bliss_bitpacker_t *this, uint32_t *value, size_t bits)
{
	if (bits > 32)
	{
		return FALSE;
	}
	*value = 0;

	while (TRUE)
	{
		if (this->bits_left == 0)
		{
			if (this->pos.len < 4)
			{
				return FALSE;
			}
			this->bits_buf = untoh32(this->pos.ptr);
			this->pos = chunk_skip(this->pos, 4);
			this->bits_left = 32;
		}
		if (bits <= this->bits_left)
		{
			*value |= this->bits_buf >> (this->bits_left - bits);
			this->bits_buf &= (1 << (this->bits_left - bits)) - 1;
			this->bits_left -= bits;

			return TRUE;
		}
		*value |= this->bits_buf << (bits - this->bits_left);
		bits -= this->bits_left;
		this->bits_left = 0;
	}
}

METHOD(bliss_bitpacker_t, extract_buf, chunk_t,
	private_bliss_bitpacker_t *this)
{
	chunk_t buf;

	htoun32(this->pos.ptr, this->bits_buf);
	this->pos.len -= 4;
	buf = this->buf;
	buf.len = this->buf.len - this->pos.len - this->bits_left/8;
	this->buf = this->pos = chunk_empty;

	return buf;
}

METHOD(bliss_bitpacker_t, destroy, void,
	private_bliss_bitpacker_t *this)
{
	free(this->buf.ptr);
	free(this);
}

/**
 * See header.
 */
bliss_bitpacker_t *bliss_bitpacker_create(uint16_t max_bits)
{
	private_bliss_bitpacker_t *this;

	INIT(this,
		.public = {
			.get_bits = _get_bits,
			.write_bits = _write_bits,
			.read_bits = _read_bits,
			.extract_buf = _extract_buf,
			.destroy = _destroy,
		},
		.bits_left = 32,
		.buf = chunk_alloc(round_up(max_bits, 32)/8),
	);

	this->pos = this->buf;

	return &this->public;
}

/**
 * See header.
 */
bliss_bitpacker_t *bliss_bitpacker_create_from_data(chunk_t data)
{
	private_bliss_bitpacker_t *this;

	INIT(this,
		.public = {
			.get_bits = _get_bits,
			.write_bits = _write_bits,
			.read_bits = _read_bits,
			.extract_buf = _extract_buf,
			.destroy = _destroy,
		},
		.bits = 8 * data.len,
		.buf = chunk_alloc(round_up(data.len, 4)),
	);

	memset(this->buf.ptr + this->buf.len - 4, 0x00, 4);
	memcpy(this->buf.ptr, data.ptr, data.len);
	this->pos = this->buf;

	return &this->public;
}
