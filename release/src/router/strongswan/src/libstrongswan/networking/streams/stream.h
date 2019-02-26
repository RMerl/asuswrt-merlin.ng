/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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
 * @defgroup stream stream
 * @{ @ingroup streams
 */

#ifndef STREAM_H_
#define STREAM_H_

typedef struct stream_t stream_t;

#include <library.h>

/**
 * Constructor function prototype for stream_t.
 *
 * @param uri			URI to create a stream for
 * @return				stream instance, NULL on error
 */
typedef stream_t*(*stream_constructor_t)(char *uri);

/**
 * Callback function prototype, called when stream is ready.
 *
 * It is not allowed to destroy the stream nor to call on_read()/on_write/()
 * during the callback.
 *
 * As select() may return even if a read()/write() would actually block, it is
 * recommended to use the non-blocking calls and handle return values
 * appropriately.
 *
 * @param data			data passed during callback registration
 * @param stream		associated stream
 * @return				FALSE unregisters the invoked callback, TRUE keeps it
 */
typedef bool (*stream_cb_t)(void *data, stream_t *stream);

/**
 * Abstraction of a Berkley socket using stream semantics.
 */
struct stream_t {

	/**
	 * Read data from the stream.
	 *
	 * If "block" is FALSE and no data is available, the function returns -1
	 * and sets errno to EWOULDBLOCK.
	 *
	 * @param buf		data buffer to read into
	 * @param len		number of bytes to read
	 * @param block		TRUE to use a blocking read
	 * @return			number of bytes read, -1 on error
	 */
	ssize_t (*read)(stream_t *this, void *buf, size_t len, bool block);

	/**
	 * Read data from the stream, avoiding short reads.
	 *
	 * This call is always blocking, and reads until len has been read
	 * completely. If the connection is closed before enough bytes could be
	 * returned, errno is set to ECONNRESET.
	 *
	 * @param buf		data buffer to read into
	 * @param len		number of bytes to read
	 * @return			TRUE if len bytes read, FALSE on error
	 */
	bool (*read_all)(stream_t *this, void *buf, size_t len);

	/**
	 * Register a callback to invoke when stream has data to read.
	 *
	 * @param cb		callback function, NULL to unregister
	 * @param data		data to pass to callback
	 */
	void (*on_read)(stream_t *this, stream_cb_t cb, void *data);

	/**
	 * Write data to the stream.
	 *
	 * If "block" is FALSE and the write would block, the function returns -1
	 * and sets errno to EWOULDBLOCK.
	 *
	 * @param buf		data buffer to write
	 * @param len		number of bytes to write
	 * @param block		TRUE to use a blocking write
	 * @return			number of bytes written, -1 on error
	 */
	ssize_t (*write)(stream_t *this, void *buf, size_t len, bool block);

	/**
	 * Write data to the stream, avoiding short writes.
	 *
	 * This call is always blocking, and writes until len bytes has been
	 * written.
	 *
	 * @param buf		data buffer to write
	 * @param len		number of bytes to write
	 * @return			TRUE if len bytes written, FALSE on error
	 */
	bool (*write_all)(stream_t *this, void *buf, size_t len);

	/**
	 * Register a callback to invoke when a write would not block.
	 *
	 * @param cb		callback function, NULL to unregister
	 * @param data		data to pass to callback
	 */
	void (*on_write)(stream_t *this, stream_cb_t cb, void *data);

	/**
	 * Get a FILE reference for this stream.
	 *
	 * @return			FILE*, must be fclose()d, NULL on error
	 */
	FILE* (*get_file)(stream_t *this);

	/**
	 * Destroy a stream_t.
	 */
	void (*destroy)(stream_t *this);
};

/**
 * Create a stream from a file descriptor.
 *
 * The file descriptor MUST be a socket for non-blocking operation.
 *
 * @param fd		file descriptor to wrap into a stream_t
 * @return			stream instance
 */
stream_t *stream_create_from_fd(int fd);

#endif /** STREAM_H_ @}*/
