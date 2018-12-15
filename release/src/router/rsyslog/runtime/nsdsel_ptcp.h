/* An implementation of the nsd select interface for plain tcp sockets.
 *
 * Copyright 2008 Rainer Gerhards and Adiscon GmbH.
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

#ifndef INCLUDED_NSDSEL_PTCP_H
#define INCLUDED_NSDSEL_PTCP_H

#include <poll.h>
#include "nsd.h"
typedef nsdsel_if_t nsdsel_ptcp_if_t; /* we just *implement* this interface */

/* the nsdsel_ptcp object */
struct nsdsel_ptcp_s {
	BEGINobjInstance;
	uint32_t maxfds;
	uint32_t currfds;
	struct pollfd *fds;
};

/* interface is defined in nsd.h, we just implement it! */
#define nsdsel_ptcpCURR_IF_VERSION nsdCURR_IF_VERSION

/* prototypes */
PROTOTYPEObj(nsdsel_ptcp);

#endif /* #ifndef INCLUDED_NSDSEL_PTCP_H */
