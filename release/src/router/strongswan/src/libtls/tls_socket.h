/*
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
 * @defgroup tls_socket tls_socket
 * @{ @ingroup libtls
 */

#ifndef TLS_SOCKET_H_
#define TLS_SOCKET_H_

#include "tls.h"

typedef struct tls_socket_t tls_socket_t;

/**
 * TLS secured socket.
 *
 * Wraps a blocking (socket) file descriptor for a reliable transport into a
 * TLS secured socket. TLS negotiation happens on demand, certificates and
 * private keys are fetched from any registered credential set.
 */
struct tls_socket_t {

	/**
	 * Read data from secured socket.
	 *
	 * This call is blocking, you may use select() on the underlying socket to
	 * wait for data. If "block" is FALSE and no application data is available,
	 * the function returns -1 and sets errno to EWOULDBLOCK.
	 *
	 * @param buf		buffer to write received data to
	 * @param len		size of buffer
	 * @param block		TRUE to block this call, FALSE to fail if it would block
	 * @return			number of bytes read, 0 on EOF, -1 on error
	 */
	ssize_t (*read)(tls_socket_t *this, void *buf, size_t len, bool block);

	/**
	 * Write data over the secured socket.
	 *
	 * @param buf		data to send
	 * @param len		number of bytes to write from buf
	 * @return			number of bytes written, -1 on error
	 */
	ssize_t (*write)(tls_socket_t *this, void *buf, size_t len);

	/**
	 * Read/write plain data from file descriptor.
	 *
	 * This call is blocking, but a thread cancellation point. Data is
	 * exchanged until one of the sockets gets closed or an error occurs.
	 *
	 * @param rfd		file descriptor to read plain data from
	 * @param wfd		file descriptor to write plain data to
	 * @return			TRUE if data exchanged successfully
	 */
	bool (*splice)(tls_socket_t *this, int rfd, int wfd);

	/**
	 * Get the underlying file descriptor passed to the constructor.
	 *
	 * @return			file descriptor
	 */
	int (*get_fd)(tls_socket_t *this);

	/**
	 * Return the server identity.
	 *
	 * @return			server identity
	 */
	identification_t* (*get_server_id)(tls_socket_t *this);

	/**
	 * Return the peer identity.
	 *
	 * @return			peer identity
	 */
	identification_t* (*get_peer_id)(tls_socket_t *this);

	/**
	 * Destroy a tls_socket_t.
	 */
	void (*destroy)(tls_socket_t *this);
};

/**
 * Create a tls_socket instance.
 *
 * @param is_server			TRUE to act as TLS server
 * @param server			server identity
 * @param peer				client identity, NULL for no client authentication
 * @param fd				socket to read/write from
 * @param cache				session cache to use, or NULL
 * @param max_version		maximum TLS version to negotiate
 * @param nullok			accept NULL encryption ciphers
 * @return					TLS socket wrapper
 */
tls_socket_t *tls_socket_create(bool is_server, identification_t *server,
							identification_t *peer, int fd, tls_cache_t *cache,
							tls_version_t max_version, bool nullok);

#endif /** TLS_SOCKET_H_ @}*/
