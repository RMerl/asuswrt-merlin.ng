/* nsd_ptcp.c
 *
 * An implementation of the nsd interface for plain tcp sockets.
 *
 * Copyright 2007-2018 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of the rsyslog runtime library.
 *
 * The rsyslog runtime library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The rsyslog runtime library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the rsyslog runtime library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 * A copy of the LGPL can be found in the file "COPYING.LESSER" in this distribution.
 */
#include "config.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <netdb.h>
#include <fnmatch.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/tcp.h>

#include "rsyslog.h"
#include "syslogd-types.h"
#include "module-template.h"
#include "parse.h"
#include "srUtils.h"
#include "obj.h"
#include "errmsg.h"
#include "net.h"
#include "netstrms.h"
#include "netstrm.h"
#include "nsdsel_ptcp.h"
#include "nsdpoll_ptcp.h"
#include "nsd_ptcp.h"
#include "prop.h"
#include "dnscache.h"

MODULE_TYPE_LIB
MODULE_TYPE_NOKEEP

/* static data */
DEFobjStaticHelpers
DEFobjCurrIf(glbl)
DEFobjCurrIf(net)
DEFobjCurrIf(netstrms)
DEFobjCurrIf(netstrm)
DEFobjCurrIf(prop)


/* a few deinit helpers */

/* close socket if open (may always be called) */
static void
sockClose(int *pSock)
{
	if(*pSock >= 0) {
		close(*pSock);
		*pSock = -1;
	}
}

/* Standard-Constructor
 */
BEGINobjConstruct(nsd_ptcp) /* be sure to specify the object type also in END macro! */
	pThis->sock = -1;
ENDobjConstruct(nsd_ptcp)


/* destructor for the nsd_ptcp object */
BEGINobjDestruct(nsd_ptcp) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDestruct(nsd_ptcp)
	sockClose(&pThis->sock);
	if(pThis->remoteIP != NULL)
		prop.Destruct(&pThis->remoteIP);
	free(pThis->pRemHostName);
ENDobjDestruct(nsd_ptcp)


/* Provide access to the sockaddr_storage of the remote peer. This
 * is needed by the legacy ACL system. --- gerhards, 2008-12-01
 */
static rsRetVal
GetRemAddr(nsd_t *pNsd, struct sockaddr_storage **ppAddr)
{
	nsd_ptcp_t *pThis = (nsd_ptcp_t*) pNsd;
	DEFiRet;

	ISOBJ_TYPE_assert((pThis), nsd_ptcp);
	assert(ppAddr != NULL);

	*ppAddr = &(pThis->remAddr);

	RETiRet;
}


/* Provide access to the underlying OS socket. This is primarily
 * useful for other drivers (like nsd_gtls) who utilize ourselfs
 * for some of their functionality. -- rgerhards, 2008-04-18
 */
static rsRetVal
GetSock(nsd_t *pNsd, int *pSock)
{
	nsd_ptcp_t *pThis = (nsd_ptcp_t*) pNsd;
	DEFiRet;

	ISOBJ_TYPE_assert((pThis), nsd_ptcp);
	assert(pSock != NULL);

	*pSock = pThis->sock;

	RETiRet;
}


/* Set the driver mode. We support no different modes, but allow mode
 * 0 to be set to be compatible with config file defaults and the other
 * drivers.
 * rgerhards, 2008-04-28
 */
static rsRetVal
SetMode(nsd_t __attribute__((unused)) *pNsd, int mode)
{
	DEFiRet;
	if(mode != 0) {
		LogError(0, RS_RET_INVALID_DRVR_MODE, "error: driver mode %d not supported by "
				"ptcp netstream driver", mode);
		ABORT_FINALIZE(RS_RET_INVALID_DRVR_MODE);
	}
finalize_it:
	RETiRet;
}


/* Set the authentication mode. For us, the following is supported:
 * anon - no certificate checks whatsoever (discouraged, but supported)
 * mode == NULL is valid and defaults to anon
 * Actually, we do not even record the mode right now, because we can
 * always work in anon mode, only. So there is no point in recording
 * something if that's the only choice. What the function does is
 * return an error if something is requested that we can not support.
 * rgerhards, 2008-05-17
 */
static rsRetVal
SetAuthMode(nsd_t __attribute__((unused)) *pNsd, uchar *mode)
{
	DEFiRet;
	if(mode != NULL && strcasecmp((char*)mode, "anon")) {
		LogError(0, RS_RET_VALUE_NOT_SUPPORTED, "error: authentication mode '%s' not supported by "
				"ptcp netstream driver", mode);
		ABORT_FINALIZE(RS_RET_VALUE_NOT_SUPPORTED);
	}

finalize_it:
	RETiRet;
}


/* Set priorityString
 * PascalWithopf 2017-08-18 */
static rsRetVal
SetGnutlsPriorityString(nsd_t __attribute__((unused)) *pNsd, uchar *iVal)
{
	DEFiRet;
	if(iVal != NULL) {
		LogError(0, RS_RET_VALUE_NOT_SUPPORTED, "error: "
		"gnutlsPriorityString '%s' not supported by ptcp netstream "
		"driver", iVal);
		ABORT_FINALIZE(RS_RET_VALUE_NOT_SUPPORTED);
	}
finalize_it:
	RETiRet;
}


/* Set the permitted peers. This is a dummy, always returning an
 * error because we do not support fingerprint authentication.
 * rgerhards, 2008-05-17
 */
static rsRetVal
SetPermPeers(nsd_t __attribute__((unused)) *pNsd, permittedPeers_t __attribute__((unused)) *pPermPeers)
{
	DEFiRet;

	if(pPermPeers != NULL) {
		LogError(0, RS_RET_VALUE_NOT_IN_THIS_MODE, "authentication not supported by ptcp netstream driver");
		ABORT_FINALIZE(RS_RET_VALUE_NOT_IN_THIS_MODE);
	}

finalize_it:
	RETiRet;
}




/* Provide access to the underlying OS socket. This is primarily
 * useful for other drivers (like nsd_gtls) who utilize ourselfs
 * for some of their functionality.
 * This function sets the socket -- rgerhards, 2008-04-25
 */
static rsRetVal
SetSock(nsd_t *pNsd, int sock)
{
	nsd_ptcp_t *pThis = (nsd_ptcp_t*) pNsd;
	DEFiRet;

	ISOBJ_TYPE_assert((pThis), nsd_ptcp);
	assert(sock >= 0);

	pThis->sock = sock;

	RETiRet;
}

/* Keep Alive Options
 */
static rsRetVal
SetKeepAliveIntvl(nsd_t *pNsd, int keepAliveIntvl)
{
	nsd_ptcp_t *pThis = (nsd_ptcp_t*) pNsd;
	DEFiRet;

	ISOBJ_TYPE_assert((pThis), nsd_ptcp);

	pThis->iKeepAliveIntvl = keepAliveIntvl;

	RETiRet;
}

/* Keep Alive Options
 */
static rsRetVal
SetKeepAliveProbes(nsd_t *pNsd, int keepAliveProbes)
{
	nsd_ptcp_t *pThis = (nsd_ptcp_t*) pNsd;
	DEFiRet;

	ISOBJ_TYPE_assert((pThis), nsd_ptcp);

	pThis->iKeepAliveProbes = keepAliveProbes;

	RETiRet;
}

/* Keep Alive Options
 */
static rsRetVal
SetKeepAliveTime(nsd_t *pNsd, int keepAliveTime)
{
	nsd_ptcp_t *pThis = (nsd_ptcp_t*) pNsd;
	DEFiRet;

	ISOBJ_TYPE_assert((pThis), nsd_ptcp);

	pThis->iKeepAliveTime = keepAliveTime;

	RETiRet;
}

/* abort a connection. This is meant to be called immediately
 * before the Destruct call. -- rgerhards, 2008-03-24
 */
static rsRetVal
Abort(nsd_t *pNsd)
{
	struct linger ling;
	nsd_ptcp_t *pThis = (nsd_ptcp_t*) pNsd;

	DEFiRet;
	ISOBJ_TYPE_assert((pThis), nsd_ptcp);

	if((pThis)->sock != -1) {
		ling.l_onoff = 1;
		ling.l_linger = 0;
		if(setsockopt((pThis)->sock, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling)) < 0 ) {
			dbgprintf("could not set SO_LINGER, errno %d\n", errno);
		}
	}

	RETiRet;
}


/* Set pRemHost based on the address provided. This is to be called upon accept()ing
 * a connection request. It must be provided by the socket we received the
 * message on as well as a NI_MAXHOST size large character buffer for the FQDN.
 * Please see http://www.hmug.org/man/3/getnameinfo.php (under Caveats)
 * for some explanation of the code found below. If we detect a malicious
 * hostname, we return RS_RET_MALICIOUS_HNAME and let the caller decide
 * on how to deal with that.
 * rgerhards, 2008-03-31
 */
static rsRetVal
FillRemHost(nsd_ptcp_t *pThis, struct sockaddr_storage *pAddr)
{
	prop_t *fqdn;

	DEFiRet;
	ISOBJ_TYPE_assert(pThis, nsd_ptcp);
	assert(pAddr != NULL);

	CHKiRet(dnscacheLookup(pAddr, &fqdn, NULL, NULL, &pThis->remoteIP));

	/* We now have the names, so now let's allocate memory and store them permanently.
	 * (side note: we may hold on to these values for quite a while, thus we trim their
	 * memory consumption)
	 */
	if((pThis->pRemHostName = MALLOC(prop.GetStringLen(fqdn)+1)) == NULL)
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
	memcpy(pThis->pRemHostName, propGetSzStr(fqdn), prop.GetStringLen(fqdn)+1);
	prop.Destruct(&fqdn);

finalize_it:
	RETiRet;
}


/* accept an incoming connection request
 * rgerhards, 2008-04-22
 */
static rsRetVal
AcceptConnReq(nsd_t *pNsd, nsd_t **ppNew)
{
	int sockflags;
	nsd_ptcp_t *pThis = (nsd_ptcp_t*) pNsd;
	struct sockaddr_storage addr;
	socklen_t addrlen = sizeof(addr);
	nsd_ptcp_t *pNew = NULL;
	int iNewSock = -1;

	DEFiRet;
	assert(ppNew != NULL);
	ISOBJ_TYPE_assert(pThis, nsd_ptcp);

	iNewSock = accept(pThis->sock, (struct sockaddr*) &addr, &addrlen);
	if(iNewSock < 0) {
		if(Debug) {
			char errStr[1024];
			rs_strerror_r(errno, errStr, sizeof(errStr));
			dbgprintf("nds_ptcp: error accepting connection on socket %d, errno %d: %s\n",
				  pThis->sock, errno, errStr);
		}
		ABORT_FINALIZE(RS_RET_ACCEPT_ERR);
	}

	/* construct our object so that we can use it... */
	CHKiRet(nsd_ptcpConstruct(&pNew));

	/* for the legacy ACL code, we need to preserve addr. While this is far from
	 * begin perfect (from an abstract design perspective), we need this to prevent
	 * breaking everything. TODO: we need to implement a new ACL module to get rid
	 * of this function. -- rgerhards, 2008-12-01
	 */
	memcpy(&pNew->remAddr, &addr, sizeof(struct sockaddr_storage));
	CHKiRet(FillRemHost(pNew, &addr));

	/* set the new socket to non-blocking IO -TODO:do we really need to do this here? Do we always want it? */
	if((sockflags = fcntl(iNewSock, F_GETFL)) != -1) {
		sockflags |= O_NONBLOCK;
		/* SETFL could fail too, so get it caught by the subsequent
		 * error check.
		 */
		sockflags = fcntl(iNewSock, F_SETFL, sockflags);
	}
	if(sockflags == -1) {
		dbgprintf("error %d setting fcntl(O_NONBLOCK) on tcp socket %d", errno, iNewSock);
		ABORT_FINALIZE(RS_RET_IO_ERROR);
	}

	pNew->sock = iNewSock;
	*ppNew = (nsd_t*) pNew;

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pNew != NULL)
			nsd_ptcpDestruct(&pNew);
		/* the close may be redundant, but that doesn't hurt... */
		sockClose(&iNewSock);
	}

	RETiRet;
}


/* initialize tcp sockets for a listner. The initialized sockets are passed to the
 * app-level caller via a callback.
 * pLstnPort must point to a port name or number. NULL is NOT permitted. pLstnIP
 * points to the port to listen to (NULL means "all"), iMaxSess has the maximum
 * number of sessions permitted.
 * rgerhards, 2008-04-22
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
static rsRetVal
LstnInit(netstrms_t *pNS, void *pUsr, rsRetVal(*fAddLstn)(void*,netstrm_t*),
	 uchar *pLstnPort, uchar *pLstnIP, int iSessMax,
	 uchar *pszLstnPortFileName)
{
	DEFiRet;
	netstrm_t *pNewStrm = NULL;
	nsd_t *pNewNsd = NULL;
	int error, maxs, on = 1;
	int isIPv6 = 0;
	int sock = -1;
	int numSocks;
	int sockflags;
	int port_override = 0; /* if dyn port (0): use this for actually bound port */
	struct addrinfo hints, *res = NULL, *r;

	ISOBJ_TYPE_assert(pNS, netstrms);
	assert(fAddLstn != NULL);
	assert(pLstnPort != NULL);
	assert(iSessMax >= 0);

	dbgprintf("creating tcp listen socket on port %s\n", pLstnPort);

	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = glbl.GetDefPFFamily();
	hints.ai_socktype = SOCK_STREAM;

	error = getaddrinfo((char*)pLstnIP, (char*) pLstnPort, &hints, &res);
	if(error) {
		LogError(0, RS_RET_INVALID_PORT, "error querying port '%s': %s",
			pLstnPort, gai_strerror(error));
		ABORT_FINALIZE(RS_RET_INVALID_PORT);
	}

	/* Count max number of sockets we may open */
	for(maxs = 0, r = res; r != NULL ; r = r->ai_next, maxs++)
		/* EMPTY */;

	numSocks = 0;   /* num of sockets counter at start of array */
	for(r = res; r != NULL ; r = r->ai_next) {
		if(port_override != 0) {
			if(r->ai_family == AF_INET6) {
				((struct sockaddr_in6*)r->ai_addr)->sin6_port = port_override;
			} else {
				((struct sockaddr_in*)r->ai_addr)->sin_port = port_override;
			}
		}
		sock = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
		if(sock < 0) {
			if(!(r->ai_family == PF_INET6 && errno == EAFNOSUPPORT)) {
				dbgprintf("error %d creating tcp listen socket", errno);
				/* it is debatable if PF_INET with EAFNOSUPPORT should
				 * also be ignored...
				 */
			}
			continue;
		}

		#ifdef IPV6_V6ONLY
			if(r->ai_family == AF_INET6) {
				isIPv6 = 1;
				int iOn = 1;
				if(setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY,
				      (char *)&iOn, sizeof (iOn)) < 0) {
					close(sock);
					sock = -1;
					continue;
				}
			}
		#endif
		if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on)) < 0 ) {
			dbgprintf("error %d setting tcp socket option\n", errno);
			close(sock);
			sock = -1;
			continue;
		}

		/* We use non-blocking IO! */
		if((sockflags = fcntl(sock, F_GETFL)) != -1) {
			sockflags |= O_NONBLOCK;
			/* SETFL could fail too, so get it caught by the subsequent
			 * error check.
			 */
			sockflags = fcntl(sock, F_SETFL, sockflags);
		}
		if(sockflags == -1) {
			dbgprintf("error %d setting fcntl(O_NONBLOCK) on tcp socket", errno);
			close(sock);
			sock = -1;
			continue;
		}

		/* We need to enable BSD compatibility. Otherwise an attacker
		 * could flood our log files by sending us tons of ICMP errors.
		 */
		#if !defined(_AIX) && !defined(BSD)
			if(net.should_use_so_bsdcompat()) {
				if (setsockopt(sock, SOL_SOCKET, SO_BSDCOMPAT,
						(char *) &on, sizeof(on)) < 0) {
					LogError(errno, NO_ERRCODE, "TCP setsockopt(BSDCOMPAT)");
					close(sock);
					sock = -1;
					continue;
				}
			}
		#endif

	        if( (bind(sock, r->ai_addr, r->ai_addrlen) < 0)
			#ifndef IPV6_V6ONLY
			&& (errno != EADDRINUSE)
			#endif
	           ) {
			/* TODO: check if *we* bound the socket - else we *have* an error! */
			char errStr[1024];
			rs_strerror_r(errno, errStr, sizeof(errStr));
			LogError(errno, NO_ERRCODE, "Error while binding tcp socket");
			dbgprintf("error %d while binding tcp socket: %s\n", errno, errStr);
			close(sock);
			sock = -1;
			continue;
		}

		/* if we bind to dynamic port (port 0 given), we will do so consistently. Thus
		 * once we got a dynamic port, we will keep it and use it for other protocols
		 * as well. As of my understanding, this should always work as the OS does not
		 * pick a port that is used by some protocol (well, at least this looks very
		 * unlikely...). If our asusmption is wrong, we should iterate until we find a
		 * combination that works - it is very unusual to have the same service listen
		 * on differnt ports on IPv4 and IPv6.
		 */
		const int currport = (isIPv6) ?
			(((struct sockaddr_in6*)r->ai_addr)->sin6_port) :
			(((struct sockaddr_in*)r->ai_addr)->sin_port) ;
		if(currport == 0) {
			if(getsockname(sock, r->ai_addr, &r->ai_addrlen) == -1) {
				LogError(errno, NO_ERRCODE, "nsd_ptcp: ListenPortFileName: getsockname:"
						"error while trying to get socket");
			}
			port_override = (isIPv6) ?
				(((struct sockaddr_in6*)r->ai_addr)->sin6_port) :
				(((struct sockaddr_in*)r->ai_addr)->sin_port) ;
			if(pszLstnPortFileName != NULL) {
				FILE *fp;
				if((fp = fopen((const char*)pszLstnPortFileName, "w+")) == NULL) {
					LogError(errno, RS_RET_IO_ERROR, "nsd_ptcp: ListenPortFileName: "
							"error while trying to open file");
					ABORT_FINALIZE(RS_RET_IO_ERROR);
				}
				if(isIPv6) {
					fprintf(fp, "%d", ntohs((((struct sockaddr_in6*)r->ai_addr)->sin6_port)));
				} else {
					fprintf(fp, "%d", ntohs((((struct sockaddr_in*)r->ai_addr)->sin_port)));
				}
				fclose(fp);
			}
		}

		if(listen(sock, iSessMax / 10 + 5) < 0) {
			/* If the listen fails, it most probably fails because we ask
			 * for a too-large backlog. So in this case we first set back
			 * to a fixed, reasonable, limit that should work. Only if
			 * that fails, too, we give up.
			 */
			dbgprintf("listen with a backlog of %d failed - retrying with default of 32.\n",
				   iSessMax / 10 + 5);
			if(listen(sock, 32) < 0) {
				dbgprintf("tcp listen error %d, suspending\n", errno);
	                	close(sock);
				sock = -1;
				continue;
			}
		}


		/* if we reach this point, we were able to obtain a valid socket, so we can
		 * construct a new netstrm obj and hand it over to the upper layers for inclusion
		 * into their socket array. -- rgerhards, 2008-04-23
		 */
		CHKiRet(pNS->Drvr.Construct(&pNewNsd));
		CHKiRet(pNS->Drvr.SetSock(pNewNsd, sock));
		CHKiRet(pNS->Drvr.SetMode(pNewNsd, netstrms.GetDrvrMode(pNS)));
		CHKiRet(pNS->Drvr.SetAuthMode(pNewNsd, netstrms.GetDrvrAuthMode(pNS)));
		CHKiRet(pNS->Drvr.SetPermPeers(pNewNsd, netstrms.GetDrvrPermPeers(pNS)));
		CHKiRet(pNS->Drvr.SetGnutlsPriorityString(pNewNsd, netstrms.GetDrvrGnutlsPriorityString(pNS)));
		CHKiRet(netstrms.CreateStrm(pNS, &pNewStrm));
		pNewStrm->pDrvrData = (nsd_t*) pNewNsd;
		pNewNsd = NULL;
		CHKiRet(fAddLstn(pUsr, pNewStrm));
		pNewStrm = NULL;
		/* sock has been handed over by SetSock() above, so invalidate it here
		 * coverity scan falsely identifies this as ressource leak
		 */
		sock = -1;
		++numSocks;
	}

	if(numSocks != maxs)
		dbgprintf("We could initialize %d TCP listen sockets out of %d we received "
		 	  "- this may or may not be an error indication.\n", numSocks, maxs);

	if(numSocks == 0) {
		dbgprintf("No TCP listen sockets could successfully be initialized\n");
		ABORT_FINALIZE(RS_RET_COULD_NOT_BIND);
	}

finalize_it:
	if(sock != -1) {
		close(sock);
	}
	if(res != NULL)
		freeaddrinfo(res);

	if(iRet != RS_RET_OK) {
		if(pNewStrm != NULL)
			netstrm.Destruct(&pNewStrm);
		if(pNewNsd != NULL)
			pNS->Drvr.Destruct(&pNewNsd);
	}

	RETiRet;
}
#pragma GCC diagnostic pop

/* receive data from a tcp socket
 * The lenBuf parameter must contain the max buffer size on entry and contains
 * the number of octets read (or -1 in case of error) on exit. This function
 * never blocks, not even when called on a blocking socket. That is important
 * for client sockets, which are set to block during send, but should not
 * block when trying to read data. If *pLenBuf is -1, an error occured and
 * oserr holds the exact error cause.
 * rgerhards, 2008-03-17
 */
static rsRetVal
Rcv(nsd_t *pNsd, uchar *pRcvBuf, ssize_t *pLenBuf, int *const oserr)
{
	char errStr[1024];
	DEFiRet;
	nsd_ptcp_t *pThis = (nsd_ptcp_t*) pNsd;
	ISOBJ_TYPE_assert(pThis, nsd_ptcp);
	assert(oserr != NULL);
/*  AIXPORT : MSG_DONTWAIT not supported */
#if defined (_AIX)
#define MSG_DONTWAIT    MSG_NONBLOCK
#endif

	*pLenBuf = recv(pThis->sock, pRcvBuf, *pLenBuf, MSG_DONTWAIT);
	*oserr = errno;

	if(*pLenBuf == 0) {
		ABORT_FINALIZE(RS_RET_CLOSED);
	} else if (*pLenBuf < 0) {
		rs_strerror_r(errno, errStr, sizeof(errStr));
		dbgprintf("error during recv on NSD %p: %s\n", pNsd, errStr);
		ABORT_FINALIZE(RS_RET_RCV_ERR);
	}

finalize_it:
	RETiRet;
}


/* send a buffer. On entry, pLenBuf contains the number of octets to
 * write. On exit, it contains the number of octets actually written.
 * If this number is lower than on entry, only a partial buffer has
 * been written.
 * rgerhards, 2008-03-19
 */
static rsRetVal
Send(nsd_t *pNsd, uchar *pBuf, ssize_t *pLenBuf)
{
	nsd_ptcp_t *pThis = (nsd_ptcp_t*) pNsd;
	ssize_t written;
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, nsd_ptcp);

	written = send(pThis->sock, pBuf, *pLenBuf, 0);

	if(written == -1) {
		switch(errno) {
			case EAGAIN:
			case EINTR:
				/* this is fine, just retry... */
				written = 0;
				break;
			default:
				ABORT_FINALIZE(RS_RET_IO_ERROR);
				break;
		}
	}

	*pLenBuf = written;
finalize_it:
	RETiRet;
}


/* Enable KEEPALIVE handling on the socket.
 * rgerhards, 2009-06-02
 */
static rsRetVal
EnableKeepAlive(nsd_t *pNsd)
{
	nsd_ptcp_t *pThis = (nsd_ptcp_t*) pNsd;
	int ret;
	int optval;
	socklen_t optlen;
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, nsd_ptcp);

	optval = 1;
	optlen = sizeof(optval);
	ret = setsockopt(pThis->sock, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen);
	if(ret < 0) {
		dbgprintf("EnableKeepAlive socket call returns error %d\n", ret);
		ABORT_FINALIZE(RS_RET_ERR);
	}

#	if defined(IPPROTO_TCP) && defined(TCP_KEEPCNT)
	if(pThis->iKeepAliveProbes > 0) {
		optval = pThis->iKeepAliveProbes;
		optlen = sizeof(optval);
		ret = setsockopt(pThis->sock, IPPROTO_TCP, TCP_KEEPCNT, &optval, optlen);
	} else {
		ret = 0;
	}
#	else
	ret = -1;
#	endif
	if(ret < 0) {
		LogError(ret, NO_ERRCODE, "imptcp cannot set keepalive probes - ignored");
	}

#	if defined(IPPROTO_TCP) && defined(TCP_KEEPIDLE)
	if(pThis->iKeepAliveTime > 0) {
		optval = pThis->iKeepAliveTime;
		optlen = sizeof(optval);
		ret = setsockopt(pThis->sock, IPPROTO_TCP, TCP_KEEPIDLE, &optval, optlen);
	} else {
		ret = 0;
	}
#	else
	ret = -1;
#	endif
	if(ret < 0) {
		LogError(ret, NO_ERRCODE, "imptcp cannot set keepalive time - ignored");
	}

#	if defined(IPPROTO_TCP) && defined(TCP_KEEPCNT)
	if(pThis->iKeepAliveIntvl > 0) {
		optval = pThis->iKeepAliveIntvl;
		optlen = sizeof(optval);
		ret = setsockopt(pThis->sock, IPPROTO_TCP, TCP_KEEPINTVL, &optval, optlen);
	} else {
		ret = 0;
	}
#	else
	ret = -1;
#	endif
	if(ret < 0) {
		LogError(errno, NO_ERRCODE, "imptcp cannot set keepalive intvl - ignored");
	}

	dbgprintf("KEEPALIVE enabled for socket %d\n", pThis->sock);

finalize_it:
	RETiRet;

}


/* open a connection to a remote host (server).
 * rgerhards, 2008-03-19
 */
static rsRetVal
Connect(nsd_t *pNsd, int family, uchar *port, uchar *host, char *device)
{
	nsd_ptcp_t *pThis = (nsd_ptcp_t*) pNsd;
	struct addrinfo *res = NULL;
	struct addrinfo hints;

	DEFiRet;
	ISOBJ_TYPE_assert(pThis, nsd_ptcp);
	assert(port != NULL);
	assert(host != NULL);
	assert(pThis->sock == -1);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = family;
	hints.ai_socktype = SOCK_STREAM;
	if(getaddrinfo((char*)host, (char*)port, &hints, &res) != 0) {
		LogError(errno, RS_RET_IO_ERROR, "cannot resolve hostname '%s'",
			host);
		ABORT_FINALIZE(RS_RET_IO_ERROR);
	}

	if((pThis->sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
		LogError(errno, RS_RET_IO_ERROR, "cannot bind socket for %s:%s",
			host, port);
		ABORT_FINALIZE(RS_RET_IO_ERROR);
	}

	if(device) {
#		if defined(SO_BINDTODEVICE)
		if(setsockopt(pThis->sock, SOL_SOCKET, SO_BINDTODEVICE, device, strlen(device) + 1) < 0)
#		endif
		{
			dbgprintf("setsockopt(SO_BINDTODEVICE) failed\n");
			ABORT_FINALIZE(RS_RET_IO_ERROR);
		}
	}

	if(connect(pThis->sock, res->ai_addr, res->ai_addrlen) != 0) {
		LogError(errno, RS_RET_IO_ERROR, "cannot connect to %s:%s",
			host, port);
		ABORT_FINALIZE(RS_RET_IO_ERROR);
	}

finalize_it:
	if(res != NULL)
		freeaddrinfo(res);

	if(iRet != RS_RET_OK) {
		sockClose(&pThis->sock);
	}

	RETiRet;
}


/* get the remote hostname. The returned hostname must be freed by the
 * caller.
 * rgerhards, 2008-04-24
 */
static rsRetVal
GetRemoteHName(nsd_t *pNsd, uchar **ppszHName)
{
	DEFiRet;
	nsd_ptcp_t *pThis = (nsd_ptcp_t*) pNsd;
	ISOBJ_TYPE_assert(pThis, nsd_ptcp);
	assert(ppszHName != NULL);

	// TODO: how can the RemHost be empty?
	CHKmalloc(*ppszHName = (uchar*)strdup(pThis->pRemHostName == NULL ? "" : (char*) pThis->pRemHostName));

finalize_it:
	RETiRet;
}


/* This function checks if the connection is still alive - well, kind of... It
 * is primarily being used for plain TCP syslog and it is quite a hack. However,
 * as it seems to work, it is worth supporting it. The bottom line is that it
 * should not be called by anything else but a plain tcp syslog sender.
 * In order for it to work, it must be called *immediately* *before* the send()
 * call. For details about what is done, see here:
 * http://blog.gerhards.net/2008/06/getting-bit-more-reliability-from-plain.html
 * rgerhards, 2008-06-09
 */
static rsRetVal
CheckConnection(nsd_t *pNsd)
{
	DEFiRet;
	int rc;
	char msgbuf[1]; /* dummy */
	nsd_ptcp_t *pThis = (nsd_ptcp_t*) pNsd;
	ISOBJ_TYPE_assert(pThis, nsd_ptcp);

	rc = recv(pThis->sock, msgbuf, 1, MSG_DONTWAIT | MSG_PEEK);
	if(rc == 0) {
		dbgprintf("CheckConnection detected broken connection - closing it\n");
		/* in this case, the remote peer had shut down the connection and we
		 * need to close our side, too.
		 */
		sockClose(&pThis->sock);
		ABORT_FINALIZE(RS_RET_IO_ERROR);
	}
finalize_it:
	RETiRet;
}


/* get the remote host's IP address. Caller must Destruct the object.
 */
static rsRetVal
GetRemoteIP(nsd_t *pNsd, prop_t **ip)
{
	DEFiRet;
	nsd_ptcp_t *pThis = (nsd_ptcp_t*) pNsd;
	ISOBJ_TYPE_assert(pThis, nsd_ptcp);
	prop.AddRef(pThis->remoteIP);
	*ip = pThis->remoteIP;
	RETiRet;
}


/* queryInterface function */
BEGINobjQueryInterface(nsd_ptcp)
CODESTARTobjQueryInterface(nsd_ptcp)
	if(pIf->ifVersion != nsdCURR_IF_VERSION) {/* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->Construct = (rsRetVal(*)(nsd_t**)) nsd_ptcpConstruct;
	pIf->Destruct = (rsRetVal(*)(nsd_t**)) nsd_ptcpDestruct;
	pIf->Abort = Abort;
	pIf->GetRemAddr = GetRemAddr;
	pIf->GetSock = GetSock;
	pIf->SetSock = SetSock;
	pIf->SetMode = SetMode;
	pIf->SetAuthMode = SetAuthMode;
	pIf->SetGnutlsPriorityString = SetGnutlsPriorityString;
	pIf->SetPermPeers = SetPermPeers;
	pIf->Rcv = Rcv;
	pIf->Send = Send;
	pIf->LstnInit = LstnInit;
	pIf->AcceptConnReq = AcceptConnReq;
	pIf->Connect = Connect;
	pIf->GetRemoteHName = GetRemoteHName;
	pIf->GetRemoteIP = GetRemoteIP;
	pIf->CheckConnection = CheckConnection;
	pIf->EnableKeepAlive = EnableKeepAlive;
	pIf->SetKeepAliveIntvl = SetKeepAliveIntvl;
	pIf->SetKeepAliveProbes = SetKeepAliveProbes;
	pIf->SetKeepAliveTime = SetKeepAliveTime;
finalize_it:
ENDobjQueryInterface(nsd_ptcp)


/* exit our class
 */
BEGINObjClassExit(nsd_ptcp, OBJ_IS_LOADABLE_MODULE) /* CHANGE class also in END MACRO! */
CODESTARTObjClassExit(nsd_ptcp)
	/* release objects we no longer need */
	objRelease(net, CORE_COMPONENT);
	objRelease(glbl, CORE_COMPONENT);
	objRelease(prop, CORE_COMPONENT);
	objRelease(netstrm, DONT_LOAD_LIB);
	objRelease(netstrms, LM_NETSTRMS_FILENAME);
ENDObjClassExit(nsd_ptcp)


/* Initialize the nsd_ptcp class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-02-19
 */
BEGINObjClassInit(nsd_ptcp, 1, OBJ_IS_LOADABLE_MODULE) /* class, version */
	/* request objects we use */
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(prop, CORE_COMPONENT));
	CHKiRet(objUse(net, CORE_COMPONENT));
	CHKiRet(objUse(netstrms, LM_NETSTRMS_FILENAME));
	CHKiRet(objUse(netstrm, DONT_LOAD_LIB));

	/* set our own handlers */
ENDObjClassInit(nsd_ptcp)


/* --------------- here now comes the plumbing that makes as a library module --------------- */


BEGINmodExit
CODESTARTmodExit
#	ifdef HAVE_EPOLL_CREATE /* module only available if epoll() is supported! */
	nsdpoll_ptcpClassExit();
#	endif
	nsdsel_ptcpClassExit();
	nsd_ptcpClassExit();
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_LIB_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */

	/* Initialize all classes that are in our module - this includes ourselfs */
	CHKiRet(nsd_ptcpClassInit(pModInfo)); /* must be done after tcps_sess, as we use it */
	CHKiRet(nsdsel_ptcpClassInit(pModInfo)); /* must be done after tcps_sess, as we use it */
#	ifdef HAVE_EPOLL_CREATE /* module only available if epoll() is supported! */
	CHKiRet(nsdpoll_ptcpClassInit(pModInfo)); /* must be done after tcps_sess, as we use it */
#	endif
ENDmodInit
/* vi:set ai:
 */
