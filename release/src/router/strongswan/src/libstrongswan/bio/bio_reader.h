/*
 * Copyright (C) 2012 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
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

/**
 * @defgroup bio_reader bio_reader
 * @{ @ingroup bio
 */

#ifndef BIO_READER_H_
#define BIO_READER_H_

typedef struct bio_reader_t bio_reader_t;

#include <library.h>

/**
 * Buffered input parser.
 *
 * @note Integers are returned in host byte order.
 */
struct bio_reader_t {

	/**
	 * Get the number of remaining bytes.
	 *
	 * @return			number of remaining bytes in buffer
	 */
	uint32_t (*remaining)(bio_reader_t *this);

	/**
	 * Peek the remaining data, not consuming any bytes.
	 *
	 * @return			remaining data
	 */
	chunk_t (*peek)(bio_reader_t *this);

	/**
	 * Read a 8-bit integer from the buffer, advance.
	 *
	 * @param res		pointer to result
	 * @return			TRUE if integer read successfully
	 */
	bool (*read_uint8)(bio_reader_t *this, uint8_t *res);

	/**
	 * Read a 16-bit integer from the buffer, advance.
	 *
	 * @param res		pointer to result
	 * @return			TRUE if integer read successfully
	 */
	bool (*read_uint16)(bio_reader_t *this, uint16_t *res);

	/**
	 * Read a 24-bit integer from the buffer, advance.
	 *
	 * @param res		pointer to result
	 * @return			TRUE if integer read successfully
	 */
	bool (*read_uint24)(bio_reader_t *this, uint32_t *res);

	/**
	 * Read a 32-bit integer from the buffer, advance.
	 *
	 * @param res		pointer to result
	 * @return			TRUE if integer read successfully
	 */
	bool (*read_uint32)(bio_reader_t *this, uint32_t *res);

	/**
	 * Read a 64-bit integer from the buffer, advance.
	 *
	 * @param res		pointer to result
	 * @return			TRUE if integer read successfully
	 */
	bool (*read_uint64)(bio_reader_t *this, uint64_t *res);

	/**
	 * Read a chunk of len bytes, advance.
	 *
	 * @param len		number of bytes to read
	 * @param res		pointer to result, not cloned
	 * @return			TRUE if data read successfully
	 */
	bool (*read_data)(bio_reader_t *this, uint32_t len, chunk_t *res);

	/**
	 * Read a 8-bit integer from the end of the buffer, reduce remaining.
	 *
	 * @param res		pointer to result
	 * @return			TRUE if integer read successfully
	 */
	bool (*read_uint8_end)(bio_reader_t *this, uint8_t *res);

	/**
	 * Read a 16-bit integer from the end of the buffer, reduce remaining.
	 *
	 * @param res		pointer to result
	 * @return			TRUE if integer read successfully
	 */
	bool (*read_uint16_end)(bio_reader_t *this, uint16_t *res);

	/**
	 * Read a 24-bit integer from the end of the buffer, reduce remaining.
	 *
	 * @param res		pointer to result
	 * @return			TRUE if integer read successfully
	 */
	bool (*read_uint24_end)(bio_reader_t *this, uint32_t *res);

	/**
	 * Read a 32-bit integer from the end of the buffer, reduce remaining.
	 *
	 * @param res		pointer to result
	 * @return			TRUE if integer read successfully
	 */
	bool (*read_uint32_end)(bio_reader_t *this, uint32_t *res);

	/**
	 * Read a 64-bit integer from the end of the buffer, reduce remaining.
	 *
	 * @param res		pointer to result
	 * @return			TRUE if integer read successfully
	 */
	bool (*read_uint64_end)(bio_reader_t *this, uint64_t *res);

	/**
	 * Read a chunk of len bytes from the end of the buffer, reduce remaining.
	 *
	 * @param len		number of bytes to read
	 * @param res		pointer to result, not cloned
	 * @return			TRUE if data read successfully
	 */
	bool (*read_data_end)(bio_reader_t *this, uint32_t len, chunk_t *res);

	/**
	 * Read a chunk of bytes with a 8-bit length header, advance.
	 *
	 * @param res		pointer to result, not cloned
	 * @return			TRUE if data read successfully
	 */
	bool (*read_data8)(bio_reader_t *this, chunk_t *res);

	/**
	 * Read a chunk of bytes with a 16-bit length header, advance.
	 *
	 * @param res		pointer to result, not cloned
	 * @return			TRUE if data read successfully
	 */
	bool (*read_data16)(bio_reader_t *this, chunk_t *res);

	/**
	 * Read a chunk of bytes with a 24-bit length header, advance.
	 *
	 * @param res		pointer to result, not cloned
	 * @return			TRUE if data read successfully
	 */
	bool (*read_data24)(bio_reader_t *this, chunk_t *res);

	/**
	 * Read a chunk of bytes with a 32-bit length header, advance.
	 *
	 * @param res		pointer to result, not cloned
	 * @return			TRUE if data read successfully
	 */
	bool (*read_data32)(bio_reader_t *this, chunk_t *res);

	/**
	 * Destroy a bio_reader_t.
	 */
	void (*destroy)(bio_reader_t *this);
};

/**
 * Create a bio_reader instance.
 *
 * @param data			data buffer, must survive lifetime of reader
 * @return				reader
 */
bio_reader_t *bio_reader_create(chunk_t data);

/**
 * Create a bio_reader instance owning buffer.
 *
 * @param data			data buffer, gets freed with destroy()
 * @return				reader
 */
bio_reader_t *bio_reader_create_own(chunk_t data);

#endif /** BIO_READER_H_ @}*/
