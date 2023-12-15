/*
 * Unix SMB/CIFS implementation.
 *
 * Replaceable functions by cwrap
 *
 * Copyright (c) 2014      Andreas Schneider <asn@samba.org>
 *
 *   ** NOTE! The following LGPL license applies to the replace
 *   ** library. This does NOT imply that all of Samba is released
 *   ** under the LGPL
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "replace.h"

bool nss_wrapper_enabled(void)
{
	return false;
}

bool nss_wrapper_hosts_enabled(void)
{
	return false;
}

bool socket_wrapper_enabled(void)
{
	return false;
}

bool uid_wrapper_enabled(void)
{
	return false;
}
