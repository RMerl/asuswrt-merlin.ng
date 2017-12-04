/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2012 Vincent Bernat <bernat@luffy.cx>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "lldpctl.h"
#include "atom.h"
#include "../log.h"

const char*
lldpctl_strerror(lldpctl_error_t error)
{
	/* No default case to let the compiler warns us if we miss an error code. */
	switch (error) {
	case LLDPCTL_NO_ERROR: return "No error";
	case LLDPCTL_ERR_WOULDBLOCK: return "Requested operation would block";
	case LLDPCTL_ERR_EOF: return "End of file reached";
	case LLDPCTL_ERR_NOT_EXIST: return "The requested information does not exist";
	case LLDPCTL_ERR_CANNOT_CONNECT: return "Unable to connect to lldpd daemon";
	case LLDPCTL_ERR_INCORRECT_ATOM_TYPE: return "Provided atom is of incorrect type";
	case LLDPCTL_ERR_SERIALIZATION: return "Error while serializing or unserializing data";
	case LLDPCTL_ERR_INVALID_STATE: return "Other input/output operation already in progress";
	case LLDPCTL_ERR_CANNOT_ITERATE: return "Cannot iterate on this atom";
	case LLDPCTL_ERR_CANNOT_CREATE: return "Cannot create a new element for this atom";
	case LLDPCTL_ERR_BAD_VALUE: return "Provided value is invalid";
	case LLDPCTL_ERR_FATAL: return "Unexpected fatal error";
	case LLDPCTL_ERR_NOMEM: return "Not enough memory available";
	case LLDPCTL_ERR_CALLBACK_FAILURE: return "A failure occurred during callback processing";
	}
	return "Unknown error code";
}

lldpctl_error_t
lldpctl_last_error(lldpctl_conn_t *lldpctl)
{
	return lldpctl->error;
}

void
lldpctl_log_callback(void (*cb)(int severity, const char *msg))
{
	log_register(cb);
}

void
lldpctl_log_level(int level)
{
	if (level >= 1) log_level(level-1);
}
