/*
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

#include <library.h>

#include "stream_service_vsock.h"
#include "stream_vsock.h"

/*
 * Described in header
 */
stream_service_t *stream_service_create_vsock(char *uri, int backlog)
{
	int fd = stream_initialize_socket_vsock(uri, &backlog);

	return (fd == -1) ? NULL : stream_service_create_from_fd(fd);
}
