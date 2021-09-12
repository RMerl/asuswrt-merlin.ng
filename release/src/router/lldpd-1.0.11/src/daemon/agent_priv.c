/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2008 Vincent Bernat <bernat@luffy.cx>
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

/* Some of the code here (agent_priv_unix_*) has been adapted from code from
 * Net-SNMP project (snmplib/snmpUnixDomain.c). Net-SNMP project is licensed
 * using BSD and BSD-like licenses. I don't know the exact license of the file
 * snmplib/snmpUnixDomain.c. */

#include "lldpd.h"

#include <unistd.h>
#include <errno.h>
#include <poll.h>

#ifdef ENABLE_PRIVSEP
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/snmp_vars.h>
#include <net-snmp/library/snmpUnixDomain.h>

#ifdef ASN_PRIV_STOP
/* NetSNMP 5.8+ */
#define F_SEND_SIGNATURE netsnmp_transport *t, const void *buf, int size, void **opaque, int *olength
#define F_FMTADDR_SIGNATURE netsnmp_transport *t, const void *data, int len
#define F_FROM_OSTRING_SIGNATURE const void* o, size_t o_len, int local
#else
#define F_SEND_SIGNATURE netsnmp_transport *t, void *buf, int size, void **opaque, int *olength
#define F_FMTADDR_SIGNATURE netsnmp_transport *t, void *data, int len
#define F_FROM_OSTRING_SIGNATURE const u_char* o, size_t o_len, int local
#endif

static oid netsnmp_unix[] = { TRANSPORT_DOMAIN_LOCAL };
static netsnmp_tdomain unixDomain;

static char *
agent_priv_unix_fmtaddr(F_FMTADDR_SIGNATURE)
{
	/* We don't bother to implement the full function */
	return strdup("Local Unix socket with privilege separation: unknown");
}

static int
agent_priv_unix_recv(netsnmp_transport *t, void *buf, int size,
    void **opaque, int *olength)
{
	int rc = -1;
	socklen_t  tolen = sizeof(struct sockaddr_un);
	struct sockaddr *to = NULL;

	if (t == NULL || t->sock < 0)
		goto recv_error;
	to = (struct sockaddr *)calloc(1, sizeof(struct sockaddr_un));
	if (to == NULL)
		goto recv_error;
	if (getsockname(t->sock, to, &tolen) != 0)
		goto recv_error;
	while (rc < 0) {
		rc = recv(t->sock, buf, size, 0);
		/* TODO: handle the (unlikely) case where we get EAGAIN or EWOULDBLOCK */
		if (rc < 0 && errno != EINTR) {
			log_warn("snmp", "unable to receive from fd %d",
			    t->sock);
			goto recv_error;
		}
	}
	*opaque = (void*)to;
	*olength = sizeof(struct sockaddr_un);
	return rc;

recv_error:
	free(to);
	*opaque = NULL;
	*olength = 0;
	return -1;
}

#define AGENT_WRITE_TIMEOUT 2000
static int
agent_priv_unix_send(F_SEND_SIGNATURE)
{
	int rc = -1;

	if (t != NULL && t->sock >= 0) {
		struct pollfd sagentx = {
			.fd = t->sock,
			.events = POLLOUT | POLLERR | POLLHUP
		};
		while (rc < 0) {
			rc = poll(&sagentx, 1, AGENT_WRITE_TIMEOUT);
			if (rc == 0) {
				log_warnx("snmp",
				    "timeout while communicating with the master agent");
				rc = -1;
				break;
			}
			if (rc > 0) {
				/* We can either write or have an error somewhere */
				rc = send(t->sock, buf, size, 0);
				if (rc < 0) {
					if (errno == EAGAIN ||
					    errno == EWOULDBLOCK ||
					    errno == EINTR)
						/* Let's retry */
						continue;
					log_warn("snmp",
					    "error while sending to master agent");
					break;
				}
			} else {
				if (errno != EINTR) {
					log_warn("snmp",
					    "error while attempting to send to master agent");
					break;
				}
				continue;
			}
		}
	}
	return rc;
}

static int
agent_priv_unix_close(netsnmp_transport *t)
{
	int rc = 0;

	if (t->sock >= 0) {
		rc = close(t->sock);
		t->sock = -1;
		return rc;
	}
	return -1;
}

static int
agent_priv_unix_accept(netsnmp_transport *t)
{
	log_warnx("snmp", "should not have been called");
	return -1;
}

static netsnmp_transport *
agent_priv_unix_transport(const char *string, int len, int local)
{
	struct sockaddr_un addr = {
		.sun_family = AF_UNIX
	};
	netsnmp_transport *t = NULL;

	if (local) {
		log_warnx("snmp", "should not have been called for local transport");
		return NULL;
	}
	if (!string)
		return NULL;
	if (len >= sizeof(addr.sun_path) ||
	    strlcpy(addr.sun_path, string, sizeof(addr.sun_path)) >= sizeof(addr.sun_path)) {
		log_warnx("snmp", "path too long for Unix domain transport");
		return NULL;
	}

	if ((t = (netsnmp_transport *)
		calloc(1, sizeof(netsnmp_transport))) == NULL)
		return NULL;

	t->domain = netsnmp_unix;
	t->domain_length =
	    sizeof(netsnmp_unix) / sizeof(netsnmp_unix[0]);

	if ((t->sock = priv_snmp_socket(&addr)) < 0) {
		netsnmp_transport_free(t);
		return NULL;
	}

	t->flags = NETSNMP_TRANSPORT_FLAG_STREAM;

	if ((t->remote = (u_char *)
		calloc(1, strlen(addr.sun_path) + 1)) == NULL) {
		agent_priv_unix_close(t);
		netsnmp_transport_free(t);
		return NULL;
	}
	memcpy(t->remote, addr.sun_path, strlen(addr.sun_path));
	t->remote_length = strlen(addr.sun_path);

	t->msgMaxSize = 0x7fffffff;
	t->f_recv     = agent_priv_unix_recv;
	t->f_send     = agent_priv_unix_send;
	t->f_close    = agent_priv_unix_close;
	t->f_accept   = agent_priv_unix_accept;
	t->f_fmtaddr  = agent_priv_unix_fmtaddr;

	return t;
}

#if HAVE_NETSNMP_TDOMAIN_F_CREATE_FROM_TSTRING_NEW
netsnmp_transport *
agent_priv_unix_create_tstring_new(const char *string, int local, const char *default_target)
{
	if ((!string || *string == '\0') && default_target &&
	    *default_target != '\0') {
		string = default_target;
	}
	if (!string) return NULL;
	return agent_priv_unix_transport(string, strlen(string), local);
}
#else
netsnmp_transport *
agent_priv_unix_create_tstring(const char *string, int local)
{
	if (!string) return NULL;
	return agent_priv_unix_transport(string, strlen(string), local);
}
#endif

static netsnmp_transport *
agent_priv_unix_create_ostring(F_FROM_OSTRING_SIGNATURE)
{
	return agent_priv_unix_transport((char *)o, o_len, local);
}

void
agent_priv_register_domain()
{
	unixDomain.name = netsnmp_unix;
	unixDomain.name_length = sizeof(netsnmp_unix) / sizeof(oid);
	unixDomain.prefix = (const char**)calloc(2, sizeof(char *));
	unixDomain.prefix[0] = "unix";
#if HAVE_NETSNMP_TDOMAIN_F_CREATE_FROM_TSTRING_NEW
	unixDomain.f_create_from_tstring_new = agent_priv_unix_create_tstring_new;
#else
	unixDomain.f_create_from_tstring = agent_priv_unix_create_tstring;
#endif
	unixDomain.f_create_from_ostring = agent_priv_unix_create_ostring;
	netsnmp_tdomain_register(&unixDomain);
}
#endif
