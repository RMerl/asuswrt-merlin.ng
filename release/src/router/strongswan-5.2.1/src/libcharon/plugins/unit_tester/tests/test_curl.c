/*
 * Copyright (C) 2007 Martin Willi
 * Hochschule fuer Technik Rapperswil
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

#include <daemon.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/*******************************************************************************
 * curl get test
 ******************************************************************************/

bool test_curl_get()
{
	chunk_t chunk;

	if (lib->fetcher->fetch(lib->fetcher, "http://www.strongswan.org",
							&chunk, FETCH_END) != SUCCESS)
	{
		return FALSE;
	}
	free(chunk.ptr);

	if (lib->fetcher->fetch(lib->fetcher, "http://www.google.com",
							&chunk, FETCH_END) != SUCCESS)
	{
		return FALSE;
	}
	free(chunk.ptr);
	return TRUE;
}

