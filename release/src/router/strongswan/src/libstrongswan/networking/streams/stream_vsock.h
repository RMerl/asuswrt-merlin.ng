/*
 * Copyright (C) 2024 Tobias Brunner
 * Copyright (C) 2024 Thomas Egerer
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

/**
 * @defgroup stream_vsock stream_vsock
 * @{ @ingroup streams
 */

#ifndef STREAM_VSOCK_H_
#define STREAM_VSOCK_H_

/**
 * Create a stream for VSOCK sockets.
 *
 * VSOCK URIs start with vsock://, followed by an integer address (context
 * identifier, CID), followed by a colon separated port. CID as well as port
 * are 32-bit unsigned integers. A full VSOCK uri looks something like:
 *
 * * vsock://1:12345 (loopback)
 * * vsock://2:12345 (connect from the VM to the well-known CID of the host)
 * * vsock://3:12345 (CID 3 as assigned to a VM)
 * * vsock://\htmlonly\endhtmlonly*:12345 (listen on any CID)
 *
 * There is no default port, so a colon after vsock:// is mandatory.
 *
 * @param uri		VSOCK socket specific URI, must start with "vsock://"
 * @return			stream instance, NULL on failure
 */
stream_t *stream_create_vsock(char *uri);

/**
 * Create and initialize a VSOCK socket.
 *
 * @param uri		VSOCK socket specific URI, must start with "vsock://"
 * @param backlog	pointer to value for backlog for listen(2) if a service
 * 					socket shall be created (bind/listen); use NULL for a
 * 					VSOCK socket that just connects to \p uri
 * @return			file descriptor for created socket, -1 on error
 */
int stream_initialize_socket_vsock(char *uri, int *backlog);

#endif /** STREAM_VSOCK_H_ @}*/
