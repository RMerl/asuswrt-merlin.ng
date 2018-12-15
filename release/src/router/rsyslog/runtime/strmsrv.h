/* Definitions for strmsrv class.
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
#ifndef INCLUDED_STRMSRV_H
#define INCLUDED_STRMSRV_H

#include "obj.h"
#include "strms_sess.h"

/* list of strm listen ports */
struct strmLstnPortList_s {
	uchar *pszPort;			/**< the ports the listener shall listen on */
	uchar *pszInputName;		/**< value to be used as input name */
	strmsrv_t *pSrv;			/**< pointer to higher-level server instance */
	strmLstnPortList_t *pNext;	/**< next port or NULL */
};


/* the strmsrv object */
struct strmsrv_s {
	BEGINobjInstance;	/**< Data to implement generic object - MUST be the first data element! */
	int bUseKeepAlive;	/**< use socket layer KEEPALIVE handling? */
	int iKeepAliveIntvl;	/**< socket layer KEEPALIVE interval */
	int iKeepAliveProbes;	/**< socket layer KEEPALIVE probes */
	int iKeepAliveTime;	/**< socket layer KEEPALIVE timeout */
	netstrms_t *pNS;	/**< pointer to network stream subsystem */
	int iDrvrMode;		/**< mode of the stream driver to use */
	uchar *pszDrvrAuthMode;	/**< auth mode of the stream driver to use */
	uchar *pszInputName;	/**< value to be used as input name */
	permittedPeers_t *pPermPeers;/**< driver's permitted peers */
	int iLstnMax;		/**< max nbr of listeners currently supported */
	netstrm_t **ppLstn;	/**< our netstream listeners */
	strmLstnPortList_t **ppLstnPort; /**< pointer to relevant listen port description */
	int iSessMax;		/**< max number of sessions supported */
	strmLstnPortList_t *pLstnPorts;	/**< head pointer for listen ports */
	int addtlFrameDelim;
	/**< additional frame delimiter for plain STRM syslog framing (e.g. to handle NetScreen) */
	strms_sess_t **pSessions;/**< array of all of our sessions */
	void *pUsr;		/**< a user-settable pointer (provides extensibility for "derived classes")*/
	/* callbacks */
	int      (*pIsPermittedHost)(struct sockaddr *addr, char *fromHostFQDN, void*pUsrSrv, void*pUsrSess);
	rsRetVal (*pRcvData)(strms_sess_t*, char*, size_t, ssize_t *, int *);
	rsRetVal (*OpenLstnSocks)(struct strmsrv_s*);
	rsRetVal (*pOnListenDeinit)(void*);
	rsRetVal (*OnDestruct)(void*);
	rsRetVal (*pOnRegularClose)(strms_sess_t *pSess);
	rsRetVal (*pOnErrClose)(strms_sess_t *pSess);
	/* session specific callbacks */
	rsRetVal (*pOnSessAccept)(strmsrv_t *, strms_sess_t*);
	rsRetVal (*OnSessConstructFinalize)(void*);
	rsRetVal (*pOnSessDestruct)(void*);
	rsRetVal (*OnCharRcvd)(strms_sess_t*, uchar);
};


/* interfaces */
BEGINinterface(strmsrv) /* name must also be changed in ENDinterface macro! */
	INTERFACEObjDebugPrint(strmsrv);
	rsRetVal (*Construct)(strmsrv_t **ppThis);
	rsRetVal (*ConstructFinalize)(strmsrv_t __attribute__((unused)) *pThis);
	rsRetVal (*Destruct)(strmsrv_t **ppThis);
	rsRetVal (*configureSTRMListen)(strmsrv_t*, uchar *pszPort);
	//rsRetVal (*SessAccept)(strmsrv_t *pThis, strmLstnPortList_t*, strms_sess_t **ppSess, netstrm_t *pStrm);
	rsRetVal (*create_strm_socket)(strmsrv_t *pThis);
	rsRetVal (*Run)(strmsrv_t *pThis);
	/* set methods */
	rsRetVal (*SetAddtlFrameDelim)(strmsrv_t*, int);
	rsRetVal (*SetInputName)(strmsrv_t*, uchar*);
	rsRetVal (*SetKeepAlive)(strmsrv_t*, int);
	rsRetVal (*SetUsrP)(strmsrv_t*, void*);
	rsRetVal (*SetCBIsPermittedHost)(strmsrv_t*, int (*) (struct sockaddr *addr, char*, void*, void*));
	rsRetVal (*SetCBOpenLstnSocks)(strmsrv_t *, rsRetVal (*)(strmsrv_t*));
	rsRetVal (*SetCBOnDestruct)(strmsrv_t*, rsRetVal (*) (void*));
	rsRetVal (*SetCBOnRegularClose)(strmsrv_t*, rsRetVal (*) (strms_sess_t*));
	rsRetVal (*SetCBOnErrClose)(strmsrv_t*, rsRetVal (*) (strms_sess_t*));
	rsRetVal (*SetDrvrMode)(strmsrv_t *pThis, int iMode);
	rsRetVal (*SetDrvrAuthMode)(strmsrv_t *pThis, uchar *pszMode);
	rsRetVal (*SetDrvrPermPeers)(strmsrv_t *pThis, permittedPeers_t*);
	/* session specifics */
	rsRetVal (*SetCBOnSessAccept)(strmsrv_t*, rsRetVal (*) (strmsrv_t*, strms_sess_t*));
	rsRetVal (*SetCBOnSessDestruct)(strmsrv_t*, rsRetVal (*) (void*));
	rsRetVal (*SetCBOnSessConstructFinalize)(strmsrv_t*, rsRetVal (*) (void*));
	rsRetVal (*SetSessMax)(strmsrv_t *pThis, int iMaxSess);
	rsRetVal (*SetOnCharRcvd)(strmsrv_t *pThis, rsRetVal (*OnMsgCharRcvd)(strms_sess_t*, uchar));
	/* v2 */
	rsRetVal (*SetKeepAliveProbes)(strmsrv_t *pThis, int keepAliveProbes);
	rsRetVal (*SetKeepAliveTime)(strmsrv_t *pThis, int keepAliveTime);
	rsRetVal (*SetKeepAliveIntvl)(strmsrv_t *pThis, int keepAliveIntvl);
ENDinterface(strmsrv)
#define strmsrvCURR_IF_VERSION 2 /* increment whenever you change the interface structure! */
/* interface version 2 added keep alive parameter set functions
 */


/* prototypes */
PROTOTYPEObj(strmsrv);

/* the name of our library binary */
#define LM_STRMSRV_FILENAME "lmstrmsrv"

#endif /* #ifndef INCLUDED_STRMSRV_H */
