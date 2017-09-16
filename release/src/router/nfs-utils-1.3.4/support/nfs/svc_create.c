/*
 * Copyright 2009 Oracle.  All rights reserved.
 *
 * This file is part of nfs-utils.
 *
 * nfs-utils is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * nfs-utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with nfs-utils.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include "nfslib.h"

#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/resource.h>

#include <rpc/rpc.h>
#include <rpc/svc.h>

#ifdef HAVE_TCP_WRAPPER
#include "tcpwrapper.h"
#endif

#include "sockaddr.h"
#include "rpcmisc.h"
#include "xlog.h"

#ifdef HAVE_LIBTIRPC

#include <rpc/rpc_com.h>

#define SVC_CREATE_XPRT_CACHE_SIZE	(8)
static SVCXPRT *svc_create_xprt_cache[SVC_CREATE_XPRT_CACHE_SIZE] = { NULL, };

/*
 * Cache an SVC xprt, in case there are more programs or versions to
 * register against it.
 */
static void
svc_create_cache_xprt(SVCXPRT *xprt)
{
	unsigned int i;

	/* Check if we've already got this one... */
	for (i = 0; i < SVC_CREATE_XPRT_CACHE_SIZE; i++)
		if (svc_create_xprt_cache[i] == xprt)
			return;

	/* No, we don't.  Cache it. */
	for (i = 0; i < SVC_CREATE_XPRT_CACHE_SIZE; i++)
		if (svc_create_xprt_cache[i] == NULL) {
			svc_create_xprt_cache[i] = xprt;
			return;
		}

	xlog(L_ERROR, "%s: Failed to cache an xprt", __func__);
}

/*
 * Find a previously cached SVC xprt structure with the given bind address
 * and transport semantics.
 *
 * Returns pointer to a cached SVC xprt.
 *
 * If no matching SVC XPRT can be found, NULL is returned.
 */
static SVCXPRT *
svc_create_find_xprt(const struct sockaddr *bindaddr, const struct netconfig *nconf)
{
	unsigned int i;

	for (i = 0; i < SVC_CREATE_XPRT_CACHE_SIZE; i++) {
		SVCXPRT *xprt = svc_create_xprt_cache[i];
		struct sockaddr *sap;

		if (xprt == NULL)
			continue;
		if (strcmp(nconf->nc_netid, xprt->xp_netid) != 0)
			continue;
		sap = (struct sockaddr *)xprt->xp_ltaddr.buf;
		if (!nfs_compare_sockaddr(bindaddr, sap))
			continue;
		return xprt;
	}
	return NULL;
}

/*
 * Set up an appropriate bind address, given @port and @nconf.
 *
 * Returns getaddrinfo(3) results if successful.  Caller must
 * invoke freeaddrinfo(3) on these results.
 *
 * Otherwise NULL is returned if an error occurs.
 */
__attribute__((__malloc__))
static struct addrinfo *
svc_create_bindaddr(struct netconfig *nconf, const uint16_t port)
{
	struct addrinfo *ai = NULL;
	struct addrinfo hint = {
		.ai_flags	= AI_PASSIVE | AI_NUMERICSERV,
	};
	char buf[8];
	int error;

	if (strcmp(nconf->nc_protofmly, NC_INET) == 0)
		hint.ai_family = AF_INET;
#ifdef IPV6_SUPPORTED
	else if (strcmp(nconf->nc_protofmly, NC_INET6) == 0)
		hint.ai_family = AF_INET6;
#endif	/* IPV6_SUPPORTED */
	else {
		xlog(L_ERROR, "Unrecognized bind address family: %s",
			nconf->nc_protofmly);
		return NULL;
	}

	if (strcmp(nconf->nc_proto, NC_UDP) == 0)
		hint.ai_protocol = (int)IPPROTO_UDP;
	else if (strcmp(nconf->nc_proto, NC_TCP) == 0)
		hint.ai_protocol = (int)IPPROTO_TCP;
	else {
		xlog(L_ERROR, "Unrecognized bind address protocol: %s",
			nconf->nc_proto);
		return NULL;
	}

	(void)snprintf(buf, sizeof(buf), "%u", port);
	error = getaddrinfo(NULL, buf, &hint, &ai);
	if (error != 0) {
		xlog(L_ERROR, "Failed to construct bind address: %s",
			gai_strerror(error));
		return NULL;
	}

	return ai;
}

/*
 * Create a listener socket on a specific bindaddr, and set
 * special socket options to allow it to share the same port
 * as other listeners.
 *
 * Returns an open, bound, and possibly listening network
 * socket on success.
 *
 * Otherwise returns -1 if some error occurs.
 */
static int
svc_create_sock(const struct sockaddr *sap, socklen_t salen,
		struct netconfig *nconf)
{
	int fd, type, protocol;
	int one = 1;

	switch(nconf->nc_semantics) {
	case NC_TPI_CLTS:
		type = SOCK_DGRAM;
		break;
	case NC_TPI_COTS_ORD:
		type = SOCK_STREAM;
		break;
	default:
		xlog(D_GENERAL, "%s: Unrecognized bind address semantics: %u",
			__func__, nconf->nc_semantics);
		return -1;
	}

	if (strcmp(nconf->nc_proto, NC_UDP) == 0)
		protocol = (int)IPPROTO_UDP;
	else if (strcmp(nconf->nc_proto, NC_TCP) == 0)
		protocol = (int)IPPROTO_TCP;
	else {
		xlog(D_GENERAL, "%s: Unrecognized bind address protocol: %s",
			__func__, nconf->nc_proto);
		return -1;
	}

	fd = socket((int)sap->sa_family, type, protocol);
	if (fd == -1) {
		xlog(L_ERROR, "Could not make a socket: (%d) %m",
			errno);
		return -1;
	}

#ifdef IPV6_SUPPORTED
	if (sap->sa_family == AF_INET6) {
		if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY,
				&one, sizeof(one)) == -1) {
			xlog(L_ERROR, "Failed to set IPV6_V6ONLY: (%d) %m",
				errno);
			(void)close(fd);
			return -1;
		}
	}
#endif	/* IPV6_SUPPORTED */

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
		       &one, sizeof(one)) == -1) {
		xlog(L_ERROR, "Failed to set SO_REUSEADDR: (%d) %m",
			errno);
		(void)close(fd);
		return -1;
	}

	if (bind(fd, sap, salen) == -1) {
		xlog(L_ERROR, "Could not bind socket: (%d) %m",
			errno);
		(void)close(fd);
		return -1;
	}

	if (nconf->nc_semantics == NC_TPI_COTS_ORD)
		if (listen(fd, SOMAXCONN) == -1) {
			xlog(L_ERROR, "Could not listen on socket: (%d) %m",
				errno);
			(void)close(fd);
			return -1;
		}

	return fd;
}

/*
 * The simple case is allowing the TI-RPC library to create a
 * transport itself, given just the bind address and transport
 * semantics.
 *
 * Our local xprt cache is ignored in this path, since the
 * caller is not interested in sharing listeners or ports, and
 * the library automatically avoids ports already in use.
 *
 * Returns the count of started listeners (one or zero).
 */
static unsigned int
svc_create_nconf_rand_port(const char *name, const rpcprog_t program,
		const rpcvers_t version,
		void (*dispatch)(struct svc_req *, SVCXPRT *),
		struct netconfig *nconf)
{
	struct t_bind bindaddr;
	struct addrinfo *ai;
	SVCXPRT	*xprt;

	ai = svc_create_bindaddr(nconf, 0);
	if (ai == NULL)
		return 0;

	bindaddr.addr.buf = ai->ai_addr;
	bindaddr.qlen = SOMAXCONN;

	xprt = svc_tli_create(RPC_ANYFD, nconf, &bindaddr, 0, 0);
	freeaddrinfo(ai);
	if (xprt == NULL) {
		xlog(L_ERROR, "Failed to create listener xprt "
			"(%s, %u, %s)", name, version, nconf->nc_netid);
		return 0;
	}
	if (svcsock_nonblock(xprt->xp_fd) < 0) {
		/* close() already done by svcsock_nonblock() */
		xprt->xp_fd = RPC_ANYFD;
		SVC_DESTROY(xprt);
		return 0;
	}

	rpc_createerr.cf_stat = rpc_createerr.cf_error.re_errno = 0;
	if (!svc_reg(xprt, program, version, dispatch, nconf)) {
		/* svc_reg(3) destroys @xprt in this case */
		xlog(L_ERROR, "Failed to register (%s, %u, %s): %s",
				name, version, nconf->nc_netid, 
				clnt_spcreateerror("svc_reg() err"));
		return 0;
	}

	return 1;
}

/*
 * If a port is specified on the command line, that port value will be
 * the same for all listeners created here.  Create each listener
 * socket in advance and set SO_REUSEADDR, rather than allowing the
 * RPC library to create the listeners for us on a randomly chosen
 * port via svc_tli_create(RPC_ANYFD).
 *
 * Some callers want to listen for more than one RPC version using the
 * same port number.  For example, mountd could want to listen for MNT
 * version 1, 2, and 3 requests.  This means mountd must use the same
 * set of listener sockets for multiple RPC versions, since, on one
 * system, you can't have two listener sockets with the exact same
 * bind address (and port) and transport protocol.
 *
 * To accomplish this, this function caches xprts as they are created.
 * This cache is checked to see if a previously created xprt can be
 * used, before creating a new xprt for this [program, version].  If
 * there is a cached xprt with the same bindaddr and transport
 * semantics, we simply register the new version with that xprt,
 * rather than creating a fresh xprt for it.
 *
 * The xprt cache implemented here is local to a process.  Two
 * separate RPC daemons can not share a set of listeners.
 *
 * Returns the count of started listeners (one or zero).
 */
static unsigned int
svc_create_nconf_fixed_port(const char *name, const rpcprog_t program,
		const rpcvers_t version,
		void (*dispatch)(struct svc_req *, SVCXPRT *),
		const uint16_t port, struct netconfig *nconf)
{
	struct addrinfo *ai;
	SVCXPRT	*xprt;

	ai = svc_create_bindaddr(nconf, port);
	if (ai == NULL)
		return 0;

	xprt = svc_create_find_xprt(ai->ai_addr, nconf);
	if (xprt == NULL) {
		int fd;

		fd = svc_create_sock(ai->ai_addr, ai->ai_addrlen, nconf);
		fd = svcsock_nonblock(fd);
		if (fd == -1)
			goto out_free;

		xprt = svc_tli_create(fd, nconf, NULL, 0, 0);
		if (xprt == NULL) {
			xlog(D_GENERAL, "Failed to create listener xprt "
				"(%s, %u, %s)", name, version, nconf->nc_netid);
			(void)close(fd);
			goto out_free;
		}
	}

	if (!svc_reg(xprt, program, version, dispatch, nconf)) {
		/* svc_reg(3) destroys @xprt in this case */
		xlog(D_GENERAL, "Failed to register (%s, %u, %s)",
				name, version, nconf->nc_netid);
		goto out_free;
	}

	svc_create_cache_xprt(xprt);

	freeaddrinfo(ai);
	return 1;

out_free:
	freeaddrinfo(ai);
	return 0;
}

static unsigned int
svc_create_nconf(const char *name, const rpcprog_t program,
		const rpcvers_t version,
		void (*dispatch)(struct svc_req *, SVCXPRT *),
		const uint16_t port, struct netconfig *nconf)
{
	if (port != 0)
		return svc_create_nconf_fixed_port(name, program,
			version, dispatch, port, nconf);

	return svc_create_nconf_rand_port(name, program,
			version, dispatch, nconf);
}

/**
 * nfs_svc_create - start up RPC svc listeners
 * @name: C string containing name of new service
 * @program: RPC program number to register
 * @version: RPC version number to register
 * @dispatch: address of function that handles incoming RPC requests
 * @port: if not zero, transport listens on this port
 *
 * Sets up network transports for receiving RPC requests, and starts
 * the RPC dispatcher.  Returns the number of started network transports.
 */
unsigned int
nfs_svc_create(char *name, const rpcprog_t program, const rpcvers_t version,
		void (*dispatch)(struct svc_req *, SVCXPRT *),
		const uint16_t port)
{
	const struct sigaction create_sigaction = {
		.sa_handler	= SIG_IGN,
	};
	int maxrec = RPC_MAXDATASIZE;
	unsigned int visible, up, servport;
	struct netconfig *nconf;
	void *handlep;

	/*
	 * Ignore SIGPIPE to avoid exiting sideways when peers
	 * close their TCP connection while we're trying to reply
	 * to them.
	 */
	(void)sigaction(SIGPIPE, &create_sigaction, NULL);

	/*
	 * Setting MAXREC also enables non-blocking mode for tcp connections.
	 * This avoids DOS attacks by a client sending many requests but never
	 * reading the reply:
	 * - if a second request already is present for reading in the socket,
	 *   after the first request just was read, libtirpc will break the
	 *   connection. Thus an attacker can't simply send requests as fast as
	 *   he can without waiting for the response.
	 * - if the write buffer of the socket is full, the next write() will
	 *   fail with EAGAIN. libtirpc will retry the write in a loop for max.
	 *   2 seconds. If write still fails, the connection will be closed.
	 */   
	rpc_control(RPC_SVC_CONNMAXREC_SET, &maxrec);

	handlep = setnetconfig();
	if (handlep == NULL) {
		xlog(L_ERROR, "Failed to access local netconfig database: %s",
			nc_sperror());
		return 0;
	}

	visible = 0;
	up = 0;
	while ((nconf = getnetconfig(handlep)) != NULL) {
		if (!(nconf->nc_flag & NC_VISIBLE))
			continue;
		visible++;

		if (!strcmp(nconf->nc_proto, NC_UDP) && !NFSCTL_UDPISSET(_rpcprotobits))
			continue;

		if (!strcmp(nconf->nc_proto, NC_TCP) && !NFSCTL_TCPISSET(_rpcprotobits))
			continue;

		if (port == 0)
			servport = getservport(program, nconf->nc_proto);
		else
			servport = port;

		up += svc_create_nconf(name, program, version, dispatch,
						servport, nconf);
	}

	if (visible == 0)
		xlog(L_ERROR, "Failed to find any visible netconfig entries");

	if (endnetconfig(handlep) == -1)
		xlog(L_ERROR, "Failed to close local netconfig database: %s",
			nc_sperror());

	return up;
}

/**
 * nfs_svc_unregister - remove service registrations from local rpcbind database
 * @program: RPC program number to unregister
 * @version: RPC version number to unregister
 *
 * Removes all registrations for [ @program, @version ] .
 */
void
nfs_svc_unregister(const rpcprog_t program, const rpcvers_t version)
{
	if (rpcb_unset(program, version, NULL) == FALSE)
		xlog(D_GENERAL, "Failed to unregister program %lu, version %lu",
			(unsigned long)program, (unsigned long)version);
}

#else	/* !HAVE_LIBTIRPC */

/**
 * nfs_svc_create - start up RPC svc listeners
 * @name: C string containing name of new service
 * @program: RPC program number to register
 * @version: RPC version number to register
 * @dispatch: address of function that handles incoming RPC requests
 * @port: if not zero, transport listens on this port
 *
 * Sets up network transports for receiving RPC requests, and starts
 * the RPC dispatcher.  Returns the number of started network transports.
 */
unsigned int
nfs_svc_create(char *name, const rpcprog_t program, const rpcvers_t version,
		void (*dispatch)(struct svc_req *, SVCXPRT *),
		const uint16_t port)
{
	rpc_init(name, (int)program, (int)version, dispatch, (int)port);
	return 1;
}

/**
 * nfs_svc_unregister - remove service registrations from local rpcbind database
 * @program: RPC program number to unregister
 * @version: RPC version number to unregister
 *
 * Removes all registrations for [ @program, @version ] .
 */
void
nfs_svc_unregister(const rpcprog_t program, const rpcvers_t version)
{
	if (pmap_unset((unsigned long)program, (unsigned long)version) == FALSE)
		xlog(D_GENERAL, "Failed to unregister program %lu, version %lu",
			(unsigned long)program, (unsigned long)version);
}

#endif	/* !HAVE_LIBTIRPC */
