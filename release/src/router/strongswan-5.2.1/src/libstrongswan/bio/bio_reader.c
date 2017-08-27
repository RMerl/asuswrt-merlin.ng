/*
 * Copyright (C) 2012 Tobias Brunner
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

#include "bio_reader.h"

#include <utils/debug.h>

typedef struct private_bio_reader_t private_bio_reader_t;

/**
 * Private data of an bio_reader_t object.
 */
struct private_bio_reader_t {

	/**
	 * Public bio_reader_t interface.
	 */
	bio_reader_t public;

	/**
	 * Remaining data to process
	 */
	chunk_t buf;

	/**
	 * Optional data to free during destruction
	 */
	chunk_t cleanup;
};

METHOD(bio_reader_t, remaining, u_int32_t,
	private_bio_reader_t *this)
{
	return this->buf.len;
}

METHOD(bio_reader_t, peek, chunk_t,
	private_bio_reader_t *this)
{
	return this->buf;
}

/**
 * A version of chunk_skip() that supports skipping from the end (i.e. simply
 * reducing the size)
 */
static inline chunk_t chunk_skip_end(chunk_t chunk, size_t bytes, bool from_end)
{
	if (chunk.len > bytes)
	{
		if (!from_end)
		{
			chunk.ptr += bytes;
		}
		chunk.len -= bytes;
		return chunk;
	}
	return chunk_empty;
}

/**
 * Returns a pointer to the data to read, optionally from the end
 */
static inline u_char *get_ptr_end(private_bio_reader_t *this, u_int32_t len,
								  bool from_end)
{
	return from_end ? this->buf.ptr + (this->buf.len - len) : this->buf.ptr;
}

/**
 * Read an u_int8_t from the buffer, optionally from the end of the buffer
 */
static bool read_uint8_internal(private_bio_reader_t *this, u_int8_t *res,
								bool from_end)
{
	if (this->buf.len < 1)
	{
		DBG1(DBG_LIB, "%d bytes insufficient to parse u_int8 data",
			 this->buf.len);
		return FALSE;
	}
	*res = *get_ptr_end(this, 1, from_end);
	this->buf = chunk_skip_end(this->buf, 1, from_end);
	return TRUE;
}

/**
 * Read an u_int16_t from the buffer, optionally from the end
 */
static bool read_uint16_internal(private_bio_reader_t *this, u_int16_t *res,
								 bool from_end)
{
	if (this->buf.len < 2)
	{
		DBG1(DBG_LIB, "%d bytes insufficient to parse u_int16 data",
			 this->buf.len);
		return FALSE;
	}
	*res = untoh16(get_ptr_end(this, 2, from_end));
	this->buf = chunk_skip_end(this->buf, 2, from_end);
	return TRUE;
}

/**
 * Read an u_int32_t (only 24-bit) from the buffer, optionally from the end
 */
static bool read_uint24_internal(private_bio_reader_t *this, u_int32_t *res,
								 bool from_end)
{
	if (this->buf.len < 3)
	{
		DBG1(DBG_LIB, "%d bytes insufficient to parse u_int24 data",
			 this->buf.len);
		return FALSE;
	}
	*res = untoh32(get_ptr_end(this, 3, from_end)) >> 8;
	this->buf = chunk_skip_end(this->buf, 3, from_end);
	return TRUE;
}

/**
 * Read an u_int32_t from the buffer, optionally from the end
 */
static bool read_uint32_internal(private_bio_reader_t *this, u_int32_t *res,
								 bool from_end)
{
	if (this->buf.len < 4)
	{
		DBG1(DBG_LIB, "%d bytes insufficient to parse u_int32 data",
			 this->buf.len);
		return FALSE;
	}
	*res = untoh32(get_ptr_end(this, 4, from_end));
	this->buf = chunk_skip_end(this->buf, 4, from_end);
	return TRUE;
}

/**
 * Read an u_int64_t from the buffer, optionally from the end
 */
static bool read_uint64_internal(private_bio_reader_t *this, u_int64_t *res,
								 bool from_end)
{
	if (this->buf.len < 8)
	{
		DBG1(DBG_LIB, "%d bytes insufficient to parse u_int64 data",
			 this->buf.len);
		return FALSE;
	}
	*res = untoh64(get_ptr_end(this, 8, from_end));
	this->buf = chunk_skip_end(this->buf, 8, from_end);
	return TRUE;
}

/**
 * Read a chunk of data from the buffer, optionally from the end
 */
static bool read_data_internal(private_bio_reader_t *this, u_int32_t len,
							   chunk_t *res, bool from_end)
{
	if (this->buf.len < len)
	{
		DBG1(DBG_LIB, "%d bytes insufficient to parse %d bytes of data",
			 this->buf.len, len);
		return FALSE;
	}
	*res = chunk_create(get_ptr_end(this, len, from_end), len);
	this->buf = chunk_skip_end(this->buf, len, from_end);
	return TRUE;
}

METHOD(bio_reader_t, read_uint8, bool,
	private_bio_reader_t *this, u_int8_t *res)
{
	return read_uint8_internal(this, res, FALSE);
}

METHOD(bio_reader_t, read_uint16, bool,
	private_bio_reader_t *this, u_int16_t *res)
{
	return read_uint16_internal(this, res, FALSE);
}

METHOD(bio_reader_t, read_uint24, bool,
	private_bio_reader_t *this, u_int32_t *res)
{
	return read_uint24_internal(this, res, FALSE);
}

METHOD(bio_reader_t, read_uint32, bool,
	private_bio_reader_t *this, u_int32_t *res)
{
	return read_uint32_internal(this, res, FALSE);
}

METHOD(bio_reader_t, read_uint64, bool,
	private_bio_reader_t *this, u_int64_t *res)
{
	return read_uint64_internal(this, res, FALSE);
}

METHOD(bio_reader_t, read_data, bool,
	private_bio_reader_t *this, u_int32_t len, chunk_t *res)
{
	return read_data_internal(this, len, res, FALSE);
}

METHOD(bio_reader_t, read_uint8_end, bool,
	private_bio_reader_t *this, u_int8_t *res)
{
	return read_uint8_internal(this, res, TRUE);
}

METHOD(bio_reader_t, read_uint16_end, bool,
	private_bio_reader_t *this, u_int16_t *res)
{
	return read_uint16_internal(this, res, TRUE);
}

METHOD(bio_reader_t, read_uint24_end, bool,
	private_bio_reader_t *this, u_int32_t *res)
{
	return read_uint24_internal(this, res, TRUE);
}

METHOD(bio_reader_t, read_uint32_end, bool,
	private_bio_reader_t *this, u_int32_t *res)
{
	return read_uint32_internal(this, res, TRUE);
}

METHOD(bio_reader_t, read_uint64_end, bool,
	private_bio_reader_t *this, u_int64_t *res)
{
	return read_uint64_internal(this, res, TRUE);
}

METHOD(bio_reader_t, read_data_end, bool,
	private_bio_reader_t *this, u_int32_t len, chunk_t *res)
{
	return read_data_internal(this, len, res, TRUE);
}

METHOD(bio_reader_t, read_data8, bool,
	private_bio_reader_t *this, chunk_t *res)
{
	u_int8_t len;

	if (!read_uint8(this, &len))
	{
		return FALSE;
	}
	return read_data(this, len, res);
}

METHOD(bio_reader_t, read_data16, bool,
	private_bio_reader_t *this, chunk_t *res)
{
	u_int16_t len;

	if (!read_uint16(this, &len))
	{
		return FALSE;
	}
	return read_data(this, len, res);
}

METHOD(bio_reader_t, read_data24, bool,
	private_bio_reader_t *this, chunk_t *res)
{
	u_int32_t len;

	if (!read_uint24(this, &len))
	{
		return FALSE;
	}
	return read_data(this, len, res);
}

METHOD(bio_reader_t, read_data32, bool,
	private_bio_reader_t *this, chunk_t *res)
{
	u_int32_t len;

	if (!read_uint32(this, &len))
	{
		return FALSE;
	}
	return read_data(this, len, res);
}

METHOD(bio_reader_t, destroy, void,
	private_bio_reader_t *this)
{
	free(this->cleanup.ptr);
	free(this);
}

/**
 * See header
 */
bio_reader_t *bio_reader_create(chunk_t data)
{
	private_bio_reader_t *this;

	INIT(this,
		.public = {
			.remaining = _remaining,
			.peek = _peek,
			.read_uint8 = _read_uint8,
			.read_uint16 = _read_uint16,
			.read_uint24 = _read_uint24,
			.read_uint32 = _read_uint32,
			.read_uint64 = _read_uint64,
			.read_data = _read_data,
			.read_uint8_end = _read_uint8_end,
			.read_uint16_end = _read_uint16_end,
			.read_uint24_end = _read_uint24_end,
			.read_uint32_end = _read_uint32_end,
			.read_uint64_end = _read_uint64_end,
			.read_data_end = _read_data_end,
			.read_data8 = _read_data8,
			.read_data16 = _read_data16,
			.read_data24 = _read_data24,
			.read_data32 = _read_data32,
			.destroy = _destroy,
		},
		.buf = data,
	);

	return &this->public;
}

/**
 * See header
 */
bio_reader_t *bio_reader_create_own(chunk_t data)
{
	private_bio_reader_t *this;

	this = (private_bio_reader_t*)bio_reader_create(data);

	this->cleanup = data;

	return &this->public;
}
