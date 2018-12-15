/* An implementation of the nsd interface for plain tcp sockets.
 *
 * Copyright 2007-2012 Adiscon GmbH.
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

#ifndef INCLUDED_NSD_PTCP_H
#define INCLUDED_NSD_PTCP_H

#include <sys/socket.h>

#include "nsd.h"
typedef nsd_if_t nsd_ptcp_if_t; /* we just *implement* this interface */

/* the nsd_ptcp object */
struct nsd_ptcp_s {
	BEGINobjInstance; /* Data to implement generic object - MUST be the first data element! */
	prop_t *remoteIP; /**< IP address of remote peer (currently used in server mode, only) */
	uchar *pRemHostName; /**< host name of remote peer (currently used in server mode, only) */
	struct sockaddr_storage remAddr; /**< remote addr as sockaddr - used for legacy ACL code */
	int sock;	/**< the socket we use for regular, single-socket, operations */
	int iKeepAliveIntvl;	/**< socket layer KEEPALIVE interval */
	int iKeepAliveProbes;	/**< socket layer KEEPALIVE probes */
	int iKeepAliveTime;	/**< socket layer KEEPALIVE timeout */
};

/* interface is defined in nsd.h, we just implement it! */
#define nsd_ptcpCURR_IF_VERSION nsdCURR_IF_VERSION

/* prototypes */
PROTOTYPEObj(nsd_ptcp);

/* the name of our library binary */
#define LM_NSD_PTCP_FILENAME "lmnsd_ptcp"

#endif /* #ifndef INCLUDED_NSD_PTCP_H */
