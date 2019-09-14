/* imuxsock.c
 * This is the implementation of the Unix sockets input module.
 *
 * NOTE: read comments in module-template.h to understand how this file
 *       works!
 *
 * File begun on 2007-12-20 by RGerhards (extracted from syslogd.c)
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
#ifdef __sun
#define _XPG4_2
#endif
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/socket.h>
#ifdef HAVE_LIBSYSTEMD
#	include <systemd/sd-daemon.h>
#endif
#include "rsyslog.h"
#include "dirty.h"
#include "cfsysline.h"
#include "unicode-helper.h"
#include "module-template.h"
#include "srUtils.h"
#include "errmsg.h"
#include "net.h"
#include "glbl.h"
#include "msg.h"
#include "parser.h"
#include "prop.h"
#include "debug.h"
#include "ruleset.h"
#include "unlimited_select.h"
#include "statsobj.h"
#include "datetime.h"
#include "hashtable.h"
#include "ratelimit.h"

#if !defined(_AIX)
#pragma GCC diagnostic ignored "-Wswitch-enum"
#endif

MODULE_TYPE_INPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("imuxsock")

/* defines */
#ifndef _PATH_LOG
#ifdef BSD
#define _PATH_LOG	"/var/run/log"
#else
#define _PATH_LOG	"/dev/log"
#endif
#endif
#ifndef SYSTEMD_JOURNAL
#define SYSTEMD_JOURNAL  "/run/systemd/journal"
#endif
#ifndef SYSTEMD_PATH_LOG
#define SYSTEMD_PATH_LOG SYSTEMD_JOURNAL "/syslog"
#endif
#define UNSET -1 /* to indicate a value has not been configured */

/* forward definitions */
static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal);

#if defined(_AIX)
#define ucred  ucred_t
#endif
/* emulate struct ucred for platforms that do not have it */
#ifndef HAVE_SCM_CREDENTIALS
struct ucred { int pid; uid_t uid; gid_t gid; };
#endif

/* handle some defines missing on more than one platform */
#ifndef SUN_LEN
#define SUN_LEN(su) \
	(sizeof(*(su)) - sizeof((su)->sun_path) + strlen((su)->sun_path))
#endif
/* Module static data */
DEF_IMOD_STATIC_DATA
DEFobjCurrIf(glbl)
DEFobjCurrIf(prop)
DEFobjCurrIf(net)
DEFobjCurrIf(parser)
DEFobjCurrIf(datetime)
DEFobjCurrIf(statsobj)
DEFobjCurrIf(ruleset)


statsobj_t *modStats;
STATSCOUNTER_DEF(ctrSubmit, mutCtrSubmit)
STATSCOUNTER_DEF(ctrLostRatelimit, mutCtrLostRatelimit)
STATSCOUNTER_DEF(ctrNumRatelimiters, mutCtrNumRatelimiters)


/* a very simple "hash function" for process IDs - we simply use the
 * pid itself: it is quite expected that all pids may log some time, but
 * from a collision point of view it is likely that long-running daemons
 * start early and so will stay right in the top spots of the
 * collision list.
 */
static unsigned int
hash_from_key_fn(void *k)
{
	return((unsigned) *((pid_t*) k));
}

static int
key_equals_fn(void *key1, void *key2)
{
	return *((pid_t*) key1) == *((pid_t*) key2);
}


/* structure to describe a specific listener */
typedef struct lstn_s {
	uchar *sockName;	/* read-only after startup */
	prop_t *hostName;	/* host-name override - if set, use this instead of actual name */
	int fd;			/* read-only after startup */
	int flags;		/* should parser parse host name?  read-only after startup */
	int flowCtl;		/* flow control settings for this socket */
	int ratelimitInterval;
	int ratelimitBurst;
	ratelimit_t *dflt_ratelimiter;/*ratelimiter to apply if none else is to be used */
	intTiny ratelimitSev;	/* severity level (and below) for which rate-limiting shall apply */
	struct hashtable *ht;	/* our hashtable for rate-limiting */
	sbool bParseHost;	/* should parser parse host name?  read-only after startup */
	sbool bCreatePath;	/* auto-creation of socket directory? */
	sbool bUseCreds;	/* pull original creator credentials from socket */
	sbool bAnnotate;	/* annotate events with trusted properties */
	sbool bParseTrusted;	/* parse trusted properties */
	sbool bWritePid;	/* write original PID into tag */
	sbool bDiscardOwnMsgs;	/* discard messages that originated from ourselves */
	sbool bUseSysTimeStamp;	/* use timestamp from system (instead of from message) */
	sbool bUnlink;		/* unlink&re-create socket at start and end of processing */
	sbool bUseSpecialParser;/* use "canned" log socket parser instead of parser chain? */
	ruleset_t *pRuleset;
} lstn_t;
static lstn_t *listeners;

static prop_t *pLocalHostIP = NULL;	/* there is only one global IP for all internally-generated messages */
static prop_t *pInputName = NULL;	/* our inputName currently is always "imuxsock", and this will hold it */
static int startIndexUxLocalSockets; /* process fd from that index on (used to
				* suppress local logging. rgerhards 2005-08-01
				* read-only after startup
				*/
static int nfd = 1; /* number of active unix sockets  (socket 0 is always reserved for the system
			socket, even if it is not enabled. */
static int sd_fds = 0;			/* number of systemd activated sockets */

#define DFLT_bCreatePath 0
#define DFLT_ratelimitInterval 0
#define DFLT_ratelimitBurst 200
#define DFLT_ratelimitSeverity 1			/* do not rate-limit emergency messages */
/* config vars for the legacy config system */
static struct configSettings_s {
	int bOmitLocalLogging;
	uchar *pLogSockName;
	uchar *pLogHostName;		/* host name to use with this socket */
	int bUseFlowCtl;		/* use flow control or not (if yes, only LIGHT is used!) */
	int bUseFlowCtlSysSock;
	int bIgnoreTimestamp;		/* ignore timestamps present in the incoming message? */
	int bIgnoreTimestampSysSock;
	int bUseSysTimeStamp;		/* use timestamp from system (rather than from message) */
	int bUseSysTimeStampSysSock;	/* same, for system log socket */
	int bWritePid;			/* use credentials from recvmsg() and fixup PID in TAG */
	int bWritePidSysSock;		/* use credentials from recvmsg() and fixup PID in TAG */
	int bCreatePath;		/* auto-create socket path? */
	int ratelimitInterval;		/* interval in seconds, 0 = off */
	int ratelimitIntervalSysSock;
	int ratelimitBurst;		/* max nbr of messages in interval */
	int ratelimitBurstSysSock;
	int ratelimitSeverity;
	int ratelimitSeveritySysSock;
	int bAnnotate;			/* annotate trusted properties */
	int bAnnotateSysSock;		/* same, for system log socket */
	int bParseTrusted;		/* parse trusted properties */
} cs;

/* config vars for the v2 config system (rsyslog v6+) */
struct instanceConf_s {
	uchar *sockName;
	uchar *pLogHostName;		/* host name to use with this socket */
	sbool bUseFlowCtl;		/* use flow control or not (if yes, only LIGHT is used! */
	sbool bIgnoreTimestamp;		/* ignore timestamps present in the incoming message? */
	sbool bWritePid;		/* use credentials from recvmsg() and fixup PID in TAG */
	sbool bUseSysTimeStamp;		/* use timestamp from system (instead of from message) */
	int bCreatePath;		/* auto-create socket path? */
	int ratelimitInterval;		/* interval in seconds, 0 = off */
	int ratelimitBurst;		/* max nbr of messages in interval */
	int ratelimitSeverity;
	int bAnnotate;			/* annotate trusted properties */
	int bParseTrusted;		/* parse trusted properties */
	sbool bDiscardOwnMsgs;		/* discard messages that originated from our own pid? */
	sbool bUnlink;
	sbool bUseSpecialParser;
	sbool bParseHost;
	uchar *pszBindRuleset;		/* name of ruleset to bind to */
	ruleset_t *pBindRuleset;	/* ruleset to bind listener to (use system default if unspecified) */
	struct instanceConf_s *next;
};

struct modConfData_s {
	rsconf_t *pConf;		/* our overall config object */
	instanceConf_t *root, *tail;
	uchar *pLogSockName;
	int ratelimitIntervalSysSock;
	int ratelimitBurstSysSock;
	int ratelimitSeveritySysSock;
	int bAnnotateSysSock;
	int bParseTrusted;
	int bUseSpecialParser;
	int bParseHost;
	sbool bIgnoreTimestamp;		/* ignore timestamps present in the incoming message? */
	sbool bUseFlowCtl;		/* use flow control or not (if yes, only LIGHT is used! */
	sbool bOmitLocalLogging;
	sbool bWritePidSysSock;
	sbool bUseSysTimeStamp;
	sbool bDiscardOwnMsgs;
	sbool configSetViaV2Method;
	sbool bUnlink;
};
static modConfData_t *loadModConf = NULL;/* modConf ptr to use for the current load process */
static modConfData_t *runModConf = NULL;/* modConf ptr to use for the current load process */

/* module-global parameters */
static struct cnfparamdescr modpdescr[] = {
	{ "syssock.use", eCmdHdlrBinary, 0 },
	{ "syssock.name", eCmdHdlrGetWord, 0 },
	{ "syssock.unlink", eCmdHdlrBinary, 0 },
	{ "syssock.ignoretimestamp", eCmdHdlrBinary, 0 },
	{ "syssock.ignoreownmessages", eCmdHdlrBinary, 0 },
	{ "syssock.flowcontrol", eCmdHdlrBinary, 0 },
	{ "syssock.usesystimestamp", eCmdHdlrBinary, 0 },
	{ "syssock.annotate", eCmdHdlrBinary, 0 },
	{ "syssock.parsetrusted", eCmdHdlrBinary, 0 },
	{ "syssock.usespecialparser", eCmdHdlrBinary, 0 },
	{ "syssock.parsehostname", eCmdHdlrBinary, 0 },
	{ "syssock.usepidfromsystem", eCmdHdlrBinary, 0 },
	{ "syssock.ratelimit.interval", eCmdHdlrInt, 0 },
	{ "syssock.ratelimit.burst", eCmdHdlrInt, 0 },
	{ "syssock.ratelimit.severity", eCmdHdlrInt, 0 }
};
static struct cnfparamblk modpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(modpdescr)/sizeof(struct cnfparamdescr),
	  modpdescr
	};

/* input instance parameters */
static struct cnfparamdescr inppdescr[] = {
	{ "socket", eCmdHdlrString, CNFPARAM_REQUIRED }, /* legacy: addunixlistensocket */
	{ "unlink", eCmdHdlrBinary, 0 },
	{ "createpath", eCmdHdlrBinary, 0 },
	{ "parsetrusted", eCmdHdlrBinary, 0 },
	{ "ignoreownmessages", eCmdHdlrBinary, 0 },
	{ "hostname", eCmdHdlrString, 0 },
	{ "ignoretimestamp", eCmdHdlrBinary, 0 },
	{ "flowcontrol", eCmdHdlrBinary, 0 },
	{ "usesystimestamp", eCmdHdlrBinary, 0 },
	{ "annotate", eCmdHdlrBinary, 0 },
	{ "usespecialparser", eCmdHdlrBinary, 0 },
	{ "parsehostname", eCmdHdlrBinary, 0 },
	{ "usepidfromsystem", eCmdHdlrBinary, 0 },
	{ "ruleset", eCmdHdlrString, 0 },
	{ "ratelimit.interval", eCmdHdlrInt, 0 },
	{ "ratelimit.burst", eCmdHdlrInt, 0 },
	{ "ratelimit.severity", eCmdHdlrInt, 0 }
};
static struct cnfparamblk inppblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(inppdescr)/sizeof(struct cnfparamdescr),
	  inppdescr
	};

#include "im-helper.h" /* must be included AFTER the type definitions! */

static int bLegacyCnfModGlobalsPermitted;/* are legacy module-global config parameters permitted? */


/* create input instance, set default parameters, and
 * add it to the list of instances.
 */
static rsRetVal
createInstance(instanceConf_t **pinst)
{
	instanceConf_t *inst;
	DEFiRet;
	CHKmalloc(inst = MALLOC(sizeof(instanceConf_t)));
	inst->sockName = NULL;
	inst->pLogHostName = NULL;
	inst->pszBindRuleset = NULL;
	inst->pBindRuleset = NULL;
	inst->ratelimitInterval = DFLT_ratelimitInterval;
	inst->ratelimitBurst = DFLT_ratelimitBurst;
	inst->ratelimitSeverity = DFLT_ratelimitSeverity;
	inst->bUseFlowCtl = 0;
	inst->bUseSpecialParser = 1;
	inst->bParseHost = UNSET;
	inst->bIgnoreTimestamp = 1;
	inst->bCreatePath = DFLT_bCreatePath;
	inst->bUseSysTimeStamp = 1;
	inst->bWritePid = 0;
	inst->bAnnotate = 0;
	inst->bParseTrusted = 0;
	inst->bDiscardOwnMsgs = bProcessInternalMessages;
	inst->bUnlink = 1;
	inst->next = NULL;

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


/* This function is called when a new listen socket instance shall be added to
 * the current config object via the legacy config system. It just shuffles
 * all parameters to the listener in-memory instance.
 * rgerhards, 2011-05-12
 */
static rsRetVal addInstance(void __attribute__((unused)) *pVal, uchar *pNewVal)
{
	instanceConf_t *inst;
	DEFiRet;

	if(pNewVal == NULL || pNewVal[0] == '\0') {
		LogError(0, RS_RET_SOCKNAME_MISSING , "imuxsock: socket name must be specified, "
			        "but is not - listener not created\n");
		if(pNewVal != NULL)
			free(pNewVal);
		ABORT_FINALIZE(RS_RET_SOCKNAME_MISSING);
	}

	CHKiRet(createInstance(&inst));
	inst->sockName = pNewVal;
	inst->ratelimitInterval = cs.ratelimitInterval;
	inst->pLogHostName = cs.pLogHostName;
	inst->ratelimitBurst = cs.ratelimitBurst;
	inst->ratelimitSeverity = cs.ratelimitSeverity;
	inst->bUseFlowCtl = cs.bUseFlowCtl;
	inst->bIgnoreTimestamp = cs.bIgnoreTimestamp;
	inst->bCreatePath = cs.bCreatePath;
	inst->bUseSysTimeStamp = cs.bUseSysTimeStamp;
	inst->bWritePid = cs.bWritePid;
	inst->bAnnotate = cs.bAnnotate;
	inst->bParseTrusted = cs.bParseTrusted;
	inst->bParseHost = UNSET;
	inst->next = NULL;

	/* reset hostname for next socket */
	cs.pLogHostName = NULL;

finalize_it:
	RETiRet;
}


/* add an additional listen socket.
 * added capability to specify hostname for socket -- rgerhards, 2008-08-01
 */
static rsRetVal
addListner(instanceConf_t *inst)
{
	DEFiRet;

	if(inst->bParseHost == UNSET) {
		if(*inst->sockName == ':') {
			listeners[nfd].bParseHost = 1;
		} else {
			listeners[nfd].bParseHost = 0;
		}
	} else {
		listeners[nfd].bParseHost = inst->bParseHost;
	}
	if(inst->pLogHostName == NULL) {
		listeners[nfd].hostName = NULL;
	} else {
		CHKiRet(prop.Construct(&(listeners[nfd].hostName)));
		CHKiRet(prop.SetString(listeners[nfd].hostName, inst->pLogHostName, ustrlen(inst->pLogHostName)));
		CHKiRet(prop.ConstructFinalize(listeners[nfd].hostName));
	}
	if(inst->ratelimitInterval > 0) {
		if((listeners[nfd].ht = create_hashtable(100, hash_from_key_fn, key_equals_fn,
			(void(*)(void*))ratelimitDestruct)) == NULL) {
			/* in this case, we simply turn off rate-limiting */
			DBGPRINTF("imuxsock: turning off rate limiting because we could not "
				  "create hash table\n");
			inst->ratelimitInterval = 0;
		}
	} else {
		listeners[nfd].ht = NULL;
	}
	listeners[nfd].ratelimitInterval = inst->ratelimitInterval;
	listeners[nfd].ratelimitBurst = inst->ratelimitBurst;
	listeners[nfd].ratelimitSev = inst->ratelimitSeverity;
	listeners[nfd].flowCtl = inst->bUseFlowCtl ? eFLOWCTL_LIGHT_DELAY : eFLOWCTL_NO_DELAY;
	listeners[nfd].flags = inst->bIgnoreTimestamp ? IGNDATE : NOFLAG;
	listeners[nfd].bCreatePath = inst->bCreatePath;
	listeners[nfd].sockName = ustrdup(inst->sockName);
	listeners[nfd].bUseCreds = (inst->bDiscardOwnMsgs || inst->bWritePid || inst->ratelimitInterval
	|| inst->bAnnotate || inst->bUseSysTimeStamp) ? 1 : 0;
	listeners[nfd].bAnnotate = inst->bAnnotate;
	listeners[nfd].bParseTrusted = inst->bParseTrusted;
	listeners[nfd].bDiscardOwnMsgs = inst->bDiscardOwnMsgs;
	listeners[nfd].bUnlink = inst->bUnlink;
	listeners[nfd].bWritePid = inst->bWritePid;
	listeners[nfd].bUseSysTimeStamp = inst->bUseSysTimeStamp;
	listeners[nfd].bUseSpecialParser = inst->bUseSpecialParser;
	listeners[nfd].pRuleset = inst->pBindRuleset;
	CHKiRet(ratelimitNew(&listeners[nfd].dflt_ratelimiter, "imuxsock", NULL));
	ratelimitSetLinuxLike(listeners[nfd].dflt_ratelimiter,
			      listeners[nfd].ratelimitInterval,
			      listeners[nfd].ratelimitBurst);
	ratelimitSetSeverity(listeners[nfd].dflt_ratelimiter,
			     listeners[nfd].ratelimitSev);
	nfd++;

finalize_it:
	RETiRet;
}


static rsRetVal discardLogSockets(void)
{
	int i;

	/* Check whether the system socket is in use */
	if(startIndexUxLocalSockets == 0) {
		/* Clean up rate limiting data for the system socket */
		if(listeners[0].ht != NULL) {
			hashtable_destroy(listeners[0].ht, 1); /* 1 => free all values automatically */
		}
		ratelimitDestruct(listeners[0].dflt_ratelimiter);
	}

	/* Clean up all other sockets */
	for (i = 1; i < nfd; i++) {
		if(listeners[i].sockName != NULL) {
			free(listeners[i].sockName);
			listeners[i].sockName = NULL;
		}
		if(listeners[i].hostName != NULL) {
			prop.Destruct(&(listeners[i].hostName));
		}
		if(listeners[i].ht != NULL) {
			hashtable_destroy(listeners[i].ht, 1); /* 1 => free all values automatically */
		}
		ratelimitDestruct(listeners[i].dflt_ratelimiter);
	}

	return RS_RET_OK;
}


/* used to create a log socket if NOT passed in via systemd.
 */
/* note: the linux SUN_LEN macro uses a sizeof based on a NULL pointer. This
 * triggers UBSan warning. As such, we turn that warning off for the fuction.
 * As it is OS-provided, there is no way to solve it ourselves. The problem
 * may also exist on other platforms, we have just noticed it on Linux.
 */
#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wunknown-attributes"
#endif
static rsRetVal
#if defined(__clang__)
__attribute__((no_sanitize("undefined")))
#endif
createLogSocket(lstn_t *pLstn)
{
	struct sockaddr_un sunx;
	DEFiRet;

	if(pLstn->bUnlink)
		unlink((char*)pLstn->sockName);
	memset(&sunx, 0, sizeof(sunx));
	sunx.sun_family = AF_UNIX;
	if(pLstn->bCreatePath) {
		makeFileParentDirs((uchar*)pLstn->sockName, ustrlen(pLstn->sockName), 0755, -1, -1, 0);
	}
	strncpy(sunx.sun_path, (char*)pLstn->sockName, sizeof(sunx.sun_path));
	sunx.sun_path[sizeof(sunx.sun_path)-1] = '\0';
	pLstn->fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if(pLstn->fd < 0 ) {
		ABORT_FINALIZE(RS_RET_ERR_CRE_AFUX);
	}
	if(bind(pLstn->fd, (struct sockaddr *) &sunx, SUN_LEN(&sunx)) < 0) {
		ABORT_FINALIZE(RS_RET_ERR_CRE_AFUX);
	}
	if(chmod((char*)pLstn->sockName, 0666) < 0) {
		ABORT_FINALIZE(RS_RET_ERR_CRE_AFUX);
	}
finalize_it:
	if(iRet != RS_RET_OK) {
		LogError(errno, iRet, "cannot create '%s'", pLstn->sockName);
		if(pLstn->fd != -1) {
			close(pLstn->fd);
			pLstn->fd = -1;
		}
	}
	RETiRet;
}


static rsRetVal
openLogSocket(lstn_t *pLstn)
{
	DEFiRet;
#	ifdef HAVE_SCM_CREDENTIALS
	int one;
#	endif /* HAVE_SCM_CREDENTIALS */

	if(pLstn->sockName[0] == '\0')
		return -1;

	pLstn->fd = -1;

#ifdef HAVE_LIBSYSTEMD
	if (sd_fds > 0) {
		/* Check if the current socket is a systemd activated one.
	        * If so, just use it.
		*/
		int fd;

		for (fd = SD_LISTEN_FDS_START; fd < SD_LISTEN_FDS_START + sd_fds; fd++) {
			if( sd_is_socket_unix(fd, SOCK_DGRAM, -1, (const char*) pLstn->sockName, 0) == 1) {
				/* ok, it matches -- just use as is */
				pLstn->fd = fd;

				LogMsg(0, NO_ERRCODE, LOG_INFO,
					"imuxsock: Acquired UNIX socket '%s' (fd %d) from systemd.\n",
					pLstn->sockName, pLstn->fd);
				break;
			}
			/*
			 * otherwise it either didn't match *this* socket and
			 * we just continue to check the next one or there was
			 * an error and we will create a new socket below.
			 */
		}
	}
#endif

	if (pLstn->fd == -1) {
		CHKiRet(createLogSocket(pLstn));
		assert(pLstn->fd != -1); /* else createLogSocket() should have failed! */
	}

#	ifdef HAVE_SCM_CREDENTIALS
	if(pLstn->bUseCreds) {
		one = 1;
		if(setsockopt(pLstn->fd, SOL_SOCKET, SO_PASSCRED, &one, (socklen_t) sizeof(one)) != 0) {
			LogError(errno, NO_ERRCODE, "set SO_PASSCRED failed on '%s'", pLstn->sockName);
			pLstn->bUseCreds = 0;
		}
// TODO: move to its own #if
		if(setsockopt(pLstn->fd, SOL_SOCKET, SO_TIMESTAMP, &one, sizeof(one)) != 0) {
			LogError(errno, NO_ERRCODE, "set SO_TIMESTAMP failed on '%s'", pLstn->sockName);
		}
	}
#	else /* HAVE_SCM_CREDENTIALS */
	pLstn->bUseCreds = 0;
	pLstn->bAnnotate = 0;
#	endif /* HAVE_SCM_CREDENTIALS */

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pLstn->fd != -1) {
			close(pLstn->fd);
			pLstn->fd = -1;
		}
	}

	RETiRet;
}


/* find ratelimiter to use for this message. Currently, we use the
 * pid, but may change to cgroup later (probably via a config switch).
 * Returns NULL if not found or rate-limiting not activated for this
 * listener (the latter being a performance enhancement).
 */
static rsRetVal
findRatelimiter(lstn_t *pLstn, struct ucred *cred, ratelimit_t **prl)
{
	ratelimit_t *rl = NULL;
	int r;
	pid_t *keybuf;
	char pinfobuf[512];
	DEFiRet;

	if(cred == NULL)
		FINALIZE;
#if 0 // TODO: check deactivated?
	if(pLstn->ratelimitInterval == 0) {
		*prl = NULL;
		FINALIZE;
	}
#endif
	if(pLstn->ht == NULL) {
		*prl = NULL;
		FINALIZE;
	}

	rl = hashtable_search(pLstn->ht, &cred->pid);
	if(rl == NULL) {
		/* we need to add a new ratelimiter, process not seen before! */
		DBGPRINTF("imuxsock: no ratelimiter for pid %lu, creating one\n",
			  (unsigned long) cred->pid);
		STATSCOUNTER_INC(ctrNumRatelimiters, mutCtrNumRatelimiters);
		/* read process name from system  */
		char procName[256]; /* enough for any sane process name  */
		snprintf(procName, sizeof(procName), "/proc/%lu/cmdline", (unsigned long) cred->pid);
		FILE *f = fopen(procName, "r");
		if (f) {
			size_t len;
			len = fread(procName, sizeof(char), 256, f);
			if (len > 0) {
				snprintf(pinfobuf, sizeof(pinfobuf), "pid: %lu, name: %s",
					(unsigned long) cred->pid, procName);
			}
			fclose(f);
		}
		else {
			snprintf(pinfobuf, sizeof(pinfobuf), "pid: %lu",
				(unsigned long) cred->pid);
		}
		pinfobuf[sizeof(pinfobuf)-1] = '\0'; /* to be on safe side */
		CHKiRet(ratelimitNew(&rl, "imuxsock", pinfobuf));
		ratelimitSetLinuxLike(rl, pLstn->ratelimitInterval, pLstn->ratelimitBurst);
		ratelimitSetSeverity(rl, pLstn->ratelimitSev);
		CHKmalloc(keybuf = malloc(sizeof(pid_t)));
		*keybuf = cred->pid;
		r = hashtable_insert(pLstn->ht, keybuf, rl);
		if(r == 0)
			ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
	}

	*prl = rl;
	rl = NULL;

finalize_it:
	if(rl != NULL)
		ratelimitDestruct(rl);
	if(*prl == NULL)
		*prl = pLstn->dflt_ratelimiter;
	RETiRet;
}


/* patch correct pid into tag. bufTAG MUST be CONF_TAG_MAXSIZE long!
 */
static void
fixPID(uchar *bufTAG, int *lenTag, struct ucred *cred)
{
	int i;
	char bufPID[16];
	int lenPID;

	if(cred == NULL)
		return;
	
	lenPID = snprintf(bufPID, sizeof(bufPID), "[%lu]:", (unsigned long) cred->pid);

	for(i = *lenTag ; i >= 0  && bufTAG[i] != '[' ; --i)
		/*JUST SKIP*/;

	if(i < 0)
		i = *lenTag - 1; /* go right at end of TAG, pid was not present (-1 for ':') */
	
	if(i + lenPID > CONF_TAG_MAXSIZE)
		return; /* do not touch, as things would break */

	memcpy(bufTAG + i, bufPID, lenPID);
	*lenTag = i + lenPID;
}


/* Get an "trusted property" from the system. Returns an empty string if the
 * property can not be obtained. Inspired by similiar functionality inside
 * journald. Currently works with Linux /proc filesystem, only.
 */
static rsRetVal
getTrustedProp(struct ucred *cred, const char *propName, uchar *buf, size_t lenBuf, int *lenProp)
{
	int fd;
	int i;
	int lenRead;
	char namebuf[1024];
	DEFiRet;

	if(snprintf(namebuf, sizeof(namebuf), "/proc/%lu/%s", (long unsigned) cred->pid,
		propName) >= (int) sizeof(namebuf)) {
		ABORT_FINALIZE(RS_RET_ERR);
	}

	if((fd = open(namebuf, O_RDONLY)) == -1) {
		DBGPRINTF("error reading '%s'\n", namebuf);
		ABORT_FINALIZE(RS_RET_ERR);
	}
	if((lenRead = read(fd, buf, lenBuf - 1)) == -1) {
		DBGPRINTF("error reading file data for '%s'\n", namebuf);
		close(fd);
		ABORT_FINALIZE(RS_RET_ERR);
	}
	
	/* we strip after the first \n */
	for(i = 0 ; i < lenRead ; ++i) {
		if(buf[i] == '\n')
			break;
		else if(iscntrl(buf[i]))
			buf[i] = ' ';
	}
	buf[i] = '\0';
	*lenProp = i;

	close(fd);

finalize_it:
	RETiRet;
}


/* read the exe trusted property path (so far, /proc fs only)
 */
static rsRetVal
getTrustedExe(struct ucred *cred, uchar *buf, size_t lenBuf, int* lenProp)
{
	int lenRead;
	char namebuf[1024];
	DEFiRet;

	if(snprintf(namebuf, sizeof(namebuf), "/proc/%lu/exe", (long unsigned) cred->pid)
		>= (int) sizeof(namebuf)) {
		ABORT_FINALIZE(RS_RET_ERR);
	}

	if((lenRead = readlink(namebuf, (char*)buf, lenBuf - 1)) == -1) {
		DBGPRINTF("error reading link '%s'\n", namebuf);
		ABORT_FINALIZE(RS_RET_ERR);
	}
	
	buf[lenRead] = '\0';
	*lenProp = lenRead;

finalize_it:
	RETiRet;
}


/* copy a trusted property in escaped mode. That is, the property can contain
 * any character and so it must be properly quoted AND escaped.
 * It is assumed the output buffer is large enough. Returns the number of
 * characters added.
 */
static int
copyescaped(uchar *dstbuf, uchar *inbuf, int inlen)
{
	int iDst, iSrc;

	*dstbuf = '"';
	for(iDst=1, iSrc=0 ; iSrc < inlen ; ++iDst, ++iSrc) {
		if(inbuf[iSrc] == '"' || inbuf[iSrc] == '\\') {
			dstbuf[iDst++] = '\\';
		}
		dstbuf[iDst] = inbuf[iSrc];
	}
	dstbuf[iDst++] = '"';
	return iDst;
}


/* submit received message to the queue engine
 * We now parse the message according to expected format so that we
 * can also mangle it if necessary.
 */
static rsRetVal
SubmitMsg(uchar *pRcv, int lenRcv, lstn_t *pLstn, struct ucred *cred, struct timeval *ts)
{
	smsg_t *pMsg = NULL;
	int lenMsg;
	int offs;
	int i;
	uchar *parse;
	syslog_pri_t pri;
	uchar bufParseTAG[CONF_TAG_MAXSIZE];
	struct syslogTime st;
	time_t tt;
	ratelimit_t *ratelimiter = NULL;
	struct syslogTime dummyTS;
	DEFiRet;

	if(pLstn->bDiscardOwnMsgs && cred != NULL && cred->pid == glblGetOurPid()) {
		DBGPRINTF("imuxsock: discarding message from our own pid\n");
		FINALIZE;
	}

	/* TODO: handle format errors?? */
	/* we need to parse the pri first, because we need the severity for
	 * rate-limiting as well.
	 */
	parse = pRcv;
	lenMsg = lenRcv;
	offs = 1; /* '<' */
	
	parse++;
	pri = 0;
	while(offs < lenMsg && isdigit(*parse)) {
		pri = pri * 10 + *parse - '0';
		++parse;
		++offs;
	}

	findRatelimiter(pLstn, cred, &ratelimiter); /* ignore error, better so than others... */

	if(ts == NULL) {
		datetime.getCurrTime(&st, &tt, TIME_IN_LOCALTIME);
	} else {
		datetime.timeval2syslogTime(ts, &st, TIME_IN_LOCALTIME);
		tt = ts->tv_sec;
	}

#if 0 // TODO: think about stats counters (or wait for request...?)
	if(ratelimiter != NULL && !withinRatelimit(ratelimiter, tt, cred->pid)) {
		STATSCOUNTER_INC(ctrLostRatelimit, mutCtrLostRatelimit);
		FINALIZE;
	}
#endif

	/* we now create our own message object and submit it to the queue */
	CHKiRet(msgConstructWithTime(&pMsg, &st, tt));

	/* created trusted properties */
	if(cred != NULL && pLstn->bAnnotate) {
		uchar propBuf[1024];
		int lenProp;

		if (pLstn->bParseTrusted) {
			struct json_object *json, *jval;

#define CHKjson(operation, toBeFreed)					\
			if((operation) == NULL) {			\
				json_object_put(toBeFreed);		\
				ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);	\
			}

			CHKmalloc(json = json_object_new_object());
			/* create value string, create field, and add it */
			CHKjson(jval = json_object_new_int(cred->pid), json);
			json_object_object_add(json, "pid", jval);
			CHKjson(jval = json_object_new_int(cred->uid), json);
			json_object_object_add(json, "uid", jval);
			CHKjson(jval = json_object_new_int(cred->gid), json);
			json_object_object_add(json, "gid", jval);
			if(getTrustedProp(cred, "comm", propBuf, sizeof(propBuf), &lenProp) == RS_RET_OK) {
				CHKjson(jval = json_object_new_string((char*)propBuf), json);
				json_object_object_add(json, "appname", jval);
			}
			if(getTrustedExe(cred, propBuf, sizeof(propBuf), &lenProp) == RS_RET_OK) {
				CHKjson(jval = json_object_new_string((char*)propBuf), json);
				json_object_object_add(json, "exe", jval);
			}
			if(getTrustedProp(cred, "cmdline", propBuf, sizeof(propBuf), &lenProp) == RS_RET_OK) {
				CHKjson(jval = json_object_new_string((char*)propBuf), json);
				json_object_object_add(json, "cmd", jval);
			}
#undef CHKjson

			/* as per lumberjack spec, these properties need to go into
			 * the CEE root.
			 */
			msgAddJSON(pMsg, (uchar*)"!", json, 0, 0);

			MsgSetRawMsg(pMsg, (char*)pRcv, lenRcv);
		} else {
			uchar msgbuf[8192];
			uchar *pmsgbuf = msgbuf;
			int toffs; /* offset for trusted properties */

			if((unsigned) (lenRcv + 4096) >= sizeof(msgbuf)) {
				CHKmalloc(pmsgbuf = malloc(lenRcv+4096));
			}

			memcpy(pmsgbuf, pRcv, lenRcv);
			memcpy(pmsgbuf+lenRcv, " @[", 3);
			toffs = lenRcv + 3; /* next free location */
			lenProp = snprintf((char*)propBuf, sizeof(propBuf), "_PID=%lu _UID=%lu _GID=%lu",
				 		(long unsigned) cred->pid, (long unsigned) cred->uid,
						(long unsigned) cred->gid);
			memcpy(pmsgbuf+toffs, propBuf, lenProp);
			toffs = toffs + lenProp;
	
			if(getTrustedProp(cred, "comm", propBuf, sizeof(propBuf), &lenProp) == RS_RET_OK) {
				memcpy(pmsgbuf+toffs, " _COMM=", 7);
				memcpy(pmsgbuf+toffs+7, propBuf, lenProp);
				toffs = toffs + 7 + lenProp;
			}
			if(getTrustedExe(cred, propBuf, sizeof(propBuf), &lenProp) == RS_RET_OK) {
				memcpy(pmsgbuf+toffs, " _EXE=", 6);
				memcpy(pmsgbuf+toffs+6, propBuf, lenProp);
				toffs = toffs + 6 + lenProp;
			}
			if(getTrustedProp(cred, "cmdline", propBuf, sizeof(propBuf), &lenProp) == RS_RET_OK) {
				memcpy(pmsgbuf+toffs, " _CMDLINE=", 10);
				toffs = toffs + 10 +
					copyescaped(pmsgbuf+toffs+10, propBuf, lenProp);
			}

			/* finalize string */
			pmsgbuf[toffs] = ']';
			pmsgbuf[toffs+1] = '\0';

			MsgSetRawMsg(pMsg, (char*)pmsgbuf, toffs + 1);
			if (pmsgbuf != msgbuf) {
				free(pmsgbuf);
			}
		}
	} else {
		/* just add the unmodified message */
		MsgSetRawMsg(pMsg, (char*)pRcv, lenRcv);
	}

	MsgSetFlowControlType(pMsg, pLstn->flowCtl);
	MsgSetInputName(pMsg, pInputName);
	if(pLstn->bParseHost) {
		pMsg->msgFlags  = pLstn->flags | PARSE_HOSTNAME;
	} else {
		pMsg->msgFlags  = pLstn->flags;
	}

	if(pLstn->bUseSpecialParser) {
		/* this is the legacy "log socket" parser which was written on the assumption
		 * that the log socket format would be fixed. While many folks said so, it
		 * seems to be different in practice, and this is why we now have choices...
		 * rgerhards, 2015-03-03
		 */
		parser.SanitizeMsg(pMsg);
		lenMsg = pMsg->iLenRawMsg - offs; /* SanitizeMsg() may have changed the size */
		msgSetPRI(pMsg, pri);
		MsgSetAfterPRIOffs(pMsg, offs);

		parse++; lenMsg--; /* '>' */
		if(ts == NULL) {
			if((pLstn->flags & IGNDATE)) {
				/* in this case, we still need to find out if we have a valid
				 * datestamp or not .. and advance the parse pointer accordingly.
				 */
				if (datetime.ParseTIMESTAMP3339(&dummyTS, &parse, &lenMsg) != RS_RET_OK) {
					datetime.ParseTIMESTAMP3164(&dummyTS, &parse, &lenMsg,
					NO_PARSE3164_TZSTRING, NO_PERMIT_YEAR_AFTER_TIME);
				}
			} else {
				if(datetime.ParseTIMESTAMP3339(&(pMsg->tTIMESTAMP), &parse, &lenMsg) != RS_RET_OK &&
				datetime.ParseTIMESTAMP3164(&(pMsg->tTIMESTAMP), &parse, &lenMsg,
				NO_PARSE3164_TZSTRING, NO_PERMIT_YEAR_AFTER_TIME) != RS_RET_OK) {
					DBGPRINTF("we have a problem, invalid timestamp in msg!\n");
				}
			}
		} else { /* if we pulled the time from the system, we need to update the message text */
			uchar *tmpParse = parse; /* just to check correctness of TS */
			if(datetime.ParseTIMESTAMP3339(&dummyTS, &tmpParse, &lenMsg) == RS_RET_OK ||
		 	datetime.ParseTIMESTAMP3164(&dummyTS, &tmpParse, &lenMsg, NO_PARSE3164_TZSTRING,
			NO_PERMIT_YEAR_AFTER_TIME) == RS_RET_OK) {
			/* We modify the message only if it contained a valid timestamp,
			otherwise we do not touch it at all. */
				datetime.formatTimestamp3164(&st, (char*)parse, 0);
				parse[15] = ' '; /* re-write \0 from fromatTimestamp3164 by SP */
				/* update "counters" to reflect processed timestamp */
				parse += 16;
			}
		}

		/* pull tag */

		i = 0;
		while(lenMsg > 0 && *parse != ' ' && i < CONF_TAG_MAXSIZE - 1) {
			bufParseTAG[i++] = *parse++;
			--lenMsg;
		}
		bufParseTAG[i] = '\0';	/* terminate string */
		if(pLstn->bWritePid)
			fixPID(bufParseTAG, &i, cred);
		MsgSetTAG(pMsg, bufParseTAG, i);
		MsgSetMSGoffs(pMsg, pMsg->iLenRawMsg - lenMsg);
	} else { /* we are configured to use regular parser chain */
		pMsg->msgFlags  |= NEEDS_PARSING;
	}

	MsgSetRcvFrom(pMsg, pLstn->hostName == NULL ? glbl.GetLocalHostNameProp() : pLstn->hostName);
	CHKiRet(MsgSetRcvFromIP(pMsg, pLocalHostIP));
	MsgSetRuleset(pMsg, pLstn->pRuleset);
	ratelimitAddMsg(ratelimiter, NULL, pMsg);
	STATSCOUNTER_INC(ctrSubmit, mutCtrSubmit);
finalize_it:
	if(iRet != RS_RET_OK) {
		if(pMsg != NULL)
			msgDestruct(&pMsg);
	}
	RETiRet;
}


/* This function receives data from a socket indicated to be ready
 * to receive and submits the message received for processing.
 * rgerhards, 2007-12-20
 * Interface changed so that this function is passed the array index
 * of the socket which is to be processed. This eases access to the
 * growing number of properties. -- rgerhards, 2008-08-01
 */
static rsRetVal readSocket(lstn_t *pLstn)
{
	DEFiRet;
	int iRcvd;
	int iMaxLine;
	struct msghdr msgh;
	struct iovec msgiov;
	struct ucred cred;
	struct timeval ts;
	int cred_set = 0;
	int ts_set = 0;
	uchar bufRcv[4096+1];
	uchar *pRcv = NULL; /* receive buffer */
#	ifdef HAVE_SCM_CREDENTIALS
	/* aux is a union rather than a direct char array to force alignment with cmsghdr */
	union {
		char buf[128];
		struct cmsghdr cm;
	} aux;
#	endif

	assert(pLstn->fd >= 0);

	iMaxLine = glbl.GetMaxLine();

	/* we optimize performance: if iMaxLine is below 4K (which it is in almost all
	 * cases, we use a fixed buffer on the stack. Only if it is higher, heap memory
	 * is used. We could use alloca() to achive a similar aspect, but there are so
	 * many issues with alloca() that I do not want to take that route.
	 * rgerhards, 2008-09-02
	 */
	if((size_t) iMaxLine < sizeof(bufRcv) - 1) {
		pRcv = bufRcv;
	} else {
		CHKmalloc(pRcv = (uchar*) MALLOC(iMaxLine + 1));
	}

	memset(&msgh, 0, sizeof(msgh));
	memset(&msgiov, 0, sizeof(msgiov));
#	ifdef HAVE_SCM_CREDENTIALS
	if(pLstn->bUseCreds) {
		memset(&aux, 0, sizeof(aux));
		msgh.msg_control = &aux;
		msgh.msg_controllen = sizeof(aux);
	}
#	endif
	msgiov.iov_base = (char*)pRcv;
	msgiov.iov_len = iMaxLine;
	msgh.msg_iov = &msgiov;
	msgh.msg_iovlen = 1;
/*  AIXPORT : MSG_DONTWAIT not supported */
#if defined (_AIX)
#define MSG_DONTWAIT    MSG_NONBLOCK
#endif
	iRcvd = recvmsg(pLstn->fd, &msgh, MSG_DONTWAIT);

	DBGPRINTF("Message from UNIX socket: #%d, size %d\n", pLstn->fd, (int) iRcvd);
	if(iRcvd > 0) {
#		if defined(HAVE_SCM_CREDENTIALS) || defined(HAVE_SO_TIMESTAMP)
		if(pLstn->bUseCreds) {
			struct cmsghdr *cm;
			for(cm = CMSG_FIRSTHDR(&msgh); cm; cm = CMSG_NXTHDR(&msgh, cm)) {
#				ifdef HAVE_SCM_CREDENTIALS
				if(   pLstn->bUseCreds
				   && cm->cmsg_level == SOL_SOCKET && cm->cmsg_type == SCM_CREDENTIALS) {
					memcpy(&cred, CMSG_DATA(cm), sizeof(cred));
					cred_set = 1;
				}
#				endif /* HAVE_SCM_CREDENTIALS */
#				if HAVE_SO_TIMESTAMP
				if(   pLstn->bUseSysTimeStamp
				   && cm->cmsg_level == SOL_SOCKET && cm->cmsg_type == SO_TIMESTAMP) {
					memcpy(&ts, CMSG_DATA(cm), sizeof(ts));
					ts_set = 1;
				}
#				endif /* HAVE_SO_TIMESTAMP */
			}
		}
#		endif /* defined(HAVE_SCM_CREDENTIALS) || defined(HAVE_SO_TIMESTAMP) */
		CHKiRet(SubmitMsg(pRcv, iRcvd, pLstn, (cred_set ? &cred : NULL), (ts_set ? &ts : NULL)));
	} else if(iRcvd < 0 && errno != EINTR && errno != EAGAIN) {
		char errStr[1024];
		rs_strerror_r(errno, errStr, sizeof(errStr));
		DBGPRINTF("UNIX socket error: %d = %s.\n", errno, errStr);
		LogError(errno, NO_ERRCODE, "imuxsock: recvfrom UNIX");
	}

finalize_it:
	if(pRcv != NULL && (size_t) iMaxLine >= sizeof(bufRcv) - 1)
		free(pRcv);

	RETiRet;
}


/* activate current listeners */
static rsRetVal
activateListeners(void)
{
	int actSocks;
	int i;
	DEFiRet;

	/* Initialize the system socket only if it's in use */
	if(startIndexUxLocalSockets == 0) {
		/* first apply some config settings */
		listeners[0].sockName = UCHAR_CONSTANT(_PATH_LOG);
		if(runModConf->pLogSockName != NULL) {
			listeners[0].sockName = runModConf->pLogSockName;
		}
#ifdef HAVE_LIBSYSTEMD
		else if(sd_booted()) {
			struct stat st;
			if(stat(SYSTEMD_PATH_LOG, &st) != -1 && S_ISSOCK(st.st_mode)) {
				listeners[0].sockName = (uchar*) SYSTEMD_PATH_LOG;
			}
		}
#endif
		if(runModConf->ratelimitIntervalSysSock > 0) {
			if((listeners[0].ht = create_hashtable(100, hash_from_key_fn, key_equals_fn, NULL)) == NULL) {
				/* in this case, we simply turn of rate-limiting */
				LogError(0, NO_ERRCODE, "imuxsock: turning off rate limiting because "
					"we could not create hash table\n");
				runModConf->ratelimitIntervalSysSock = 0;
			}
		} else {
			listeners[0].ht = NULL;
		}
		listeners[0].fd = -1;
		listeners[0].pRuleset = NULL;
		listeners[0].hostName = NULL;
		listeners[0].bParseHost = 0;
		listeners[0].bCreatePath = 0;
		listeners[0].ratelimitInterval = runModConf->ratelimitIntervalSysSock;
		listeners[0].ratelimitBurst = runModConf->ratelimitBurstSysSock;
		listeners[0].ratelimitSev = runModConf->ratelimitSeveritySysSock;
		listeners[0].bUseCreds = (runModConf->bWritePidSysSock || runModConf->ratelimitIntervalSysSock
		|| runModConf->bAnnotateSysSock || runModConf->bDiscardOwnMsgs
		|| runModConf->bUseSysTimeStamp) ? 1 : 0;
		listeners[0].bWritePid = runModConf->bWritePidSysSock;
		listeners[0].bAnnotate = runModConf->bAnnotateSysSock;
		listeners[0].bParseTrusted = runModConf->bParseTrusted;
		listeners[0].bParseHost = runModConf->bParseHost;
		listeners[0].bUseSpecialParser = runModConf->bUseSpecialParser;
		listeners[0].bDiscardOwnMsgs = runModConf->bDiscardOwnMsgs;
		listeners[0].bUnlink = runModConf->bUnlink;
		listeners[0].bUseSysTimeStamp = runModConf->bUseSysTimeStamp;
		listeners[0].flags = runModConf->bIgnoreTimestamp ? IGNDATE : NOFLAG;
		listeners[0].flowCtl = runModConf->bUseFlowCtl ? eFLOWCTL_LIGHT_DELAY : eFLOWCTL_NO_DELAY;
		CHKiRet(ratelimitNew(&listeners[0].dflt_ratelimiter, "imuxsock", NULL));
			ratelimitSetLinuxLike(listeners[0].dflt_ratelimiter,
			listeners[0].ratelimitInterval,
			listeners[0].ratelimitBurst);
		ratelimitSetSeverity(listeners[0].dflt_ratelimiter,listeners[0].ratelimitSev);
	}

#ifdef HAVE_LIBSYSTEMD
	sd_fds = sd_listen_fds(0);
	if(sd_fds < 0) {
		LogError(-sd_fds, NO_ERRCODE, "imuxsock: Failed to acquire systemd socket");
		ABORT_FINALIZE(RS_RET_ERR_CRE_AFUX);
	}
#endif

	/* initialize and return if will run or not */
	actSocks = 0;
	for (i = startIndexUxLocalSockets ; i < nfd ; i++) {
		if(openLogSocket(&(listeners[i])) == RS_RET_OK) {
			++actSocks;
			DBGPRINTF("imuxsock: Opened UNIX socket '%s' (fd %d).\n",
				  listeners[i].sockName, listeners[i].fd);
		}
	}

	if(actSocks == 0) {
		LogError(0, RS_RET_ERR, "imuxsock does not run because we could not "
			"aquire any socket\n");
		ABORT_FINALIZE(RS_RET_ERR);
	}

finalize_it:
	RETiRet;
}



BEGINbeginCnfLoad
CODESTARTbeginCnfLoad
	loadModConf = pModConf;
	pModConf->pConf = pConf;
	/* init our settings */
	pModConf->pLogSockName = NULL;
	pModConf->bOmitLocalLogging = 0;
	pModConf->bIgnoreTimestamp = 1;
	pModConf->bUseFlowCtl = 0;
	pModConf->bUseSysTimeStamp = 1;
	pModConf->bWritePidSysSock = 0;
	pModConf->bAnnotateSysSock = 0;
	pModConf->bParseTrusted = 0;
	pModConf->bParseHost = UNSET;
	pModConf->bUseSpecialParser = 1;
	/* if we do not process internal messages, we will see messages
	 * from ourselves, and so we need to permit this.
	 */
	pModConf->bDiscardOwnMsgs = bProcessInternalMessages;
	pModConf->bUnlink = 1;
	pModConf->ratelimitIntervalSysSock = DFLT_ratelimitInterval;
	pModConf->ratelimitBurstSysSock = DFLT_ratelimitBurst;
	pModConf->ratelimitSeveritySysSock = DFLT_ratelimitSeverity;
	bLegacyCnfModGlobalsPermitted = 1;
	/* reset legacy config vars */
	resetConfigVariables(NULL, NULL);
ENDbeginCnfLoad


BEGINsetModCnf
	struct cnfparamvals *pvals = NULL;
	int i;
CODESTARTsetModCnf
	pvals = nvlstGetParams(lst, &modpblk, NULL);
	if(pvals == NULL) {
		LogError(0, RS_RET_MISSING_CNFPARAMS, "error processing module "
				"config parameters [module(...)]");
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	if(Debug) {
		dbgprintf("module (global) param blk for imuxsock:\n");
		cnfparamsPrint(&modpblk, pvals);
	}

	for(i = 0 ; i < modpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(modpblk.descr[i].name, "syssock.use")) {
			loadModConf->bOmitLocalLogging = ((int) pvals[i].val.d.n) ? 0 : 1;
		} else if(!strcmp(modpblk.descr[i].name, "syssock.name")) {
			loadModConf->pLogSockName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(modpblk.descr[i].name, "syssock.ignoretimestamp")) {
			loadModConf->bIgnoreTimestamp = (int) pvals[i].val.d.n;
		} else if(!strcmp(modpblk.descr[i].name, "syssock.ignoreownmessages")) {
			loadModConf->bDiscardOwnMsgs = (int) pvals[i].val.d.n;
		} else if(!strcmp(modpblk.descr[i].name, "syssock.unlink")) {
			loadModConf->bUnlink = (int) pvals[i].val.d.n;
		} else if(!strcmp(modpblk.descr[i].name, "syssock.flowcontrol")) {
			loadModConf->bUseFlowCtl = (int) pvals[i].val.d.n;
		} else if(!strcmp(modpblk.descr[i].name, "syssock.usesystimestamp")) {
			loadModConf->bUseSysTimeStamp = (int) pvals[i].val.d.n;
		} else if(!strcmp(modpblk.descr[i].name, "syssock.annotate")) {
			loadModConf->bAnnotateSysSock = (int) pvals[i].val.d.n;
		} else if(!strcmp(modpblk.descr[i].name, "syssock.parsetrusted")) {
			loadModConf->bParseTrusted = (int) pvals[i].val.d.n;
		} else if(!strcmp(modpblk.descr[i].name, "syssock.parsehostname")) {
			loadModConf->bParseHost = (int) pvals[i].val.d.n;
		} else if(!strcmp(modpblk.descr[i].name, "syssock.usespecialparser")) {
			loadModConf->bUseSpecialParser = (int) pvals[i].val.d.n;
		} else if(!strcmp(modpblk.descr[i].name, "syssock.usepidfromsystem")) {
			loadModConf->bWritePidSysSock = (int) pvals[i].val.d.n;
		} else if(!strcmp(modpblk.descr[i].name, "syssock.ratelimit.interval")) {
			loadModConf->ratelimitIntervalSysSock = (int) pvals[i].val.d.n;
		} else if(!strcmp(modpblk.descr[i].name, "syssock.ratelimit.burst")) {
			loadModConf->ratelimitBurstSysSock = (int) pvals[i].val.d.n;
		} else if(!strcmp(modpblk.descr[i].name, "syssock.ratelimit.severity")) {
			loadModConf->ratelimitSeveritySysSock = (int) pvals[i].val.d.n;
		} else {
			dbgprintf("imuxsock: program error, non-handled "
			  "param '%s' in beginCnfLoad\n", modpblk.descr[i].name);
		}
	}

	/* disable legacy module-global config directives */
	bLegacyCnfModGlobalsPermitted = 0;
	loadModConf->configSetViaV2Method = 1;

finalize_it:
	if(pvals != NULL)
		cnfparamvalsDestruct(pvals, &modpblk);
ENDsetModCnf


BEGINnewInpInst
	struct cnfparamvals *pvals;
	instanceConf_t *inst;
	int i;
CODESTARTnewInpInst
	DBGPRINTF("newInpInst (imuxsock)\n");

	pvals = nvlstGetParams(lst, &inppblk, NULL);
	if(pvals == NULL) {
		LogError(0, RS_RET_MISSING_CNFPARAMS,
			        "imuxsock: required parameter are missing\n");
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	if(Debug) {
		dbgprintf("input param blk in imuxsock:\n");
		cnfparamsPrint(&inppblk, pvals);
	}

	CHKiRet(createInstance(&inst));

	for(i = 0 ; i < inppblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(inppblk.descr[i].name, "socket")) {
			inst->sockName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "createpath")) {
			inst->bCreatePath = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "parsetrusted")) {
			inst->bParseTrusted = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "ignoreownmessages")) {
			inst->bDiscardOwnMsgs = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "unlink")) {
			inst->bUnlink = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "hostname")) {
			inst->pLogHostName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "ignoretimestamp")) {
			inst->bIgnoreTimestamp = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "flowcontrol")) {
			inst->bUseFlowCtl = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "usesystimestamp")) {
			inst->bUseSysTimeStamp = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "annotate")) {
			inst->bAnnotate = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "usepidfromsystem")) {
			inst->bWritePid = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "parsehostname")) {
			inst->bParseHost  = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "usespecialparser")) {
			inst->bUseSpecialParser  = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "ruleset")) {
			inst->pszBindRuleset = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "ratelimit.interval")) {
			inst->ratelimitInterval = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "ratelimit.burst")) {
			inst->ratelimitBurst = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "ratelimit.severity")) {
			inst->ratelimitSeverity = (int) pvals[i].val.d.n;
		} else {
			dbgprintf("imuxsock: program error, non-handled "
			  "param '%s'\n", inppblk.descr[i].name);
		}
	}
finalize_it:
CODE_STD_FINALIZERnewInpInst
	cnfparamvalsDestruct(pvals, &inppblk);
ENDnewInpInst


BEGINendCnfLoad
CODESTARTendCnfLoad
	if(!loadModConf->configSetViaV2Method) {
		/* persist module-specific settings from legacy config system */
		/* these are used to initialize the system log socket (listeners[0]) */
		loadModConf->bOmitLocalLogging = cs.bOmitLocalLogging;
		loadModConf->pLogSockName = cs.pLogSockName;
		loadModConf->bIgnoreTimestamp = cs.bIgnoreTimestampSysSock;
		loadModConf->bUseSysTimeStamp = cs.bUseSysTimeStampSysSock;
		loadModConf->bUseFlowCtl = cs.bUseFlowCtlSysSock;
		loadModConf->bAnnotateSysSock = cs.bAnnotateSysSock;
		loadModConf->bWritePidSysSock = cs.bWritePidSysSock;
		loadModConf->bParseTrusted = cs.bParseTrusted;
		loadModConf->ratelimitIntervalSysSock = cs.ratelimitIntervalSysSock;
		loadModConf->ratelimitBurstSysSock = cs.ratelimitBurstSysSock;
		loadModConf->ratelimitSeveritySysSock = cs.ratelimitSeveritySysSock;
	}

	loadModConf = NULL; /* done loading */
	/* free legacy config vars */
	free(cs.pLogHostName);
	cs.pLogSockName = NULL;
	cs.pLogHostName = NULL;
ENDendCnfLoad


/* function to generate error message if framework does not find requested ruleset */
static void
std_checkRuleset_genErrMsg(__attribute__((unused)) modConfData_t *modConf, instanceConf_t *inst)
{
	LogError(0, NO_ERRCODE, "imuxsock: ruleset '%s' for socket %s not found - "
			"using default ruleset instead", inst->pszBindRuleset,
			inst->sockName);
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
	int nLstn;
	int i;
CODESTARTactivateCnfPrePrivDrop
	runModConf = pModConf;
#	ifdef OS_SOLARIS
		/* under solaris, we must NEVER process the local log socket, because
		 * it is implemented there differently. If we used it, we would actually
		 * delete it and render the system partly unusable. So don't do that.
		 * rgerhards, 2010-03-26
		 */
		startIndexUxLocalSockets = 1;
#	else
		startIndexUxLocalSockets = runModConf->bOmitLocalLogging ? 1 : 0;
#	endif
	/* we first calculate the number of listeners so that we can
	 * appropriately size the listener array. Note that we will
	 * always allocate memory for the system log socket.
	 */
	nLstn = 0;
	for(inst = runModConf->root ; inst != NULL ; inst = inst->next) {
		++nLstn;
	}
	if(nLstn > 0 || startIndexUxLocalSockets == 0) {
		DBGPRINTF("imuxsock: allocating memory for %d listeners\n", nLstn);
		CHKmalloc(listeners = realloc(listeners, (1+nLstn)*sizeof(lstn_t)));
		for(i = 1 ; i < nLstn ; ++i) {
			listeners[i].sockName = NULL;
			listeners[i].fd  = -1;
		}
		for(inst = runModConf->root ; inst != NULL ; inst = inst->next) {
			addListner(inst);
		}
		CHKiRet(activateListeners());
	}
finalize_it:
ENDactivateCnfPrePrivDrop


BEGINactivateCnf
CODESTARTactivateCnf
ENDactivateCnf


BEGINfreeCnf
	instanceConf_t *inst, *del;
CODESTARTfreeCnf
	free(pModConf->pLogSockName);
	for(inst = pModConf->root ; inst != NULL ; ) {
		free(inst->sockName);
		free(inst->pszBindRuleset);
		free(inst->pLogHostName);
		del = inst;
		inst = inst->next;
		free(del);
	}
ENDfreeCnf


/* This function is called to gather input. */
BEGINrunInput
	int nfds;
	int i;
CODESTARTrunInput
	struct pollfd *const pollfds = calloc(nfd, sizeof(struct pollfd));
	CHKmalloc(pollfds);
	if(startIndexUxLocalSockets == 1 && nfd == 1) {
		/* No sockets were configured, no reason to run. */
		ABORT_FINALIZE(RS_RET_OK);
	}
	if(startIndexUxLocalSockets == 1) {
		pollfds[0].fd = -1;
	}
	for (i = startIndexUxLocalSockets; i < nfd; i++) {
		pollfds[i].fd = listeners[i].fd;
		pollfds[i].events = POLLIN;
	}

	/* this is an endless loop - it is terminated when the thread is
	 * signalled to do so.
	 */
	while(1) {
		DBGPRINTF("--------imuxsock calling poll() on %d fds\n", nfd);

		nfds = poll(pollfds, nfd, -1);
		if(glbl.GetGlobalInputTermState() == 1)
			break; /* terminate input! */

		if(nfds < 0) {
			if(errno == EINTR) {
				DBGPRINTF("imuxsock: EINTR occured\n");
			} else {
				LogMsg(errno, RS_RET_POLL_ERR, LOG_WARNING, "imuxsock: poll "
					"system call failed, may cause further troubles");
			}
			nfds = 0;
		}

		for (i = startIndexUxLocalSockets ; i < nfd && nfds > 0; i++) {
			if(glbl.GetGlobalInputTermState() == 1)
				ABORT_FINALIZE(RS_RET_FORCE_TERM); /* terminate input! */
			if(pollfds[i].revents & POLLIN) {
				readSocket(&(listeners[i]));
				--nfds; /* indicate we have processed one */
			}
		}
	}

finalize_it:
	free(pollfds);
ENDrunInput


BEGINwillRun
CODESTARTwillRun
ENDwillRun


BEGINafterRun
	int i;
CODESTARTafterRun
	/* do cleanup here */
	if(startIndexUxLocalSockets == 1 && nfd == 1) {
		/* No sockets were configured, no cleanup needed. */
		return RS_RET_OK;
	}

	/* Close the UNIX sockets. */
	for (i = 0; i < nfd; i++)
		if (listeners[i].fd != -1)
			close(listeners[i].fd);

	/* Clean-up files. */
	for(i = startIndexUxLocalSockets; i < nfd; i++)
		if (listeners[i].sockName && listeners[i].fd != -1) {
			/* If systemd passed us a socket it is systemd's job to clean it up.
			 * Do not unlink it -- we will get same socket (node) from systemd
			 * e.g. on restart again.
			 */
			if (sd_fds > 0
#			ifdef HAVE_LIBSYSTEMD
			    && listeners[i].fd >= SD_LISTEN_FDS_START &&
			       listeners[i].fd <  SD_LISTEN_FDS_START + sd_fds
#			endif
			   )
				continue;

			if(listeners[i].bUnlink) {
				DBGPRINTF("imuxsock: unlinking unix socket file[%d] %s\n", i, listeners[i].sockName);
				unlink((char*) listeners[i].sockName);
			}
		}

	discardLogSockets();
	nfd = 1;
ENDafterRun


BEGINmodExit
CODESTARTmodExit
	free(listeners);
	if(pInputName != NULL)
		prop.Destruct(&pInputName);

	statsobj.Destruct(&modStats);

	objRelease(parser, CORE_COMPONENT);
	objRelease(glbl, CORE_COMPONENT);
	objRelease(prop, CORE_COMPONENT);
	objRelease(statsobj, CORE_COMPONENT);
	objRelease(datetime, CORE_COMPONENT);
	objRelease(ruleset, CORE_COMPONENT);
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
	free(cs.pLogSockName);
	cs.pLogSockName = NULL;
	free(cs.pLogHostName);
	cs.bOmitLocalLogging = 0;
	cs.pLogHostName = NULL;
	cs.bIgnoreTimestamp = 1;
	cs.bIgnoreTimestampSysSock = 1;
	cs.bUseFlowCtl = 0;
	cs.bUseFlowCtlSysSock = 0;
	cs.bUseSysTimeStamp = 1;
	cs.bUseSysTimeStampSysSock = 1;
	cs.bWritePid = 0;
	cs.bWritePidSysSock = 0;
	cs.bAnnotate = 0;
	cs.bAnnotateSysSock = 0;
	cs.bParseTrusted = 0;
	cs.bCreatePath = DFLT_bCreatePath;
	cs.ratelimitInterval = DFLT_ratelimitInterval;
	cs.ratelimitIntervalSysSock = DFLT_ratelimitInterval;
	cs.ratelimitBurst = DFLT_ratelimitBurst;
	cs.ratelimitBurstSysSock = DFLT_ratelimitBurst;
	cs.ratelimitSeverity = DFLT_ratelimitSeverity;
	cs.ratelimitSeveritySysSock = DFLT_ratelimitSeverity;

	return RS_RET_OK;
}


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(net, CORE_COMPONENT));
	CHKiRet(objUse(prop, CORE_COMPONENT));
	CHKiRet(objUse(statsobj, CORE_COMPONENT));
	CHKiRet(objUse(datetime, CORE_COMPONENT));
	CHKiRet(objUse(parser, CORE_COMPONENT));
	CHKiRet(objUse(ruleset, CORE_COMPONENT));

	DBGPRINTF("imuxsock version %s initializing\n", PACKAGE_VERSION);

	/* init legacy config vars */
	cs.pLogSockName = NULL;
	cs.pLogHostName = NULL;	/* host name to use with this socket */

	/* we need to create the inputName property (only once during our lifetime) */
	CHKiRet(prop.Construct(&pInputName));
	CHKiRet(prop.SetString(pInputName, UCHAR_CONSTANT("imuxsock"), sizeof("imuxsock") - 1));
	CHKiRet(prop.ConstructFinalize(pInputName));

	/* right now, glbl does not permit per-instance IP address notation. As long as this
	 * is the case, it is OK to query the HostIP once here at this location. HOWEVER, the
	 * whole concept is not 100% clean and needs to be addressed on a higher layer.
	 * TODO / rgerhards, 2012-04-11
	 */
	pLocalHostIP = glbl.GetLocalHostIP();

	/* register config file handlers */
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputunixlistensocketignoremsgtimestamp", 0, eCmdHdlrBinary,
		NULL, &cs.bIgnoreTimestamp, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputunixlistensockethostname", 0, eCmdHdlrGetWord,
		NULL, &cs.pLogHostName, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputunixlistensocketflowcontrol", 0, eCmdHdlrBinary,
		NULL, &cs.bUseFlowCtl, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputunixlistensocketannotate", 0, eCmdHdlrBinary,
		NULL, &cs.bAnnotate, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputunixlistensocketcreatepath", 0, eCmdHdlrBinary,
		NULL, &cs.bCreatePath, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputunixlistensocketusesystimestamp", 0, eCmdHdlrBinary,
		NULL, &cs.bUseSysTimeStamp, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"addunixlistensocket", 0, eCmdHdlrGetWord,
		addInstance, NULL, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputunixlistensocketusepidfromsystem", 0, eCmdHdlrBinary,
		NULL, &cs.bWritePid, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"imuxsockratelimitinterval", 0, eCmdHdlrInt,
		NULL, &cs.ratelimitInterval, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"imuxsockratelimitburst", 0, eCmdHdlrInt,
		NULL, &cs.ratelimitBurst, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"imuxsockratelimitseverity", 0, eCmdHdlrInt,
		NULL, &cs.ratelimitSeverity, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler,
		resetConfigVariables, NULL, STD_LOADABLE_MODULE_ID));
	/* the following one is a (dirty) trick: the system log socket is not added via
	 * an "addUnixListenSocket" config format. As such, it's properties can not be modified
	 * via $InputUnixListenSocket*". So we need to add a special directive
	 * for that. We should revisit all of that once we have the new config format...
	 * rgerhards, 2008-03-06
	 */
	CHKiRet(regCfSysLineHdlr2((uchar *)"omitlocallogging", 0, eCmdHdlrBinary,
		NULL, &cs.bOmitLocalLogging, STD_LOADABLE_MODULE_ID, &bLegacyCnfModGlobalsPermitted));
	CHKiRet(regCfSysLineHdlr2((uchar *)"systemlogsocketname", 0, eCmdHdlrGetWord,
		NULL, &cs.pLogSockName, STD_LOADABLE_MODULE_ID, &bLegacyCnfModGlobalsPermitted));
	CHKiRet(regCfSysLineHdlr2((uchar *)"systemlogsocketignoremsgtimestamp", 0, eCmdHdlrBinary,
		NULL, &cs.bIgnoreTimestampSysSock, STD_LOADABLE_MODULE_ID, &bLegacyCnfModGlobalsPermitted));
	CHKiRet(regCfSysLineHdlr2((uchar *)"systemlogsocketflowcontrol", 0, eCmdHdlrBinary,
		NULL, &cs.bUseFlowCtlSysSock, STD_LOADABLE_MODULE_ID, &bLegacyCnfModGlobalsPermitted));
	CHKiRet(regCfSysLineHdlr2((uchar *)"systemlogusesystimestamp", 0, eCmdHdlrBinary,
		NULL, &cs.bUseSysTimeStampSysSock, STD_LOADABLE_MODULE_ID, &bLegacyCnfModGlobalsPermitted));
	CHKiRet(regCfSysLineHdlr2((uchar *)"systemlogsocketannotate", 0, eCmdHdlrBinary,
		NULL, &cs.bAnnotateSysSock, STD_LOADABLE_MODULE_ID, &bLegacyCnfModGlobalsPermitted));
	CHKiRet(regCfSysLineHdlr2((uchar *)"systemlogparsetrusted", 0, eCmdHdlrBinary,
		NULL, &cs.bParseTrusted, STD_LOADABLE_MODULE_ID, &bLegacyCnfModGlobalsPermitted));
	CHKiRet(regCfSysLineHdlr2((uchar *)"systemlogusepidfromsystem", 0, eCmdHdlrBinary,
		NULL, &cs.bWritePidSysSock, STD_LOADABLE_MODULE_ID, &bLegacyCnfModGlobalsPermitted));
	CHKiRet(regCfSysLineHdlr2((uchar *)"systemlogratelimitinterval", 0, eCmdHdlrInt,
		NULL, &cs.ratelimitIntervalSysSock, STD_LOADABLE_MODULE_ID, &bLegacyCnfModGlobalsPermitted));
	CHKiRet(regCfSysLineHdlr2((uchar *)"systemlogratelimitburst", 0, eCmdHdlrInt,
		NULL, &cs.ratelimitBurstSysSock, STD_LOADABLE_MODULE_ID, &bLegacyCnfModGlobalsPermitted));
	CHKiRet(regCfSysLineHdlr2((uchar *)"systemlogratelimitseverity", 0, eCmdHdlrInt,
		NULL, &cs.ratelimitSeveritySysSock, STD_LOADABLE_MODULE_ID, &bLegacyCnfModGlobalsPermitted));
	
	/* support statistics gathering */
	CHKiRet(statsobj.Construct(&modStats));
	CHKiRet(statsobj.SetName(modStats, UCHAR_CONSTANT("imuxsock")));
	CHKiRet(statsobj.SetOrigin(modStats, UCHAR_CONSTANT("imuxsock")));
	STATSCOUNTER_INIT(ctrSubmit, mutCtrSubmit);
	CHKiRet(statsobj.AddCounter(modStats, UCHAR_CONSTANT("submitted"),
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &ctrSubmit));
	STATSCOUNTER_INIT(ctrLostRatelimit, mutCtrLostRatelimit);
	CHKiRet(statsobj.AddCounter(modStats, UCHAR_CONSTANT("ratelimit.discarded"),
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &ctrLostRatelimit));
	STATSCOUNTER_INIT(ctrNumRatelimiters, mutCtrNumRatelimiters);
	CHKiRet(statsobj.AddCounter(modStats, UCHAR_CONSTANT("ratelimit.numratelimiters"),
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &ctrNumRatelimiters));
	CHKiRet(statsobj.ConstructFinalize(modStats));

ENDmodInit
