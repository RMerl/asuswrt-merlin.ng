/* Error code definitions
 *
 * Copyright (C) 2003-2004  Narcis Ilisei <inarcis2002@hotpop.com>
 * Copyright (C) 2010-2020  Joachim Nilsson <troglobit@gmail.com>
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

static const ERROR_NAME global_error_table[] = {
	{ RC_OK,                          "OK"                               },
	{ RC_ERROR,                       "Error"                            },
	{ RC_INVALID_POINTER,             "Invalid pointer"                  },
	{ RC_OUT_OF_MEMORY,               "Out of memory"                    },
	{ RC_BUFFER_OVERFLOW,             "Too small internal buffer"        },
	{ RC_PIDFILE_EXISTS_ALREADY,      "Already running"                  },

	{ RC_TCP_SOCKET_CREATE_ERROR,     "Failed creating IP socket"        },
	{ RC_TCP_BAD_PARAMETER,           "Invalid Internet port"            },
	{ RC_TCP_INVALID_REMOTE_ADDR,     "Temporary network error (DNS)"    },
	{ RC_TCP_CONNECT_FAILED,          "Failed connecting to DDNS server" },
	{ RC_TCP_SEND_ERROR,              "Temporary network error (send)"   },
	{ RC_TCP_RECV_ERROR,              "Temporary network error (recv)"   },

	{ RC_TCP_OBJECT_NOT_INITIALIZED,  "Internal error (TCP)"             },
	{ RC_HTTP_OBJECT_NOT_INITIALIZED, "Internal error (HTTP)"            },

	{ RC_HTTPS_NO_TRUSTED_CA_STORE,   "System has no trusted CA store"             },
	{ RC_HTTPS_OUT_OF_MEMORY,         "Out of memory (HTTPS)"                      },
	{ RC_HTTPS_FAILED_CONNECT,        "Failed connecting to DDNS server (HTTPS)"   },
	{ RC_HTTPS_FAILED_GETTING_CERT,   "Failed retrieving DDNS server cert (HTTPS)" },
	{ RC_HTTPS_SEND_ERROR,            "Temporary network error (HTTPS send)"       },
	{ RC_HTTPS_RECV_ERROR,            "Temporary network error (HTTPS recv)"       },
	{ RC_HTTPS_SNI_ERROR,             "Failed setting HTTPS server name"           },
	{ RC_HTTPS_INVALID_REQUEST,       "Invalid request (HTTPS)"                    },

	{ RC_DDNS_INVALID_CHECKIP_RSP,    "Check IP server response not OK"  },
	{ RC_DDNS_INVALID_OPTION,         "Invalid or missing DDNS option"   },
	{ RC_DDNS_RSP_NOTOK,              "DDNS server response not OK"      },
	{ RC_DDNS_RSP_RETRY_LATER,        "DDNS server busy, try later"      },
	{ RC_DDNS_RSP_AUTH_FAIL,          "Authentication failure"           },

	{ RC_OS_FORK_FAILURE,             "Failed forking off child"         },
	{ RC_OS_CHANGE_PERSONA_FAILURE,   "Failed dropping privileges"       },
	{ RC_OS_INVALID_UID,              "Invalid or unknown UID"           },
	{ RC_OS_INVALID_GID,              "Invalid or unknown GID"           },

	{ RC_FILE_IO_ACCESS_ERROR,        "Failed create/modify file/dir"    },
	{ RC_FILE_IO_MISSING_FILE,        "Missing .conf file"               },

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
