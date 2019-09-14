/* Definitions for strms_sess class. This implements a session of the
 * generic stream server.
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
#ifndef INCLUDED_STRMS_SESS_H
#define INCLUDED_STRMS_SESS_H

#include "obj.h"

/* a forward-definition, we are somewhat cyclic */
struct strmsrv_s;

/* the strms_sess object */
struct strms_sess_s {
	BEGINobjInstance;	/* Data to implement generic object - MUST be the first data element! */
	strmsrv_t *pSrv;	/* pointer back to my server (e.g. for callbacks) */
	strmLstnPortList_t *pLstnInfo;	/* pointer back to listener info */
	netstrm_t *pStrm;
	uchar *fromHost;
	prop_t *fromHostIP;
	void *pUsr;		/* a user-pointer */
};


/* interfaces */
BEGINinterface(strms_sess) /* name must also be changed in ENDinterface macro! */
	INTERFACEObjDebugPrint(strms_sess);
	rsRetVal (*Construct)(strms_sess_t **ppThis);
	rsRetVal (*ConstructFinalize)(strms_sess_t __attribute__((unused)) *pThis);
	rsRetVal (*Destruct)(strms_sess_t **ppThis);
	rsRetVal (*Close)(strms_sess_t *pThis);
	rsRetVal (*DataRcvd)(strms_sess_t *pThis, char *pData, size_t iLen);
	/* set methods */
	rsRetVal (*SetStrmsrv)(strms_sess_t *pThis, struct strmsrv_s *pSrv);
	rsRetVal (*SetLstnInfo)(strms_sess_t *pThis, strmLstnPortList_t *pLstnInfo);
	rsRetVal (*SetUsrP)(strms_sess_t*, void*);
	void*    (*GetUsrP)(strms_sess_t*);
	rsRetVal (*SetHost)(strms_sess_t *pThis, uchar*);
	rsRetVal (*SetHostIP)(strms_sess_t *pThis, prop_t*);
	rsRetVal (*SetStrm)(strms_sess_t *pThis, netstrm_t*);
	rsRetVal (*SetOnMsgReceive)(strms_sess_t *pThis, rsRetVal (*OnMsgReceive)(strms_sess_t*, uchar*, int));
ENDinterface(strms_sess)
#define strms_sessCURR_IF_VERSION 3 /* increment whenever you change the interface structure! */
/* interface changes
 * to version v2, rgerhards, 2009-05-22
 * - Data structures changed
 * - SetLstnInfo entry point added
 * version 3, rgerhads, 2013-01-21:
 * - signature of SetHostIP() changed
 */


/* prototypes */
PROTOTYPEObj(strms_sess);


#endif /* #ifndef INCLUDED_STRMS_SESS_H */
