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

/*
 * NSM for Linux.
 *
 * Instead of using ONC or TI RPC library calls, statd constructs
 * RPC calls directly in socket buffers.  This allows a single
 * socket to be concurrently shared among several different RPC
 * programs and versions using a simple RPC request dispatcher.
 *
 * This file contains the details of RPC header and call
 * construction and reply parsing, and a method for creating a
 * socket for use with these functions.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif	/* HAVE_CONFIG_H */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <rpc/pmap_rmt.h>

#ifdef HAVE_LIBTIRPC
#include <netconfig.h>
#include <rpc/rpcb_prot.h>
#endif	/* HAVE_LIBTIRPC */

#include "xlog.h"
#include "nfsrpc.h"
#include "nsm.h"
#include "sm_inter.h"

/*
 * Returns a fresh XID appropriate for RPC over UDP -- never zero.
 */
static uint32_t
nsm_next_xid(void)
{
	static uint32_t nsm_xid = 0;
	struct timeval now;

	if (nsm_xid == 0) {
		(void)gettimeofday(&now, NULL);
		nsm_xid = (uint32_t)getpid() ^
				(uint32_t)now.tv_sec ^ (uint32_t)now.tv_usec;
	}

	return nsm_xid++;
}

/*
 * Select a fresh XID and construct an RPC header in @mesg.
 * Always use AUTH_NULL credentials and verifiers.
 *
 * Returns the new XID.
 */
static uint32_t
nsm_init_rpc_header(const rpcprog_t program, const rpcvers_t version,
			const rpcproc_t procedure, struct rpc_msg *mesg)
{
	struct call_body *cb = &mesg->rm_call;
	uint32_t xid = nsm_next_xid();

	memset(mesg, 0, sizeof(*mesg));

	mesg->rm_xid = (unsigned long)xid;
	mesg->rm_direction = CALL;

	cb->cb_rpcvers = RPC_MSG_VERSION;
	cb->cb_prog = program;
	cb->cb_vers = version;
	cb->cb_proc = procedure;

	cb->cb_cred.oa_flavor = AUTH_NULL;
	cb->cb_cred.oa_base = (caddr_t) NULL;
	cb->cb_cred.oa_length = 0;
	cb->cb_verf.oa_flavor = AUTH_NULL;
	cb->cb_verf.oa_base = (caddr_t) NULL;
	cb->cb_verf.oa_length = 0;

	return xid;
}

/*
 * Initialize the network send buffer and XDR memory for encoding.
 */
static void
nsm_init_xdrmem(char *msgbuf, const unsigned int msgbuflen,
		XDR *xdrp)
{
	memset(msgbuf, 0, (size_t)msgbuflen);
	memset(xdrp, 0, sizeof(*xdrp));
	xdrmem_create(xdrp, msgbuf, msgbuflen, XDR_ENCODE);
}

/*
 * Send a completed RPC call on a socket.
 *
 * Returns true if all the bytes were sent successfully; otherwise
 * false if any error occurred.
 */
static _Bool
nsm_rpc_sendto(const int sock, const struct sockaddr *sap,
			const socklen_t salen, XDR *xdrs, void *buf)
{
	const size_t buflen = (size_t)xdr_getpos(xdrs);
	ssize_t err;

	err = sendto(sock, buf, buflen, 0, sap, salen);
	if ((err < 0) || ((size_t)err != buflen)) {
		xlog(L_ERROR, "%s: sendto failed: %m", __func__);
		return false;
	}
	return true;
}

/**
 * nsm_xmit_getport - post a PMAP_GETPORT call on a socket descriptor
 * @sock: datagram socket descriptor
 * @sin: pointer to AF_INET socket address of server
 * @program: RPC program number to query
 * @version: RPC version number to query
 *
 * Send a PMAP_GETPORT call to the portmap daemon at @sin using
 * socket descriptor @sock.  This request queries the RPC program
 * [program, version, IPPROTO_UDP].
 *
 * NB: PMAP_GETPORT works only for IPv4 hosts.  This implementation
 *     works only over UDP, and queries only UDP registrations.
 *
 * Returns the XID of the call, or zero if an error occurred.
 */
uint32_t
nsm_xmit_getport(const int sock, const struct sockaddr_in *sin,
			const unsigned long program,
			const unsigned long version)
{
	char msgbuf[NSM_MAXMSGSIZE];
	struct sockaddr_in addr;
	struct rpc_msg mesg;
	_Bool sent = false;
	struct pmap parms = {
		.pm_prog	= program,
		.pm_vers	= version,
		.pm_prot	= (unsigned long)IPPROTO_UDP,
	};
	uint32_t xid;
	XDR xdr;

	xlog(D_CALL, "Sending PMAP_GETPORT for %u, %u, udp", program, version);

	nsm_init_xdrmem(msgbuf, NSM_MAXMSGSIZE, &xdr);
	xid = nsm_init_rpc_header(PMAPPROG, PMAPVERS,
					(rpcproc_t)PMAPPROC_GETPORT, &mesg);

	addr = *sin;
	addr.sin_port = htons(PMAPPORT);

	if (xdr_callmsg(&xdr, &mesg) == TRUE &&
	    xdr_pmap(&xdr, &parms) == TRUE)
		sent = nsm_rpc_sendto(sock, (struct sockaddr *)(char *)&addr,
					(socklen_t)sizeof(addr), &xdr, msgbuf);
	else
		xlog(L_ERROR, "%s: can't encode PMAP_GETPORT call", __func__);

	xdr_destroy(&xdr);

	if (sent == false)
		return 0;
	return xid;
}

/**
 * nsm_xmit_getaddr - post an RPCB_GETADDR call on a socket descriptor
 * @sock: datagram socket descriptor
 * @sin: pointer to AF_INET6 socket address of server
 * @program: RPC program number to query
 * @version: RPC version number to query
 *
 * Send an RPCB_GETADDR call to the rpcbind daemon at @sap using
 * socket descriptor @sock.  This request queries the RPC program
 * [program, version, "udp6"].
 *
 * NB: RPCB_GETADDR works for both IPv4 and IPv6 hosts.  This
 *     implementation works only over UDP and AF_INET6, and queries
 *     only "udp6" registrations.
 *
 * Returns the XID of the call, or zero if an error occurred.
 */
#ifdef HAVE_LIBTIRPC
uint32_t
nsm_xmit_getaddr(const int sock, const struct sockaddr_in6 *sin6,
			const rpcprog_t program, const rpcvers_t version)
{
	char msgbuf[NSM_MAXMSGSIZE];
	struct sockaddr_in6 addr;
	struct rpc_msg mesg;
	_Bool sent = false;
	struct rpcb parms = {
		.r_prog		= program,
		.r_vers		= version,
		.r_netid	= "udp6",
		.r_owner	= "",
	};
	uint32_t xid;
	XDR xdr;

	xlog(D_CALL, "Sending RPCB_GETADDR for %u, %u, udp6", program, version);

	nsm_init_xdrmem(msgbuf, NSM_MAXMSGSIZE, &xdr);
	xid = nsm_init_rpc_header(RPCBPROG, RPCBVERS,
					(rpcproc_t)RPCBPROC_GETADDR, &mesg);

	addr = *sin6;
	addr.sin6_port = htons(PMAPPORT);
	parms.r_addr = nfs_sockaddr2universal((struct sockaddr *)(char *)&addr);
	if (parms.r_addr == NULL) {
		xlog(L_ERROR, "%s: can't encode socket address", __func__);
		return 0;
	}

	if (xdr_callmsg(&xdr, &mesg) == TRUE &&
	    xdr_rpcb(&xdr, &parms) == TRUE)
		sent = nsm_rpc_sendto(sock, (struct sockaddr *)(char *)&addr,
					(socklen_t)sizeof(addr), &xdr, msgbuf);
	else
		xlog(L_ERROR, "%s: can't encode RPCB_GETADDR call", __func__);

	xdr_destroy(&xdr);
	free(parms.r_addr);

	if (sent == false)
		return 0;
	return xid;
}
#else	/* !HAVE_LIBTIRPC */
uint32_t
nsm_xmit_getaddr(const int sock __attribute__((unused)),
			const struct sockaddr_in6 *sin6 __attribute__((unused)),
			const rpcprog_t program __attribute__((unused)),
			const rpcvers_t version __attribute__((unused)))
{
	return 0;
}
#endif	/* !HAVE_LIBTIRPC */

/**
 * nsm_xmit_rpcbind - post an rpcbind request
 * @sock: datagram socket descriptor
 * @sap: pointer to socket address of server
 * @program: RPC program number to query
 * @version: RPC version number to query
 *
 * Send an rpcbind query to the rpcbind daemon at @sap using
 * socket descriptor @sock.
 *
 * NB: This implementation works only over UDP, but can query IPv4 or IPv6
 *     hosts.  It queries only UDP registrations.
 *
 * Returns the XID of the call, or zero if an error occurred.
 */
uint32_t
nsm_xmit_rpcbind(const int sock, const struct sockaddr *sap,
			const rpcprog_t program, const rpcvers_t version)
{
	switch (sap->sa_family) {
	case AF_INET:
		return nsm_xmit_getport(sock, (const struct sockaddr_in *)sap,
						program, version);
	case AF_INET6:
		return nsm_xmit_getaddr(sock, (const struct sockaddr_in6 *)sap,
						program, version);
	}
	return 0;
}

/**
 * nsm_xmit_notify - post an NSMPROC_NOTIFY call on a socket descriptor
 * @sock: datagram socket descriptor
 * @sap: pointer to socket address of peer to notify (port already filled in)
 * @salen: length of socket address
 * @program: RPC program number to use
 * @mon_name: mon_name of local peer (ie the rebooting system)
 * @state: state of local peer
 *
 * Send an NSMPROC_NOTIFY call to the peer at @sap using socket descriptor @sock.
 * This request notifies the peer that we have rebooted.
 *
 * NB: This implementation works only over UDP, but supports both AF_INET
 *     and AF_INET6.
 *
 * Returns the XID of the call, or zero if an error occurred.
 */
uint32_t
nsm_xmit_notify(const int sock, const struct sockaddr *sap,
			const socklen_t salen, const rpcprog_t program,
			const char *mon_name, const int state)
{
	char msgbuf[NSM_MAXMSGSIZE];
	struct stat_chge state_change;
	struct rpc_msg mesg;
	_Bool sent = false;
	uint32_t xid;
	XDR xdr;

	state_change.mon_name = strdup(mon_name);
	if (state_change.mon_name == NULL) {
		xlog(L_ERROR, "%s: no memory", __func__);
		return 0;
	}
	state_change.state = state;

	xlog(D_CALL, "Sending SM_NOTIFY for %s", mon_name);

	nsm_init_xdrmem(msgbuf, NSM_MAXMSGSIZE, &xdr);
	xid = nsm_init_rpc_header(program, SM_VERS, SM_NOTIFY, &mesg);

	if (xdr_callmsg(&xdr, &mesg) == TRUE &&
	    xdr_stat_chge(&xdr, &state_change) == TRUE)
		sent = nsm_rpc_sendto(sock, sap, salen, &xdr, msgbuf);
	else
		xlog(L_ERROR, "%s: can't encode NSMPROC_NOTIFY call",
				__func__);

	xdr_destroy(&xdr);
	free(state_change.mon_name);

	if (sent == false)
		return 0;
	return xid;
}

/**
 * nsm_xmit_nlmcall - post an unnamed call to local NLM on a socket descriptor
 * @sock: datagram socket descriptor
 * @sap: address/port of NLM service to contact
 * @salen: size of @sap
 * @m: callback data defining RPC call to make
 * @state: state of rebooting host
 *
 * Send an unnamed call (previously requested via NSMPROC_MON) to the
 * specified local UDP-based RPC service using socket descriptor @sock.
 *
 * NB: This implementation works only over UDP, but supports both AF_INET
 *     and AF_INET6.
 *
 * Returns the XID of the call, or zero if an error occurred.
 */
uint32_t
nsm_xmit_nlmcall(const int sock, const struct sockaddr *sap,
			const socklen_t salen, const struct mon *m,
			const int state)
{
	const struct my_id *id = &m->mon_id.my_id;
	char msgbuf[NSM_MAXMSGSIZE];
	struct status new_status;
	struct rpc_msg mesg;
	_Bool sent = false;
	uint32_t xid;
	XDR xdr;

	xlog(D_CALL, "Sending NLM downcall for %s", m->mon_id.mon_name);

	nsm_init_xdrmem(msgbuf, NSM_MAXMSGSIZE, &xdr);
	xid = nsm_init_rpc_header((rpcprog_t)id->my_prog,
					(rpcvers_t)id->my_vers,
					(rpcproc_t)id->my_proc, &mesg);

	new_status.mon_name = m->mon_id.mon_name;
	new_status.state = state;
	memcpy(&new_status.priv, &m->priv, sizeof(new_status.priv));

	if (xdr_callmsg(&xdr, &mesg) == TRUE &&
	    xdr_status(&xdr, &new_status) == TRUE)
		sent = nsm_rpc_sendto(sock, sap, salen, &xdr, msgbuf);
	else
		xlog(L_ERROR, "%s: can't encode NLM downcall", __func__);

	xdr_destroy(&xdr);

	if (sent == false)
		return 0;
	return xid;
}

/**
 * nsm_parse_reply - parse and validate the header in an RPC reply
 * @xdrs: pointer to XDR
 *
 * Returns the XID of the reply, or zero if an error occurred.
 */
uint32_t
nsm_parse_reply(XDR *xdrs)
{
	struct rpc_msg mesg = {
		.rm_reply.rp_acpt.ar_results.proc	= (xdrproc_t)xdr_void,
	};
	uint32_t xid;

	if (xdr_replymsg(xdrs, &mesg) == FALSE) {
		xlog(L_ERROR, "%s: can't decode RPC reply", __func__);
		return 0;
	}
	xid = (uint32_t)mesg.rm_xid;

	if (mesg.rm_reply.rp_stat != MSG_ACCEPTED) {
		xlog(L_ERROR, "%s: [0x%x] RPC status %d",
			__func__, xid, mesg.rm_reply.rp_stat);
		return 0;
	}

	if (mesg.rm_reply.rp_acpt.ar_stat != SUCCESS) {
		xlog(L_ERROR, "%s: [0x%x] RPC accept status %d",
			__func__, xid, mesg.rm_reply.rp_acpt.ar_stat);
		return 0;
	}

	return xid;
}

/**
 * nsm_recv_getport - parse PMAP_GETPORT reply
 * @xdrs: pointer to XDR
 *
 * Returns the port number from the RPC reply, or zero
 * if an error occurred.
 */
unsigned long
nsm_recv_getport(XDR *xdrs)
{
	unsigned long port = 0;

	if (xdr_u_long(xdrs, &port) == FALSE)
		xlog(L_ERROR, "%s: can't decode pmap reply",
			__func__);
	if (port > UINT16_MAX) {
		xlog(L_ERROR, "%s: bad port number",
			__func__);
		port = 0;
	}

	xlog(D_CALL, "Received PMAP_GETPORT result: %lu", port);
	return port;
}

/**
 * nsm_recv_getaddr - parse RPCB_GETADDR reply
 * @xdrs: pointer to XDR
 *
 * Returns the port number from the RPC reply, or zero
 * if an error occurred.
 */
uint16_t
nsm_recv_getaddr(XDR *xdrs)
{
	char *uaddr = NULL;
	int port;

	if (xdr_wrapstring(xdrs, &uaddr) == FALSE)
		xlog(L_ERROR, "%s: can't decode rpcb reply",
			__func__);

	if ((uaddr == NULL) || (uaddr[0] == '\0')) {
		xlog(D_CALL, "Received RPCB_GETADDR result: "
				"program not registered");
		return 0;
	}

	port = nfs_universal2port(uaddr);

	xdr_free((xdrproc_t)xdr_wrapstring, (char *)&uaddr);

	if (port < 0 || port > UINT16_MAX) {
		xlog(L_ERROR, "%s: bad port number",
			__func__);
		return 0;
	}

	xlog(D_CALL, "Received RPCB_GETADDR result: %d", port);
	return (uint16_t)port;
}

/**
 * nsm_recv_rpcbind - parse rpcbind reply
 * @af: address family of reply
 * @xdrs: pointer to XDR
 *
 * Returns the port number from the RPC reply, or zero
 * if an error occurred.
 */
uint16_t
nsm_recv_rpcbind(const sa_family_t family, XDR *xdrs)
{
	switch (family) {
	case AF_INET:
		return (uint16_t)nsm_recv_getport(xdrs);
	case AF_INET6:
		return nsm_recv_getaddr(xdrs);
	}
	return 0;
}
