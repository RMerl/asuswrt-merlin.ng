/* tcpclt.h
 *
 * This are the definitions for the TCP based clients class.
 *
 * File begun on 2007-07-21 by RGerhards (extracted from syslogd.c)
 *
 * Copyright 2007-2012 Adiscon GmbH.
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
#ifndef	TCPCLT_H_INCLUDED
#define	TCPCLT_H_INCLUDED 1

#include "obj.h"

/* the tcpclt object */
typedef struct tcpclt_s {
	BEGINobjInstance;	/**< Data to implement generic object - MUST be the first data element! */
	TCPFRAMINGMODE tcp_framing;
	uchar tcp_framingDelimiter;
	char *prevMsg;
	short bResendLastOnRecon; /* should the last message be resent on a successful reconnect? */
	size_t lenPrevMsg;
	/* session specific callbacks */
	int iRebindInterval;	/* how often should the send socket be rebound? */
	int iNumMsgs;		/* number of messages during current "rebind session" */
	rsRetVal (*initFunc)(void*);
	rsRetVal (*sendFunc)(void*, char*, size_t);
	rsRetVal (*prepRetryFunc)(void*);
} tcpclt_t;


/* interfaces */
BEGINinterface(tcpclt) /* name must also be changed in ENDinterface macro! */
	rsRetVal (*Construct)(tcpclt_t **ppThis);
	rsRetVal (*ConstructFinalize)(tcpclt_t __attribute__((unused)) *pThis);
	rsRetVal (*Destruct)(tcpclt_t **ppThis);
	int (*Send)(tcpclt_t *pThis, void*pData, char*msg, size_t len);
	int (*CreateSocket)(struct addrinfo *addrDest);
	/* set methods */
	rsRetVal (*SetResendLastOnRecon)(tcpclt_t*, int);
	rsRetVal (*SetSendInit)(tcpclt_t*, rsRetVal (*)(void*));
	rsRetVal (*SetSendFrame)(tcpclt_t*, rsRetVal (*)(void*, char*, size_t));
	rsRetVal (*SetSendPrepRetry)(tcpclt_t*, rsRetVal (*)(void*));
	rsRetVal (*SetFraming)(tcpclt_t*, TCPFRAMINGMODE framing);
	/* v3, 2009-07-14*/
	rsRetVal (*SetRebindInterval)(tcpclt_t*, int iRebindInterval);
	/* v4, 2017-06-10*/
	rsRetVal (*SetFramingDelimiter)(tcpclt_t*, uchar tcp_framingDelimiter);
ENDinterface(tcpclt)
#define tcpcltCURR_IF_VERSION 4 /* increment whenever you change the interface structure! */


/* prototypes */
PROTOTYPEObj(tcpclt);

/* the name of our library binary */
#define LM_TCPCLT_FILENAME "lmtcpclt"

#endif /* #ifndef TCPCLT_H_INCLUDED */
/* vim:set ai:
 */
