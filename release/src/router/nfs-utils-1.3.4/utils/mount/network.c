/*
 * network.c -- Provide common network functions for NFS mount/umount
 *
 * Copyright (C) 2007 Oracle.  All rights reserved.
 * Copyright (C) 2007 Chuck Lever <chuck.lever@oracle.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 0211-1301 USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <linux/in6.h>
#include <netinet/in.h>
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <rpc/pmap_clnt.h>

#include "sockaddr.h"
#include "xcommon.h"
#include "mount.h"
#include "nls.h"
#include "nfs_mount.h"
#include "mount_constants.h"
#include "nfsrpc.h"
#include "parse_opt.h"
#include "network.h"
#include "conffile.h"
#include "nfslib.h"

#define PMAP_TIMEOUT	(10)
#define CONNECT_TIMEOUT	(20)
#define MOUNT_TIMEOUT	(30)
#define STATD_TIMEOUT	(10)

#define SAFE_SOCKADDR(x)	(struct sockaddr *)(char *)(x)

extern int nfs_mount_data_version;
extern char *progname;
extern int verbose;

static const char *nfs_mnt_pgmtbl[] = {
	"mount",
	"mountd",
	NULL,
};

static const char *nfs_nfs_pgmtbl[] = {
	"nfs",
	"nfsprog",
	NULL,
};

static const char *nfs_transport_opttbl[] = {
	"udp",
	"tcp",
	"rdma",
	"proto",
	NULL,
};

static const char *nfs_version_opttbl[] = {
	"v2",
	"v3",
	"v4",
	"vers",
	"nfsvers",
	"minorversion",
	NULL,
};

static const unsigned long nfs_to_mnt[] = {
	0,
	0,
	1,
	3,
};

static const unsigned long mnt_to_nfs[] = {
	0,
	2,
	2,
	3,
};

/*
 * Map an NFS version into the corresponding Mountd version
 */
unsigned long nfsvers_to_mnt(const unsigned long vers)
{
	if (vers <= 3)
		return nfs_to_mnt[vers];
	return 0;
}

/*
 * Map a Mountd version into the corresponding NFS version
 */
static unsigned long mntvers_to_nfs(const unsigned long vers)
{
	if (vers <= 3)
		return mnt_to_nfs[vers];
	return 0;
}

static const unsigned int probe_udp_only[] = {
	IPPROTO_UDP,
	0,
};

static const unsigned int probe_udp_first[] = {
	IPPROTO_UDP,
	IPPROTO_TCP,
	0,
};

static const unsigned int probe_tcp_first[] = {
	IPPROTO_TCP,
	IPPROTO_UDP,
	0,
};

static const unsigned long probe_nfs2_only[] = {
	2,
	0,
};

static const unsigned long probe_nfs3_only[] = {
	3,
	0,
};

static const unsigned long probe_mnt1_first[] = {
	1,
	2,
	0,
};

static const unsigned long probe_mnt3_only[] = {
	3,
	0,
};

static const unsigned int *nfs_default_proto(void);
#ifdef MOUNT_CONFIG
static const unsigned int *nfs_default_proto()
{
	extern unsigned long config_default_proto;
	/*
	 * If the default proto has been set and 
	 * its not TCP, start with UDP
	 */
	if (config_default_proto && config_default_proto != IPPROTO_TCP)
		return probe_udp_first;

	return probe_tcp_first; 
}
#else
static const unsigned int *nfs_default_proto() 
{
	return probe_tcp_first; 
}
#endif /* MOUNT_CONFIG */

/**
 * nfs_lookup - resolve hostname to an IPv4 or IPv6 socket address
 * @hostname: pointer to C string containing DNS hostname to resolve
 * @family: address family hint
 * @sap: pointer to buffer to fill with socket address
 * @len: IN: size of buffer to fill; OUT: size of socket address
 *
 * Returns 1 and places a socket address at @sap if successful;
 * otherwise zero.
 */
int nfs_lookup(const char *hostname, const sa_family_t family,
		struct sockaddr *sap, socklen_t *salen)
{
	struct addrinfo *gai_results;
	struct addrinfo gai_hint = {
		.ai_family	= family,
	};
	socklen_t len = *salen;
	int error, ret = 0;

	*salen = 0;

	error = getaddrinfo(hostname, NULL, &gai_hint, &gai_results);
	switch (error) {
	case 0:
		break;
	case EAI_SYSTEM:
		nfs_error(_("%s: DNS resolution failed for %s: %s"),
			progname, hostname, strerror(errno));
		return ret;
	default:
		nfs_error(_("%s: DNS resolution failed for %s: %s"),
			progname, hostname, gai_strerror(error));
		return ret;
	}

	switch (gai_results->ai_addr->sa_family) {
	case AF_INET:
	case AF_INET6:
		if (len >= gai_results->ai_addrlen) {
			*salen = gai_results->ai_addrlen;
			memcpy(sap, gai_results->ai_addr, *salen);
			ret = 1;
		}
		break;
	default:
		/* things are really broken if we get here, so warn */
		nfs_error(_("%s: unrecognized DNS resolution results for %s"),
				progname, hostname);
		break;
	}

	freeaddrinfo(gai_results);
	return ret;
}

/**
 * nfs_gethostbyname - resolve a hostname to an IPv4 address
 * @hostname: pointer to a C string containing a DNS hostname
 * @sin: returns an IPv4 address 
 *
 * Returns 1 if successful, otherwise zero.
 */
int nfs_gethostbyname(const char *hostname, struct sockaddr_in *sin)
{
	socklen_t len = sizeof(*sin);

	return nfs_lookup(hostname, AF_INET, (struct sockaddr *)sin, &len);
}

/**
 * nfs_string_to_sockaddr - convert string address to sockaddr
 * @address:	pointer to presentation format address to convert
 * @sap:	pointer to socket address buffer to fill in
 * @salen:	IN: length of address buffer
 *		OUT: length of converted socket address
 *
 * Convert a presentation format address string to a socket address.
 * Similar to nfs_lookup(), but the DNS query is squelched, and it
 * won't make any noise if the getaddrinfo() call fails.
 *
 * Returns 1 and fills in @sap and @salen if successful; otherwise zero.
 *
 * See RFC 4038 section 5.1 or RFC 3513 section 2.2 for more details
 * on presenting IPv6 addresses as text strings.
 */
int nfs_string_to_sockaddr(const char *address, struct sockaddr *sap,
			   socklen_t *salen)
{
	struct addrinfo *gai_results;
	struct addrinfo gai_hint = {
		.ai_flags	= AI_NUMERICHOST,
	};
	socklen_t len = *salen;
	int ret = 0;

	*salen = 0;

	if (getaddrinfo(address, NULL, &gai_hint, &gai_results) == 0) {
		switch (gai_results->ai_addr->sa_family) {
		case AF_INET:
		case AF_INET6:
			if (len >= gai_results->ai_addrlen) {
				*salen = gai_results->ai_addrlen;
				memcpy(sap, gai_results->ai_addr, *salen);
				ret = 1;
			}
			break;
		}
		freeaddrinfo(gai_results);
	}

	return ret;
}

/**
 * nfs_present_sockaddr - convert sockaddr to string
 * @sap: pointer to socket address to convert
 * @salen: length of socket address
 * @buf: pointer to buffer to fill in
 * @buflen: length of buffer
 *
 * Convert the passed-in sockaddr-style address to presentation format.
 * The presentation format address is placed in @buf and is
 * '\0'-terminated.
 *
 * Returns 1 if successful; otherwise zero.
 *
 * See RFC 4038 section 5.1 or RFC 3513 section 2.2 for more details
 * on presenting IPv6 addresses as text strings.
 */
int nfs_present_sockaddr(const struct sockaddr *sap, const socklen_t salen,
			 char *buf, const size_t buflen)
{
#ifdef HAVE_GETNAMEINFO
	int result;

	result = getnameinfo(sap, salen, buf, buflen,
					NULL, 0, NI_NUMERICHOST);
	if (!result)
		return 1;

	nfs_error(_("%s: invalid server address: %s"), progname,
			gai_strerror(result));
	return 0;
#else	/* HAVE_GETNAMEINFO */
	char *addr;

	if (sap->sa_family == AF_INET) {
		addr = inet_ntoa(((struct sockaddr_in *)sap)->sin_addr);
		if (addr && strlen(addr) < buflen) {
			strcpy(buf, addr);
			return 1;
		}
	}

	nfs_error(_("%s: invalid server address"), progname);
	return 0;
#endif	/* HAVE_GETNAMEINFO */
}

/*
 * Attempt to connect a socket, but time out after "timeout" seconds.
 *
 * On error return, caller closes the socket.
 */
static int connect_to(int fd, struct sockaddr *addr,
			socklen_t addrlen, int timeout)
{
	int ret, saved;
	fd_set rset, wset;
	struct timeval tv = {
		.tv_sec = timeout,
	};

	saved = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, saved | O_NONBLOCK);

	ret = connect(fd, addr, addrlen);
	if (ret < 0 && errno != EINPROGRESS)
		return -1;
	if (ret == 0)
		goto out;

	FD_ZERO(&rset);
	FD_SET(fd, &rset);
	wset = rset;
	ret = select(fd + 1, &rset, &wset, NULL, &tv);
	if (ret == 0) {
		errno = ETIMEDOUT;
		return -1;
	}
	if (FD_ISSET(fd, &rset) || FD_ISSET(fd, &wset)) {
		int error;
		socklen_t len = sizeof(error);
		if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
			return -1;
		if (error) {
			errno = error;
			return -1;
		}
	} else
		return -1;

out:
	fcntl(fd, F_SETFL, saved);
	return 0;
}

/*
 * Create a socket that is locally bound to a reserved or non-reserved port.
 *
 * The caller should check rpc_createerr to determine the cause of any error.
 */
static int get_socket(struct sockaddr_in *saddr, unsigned int p_prot,
			unsigned int timeout, int resvp, int conn)
{
	int so, cc, type;
	struct sockaddr_in laddr;
	socklen_t namelen = sizeof(laddr);

	type = (p_prot == IPPROTO_UDP ? SOCK_DGRAM : SOCK_STREAM);
	if ((so = socket (AF_INET, type, p_prot)) < 0)
		goto err_socket;

	laddr.sin_family = AF_INET;
	laddr.sin_port = 0;
	laddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (resvp) {
		if (bindresvport(so, &laddr) < 0)
			goto err_bindresvport;
	} else {
		cc = bind(so, SAFE_SOCKADDR(&laddr), namelen);
		if (cc < 0)
			goto err_bind;
	}
	if (type == SOCK_STREAM || (conn && type == SOCK_DGRAM)) {
		cc = connect_to(so, SAFE_SOCKADDR(saddr), namelen,
				timeout);
		if (cc < 0)
			goto err_connect;
	}
	return so;

err_socket:
	rpc_createerr.cf_stat = RPC_SYSTEMERROR;
	rpc_createerr.cf_error.re_errno = errno;
	if (verbose) {
		nfs_error(_("%s: Unable to create %s socket: errno %d (%s)\n"),
			progname, p_prot == IPPROTO_UDP ? _("UDP") : _("TCP"),
			errno, strerror(errno));
	}
	return RPC_ANYSOCK;

err_bindresvport:
	rpc_createerr.cf_stat = RPC_SYSTEMERROR;
	rpc_createerr.cf_error.re_errno = errno;
	if (verbose) {
		nfs_error(_("%s: Unable to bindresvport %s socket: errno %d"
				" (%s)\n"),
			progname, p_prot == IPPROTO_UDP ? _("UDP") : _("TCP"),
			errno, strerror(errno));
	}
	close(so);
	return RPC_ANYSOCK;

err_bind:
	rpc_createerr.cf_stat = RPC_SYSTEMERROR;
	rpc_createerr.cf_error.re_errno = errno;
	if (verbose) {
		nfs_error(_("%s: Unable to bind to %s socket: errno %d (%s)\n"),
			progname, p_prot == IPPROTO_UDP ? _("UDP") : _("TCP"),
			errno, strerror(errno));
	}
	close(so);
	return RPC_ANYSOCK;

err_connect:
	rpc_createerr.cf_stat = RPC_SYSTEMERROR;
	rpc_createerr.cf_error.re_errno = errno;
	if (verbose) {
		nfs_error(_("%s: Unable to connect to %s:%d, errno %d (%s)\n"),
			progname, inet_ntoa(saddr->sin_addr),
			ntohs(saddr->sin_port), errno, strerror(errno));
	}
	close(so);
	return RPC_ANYSOCK;
}

static void nfs_pp_debug(const struct sockaddr *sap, const socklen_t salen,
			 const rpcprog_t program, const rpcvers_t version,
			 const unsigned short protocol,
			 const unsigned short port)
{
	char buf[NI_MAXHOST];

	if (!verbose)
		return;

	if (nfs_present_sockaddr(sap, salen, buf, sizeof(buf)) == 0) {
		buf[0] = '\0';
		strcat(buf, "unknown host");
	}

	fprintf(stderr, _("%s: trying %s prog %lu vers %lu prot %s port %d\n"),
			progname, buf, (unsigned long)program,
			(unsigned long)version,
			(protocol == IPPROTO_UDP ? _("UDP") : _("TCP")),
			port);
}

static void nfs_pp_debug2(const char *str)
{
	if (!verbose)
		return;

	if (rpc_createerr.cf_error.re_status == RPC_CANTRECV ||
	    rpc_createerr.cf_error.re_status == RPC_CANTSEND)
		nfs_error(_("%s: portmap query %s%s - %s"),
				progname, str, clnt_spcreateerror(""),
				strerror(rpc_createerr.cf_error.re_errno));
	else
		nfs_error(_("%s: portmap query %s%s"),
				progname, str, clnt_spcreateerror(""));
}

/*
 * Use the portmapper to discover whether or not the service we want is
 * available. The lists 'versions' and 'protos' define ordered sequences
 * of service versions and udp/tcp protocols to probe for.
 *
 * Returns 1 if the requested service port is unambiguous and pingable;
 * @pmap is filled in with the version, port, and transport protocol used
 * during the successful ping.  Note that if a port is already specified
 * in @pmap and it matches the rpcbind query result, nfs_probe_port() does
 * not perform an RPC ping.
 * 
 * If an error occurs or the requested service isn't available, zero is
 * returned; rpccreateerr.cf_stat is set to reflect the nature of the error.
 */
static int nfs_probe_port(const struct sockaddr *sap, const socklen_t salen,
			  struct pmap *pmap, const unsigned long *versions,
			  const unsigned int *protos)
{
	union nfs_sockaddr address;
	struct sockaddr *saddr = &address.sa;
	const unsigned long prog = pmap->pm_prog, *p_vers;
	const unsigned int prot = (u_int)pmap->pm_prot, *p_prot;
	const u_short port = (u_short) pmap->pm_port;
	unsigned long vers = pmap->pm_vers;
	unsigned short p_port;

	memcpy(saddr, sap, salen);
	p_prot = prot ? &prot : protos;
	p_vers = vers ? &vers : versions;

	for (;;) {
		if (verbose)
			printf(_("%s: prog %lu, trying vers=%lu, prot=%u\n"),
				progname, prog, *p_vers, *p_prot);
		p_port = nfs_getport(saddr, salen, prog, *p_vers, *p_prot);
		if (p_port) {
			if (!port || port == p_port) {
				nfs_set_port(saddr, p_port);
				nfs_pp_debug(saddr, salen, prog, *p_vers,
						*p_prot, p_port);
				if (nfs_rpc_ping(saddr, salen, prog,
							*p_vers, *p_prot, NULL))
					goto out_ok;
			} else
				rpc_createerr.cf_stat = RPC_PROGNOTREGISTERED;
		}
		if (rpc_createerr.cf_stat != RPC_PROGNOTREGISTERED &&
		    rpc_createerr.cf_stat != RPC_TIMEDOUT &&
		    rpc_createerr.cf_stat != RPC_CANTRECV &&
		    rpc_createerr.cf_stat != RPC_PROGVERSMISMATCH)
			break;

		if (!prot) {
			if (*++p_prot) {
				nfs_pp_debug2("retrying");
				continue;
			}
			p_prot = protos;
		}
		if (rpc_createerr.cf_stat == RPC_TIMEDOUT ||
		    rpc_createerr.cf_stat == RPC_CANTRECV)
			break;

		if (vers || !*++p_vers)
			break;
	}

	nfs_pp_debug2("failed");
	return 0;

out_ok:
	if (!vers)
		pmap->pm_vers = *p_vers;
	if (!prot)
		pmap->pm_prot = *p_prot;
	if (!port)
		pmap->pm_port = p_port;
	nfs_clear_rpc_createerr();
	return 1;
}
/*
 * Probe a server's NFS service to determine which versions and
 * transport protocols are supported.
 *
 * Returns 1 if the requested service port is unambiguous and pingable;
 * @pmap is filled in with the version, port, and transport protocol used
 * during the successful ping.  If all three are already specified, simply
 * return success without an rpcbind query or RPC ping (we may be trying
 * to mount an NFS service that is not advertised via rpcbind).
 *
 * If an error occurs or the requested service isn't available, zero is
 * returned; rpccreateerr.cf_stat is set to reflect the nature of the error.
 */
static int nfs_probe_nfsport(const struct sockaddr *sap, const socklen_t salen,
			     struct pmap *pmap, int checkv4)
{
	if (pmap->pm_vers && pmap->pm_prot && pmap->pm_port)
		return 1;

	if (nfs_mount_data_version >= 4) {
		const unsigned int *probe_proto;
		int ret;
		struct sockaddr_storage save_sa;

		probe_proto = nfs_default_proto();
		memcpy(&save_sa, sap, salen);

		ret = nfs_probe_port(sap, salen, pmap,
				     probe_nfs3_only, probe_proto);
		if (!ret || !checkv4 || probe_proto != probe_tcp_first)
			return ret;

		nfs_set_port((struct sockaddr *)&save_sa, NFS_PORT);
		ret =  nfs_rpc_ping((struct sockaddr *)&save_sa, salen, 
			NFS_PROGRAM, 4, IPPROTO_TCP, NULL);
		if (ret) {
			rpc_createerr.cf_stat = RPC_FAILED;
			rpc_createerr.cf_error.re_errno = EAGAIN;
			return 0;
		}
		return 1;
	} else
		return nfs_probe_port(sap, salen, pmap,
					probe_nfs2_only, probe_udp_only);
}

/*
 * Probe a server's mountd service to determine which versions and
 * transport protocols are supported.
 *
 * Returns 1 if the requested service port is unambiguous and pingable;
 * @pmap is filled in with the version, port, and transport protocol used
 * during the successful ping.  If all three are already specified, simply
 * return success without an rpcbind query or RPC ping (we may be trying
 * to mount an NFS service that is not advertised via rpcbind).
 * 
 * If an error occurs or the requested service isn't available, zero is
 * returned; rpccreateerr.cf_stat is set to reflect the nature of the error.
 */
static int nfs_probe_mntport(const struct sockaddr *sap, const socklen_t salen,
				struct pmap *pmap)
{
	if (pmap->pm_vers && pmap->pm_prot && pmap->pm_port)
		return 1;

	if (nfs_mount_data_version >= 4)
		return nfs_probe_port(sap, salen, pmap,
					probe_mnt3_only, probe_udp_first);
	else
		return nfs_probe_port(sap, salen, pmap,
					probe_mnt1_first, probe_udp_only);
}

/*
 * Probe a server's mountd service to determine which versions and
 * transport protocols are supported.  Invoked when the protocol
 * version is already known for both the NFS and mountd service.
 *
 * Returns 1 and fills in both @pmap structs if the requested service
 * ports are unambiguous and pingable.  Otherwise zero is returned;
 * rpccreateerr.cf_stat is set to reflect the nature of the error.
 */
static int nfs_probe_version_fixed(const struct sockaddr *mnt_saddr,
			const socklen_t mnt_salen,
			struct pmap *mnt_pmap,
			const struct sockaddr *nfs_saddr,
			const socklen_t nfs_salen,
			struct pmap *nfs_pmap)
{
	if (!nfs_probe_nfsport(nfs_saddr, nfs_salen, nfs_pmap, 0))
		return 0;
	return nfs_probe_mntport(mnt_saddr, mnt_salen, mnt_pmap);
}

/**
 * nfs_probe_bothports - discover the RPC endpoints of mountd and NFS server
 * @mnt_saddr:	pointer to socket address of mountd server
 * @mnt_salen:	length of mountd server's address
 * @mnt_pmap:	IN: partially filled-in mountd RPC service tuple;
 *		OUT: fully filled-in mountd RPC service tuple
 * @nfs_saddr:	pointer to socket address of NFS server
 * @nfs_salen:	length of NFS server's address
 * @nfs_pmap:	IN: partially filled-in NFS RPC service tuple;
 *		OUT: fully filled-in NFS RPC service tuple
 * @checkv4:	Flag indicating that if v3 is available we must also
 *		check v4, and if that is available, set re_errno to EAGAIN.
 *
 * Returns 1 and fills in both @pmap structs if the requested service
 * ports are unambiguous and pingable.  Otherwise zero is returned;
 * rpccreateerr.cf_stat is set to reflect the nature of the error.
 */
int nfs_probe_bothports(const struct sockaddr *mnt_saddr,
			const socklen_t mnt_salen,
			struct pmap *mnt_pmap,
			const struct sockaddr *nfs_saddr,
			const socklen_t nfs_salen,
			struct pmap *nfs_pmap,
			int checkv4)
{
	struct pmap save_nfs, save_mnt;
	const unsigned long *probe_vers;

	if (mnt_pmap->pm_vers && !nfs_pmap->pm_vers)
		nfs_pmap->pm_vers = mntvers_to_nfs(mnt_pmap->pm_vers);
	else if (nfs_pmap->pm_vers && !mnt_pmap->pm_vers)
		mnt_pmap->pm_vers = nfsvers_to_mnt(nfs_pmap->pm_vers);

	if (nfs_pmap->pm_vers)
		return nfs_probe_version_fixed(mnt_saddr, mnt_salen, mnt_pmap,
					       nfs_saddr, nfs_salen, nfs_pmap);

	memcpy(&save_nfs, nfs_pmap, sizeof(save_nfs));
	memcpy(&save_mnt, mnt_pmap, sizeof(save_mnt));
	probe_vers = (nfs_mount_data_version >= 4) ?
			probe_mnt3_only : probe_mnt1_first;

	for (; *probe_vers; probe_vers++) {
		nfs_pmap->pm_vers = mntvers_to_nfs(*probe_vers);
		if (nfs_probe_nfsport(nfs_saddr, nfs_salen, nfs_pmap, checkv4) != 0) {
			mnt_pmap->pm_vers = *probe_vers;
			if (nfs_probe_mntport(mnt_saddr, mnt_salen, mnt_pmap) != 0)
				return 1;
			memcpy(mnt_pmap, &save_mnt, sizeof(*mnt_pmap));
		}
		switch (rpc_createerr.cf_stat) {
		case RPC_PROGVERSMISMATCH:
		case RPC_PROGNOTREGISTERED:
			break;
		default:
			return 0;
		}
		memcpy(nfs_pmap, &save_nfs, sizeof(*nfs_pmap));
	}

	return 0;
}

/**
 * probe_bothports - discover the RPC endpoints of mountd and NFS server
 * @mnt_server: pointer to address and pmap argument for mountd results
 * @nfs_server: pointer to address and pmap argument for NFS server
 *
 * This is the legacy API that takes "clnt_addr_t" for both servers,
 * but supports only AF_INET addresses.
 *
 * Returns 1 and fills in the pmap field in both clnt_addr_t structs
 * if the requested service ports are unambiguous and pingable.
 * Otherwise zero is returned; rpccreateerr.cf_stat is set to reflect
 * the nature of the error.
 */
int probe_bothports(clnt_addr_t *mnt_server, clnt_addr_t *nfs_server)
{
	struct sockaddr *mnt_addr = SAFE_SOCKADDR(&mnt_server->saddr);
	struct sockaddr *nfs_addr = SAFE_SOCKADDR(&nfs_server->saddr);

	return nfs_probe_bothports(mnt_addr, sizeof(mnt_server->saddr),
					&mnt_server->pmap,
					nfs_addr, sizeof(nfs_server->saddr),
					&nfs_server->pmap, 0);
}

/**
 * start_statd - attempt to start rpc.statd
 *
 * Returns 1 if statd is running; otherwise zero.
 */
int start_statd(void)
{
#ifdef START_STATD
	struct stat stb;
#endif

	if (nfs_probe_statd())
		return 1;

#ifdef START_STATD
	if (stat(START_STATD, &stb) == 0) {
		if (S_ISREG(stb.st_mode) && (stb.st_mode & S_IXUSR)) {
			int cnt = STATD_TIMEOUT * 10;
			int status = 0;
			char * const envp[1] = { NULL };
			const struct timespec ts = {
				.tv_sec = 0,
				.tv_nsec = 100000000,
			};
			pid_t pid = fork();
			switch (pid) {
			case 0: /* child */
				setgid(0);
				setuid(0);
				execle(START_STATD, START_STATD, NULL, envp);
				exit(1);
			case -1: /* error */
				nfs_error(_("%s: fork failed: %s"),
						progname, strerror(errno));
				break;
			default: /* parent */
				if (waitpid(pid, &status,0) == pid &&
				    status == 0)
					/* assume it worked */
					return 1;
				break;
			}
			while (1) {
				if (nfs_probe_statd())
					return 1;
				if (! cnt--)
					return 0;
				nanosleep(&ts, NULL);
			}
		}
	}
#endif

	return 0;
}

/**
 * nfs_advise_umount - ask the server to remove a share from it's rmtab
 * @sap: pointer to IP address of server to call
 * @salen: length of server address
 * @pmap: partially filled-in mountd RPC service tuple
 * @argp: directory path of share to "unmount"
 *
 * Returns one if the unmount call succeeded; zero if the unmount
 * failed for any reason;  rpccreateerr.cf_stat is set to reflect
 * the nature of the error.
 *
 * We use a fast timeout since this call is advisory only.
 */
int nfs_advise_umount(const struct sockaddr *sap, const socklen_t salen,
		      const struct pmap *pmap, const dirpath *argp)
{
	union nfs_sockaddr address;
	struct sockaddr *saddr = &address.sa;
	struct pmap mnt_pmap = *pmap;
	struct timeval timeout = {
		.tv_sec		= MOUNT_TIMEOUT >> 3,
	};
	CLIENT *client;
	enum clnt_stat res = 0;

	memcpy(saddr, sap, salen);
	if (nfs_probe_mntport(saddr, salen, &mnt_pmap) == 0) {
		if (verbose)
			nfs_error(_("%s: Failed to discover mountd port%s"),
				progname, clnt_spcreateerror(""));
		return 0;
	}
	nfs_set_port(saddr, mnt_pmap.pm_port);

	client = nfs_get_priv_rpcclient(saddr, salen, mnt_pmap.pm_prot,
					mnt_pmap.pm_prog, mnt_pmap.pm_vers,
					&timeout);
	if (client == NULL) {
		if (verbose)
			nfs_error(_("%s: Failed to create RPC client%s"),
				progname, clnt_spcreateerror(""));
		return 0;
	}

	client->cl_auth = nfs_authsys_create();
	if (client->cl_auth == NULL) {
		if (verbose)
			nfs_error(_("%s: Failed to create RPC auth handle"),
				progname);
		CLNT_DESTROY(client);
		return 0;
	}

	res = CLNT_CALL(client, MOUNTPROC_UMNT,
			(xdrproc_t)xdr_dirpath, (caddr_t)argp,
			(xdrproc_t)xdr_void, NULL,
			timeout);
	if (res != RPC_SUCCESS) {
		rpc_createerr.cf_stat = res;
		CLNT_GETERR(client, &rpc_createerr.cf_error);
		if (verbose)
			nfs_error(_("%s: UMNT call failed: %s"),
				progname, clnt_sperrno(res));

	}
	auth_destroy(client->cl_auth);
	CLNT_DESTROY(client);

	if (res != RPC_SUCCESS)
		return 0;
	return 1;
}

/**
 * nfs_call_umount - ask the server to remove a share from it's rmtab
 * @mnt_server: address of RPC MNT program server
 * @argp: directory path of share to "unmount"
 *
 * Returns one if the unmount call succeeded; zero if the unmount
 * failed for any reason.
 *
 * Note that a side effect of calling this function is that rpccreateerr
 * is set.
 */
int nfs_call_umount(clnt_addr_t *mnt_server, dirpath *argp)
{
	struct sockaddr *sap = SAFE_SOCKADDR(&mnt_server->saddr);
	socklen_t salen = sizeof(mnt_server->saddr);
	struct pmap *pmap = &mnt_server->pmap;
	CLIENT *clnt;
	enum clnt_stat res = 0;
	int msock;

	if (!nfs_probe_mntport(sap, salen, pmap))
		return 0;
	clnt = mnt_openclnt(mnt_server, &msock);
	if (!clnt)
		return 0;
	res = clnt_call(clnt, MOUNTPROC_UMNT,
			(xdrproc_t)xdr_dirpath, (caddr_t)argp,
			(xdrproc_t)xdr_void, NULL,
			TIMEOUT);
	mnt_closeclnt(clnt, msock);

	if (res == RPC_SUCCESS)
		return 1;
	return 0;
}

/**
 * mnt_openclnt - get a handle for a remote mountd service
 * @mnt_server: address and pmap arguments of mountd service
 * @msock: returns a file descriptor of the underlying transport socket
 *
 * Returns an active handle for the remote's mountd service
 */
CLIENT *mnt_openclnt(clnt_addr_t *mnt_server, int *msock)
{
	struct sockaddr_in *mnt_saddr = &mnt_server->saddr;
	struct pmap *mnt_pmap = &mnt_server->pmap;
	CLIENT *clnt = NULL;

	mnt_saddr->sin_port = htons((u_short)mnt_pmap->pm_port);
	*msock = get_socket(mnt_saddr, mnt_pmap->pm_prot, MOUNT_TIMEOUT,
				TRUE, FALSE);
	if (*msock == RPC_ANYSOCK) {
		if (rpc_createerr.cf_error.re_errno == EADDRINUSE)
			/*
			 * Probably in-use by a TIME_WAIT connection,
			 * It is worth waiting a while and trying again.
			 */
			rpc_createerr.cf_stat = RPC_TIMEDOUT;
		return NULL;
	}

	switch (mnt_pmap->pm_prot) {
	case IPPROTO_UDP:
		clnt = clntudp_bufcreate(mnt_saddr,
					 mnt_pmap->pm_prog, mnt_pmap->pm_vers,
					 RETRY_TIMEOUT, msock,
					 MNT_SENDBUFSIZE, MNT_RECVBUFSIZE);
		break;
	case IPPROTO_TCP:
		clnt = clnttcp_create(mnt_saddr,
				      mnt_pmap->pm_prog, mnt_pmap->pm_vers,
				      msock,
				      MNT_SENDBUFSIZE, MNT_RECVBUFSIZE);
		break;
	}
	if (clnt) {
		/* try to mount hostname:dirname */
		clnt->cl_auth = nfs_authsys_create();
		if (clnt->cl_auth)
			return clnt;
		CLNT_DESTROY(clnt);
	}
	return NULL;
}

/**
 * mnt_closeclnt - terminate a handle for a remote mountd service
 * @clnt: pointer to an active handle for a remote mountd service
 * @msock: file descriptor of the underlying transport socket
 *
 */
void mnt_closeclnt(CLIENT *clnt, int msock)
{
	auth_destroy(clnt->cl_auth);
	clnt_destroy(clnt);
	close(msock);
}

/**
 * clnt_ping - send an RPC ping to the remote RPC service endpoint
 * @saddr: server's address
 * @prog: target RPC program number
 * @vers: target RPC version number
 * @prot: target RPC protocol
 * @caddr: filled in with our network address
 *
 * Sigh... GETPORT queries don't actually check the version number.
 * In order to make sure that the server actually supports the service
 * we're requesting, we open an RPC client, and fire off a NULL
 * RPC call.
 *
 * caddr is the network address that the server will use to call us back.
 * On multi-homed clients, this address depends on which NIC we use to
 * route requests to the server.
 *
 * Returns one if successful, otherwise zero.
 */
int clnt_ping(struct sockaddr_in *saddr, const unsigned long prog,
		const unsigned long vers, const unsigned int prot,
		struct sockaddr_in *caddr)
{
	CLIENT *clnt = NULL;
	int sock, status;
	static char clnt_res;
	struct sockaddr dissolve;

	rpc_createerr.cf_stat = status = 0;
	sock = get_socket(saddr, prot, CONNECT_TIMEOUT, FALSE, TRUE);
	if (sock == RPC_ANYSOCK) {
		if (rpc_createerr.cf_error.re_errno == ETIMEDOUT) {
			/*
			 * TCP timeout. Bubble up the error to see 
			 * how it should be handled.
			 */
			rpc_createerr.cf_stat = RPC_TIMEDOUT;
		}
		return 0;
	}

	if (caddr) {
		/* Get the address of our end of this connection */
		socklen_t len = sizeof(*caddr);
		if (getsockname(sock, caddr, &len) != 0)
			caddr->sin_family = 0;
	}

	switch(prot) {
	case IPPROTO_UDP:
		/* The socket is connected (so we could getsockname successfully),
		 * but some servers on multi-homed hosts reply from
		 * the wrong address, so if we stay connected, we lose the reply.
		 */
		dissolve.sa_family = AF_UNSPEC;
		connect(sock, &dissolve, sizeof(dissolve));

		clnt = clntudp_bufcreate(saddr, prog, vers,
					 RETRY_TIMEOUT, &sock,
					 RPCSMALLMSGSIZE, RPCSMALLMSGSIZE);
		break;
	case IPPROTO_TCP:
		clnt = clnttcp_create(saddr, prog, vers, &sock,
				      RPCSMALLMSGSIZE, RPCSMALLMSGSIZE);
		break;
	}
	if (!clnt) {
		close(sock);
		return 0;
	}
	memset(&clnt_res, 0, sizeof(clnt_res));
	status = clnt_call(clnt, NULLPROC,
			 (xdrproc_t)xdr_void, (caddr_t)NULL,
			 (xdrproc_t)xdr_void, (caddr_t)&clnt_res,
			 TIMEOUT);
	if (status) {
		clnt_geterr(clnt, &rpc_createerr.cf_error);
		rpc_createerr.cf_stat = status;
	}
	clnt_destroy(clnt);
	close(sock);

	if (status == RPC_SUCCESS)
		return 1;
	else
		return 0;
}

/*
 * Try a getsockname() on a connected datagram socket.
 *
 * Returns 1 and fills in @buf if successful; otherwise, zero.
 *
 * A connected datagram socket prevents leaving a socket in TIME_WAIT.
 * This conserves the ephemeral port number space, helping reduce failed
 * socket binds during mount storms.
 */
static int nfs_ca_sockname(const struct sockaddr *sap, const socklen_t salen,
			   struct sockaddr *buf, socklen_t *buflen)
{
	struct sockaddr_in sin = {
		.sin_family		= AF_INET,
		.sin_addr.s_addr	= htonl(INADDR_ANY),
	};
	struct sockaddr_in6 sin6 = {
		.sin6_family		= AF_INET6,
		.sin6_addr		= IN6ADDR_ANY_INIT,
	};
	int sock, result = 0;
	int val;

	sock = socket(sap->sa_family, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0)
		return 0;

	switch (sap->sa_family) {
	case AF_INET:
		if (bind(sock, SAFE_SOCKADDR(&sin), sizeof(sin)) < 0)
			goto out;
		break;
	case AF_INET6:
		/* Make sure the call-back address is public/permanent */
		val = IPV6_PREFER_SRC_PUBLIC;
		setsockopt(sock, SOL_IPV6, IPV6_ADDR_PREFERENCES, &val, sizeof(val));
		if (bind(sock, SAFE_SOCKADDR(&sin6), sizeof(sin6)) < 0)
			goto out;
		break;
	default:
		errno = EAFNOSUPPORT;
		goto out;
	}

	if (connect(sock, sap, salen) < 0)
		goto out;

	result = !getsockname(sock, buf, buflen);

out:
	close(sock);
	return result;
}

/*
 * Try to generate an address that prevents the server from calling us.
 *
 * Returns 1 and fills in @buf if successful; otherwise, zero.
 */
static int nfs_ca_gai(const struct sockaddr *sap,
		      struct sockaddr *buf, socklen_t *buflen)
{
	struct addrinfo *gai_results;
	struct addrinfo gai_hint = {
		.ai_family	= sap->sa_family,
		.ai_flags	= AI_PASSIVE,	/* ANYADDR */
	};

	if (getaddrinfo(NULL, "", &gai_hint, &gai_results))
		return 0;

	*buflen = gai_results->ai_addrlen;
	memcpy(buf, gai_results->ai_addr, *buflen);

	freeaddrinfo(gai_results);

	return 1;
}

/**
 * nfs_callback_address - acquire our local network address
 * @sap: pointer to address of remote
 * @sap_len: length of address
 * @buf: pointer to buffer to be filled in with local network address
 * @buflen: IN: length of buffer to fill in; OUT: length of filled-in address
 *
 * Discover a network address that an NFSv4 server can use to call us back.
 * On multi-homed clients, this address depends on which NIC we use to
 * route requests to the server.
 *
 * Returns 1 and fills in @buf if an unambiguous local address is
 * available; returns 1 and fills in an appropriate ANYADDR address
 * if a local address isn't available; otherwise, returns zero.
 */
int nfs_callback_address(const struct sockaddr *sap, const socklen_t salen,
			 struct sockaddr *buf, socklen_t *buflen)
{
	struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)buf;

	if (nfs_ca_sockname(sap, salen, buf, buflen) == 0)
		if (nfs_ca_gai(sap, buf, buflen) == 0)
			goto out_failed;

	/*
	 * The server can't use an interface ID that was generated
	 * here on the client, so always clear sin6_scope_id.
	 */
	if (sin6->sin6_family == AF_INET6)
		sin6->sin6_scope_id = 0;

	return 1;

out_failed:
	*buflen = 0;
	if (verbose)
		nfs_error(_("%s: failed to construct callback address"),
				progname);
	return 0;
}

/*
 * "nfsprog" is supported only by the legacy mount command.  The
 * kernel mount client does not support this option.
 *
 * Returns TRUE if @program contains a valid value for this option,
 * or FALSE if the option was specified with an invalid value.
 */
static int
nfs_nfs_program(struct mount_options *options, unsigned long *program)
{
	long tmp;

	switch (po_get_numeric(options, "nfsprog", &tmp)) {
	case PO_NOT_FOUND:
		break;
	case PO_FOUND:
		if (tmp > 0) {
			*program = tmp;
			return 1;
		}
	case PO_BAD_VALUE:
		nfs_error(_("%s: invalid value for 'nfsprog=' option"),
				progname);
		return 0;
	}

	/*
	 * NFS RPC program wasn't specified.  The RPC program
	 * cannot be determined via an rpcbind query.
	 */
	*program = nfs_getrpcbyname(NFSPROG, nfs_nfs_pgmtbl);
	return 1;
}

/*
 * Returns TRUE if @version contains a valid value for this option,
 * or FALSE if the option was specified with an invalid value.
 */
int
nfs_nfs_version(struct mount_options *options, struct nfs_version *version)
{
	char *version_key, *version_val, *cptr;
	int i, found = 0;

	version->v_mode = V_DEFAULT;

	for (i = 0; nfs_version_opttbl[i]; i++) {
		if (po_contains_prefix(options, nfs_version_opttbl[i],
								&version_key) == PO_FOUND) {
			found++;
			break;
		}
	}

	if (!found)
		return 1;

	if (i <= 2 ) {
		/* v2, v3, v4 */
		version_val = version_key + 1;
		version->v_mode = V_SPECIFIC;
	} else if (i > 2 ) {
		/* vers=, nfsvers= */
		version_val = po_get(options, version_key);
	}

	if (!version_val)
		goto ret_error;

	if (!(version->major = strtol(version_val, &cptr, 10)))
		goto ret_error;

	if (strcmp(nfs_version_opttbl[i], "minorversion") == 0) {
		version->v_mode = V_SPECIFIC;
		version->minor = version->major;
		version->major = 4;
	} else if (version->major < 4)
		version->v_mode = V_SPECIFIC;

	if (*cptr == '.') {
		version_val = ++cptr;
		if (!(version->minor = strtol(version_val, &cptr, 10)) && cptr == version_val)
			goto ret_error;
		version->v_mode = V_SPECIFIC;
	} else if (version->major > 3 && *cptr == '\0')
		version->v_mode = V_GENERAL;

	if (*cptr != '\0')
		goto ret_error;

	return 1;

ret_error:
	if (i <= 2 ) {
		nfs_error(_("%s: parsing error on 'v' option"),
			progname);
	} else if (i == 3 ) {
		nfs_error(_("%s: parsing error on 'vers=' option"),
			progname);
	} else if (i == 4) {
		nfs_error(_("%s: parsing error on 'nfsvers=' option"),
			progname);
	}
	version->v_mode = V_PARSE_ERR;
	errno = EINVAL;
	return 0;
}

/*
 * Returns TRUE if @protocol contains a valid value for this option,
 * or FALSE if the option was specified with an invalid value. On
 * error, errno is set.
 */
int
nfs_nfs_protocol(struct mount_options *options, unsigned long *protocol)
{
	sa_family_t family;
	char *option;

	switch (po_rightmost(options, nfs_transport_opttbl)) {
	case 0:	/* udp */
		*protocol = IPPROTO_UDP;
		return 1;
	case 1: /* tcp */
		*protocol = IPPROTO_TCP;
		return 1;
	case 2: /* rdma */
		*protocol = NFSPROTO_RDMA;
		return 1;
	case 3: /* proto */
		option = po_get(options, "proto");
		if (option != NULL) {
			if (!nfs_get_proto(option, &family, protocol)) {
				errno = EPROTONOSUPPORT;
				nfs_error(_("%s: Failed to find '%s' protocol"), 
					progname, option);
				return 0;
			}
			return 1;
		}
	}

	/*
	 * NFS transport protocol wasn't specified.  The pmap
	 * protocol value will be filled in later by an rpcbind
	 * query in this case.
	 */
	*protocol = 0;
	return 1;
}

/*
 * Returns TRUE if @port contains a valid value for this option,
 * or FALSE if the option was specified with an invalid value.
 */
static int
nfs_nfs_port(struct mount_options *options, unsigned long *port)
{
	long tmp;

	switch (po_get_numeric(options, "port", &tmp)) {
	case PO_NOT_FOUND:
		break;
	case PO_FOUND:
		if (tmp >= 0 && tmp <= 65535) {
			*port = tmp;
			return 1;
		}
	case PO_BAD_VALUE:
		nfs_error(_("%s: invalid value for 'port=' option"),
				progname);
		return 0;
	}

	/*
	 * NFS service port wasn't specified.  The pmap port value
	 * will be filled in later by an rpcbind query in this case.
	 */
	*port = 0;
	return 1;
}

#ifdef IPV6_SUPPORTED
sa_family_t	config_default_family = AF_UNSPEC;

static int
nfs_verify_family(sa_family_t UNUSED(family))
{
	return 1;
}
#else /* IPV6_SUPPORTED */
sa_family_t	config_default_family = AF_INET;

static int
nfs_verify_family(sa_family_t family)
{
	if (family != AF_INET)
		return 0;

	return 1;
}
#endif /* IPV6_SUPPORTED */

/*
 * Returns TRUE and fills in @family if a valid NFS protocol option
 * is found, or FALSE if the option was specified with an invalid value
 * or if the protocol family isn't supported. On error, errno is set.
 */
int nfs_nfs_proto_family(struct mount_options *options,
				sa_family_t *family)
{
	unsigned long protocol;
	char *option;
	sa_family_t tmp_family = config_default_family;

	switch (po_rightmost(options, nfs_transport_opttbl)) {
	case 0:	/* udp */
	case 1: /* tcp */
	case 2: /* rdma */
		/* for compatibility; these are always AF_INET */
		*family = AF_INET;
		return 1;
	case 3: /* proto */
		option = po_get(options, "proto");
		if (option != NULL &&
		    !nfs_get_proto(option, &tmp_family, &protocol)) {

			nfs_error(_("%s: Failed to find '%s' protocol"), 
				progname, option);
			errno = EPROTONOSUPPORT;
			return 0;
		}
	}

	if (!nfs_verify_family(tmp_family))
		goto out_err;
	*family = tmp_family;
	return 1;
out_err:
	errno = EAFNOSUPPORT;
	return 0;
}

/*
 * "mountprog" is supported only by the legacy mount command.  The
 * kernel mount client does not support this option.
 *
 * Returns TRUE if @program contains a valid value for this option,
 * or FALSE if the option was specified with an invalid value.
 */
static int
nfs_mount_program(struct mount_options *options, unsigned long *program)
{
	long tmp;

	switch (po_get_numeric(options, "mountprog", &tmp)) {
	case PO_NOT_FOUND:
		break;
	case PO_FOUND:
		if (tmp > 0) {
			*program = tmp;
			return 1;
		}
	case PO_BAD_VALUE:
		nfs_error(_("%s: invalid value for 'mountprog=' option"),
				progname);
		return 0;
	}

	/*
	 * MNT RPC program wasn't specified.  The RPC program
	 * cannot be determined via an rpcbind query.
	 */
	*program = nfs_getrpcbyname(MOUNTPROG, nfs_mnt_pgmtbl);
	return 1;
}

/*
 * Returns TRUE if @version contains a valid value for this option,
 * or FALSE if the option was specified with an invalid value.
 */
static int
nfs_mount_version(struct mount_options *options, unsigned long *version)
{
	long tmp;

	switch (po_get_numeric(options, "mountvers", &tmp)) {
	case PO_NOT_FOUND:
		break;
	case PO_FOUND:
		if (tmp >= 1 && tmp <= 4) {
			*version = tmp;
			return 1;
		}
	case PO_BAD_VALUE:
		nfs_error(_("%s: invalid value for 'mountvers=' option"),
				progname);
		return 0;
	}

	/*
	 * MNT version wasn't specified.  The pmap version value
	 * will be filled in later by an rpcbind query in this case.
	 */
	*version = 0;
	return 1;
}

/*
 * Returns TRUE if @protocol contains a valid value for this option,
 * or FALSE if the option was specified with an invalid value. On
 * error, errno is set.
 */
static int
nfs_mount_protocol(struct mount_options *options, unsigned long *protocol)
{
	sa_family_t family;
	char *option;

	option = po_get(options, "mountproto");
	if (option != NULL) {
		if (!nfs_get_proto(option, &family, protocol)) {
			errno = EPROTONOSUPPORT;
			nfs_error(_("%s: Failed to find '%s' protocol"), 
				progname, option);
			return 0;
		}
		return 1;
	}

	/*
	 * MNT transport protocol wasn't specified.  If the NFS
	 * transport protocol was specified, use that; otherwise
	 * set @protocol to zero.  The pmap protocol value will
	 * be filled in later by an rpcbind query in this case.
	 */
	if (!nfs_nfs_protocol(options, protocol))
		return 0;
	if (*protocol == NFSPROTO_RDMA)
		*protocol = IPPROTO_TCP;
	return 1;
}

/*
 * Returns TRUE if @port contains a valid value for this option,
 * or FALSE if the option was specified with an invalid value.
 */
static int
nfs_mount_port(struct mount_options *options, unsigned long *port)
{
	long tmp;

	switch (po_get_numeric(options, "mountport", &tmp)) {
	case PO_NOT_FOUND:
		break;
	case PO_FOUND:
		if (tmp >= 0 && tmp <= 65535) {
			*port = tmp;
			return 1;
		}
	case PO_BAD_VALUE:
		nfs_error(_("%s: invalid value for 'mountport=' option"),
				progname);
		return 0;
	}

	/*
	 * MNT service port wasn't specified.  The pmap port value
	 * will be filled in later by an rpcbind query in this case.
	 */
	*port = 0;
	return 1;
}

/*
 * Returns TRUE and fills in @family if a valid MNT protocol option
 * is found, or FALSE if the option was specified with an invalid value
 * or if the protocol family isn't supported. On error, errno is set.
 */
int nfs_mount_proto_family(struct mount_options *options,
				sa_family_t *family)
{
	unsigned long protocol;
	char *option;
	sa_family_t tmp_family = config_default_family;

	option = po_get(options, "mountproto");
	if (option != NULL) {
		if (!nfs_get_proto(option, &tmp_family, &protocol)) {
			nfs_error(_("%s: Failed to find '%s' protocol"), 
				progname, option);
			errno = EPROTONOSUPPORT;
			goto out_err;
		}
		if (!nfs_verify_family(tmp_family))
			goto out_err;
		*family = tmp_family;
		return 1;
	}

	/*
	 * MNT transport protocol wasn't specified.  If the NFS
	 * transport protocol was specified, derive the family
	 * from that; otherwise, return the default family for
	 * NFS.
	 */
	return nfs_nfs_proto_family(options, family);
out_err:
	errno = EAFNOSUPPORT;
	return 0;
}

/**
 * nfs_options2pmap - set up pmap structs based on mount options
 * @options: pointer to mount options
 * @nfs_pmap: OUT: pointer to pmap arguments for NFS server
 * @mnt_pmap: OUT: pointer to pmap arguments for mountd server
 *
 * Returns TRUE if the pmap options specified in @options have valid
 * values; otherwise FALSE is returned.
 */
int nfs_options2pmap(struct mount_options *options,
		     struct pmap *nfs_pmap, struct pmap *mnt_pmap)
{
	struct nfs_version version;

	if (!nfs_nfs_program(options, &nfs_pmap->pm_prog))
		return 0;
	if (!nfs_nfs_version(options, &version))
		return 0;
	if (version.v_mode == V_DEFAULT)
		nfs_pmap->pm_vers = 0;
	else
		nfs_pmap->pm_vers = version.major;
	if (!nfs_nfs_protocol(options, &nfs_pmap->pm_prot))
		return 0;
	if (!nfs_nfs_port(options, &nfs_pmap->pm_port))
		return 0;

	if (!nfs_mount_program(options, &mnt_pmap->pm_prog))
		return 0;
	if (!nfs_mount_version(options, &mnt_pmap->pm_vers))
		return 0;
	if (!nfs_mount_protocol(options, &mnt_pmap->pm_prot))
		return 0;
	if (!nfs_mount_port(options, &mnt_pmap->pm_port))
		return 0;

	return 1;
}

/*
 * Discover mount server's hostname/address by examining mount options
 *
 * Returns a pointer to a string that the caller must free, on
 * success; otherwise NULL is returned.
 */
static char *nfs_umount_hostname(struct mount_options *options,
				 char *hostname)
{
	char *option;

	option = po_get(options, "mountaddr");
	if (option)
		goto out;
	option = po_get(options, "mounthost");
	if (option)
		goto out;
	option = po_get(options, "addr");
	if (option)
		goto out;

	return hostname;

out:
	free(hostname);
	return strdup(option);
}


/*
 * Returns EX_SUCCESS if mount options and device name have been
 * parsed successfully; otherwise EX_FAIL.
 */
int nfs_umount_do_umnt(struct mount_options *options,
		       char **hostname, char **dirname)
{
	union nfs_sockaddr address;
	struct sockaddr *sap = &address.sa;
	socklen_t salen = sizeof(address);
	struct pmap nfs_pmap, mnt_pmap;
	sa_family_t family;

	if (!nfs_options2pmap(options, &nfs_pmap, &mnt_pmap))
		return EX_FAIL;

	/* Skip UMNT call for vers=4 mounts */
	if (nfs_pmap.pm_vers == 4)
		return EX_SUCCESS;

	*hostname = nfs_umount_hostname(options, *hostname);
	if (!*hostname) {
		nfs_error(_("%s: out of memory"), progname);
		return EX_FAIL;
	}

	if (!nfs_mount_proto_family(options, &family))
		return 0;
	if (!nfs_lookup(*hostname, family, sap, &salen))
		/* nfs_lookup reports any errors */
		return EX_FAIL;

	if (nfs_advise_umount(sap, salen, &mnt_pmap, dirname) == 0)
		/* nfs_advise_umount reports any errors */
		return EX_FAIL;

	return EX_SUCCESS;
}
