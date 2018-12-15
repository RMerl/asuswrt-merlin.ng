/* imudp.c
 * This is the implementation of the UDP input module.
 *
 * NOTE: read comments in module-template.h to understand how this file
 *       works!
 *
 * Copyright 2007-2018 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of rsyslog.
 *
 * Rsyslog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rsyslog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rsyslog.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <poll.h>
#ifdef HAVE_SYS_EPOLL_H
#	include <sys/epoll.h>
#endif
#ifdef HAVE_SCHED_H
#	include <sched.h>
#endif
#include "rsyslog.h"
#include "dirty.h"
#include "net.h"
#include "cfsysline.h"
#include "module-template.h"
#include "srUtils.h"
#include "errmsg.h"
#include "glbl.h"
#include "msg.h"
#include "parser.h"
#include "datetime.h"
#include "prop.h"
#include "ruleset.h"
#include "statsobj.h"
#include "ratelimit.h"
#include "unicode-helper.h"

MODULE_TYPE_INPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("imudp")

/* defines */
#define MAX_WRKR_THREADS 32

/* Module static data */
DEF_IMOD_STATIC_DATA
DEFobjCurrIf(glbl)
DEFobjCurrIf(net)
DEFobjCurrIf(datetime)
DEFobjCurrIf(prop)
DEFobjCurrIf(ruleset)
DEFobjCurrIf(statsobj)


static struct lstn_s {
	struct lstn_s *next;
	int sock;		/* socket */
	ruleset_t *pRuleset;	/* bound ruleset */
	prop_t *pInputName;
	statsobj_t *stats;	/* listener stats */
	ratelimit_t *ratelimiter;
	uchar *dfltTZ;
	STATSCOUNTER_DEF(ctrSubmit, mutCtrSubmit)
	STATSCOUNTER_DEF(ctrDisallowed, mutCtrDisallowed)
} *lcnfRoot = NULL, *lcnfLast = NULL;


static int bLegacyCnfModGlobalsPermitted;/* are legacy module-global config parameters permitted? */
static int bDoACLCheck;			/* are ACL checks neeed? Cached once immediately before listener startup */
static int iMaxLine;			/* maximum UDP message size supported */
#define BATCH_SIZE_DFLT 32		/* do not overdo, has heavy toll on memory, especially with large msgs */
#define TIME_REQUERY_DFLT 2
#define SCHED_PRIO_UNSET -12345678	/* a value that indicates that the scheduling priority has not been set */
/* config vars for legacy config system */
static struct configSettings_s {
	uchar *pszBindAddr;		/* IP to bind socket to */
	char  *pszBindDevice;		/* Device to bind socket to */
	uchar *pszSchedPolicy;		/* scheduling policy string */
	uchar *pszBindRuleset;		/* name of Ruleset to bind to */
	int iSchedPrio;			/* scheduling priority */
	int iTimeRequery;		/* how often is time to be queried inside tight recv loop? 0=always */
} cs;

struct instanceConf_s {
	uchar *pszBindAddr;		/* IP to bind socket to */
	char  *pszBindDevice;		/* Device to bind socket to */
	uchar *pszBindPort;		/* Port to bind socket to */
	uchar *pszBindRuleset;		/* name of ruleset to bind to */
	uchar *inputname;
	ruleset_t *pBindRuleset;	/* ruleset to bind listener to (use system default if unspecified) */
	uchar *dfltTZ;
	int ratelimitInterval;
	int ratelimitBurst;
	int rcvbuf;			/* 0 means: do not set, keep OS default */
	/*  0 means:  IP_FREEBIND is disabled
	1 means:  IP_FREEBIND enabled + warning disabled
	1+ means: IP+FREEBIND enabled + warning enabled */
	int ipfreebind;
	struct instanceConf_s *next;
	sbool bAppendPortToInpname;
};

/* The following structure controls the worker threads. Global data is
 * needed for their access.
 */
static struct wrkrInfo_s {
	pthread_t tid;	/* the worker's thread ID */
	int id;
	thrdInfo_t *pThrd;
	statsobj_t *stats;	/* worker thread stats */
	STATSCOUNTER_DEF(ctrCall_recvmmsg, mutCtrCall_recvmmsg)
	STATSCOUNTER_DEF(ctrCall_recvmsg, mutCtrCall_recvmsg)
	STATSCOUNTER_DEF(ctrMsgsRcvd, mutCtrMsgsRcvd)
	uchar *pRcvBuf;		/* receive buffer (for a single packet) */
#	ifdef HAVE_RECVMMSG
	struct sockaddr_storage *frominet;
	struct mmsghdr *recvmsg_mmh;
	struct iovec *recvmsg_iov;
#	endif
} wrkrInfo[MAX_WRKR_THREADS];

struct modConfData_s {
	rsconf_t *pConf;		/* our overall config object */
	instanceConf_t *root, *tail;
	uchar *pszSchedPolicy;		/* scheduling policy string */
	int iSchedPolicy;		/* scheduling policy as SCHED_xxx */
	int iSchedPrio;			/* scheduling priority */
	int iTimeRequery;		/* how often is time to be queried inside tight recv loop? 0=always */
	int batchSize;			/* max nbr of input batch --> also recvmmsg() max count */
	int8_t wrkrMax;			/* max nbr of worker threads */
	sbool configSetViaV2Method;
	sbool bPreserveCase;	/* preserves the case of fromhost; "off" by default */
};
static modConfData_t *loadModConf = NULL;/* modConf ptr to use for the current load process */
static modConfData_t *runModConf = NULL;/* modConf ptr to use for the current load process */

/* module-global parameters */
static struct cnfparamdescr modpdescr[] = {
	{ "schedulingpolicy", eCmdHdlrGetWord, 0 },
	{ "schedulingpriority", eCmdHdlrInt, 0 },
	{ "batchsize", eCmdHdlrInt, 0 },
	{ "threads", eCmdHdlrPositiveInt, 0 },
	{ "timerequery", eCmdHdlrInt, 0 },
	{ "preservecase", eCmdHdlrBinary, 0 }
};
static struct cnfparamblk modpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(modpdescr)/sizeof(struct cnfparamdescr),
	  modpdescr
	};

/* input instance parameters */
static struct cnfparamdescr inppdescr[] = {
	{ "port", eCmdHdlrArray, CNFPARAM_REQUIRED }, /* legacy: InputTCPServerRun */
	{ "defaulttz", eCmdHdlrString, 0 },
	{ "inputname", eCmdHdlrGetWord, 0 },
	{ "inputname.appendport", eCmdHdlrBinary, 0 },
	{ "name", eCmdHdlrGetWord, 0 },
	{ "name.appendport", eCmdHdlrBinary, 0 },
	{ "address", eCmdHdlrString, 0 },
	{ "device", eCmdHdlrString, 0 },
	{ "ratelimit.interval", eCmdHdlrInt, 0 },
	{ "ratelimit.burst", eCmdHdlrInt, 0 },
	{ "rcvbufsize", eCmdHdlrSize, 0 },
	{ "ipfreebind", eCmdHdlrInt, 0 },
	{ "ruleset", eCmdHdlrString, 0 }
};
static struct cnfparamblk inppblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(inppdescr)/sizeof(struct cnfparamdescr),
	  inppdescr
	};

#include "im-helper.h" /* must be included AFTER the type definitions! */


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
	inst->pBindRuleset = NULL;

	inst->pszBindPort = NULL;
	inst->pszBindAddr = NULL;
	inst->pszBindDevice = NULL;
	inst->pszBindRuleset = NULL;
	inst->inputname = NULL;
	inst->bAppendPortToInpname = 0;
	inst->ratelimitBurst = 10000; /* arbitrary high limit */
	inst->ratelimitInterval = 0; /* off */
	inst->rcvbuf = 0;
	inst->ipfreebind = IPFREEBIND_ENABLED_WITH_LOG;
	inst->dfltTZ = NULL;

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
 * rgerhards, 2011-05-04
 */
static rsRetVal addInstance(void __attribute__((unused)) *pVal, uchar *pNewVal)
{
	instanceConf_t *inst;
	DEFiRet;

	CHKiRet(createInstance(&inst));
	CHKmalloc(inst->pszBindPort = ustrdup((pNewVal == NULL || *pNewVal == '\0')
				 	       ? (uchar*) "514" : pNewVal));
	if((cs.pszBindAddr == NULL) || (cs.pszBindAddr[0] == '\0')) {
		inst->pszBindAddr = NULL;
	} else {
		CHKmalloc(inst->pszBindAddr = ustrdup(cs.pszBindAddr));
	}
	if((cs.pszBindDevice == NULL) || (cs.pszBindDevice[0] == '\0')) {
		inst->pszBindDevice= NULL;
	} else {
		CHKmalloc(inst->pszBindDevice = strdup(cs.pszBindDevice));
	}
	if((cs.pszBindRuleset == NULL) || (cs.pszBindRuleset[0] == '\0')) {
		inst->pszBindRuleset = NULL;
	} else {
		CHKmalloc(inst->pszBindRuleset = ustrdup(cs.pszBindRuleset));
	}

finalize_it:
	free(pNewVal);
	RETiRet;
}


/* This function is called when a new listener shall be added. It takes
 * the instance config description, tries to bind the socket and, if that
 * succeeds, adds it to the list of existing listen sockets.
 */
static rsRetVal
addListner(instanceConf_t *inst)
{
	DEFiRet;
	uchar *bindAddr;
	int *newSocks;
	int iSrc;
	struct lstn_s *newlcnfinfo;
	uchar *bindName;
	uchar *port;
	uchar dispname[64], inpnameBuf[128];
	uchar *inputname;

	/* check which address to bind to. We could do this more compact, but have not
	 * done so in order to make the code more readable. -- rgerhards, 2007-12-27
	 */
	if(inst->pszBindAddr == NULL)
		bindAddr = NULL;
	else if(inst->pszBindAddr[0] == '*' && inst->pszBindAddr[1] == '\0')
		bindAddr = NULL;
	else
		bindAddr = inst->pszBindAddr;
	bindName = (bindAddr == NULL) ? (uchar*)"*" : bindAddr;
	port = (inst->pszBindPort == NULL || *inst->pszBindPort == '\0') ? (uchar*) "514" : inst->pszBindPort;

	DBGPRINTF("Trying to open syslog UDP ports at %s:%s.\n", bindName, inst->pszBindPort);

	newSocks = net.create_udp_socket(bindAddr, port, 1, inst->rcvbuf, 0, inst->ipfreebind, inst->pszBindDevice);
	if(newSocks != NULL) {
		/* we now need to add the new sockets to the existing set */
		/* ready to copy */
		for(iSrc = 1 ; iSrc <= newSocks[0] ; ++iSrc) {
			CHKmalloc(newlcnfinfo = (struct lstn_s*) calloc(1, sizeof(struct lstn_s)));
			newlcnfinfo->next = NULL;
			newlcnfinfo->sock = newSocks[iSrc];
			newlcnfinfo->pRuleset = inst->pBindRuleset;
			newlcnfinfo->dfltTZ = inst->dfltTZ;
			if(inst->inputname == NULL) {
				inputname = (uchar*)"imudp";
			} else {
				inputname = inst->inputname;
			}
			snprintf((char*)dispname, sizeof(dispname), "%s(%s:%s)", inputname, bindName, port);
			dispname[sizeof(dispname)-1] = '\0'; /* just to be on the save side... */
			CHKiRet(ratelimitNew(&newlcnfinfo->ratelimiter, (char*)dispname, NULL));
			if(inst->bAppendPortToInpname) {
				snprintf((char*)inpnameBuf, sizeof(inpnameBuf), "%s%s",
					inputname, port);
				inpnameBuf[sizeof(inpnameBuf)-1] = '\0';
				inputname = inpnameBuf;
			}
			CHKiRet(prop.Construct(&newlcnfinfo->pInputName));
			CHKiRet(prop.SetString(newlcnfinfo->pInputName,
				inputname, ustrlen(inputname)));
			CHKiRet(prop.ConstructFinalize(newlcnfinfo->pInputName));
			ratelimitSetLinuxLike(newlcnfinfo->ratelimiter, inst->ratelimitInterval,
					      inst->ratelimitBurst);
			ratelimitSetThreadSafe(newlcnfinfo->ratelimiter);
			/* support statistics gathering */
			CHKiRet(statsobj.Construct(&(newlcnfinfo->stats)));
			CHKiRet(statsobj.SetName(newlcnfinfo->stats, dispname));
			CHKiRet(statsobj.SetOrigin(newlcnfinfo->stats, (uchar*)"imudp"));
			STATSCOUNTER_INIT(newlcnfinfo->ctrSubmit, newlcnfinfo->mutCtrSubmit);
			CHKiRet(statsobj.AddCounter(newlcnfinfo->stats, UCHAR_CONSTANT("submitted"),
				ctrType_IntCtr, CTR_FLAG_RESETTABLE, &(newlcnfinfo->ctrSubmit)));
			STATSCOUNTER_INIT(newlcnfinfo->ctrDisallowed, newlcnfinfo->mutCtrDisallowed);
			CHKiRet(statsobj.AddCounter(newlcnfinfo->stats, UCHAR_CONSTANT("disallowed"),
				ctrType_IntCtr, CTR_FLAG_RESETTABLE, &(newlcnfinfo->ctrDisallowed)));
			CHKiRet(statsobj.ConstructFinalize(newlcnfinfo->stats));
			/* link to list. Order must be preserved to take care for
			 * conflicting matches.
			 */
			if(lcnfRoot == NULL)
				lcnfRoot = newlcnfinfo;
			if(lcnfLast == NULL)
				lcnfLast = newlcnfinfo;
			else {
				lcnfLast->next = newlcnfinfo;
				lcnfLast = newlcnfinfo;
			}
		}
	} else {
		LogError(0, NO_ERRCODE, "imudp: Could not create udp listener,"
				" ignoring port %s bind-address %s.",
				port, bindAddr);
	}

finalize_it:
	if(iRet != RS_RET_OK) {
		if(newlcnfinfo != NULL) {
			if(newlcnfinfo->ratelimiter != NULL)
				ratelimitDestruct(newlcnfinfo->ratelimiter);
			if(newlcnfinfo->pInputName != NULL)
				prop.Destruct(&newlcnfinfo->pInputName);
			if(newlcnfinfo->stats != NULL)
				statsobj.Destruct(&newlcnfinfo->stats);
			free(newlcnfinfo);
		}
		/* close the rest of the open sockets as there's
		   nowhere to put them */
		for(; iSrc <= newSocks[0]; iSrc++) {
			close(newSocks[iSrc]);
		}
	}

	free(newSocks);
	RETiRet;
}


static inline void
std_checkRuleset_genErrMsg(__attribute__((unused)) modConfData_t *modConf, instanceConf_t *inst)
{
	LogError(0, NO_ERRCODE, "imudp: ruleset '%s' for %s:%s not found - "
			"using default ruleset instead", inst->pszBindRuleset,
			inst->pszBindAddr == NULL ? "*" : (char*) inst->pszBindAddr,
			inst->pszBindPort);
}


/* This function processes received data. It provides unified handling
 * in cases where recvmmsg() is available and not.
 */
static rsRetVal
processPacket(struct lstn_s *lstn, struct sockaddr_storage *frominetPrev, int *pbIsPermitted,
	uchar *rcvBuf, ssize_t lenRcvBuf, struct syslogTime *stTime, time_t ttGenTime,
	struct sockaddr_storage *frominet, socklen_t socklen, multi_submit_t *multiSub)
{
	DEFiRet;
	smsg_t *pMsg = NULL;

	if(lenRcvBuf == 0)
		FINALIZE; /* this looks a bit strange, but practice shows it happens... */

	/* if we reach this point, we had a good receive and can process the packet received */
	/* check if we have a different sender than before, if so, we need to query some new values */
	if(bDoACLCheck) {
		socklen = sizeof(struct sockaddr_storage);
		if(net.CmpHost(frominet, frominetPrev, socklen) != 0) {
			memcpy(frominetPrev, frominet, socklen); /* update cache indicator */
			/* Here we check if a host is permitted to send us syslog messages. If it isn't,
			 * we do not further process the message but log a warning (if we are
			 * configured to do this). However, if the check would require name resolution,
			 * it is postponed to the main queue. See also my blog post at
			 * http://blog.gerhards.net/2009/11/acls-imudp-and-accepting-messages.html
			 * rgerhards, 2009-11-16
			 */
			*pbIsPermitted = net.isAllowedSender2((uchar*)"UDP",
					    (struct sockaddr *)frominet, "", 0);
	
			if(*pbIsPermitted == 0) {
				DBGPRINTF("msg is not from an allowed sender\n");
				STATSCOUNTER_INC(lstn->ctrDisallowed, lstn->mutCtrDisallowed);
				if(glbl.GetOption_DisallowWarning) {
					LogError(0, NO_ERRCODE,
						"imudp: UDP message from disallowed sender discarded");
				}
			}
		}
	} else {
		*pbIsPermitted = 1; /* no check -> everything permitted */
	}

	DBGPRINTF("recv(%d,%d),acl:%d,msg:%.*s\n", lstn->sock, (int) lenRcvBuf, *pbIsPermitted,
			(int)lenRcvBuf, rcvBuf);

	if(*pbIsPermitted != 0)  {
		/* we now create our own message object and submit it to the queue */
		CHKiRet(msgConstructWithTime(&pMsg, stTime, ttGenTime));
		MsgSetRawMsg(pMsg, (char*)rcvBuf, lenRcvBuf);
		MsgSetInputName(pMsg, lstn->pInputName);
		MsgSetRuleset(pMsg, lstn->pRuleset);
		MsgSetFlowControlType(pMsg, eFLOWCTL_NO_DELAY);
		if(lstn->dfltTZ != NULL)
			MsgSetDfltTZ(pMsg, (char*) lstn->dfltTZ);
		pMsg->msgFlags  = NEEDS_PARSING | PARSE_HOSTNAME | NEEDS_DNSRESOL;
		if(*pbIsPermitted == 2) {
			pMsg->msgFlags |= NEEDS_ACLCHK_U; /* request ACL check after resolution */
		}
		if(runModConf->bPreserveCase) {
			pMsg->msgFlags |= PRESERVE_CASE; /* preserve case of fromhost */
		}
		CHKiRet(msgSetFromSockinfo(pMsg, frominet));
		CHKiRet(ratelimitAddMsg(lstn->ratelimiter, multiSub, pMsg));
		STATSCOUNTER_INC(lstn->ctrSubmit, lstn->mutCtrSubmit);
	}

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pMsg != NULL && iRet != RS_RET_DISCARDMSG) {
			msgDestruct(&pMsg);
		}
	}

	RETiRet;
}




/* The following "two" functions are helpers to runInput. Actually, it is
 * just one function. Depending on whether or not we have recvmmsg(),
 * an appropriate version is compiled (as such we need to maintain both!).
 */
#ifdef HAVE_RECVMMSG
static rsRetVal
processSocket(struct wrkrInfo_s *pWrkr, struct lstn_s *lstn, struct sockaddr_storage *frominetPrev,
int *pbIsPermitted)
{
	DEFiRet;
	int iNbrTimeUsed;
	time_t ttGenTime = 0; /* to avoid clang static analyzer false positive */
		/* note: we do never use this time, because we always get a
		 * requery below on first loop iteration */
	struct syslogTime stTime;
	char errStr[1024];
	smsg_t *pMsgs[CONF_NUM_MULTISUB];
	multi_submit_t multiSub;
	int nelem;
	int i;

	multiSub.ppMsgs = pMsgs;
	multiSub.maxElem = CONF_NUM_MULTISUB;
	multiSub.nElem = 0;
	iNbrTimeUsed = 0;
	while(1) { /* loop is terminated if we have a "bad" receive, done below in the body */
		if(pWrkr->pThrd->bShallStop == RSTRUE)
			ABORT_FINALIZE(RS_RET_FORCE_TERM);
		memset(pWrkr->recvmsg_iov, 0, runModConf->batchSize * sizeof(struct iovec));
		memset(pWrkr->recvmsg_mmh, 0, runModConf->batchSize * sizeof(struct mmsghdr));
		for(i = 0 ; i < runModConf->batchSize ; ++i) {
			pWrkr->recvmsg_iov[i].iov_base = pWrkr->pRcvBuf+(i*(iMaxLine+1));
			pWrkr->recvmsg_iov[i].iov_len = iMaxLine;
			pWrkr->recvmsg_mmh[i].msg_hdr.msg_namelen = sizeof(struct sockaddr_storage);
			pWrkr->recvmsg_mmh[i].msg_hdr.msg_name = &(pWrkr->frominet[i]);
			pWrkr->recvmsg_mmh[i].msg_hdr.msg_iov = &(pWrkr->recvmsg_iov[i]);
			pWrkr->recvmsg_mmh[i].msg_hdr.msg_iovlen = 1;
		}
		nelem = recvmmsg(lstn->sock, pWrkr->recvmsg_mmh, runModConf->batchSize, 0, NULL);
		STATSCOUNTER_INC(pWrkr->ctrCall_recvmmsg, pWrkr->mutCtrCall_recvmmsg);
		DBGPRINTF("imudp: recvmmsg returned %d\n", nelem);
		if(nelem < 0 && errno == ENOSYS) {
			/* be careful: some versions of valgrind do not support recvmmsg()! */
			DBGPRINTF("imudp: error ENOSYS on call to recvmmsg() - fall back to recvmsg\n");
			nelem = recvmsg(lstn->sock, &(pWrkr->recvmsg_mmh[0].msg_hdr), 0);
			STATSCOUNTER_INC(pWrkr->ctrCall_recvmsg, pWrkr->mutCtrCall_recvmsg);
			if(nelem >= 0) {
				pWrkr->recvmsg_mmh[0].msg_len = nelem;
				nelem = 1;
			}
		}
		if(nelem < 0) {
			if(errno != EINTR && errno != EAGAIN) {
				rs_strerror_r(errno, errStr, sizeof(errStr));
				DBGPRINTF("INET socket error: %d = %s.\n", errno, errStr);
				LogError(errno, NO_ERRCODE, "imudp: error receiving on socket: %s", errStr);
			}
			ABORT_FINALIZE(RS_RET_ERR);
			// this most often is NOT an error, state is not checked by caller!
		}

		if((runModConf->iTimeRequery == 0) || (iNbrTimeUsed++ % runModConf->iTimeRequery) == 0) {
			datetime.getCurrTime(&stTime, &ttGenTime, TIME_IN_LOCALTIME);
		}

		pWrkr->ctrMsgsRcvd += nelem;
		for(i = 0 ; i < nelem ; ++i) {
			processPacket(lstn, frominetPrev, pbIsPermitted,
				pWrkr->recvmsg_mmh[i].msg_hdr.msg_iov->iov_base,
				pWrkr->recvmsg_mmh[i].msg_len, &stTime, ttGenTime, &(pWrkr->frominet[i]),
				pWrkr->recvmsg_mmh[i].msg_hdr.msg_namelen, &multiSub);
		}
	}

finalize_it:
	multiSubmitFlush(&multiSub);
	RETiRet;
}
#else /* we do not have recvmmsg() */
/* This function is a helper to runInput. I have extracted it
 * from the main loop just so that we do not have that large amount of code
 * in a single place. This function takes a socket and pulls messages from
 * it until the socket does not have any more waiting.
 * rgerhards, 2008-01-08
 * We try to read from the file descriptor until there
 * is no more data. This is done in the hope to get better performance
 * out of the system. However, this also means that a descriptor
 * monopolizes processing while it contains data. This can lead to
 * data loss in other descriptors. However, if the system is incapable of
 * handling the workload, we will loss data in any case. So it doesn't really
 * matter where the actual loss occurs - it is always random, because we depend
 * on scheduling order. -- rgerhards, 2008-10-02
 */
static rsRetVal
processSocket(struct wrkrInfo_s *pWrkr, struct lstn_s *lstn, struct sockaddr_storage *frominetPrev,
int *pbIsPermitted)
{
	int iNbrTimeUsed;
	time_t ttGenTime;
	struct syslogTime stTime;
	ssize_t lenRcvBuf;
	struct sockaddr_storage frominet;
	multi_submit_t multiSub;
	smsg_t *pMsgs[CONF_NUM_MULTISUB];
	char errStr[1024];
	struct msghdr mh;
	struct iovec iov[1];
	DEFiRet;

	multiSub.ppMsgs = pMsgs;
	multiSub.maxElem = CONF_NUM_MULTISUB;
	multiSub.nElem = 0;
	iNbrTimeUsed = 0;
	while(1) { /* loop is terminated if we have a bad receive, done below in the body */
		if(pWrkr->pThrd->bShallStop == RSTRUE)
			ABORT_FINALIZE(RS_RET_FORCE_TERM);
		memset(iov, 0, sizeof(iov));
		iov[0].iov_base = pWrkr->pRcvBuf;
		iov[0].iov_len = iMaxLine;
		memset(&mh, 0, sizeof(mh));
		mh.msg_name = &frominet;
		mh.msg_namelen = sizeof(struct sockaddr_storage);
		mh.msg_iov = iov;
		mh.msg_iovlen = 1;
		lenRcvBuf = recvmsg(lstn->sock, &mh, 0);
		STATSCOUNTER_INC(pWrkr->ctrCall_recvmsg, pWrkr->mutCtrCall_recvmsg);
		if(lenRcvBuf < 0) {
			if(errno != EINTR && errno != EAGAIN) {
				rs_strerror_r(errno, errStr, sizeof(errStr));
				DBGPRINTF("INET socket error: %d = %s.\n", errno, errStr);
				LogError(errno, NO_ERRCODE, "imudp: error receiving on socket: %s", errStr);
			}
			ABORT_FINALIZE(RS_RET_ERR);
			// this most often is NOT an error, state is not checked by caller!
		}

		++pWrkr->ctrMsgsRcvd;
		if((runModConf->iTimeRequery == 0) || (iNbrTimeUsed++ % runModConf->iTimeRequery) == 0) {
			datetime.getCurrTime(&stTime, &ttGenTime, TIME_IN_LOCALTIME);
		}

		CHKiRet(processPacket(lstn, frominetPrev, pbIsPermitted, pWrkr->pRcvBuf, lenRcvBuf, &stTime,
			ttGenTime, &frominet, mh.msg_namelen, &multiSub));
	}


finalize_it:
	multiSubmitFlush(&multiSub);
	RETiRet;
}
#endif /* #ifdef HAVE_RECVMMSG */


/* check configured scheduling priority.
 * Precondition: iSchedPolicy must have been set
 */
static rsRetVal
checkSchedulingPriority(modConfData_t *modConf)
{
	DEFiRet;

#ifdef HAVE_SCHED_GET_PRIORITY_MAX
	if(   modConf->iSchedPrio < sched_get_priority_min(modConf->iSchedPolicy)
	   || modConf->iSchedPrio > sched_get_priority_max(modConf->iSchedPolicy)) {
		LogError(0, NO_ERRCODE,
			"imudp: scheduling priority %d out of range (%d - %d)"
			" for scheduling policy '%s' - ignoring settings",
			modConf->iSchedPrio,
			sched_get_priority_min(modConf->iSchedPolicy),
			sched_get_priority_max(modConf->iSchedPolicy),
			modConf->pszSchedPolicy);
		ABORT_FINALIZE(RS_RET_VALIDATION_RUN);
	}
finalize_it:
#endif
	RETiRet;
}


/* check scheduling policy string and, if valid, set its
 * numeric equivalent in current load config
 */
static rsRetVal
checkSchedulingPolicy(modConfData_t *modConf)
{
	DEFiRet;

	if (0) { /* trick to use conditional compilation */
#ifdef SCHED_FIFO
	} else if (!strcasecmp((char*)modConf->pszSchedPolicy, "fifo")) {
		modConf->iSchedPolicy = SCHED_FIFO;
#endif
#ifdef SCHED_RR
	} else if (!strcasecmp((char*)modConf->pszSchedPolicy, "rr")) {
		modConf->iSchedPolicy = SCHED_RR;
#endif
#ifdef SCHED_OTHER
	} else if (!strcasecmp((char*)modConf->pszSchedPolicy, "other")) {
		modConf->iSchedPolicy = SCHED_OTHER;
#endif
	} else {
		LogError(errno, NO_ERRCODE,
			    "imudp: invalid scheduling policy '%s' "
			    "- ignoring setting", modConf->pszSchedPolicy);
		ABORT_FINALIZE(RS_RET_ERR_SCHED_PARAMS);
	}
finalize_it:
	RETiRet;
}

/* checks scheduling parameters during config check phase */
static rsRetVal
checkSchedParam(modConfData_t *modConf)
{
	DEFiRet;

	if(modConf->pszSchedPolicy != NULL && modConf->iSchedPrio == SCHED_PRIO_UNSET) {
		LogError(0, RS_RET_ERR_SCHED_PARAMS,
			"imudp: scheduling policy set, but without priority - ignoring settings");
		ABORT_FINALIZE(RS_RET_ERR_SCHED_PARAMS);
	} else if(modConf->pszSchedPolicy == NULL && modConf->iSchedPrio != SCHED_PRIO_UNSET) {
		LogError(0, RS_RET_ERR_SCHED_PARAMS,
			"imudp: scheduling priority set, but without policy - ignoring settings");
		ABORT_FINALIZE(RS_RET_ERR_SCHED_PARAMS);
	} else if(modConf->pszSchedPolicy != NULL && modConf->iSchedPrio != SCHED_PRIO_UNSET) {
		/* we have parameters set, so check them */
		CHKiRet(checkSchedulingPolicy(modConf));
		CHKiRet(checkSchedulingPriority(modConf));
	} else { /* nothing set */
		modConf->iSchedPrio = SCHED_PRIO_UNSET; /* prevents doing the activation call */
	}
#ifndef HAVE_PTHREAD_SETSCHEDPARAM
	LogError(0, NO_ERRCODE,
		"imudp: cannot set thread scheduling policy, "
		"pthread_setschedparam() not available");
	ABORT_FINALIZE(RS_RET_ERR_SCHED_PARAMS);
#endif

finalize_it:
	if(iRet != RS_RET_OK)
		modConf->iSchedPrio = SCHED_PRIO_UNSET; /* prevents doing the activation call */

	RETiRet;
}

/* set the configured scheduling policy (if possible) */
static rsRetVal
setSchedParams(modConfData_t *modConf)
{
	DEFiRet;

#	ifdef HAVE_PTHREAD_SETSCHEDPARAM
	int err;
	struct sched_param sparam;

	if(modConf->iSchedPrio == SCHED_PRIO_UNSET)
		FINALIZE;

	memset(&sparam, 0, sizeof sparam);
	sparam.sched_priority = modConf->iSchedPrio;
	dbgprintf("imudp trying to set sched policy to '%s', prio %d\n",
		  modConf->pszSchedPolicy, modConf->iSchedPrio);
	err = pthread_setschedparam(pthread_self(), modConf->iSchedPolicy, &sparam);
	if(err != 0) {
		LogError(err, NO_ERRCODE, "imudp: pthread_setschedparam() failed - ignoring");
	}
finalize_it:
#	endif

	RETiRet;
}


/* This function implements the main reception loop. Depending on the environment,
 * we either use the traditional (but slower) select() or the Linux-specific epoll()
 * interface. ./configure settings control which one is used.
 * rgerhards, 2009-09-09
 */
#if defined(HAVE_EPOLL_CREATE1) || defined(HAVE_EPOLL_CREATE)
#define NUM_EPOLL_EVENTS 10
static rsRetVal
rcvMainLoop(struct wrkrInfo_s *const __restrict__ pWrkr)
{
	DEFiRet;
	int nfds;
	int efd;
	int i;
	struct sockaddr_storage frominetPrev;
	int bIsPermitted;
	struct epoll_event *udpEPollEvt = NULL;
	struct epoll_event currEvt[NUM_EPOLL_EVENTS];
	char errStr[1024];
	struct lstn_s *lstn;
	int nLstn;

	/* start "name caching" algo by making sure the previous system indicator
	 * is invalidated.
	 */
	bIsPermitted = 0;
	memset(&frominetPrev, 0, sizeof(frominetPrev));

	/* count num listeners -- do it here in order to avoid inconsistency */
	nLstn = 0;
	for(lstn = lcnfRoot ; lstn != NULL ; lstn = lstn->next)
		++nLstn;

	if(nLstn == 0) {
		LogError(errno, RS_RET_ERR,
			"imudp error: we have 0 listeners, terminating"
			"worker thread");
		ABORT_FINALIZE(RS_RET_ERR);
	}
	CHKmalloc(udpEPollEvt = calloc(nLstn, sizeof(struct epoll_event)));

#if defined(EPOLL_CLOEXEC) && defined(HAVE_EPOLL_CREATE1)
	DBGPRINTF("imudp uses epoll_create1()\n");
	efd = epoll_create1(EPOLL_CLOEXEC);
	if(efd < 0 && errno == ENOSYS)
#endif
	{
		DBGPRINTF("imudp uses epoll_create()\n");
		efd = epoll_create(NUM_EPOLL_EVENTS);
	}

	if(efd < 0) {
		DBGPRINTF("epoll_create1() could not create fd\n");
		ABORT_FINALIZE(RS_RET_IO_ERROR);
	}

	/* fill the epoll set - we need to do this only once, as the set
	 * can not change dyamically.
	 */
	i = 0;
	for(lstn = lcnfRoot ; lstn != NULL ; lstn = lstn->next) {
		if(lstn->sock != -1) {
			udpEPollEvt[i].events = EPOLLIN | EPOLLET;
			udpEPollEvt[i].data.ptr = lstn;
			if(epoll_ctl(efd, EPOLL_CTL_ADD,  lstn->sock, &(udpEPollEvt[i])) < 0) {
				rs_strerror_r(errno, errStr, sizeof(errStr));
				LogError(errno, NO_ERRCODE, "epoll_ctrl failed on fd %d with %s\n",
					lstn->sock, errStr);
			}
		}
		i++;
	}

	while(1) {
		/* wait for io to become ready */
		nfds = epoll_wait(efd, currEvt, NUM_EPOLL_EVENTS, -1);
		DBGPRINTF("imudp: epoll_wait() returned with %d fds\n", nfds);

		if(pWrkr->pThrd->bShallStop == RSTRUE)
			break; /* terminate input! */

		for(i = 0 ; i < nfds ; ++i) {
			processSocket(pWrkr, currEvt[i].data.ptr, &frominetPrev, &bIsPermitted);
		}
		if(pWrkr->pThrd->bShallStop == RSTRUE)
			break; /* terminate input! */
	}

finalize_it:
	if(udpEPollEvt != NULL)
		free(udpEPollEvt);

	RETiRet;
}
#else /* #if HAVE_EPOLL_CREATE1 */
/* this is the code for the select() interface */
static rsRetVal ATTR_NONNULL()
rcvMainLoop(struct wrkrInfo_s *const __restrict__ pWrkr)
{
	DEFiRet;
	int nfds;
	struct sockaddr_storage frominetPrev;
	int bIsPermitted;
	int i = 0;
	struct lstn_s *lstn;

	DBGPRINTF("imudp uses poll() [ex-select]\n");
	/* start "name caching" algo by making sure the previous system indicator
	 * is invalidated. */
	bIsPermitted = 0;
	memset(&frominetPrev, 0, sizeof(frominetPrev));

	/* setup poll() subsystem */
	int nfd = 0;
	for(lstn = lcnfRoot ; lstn != NULL ; lstn = lstn->next) {
		if(lstn->sock != -1) {
			if(Debug) {
				net.debugListenInfo(lstn->sock, (char*)"UDP");
			}
			++nfd;
		}
	}
	struct pollfd *const pollfds = calloc(nfd, sizeof(struct pollfd));
	CHKmalloc(pollfds);

	for(lstn = lcnfRoot ; lstn != NULL ; lstn = lstn->next) {
		assert(i < nfd);
		if (lstn->sock != -1) {
			pollfds[i].fd = lstn->sock;
			pollfds[i].events = POLLIN;
			++i;
		}
	}

	while(1) {
		DBGPRINTF("--------imudp calling poll() on %d fds\n", nfd);
		nfds = poll(pollfds, nfd, -1);
		if(glbl.GetGlobalInputTermState() == 1)
			break; /* terminate input! */

		if(nfds < 0) {
			if(errno == EINTR) {
				DBGPRINTF("imudp: EINTR occured\n");
			} else {
				LogMsg(errno, RS_RET_POLL_ERR, LOG_WARNING, "imudp: poll "
					"system call failed, may cause further troubles");
			}
			nfds = 0;
		}

		i = 0;
		for(lstn = lcnfRoot ; nfds && lstn != NULL ; lstn = lstn->next) {
			assert(i < nfd);
			if(glbl.GetGlobalInputTermState() == 1)
				ABORT_FINALIZE(RS_RET_FORCE_TERM); /* terminate input! */
			if(pollfds[i].revents & POLLIN) {
		       		processSocket(pWrkr, lstn, &frominetPrev, &bIsPermitted);
				--nfds;
			}
			++i;
	       }
	       /* end of a run, back to loop for next recv() */
	}

finalize_it:
	RETiRet;
}
#endif /* #if HAVE_EPOLL_CREATE1 */


static rsRetVal
createListner(es_str_t *port, struct cnfparamvals *pvals)
{
	instanceConf_t *inst;
	int i;
	int bAppendPortUsed = 0;
	DEFiRet;

	CHKiRet(createInstance(&inst));
	inst->pszBindPort = (uchar*)es_str2cstr(port, NULL);
	for(i = 0 ; i < inppblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(inppblk.descr[i].name, "port")) {
			continue;	/* array, handled by caller */
		} else if(!strcmp(inppblk.descr[i].name, "name")) {
			if(inst->inputname != NULL) {
				LogError(0, RS_RET_INVALID_PARAMS, "imudp: name and inputname "
						"parameter specified - only one can be used");
				ABORT_FINALIZE(RS_RET_INVALID_PARAMS);
			}
			inst->inputname = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "name.appendport")) {
			if(bAppendPortUsed) {
				LogError(0, RS_RET_INVALID_PARAMS, "imudp: name.appendport and "
						"inputname.appendport parameter specified - only one can be used");
				ABORT_FINALIZE(RS_RET_INVALID_PARAMS);
			}
			inst->bAppendPortToInpname = (int) pvals[i].val.d.n;
			bAppendPortUsed = 1;
		} else if(!strcmp(inppblk.descr[i].name, "inputname")) {
			LogError(0, RS_RET_DEPRECATED , "imudp: deprecated parameter inputname "
					"used. Suggest to use name instead");
			if(inst->inputname != NULL) {
				LogError(0, RS_RET_INVALID_PARAMS, "imudp: name and inputname "
						"parameter specified - only one can be used");
				ABORT_FINALIZE(RS_RET_INVALID_PARAMS);
			}
			inst->inputname = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "inputname.appendport")) {
			LogError(0, RS_RET_DEPRECATED , "imudp: deprecated parameter inputname.appendport "
					"used. Suggest to use name.appendport instead");
			if(bAppendPortUsed) {
				LogError(0, RS_RET_INVALID_PARAMS, "imudp: name.appendport and "
						"inputname.appendport parameter specified - only one can be used");
				ABORT_FINALIZE(RS_RET_INVALID_PARAMS);
			}
			bAppendPortUsed = 1;
			inst->bAppendPortToInpname = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "defaulttz")) {
			inst->dfltTZ = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "address")) {
			inst->pszBindAddr = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "device")) {
			inst->pszBindDevice = (char*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "ruleset")) {
			inst->pszBindRuleset = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "ratelimit.burst")) {
			inst->ratelimitBurst = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "ratelimit.interval")) {
			inst->ratelimitInterval = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "rcvbufsize")) {
			const uint64_t val = pvals[i].val.d.n;
			if(val > 1024 * 1024 * 1024) {
				LogError(0, RS_RET_MISSING_CNFPARAMS,
					"imudp: rcvbufsize maximum is 1 GiB, using "
					"default instead");
			} else {
				inst->rcvbuf = (int) val;
			}
		} else if(!strcmp(inppblk.descr[i].name, "ipfreebind")) {
			inst->ipfreebind = (int) pvals[i].val.d.n;
		} else {
			dbgprintf("imudp: program error, non-handled "
			  "param '%s'\n", inppblk.descr[i].name);
		}
	}
finalize_it:
	RETiRet;
}


BEGINnewInpInst
	struct cnfparamvals *pvals;
	int i;
	int portIdx;
CODESTARTnewInpInst
	DBGPRINTF("newInpInst (imudp)\n");

	if((pvals = nvlstGetParams(lst, &inppblk, NULL)) == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}
	if(Debug) {
		dbgprintf("input param blk in imudp:\n");
		cnfparamsPrint(&inppblk, pvals);
	}

	portIdx = cnfparamGetIdx(&inppblk, "port");
	assert(portIdx != -1);
	for(i = 0 ; i <  pvals[portIdx].val.d.ar->nmemb ; ++i) {
		createListner(pvals[portIdx].val.d.ar->arr[i], pvals);
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
	loadModConf->configSetViaV2Method = 0;
	loadModConf->wrkrMax = 1; /* conservative, but least msg reordering */
	loadModConf->batchSize = BATCH_SIZE_DFLT;
	loadModConf->iTimeRequery = TIME_REQUERY_DFLT;
	loadModConf->iSchedPrio = SCHED_PRIO_UNSET;
	loadModConf->pszSchedPolicy = NULL;
	loadModConf->bPreserveCase = 0; /* off */
	bLegacyCnfModGlobalsPermitted = 1;
	/* init legacy config vars */
	cs.pszBindRuleset = NULL;
	cs.pszSchedPolicy = NULL;
	cs.pszBindAddr = NULL;
	cs.pszBindDevice = NULL;
	cs.iSchedPrio = SCHED_PRIO_UNSET;
	cs.iTimeRequery = TIME_REQUERY_DFLT;
ENDbeginCnfLoad


BEGINsetModCnf
	struct cnfparamvals *pvals = NULL;
	int i;
	int wrkrMax;
CODESTARTsetModCnf
	pvals = nvlstGetParams(lst, &modpblk, NULL);
	if(pvals == NULL) {
		LogError(0, RS_RET_MISSING_CNFPARAMS, "imudp: error processing module "
				"config parameters [module(...)]");
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	if(Debug) {
		dbgprintf("module (global) param blk for imudp:\n");
		cnfparamsPrint(&modpblk, pvals);
	}

	for(i = 0 ; i < modpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(modpblk.descr[i].name, "timerequery")) {
			loadModConf->iTimeRequery = (int) pvals[i].val.d.n;
		} else if(!strcmp(modpblk.descr[i].name, "batchsize")) {
			loadModConf->batchSize = (int) pvals[i].val.d.n;
		} else if(!strcmp(modpblk.descr[i].name, "schedulingpriority")) {
			loadModConf->iSchedPrio = (int) pvals[i].val.d.n;
		} else if(!strcmp(modpblk.descr[i].name, "schedulingpolicy")) {
			loadModConf->pszSchedPolicy = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(modpblk.descr[i].name, "threads")) {
			wrkrMax = (int) pvals[i].val.d.n;
			if(wrkrMax > MAX_WRKR_THREADS) {
				LogError(0, RS_RET_PARAM_ERROR, "imudp: configured for %d"
						"worker threads, but maximum permitted is %d",
						wrkrMax, MAX_WRKR_THREADS);
				loadModConf->wrkrMax = MAX_WRKR_THREADS;
			} else {
				loadModConf->wrkrMax = wrkrMax;
			}
		} else if(!strcmp(modpblk.descr[i].name, "preservecase")) {
			loadModConf->bPreserveCase = (int) pvals[i].val.d.n;
		} else {
			dbgprintf("imudp: program error, non-handled "
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
		loadModConf->iSchedPrio = cs.iSchedPrio;
		loadModConf->iTimeRequery = cs.iTimeRequery;
		if((cs.pszSchedPolicy != NULL) && (cs.pszSchedPolicy[0] != '\0')) {
			CHKmalloc(loadModConf->pszSchedPolicy = ustrdup(cs.pszSchedPolicy));
		}
	}

finalize_it:
	loadModConf = NULL; /* done loading */
	/* free legacy config vars */
	free(cs.pszBindRuleset);
	free(cs.pszSchedPolicy);
	free(cs.pszBindAddr);
	free(cs.pszBindDevice);
ENDendCnfLoad


BEGINcheckCnf
	instanceConf_t *inst;
CODESTARTcheckCnf
	checkSchedParam(pModConf); /* this can not cause fatal errors */
	for(inst = pModConf->root ; inst != NULL ; inst = inst->next) {
		std_checkRuleset(pModConf, inst);
	}
	if(pModConf->root == NULL) {
		LogError(0, RS_RET_NO_LISTNERS , "imudp: module loaded, but "
				"no listeners defined - no input will be gathered");
		iRet = RS_RET_NO_LISTNERS;
	}
ENDcheckCnf


BEGINactivateCnfPrePrivDrop
	instanceConf_t *inst;
CODESTARTactivateCnfPrePrivDrop
	runModConf = pModConf;
	for(inst = runModConf->root ; inst != NULL ; inst = inst->next) {
		addListner(inst);
	}
	/* if we could not set up any listeners, there is no point in running... */
	if(lcnfRoot == NULL) {
		LogError(0, NO_ERRCODE, "imudp: no listeners could be started, "
				"input not activated.\n");
		ABORT_FINALIZE(RS_RET_NO_RUN);
	}

finalize_it:
ENDactivateCnfPrePrivDrop


BEGINactivateCnf
	int i;
	int lenRcvBuf;
CODESTARTactivateCnf
	/* caching various settings */
	iMaxLine = glbl.GetMaxLine();
	lenRcvBuf = iMaxLine + 1;
#	ifdef HAVE_RECVMMSG
	lenRcvBuf *= runModConf->batchSize;
#	endif
	DBGPRINTF("imudp: config params iMaxLine %d, lenRcvBuf %d\n", iMaxLine, lenRcvBuf);
	for(i = 0 ; i < runModConf->wrkrMax ; ++i) {
#		ifdef HAVE_RECVMMSG
		CHKmalloc(wrkrInfo[i].recvmsg_iov = MALLOC(runModConf->batchSize * sizeof(struct iovec)));
		CHKmalloc(wrkrInfo[i].recvmsg_mmh = MALLOC(runModConf->batchSize * sizeof(struct mmsghdr)));
		CHKmalloc(wrkrInfo[i].frominet = MALLOC(runModConf->batchSize * sizeof(struct sockaddr_storage)));
#		endif
		CHKmalloc(wrkrInfo[i].pRcvBuf = MALLOC(lenRcvBuf));
		wrkrInfo[i].id = i;
	}
finalize_it:
ENDactivateCnf


BEGINfreeCnf
	instanceConf_t *inst, *del;
CODESTARTfreeCnf
	for(inst = pModConf->root ; inst != NULL ; ) {
		free(inst->pszBindPort);
		free(inst->pszBindAddr);
		free(inst->pszBindDevice);
		free(inst->inputname);
		free(inst->dfltTZ);
		del = inst;
		inst = inst->next;
		free(del);
	}
ENDfreeCnf


static void *
wrkr(void *myself)
{
	struct wrkrInfo_s *pWrkr = (struct wrkrInfo_s*) myself;
#	if defined(HAVE_PRCTL) && defined(PR_SET_NAME)
	uchar *pszDbgHdr;
#	endif
	uchar thrdName[32];

	snprintf((char*)thrdName, sizeof(thrdName), "imudp(w%d)", pWrkr->id);
#	if defined(HAVE_PRCTL) && defined(PR_SET_NAME)
	/* set thread name - we ignore if the call fails, has no harsh consequences... */
	if(prctl(PR_SET_NAME, thrdName, 0, 0, 0) != 0) {
		DBGPRINTF("prctl failed, not setting thread name for '%s'\n", thrdName);
	}
#	endif
	dbgOutputTID((char*)thrdName);

	/* Note well: the setting of scheduling parameters will not work
	 * when we dropped privileges (if the user is not sufficiently
	 * privileged, of course). Howerver, we can't change the
	 * scheduling params in PrePrivDrop(), as at that point our thread
	 * is not yet created. So at least as an interim solution, we do
	 * NOT support both setting sched parameters and dropping
	 * privileges within the same instance.
	 */
	setSchedParams(runModConf);

	/* support statistics gathering */
	statsobj.Construct(&(pWrkr->stats));
	statsobj.SetName(pWrkr->stats, thrdName);
	statsobj.SetOrigin(pWrkr->stats, (uchar*)"imudp");
	STATSCOUNTER_INIT(pWrkr->ctrCall_recvmmsg, pWrkr->mutCtrCall_recvmmsg);
	statsobj.AddCounter(pWrkr->stats, UCHAR_CONSTANT("called.recvmmsg"),
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &(pWrkr->ctrCall_recvmmsg));
	STATSCOUNTER_INIT(pWrkr->ctrCall_recvmsg, pWrkr->mutCtrCall_recvmsg);
	statsobj.AddCounter(pWrkr->stats, UCHAR_CONSTANT("called.recvmsg"),
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &(pWrkr->ctrCall_recvmsg));
	STATSCOUNTER_INIT(pWrkr->ctrMsgsRcvd, pWrkr->mutCtrMsgsRcvd);
	statsobj.AddCounter(pWrkr->stats, UCHAR_CONSTANT("msgs.received"),
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &(pWrkr->ctrMsgsRcvd));
	statsobj.ConstructFinalize(pWrkr->stats);

	rcvMainLoop(pWrkr);

	/* cleanup */
	return NULL;
}

/* This function is called to gather input.
 * In essence, it just starts the pool of workers. To save resources,
 * we run one of the workers on our own thread -- otherwise that thread would
 * just idle around and wait for the workers to finish.
 */
BEGINrunInput
	int i;
	pthread_attr_t wrkrThrdAttr;
CODESTARTrunInput
	pthread_attr_init(&wrkrThrdAttr);
	pthread_attr_setstacksize(&wrkrThrdAttr, 4096*1024);
	for(i = 0 ; i < runModConf->wrkrMax - 1 ; ++i) {
		wrkrInfo[i].pThrd = pThrd;
		pthread_create(&wrkrInfo[i].tid, &wrkrThrdAttr, wrkr, &(wrkrInfo[i]));
	}
	pthread_attr_destroy(&wrkrThrdAttr);

	wrkrInfo[i].pThrd = pThrd;
	wrkrInfo[i].id = i;
	wrkr(&wrkrInfo[i]);

	for(i = 0 ; i < runModConf->wrkrMax - 1 ; ++i) {
		pthread_kill(wrkrInfo[i].tid, SIGTTIN);
	}
	for(i = 0 ; i < runModConf->wrkrMax - 1 ; ++i) {
		pthread_join(wrkrInfo[i].tid, NULL);
	}
ENDrunInput


/* initialize and return if will run or not */
BEGINwillRun
CODESTARTwillRun
	net.PrintAllowedSenders(1); /* UDP */
	net.HasRestrictions(UCHAR_CONSTANT("UDP"), &bDoACLCheck); /* UDP */
ENDwillRun


BEGINafterRun
	struct lstn_s *lstn, *lstnDel;
	int i;
CODESTARTafterRun
	/* do cleanup here */
	net.clearAllowedSenders((uchar*)"UDP");
	for(lstn = lcnfRoot ; lstn != NULL ; ) {
		statsobj.Destruct(&(lstn->stats));
		ratelimitDestruct(lstn->ratelimiter);
		close(lstn->sock);
		prop.Destruct(&lstn->pInputName);
		lstnDel = lstn;
		lstn = lstn->next;
		free(lstnDel);
	}
	lcnfRoot = lcnfLast = NULL;
	for(i = 0 ; i < runModConf->wrkrMax ; ++i) {
#		ifdef HAVE_RECVMMSG
		free(wrkrInfo[i].recvmsg_iov);
		free(wrkrInfo[i].recvmsg_mmh);
		free(wrkrInfo[i].frominet);
#		endif
		free(wrkrInfo[i].pRcvBuf);
	}
ENDafterRun


BEGINmodExit
CODESTARTmodExit
	/* release what we no longer need */
	objRelease(glbl, CORE_COMPONENT);
	objRelease(statsobj, CORE_COMPONENT);
	objRelease(datetime, CORE_COMPONENT);
	objRelease(prop, CORE_COMPONENT);
	objRelease(ruleset, CORE_COMPONENT);
	objRelease(net, LM_NET_FILENAME);
ENDmodExit


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

static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal)
{
	free(cs.pszBindAddr);
	cs.pszBindAddr = NULL;
	free(cs.pszBindDevice);
	cs.pszBindDevice = NULL;
	free(cs.pszSchedPolicy);
	cs.pszSchedPolicy = NULL;
	free(cs.pszBindRuleset);
	cs.pszBindRuleset = NULL;
	cs.iSchedPrio = SCHED_PRIO_UNSET;
	cs.iTimeRequery = TIME_REQUERY_DFLT;/* the default is to query only every second time */
	return RS_RET_OK;
}


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(statsobj, CORE_COMPONENT));
	CHKiRet(objUse(datetime, CORE_COMPONENT));
	CHKiRet(objUse(prop, CORE_COMPONENT));
	CHKiRet(objUse(ruleset, CORE_COMPONENT));
	CHKiRet(objUse(net, LM_NET_FILENAME));

	DBGPRINTF("imudp: version %s initializing\n", VERSION);
#	ifdef HAVE_RECVMMSG
	DBGPRINTF("imdup: support for recvmmsg() present\n");
#	endif

	/* register config file handlers */
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputudpserverbindruleset", 0, eCmdHdlrGetWord,
		NULL, &cs.pszBindRuleset, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"udpserverrun", 0, eCmdHdlrGetWord,
		addInstance, NULL, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"udpserveraddress", 0, eCmdHdlrGetWord,
		NULL, &cs.pszBindAddr, STD_LOADABLE_MODULE_ID));
	/* module-global config params - will be disabled in configs that are loaded
	 * via module(...).
	 */
	CHKiRet(regCfSysLineHdlr2((uchar *)"imudpschedulingpolicy", 0, eCmdHdlrGetWord,
		NULL, &cs.pszSchedPolicy, STD_LOADABLE_MODULE_ID, &bLegacyCnfModGlobalsPermitted));
	CHKiRet(regCfSysLineHdlr2((uchar *)"imudpschedulingpriority", 0, eCmdHdlrInt,
		NULL, &cs.iSchedPrio, STD_LOADABLE_MODULE_ID, &bLegacyCnfModGlobalsPermitted));
	CHKiRet(regCfSysLineHdlr2((uchar *)"udpservertimerequery", 0, eCmdHdlrInt,
		NULL, &cs.iTimeRequery, STD_LOADABLE_MODULE_ID, &bLegacyCnfModGlobalsPermitted));

	CHKiRet(omsdRegCFSLineHdlr((uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler,
		resetConfigVariables, NULL, STD_LOADABLE_MODULE_ID));
ENDmodInit
/* vim:set ai:
 */
