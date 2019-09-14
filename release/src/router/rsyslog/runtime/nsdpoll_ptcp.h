/* An implementation of the nsd poll interface for plain tcp sockets.
 *
 * Copyright 2009-2016 Rainer Gerhards and Adiscon GmbH.
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

#ifndef INCLUDED_NSDPOLL_PTCP_H
#define INCLUDED_NSDPOLL_PTCP_H

#include "nsd.h"
#ifdef HAVE_SYS_EPOLL_H
#	include <sys/epoll.h>
#endif
typedef nsdpoll_if_t nsdpoll_ptcp_if_t; /* we just *implement* this interface */
/* a helper object to keep track of the epoll event records
 * Note that we need to keep track of that list because we need to
 * free the events when they are no longer needed.
 */
typedef struct nsdpoll_epollevt_lst_s nsdpoll_epollevt_lst_t;
struct nsdpoll_epollevt_lst_s {
#ifdef HAVE_SYS_EPOLL_H
	epoll_event_t event;
#endif
	int id;
	void *pUsr;
	nsd_ptcp_t *pSock;	/* our associated netstream driver data */
	nsdpoll_epollevt_lst_t *pNext;
};

/* the nsdpoll_ptcp object */
struct nsdpoll_ptcp_s {
	BEGINobjInstance;	/* Data to implement generic object - MUST be the first data element! */
	int efd;		/* file descriptor used by epoll */
	nsdpoll_epollevt_lst_t *pRoot;	/* Root of the epoll event list */
	pthread_mutex_t mutEvtLst;
};

/* interface is defined in nsd.h, we just implement it! */
#define nsdpoll_ptcpCURR_IF_VERSION nsdCURR_IF_VERSION

/* prototypes */
PROTOTYPEObj(nsdpoll_ptcp);

#endif /* #ifndef INCLUDED_NSDPOLL_PTCP_H */
