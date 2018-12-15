/* The interface definition for "NetStream Drivers" (nsd).
 *
 * This is just an abstract driver interface, which needs to be
 * implemented by concrete classes. As such, no nsd data type itself
 * is defined.
 *
 * Copyright 2008-2012 Adiscon GmbH.
 *
 * This file is part of the rsyslog runtime library.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *       -or-
 *       see COPYING.ASL20 in the source distribution
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef INCLUDED_NSD_H
#define INCLUDED_NSD_H

#include <sys/socket.h>

/**
 * The following structure is a set of descriptors that need to be processed.
 * This set will be the result of the epoll call and be used
 * in the actual request processing stage. -- rgerhards, 2011-01-24
 */
struct nsd_epworkset_s {
	int id;
	void *pUsr;
};

enum nsdsel_waitOp_e {
	NSDSEL_RD = 1,
	NSDSEL_WR = 2,
	NSDSEL_RDWR = 3
}; /**< the operation we wait for */

/* nsd_t is actually obj_t (which is somewhat better than void* but in essence
 * much the same).
 */

/* interface */
BEGINinterface(nsd) /* name must also be changed in ENDinterface macro! */
	rsRetVal (*Construct)(nsd_t **ppThis);
	rsRetVal (*Destruct)(nsd_t **ppThis);
	rsRetVal (*Abort)(nsd_t *pThis);
	rsRetVal (*Rcv)(nsd_t *pThis, uchar *pRcvBuf, ssize_t *pLenBuf, int *oserr);
	rsRetVal (*Send)(nsd_t *pThis, uchar *pBuf, ssize_t *pLenBuf);
	rsRetVal (*Connect)(nsd_t *pThis, int family, unsigned char *port, unsigned char *host, char *device);
	rsRetVal (*AcceptConnReq)(nsd_t *pThis, nsd_t **ppThis);
	rsRetVal (*GetRemoteHName)(nsd_t *pThis, uchar **pszName);
	rsRetVal (*GetRemoteIP)(nsd_t *pThis, prop_t **ip);
	rsRetVal (*SetMode)(nsd_t *pThis, int mode); /* sets a driver specific mode - see driver doc for details */
	rsRetVal (*SetAuthMode)(nsd_t *pThis, uchar*); /* sets a driver specific mode - see driver doc for details */
	rsRetVal (*SetPermPeers)(nsd_t *pThis, permittedPeers_t*); /* sets driver permitted peers for auth needs */
	rsRetVal (*CheckConnection)(nsd_t *pThis);	/* This is a trick mostly for plain tcp syslog */
	rsRetVal (*GetSock)(nsd_t *pThis, int *pSock);
	rsRetVal (*SetSock)(nsd_t *pThis, int sock);
	/* GetSock() and SetSock() return an error if the driver does not use plain
	 * OS sockets. This interface is primarily meant as an internal aid for
	 * those drivers that utilize the nsd_ptcp to do some of their work.
	 */
	rsRetVal (*GetRemAddr)(nsd_t *pThis, struct sockaddr_storage **ppAddr);
	/* getRemAddr() is an aid needed by the legacy ACL system. It exposes the remote
	 * peer's socket addr structure, so that the legacy matching functions can work on
	 * it. Note that this ties netstream drivers to things that can be implemented over
	 * sockets - not really desirable, but not the end of the world... TODO: should be
	 * reconsidered when a new ACL system is build. -- rgerhards, 2008-12-01
	 */
	/* v5 */
	rsRetVal (*EnableKeepAlive)(nsd_t *pThis);
	/* v8 */
	rsRetVal (*SetKeepAliveIntvl)(nsd_t *pThis, int keepAliveIntvl);
	rsRetVal (*SetKeepAliveProbes)(nsd_t *pThis, int keepAliveProbes);
	rsRetVal (*SetKeepAliveTime)(nsd_t *pThis, int keepAliveTime);
	rsRetVal (*SetGnutlsPriorityString)(nsd_t *pThis, uchar *gnutlsPriorityString);
	/* v12 -- parameter pszLstnPortFileName added to LstnInit()*/
	rsRetVal (*LstnInit)(netstrms_t *pNS, void *pUsr, rsRetVal(*fAddLstn)(void*,netstrm_t*),
			     uchar *pLstnPort, uchar *pLstnIP, int iSessMax, uchar *pszLstnPortFileName);
ENDinterface(nsd)
#define nsdCURR_IF_VERSION 11 /* increment whenever you change the interface structure! */
/* interface version 4 added GetRemAddr()
 * interface version 5 added EnableKeepAlive() -- rgerhards, 2009-06-02
 * interface version 6 changed return of CheckConnection from void to rsRetVal -- alorbach, 2012-09-06
 * interface version 7 changed signature ofGetRempoteIP() -- rgerhards, 2013-01-21
 * interface version 8 added keep alive parameter set functions
 * interface version 9 changed signature of Connect() -- dsa, 2016-11-14
 * interface version 10 added SetGnutlsPriorityString() -- PascalWithopf, 2017-08-08
 * interface version 11 added oserr to Rcv() signature -- rgerhards, 2017-09-04
 */

/* interface  for the select call */
BEGINinterface(nsdsel) /* name must also be changed in ENDinterface macro! */
	rsRetVal (*Construct)(nsdsel_t **ppThis);
	rsRetVal (*Destruct)(nsdsel_t **ppThis);
	rsRetVal (*Add)(nsdsel_t *pNsdsel, nsd_t *pNsd, nsdsel_waitOp_t waitOp);
	rsRetVal (*Select)(nsdsel_t *pNsdsel, int *piNumReady);
	rsRetVal (*IsReady)(nsdsel_t *pNsdsel, nsd_t *pNsd, nsdsel_waitOp_t waitOp, int *pbIsReady);
ENDinterface(nsdsel)
#define nsdselCURR_IF_VERSION 1 /* increment whenever you change the interface structure! */

/* interface  for the epoll call */
BEGINinterface(nsdpoll) /* name must also be changed in ENDinterface macro! */
	rsRetVal (*Construct)(nsdpoll_t **ppThis);
	rsRetVal (*Destruct)(nsdpoll_t **ppThis);
	rsRetVal (*Ctl)(nsdpoll_t *pNsdpoll, nsd_t *pNsd, int id, void *pUsr, int mode, int op);
	rsRetVal (*Wait)(nsdpoll_t *pNsdpoll, int timeout, int *numReady, nsd_epworkset_t workset[]);
ENDinterface(nsdpoll)
#define nsdpollCURR_IF_VERSION 1 /* increment whenever you change the interface structure! */

#endif /* #ifndef INCLUDED_NSD_H */
