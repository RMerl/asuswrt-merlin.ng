/* Error code definitions
 *
 * Copyright (C) 2003-2004  Narcis Ilisei <inarcis2002@hotpop.com>
 * Copyright (C) 2010-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, visit the Free Software Foundation
 * website at http://www.gnu.org/licenses/gpl-2.0.html or write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include "../include/error.h"
#include <stdlib.h>

typedef struct {
	int rc;
	const char *p_name;
} ERROR_NAME;

#ifndef DROP_VERBOSE_STRINGS
#define E(a) a
#else
#define E(a) "Err"
#endif

static const ERROR_NAME global_error_table[] = {
	{ RC_OK,                            "OK"                                },
	{ RC_ERROR,                       E("Error"                            )},
	{ RC_INVALID_POINTER,             E("Invalid pointer"                  )},
	{ RC_OUT_OF_MEMORY,               E("Out of memory"                    )},
	{ RC_BUFFER_OVERFLOW,             E("Too small internal buffer"        )},
	{ RC_PIDFILE_EXISTS_ALREADY,      E("Already running"                  )},

	{ RC_TCP_SOCKET_CREATE_ERROR,     E("Failed creating IP socket"        )},
	{ RC_TCP_BAD_PARAMETER,           E("Invalid Internet port"            )},
	{ RC_TCP_INVALID_REMOTE_ADDR,     E("Temporary network error (DNS)"    )},
	{ RC_TCP_CONNECT_FAILED,          E("Failed connecting to DDNS server" )},
	{ RC_TCP_SEND_ERROR,              E("Temporary network error (send)"   )},
	{ RC_TCP_RECV_ERROR,              E("Temporary network error (recv)"   )},

	{ RC_TCP_OBJECT_NOT_INITIALIZED,  E("Internal error (TCP)"             )},
	{ RC_HTTP_OBJECT_NOT_INITIALIZED, E("Internal error (HTTP)"            )},

	{ RC_HTTPS_NO_TRUSTED_CA_STORE,   E("System has no trusted CA store"             )},
	{ RC_HTTPS_OUT_OF_MEMORY,         E("Out of memory (HTTPS)"                      )},
	{ RC_HTTPS_FAILED_CONNECT,        E("Failed connecting to DDNS server (HTTPS)"   )},
	{ RC_HTTPS_FAILED_GETTING_CERT,   E("Failed retrieving DDNS server cert (HTTPS)" )},
	{ RC_HTTPS_SEND_ERROR,            E("Temporary network error (HTTPS send)"       )},
	{ RC_HTTPS_RECV_ERROR,            E("Temporary network error (HTTPS recv)"       )},
	{ RC_HTTPS_SNI_ERROR,             E("Failed setting HTTPS server name"           )},
	{ RC_HTTPS_INVALID_REQUEST,       E("Invalid request (HTTPS)"                    )},

	{ RC_DDNS_INVALID_CHECKIP_RSP,    E("Check IP server response not OK"  )},
	{ RC_DDNS_INVALID_OPTION,         E("Invalid or missing DDNS option"   )},
	{ RC_DDNS_RSP_NOTOK,              E("DDNS server response not OK"      )},
	{ RC_DDNS_RSP_RETRY_LATER,        E("DDNS server busy, try later"      )},
	{ RC_DDNS_RSP_AUTH_FAIL,          E("Authentication failure"           )},

	{ RC_OS_FORK_FAILURE,             E("Failed forking off child"         )},
	{ RC_OS_CHANGE_PERSONA_FAILURE,   E("Failed dropping privileges"       )},
	{ RC_OS_INVALID_UID,              E("Invalid or unknown UID"           )},
	{ RC_OS_INVALID_GID,              E("Invalid or unknown GID"           )},

	{ RC_FILE_IO_ACCESS_ERROR,        E("Failed create/modify file/dir"    )},
	{ RC_FILE_IO_MISSING_FILE,        E("Missing .conf file"               )},

	{ RC_OK, NULL }
};

static const char *unknown_error = "Unknown error";

const char *error_str(int rc)
{
	const ERROR_NAME *it = global_error_table;

	while (it->p_name) {
		if (it->rc == rc)
			return it->p_name;
		it++;
	}

	return unknown_error;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
