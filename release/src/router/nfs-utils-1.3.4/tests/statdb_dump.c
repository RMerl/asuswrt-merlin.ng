/*
 * statdb_dump.c -- dump contents of statd's monitor DB
 *
 * Copyright (C) 2010  Red Hat, Jeff Layton <jlayton@redhat.com>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>

#include "nsm.h"
#include "xlog.h"

static char cookiebuf[(SM_PRIV_SIZE * 2) + 1];
static char addrbuf[INET6_ADDRSTRLEN + 1];

static unsigned int
dump_host(const char *hostname, const struct sockaddr *sa, const struct mon *m,
	  const time_t timestamp)
{
	int ret;
	const char *addr;
	const struct sockaddr_in *sin;
	const struct sockaddr_in6 *sin6;

	ret = nsm_priv_to_hex(m->priv, cookiebuf, sizeof(cookiebuf));
	if (!ret) {
		xlog(L_ERROR, "Unable to convert cookie to hex string.\n");
		return ret;
	}

	switch (sa->sa_family) {
	case AF_INET:
		sin = (struct sockaddr_in *)(char *)sa;
		addr = inet_ntop(sa->sa_family, &sin->sin_addr.s_addr, addrbuf,
				 (socklen_t)sizeof(addrbuf));
		break;
	case AF_INET6:
		sin6 = (struct sockaddr_in6 *)(char *)sa;
		addr = inet_ntop(sa->sa_family, &sin6->sin6_addr, addrbuf,
				 (socklen_t)sizeof(addrbuf));
		break;
	default:
		xlog(L_ERROR, "Unrecognized address family: %hu\n",
			sa->sa_family);
		return 0;
	}

	if (addr == NULL) {
		xlog(L_ERROR, "Unable to convert sockaddr to string: %s\n",
				strerror(errno));
		return 0;
	}

	/*
	 * Callers of this program should assume that in the future, extra
	 * fields may be added to the output. Anyone adding extra fields to
	 * the output should add them to the end of the line.
	 */
	printf("%s %s %s %s %s %d %d %d\n",
			hostname, addr, cookiebuf,
			m->mon_id.mon_name,
			m->mon_id.my_id.my_name,
			m->mon_id.my_id.my_prog,
			m->mon_id.my_id.my_vers,
			m->mon_id.my_id.my_proc); 

	return 1;
}

int
main(int argc, char **argv)
{
	xlog_syslog(0);
	xlog_stderr(1);
	xlog_open(argv[0]);
	
	nsm_load_monitor_list(dump_host);
	return 0;
}
