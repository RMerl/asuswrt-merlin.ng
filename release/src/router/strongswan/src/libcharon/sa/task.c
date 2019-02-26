/*
 * Copyright (C) 2007 Tobias Brunner
 * Copyright (C) 2007 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
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

#include "task.h"

ENUM(task_type_names, TASK_IKE_INIT, TASK_ISAKMP_CERT_POST,
	"IKE_INIT",
	"IKE_NATD",
	"IKE_MOBIKE",
	"IKE_AUTH",
	"IKE_AUTH_LIFETIME",
	"IKE_CERT_PRE",
	"IKE_CERT_POST",
	"IKE_CONFIG",
	"IKE_REKEY",
	"IKE_REAUTH",
	"IKE_REAUTH_COMPLETE",
	"IKE_REDIRECT",
	"IKE_VERIFY_PEER_CERT",
	"IKE_MID_SYNC",
	"IKE_DELETE",
	"IKE_DPD",
	"IKE_VENDOR",
#ifdef ME
	"IKE_ME",
#endif /* ME */
	"CHILD_CREATE",
	"CHILD_DELETE",
	"CHILD_REKEY",
	"MAIN_MODE",
	"AGGRESSIVE_MODE",
	"INFORMATIONAL",
	"ISAKMP_DELETE",
	"XAUTH",
	"MODE_CONFIG",
	"QUICK_MODE",
	"QUICK_DELETE",
	"ISAKMP_VENDOR",
	"ISAKMP_NATD",
	"ISAKMP_DPD",
	"ISAKMP_CERT_PRE",
	"ISAKMP_CERT_POST",
);
