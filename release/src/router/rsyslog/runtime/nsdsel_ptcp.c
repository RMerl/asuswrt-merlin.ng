/* nsdsel_ptcp.c
 *
 * An implementation of the nsd select() interface for plain tcp sockets.
 *
 * Copyright 2008-2018 Rainer Gerhards and Adiscon GmbH.
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

#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/select.h>

#include "rsyslog.h"
#include "module-template.h"
#include "obj.h"
#include "errmsg.h"
#include "nsd_ptcp.h"
#include "nsdsel_ptcp.h"
#include "unlimited_select.h"

#define FDSET_INCREMENT 1024 /* increment for struct pollfds array allocation */
/* static data */
DEFobjStaticHelpers
DEFobjCurrIf(glbl)


/* Standard-Constructor */
BEGINobjConstruct(nsdsel_ptcp) /* be sure to specify the object type also in END macro! */
	pThis->currfds = 0;
	pThis->maxfds = FDSET_INCREMENT;
	CHKmalloc(pThis->fds = calloc(FDSET_INCREMENT, sizeof(struct pollfd)));
finalize_it:
ENDobjConstruct(nsdsel_ptcp)


/* destructor for the nsdsel_ptcp object */
BEGINobjDestruct(nsdsel_ptcp) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDestruct(nsdsel_ptcp)
	free(pThis->fds);
ENDobjDestruct(nsdsel_ptcp)


/* Add a socket to the select set */
static rsRetVal ATTR_NONNULL()
Add(nsdsel_t *const pNsdsel, nsd_t *const pNsd, const nsdsel_waitOp_t waitOp)
{
	DEFiRet;
	nsdsel_ptcp_t *const pThis = (nsdsel_ptcp_t*) pNsdsel;
	const nsd_ptcp_t *const pSock = (nsd_ptcp_t*) pNsd;
	ISOBJ_TYPE_assert(pSock, nsd_ptcp);
	ISOBJ_TYPE_assert(pThis, nsdsel_ptcp);

	if(pThis->currfds == pThis->maxfds) {
		struct pollfd *newfds;
		CHKmalloc(newfds = realloc(pThis->fds,
			sizeof(struct pollfd) * (pThis->maxfds + FDSET_INCREMENT)));
		pThis->maxfds += FDSET_INCREMENT;
		pThis->fds = newfds;
	}

	switch(waitOp) {
		case NSDSEL_RD:
			pThis->fds[pThis->currfds].events = POLLIN;
			break;
		case NSDSEL_WR:
			pThis->fds[pThis->currfds].events = POLLOUT;
			break;
		case NSDSEL_RDWR:
			pThis->fds[pThis->currfds].events = POLLIN | POLLOUT;
			break;
	}
	pThis->fds[pThis->currfds].fd = pSock->sock;
	++pThis->currfds;

finalize_it:
	RETiRet;
}


/* perform the select()  piNumReady returns how many descriptors are ready for IO
 * TODO: add timeout!
 */
static rsRetVal ATTR_NONNULL()
Select(nsdsel_t *const pNsdsel, int *const piNumReady)
{
	DEFiRet;
	nsdsel_ptcp_t *pThis = (nsdsel_ptcp_t*) pNsdsel;

	ISOBJ_TYPE_assert(pThis, nsdsel_ptcp);
	assert(piNumReady != NULL);

	assert(pThis->currfds >= 1);
	if(Debug) {
		dbgprintf("--------<NSDSEL_PTCP> calling poll, active fds (%d): ", pThis->currfds);
		for(uint32_t i = 0; i <= pThis->currfds; ++i)
			dbgprintf("%d ", pThis->fds[i].fd);
		dbgprintf("\n");
	}

	/* now do the select */
	*piNumReady = poll(pThis->fds, pThis->currfds, -1);
	if(*piNumReady < 0) {
		if(errno == EINTR) {
			DBGPRINTF("nsdsel_ptcp received EINTR\n");
		} else {
			LogMsg(errno, RS_RET_POLL_ERR, LOG_WARNING,
				"ndssel_ptcp: poll system call failed, may cause further troubles");
		}
		*piNumReady = 0;
	}

	RETiRet;
}


/* check if a socket is ready for IO */
static rsRetVal ATTR_NONNULL()
IsReady(nsdsel_t *const pNsdsel, nsd_t *const pNsd, const nsdsel_waitOp_t waitOp, int *const pbIsReady)
{
	DEFiRet;
	const nsdsel_ptcp_t *const pThis = (nsdsel_ptcp_t*) pNsdsel;
	const nsd_ptcp_t *const pSock = (nsd_ptcp_t*) pNsd;
	ISOBJ_TYPE_assert(pThis, nsdsel_ptcp);
	ISOBJ_TYPE_assert(pSock, nsd_ptcp);
	const int sock = pSock->sock;
	// TODO: consider doing a binary search

	uint32_t idx;
	for(idx = 0 ; idx < pThis->currfds ; ++idx) {
		if(pThis->fds[idx].fd == sock)
			break;
	}
	if(idx >= pThis->currfds) {
		LogMsg(0, RS_RET_INTERNAL_ERROR, LOG_ERR,
			"ndssel_ptcp: could not find socket %d which should be present", sock);
		ABORT_FINALIZE(RS_RET_INTERNAL_ERROR);
	}

	const short revent = pThis->fds[idx].revents;
	assert(!(revent & POLLNVAL));
	switch(waitOp) {
		case NSDSEL_RD:
			*pbIsReady = revent & POLLIN;
			break;
		case NSDSEL_WR:
			*pbIsReady = revent & POLLOUT;
			break;
		case NSDSEL_RDWR:
			*pbIsReady = revent & (POLLIN | POLLOUT);
			break;
	}

finalize_it:
	RETiRet;
}


/* ------------------------------ end support for the select() interface ------------------------------ */


/* queryInterface function */
BEGINobjQueryInterface(nsdsel_ptcp)
CODESTARTobjQueryInterface(nsdsel_ptcp)
	if(pIf->ifVersion != nsdCURR_IF_VERSION) {/* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->Construct = (rsRetVal(*)(nsdsel_t**)) nsdsel_ptcpConstruct;
	pIf->Destruct = (rsRetVal(*)(nsdsel_t**)) nsdsel_ptcpDestruct;
	pIf->Add = Add;
	pIf->Select = Select;
	pIf->IsReady = IsReady;
finalize_it:
ENDobjQueryInterface(nsdsel_ptcp)


/* exit our class
 */
BEGINObjClassExit(nsdsel_ptcp, OBJ_IS_CORE_MODULE) /* CHANGE class also in END MACRO! */
CODESTARTObjClassExit(nsdsel_ptcp)
	/* release objects we no longer need */
	objRelease(glbl, CORE_COMPONENT);
ENDObjClassExit(nsdsel_ptcp)


/* Initialize the nsdsel_ptcp class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-02-19
 */
BEGINObjClassInit(nsdsel_ptcp, 1, OBJ_IS_CORE_MODULE) /* class, version */
	CHKiRet(objUse(glbl, CORE_COMPONENT));
ENDObjClassInit(nsdsel_ptcp)
