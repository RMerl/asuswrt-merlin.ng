/* imptcp.c
 * This is a native implementation of plain tcp. It is intentionally
 * duplicate work (imtcp). The intent is to gain very fast and simple
 * native ptcp support, utilizing the best interfaces Linux (no cross-
 * platform intended!) has to offer.
 *
 * Note that in this module we try out some new naming conventions,
 * so it may look a bit "different" from the other modules. We are
 * investigating if removing prefixes can help make code more readable.
 *
 * File begun on 2010-08-10 by RGerhards
 *
 * Copyright 2007-2018 Rainer Gerhards and Adiscon GmbH.
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
#include "config.h"
#if !defined(HAVE_EPOLL_CREATE)
#	error imptcp requires OS support for epoll - can not build
	/* imptcp gains speed by using modern Linux capabilities. As such,
	 * it can only be build on platforms supporting the epoll API.
	 */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <sys/queue.h>
#include <netinet/tcp.h>
#include <stdint.h>
#include <zlib.h>
#include <sys/stat.h>
#include <regex.h>
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include "rsyslog.h"
#include "cfsysline.h"
#include "prop.h"
#include "dirty.h"
#include "module-template.h"
#include "unicode-helper.h"
#include "glbl.h"
#include "errmsg.h"
#include "srUtils.h"
#include "datetime.h"
#include "ruleset.h"
#include "msg.h"
#include "parserif.h"
#include "statsobj.h"
#include "ratelimit.h"
#include "net.h" /* for permittedPeers, may be removed when this is removed */

/* the define is from tcpsrv.h, we need to find a new (but easier!!!) abstraction layer some time ... */
#define TCPSRV_NO_ADDTL_DELIMITER -1 /* specifies that no additional delimiter is to be used in TCP framing */

MODULE_TYPE_INPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("imptcp")

/* static data */
DEF_IMOD_STATIC_DATA
DEFobjCurrIf(glbl)
DEFobjCurrIf(net)
DEFobjCurrIf(prop)
DEFobjCurrIf(datetime)
DEFobjCurrIf(ruleset)
DEFobjCurrIf(statsobj)

/* forward references */
static void * wrkr(void *myself);

/* unfortunately, on some platforms EAGAIN == EWOULDBOLOCK and so checking against
 * both of them generates a gcc 8 warning for this reason. We do not want to disable
 * the warning, so we need to work around this via a macro.
 */
#if EAGAIN == EWOULDBLOCK
	#define CHK_EAGAIN_EWOULDBLOCK (errno == EAGAIN)
#else
	#define CHK_EAGAIN_EWOULDBLOCK (errno == EAGAIN | errno == EWOULDBLOCK)
#endif /* #if EAGAIN == EWOULDBOLOCK */

#define DFLT_wrkrMax 2
#define DFLT_inlineDispatchThreshold 1

#define COMPRESS_NEVER 0
#define COMPRESS_SINGLE_MSG 1	/* old, single-message compression */
/* all other settings are for stream-compression */
#define COMPRESS_STREAM_ALWAYS 2

/* config settings */
typedef struct configSettings_s {
	int bKeepAlive;			/* support keep-alive packets */
	int iKeepAliveIntvl;
	int iKeepAliveProbes;
	int iKeepAliveTime;
	int bEmitMsgOnClose;		/* emit an informational message on close by remote peer */
	int bEmitMsgOnOpen;
	int bSuppOctetFram;		/* support octet-counted framing? */
	int iAddtlFrameDelim;		/* addtl frame delimiter, e.g. for netscreen, default none */
	int maxFrameSize;
	uchar *pszInputName;		/* value for inputname property, NULL is OK and handled by core engine */
	uchar *lstnIP;			/* which IP we should listen on? */
	uchar *pszBindRuleset;
	int wrkrMax;			/* max number of workers (actually "helper workers") */
} configSettings_t;
static configSettings_t cs;

struct instanceConf_s {
	int bKeepAlive;			/* support keep-alive packets */
	int iKeepAliveIntvl;
	int iKeepAliveProbes;
	int iKeepAliveTime;
	int bEmitMsgOnClose;
	int bEmitMsgOnOpen;
	int bSuppOctetFram;		/* support octet-counted framing? */
	int bSPFramingFix;
	int iAddtlFrameDelim;
	int socketBacklog;
	sbool multiLine;
	uint8_t compressionMode;
	uchar *pszBindPort;		/* port to bind to */
	uchar *pszLstnPortFileName;	/* Name of the file with dynamic port used by testbench*/
	uchar *pszBindAddr;		/* IP to bind socket to */
	uchar *pszBindPath;     /* Path to bind socket to */
	uchar *pszBindRuleset;		/* name of ruleset to bind to */
	uchar *pszInputName;		/* value for inputname property, NULL is OK and handled by core engine */
	int fCreateMode;	/* file creation mode for open() */
	uid_t fileUID;	/* IDs for creation */
	gid_t fileGID;
	int maxFrameSize;
	int bFailOnPerms;	/* fail creation if chown fails? */
	ruleset_t *pBindRuleset;	/* ruleset to bind listener to (use system default if unspecified) */
	uchar *dfltTZ;
	sbool bUnlink;
	sbool discardTruncatedMsg;
	sbool flowControl;
	int ratelimitInterval;
	int ratelimitBurst;
	uchar *startRegex;
	regex_t start_preg;	/* compiled version of startRegex */
	struct instanceConf_s *next;
};


struct modConfData_s {
	rsconf_t *pConf;		/* our overall config object */
	instanceConf_t *root, *tail;
	int wrkrMax;
	int bProcessOnPoller;
	sbool configSetViaV2Method;
};

static modConfData_t *loadModConf = NULL;/* modConf ptr to use for the current load process */
static modConfData_t *runModConf = NULL;/* modConf ptr to use for the current load process */

/* module-global parameters */
static struct cnfparamdescr modpdescr[] = {
	{ "threads", eCmdHdlrPositiveInt, 0 },
	{ "processOnPoller", eCmdHdlrBinary, 0 }
};
static struct cnfparamblk modpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(modpdescr)/sizeof(struct cnfparamdescr),
	  modpdescr
	};

/* input instance parameters */
static struct cnfparamdescr inppdescr[] = {
	{ "port", eCmdHdlrString, 0 }, /* legacy: InputTCPServerRun */
	{ "address", eCmdHdlrString, 0 },
	{ "path", eCmdHdlrString, 0 },
	{ "unlink", eCmdHdlrBinary, 0 },
	{ "discardtruncatedmsg", eCmdHdlrBinary, 0 },
	{ "fileowner", eCmdHdlrUID, 0 },
	{ "fileownernum", eCmdHdlrInt, 0 },
	{ "filegroup", eCmdHdlrGID, 0 },
	{ "filegroupnum", eCmdHdlrInt, 0 },
	{ "filecreatemode", eCmdHdlrFileCreateMode, 0 },
	{ "failonchownfailure", eCmdHdlrBinary, 0 },
	{ "flowcontrol", eCmdHdlrBinary, 0 },
	{ "name", eCmdHdlrString, 0 },
	{ "maxframesize", eCmdHdlrInt, 0 },
	{ "framing.delimiter.regex", eCmdHdlrString, 0 },
	{ "ruleset", eCmdHdlrString, 0 },
	{ "defaulttz", eCmdHdlrString, 0 },
	{ "supportoctetcountedframing", eCmdHdlrBinary, 0 },
	{ "framingfix.cisco.asa", eCmdHdlrBinary, 0 },
	{ "notifyonconnectionclose", eCmdHdlrBinary, 0 },
	{ "notifyonconnectionopen", eCmdHdlrBinary, 0 },
	{ "compression.mode", eCmdHdlrGetWord, 0 },
	{ "keepalive", eCmdHdlrBinary, 0 },
	{ "keepalive.probes", eCmdHdlrInt, 0 },
	{ "keepalive.time", eCmdHdlrInt, 0 },
	{ "keepalive.interval", eCmdHdlrInt, 0 },
	{ "addtlframedelimiter", eCmdHdlrInt, 0 },
	{ "ratelimit.interval", eCmdHdlrInt, 0 },
	{ "ratelimit.burst", eCmdHdlrInt, 0 },
	{ "multiline", eCmdHdlrBinary, 0 },
	{ "listenportfilename", eCmdHdlrString, 0 },
	{ "socketbacklog", eCmdHdlrInt, 0 }
};
static struct cnfparamblk inppblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(inppdescr)/sizeof(struct cnfparamdescr),
	  inppdescr
	};

#include "im-helper.h" /* must be included AFTER the type definitions! */
static int bLegacyCnfModGlobalsPermitted;/* are legacy module-global config parameters permitted? */

/* data elements describing our running config */
typedef struct ptcpsrv_s ptcpsrv_t;
typedef struct ptcplstn_s ptcplstn_t;
typedef struct ptcpsess_s ptcpsess_t;
typedef struct epolld_s epolld_t;

/* the ptcp server (listener) object
 * Note that the object contains support for forming a linked list
 * of them. It does not make sense to do this seperately.
 */
struct ptcpsrv_s {
	ptcpsrv_t *pNext;		/* linked list maintenance */
	uchar *port;			/* Port to listen to */
	uchar *lstnIP;			/* which IP we should listen on? */
	uchar *path;            /* Use a unix socket instead */
	int	fCreateMode;	/* file creation mode for open() */
	uid_t fileUID;	/* IDs for creation */
	gid_t fileGID;
	int maxFrameSize;
	int	bFailOnPerms;	/* fail creation if chown fails? */
	sbool bUnixSocket;
	int socketBacklog;
	uchar *pszLstnPortFileName;
	int iAddtlFrameDelim;
	sbool multiLine;
	int iKeepAliveIntvl;
	int iKeepAliveProbes;
	int iKeepAliveTime;
	uint8_t compressionMode;
	uchar *pszInputName;
	uchar *dfltTZ;
	prop_t *pInputName;		/* InputName in (fast to process) property format */
	ruleset_t *pRuleset;
	ptcplstn_t *pLstn;		/* root of our listeners */
	ptcpsess_t *pSess;		/* root of our sessions */
	pthread_mutex_t mutSessLst;
	sbool bKeepAlive;		/* support keep-alive packets */
	sbool bEmitMsgOnClose;
	sbool bEmitMsgOnOpen;
	sbool bSuppOctetFram;
	sbool bSPFramingFix;
	sbool bUnlink;
	sbool discardTruncatedMsg;
	sbool flowControl;
	ratelimit_t *ratelimiter;
	instanceConf_t *inst;
};

/* the ptcp session object. Describes a single active session.
 * includes support for doubly-linked list.
 */
struct ptcpsess_s {
	ptcplstn_t *pLstn;	/* our listener */
	ptcpsess_t *prev, *next;
	int sock;
	epolld_t *epd;
	sbool bzInitDone; /* did we do an init of zstrm already? */
	z_stream zstrm;	/* zip stream to use for tcp compression */
	uint8_t compressionMode;
	int iMsg;		 /* index of next char to store in msg */
	int iCurrLine;		 /* 2nd char of current line in regex framing mode */
	int bAtStrtOfFram;	/* are we at the very beginning of a new frame? */
	sbool bSuppOctetFram;	/**< copy from listener, to speed up access */
	sbool bSPFramingFix;
	enum {
		eAtStrtFram,
		eInOctetCnt,
		eInMsg,
		eInMsgTruncation
	} inputState;		/* our current state */
	int iOctetsRemain;	/* Number of Octets remaining in message */
	TCPFRAMINGMODE eFraming;
	uchar *pMsg;		/* message (fragment) received */
	uchar *pMsg_save;	/* message (fragment) save area in regex framing mode */
	prop_t *peerName;	/* host name we received messages from */
	prop_t *peerIP;
};


/* the ptcp listener object. Describes a single active listener.
 */
struct ptcplstn_s {
	ptcpsrv_t *pSrv;	/* our server */
	ptcplstn_t *prev, *next;
	int sock;
	sbool bSuppOctetFram;
	sbool bSPFramingFix;
	epolld_t *epd;
	statsobj_t *stats;	/* listener stats */
	intctr_t rcvdBytes;
	intctr_t rcvdDecompressed;
	STATSCOUNTER_DEF(ctrSubmit, mutCtrSubmit)
	STATSCOUNTER_DEF(ctrSessOpen, mutCtrSessOpen)
	STATSCOUNTER_DEF(ctrSessOpenErr, mutCtrSessOpenErr)
	STATSCOUNTER_DEF(ctrSessClose, mutCtrSessClose)
};


/* The following structure controls the worker threads. Global data is
 * needed for their access.
 */
static struct wrkrInfo_s {
	pthread_t tid;	/* the worker's thread ID */
	long long unsigned numCalled;	/* how often was this called */
} *wrkrInfo;
static int wrkrRunning;


/* type of object stored in epoll descriptor */
typedef enum {
	epolld_lstn,
	epolld_sess
} epolld_type_t;

/* an epoll descriptor. contains all information necessary to process
 * the result of epoll.
 */
struct epolld_s {
	epolld_type_t typ;
	void *ptr;
	int sock;
	struct epoll_event ev;
};

typedef struct io_req_s {
	STAILQ_ENTRY(io_req_s) link;
	epolld_t *epd;
} io_req_t;

typedef struct io_q_s {
	STAILQ_HEAD(ioq_s, io_req_s) q;
	STATSCOUNTER_DEF(ctrEnq, mutCtrEnq);
	int ctrMaxSz; //TODO: discuss potential problems around concurrent reads and writes
	int sz; //current q size
	statsobj_t *stats;
	pthread_mutex_t mut;
	pthread_cond_t wakeup_worker;
} io_q_t;

/* global data */
pthread_attr_t wrkrThrdAttr;	/* Attribute for session threads; read only after startup */
static ptcpsrv_t *pSrvRoot = NULL;
static int epollfd = -1;			/* (sole) descriptor for epoll */
static int iMaxLine; /* maximum size of a single message */
static io_q_t io_q;

/* forward definitions */
static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal);
static rsRetVal addLstn(ptcpsrv_t *pSrv, int sock, int isIPv6);


/* some simple constructors/destructors */
static void
destructSess(ptcpsess_t *pSess)
{
	free(pSess->pMsg_save);
	free(pSess->pMsg);
	free(pSess->epd);
	prop.Destruct(&pSess->peerName);
	prop.Destruct(&pSess->peerIP);
	/* TODO: make these inits compile-time switch depending: */
	pSess->pMsg = NULL;
	pSess->epd = NULL;
	free(pSess);
}

static void
destructSrv(ptcpsrv_t *pSrv)
{
	if(pSrv->ratelimiter != NULL)
		ratelimitDestruct(pSrv->ratelimiter);
	if(pSrv->pInputName != NULL)
		prop.Destruct(&pSrv->pInputName);
	pthread_mutex_destroy(&pSrv->mutSessLst);
	if(pSrv->pszInputName != NULL)
		free(pSrv->pszInputName);
	if(pSrv->port != NULL)
		free(pSrv->port);
	if(pSrv->pszLstnPortFileName)
		free(pSrv->pszLstnPortFileName);
	if(pSrv->path != NULL)
		free(pSrv->path);
	if(pSrv->lstnIP != NULL)
		free(pSrv->lstnIP);
	free(pSrv);
}

/****************************************** TCP SUPPORT FUNCTIONS ***********************************/
/* We may later think about moving this into a helper library again. But the whole point
 * so far was to keep everything related close togehter. -- rgerhards, 2010-08-10
 */

static rsRetVal startupUXSrv(ptcpsrv_t *pSrv) {
	DEFiRet;
	int sock;
	int sockflags;
	struct sockaddr_un local;

	uchar *path = pSrv->path == NULL ? UCHAR_CONSTANT("") : pSrv->path;
	DBGPRINTF("imptcp: creating listen unix socket at %s\n", path);

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if(sock < 0) {
		LogError(errno, RS_RET_ERR_CRE_AFUX, "imptcp: error creating unix socket");
		ABORT_FINALIZE(RS_RET_ERR_CRE_AFUX);
	}

	local.sun_family = AF_UNIX;
	strncpy(local.sun_path, (char*) path, sizeof(local.sun_path));
	if (pSrv->bUnlink) {
		unlink(local.sun_path);
	}

	/* We use non-blocking IO! */
	if ((sockflags = fcntl(sock, F_GETFL)) != -1) {
		sockflags |= O_NONBLOCK;
		/* SETFL could fail too, so get it caught by the subsequent error check. */
		sockflags = fcntl(sock, F_SETFL, sockflags);
	}

	if (sockflags == -1) {
		LogError(errno, RS_RET_ERR_CRE_AFUX, "imptcp: error setting fcntl(O_NONBLOCK) on unix socket");
		ABORT_FINALIZE(RS_RET_ERR_CRE_AFUX);
	}

	if (bind(sock, (struct sockaddr *)&local, SUN_LEN(&local)) < 0) {
		LogError(errno, RS_RET_ERR_CRE_AFUX, "imptcp: error while binding unix socket");
		ABORT_FINALIZE(RS_RET_ERR_CRE_AFUX);
	}

//	dbgprintf("pascal: listenportfilename = %s\n", pSrv->pszLstnPortFileName);

	if (listen(sock, pSrv->socketBacklog) < 0) {
		LogError(errno, RS_RET_ERR_CRE_AFUX, "imptcp: unix socket listen error");
		ABORT_FINALIZE(RS_RET_ERR_CRE_AFUX);
	}

	if(chown(local.sun_path, pSrv->fileUID, pSrv->fileGID) != 0) {
		if(pSrv->bFailOnPerms) {
			LogError(errno, RS_RET_ERR_CRE_AFUX, "imptcp: unix socket chown error");
			ABORT_FINALIZE(RS_RET_ERR_CRE_AFUX);
		}
	}

	if(chmod(local.sun_path, pSrv->fCreateMode) != 0) {
		if(pSrv->bFailOnPerms) {
			LogError(errno, RS_RET_ERR_CRE_AFUX, "imptcp: unix socket chmod error");
			ABORT_FINALIZE(RS_RET_ERR_CRE_AFUX);
		}
	}

	CHKiRet(addLstn(pSrv, sock, 0));

finalize_it:
	if (iRet != RS_RET_OK) {
		if (sock != -1) {
			close(sock);
		}
	}

	RETiRet;
}

/* Start up a server. That means all of its listeners are created.
 * Does NOT yet accept/process any incoming data (but binds ports). Hint: this
 * code is to be executed before dropping privileges.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
static rsRetVal
startupSrv(ptcpsrv_t *pSrv)
{
	DEFiRet;
	int error, maxs, on = 1;
	int sock = -1;
	int numSocks;
	int sockflags;
	struct addrinfo hints, *res = NULL, *r;
	uchar *lstnIP;
	int isIPv6 = 0;

	if (pSrv->bUnixSocket) {
		return startupUXSrv(pSrv);
	}

	lstnIP = pSrv->lstnIP == NULL ? UCHAR_CONSTANT("") : pSrv->lstnIP;

	DBGPRINTF("imptcp: creating listen socket on server '%s', port %s\n", lstnIP, pSrv->port);

	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = glbl.GetDefPFFamily();
	hints.ai_socktype = SOCK_STREAM;

	error = getaddrinfo((char*)pSrv->lstnIP, (char*) pSrv->port, &hints, &res);
	if(error) {
		DBGPRINTF("error %d querying server '%s', port '%s'\n", error, pSrv->lstnIP, pSrv->port);
		ABORT_FINALIZE(RS_RET_INVALID_PORT);
	}

	/* Count max number of sockets we may open */
	for(maxs = 0, r = res; r != NULL ; r = r->ai_next, maxs++) {
		/* EMPTY */;
	}

	numSocks = 0;   /* num of sockets counter at start of array */
	for(r = res; r != NULL ; r = r->ai_next) {
		sock = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
		if(sock < 0) {
			if(!(r->ai_family == PF_INET6 && errno == EAFNOSUPPORT)) {
				DBGPRINTF("error %d creating tcp listen socket", errno);
				/* it is debatable if PF_INET with EAFNOSUPPORT should
				 * also be ignored...
				 */
			}
			continue;
		}

		if(r->ai_family == AF_INET6) {
			isIPv6 = 1;
#ifdef IPV6_V6ONLY
			int iOn = 1;
			if(setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&iOn, sizeof (iOn)) < 0) {
				close(sock);
				sock = -1;
				continue;
			}
#endif
		} else {
			isIPv6 = 0;
		}

		if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on)) < 0 ) {
			DBGPRINTF("error %d setting tcp socket option\n", errno);
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
			DBGPRINTF("error %d setting fcntl(O_NONBLOCK) on tcp socket", errno);
			close(sock);
			sock = -1;
			continue;
		}



		/* We need to enable BSD compatibility. Otherwise an attacker
		 * could flood our log files by sending us tons of ICMP errors.
		 */
#if !defined (_AIX)
#ifndef BSD
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

		if(pSrv->pszLstnPortFileName) {
			FILE *fp;
			if(getsockname(sock, r->ai_addr, &r->ai_addrlen) == -1) {
				LogError(errno, NO_ERRCODE, "imptcp: ListenPortFileName: getsockname:"
						"error while trying to get socket");
			}
			if((fp = fopen((const char*)pSrv->pszLstnPortFileName, "w+")) == NULL) {
				LogError(errno, RS_RET_IO_ERROR, "imptcp: ListenPortFileName: "
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

		if(listen(sock, pSrv->socketBacklog) < 0) {
			DBGPRINTF("tcp listen error %d, suspending\n", errno);
			close(sock);
			sock = -1;
			continue;
		}

		/* if we reach this point, we were able to obtain a valid socket, so we can
		 * create our listener object. -- rgerhards, 2010-08-10
		 */
		CHKiRet(addLstn(pSrv, sock, isIPv6));
		++numSocks;
	}

	if(numSocks != maxs) {
		DBGPRINTF("We could initialize %d TCP listen sockets out of %d we received "
		 	  "- this may or may not be an error indication.\n", numSocks, maxs);
	}

	if(numSocks == 0) {
		DBGPRINTF("No TCP listen sockets could successfully be initialized");
		ABORT_FINALIZE(RS_RET_COULD_NOT_BIND);
	}

finalize_it:
	if(res != NULL) {
		freeaddrinfo(res);
	}

	if(iRet != RS_RET_OK) {
		if(sock != -1) {
			close(sock);
		}
	}

	RETiRet;
}
#pragma GCC diagnostic pop

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
getPeerNames(prop_t **peerName, prop_t **peerIP, struct sockaddr *pAddr, sbool bUXServer)
{
	int error;
	uchar szIP[NI_MAXHOST+1] = "";
	uchar szHname[NI_MAXHOST+1] = "";
	struct addrinfo hints, *res;
	sbool bMaliciousHName = 0;
	
	DEFiRet;

	*peerName = NULL;
	*peerIP = NULL;

	if (bUXServer) {
		strncpy((char *) szHname, (char *) glbl.GetLocalHostName(), NI_MAXHOST);
		strncpy((char *) szIP, (char *) glbl.GetLocalHostIP(), NI_MAXHOST);
		szHname[NI_MAXHOST] = '\0';
		szIP[NI_MAXHOST] = '\0';
	} else {
		error = getnameinfo(pAddr, SALEN(pAddr), (char *) szIP, sizeof(szIP), NULL, 0, NI_NUMERICHOST);
		if (error) {
			DBGPRINTF("Malformed from address %s\n", gai_strerror(error));
			strcpy((char *) szHname, "???");
			strcpy((char *) szIP, "???");
			ABORT_FINALIZE(RS_RET_INVALID_HNAME);
		}

		if (!glbl.GetDisableDNS()) {
			error = getnameinfo(pAddr, SALEN(pAddr), (char *) szHname, NI_MAXHOST, NULL, 0, NI_NAMEREQD);
			if (error == 0) {
				memset(&hints, 0, sizeof(struct addrinfo));
				hints.ai_flags = AI_NUMERICHOST;
				hints.ai_socktype = SOCK_STREAM;
				/* we now do a lookup once again. This one should fail,
				 * because we should not have obtained a non-numeric address. If
				 * we got a numeric one, someone messed with DNS!
				 */
				if (getaddrinfo((char *) szHname, NULL, &hints, &res) == 0) {
					freeaddrinfo(res);
					/* OK, we know we have evil, so let's indicate this to our caller */
					snprintf((char *) szHname, NI_MAXHOST, "[MALICIOUS:IP=%s]", szIP);
					DBGPRINTF("Malicious PTR record, IP = \"%s\" HOST = \"%s\"", szIP, szHname);
					bMaliciousHName = 1;
				}
			} else {
				strcpy((char *) szHname, (char *) szIP);
			}
		} else {
			strcpy((char *) szHname, (char *) szIP);
		}
	}

	/* We now have the names, so now let's allocate memory and store them permanently. */
	CHKiRet(prop.Construct(peerName));
	CHKiRet(prop.SetString(*peerName, szHname, ustrlen(szHname)));
	CHKiRet(prop.ConstructFinalize(*peerName));
	CHKiRet(prop.Construct(peerIP));
	CHKiRet(prop.SetString(*peerIP, szIP, ustrlen(szIP)));
	CHKiRet(prop.ConstructFinalize(*peerIP));

finalize_it:
	if(iRet != RS_RET_OK) {
		if(*peerName != NULL)
			prop.Destruct(peerName);
		if(*peerIP != NULL)
			prop.Destruct(peerIP);
	}
	if(bMaliciousHName)
		iRet = RS_RET_MALICIOUS_HNAME;
	RETiRet;
}


/* Enable KEEPALIVE handling on the socket.  */
static rsRetVal
EnableKeepAlive(ptcplstn_t *pLstn, int sock)
{
	int ret;
	int optval;
	socklen_t optlen;
	DEFiRet;

	optval = 1;
	optlen = sizeof(optval);
	ret = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen);
	if(ret < 0) {
		dbgprintf("EnableKeepAlive socket call returns error %d\n", ret);
		ABORT_FINALIZE(RS_RET_ERR);
	}

#	if defined(TCP_KEEPCNT)
	if(pLstn->pSrv->iKeepAliveProbes > 0) {
		optval = pLstn->pSrv->iKeepAliveProbes;
		optlen = sizeof(optval);
		ret = setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &optval, optlen);
	} else {
		ret = 0;
	}
#	else
	ret = -1;
#	endif
	if(ret < 0) {
		LogError(ret, NO_ERRCODE, "imptcp cannot set keepalive probes - ignored");
	}

#	if defined(TCP_KEEPCNT)
	if(pLstn->pSrv->iKeepAliveTime > 0) {
		optval = pLstn->pSrv->iKeepAliveTime;
		optlen = sizeof(optval);
		ret = setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &optval, optlen);
	} else {
		ret = 0;
	}
#	else
	ret = -1;
#	endif
	if(ret < 0) {
		LogError(ret, NO_ERRCODE, "imptcp cannot set keepalive time - ignored");
	}

#	if defined(TCP_KEEPCNT)
	if(pLstn->pSrv->iKeepAliveIntvl > 0) {
		optval = pLstn->pSrv->iKeepAliveIntvl;
		optlen = sizeof(optval);
		ret = setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &optval, optlen);
	} else {
		ret = 0;
	}
#	else
	ret = -1;
#	endif
	if(ret < 0) {
		LogError(errno, NO_ERRCODE, "imptcp cannot set keepalive intvl - ignored");
	}

	dbgprintf("KEEPALIVE enabled for socket %d\n", sock);

finalize_it:
	RETiRet;
}


/* accept an incoming connection request
 * rgerhards, 2008-04-22
 */
static rsRetVal ATTR_NONNULL()
AcceptConnReq(ptcplstn_t *const pLstn, int *const newSock, prop_t **peerName, prop_t **peerIP)
{
	int sockflags;
	struct sockaddr_storage addr;
	socklen_t addrlen = sizeof(addr);
	int iNewSock = -1;

	DEFiRet;

	*peerName = NULL; /* ensure we know if we don't have one! */
	iNewSock = accept(pLstn->sock, (struct sockaddr*) &addr, &addrlen);
	if(iNewSock < 0) {
		if(CHK_EAGAIN_EWOULDBLOCK || errno == EMFILE)
			ABORT_FINALIZE(RS_RET_NO_MORE_DATA);
		LogError(errno, RS_RET_ACCEPT_ERR, "error accepting connection "
			    "on listen socket %d", pLstn->sock);
		ABORT_FINALIZE(RS_RET_ACCEPT_ERR);
	}
	if(addrlen == 0) {
		LogError(errno, RS_RET_ACCEPT_ERR, "AcceptConnReq could not obtain "
			    "remote peer identification on listen socket %d", pLstn->sock);
	}

	if(pLstn->pSrv->bKeepAlive)
		EnableKeepAlive(pLstn, iNewSock);/* we ignore errors, best to do! */

	CHKiRet(getPeerNames(peerName, peerIP, (struct sockaddr *) &addr, pLstn->pSrv->bUnixSocket));

	/* set the new socket to non-blocking IO */
	if((sockflags = fcntl(iNewSock, F_GETFL)) != -1) {
		sockflags |= O_NONBLOCK;
		/* SETFL could fail too, so get it caught by the subsequent
		 * error check.
		 */
		sockflags = fcntl(iNewSock, F_SETFL, sockflags);
	}

	if(sockflags == -1) {
		LogError(errno, RS_RET_IO_ERROR, "error setting fcntl(O_NONBLOCK) on "
			"tcp socket %d", iNewSock);
		prop.Destruct(peerName);
		prop.Destruct(peerIP);
		ABORT_FINALIZE(RS_RET_IO_ERROR);
	}
	if(pLstn->pSrv->bEmitMsgOnOpen) {
		LogMsg(0, RS_RET_NO_ERRCODE, LOG_INFO,
			"imptcp: connection established with host: %s",
			propGetSzStr(*peerName));
	}

	STATSCOUNTER_INC(pLstn->ctrSessOpen, pLstn->mutCtrSessOpen);
	*newSock = iNewSock;

finalize_it:
	DBGPRINTF("iRet: %d\n", iRet);
	if(iRet != RS_RET_OK) {
		if(iRet != RS_RET_NO_MORE_DATA && pLstn->pSrv->bEmitMsgOnOpen) {
			LogError(0, NO_ERRCODE, "imptcp: connection could not be "
				"established with host: %s",
				*peerName == NULL ? "(could not query)"
					: (const char*)propGetSzStr(*peerName));
		}
		STATSCOUNTER_INC(pLstn->ctrSessOpenErr, pLstn->mutCtrSessOpenErr);
		/* the close may be redundant, but that doesn't hurt... */
		if(iNewSock != -1)
			close(iNewSock);
	}

	RETiRet;
}


/* This is a helper for submitting the message to the rsyslog core.
 * It does some common processing, including resetting the various
 * state variables to a "processed" state.
 * Note that this function is also called if we had a buffer overflow
 * due to a too-long message. So far, there is no indication this
 * happened and it may be worth thinking about different handling
 * of this case (what obviously would require a change to this
 * function or some related code).
 * rgerhards, 2009-04-23
 * EXTRACT from tcps_sess.c
 */
static rsRetVal
doSubmitMsg(ptcpsess_t *pThis, struct syslogTime *stTime, time_t ttGenTime, multi_submit_t *pMultiSub)
{
	smsg_t *pMsg;
	ptcpsrv_t *pSrv;
	DEFiRet;

	if(pThis->iMsg == 0) {
		DBGPRINTF("discarding zero-sized message\n");
		FINALIZE;
	}
	pSrv = pThis->pLstn->pSrv;

	/* we now create our own message object and submit it to the queue */
	CHKiRet(msgConstructWithTime(&pMsg, stTime, ttGenTime));
	MsgSetRawMsg(pMsg, (char*)pThis->pMsg, pThis->iMsg);
	MsgSetInputName(pMsg, pSrv->pInputName);
	MsgSetFlowControlType(pMsg, eFLOWCTL_LIGHT_DELAY);
	if(pSrv->dfltTZ != NULL)
		MsgSetDfltTZ(pMsg, (char*) pSrv->dfltTZ);
	MsgSetFlowControlType(pMsg, pSrv->flowControl
			            ? eFLOWCTL_LIGHT_DELAY : eFLOWCTL_NO_DELAY);
	pMsg->msgFlags  = NEEDS_PARSING | PARSE_HOSTNAME;
	MsgSetRcvFrom(pMsg, pThis->peerName);
	CHKiRet(MsgSetRcvFromIP(pMsg, pThis->peerIP));
	MsgSetRuleset(pMsg, pSrv->pRuleset);
	STATSCOUNTER_INC(pThis->pLstn->ctrSubmit, pThis->pLstn->mutCtrSubmit);

	ratelimitAddMsg(pSrv->ratelimiter, pMultiSub, pMsg);

finalize_it:
	/* reset status variables */
	pThis->bAtStrtOfFram = 1;
	pThis->iMsg = 0;

	RETiRet;
}


/* process the data received, special case if the framing is specified via
 * a regex. For more info see processDataRcvd().
 */
static rsRetVal ATTR_NONNULL()
processDataRcvd_regexFraming(ptcpsess_t *const __restrict__ pThis,
	char **const buff,
	struct syslogTime *const stTime,
	const time_t ttGenTime,
	multi_submit_t *const pMultiSub,
	unsigned *const __restrict__ pnMsgs)
{
	DEFiRet;
	const instanceConf_t *const inst = pThis->pLstn->pSrv->inst;
	assert(inst->startRegex != NULL);
	const char c = **buff;

	pThis->pMsg[pThis->iMsg++] = c;
	pThis->pMsg[pThis->iMsg] = '\0';

	if(pThis->iMsg == 2*iMaxLine) {
		LogError(0, RS_RET_OVERSIZE_MSG, "imptcp: more then double max message size (%d) "
			"received without finding frame terminator via regex - assuming "
			"end of frame now.", pThis->iMsg+1);
		doSubmitMsg(pThis, stTime, ttGenTime, pMultiSub);
		++(*pnMsgs);
		pThis->iMsg = 0;
		pThis->iCurrLine = 1;
	}


	if(c == '\n') {
		pThis->iCurrLine = pThis->iMsg;
	} else {
		const int isMatch = !regexec(&inst->start_preg, (char*)pThis->pMsg+pThis->iCurrLine, 0, NULL, 0);
		if(isMatch) {
			DBGPRINTF("regex match (%d), framing line: %s\n", pThis->iCurrLine, pThis->pMsg);
			strcpy((char*)pThis->pMsg_save, (char*) pThis->pMsg+pThis->iCurrLine);
			pThis->iMsg = pThis->iCurrLine - 1;

			doSubmitMsg(pThis, stTime, ttGenTime, pMultiSub);
			++(*pnMsgs);

			strcpy((char*)pThis->pMsg, (char*)pThis->pMsg_save);
			pThis->iMsg = ustrlen(pThis->pMsg_save);
			pThis->iCurrLine = 1;
		}
	}

	RETiRet;
}


/* process the data received. As TCP is stream based, we need to process the
 * data inside a state machine. The actual data received is passed in byte-by-byte
 * from DataRcvd, and this function here compiles messages from them and submits
 * the end result to the queue. Introducing this function fixes a long-term bug ;)
 * rgerhards, 2008-03-14
 * EXTRACT from tcps_sess.c
 */
static rsRetVal
processDataRcvd(ptcpsess_t *const __restrict__ pThis,
	char **buff,
	const int buffLen,
	struct syslogTime *stTime,
	const time_t ttGenTime,
	multi_submit_t *pMultiSub,
	unsigned *const __restrict__ pnMsgs)
{
	DEFiRet;
	char c = **buff;
	int octatesToCopy, octatesToDiscard;
	uchar *propPeerName = NULL;
	int lenPeerName = 0;
	uchar *propPeerIP = NULL;
	int lenPeerIP = 0;

	if(pThis->pLstn->pSrv->inst->startRegex != NULL) {
		processDataRcvd_regexFraming(pThis, buff, stTime, ttGenTime, pMultiSub, pnMsgs);
		FINALIZE;
	}

	if(pThis->inputState == eAtStrtFram) {
		if(pThis->bSuppOctetFram && isdigit((int) c)) {
			pThis->inputState = eInOctetCnt;
			pThis->iOctetsRemain = 0;
			pThis->eFraming = TCP_FRAMING_OCTET_COUNTING;
		} else if(pThis->bSPFramingFix && c == ' ') {
			/* Cisco very occasionally sends a SP after a LF, which
			 * thrashes framing if not taken special care of. Here,
			 * we permit space *in front of the next frame* and
			 * ignore it.
			 */
			 FINALIZE;
		} else {
			pThis->inputState = eInMsg;
			pThis->eFraming = TCP_FRAMING_OCTET_STUFFING;
		}
	}

	if(pThis->inputState == eInOctetCnt) {
		if(isdigit(c)) {
			if(pThis->iOctetsRemain <= 200000000) {
				pThis->iOctetsRemain = pThis->iOctetsRemain * 10 + c - '0';
			}
			*(pThis->pMsg + pThis->iMsg++) = c;
		} else { /* done with the octet count, so this must be the SP terminator */
			DBGPRINTF("TCP Message with octet-counter, size %d.\n", pThis->iOctetsRemain);
			prop.GetString(pThis->peerName, &propPeerName, &lenPeerName);
			prop.GetString(pThis->peerIP, &propPeerIP, &lenPeerIP);
			if(c != ' ') {
				LogError(0, NO_ERRCODE, "Framing Error in received TCP message "
						"from peer: (hostname) %s, (ip) %s: delimiter is not "
						"SP but has ASCII value %d.", propPeerName, propPeerIP, c);
			}
			if(pThis->iOctetsRemain < 1) {
				/* TODO: handle the case where the octet count is 0! */
				LogError(0, NO_ERRCODE, "Framing Error in received TCP message"
						" from peer: (hostname) %s, (ip) %s: invalid octet count %d.",
						propPeerName, propPeerIP, pThis->iOctetsRemain);
				pThis->eFraming = TCP_FRAMING_OCTET_STUFFING;
			} else if(pThis->iOctetsRemain > iMaxLine) {
				/* while we can not do anything against it, we can at least log an indication
				 * that something went wrong) -- rgerhards, 2008-03-14
				 */
				DBGPRINTF("truncating message with %d octets - max msg size is %d\n",
					  pThis->iOctetsRemain, iMaxLine);
				LogError(0, NO_ERRCODE, "received oversize message from peer: "
						"(hostname) %s, (ip) %s: size is %d bytes, max msg "
						"size is %d, truncating...", propPeerName, propPeerIP,
						pThis->iOctetsRemain, iMaxLine);
			}
			if(pThis->iOctetsRemain > pThis->pLstn->pSrv->maxFrameSize) {
				LogError(0, NO_ERRCODE, "Framing Error in received TCP message "
						"from peer: (hostname) %s, (ip) %s: frame too large: %d, "
						"change to octet stuffing", propPeerName, propPeerIP,
						pThis->iOctetsRemain);
				pThis->eFraming = TCP_FRAMING_OCTET_STUFFING;
			} else {
				pThis->iMsg = 0;
			}
			pThis->inputState = eInMsg;
		}
	} else if(pThis->inputState == eInMsgTruncation) {
		if ((c == '\n')
		|| ((pThis->pLstn->pSrv->iAddtlFrameDelim != TCPSRV_NO_ADDTL_DELIMITER)
		&& (c == pThis->pLstn->pSrv->iAddtlFrameDelim))) {
			pThis->inputState = eAtStrtFram;
		}
	} else {
		assert(pThis->inputState == eInMsg);

		if (pThis->eFraming == TCP_FRAMING_OCTET_STUFFING) {
			if(pThis->iMsg >= iMaxLine) {
				/* emergency, we now need to flush, no matter if we are at end of message or not... */
				int i = 1;
				char currBuffChar;
				while(i < buffLen && ((currBuffChar = (*buff)[i]) != '\n'
					&& (pThis->pLstn->pSrv->iAddtlFrameDelim == TCPSRV_NO_ADDTL_DELIMITER
						|| currBuffChar != pThis->pLstn->pSrv->iAddtlFrameDelim))) {
					i++;
				}
				LogError(0, NO_ERRCODE, "imptcp %s: message received is at least %d byte larger than "
					"max msg size; message will be split starting at: \"%.*s\"\n",
					pThis->pLstn->pSrv->pszInputName, i, (i < 32) ? i : 32, *buff);
				doSubmitMsg(pThis, stTime, ttGenTime, pMultiSub);
				++(*pnMsgs);
				if(pThis->pLstn->pSrv->discardTruncatedMsg == 1) {
					pThis->inputState = eInMsgTruncation;
				}
				/* we might think if it is better to ignore the rest of the
				 * message than to treat it as a new one. Maybe this is a good
				 * candidate for a configuration parameter...
				 * rgerhards, 2006-12-04
				 */
			}
			if ((c == '\n')
				   || ((pThis->pLstn->pSrv->iAddtlFrameDelim != TCPSRV_NO_ADDTL_DELIMITER)
					   && (c == pThis->pLstn->pSrv->iAddtlFrameDelim))
				   ) { /* record delimiter? */
				if(pThis->pLstn->pSrv->multiLine) {
					if((buffLen == 1) || ((*buff)[1] == '<')) {
						doSubmitMsg(pThis, stTime, ttGenTime, pMultiSub);
						++(*pnMsgs);
						pThis->inputState = eAtStrtFram;
					} else {
						if(pThis->iMsg < iMaxLine) {
							*(pThis->pMsg + pThis->iMsg++) = c;
						}
					}
				} else {
					doSubmitMsg(pThis, stTime, ttGenTime, pMultiSub);
					++(*pnMsgs);
					pThis->inputState = eAtStrtFram;
				}
			} else {
				/* IMPORTANT: here we copy the actual frame content to the message - for BOTH
				 * framing modes! If we have a message that is larger than the max msg size,
				 * we truncate it. This is the best we can do in light of what the engine supports.
				 * -- rgerhards, 2008-03-14
				 */
				if(pThis->iMsg < iMaxLine) {
					*(pThis->pMsg + pThis->iMsg++) = c;
				}
			}
		} else {
			assert(pThis->eFraming == TCP_FRAMING_OCTET_COUNTING);
			octatesToCopy = pThis->iOctetsRemain;
			octatesToDiscard = 0;
			if (buffLen < octatesToCopy) {
				octatesToCopy = buffLen;
			}
			if (octatesToCopy + pThis->iMsg > iMaxLine) {
				octatesToDiscard = octatesToCopy - (iMaxLine - pThis->iMsg);
				octatesToCopy = iMaxLine - pThis->iMsg;
			}

			memcpy(pThis->pMsg + pThis->iMsg, *buff, octatesToCopy);
			pThis->iMsg += octatesToCopy;
			pThis->iOctetsRemain -= (octatesToCopy + octatesToDiscard);
			*buff += (octatesToCopy + octatesToDiscard - 1);
			if (pThis->iOctetsRemain == 0) {
				/* we have end of frame! */
				doSubmitMsg(pThis, stTime, ttGenTime, pMultiSub);
				++(*pnMsgs);
				pThis->inputState = eAtStrtFram;
			}
		}

	}

finalize_it:
	RETiRet;
}


/* Processes the data received via a TCP session. If there
 * is no other way to handle it, data is discarded.
 * Input parameter data is the data received, iLen is its
 * len as returned from recv(). iLen must be 1 or more (that
 * is errors must be handled by caller!). iTCPSess must be
 * the index of the TCP session that received the data.
 * rgerhards 2005-07-04
 * And another change while generalizing. We now return either
 * RS_RET_OK, which means the session should be kept open
 * or anything else, which means it must be closed.
 * rgerhards, 2008-03-01
 * As a performance optimization, we pick up the timestamp here. Acutally,
 * this *is* the *correct* reception step for all the data we received, because
 * we have just received a bunch of data! -- rgerhards, 2009-06-16
 * EXTRACT from tcps_sess.c
 */
static rsRetVal
DataRcvdUncompressed(ptcpsess_t *pThis, char *pData, size_t iLen, struct syslogTime *stTime, time_t ttGenTime)
{
	multi_submit_t multiSub;
	smsg_t *pMsgs[CONF_NUM_MULTISUB];
	char *pEnd;
	unsigned nMsgs = 0;
	DEFiRet;

	assert(pData != NULL);
	assert(iLen > 0);

	if(ttGenTime == 0)
		datetime.getCurrTime(stTime, &ttGenTime, TIME_IN_LOCALTIME);
	multiSub.ppMsgs = pMsgs;
	multiSub.maxElem = CONF_NUM_MULTISUB;
	multiSub.nElem = 0;

	 /* We now copy the message to the session buffer. */
	pEnd = pData + iLen; /* this is one off, which is intensional */

	while(pData < pEnd) {
		CHKiRet(processDataRcvd(pThis, &pData, pEnd - pData, stTime, ttGenTime, &multiSub, &nMsgs));
		pData++;
	}

	iRet = multiSubmitFlush(&multiSub);

	if(glblSenderKeepTrack)
		statsRecordSender(propGetSzStr(pThis->peerName), nMsgs, ttGenTime);

finalize_it:
	RETiRet;
}

static rsRetVal
DataRcvdCompressed(ptcpsess_t *pThis, char *buf, size_t len)
{
	struct syslogTime stTime;
	time_t ttGenTime;
	int zRet;	/* zlib return state */
	unsigned outavail;
	uchar zipBuf[64*1024]; // TODO: alloc on heap, and much larger (512KiB? batch size!)
	DEFiRet;
	// TODO: can we do stats counters? Even if they are not 100% correct under all cases,
	// by simply updating the input and output sizes?
	uint64_t outtotal;

	datetime.getCurrTime(&stTime, &ttGenTime, TIME_IN_LOCALTIME);
	outtotal = 0;

	if(!pThis->bzInitDone) {
		/* allocate deflate state */
		pThis->zstrm.zalloc = Z_NULL;
		pThis->zstrm.zfree = Z_NULL;
		pThis->zstrm.opaque = Z_NULL;
		zRet = inflateInit(&pThis->zstrm);
		if(zRet != Z_OK) {
			DBGPRINTF("imptcp: error %d returned from zlib/inflateInit()\n", zRet);
			ABORT_FINALIZE(RS_RET_ZLIB_ERR);
		}
		pThis->bzInitDone = RSTRUE;
	}

	pThis->zstrm.next_in = (Bytef*) buf;
	pThis->zstrm.avail_in = len;
	/* run inflate() on buffer until everything has been uncompressed */
	do {
		DBGPRINTF("imptcp: in inflate() loop, avail_in %d, total_in %ld\n",
			pThis->zstrm.avail_in, pThis->zstrm.total_in);
		pThis->zstrm.avail_out = sizeof(zipBuf);
		pThis->zstrm.next_out = zipBuf;
		zRet = inflate(&pThis->zstrm, Z_SYNC_FLUSH);    /* no bad return value */
		//zRet = inflate(&pThis->zstrm, Z_NO_FLUSH);    /* no bad return value */
		DBGPRINTF("after inflate, ret %d, avail_out %d\n", zRet, pThis->zstrm.avail_out);
		outavail = sizeof(zipBuf) - pThis->zstrm.avail_out;
		if(outavail != 0) {
			outtotal += outavail;
			pThis->pLstn->rcvdDecompressed += outavail;
			CHKiRet(DataRcvdUncompressed(pThis, (char*)zipBuf, outavail, &stTime, ttGenTime));
		}
	} while (pThis->zstrm.avail_out == 0);

	dbgprintf("end of DataRcvCompress, sizes: in %lld, out %llu\n", (long long) len,
		(long long unsigned) outtotal);
finalize_it:
	RETiRet;
}

static rsRetVal
DataRcvd(ptcpsess_t *pThis, char *pData, size_t iLen)
{
	struct syslogTime stTime;
	DEFiRet;
	pThis->pLstn->rcvdBytes += iLen;
	if(pThis->compressionMode >= COMPRESS_STREAM_ALWAYS)
		iRet =  DataRcvdCompressed(pThis, pData, iLen);
	else
		iRet =  DataRcvdUncompressed(pThis, pData, iLen, &stTime, 0);
	RETiRet;
}


/****************************************** --END-- TCP SUPPORT FUNCTIONS ***********************************/


static void
initConfigSettings(void)
{
	cs.bEmitMsgOnClose = 0;
	cs.bEmitMsgOnOpen = 0;
	cs.wrkrMax = DFLT_wrkrMax;
	cs.bSuppOctetFram = 1;
	cs.iAddtlFrameDelim = TCPSRV_NO_ADDTL_DELIMITER;
	cs.maxFrameSize = 200000;
	cs.pszInputName = NULL;
	cs.pszBindRuleset = NULL;
	cs.pszInputName = NULL;
	cs.lstnIP = NULL;
}


/* add socket to the epoll set
 */
static rsRetVal
addEPollSock(epolld_type_t typ, void *ptr, int sock, epolld_t **pEpd)
{
	DEFiRet;
	epolld_t *epd = NULL;

	CHKmalloc(epd = calloc(1, sizeof(epolld_t)));
	epd->typ = typ;
	epd->ptr = ptr;
	epd->sock = sock;
	*pEpd = epd;
	epd->ev.events = EPOLLIN|EPOLLET|EPOLLONESHOT;
	epd->ev.data.ptr = (void*) epd;

	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &(epd->ev)) != 0) {
		char errStr[1024];
		int eno = errno;
		LogError(0, RS_RET_EPOLL_CTL_FAILED, "os error (%d) during epoll ADD: %s",
			        eno, rs_strerror_r(eno, errStr, sizeof(errStr)));
		ABORT_FINALIZE(RS_RET_EPOLL_CTL_FAILED);
	}

	DBGPRINTF("imptcp: added socket %d to epoll[%d] set\n", sock, epollfd);

finalize_it:
	if(iRet != RS_RET_OK) {
		if (epd != NULL) {
			LogError(0, RS_RET_INTERNAL_ERROR, "error: could not initialize mutex for ptcp "
			"connection for socket: %d", sock);
		}
		free(epd);
	}
	RETiRet;
}


/* add a listener to the server
 */
static rsRetVal
addLstn(ptcpsrv_t *pSrv, int sock, int isIPv6)
{
	DEFiRet;
	ptcplstn_t *pLstn = NULL;
	uchar statname[64];

	CHKmalloc(pLstn = calloc(1, sizeof(ptcplstn_t)));
	pLstn->pSrv = pSrv;
	pLstn->bSuppOctetFram = pSrv->bSuppOctetFram;
	pLstn->bSPFramingFix = pSrv->bSPFramingFix;
	pLstn->sock = sock;
	/* support statistics gathering */
	uchar *inputname;
	if(pSrv->pszInputName == NULL) {
		inputname = (uchar*)"imptcp";
	} else {
		inputname = pSrv->pszInputName;
	}
	CHKiRet(statsobj.Construct(&(pLstn->stats)));
	snprintf((char*)statname, sizeof(statname), "%s(%s/%s/%s)", inputname,
		(pSrv->lstnIP == NULL) ? "*" : (char*)pSrv->lstnIP, pSrv->port,
		isIPv6 ? "IPv6" : "IPv4");
	statname[sizeof(statname)-1] = '\0'; /* just to be on the save side... */
	CHKiRet(statsobj.SetName(pLstn->stats, statname));
	CHKiRet(statsobj.SetOrigin(pLstn->stats, (uchar*)"imptcp"));
	STATSCOUNTER_INIT(pLstn->ctrSubmit, pLstn->mutCtrSubmit);
	CHKiRet(statsobj.AddCounter(pLstn->stats, UCHAR_CONSTANT("submitted"),
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &(pLstn->ctrSubmit)));
	STATSCOUNTER_INIT(pLstn->ctrSessOpen, pLstn->mutCtrSessOpen);
	CHKiRet(statsobj.AddCounter(pLstn->stats, UCHAR_CONSTANT("sessions.opened"),
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &(pLstn->ctrSessOpen)));
	STATSCOUNTER_INIT(pLstn->ctrSessOpenErr, pLstn->mutCtrSessOpenErr);
	CHKiRet(statsobj.AddCounter(pLstn->stats, UCHAR_CONSTANT("sessions.openfailed"),
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &(pLstn->ctrSessOpenErr)));
	STATSCOUNTER_INIT(pLstn->ctrSessClose, pLstn->mutCtrSessClose);
	CHKiRet(statsobj.AddCounter(pLstn->stats, UCHAR_CONSTANT("sessions.closed"),
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &(pLstn->ctrSessClose)));
	/* the following counters are not protected by mutexes; we accept
	 * that they may not be 100% correct */
	pLstn->rcvdBytes = 0,
	pLstn->rcvdDecompressed = 0;
	CHKiRet(statsobj.AddCounter(pLstn->stats, UCHAR_CONSTANT("bytes.received"),
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &(pLstn->rcvdBytes)));
	CHKiRet(statsobj.AddCounter(pLstn->stats, UCHAR_CONSTANT("bytes.decompressed"),
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &(pLstn->rcvdDecompressed)));
	CHKiRet(statsobj.ConstructFinalize(pLstn->stats));

	CHKiRet(addEPollSock(epolld_lstn, pLstn, sock, &pLstn->epd));

	/* add to start of server's listener list */
	pLstn->prev = NULL;
	pLstn->next = pSrv->pLstn;
	if(pSrv->pLstn != NULL)
		pSrv->pLstn->prev = pLstn;
	pSrv->pLstn = pLstn;

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pLstn != NULL) {
			if(pLstn->stats != NULL)
				statsobj.Destruct(&(pLstn->stats));
			free(pLstn);
		}
	}

	RETiRet;
}


/* add a session to the server
 */
static rsRetVal
addSess(ptcplstn_t *pLstn, int sock, prop_t *peerName, prop_t *peerIP)
{
	DEFiRet;
	ptcpsess_t *pSess = NULL;
	ptcpsrv_t *pSrv = pLstn->pSrv;
	int pmsg_size_factor;

	CHKmalloc(pSess = malloc(sizeof(ptcpsess_t)));
	if(pLstn->pSrv->inst->startRegex == NULL) {
		pmsg_size_factor = 1;
		pSess->pMsg_save = NULL;
	} else {
		pmsg_size_factor = 2;
		pSess->pMsg = NULL;
		CHKmalloc(pSess->pMsg_save = malloc(1 + iMaxLine * pmsg_size_factor));
	}
	CHKmalloc(pSess->pMsg = malloc(1 + iMaxLine * pmsg_size_factor));
	pSess->pLstn = pLstn;
	pSess->sock = sock;
	pSess->bSuppOctetFram = pLstn->bSuppOctetFram;
	pSess->bSPFramingFix = pLstn->bSPFramingFix;
	pSess->inputState = eAtStrtFram;
	pSess->iMsg = 0;
	pSess->iCurrLine = 1;
	pSess->bzInitDone = 0;
	pSess->bAtStrtOfFram = 1;
	pSess->peerName = peerName;
	pSess->peerIP = peerIP;
	pSess->compressionMode = pLstn->pSrv->compressionMode;

	/* add to start of server's listener list */
	pSess->prev = NULL;
	pthread_mutex_lock(&pSrv->mutSessLst);
	pSess->next = pSrv->pSess;
	if(pSrv->pSess != NULL)
		pSrv->pSess->prev = pSess;
	pSrv->pSess = pSess;
	pthread_mutex_unlock(&pSrv->mutSessLst);

	CHKiRet(addEPollSock(epolld_sess, pSess, sock, &pSess->epd));

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pSess != NULL) {
			free(pSess->pMsg_save);
			free(pSess->pMsg);
			free(pSess);
		}
	}

	RETiRet;
}


/* finish zlib buffer, to be called before closing the session.
 */
static rsRetVal
doZipFinish(ptcpsess_t *pSess)
{
	int zRet;	/* zlib return state */
	DEFiRet;
	unsigned outavail;
	struct syslogTime stTime;
	uchar zipBuf[32*1024]; // TODO: use "global" one from pSess

	if(!pSess->bzInitDone)
		goto done;

	pSess->zstrm.avail_in = 0;
	/* run inflate() on buffer until everything has been compressed */
	do {
		DBGPRINTF("doZipFinish: in inflate() loop, avail_in %d, total_in %ld\n", pSess->zstrm.avail_in,
		pSess->zstrm.total_in);
		pSess->zstrm.avail_out = sizeof(zipBuf);
		pSess->zstrm.next_out = zipBuf;
		zRet = inflate(&pSess->zstrm, Z_FINISH);    /* no bad return value */
		DBGPRINTF("after inflate, ret %d, avail_out %d\n", zRet, pSess->zstrm.avail_out);
		outavail = sizeof(zipBuf) - pSess->zstrm.avail_out;
		if(outavail != 0) {
			pSess->pLstn->rcvdDecompressed += outavail;
			CHKiRet(DataRcvdUncompressed(pSess, (char*)zipBuf, outavail, &stTime, 0));
		// TODO: query time!
		}
	} while (pSess->zstrm.avail_out == 0);

finalize_it:
	zRet = inflateEnd(&pSess->zstrm);
	if(zRet != Z_OK) {
		DBGPRINTF("imptcp: error %d returned from zlib/inflateEnd()\n", zRet);
	}

	pSess->bzInitDone = 0;
done:	RETiRet;
}

/* close/remove a session
 * NOTE: we do not need to remove the socket from the epoll set, as according
 * to the epoll man page it is automatically removed on close (Q6). The only
 * exception is duplicated file handles, which we do not create.
 */
static rsRetVal
closeSess(ptcpsess_t *pSess)
{
	DEFiRet;
	
	if(pSess->compressionMode >= COMPRESS_STREAM_ALWAYS)
		doZipFinish(pSess);

	const int sock = pSess->sock;
	close(sock);

	pthread_mutex_lock(&pSess->pLstn->pSrv->mutSessLst);
	/* finally unlink session from structures */
	if(pSess->next != NULL)
		pSess->next->prev = pSess->prev;
	if(pSess->prev == NULL) {
		/* need to update root! */
		pSess->pLstn->pSrv->pSess = pSess->next;
	} else {
		pSess->prev->next = pSess->next;
	}
	pthread_mutex_unlock(&pSess->pLstn->pSrv->mutSessLst);

	if(pSess->pLstn->pSrv->bEmitMsgOnClose) {
		LogMsg(0, RS_RET_NO_ERRCODE, LOG_INFO, "imptcp: session on socket %d closed "
						       "with iRet %d.\n", sock, iRet);
	}
	STATSCOUNTER_INC(pSess->pLstn->ctrSessClose, pSess->pLstn->mutCtrSessClose);

	/* unlinked, now remove structure */
	destructSess(pSess);

	DBGPRINTF("imptcp: session on socket %d closed with iRet %d.\n", sock, iRet);
	RETiRet;
}


/* create input instance, set default parameters, and
 * add it to the list of instances.
 */
static rsRetVal
createInstance(instanceConf_t **pinst)
{
	instanceConf_t *inst;
	DEFiRet;
	CHKmalloc(inst = MALLOC(sizeof(instanceConf_t)));
	inst->next = NULL;

	inst->pszBindPort = NULL;
	inst->pszBindAddr = NULL;
	inst->pszBindPath = NULL;
	inst->fileUID = -1;
	inst->fileGID = -1;
	inst->maxFrameSize = 200000;
	inst->fCreateMode = 0644;
	inst->bFailOnPerms = 1;
	inst->bUnlink = 0;
	inst->discardTruncatedMsg = 0;
	inst->flowControl = 1;
	inst->pszBindRuleset = NULL;
	inst->pszInputName = NULL;
	inst->bSuppOctetFram = 1;
	inst->bSPFramingFix = 0;
	inst->bKeepAlive = 0;
	inst->iKeepAliveIntvl = 0;
	inst->iKeepAliveProbes = 0;
	inst->iKeepAliveTime = 0;
	inst->bEmitMsgOnClose = 0;
	inst->bEmitMsgOnOpen = 0;
	inst->dfltTZ = NULL;
	inst->iAddtlFrameDelim = TCPSRV_NO_ADDTL_DELIMITER;
	inst->startRegex = NULL;
	inst->pBindRuleset = NULL;
	inst->ratelimitBurst = 10000; /* arbitrary high limit */
	inst->ratelimitInterval = 0; /* off */
	inst->compressionMode = COMPRESS_SINGLE_MSG;
	inst->multiLine = 0;
	inst->socketBacklog = 5;
	inst->pszLstnPortFileName = NULL;

	/* node created, let's add to config */
	if(loadModConf->tail == NULL) {
		loadModConf->tail = loadModConf->root = inst;
	} else {
		loadModConf->tail->next = inst;
		loadModConf->tail = inst;
	}

	*pinst = inst;
finalize_it:
	RETiRet;
}


/* This function is called when a new listener instace shall be added to
 * the current config object via the legacy config system. It just shuffles
 * all parameters to the listener in-memory instance.
 */
static rsRetVal addInstance(void __attribute__((unused)) *pVal, uchar *const pNewVal)
{
	instanceConf_t *inst;
	DEFiRet;

	if(pNewVal == NULL || *pNewVal == '\0') {
		parser_errmsg("imptcp: port number must be specified, listener ignored");
		ABORT_FINALIZE(RS_RET_PARAM_ERROR);
	}

	/* if we reach this point, a valid port is given in pNewVal */
	CHKiRet(createInstance(&inst));
	CHKmalloc(inst->pszBindPort = ustrdup(pNewVal));
	if((cs.lstnIP == NULL) || (cs.lstnIP[0] == '\0')) {
		inst->pszBindAddr = NULL;
	} else {
		CHKmalloc(inst->pszBindAddr = ustrdup(cs.lstnIP));
	}
	if((cs.pszBindRuleset == NULL) || (cs.pszBindRuleset[0] == '\0')) {
		inst->pszBindRuleset = NULL;
	} else {
		CHKmalloc(inst->pszBindRuleset = ustrdup(cs.pszBindRuleset));
	}
	if((cs.pszInputName == NULL) || (cs.pszInputName[0] == '\0')) {
		inst->pszInputName = NULL;
	} else {
		CHKmalloc(inst->pszInputName = ustrdup(cs.pszInputName));
	}
	inst->pBindRuleset = NULL;
	inst->bSuppOctetFram = cs.bSuppOctetFram;
	inst->bKeepAlive = cs.bKeepAlive;
	inst->iKeepAliveIntvl = cs.iKeepAliveTime;
	inst->iKeepAliveProbes = cs.iKeepAliveProbes;
	inst->iKeepAliveTime = cs.iKeepAliveTime;
	inst->bEmitMsgOnClose = cs.bEmitMsgOnClose;
	inst->bEmitMsgOnOpen = cs.bEmitMsgOnOpen;
	inst->iAddtlFrameDelim = cs.iAddtlFrameDelim;
	inst->maxFrameSize = cs.maxFrameSize;

finalize_it:
	free(pNewVal);
	RETiRet;
}


static rsRetVal
addListner(modConfData_t __attribute__((unused)) *modConf, instanceConf_t *inst)
{
	DEFiRet;
	ptcpsrv_t *pSrv = NULL;

	CHKmalloc(pSrv = calloc(1, sizeof(ptcpsrv_t)));
	pthread_mutex_init(&pSrv->mutSessLst, NULL);
	pSrv->pSess = NULL;
	pSrv->pLstn = NULL;
	pSrv->inst = inst;
	pSrv->bSuppOctetFram = inst->bSuppOctetFram;
	pSrv->bSPFramingFix = inst->bSPFramingFix;
	pSrv->bKeepAlive = inst->bKeepAlive;
	pSrv->iKeepAliveIntvl = inst->iKeepAliveTime;
	pSrv->iKeepAliveProbes = inst->iKeepAliveProbes;
	pSrv->iKeepAliveTime = inst->iKeepAliveTime;
	pSrv->bEmitMsgOnClose = inst->bEmitMsgOnClose;
	pSrv->bEmitMsgOnOpen = inst->bEmitMsgOnOpen;
	pSrv->compressionMode = inst->compressionMode;
	pSrv->dfltTZ = inst->dfltTZ;
	if (inst->pszBindPort != NULL) {
		CHKmalloc(pSrv->port = ustrdup(inst->pszBindPort));
	}
	pSrv->iAddtlFrameDelim = inst->iAddtlFrameDelim;
	pSrv->multiLine = inst->multiLine;
	pSrv->socketBacklog = inst->socketBacklog;
	pSrv->pszLstnPortFileName = inst->pszLstnPortFileName;
	pSrv->maxFrameSize = inst->maxFrameSize;
	if (inst->pszBindAddr == NULL) {
		pSrv->lstnIP = NULL;
	} else {
		CHKmalloc(pSrv->lstnIP = ustrdup(inst->pszBindAddr));
	}
	if (inst->pszBindPath == NULL) {
		pSrv->path = NULL;
	} else {
		CHKmalloc(pSrv->path = ustrdup(inst->pszBindPath));
		CHKmalloc(pSrv->port = ustrdup(inst->pszBindPath));
		pSrv->bUnixSocket = 1;
		pSrv->fCreateMode = inst->fCreateMode;
		pSrv->fileUID = inst->fileUID;
		pSrv->fileGID = inst->fileGID;
		pSrv->bFailOnPerms = inst->bFailOnPerms;
	}

	pSrv->bUnlink = inst->bUnlink;
	pSrv->discardTruncatedMsg = inst->discardTruncatedMsg;
	pSrv->flowControl = inst->flowControl;
	pSrv->pRuleset = inst->pBindRuleset;
	pSrv->pszInputName = ustrdup((inst->pszInputName == NULL) ?  UCHAR_CONSTANT("imptcp") : inst->pszInputName);
	CHKiRet(prop.Construct(&pSrv->pInputName));
	CHKiRet(prop.SetString(pSrv->pInputName, pSrv->pszInputName, ustrlen(pSrv->pszInputName)));
	CHKiRet(prop.ConstructFinalize(pSrv->pInputName));

	CHKiRet(ratelimitNew(&pSrv->ratelimiter, "imptcp", (char*) pSrv->port));
	ratelimitSetLinuxLike(pSrv->ratelimiter, inst->ratelimitInterval, inst->ratelimitBurst);
	ratelimitSetThreadSafe(pSrv->ratelimiter);
	/* add to linked list */
	pSrv->pNext = pSrvRoot;
	pSrvRoot = pSrv;

	/* all config vars are auto-reset -- this also is very useful with the
	 * new config format effort (v6).
	 */
	resetConfigVariables(NULL, NULL);

finalize_it:
	if(iRet != RS_RET_OK) {
		LogError(0, NO_ERRCODE, "error %d trying to add listener", iRet);
		if(pSrv != NULL) {
			destructSrv(pSrv);
		}
	}
	RETiRet;
}


/* destroy worker pool structures and wait for workers to terminate
 */
static void
startWorkerPool(void)
{
	int i;
	pthread_mutex_lock(&io_q.mut); /* locking to keep Coverity happy */
	wrkrRunning = 0;
	pthread_mutex_unlock(&io_q.mut);
	DBGPRINTF("imptcp: starting worker pool, %d workers\n", runModConf->wrkrMax);
	wrkrInfo = calloc(runModConf->wrkrMax, sizeof(struct wrkrInfo_s));
	if (wrkrInfo == NULL) {
		LogError(errno, RS_RET_OUT_OF_MEMORY, "imptcp: worker-info array allocation failed.");
		return;
	}
	for(i = 0 ; i < runModConf->wrkrMax ; ++i) {
		/* init worker info structure! */
		wrkrInfo[i].numCalled = 0;
		pthread_create(&wrkrInfo[i].tid, &wrkrThrdAttr, wrkr, &(wrkrInfo[i]));
	}

}

/* destroy worker pool structures and wait for workers to terminate
 */
static void
stopWorkerPool(void)
{
	int i;
	DBGPRINTF("imptcp: stoping worker pool\n");
	pthread_mutex_lock(&io_q.mut);
	pthread_cond_broadcast(&io_q.wakeup_worker); /* awake wrkr if not running */
	pthread_mutex_unlock(&io_q.mut);
	for(i = 0 ; i < runModConf->wrkrMax ; ++i) {
		pthread_join(wrkrInfo[i].tid, NULL);
		DBGPRINTF("imptcp: info: worker %d was called %llu times\n", i, wrkrInfo[i].numCalled);
	}
	free(wrkrInfo);
}



/* start up all listeners
 * This is a one-time stop once the module is set to start.
 */
static rsRetVal
startupServers(void)
{
	DEFiRet;
	rsRetVal localRet, lastErr;
	int iOK;
	int iAll;
	ptcpsrv_t *pSrv;

	iAll = iOK = 0;
	lastErr = RS_RET_ERR;
	pSrv = pSrvRoot;
	while(pSrv != NULL) {
		DBGPRINTF("imptcp: starting up server for port %s, name '%s'\n", pSrv->port, pSrv->pszInputName);
		localRet = startupSrv(pSrv);
		if(localRet == RS_RET_OK)
			iOK++;
		else
			lastErr = localRet;
		++iAll;
		pSrv = pSrv->pNext;
	}

	DBGPRINTF("imptcp: %d out of %d servers started successfully\n", iOK, iAll);
	if(iOK == 0)	/* iff all fails, we report an error */
		iRet = lastErr;

	RETiRet;
}

/* process new activity on listener. This means we need to accept a new
 * connection.
 */
static rsRetVal
lstnActivity(ptcplstn_t *pLstn)
{
	int newSock = -1;
	prop_t *peerName;
	prop_t *peerIP;
	rsRetVal localRet;
	DEFiRet;

	DBGPRINTF("imptcp: new connection on listen socket %d\n", pLstn->sock);
	while(glbl.GetGlobalInputTermState() == 0) {
		localRet = AcceptConnReq(pLstn, &newSock, &peerName, &peerIP);
		DBGPRINTF("imptcp: AcceptConnReq on listen socket %d returned %d\n", pLstn->sock, localRet);
		if(localRet == RS_RET_NO_MORE_DATA || glbl.GetGlobalInputTermState() == 1) {
			break;
		}
		CHKiRet(localRet);
		localRet = addSess(pLstn, newSock, peerName, peerIP);
		if(localRet != RS_RET_OK) {
			close(newSock);
			prop.Destruct(&peerName);
			prop.Destruct(&peerIP);
			ABORT_FINALIZE(localRet);
		}
	}

finalize_it:
	RETiRet;
}

/* process new activity on session. This means we need to accept data
 * or close the session.
 */
static rsRetVal
sessActivity(ptcpsess_t *pSess, int *continue_polling)
{
	int lenRcv;
	int lenBuf;
	uchar *peerName;
	int lenPeer;
	int remsock = 0; /* init just to keep compiler happy... :-( */
	sbool bEmitOnClose = 0;
	char rcvBuf[128*1024];
	DEFiRet;

	DBGPRINTF("imptcp: new activity on session socket %d\n", pSess->sock);

	while(1) {
		lenBuf = sizeof(rcvBuf);
		lenRcv = recv(pSess->sock, rcvBuf, lenBuf, 0);

		if(lenRcv > 0) {
			/* have data, process it */
			DBGPRINTF("imptcp: data(%d) on socket %d: %s\n", lenBuf, pSess->sock, rcvBuf);
			CHKiRet(DataRcvd(pSess, rcvBuf, lenRcv));
		} else if (lenRcv == 0) {
			/* session was closed, do clean-up */
			if(pSess->pLstn->pSrv->bEmitMsgOnClose) {
				prop.GetString(pSess->peerName, &peerName, &lenPeer),
				remsock = pSess->sock;
				bEmitOnClose = 1;
			}
			*continue_polling = 0;
			if(bEmitOnClose) {
				LogError(0, RS_RET_PEER_CLOSED_CONN, "imptcp session %d closed by "
					  	"remote peer %s.", remsock, peerName);
			}
			CHKiRet(closeSess(pSess)); /* close may emit more messages in strmzip mode! */
			break;
		} else {
			if(CHK_EAGAIN_EWOULDBLOCK)
				break;
			DBGPRINTF("imptcp: error on session socket %d - closed.\n", pSess->sock);
			*continue_polling = 0;
			closeSess(pSess); /* try clean-up by dropping session */
			break;
		}
	}

finalize_it:
	RETiRet;
}


/* This function is called to process a single request. This may
 * be carried out by the main worker or a helper. It can be run
 * concurrently.
 */
static void
processWorkItem(epolld_t *epd)
{
	int continue_polling = 1;

	switch(epd->typ) {
	case epolld_lstn:
		/* listener never stops polling (except server shutdown) */
		lstnActivity((ptcplstn_t *) epd->ptr);
		break;
	case epolld_sess:
		sessActivity((ptcpsess_t *) epd->ptr, &continue_polling);
		break;
	default:
		LogError(0, RS_RET_INTERNAL_ERROR,
						"error: invalid epolld_type_t %d after epoll", epd->typ);
		break;
	}
	if (continue_polling == 1) {
		epoll_ctl(epollfd, EPOLL_CTL_MOD, epd->sock, &(epd->ev));
	}
}


static rsRetVal
initIoQ(void)
{
	DEFiRet;
	CHKiConcCtrl(pthread_mutex_init(&io_q.mut, NULL));
	CHKiConcCtrl(pthread_cond_init(&io_q.wakeup_worker, NULL));
	STAILQ_INIT(&io_q.q);
	io_q.sz = 0;
	io_q.ctrMaxSz = 0; /* TODO: discuss this and fix potential concurrent read/write issues */
	CHKiRet(statsobj.Construct(&io_q.stats));
	CHKiRet(statsobj.SetName(io_q.stats, (uchar*) "io-work-q"));
	CHKiRet(statsobj.SetOrigin(io_q.stats, (uchar*) "imptcp"));
	STATSCOUNTER_INIT(io_q.ctrEnq, io_q.mutCtrEnq);
	CHKiRet(statsobj.AddCounter(io_q.stats, UCHAR_CONSTANT("enqueued"),
								ctrType_IntCtr, CTR_FLAG_RESETTABLE, &io_q.ctrEnq));
	CHKiRet(statsobj.AddCounter(io_q.stats, UCHAR_CONSTANT("maxqsize"),
								ctrType_Int, CTR_FLAG_NONE, &io_q.ctrMaxSz));
	CHKiRet(statsobj.ConstructFinalize(io_q.stats));
finalize_it:
	RETiRet;
}

static void
destroyIoQ(void)
{
	io_req_t *n;
	if (io_q.stats != NULL) {
		statsobj.Destruct(&io_q.stats);
	}
	pthread_mutex_lock(&io_q.mut);
	while (!STAILQ_EMPTY(&io_q.q)) {
		n = STAILQ_FIRST(&io_q.q);
		STAILQ_REMOVE_HEAD(&io_q.q, link);
		LogError(0, RS_RET_INTERNAL_ERROR, "imptcp: discarded enqueued io-work to allow shutdown "
								"- ignored");
		free(n);
	}
	io_q.sz = 0;
	pthread_mutex_unlock(&io_q.mut);
	pthread_cond_destroy(&io_q.wakeup_worker);
	pthread_mutex_destroy(&io_q.mut);
}

static rsRetVal
enqueueIoWork(epolld_t *epd, int dispatchInlineIfQueueFull) {
	io_req_t *n;
	int dispatchInline;
	int inlineDispatchThreshold;
	DEFiRet;
	
	CHKmalloc(n = malloc(sizeof(io_req_t)));
	n->epd = epd;
	
	inlineDispatchThreshold = DFLT_inlineDispatchThreshold * runModConf->wrkrMax;
	dispatchInline = 0;
	
	pthread_mutex_lock(&io_q.mut);
	if (dispatchInlineIfQueueFull && io_q.sz > inlineDispatchThreshold) {
		dispatchInline = 1;
	} else {
		STAILQ_INSERT_TAIL(&io_q.q, n, link);
		io_q.sz++;
		STATSCOUNTER_INC(io_q.ctrEnq, io_q.mutCtrEnq);
		STATSCOUNTER_SETMAX_NOMUT(io_q.ctrMaxSz, io_q.sz);
		pthread_cond_signal(&io_q.wakeup_worker);
	}
	pthread_mutex_unlock(&io_q.mut);

	if (dispatchInline == 1) {
		free(n);
		processWorkItem(epd);
	}
finalize_it:
	if (iRet != RS_RET_OK) {
		if (n == NULL) {
			LogError(0, iRet, "imptcp: couldn't allocate memory to enqueue io-request - ignored");
		}
	}
	RETiRet;
}

/* This function is called to process a complete workset, that
 * is a set of events returned from epoll.
 */
static void
processWorkSet(int nEvents, struct epoll_event events[])
{
	int iEvt;
	int remainEvents;
	remainEvents = nEvents;
	epolld_t *epd;

	for(iEvt = 0 ; (iEvt < nEvents) && (glbl.GetGlobalInputTermState() == 0) ; ++iEvt) {
		epd = (epolld_t*)events[iEvt].data.ptr;
		if(runModConf->bProcessOnPoller && remainEvents == 1) {
			/* process self, save context switch */
			processWorkItem(epd);
		} else {
			enqueueIoWork(epd, runModConf->bProcessOnPoller);
		}
		--remainEvents;
	}
}


/* worker to process incoming requests
 */
static void *
wrkr(void *myself)
{
	struct wrkrInfo_s *me = (struct wrkrInfo_s*) myself;
	pthread_mutex_lock(&io_q.mut);
	++wrkrRunning;
	pthread_mutex_unlock(&io_q.mut);

	io_req_t *n;
	while(1) {
		n = NULL;
		pthread_mutex_lock(&io_q.mut);
		if (io_q.sz == 0) {
			--wrkrRunning;
			if (glbl.GetGlobalInputTermState() != 0) {
				pthread_mutex_unlock(&io_q.mut);
				break;
			} else {
				DBGPRINTF("imptcp: worker %llu waiting on new work items\n",
					(unsigned long long) me->tid);
				pthread_cond_wait(&io_q.wakeup_worker, &io_q.mut);
				DBGPRINTF("imptcp: worker %llu awoken\n", (unsigned long long) me->tid);
			}
			++wrkrRunning;
		}
		if (io_q.sz > 0) {
			n = STAILQ_FIRST(&io_q.q);
			STAILQ_REMOVE_HEAD(&io_q.q, link);
			io_q.sz--;
		}
		pthread_mutex_unlock(&io_q.mut);

		if (n != NULL) {
			++me->numCalled;
			processWorkItem(n->epd);
			free(n);
		}
	}
	return NULL;
}


BEGINnewInpInst
	struct cnfparamvals *pvals;
	instanceConf_t *inst;
	char *cstr;
	int i;
CODESTARTnewInpInst
	DBGPRINTF("newInpInst (imptcp)\n");

	if((pvals = nvlstGetParams(lst, &inppblk, NULL)) == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	if(Debug) {
		dbgprintf("input param blk in imptcp:\n");
		cnfparamsPrint(&inppblk, pvals);
	}

	CHKiRet(createInstance(&inst));

	for(i = 0 ; i < inppblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(inppblk.descr[i].name, "port")) {
			inst->pszBindPort = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "address")) {
			inst->pszBindAddr = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "path")) {
			inst->pszBindPath = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "unlink")) {
			inst->bUnlink = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "discardtruncatedmsg")) {
			inst->discardTruncatedMsg = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "flowcontrol")) {
			inst->flowControl = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "fileowner")) {
			inst->fileUID = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "fileownernum")) {
			inst->fileUID = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "filegroup")) {
			inst->fileGID = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "filegroupnum")) {
			inst->fileGID = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "filecreatemode")) {
			inst->fCreateMode = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "failonpermsfailure")) {
			inst->bFailOnPerms = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "name")) {
			inst->pszInputName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "maxframesize")) {
			const int max = (int) pvals[i].val.d.n;
			if(max <= 200000000) {
				inst->maxFrameSize = max;
			} else {
				parser_errmsg("imptcp: invalid value for 'maxFrameSize' "
						"parameter given is %d, max is 200000000", max);
				ABORT_FINALIZE(RS_RET_PARAM_ERROR);
			}
		} else if(!strcmp(inppblk.descr[i].name, "framing.delimiter.regex")) {
			inst->startRegex = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "ruleset")) {
			inst->pszBindRuleset = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "supportoctetcountedframing")) {
			inst->bSuppOctetFram = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "framingfix.cisco.asa")) {
			inst->bSPFramingFix = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "compression.mode")) {
			cstr = es_str2cstr(pvals[i].val.d.estr, NULL);
			if(!strcasecmp(cstr, "stream:always")) {
				inst->compressionMode = COMPRESS_STREAM_ALWAYS;
			} else if(!strcasecmp(cstr, "none")) {
				inst->compressionMode = COMPRESS_NEVER;
			} else {
				parser_errmsg("imptcp: invalid value for 'compression.mode' "
					 "parameter (given is '%s')", cstr);
				free(cstr);
				ABORT_FINALIZE(RS_RET_PARAM_ERROR);
			}
			free(cstr);
		} else if(!strcmp(inppblk.descr[i].name, "keepalive")) {
			inst->bKeepAlive = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "keepalive.probes")) {
			inst->iKeepAliveProbes = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "keepalive.time")) {
			inst->iKeepAliveTime = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "keepalive.interval")) {
			inst->iKeepAliveIntvl = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "addtlframedelimiter")) {
			inst->iAddtlFrameDelim = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "notifyonconnectionclose")) {
			inst->bEmitMsgOnClose = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "notifyonconnectionopen")) {
			inst->bEmitMsgOnOpen = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "defaulttz")) {
			inst->dfltTZ = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "ratelimit.burst")) {
			inst->ratelimitBurst = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "ratelimit.interval")) {
			inst->ratelimitInterval = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "multiline")) {
			inst->multiLine = (sbool) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "listenportfilename")) {
			inst->pszLstnPortFileName =  (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "socketbacklog")) {
			inst->socketBacklog = (int) pvals[i].val.d.n;
		} else {
			dbgprintf("imptcp: program error, non-handled "
			  "param '%s'\n", inppblk.descr[i].name);
		}

		char *bindPort = (char *) inst->pszBindPort;
		char *bindPath = (char *) inst->pszBindPath;
		if ((bindPort == NULL || strlen(bindPort) < 1) && (bindPath == NULL || strlen (bindPath) < 1)) {
			parser_errmsg("imptcp: Must have either port or path defined");
			ABORT_FINALIZE(RS_RET_PARAM_ERROR);
		}
	}

	if(inst->startRegex != NULL) {
		const int errcode = regcomp(&inst->start_preg, (char*)inst->startRegex, REG_EXTENDED);
		if(errcode != 0) {
			char errbuff[512];
			regerror(errcode, &inst->start_preg, errbuff, sizeof(errbuff));
			parser_errmsg("imptcp: error in framing.delimiter.regex expansion: %s", errbuff);
			ABORT_FINALIZE(RS_RET_ERR);
		}
	}
finalize_it:
CODE_STD_FINALIZERnewInpInst
	cnfparamvalsDestruct(pvals, &inppblk);
ENDnewInpInst


BEGINbeginCnfLoad
CODESTARTbeginCnfLoad
	loadModConf = pModConf;
	pModConf->pConf = pConf;
	/* init our settings */
	loadModConf->wrkrMax = DFLT_wrkrMax;
	loadModConf->bProcessOnPoller = 1;
	loadModConf->configSetViaV2Method = 0;
	bLegacyCnfModGlobalsPermitted = 1;
	/* init legacy config vars */
	initConfigSettings();
ENDbeginCnfLoad


BEGINsetModCnf
	struct cnfparamvals *pvals = NULL;
	int i;
CODESTARTsetModCnf
	pvals = nvlstGetParams(lst, &modpblk, NULL);
	if(pvals == NULL) {
		LogError(0, RS_RET_MISSING_CNFPARAMS, "imptcp: error processing module "
				"config parameters [module(...)]");
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	if(Debug) {
		dbgprintf("module (global) param blk for imptcp:\n");
		cnfparamsPrint(&modpblk, pvals);
	}

	for(i = 0 ; i < modpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(modpblk.descr[i].name, "threads")) {
			loadModConf->wrkrMax = (int) pvals[i].val.d.n;
		} else if(!strcmp(modpblk.descr[i].name, "processOnPoller")) {
			loadModConf->bProcessOnPoller = (int) pvals[i].val.d.n;
		} else {
			dbgprintf("imptcp: program error, non-handled "
			  "param '%s' in beginCnfLoad\n", modpblk.descr[i].name);
		}
	}

	/* remove all of our legacy handlers, as they can not used in addition
	 * the the new-style config method.
	 */
	bLegacyCnfModGlobalsPermitted = 0;
	loadModConf->configSetViaV2Method = 1;

finalize_it:
	if(pvals != NULL)
		cnfparamvalsDestruct(pvals, &modpblk);
ENDsetModCnf


BEGINendCnfLoad
CODESTARTendCnfLoad
	if(!loadModConf->configSetViaV2Method) {
		/* persist module-specific settings from legacy config system */
		loadModConf->wrkrMax = cs.wrkrMax;
	}

	loadModConf = NULL; /* done loading */
	/* free legacy config vars */
	free(cs.pszInputName);
	free(cs.lstnIP);
	cs.pszInputName = NULL;
	cs.lstnIP = NULL;
ENDendCnfLoad


/* function to generate error message if framework does not find requested ruleset */
static inline void
std_checkRuleset_genErrMsg(__attribute__((unused)) modConfData_t *modConf, instanceConf_t *inst)
{
	LogError(0, NO_ERRCODE, "imptcp: ruleset '%s' for port %s not found - "
			"using default ruleset instead", inst->pszBindRuleset,
			inst->pszBindPort);
}
BEGINcheckCnf
	instanceConf_t *inst;
CODESTARTcheckCnf
	for(inst = pModConf->root ; inst != NULL ; inst = inst->next) {
		std_checkRuleset(pModConf, inst);
	}
ENDcheckCnf


BEGINactivateCnfPrePrivDrop
	instanceConf_t *inst;
CODESTARTactivateCnfPrePrivDrop
	iMaxLine = glbl.GetMaxLine(); /* get maximum size we currently support */
	DBGPRINTF("imptcp: config params iMaxLine %d\n", iMaxLine);

	runModConf = pModConf;
	for(inst = runModConf->root ; inst != NULL ; inst = inst->next) {
		addListner(pModConf, inst);
	}
	if(pSrvRoot == NULL) {
		LogError(0, RS_RET_NO_LSTN_DEFINED, "imptcp: no ptcp server defined, module can not run.");
		ABORT_FINALIZE(RS_RET_NO_RUN);
	}

#	if defined(EPOLL_CLOEXEC) && defined(HAVE_EPOLL_CREATE1)
	DBGPRINTF("imptcp uses epoll_create1()\n");
	epollfd = epoll_create1(EPOLL_CLOEXEC);
	if(epollfd < 0 && errno == ENOSYS)
#	endif
	{
		DBGPRINTF("imptcp uses epoll_create()\n");
		/* reading the docs, the number of epoll events passed to
		 * epoll_create() seems not to be used at all in kernels. So
		 * we just provide "a" number, happens to be 10.
		 */
		epollfd = epoll_create(10);
	}

	if(epollfd < 0) {
		LogError(0, RS_RET_EPOLL_CR_FAILED, "error: epoll_create() failed");
		ABORT_FINALIZE(RS_RET_NO_RUN);
	}

	/* start up servers, but do not yet read input data */
	CHKiRet(startupServers());
	DBGPRINTF("imptcp started up, but not yet receiving data\n");
finalize_it:
ENDactivateCnfPrePrivDrop


BEGINactivateCnf
CODESTARTactivateCnf
	/* nothing to do, all done pre priv drop */
ENDactivateCnf


BEGINfreeCnf
	instanceConf_t *inst, *del;
CODESTARTfreeCnf
	for(inst = pModConf->root ; inst != NULL ; ) {
		free(inst->pszBindPort);
		free(inst->pszBindPath);
		free(inst->pszBindAddr);
		free(inst->pszBindRuleset);
		free(inst->pszInputName);
		free(inst->dfltTZ);
		if(inst->startRegex != NULL) {
			regfree(&inst->start_preg);
			free(inst->startRegex);
		}
		del = inst;
		inst = inst->next;
		free(del);
	}
ENDfreeCnf


/* This function is called to gather input.
 */
BEGINrunInput
	int nEvents;
	struct epoll_event events[128];
CODESTARTrunInput
	initIoQ();
	startWorkerPool();
	DBGPRINTF("imptcp: now beginning to process input data\n");
	while(glbl.GetGlobalInputTermState() == 0) {
		DBGPRINTF("imptcp going on epoll_wait\n");
		nEvents = epoll_wait(epollfd, events, sizeof(events)/sizeof(struct epoll_event), -1);
		DBGPRINTF("imptcp: epoll returned %d events\n", nEvents);
		processWorkSet(nEvents, events);
	}
	DBGPRINTF("imptcp: successfully terminated\n");
	/* we stop the worker pool in AfterRun, in case we get cancelled for some reason (old Interface) */
ENDrunInput


/* initialize and return if will run or not */
BEGINwillRun
CODESTARTwillRun
ENDwillRun


/* completely shut down a server, that means closing all of its
 * listeners and sessions.
 */
static void
shutdownSrv(ptcpsrv_t *pSrv)
{
	ptcplstn_t *pLstn, *lstnDel;
	ptcpsess_t *pSess, *sessDel;

	/* listeners */
	pLstn = pSrv->pLstn;
	while(pLstn != NULL) {
		close(pLstn->sock);
		statsobj.Destruct(&(pLstn->stats));
		/* now unlink listner */
		lstnDel = pLstn;
		pLstn = pLstn->next;
		DBGPRINTF("imptcp shutdown listen socket %d (rcvd %lld bytes, "
			  "decompressed %lld)\n", lstnDel->sock, lstnDel->rcvdBytes,
				  lstnDel->rcvdDecompressed);
		free(lstnDel->epd);
		free(lstnDel);
	}

	if (pSrv->bUnixSocket && pSrv->bUnlink) {
		unlink((char*) pSrv->path);
	}

	/* sessions */
	pSess = pSrv->pSess;
	while(pSess != NULL) {
		close(pSess->sock);
		sessDel = pSess;
		pSess = pSess->next;
		DBGPRINTF("imptcp shutdown session socket %d\n", sessDel->sock);
		destructSess(sessDel);
	}
}


BEGINafterRun
	ptcpsrv_t *pSrv, *srvDel;
CODESTARTafterRun
	stopWorkerPool();
	destroyIoQ();

	/* we need to close everything that is still open */
	pSrv = pSrvRoot;
	while(pSrv != NULL) {
		srvDel = pSrv;
		pSrv = pSrv->pNext;
		shutdownSrv(srvDel);
		destructSrv(srvDel);
	}

	close(epollfd);
ENDafterRun


BEGINmodExit
CODESTARTmodExit
	pthread_attr_destroy(&wrkrThrdAttr);
	/* release objects we used */
	objRelease(glbl, CORE_COMPONENT);
	objRelease(statsobj, CORE_COMPONENT);
	objRelease(prop, CORE_COMPONENT);
	objRelease(net, LM_NET_FILENAME);
	objRelease(datetime, CORE_COMPONENT);
	objRelease(ruleset, CORE_COMPONENT);
ENDmodExit


static rsRetVal
resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal)
{
	cs.bEmitMsgOnClose = 0;
	cs.bEmitMsgOnOpen = 0;
	cs.wrkrMax = DFLT_wrkrMax;
	cs.bKeepAlive = 0;
	cs.iKeepAliveProbes = 0;
	cs.iKeepAliveTime = 0;
	cs.iKeepAliveIntvl = 0;
	cs.bSuppOctetFram = 1;
	cs.iAddtlFrameDelim = TCPSRV_NO_ADDTL_DELIMITER;
	cs.maxFrameSize = 200000;
	free(cs.pszInputName);
	cs.pszInputName = NULL;
	free(cs.lstnIP);
	cs.lstnIP = NULL;
	return RS_RET_OK;
}


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	if(eFeat == sFEATURENonCancelInputTermination)
		iRet = RS_RET_OK;
ENDisCompatibleWithFeature


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_IMOD_QUERIES
CODEqueryEtryPt_STD_CONF2_QUERIES
CODEqueryEtryPt_STD_CONF2_setModCnf_QUERIES
CODEqueryEtryPt_STD_CONF2_PREPRIVDROP_QUERIES
CODEqueryEtryPt_STD_CONF2_IMOD_QUERIES
CODEqueryEtryPt_IsCompatibleWithFeature_IF_OMOD_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	/* request objects we use */
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(statsobj, CORE_COMPONENT));
	CHKiRet(objUse(prop, CORE_COMPONENT));
	CHKiRet(objUse(net, LM_NET_FILENAME));
	CHKiRet(objUse(datetime, CORE_COMPONENT));
	CHKiRet(objUse(ruleset, CORE_COMPONENT));

	/* initialize "read-only" thread attributes */
	pthread_attr_init(&wrkrThrdAttr);
	pthread_attr_setstacksize(&wrkrThrdAttr, 4096*1024);

	/* init legacy config settings */
	initConfigSettings();

	/* register config file handlers */
	CHKiRet(omsdRegCFSLineHdlr(UCHAR_CONSTANT("inputptcpserverrun"), 0, eCmdHdlrGetWord,
				   addInstance, NULL, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr(UCHAR_CONSTANT("inputptcpserverkeepalive"), 0, eCmdHdlrBinary,
				   NULL, &cs.bKeepAlive, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr(UCHAR_CONSTANT("inputptcpserverkeepalive_probes"), 0, eCmdHdlrInt,
				   NULL, &cs.iKeepAliveProbes, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr(UCHAR_CONSTANT("inputptcpserverkeepalive_time"), 0, eCmdHdlrInt,
				   NULL, &cs.iKeepAliveTime, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr(UCHAR_CONSTANT("inputptcpserverkeepalive_intvl"), 0, eCmdHdlrInt,
				   NULL, &cs.iKeepAliveIntvl, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr(UCHAR_CONSTANT("inputptcpserversupportoctetcountedframing"), 0, eCmdHdlrBinary,
				   NULL, &cs.bSuppOctetFram, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr(UCHAR_CONSTANT("inputptcpservernotifyonconnectionclose"), 0,
				   eCmdHdlrBinary, NULL, &cs.bEmitMsgOnClose, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr(UCHAR_CONSTANT("inputptcpserveraddtlframedelimiter"), 0, eCmdHdlrInt,
				   NULL, &cs.iAddtlFrameDelim, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr(UCHAR_CONSTANT("inputptcpserverinputname"), 0,
				   eCmdHdlrGetWord, NULL, &cs.pszInputName, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr(UCHAR_CONSTANT("inputptcpserverlistenip"), 0,
				   eCmdHdlrGetWord, NULL, &cs.lstnIP, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr(UCHAR_CONSTANT("inputptcpserverbindruleset"), 0,
				   eCmdHdlrGetWord, NULL, &cs.pszBindRuleset, STD_LOADABLE_MODULE_ID));
	/* module-global parameters */
	CHKiRet(regCfSysLineHdlr2(UCHAR_CONSTANT("inputptcpserverhelperthreads"), 0, eCmdHdlrInt,
				   NULL, &cs.wrkrMax, STD_LOADABLE_MODULE_ID, &bLegacyCnfModGlobalsPermitted));
	CHKiRet(omsdRegCFSLineHdlr(UCHAR_CONSTANT("resetconfigvariables"), 1, eCmdHdlrCustomHandler,
		resetConfigVariables, NULL, STD_LOADABLE_MODULE_ID));
ENDmodInit


/* vim:set ai:
 */
