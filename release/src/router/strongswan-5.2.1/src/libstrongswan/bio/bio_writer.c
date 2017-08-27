/*
 * Copyright (C) 2012-2013 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include "bio_writer.h"

typedef struct private_bio_writer_t private_bio_writer_t;

/**
 * Private data of an bio_writer_t object.
 */
struct private_bio_writer_t {

	/**
	 * Public bio_writer_t interface.
	 */
	bio_writer_t public;

	/**
	 * Allocated buffer
	 */
	chunk_t buf;

	/**
	 * Used bytes in buffer
	 */
	size_t used;

	/**
	 * Number of bytes to increase buffer size
	 */
	size_t increase;
};

/**
 * Increase buffer size, if required
 */
static inline void increase(private_bio_writer_t *this, size_t required)
{
	bool inc = FALSE;

	while (this->used + required > this->buf.len)
	{
		this->buf.len += this->increase;
		inc = TRUE;
	}
	if (inc)
	{
		this->buf.ptr = realloc(this->buf.ptr, this->buf.len);
	}
}

METHOD(bio_writer_t, write_uint8, void,
	private_bio_writer_t *this, u_int8_t value)
{
	increase(this, 1);
	this->buf.ptr[this->used] = value;
	this->used += 1;
}

METHOD(bio_writer_t, write_uint16, void,
	private_bio_writer_t *this, u_int16_t value)
{
	increase(this, 2);
	htoun16(this->buf.ptr + this->used, value);
	this->used += 2;
}

METHOD(bio_writer_t, write_uint24, void,
	private_bio_writer_t *this, u_int32_t value)
{
	increase(this, 3);
	value = htonl(value);
	memcpy(this->buf.ptr + this->used, ((char*)&value) + 1, 3);
	this->used += 3;
}

METHOD(bio_writer_t, write_uint32, void,
	private_bio_writer_t *this, u_int32_t value)
{
	increase(this, 4);
	htoun32(this->buf.ptr + this->used, value);
	this->used += 4;
}

METHOD(bio_writer_t, write_uint64, void,
	private_bio_writer_t *this, u_int64_t value)
{
	increase(this, 8);
	htoun64(this->buf.ptr + this->used, value);
	this->used += 8;
}

METHOD(bio_writer_t, write_data, void,
	private_bio_writer_t *this, chunk_t value)
{
	increase(this, value.len);
	memcpy(this->buf.ptr + this->used, value.ptr, value.len);
	this->used += value.len;
}

METHOD(bio_writer_t, write_data8, void,
	private_bio_writer_t *this, chunk_t value)
{
	increase(this, 1 + value.len);
	write_uint8(this, value.len);
	write_data(this, value);
}

METHOD(bio_writer_t, write_data16, void,
	private_bio_writer_t *this, chunk_t value)
{
	increase(this, 2 + value.len);
	write_uint16(this, value.len);
	write_data(this, value);
}

METHOD(bio_writer_t, write_data24, void,
	private_bio_writer_t *this, chunk_t value)
{
	increase(this, 3 + value.len);
	write_uint24(this, value.len);
	write_data(this, value);
}

METHOD(bio_writer_t, write_data32, void,
	private_bio_writer_t *this, chunk_t value)
{
	increase(this, 4 + value.len);
	write_uint32(this, value.len);
	write_data(this, value);
}

METHOD(bio_writer_t, wrap8, void,
	private_bio_writer_t *this)
{
	increase(this, 1);
	memmove(this->buf.ptr + 1, this->buf.ptr, this->used);
	this->buf.ptr[0] = this->used;
	this->used += 1;
}

METHOD(bio_writer_t, wrap16, void,
	private_bio_writer_t *this)
{
	increase(this, 2);
	memmove(this->buf.ptr + 2, this->buf.ptr, this->used);
	htoun16(this->buf.ptr, this->used);
	this->used += 2;
}

METHOD(bio_writer_t, wrap24, void,
	private_bio_writer_t *this)
{
	u_int32_t len;

	increase(this, 3);
	memmove(this->buf.ptr + 3, this->buf.ptr, this->used);

	len = htonl(this->used);
	memcpy(this->buf.ptr, ((char*)&len) + 1, 3);
	this->used += 3;
}

METHOD(bio_writer_t, wrap32, void,
	private_bio_writer_t *this)
{
	increase(this, 4);
	memmove(this->buf.ptr + 4, this->buf.ptr, this->used);
	htoun32(this->buf.ptr, this->used);
	this->used += 4;
}

METHOD(bio_writer_t, skip, chunk_t,
	private_bio_writer_t *this, size_t len)
{
	chunk_t skipped;

	increase(this, len);
	skipped = chunk_create(this->buf.ptr + this->used, len);
	this->used += len;
	return skipped;
}

METHOD(bio_writer_t, get_buf, chunk_t,
	private_bio_writer_t *this)
{
	return chunk_create(this->buf.ptr, this->used);
}

METHOD(bio_writer_t, extract_buf, chunk_t,
	private_bio_writer_t *this)
{
	chunk_t buf = get_buf(this);
	this->buf = chunk_empty;
	this->used = 0;
	return buf;
}

METHOD(bio_writer_t, destroy, void,
	private_bio_writer_t *this)
{
	free(this->buf.ptr);
	free(this);
}

/**
 * See header
 */
bio_writer_t *bio_writer_create(u_int32_t bufsize)
{
	private_bio_writer_t *this;

	INIT(this,
		.public = {
			.write_uint8 = _write_uint8,
			.write_uint16 = _write_uint16,
			.write_uint24 = _write_uint24,
			.write_uint32 = _write_uint32,
			.write_uint64 = _write_uint64,
			.write_data = _write_data,
			.write_data8 = _write_data8,
			.write_data16 = _write_data16,
			.write_data24 = _write_data24,
			.write_data32 = _write_data32,
			.wrap8 = _wrap8,
			.wrap16 = _wrap16,
			.wrap24 = _wrap24,
			.wrap32 = _wrap32,
			.skip = _skip,
			.get_buf = _get_buf,
			.extract_buf = _extract_buf,
			.destroy = _destroy,
		},
		.increase = bufsize ? max(bufsize, 4) : 32,
	);
	if (bufsize)
	{
		this->buf = chunk_alloc(bufsize);
	}

	return &this->public;
}
