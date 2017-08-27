/*
 * Copyright (C) 2008 Martin Willi
 * Copyright (C) 2006 Andreas Steffen
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

#include "crl.h"

#include <utils/debug.h>

ENUM(crl_reason_names, CRL_REASON_UNSPECIFIED, CRL_REASON_REMOVE_FROM_CRL,
	"unspecified",
	"key compromise",
	"ca compromise",
	"affiliation changed",
	"superseded",
	"cessation of operation",
	"certificate hold",
	"reason #7",
	"remove from crl",
);

/**
 * Check if this CRL is newer
 */
bool crl_is_newer(crl_t *this, crl_t *other)
{
	chunk_t this_num, other_num;
	bool newer;

	this_num = this->get_serial(this);
	other_num = other->get_serial(other);

	/* compare crlNumbers if available - otherwise use generic cert compare */
	if (this_num.ptr != NULL && other_num.ptr != NULL)
	{
		newer = chunk_compare(this_num, other_num) > 0;
		DBG1(DBG_LIB, "  crl #%#B is %s - existing crl #%#B %s",
			 &this_num, newer ? "newer" : "not newer",
			 &other_num, newer ? "replaced" : "retained");
	}
	else
	{
		newer = certificate_is_newer(&this->certificate, &other->certificate);
	}
	return newer;
}
