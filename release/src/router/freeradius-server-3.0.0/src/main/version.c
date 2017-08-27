/*
 * version.c	Print version number and exit.
 *
 * Version:	$Id$
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 1999-2012  The FreeRADIUS server project
 * Copyright 2012  Alan DeKok <aland@ox.org>
 * Copyright 2000  Chris Parker <cparker@starnetusa.com>
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
USES_APPLE_DEPRECATED_API	/* OpenSSL API has been deprecated by Apple */

#ifdef HAVE_OPENSSL_CRYPTO_H

#include <openssl/crypto.h>
#include <openssl/opensslv.h>

static long ssl_built = OPENSSL_VERSION_NUMBER;

/** Check build and linked versions of OpenSSL match
 *
 * Startup check for whether the linked version of OpenSSL matches the
 * version the server was built against.
 *
 * @return 0 if ok, else -1
 */
int ssl_check_version(void)
{
	long ssl_linked;

	ssl_linked = SSLeay();

	if (ssl_linked != ssl_built) {
		ERROR("libssl version mismatch."
		       "  Built with: %lx\n  Linked: %lx",
		       (unsigned long) ssl_built,
		       (unsigned long) ssl_linked);

		return -1;
	};

	return 0;
}

/** Print the current linked version of Openssl
 *
 * Print the currently linked version of the OpenSSL library.
 */
char const *ssl_version(void)
{
	return SSLeay_version(SSLEAY_VERSION);
}
#else
int ssl_check_Version(void) {
	return 0;
}

char const *ssl_version()
{
	return "not linked";
}
#endif

/*
 *	Display the revision number for this program
 */
void version(void)
{
	INFO("%s: %s", progname, radiusd_version);

	DEBUG3("Server was built with: ");

#ifdef WITH_ACCOUNTING
	DEBUG3("  accounting");
#endif
	DEBUG3("  authentication"); /* always enabled */

#ifdef WITH_ASCEND_BINARY
	DEBUG3("  ascend binary attributes");
#endif
#ifdef WITH_COA
	DEBUG3("  coa");
#endif
#ifdef WITH_COMMAND_SOCKET
	DEBUG3("  control-socket");
#endif
#ifdef WITH_DETAIL
	DEBUG3("  detail");
#endif
#ifdef WITH_DHCP
	DEBUG3("  dhcp");
#endif
#ifdef WITH_DYNAMIC_CLIENTS
	DEBUG3("  dynamic clients");
#endif
#ifdef OSFC2
	DEBUG3("  OSFC2");
#endif
#ifdef WITH_PROXY
	DEBUG3("  proxy");
#endif
#ifdef HAVE_PCREPOSIX_H
	DEBUG3("  regex-pcre");
#else
#ifdef HAVE_REGEX_H
	DEBUG3("  regex-posix");
#endif
#endif

#ifdef WITH_SESSION_MGMT
	DEBUG3("  session-management");
#endif
#ifdef WITH_STATS
	DEBUG3("  stats");
#endif
#ifdef WITH_TCP
	DEBUG3("  tcp");
#endif
#ifdef WITH_THREADS
	DEBUG3("  threads");
#endif
#ifdef WITH_TLS
	DEBUG3("  tls");
#endif
#ifdef WITH_UNLANG
	DEBUG3("  unlang");
#endif
#ifdef WITH_VMPS
	DEBUG3("  vmps");
#endif

	DEBUG3("Server core libs:");
	DEBUG3("  talloc : %i.%i.*", talloc_version_major(),
	       talloc_version_minor());
	DEBUG3("  ssl    : %s", ssl_version());


	INFO("Copyright (C) 1999-2013 The FreeRADIUS server project and contributors.");
	INFO("There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A");
	INFO("PARTICULAR PURPOSE.");
	INFO("You may redistribute copies of FreeRADIUS under the terms of the");
	INFO("GNU General Public License.");
	INFO("For more information about these matters, see the file named COPYRIGHT.");

	fflush(NULL);
}

