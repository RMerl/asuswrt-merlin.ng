/* Definitions for tcps_sess class. This implements a session of the
 * plain TCP server.
 *
 * Copyright 2008-2015 Adiscon GmbH.
 *
 * This file is part of rsyslog.
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
#ifndef INCLUDED_TCPS_SESS_H
#define INCLUDED_TCPS_SESS_H

#include "obj.h"
#include "prop.h"

/* a forward-definition, we are somewhat cyclic */
struct tcpsrv_s;

/* the tcps_sess object */
struct tcps_sess_s {
	BEGINobjInstance;	/* Data to implement generic object - MUST be the first data element! */
	tcpsrv_t *pSrv;	/* pointer back to my server (e.g. for callbacks) */
	tcpLstnPortList_t *pLstnInfo;	/* pointer back to listener info */
	netstrm_t *pStrm;
	int iMsg;		 /* index of next char to store in msg */
	sbool bSuppOctetFram;	/**< copy from listener, to speed up access */
	sbool bSPFramingFix;
	enum {
		eAtStrtFram,
		eInOctetCnt,
		eInMsg,
		eInMsgTruncating
	} inputState;		/* our current state */
	int iOctetsRemain;	/* Number of Octets remaining in message */
	TCPFRAMINGMODE eFraming;
	uchar *pMsg;		/* message (fragment) received */
	prop_t *fromHost;	/* host name we received messages from */
	prop_t *fromHostIP;
	void *pUsr;		/* a user-pointer */
	rsRetVal (*DoSubmitMessage)(tcps_sess_t*, uchar*, int); /* submit message callback */
};


/* interfaces */
BEGINinterface(tcps_sess) /* name must also be changed in ENDinterface macro! */
	INTERFACEObjDebugPrint(tcps_sess);
	rsRetVal (*Construct)(tcps_sess_t **ppThis);
	rsRetVal (*ConstructFinalize)(tcps_sess_t __attribute__((unused)) *pThis);
	rsRetVal (*Destruct)(tcps_sess_t **ppThis);
	rsRetVal (*PrepareClose)(tcps_sess_t *pThis);
	rsRetVal (*Close)(tcps_sess_t *pThis);
	rsRetVal (*DataRcvd)(tcps_sess_t *pThis, char *pData, size_t iLen);
	/* set methods */
	rsRetVal (*SetTcpsrv)(tcps_sess_t *pThis, struct tcpsrv_s *pSrv);
	rsRetVal (*SetLstnInfo)(tcps_sess_t *pThis, tcpLstnPortList_t *pLstnInfo);
	rsRetVal (*SetUsrP)(tcps_sess_t*, void*);
	rsRetVal (*SetHost)(tcps_sess_t *pThis, uchar*);
	rsRetVal (*SetHostIP)(tcps_sess_t *pThis, prop_t*);
	rsRetVal (*SetStrm)(tcps_sess_t *pThis, netstrm_t*);
	rsRetVal (*SetMsgIdx)(tcps_sess_t *pThis, int);
	rsRetVal (*SetOnMsgReceive)(tcps_sess_t *pThis, rsRetVal (*OnMsgReceive)(tcps_sess_t*, uchar*, int));
ENDinterface(tcps_sess)
#define tcps_sessCURR_IF_VERSION 3 /* increment whenever you change the interface structure! */
/* interface changes
 * to version v2, rgerhards, 2009-05-22
 * - Data structures changed
 * - SetLstnInfo entry point added
 * version 3, rgerhards, 2013-01-21:
 * - signature of SetHostIP() changed
 */


/* prototypes */
PROTOTYPEObj(tcps_sess);


#endif /* #ifndef INCLUDED_TCPS_SESS_H */
