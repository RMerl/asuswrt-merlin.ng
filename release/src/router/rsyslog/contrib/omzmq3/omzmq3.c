/* omzmq3.c
 * Copyright 2012 Talksum, Inc
 * Using the czmq interface to zeromq, we output
 * to a zmq socket.
 * Copyright (C) 2014 Rainer Gerhards
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Author: David Kelly
 * <davidk@talksum.com>
 */


#include "config.h"
#include "rsyslog.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include "conf.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "template.h"
#include "module-template.h"
#include "errmsg.h"
#include "cfsysline.h"

#include <czmq.h>

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("omzmq3")

DEF_OMOD_STATIC_DATA

static pthread_mutex_t mutDoAct = PTHREAD_MUTEX_INITIALIZER;

/* convienent symbols to denote a socket we want to bind
 * vs one we want to just connect to
 */
#define ACTION_CONNECT 1
#define ACTION_BIND    2


/* ----------------------------------------------------------------------------
 * structs to describe sockets
 */
struct socket_type {
	const char*  name;
	int type;
};

/* more overkill, but seems nice to be consistent. */
struct socket_action {
	const char* name;
	int action;
};

typedef struct _instanceData {
	void* socket;
	uchar* description;
	int type;
	int action;
	int sndHWM;
	int rcvHWM;
	uchar* identity;
	int sndBuf;
	int rcvBuf;
	int linger;
	int backlog;
	int sndTimeout;
	int rcvTimeout;
	int maxMsgSize;
	int rate;
	int recoveryIVL;
	int multicastHops;
	int reconnectIVL;
	int reconnectIVLMax;
	int ipv4Only;
	int affinity;
	uchar* tplName;
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
} wrkrInstanceData_t;


/* ----------------------------------------------------------------------------
 * Static definitions/initializations
 */

/* only 1 zctx for all the sockets, with an adjustable number of
 * worker threads which may be useful if we use affinity in particular
 * sockets
 */
static zctx_t* s_context       = NULL;
static int     s_workerThreads = -1;

static struct socket_type types[] = {
	{"PUB",    ZMQ_PUB  },
	{"PUSH",   ZMQ_PUSH },
	{"DEALER", ZMQ_DEALER },
	{"XPUB",   ZMQ_XPUB }
};

static struct socket_action actions[] = {
	{"BIND",    ACTION_BIND},
	{"CONNECT", ACTION_CONNECT},
};

static struct cnfparamdescr actpdescr[] = {
	{ "description",         eCmdHdlrGetWord, 0 },
	{ "sockType",            eCmdHdlrGetWord, 0 },
	{ "action",              eCmdHdlrGetWord, 0 },
	{ "sndHWM",              eCmdHdlrInt,     0 },
	{ "rcvHWM",              eCmdHdlrInt,     0 },
	{ "identity",            eCmdHdlrGetWord, 0 },
	{ "sndBuf",              eCmdHdlrInt,     0 },
	{ "rcvBuf",              eCmdHdlrInt,     0 },
	{ "linger",              eCmdHdlrInt,     0 },
	{ "backlog",             eCmdHdlrInt,     0 },
	{ "sndTimeout",          eCmdHdlrInt,     0 },
	{ "rcvTimeout",          eCmdHdlrInt,     0 },
	{ "maxMsgSize",          eCmdHdlrInt,     0 },
	{ "rate",                eCmdHdlrInt,     0 },
	{ "recoveryIVL",         eCmdHdlrInt,     0 },
	{ "multicastHops",       eCmdHdlrInt,     0 },
	{ "reconnectIVL",        eCmdHdlrInt,     0 },
	{ "reconnectIVLMax",     eCmdHdlrInt,     0 },
	{ "ipv4Only",            eCmdHdlrInt,     0 },
	{ "affinity",            eCmdHdlrInt,     0 },
	{ "globalWorkerThreads", eCmdHdlrInt,     0 },
	{ "template",            eCmdHdlrGetWord, 1 }
};

static struct cnfparamblk actpblk = {
	CNFPARAMBLK_VERSION,
	sizeof(actpdescr)/sizeof(struct cnfparamdescr),
	actpdescr
};

/* ----------------------------------------------------------------------------
 * Helper Functions
 */

/* get the name of a socket type, return the ZMQ_XXX type
 * or -1 if not a supported type (see above)
 */
static int getSocketType(char* name) {
	int type = -1;
	uint i;
	for(i=0; i<sizeof(types)/sizeof(struct socket_type); ++i) {
	    if( !strcmp(types[i].name, name) ) {
	        type = types[i].type;
	        break;
	    }
	}
	return type;
}


static int getSocketAction(char* name) {
	int action = -1;
	uint i;
	for(i=0; i < sizeof(actions)/sizeof(struct socket_action); ++i) {
	    if(!strcmp(actions[i].name, name)) {
	        action = actions[i].action;
	        break;
	    }
	}
	return action;
}

/* closeZMQ will destroy the context and
 * associated socket
 */
static void closeZMQ(instanceData* pData) {
	LogError(0, NO_ERRCODE, "closeZMQ called");
	if(s_context && pData->socket) {
	    if(pData->socket != NULL) {
	        zsocket_destroy(s_context, pData->socket);
	    }
	}
}


static rsRetVal initZMQ(instanceData* pData) {
	DEFiRet;

	/* create the context if necessary. */
	if (NULL == s_context) {
	    zsys_handler_set(NULL);
	    s_context = zctx_new();
	    if (s_workerThreads > 0) zctx_set_iothreads(s_context, s_workerThreads);
	}

	pData->socket = zsocket_new(s_context, pData->type);
	if (NULL == pData->socket) {
	    LogError(0, RS_RET_NO_ERRCODE,
	                    "omzmq3: zsocket_new failed for %s: %s",
	                    pData->description, zmq_strerror(errno));
	    ABORT_FINALIZE(RS_RET_NO_ERRCODE);
	}
	/* use czmq defaults for these, unless set to non-default values */
	if(pData->identity)             zsocket_set_identity(pData->socket, (char*)pData->identity);
	if(pData->sndBuf > -1)          zsocket_set_sndbuf(pData->socket, pData->sndBuf);
	if(pData->rcvBuf > -1)          zsocket_set_sndbuf(pData->socket, pData->rcvBuf);
	if(pData->linger > -1)          zsocket_set_linger(pData->socket, pData->linger);
	if(pData->backlog > -1)         zsocket_set_backlog(pData->socket, pData->backlog);
	if(pData->sndTimeout > -1)      zsocket_set_sndtimeo(pData->socket, pData->sndTimeout);
	if(pData->rcvTimeout > -1)      zsocket_set_rcvtimeo(pData->socket, pData->rcvTimeout);
	if(pData->maxMsgSize > -1)      zsocket_set_maxmsgsize(pData->socket, pData->maxMsgSize);
	if(pData->rate > -1)            zsocket_set_rate(pData->socket, pData->rate);
	if(pData->recoveryIVL > -1)     zsocket_set_recovery_ivl(pData->socket, pData->recoveryIVL);
	if(pData->multicastHops > -1)   zsocket_set_multicast_hops(pData->socket, pData->multicastHops);
	if(pData->reconnectIVL > -1)    zsocket_set_reconnect_ivl(pData->socket, pData->reconnectIVL);
	if(pData->reconnectIVLMax > -1) zsocket_set_reconnect_ivl_max(pData->socket, pData->reconnectIVLMax);
	if(pData->ipv4Only > -1)        zsocket_set_ipv4only(pData->socket, pData->ipv4Only);
	if(pData->affinity != 1)        zsocket_set_affinity(pData->socket, pData->affinity);
	if(pData->rcvHWM > -1)          zsocket_set_rcvhwm(pData->socket, pData->rcvHWM);
	if(pData->sndHWM > -1)          zsocket_set_sndhwm(pData->socket, pData->sndHWM);

	/* bind or connect to it */
	if (pData->action == ACTION_BIND) {
	    /* bind asserts, so no need to test return val here
	       which isn't the greatest api -- oh well */
	    if(-1 == zsocket_bind(pData->socket, "%s", (char*)pData->description)) {
	        LogError(0, RS_RET_NO_ERRCODE, "omzmq3: bind failed for %s: %s",
	                        pData->description, zmq_strerror(errno));
	        ABORT_FINALIZE(RS_RET_NO_ERRCODE);
	    }
	    DBGPRINTF("omzmq3: bind to %s successful\n",pData->description);
	} else {
	    if(-1 == zsocket_connect(pData->socket, "%s", (char*)pData->description)) {
	        LogError(0, RS_RET_NO_ERRCODE, "omzmq3: connect failed for %s: %s",
	                        pData->description, zmq_strerror(errno));
	        ABORT_FINALIZE(RS_RET_NO_ERRCODE);
	    }
	    DBGPRINTF("omzmq3: connect to %s successful", pData->description);
	}
finalize_it:
	RETiRet;
}

static rsRetVal writeZMQ(uchar* msg, instanceData* pData) {
	DEFiRet;

	/* initialize if necessary */
	if(NULL == pData->socket)
		CHKiRet(initZMQ(pData));

	/* send it */
	int result = zstr_send(pData->socket, (char*)msg);

	/* whine if things went wrong */
	if (result == -1) {
	    LogError(0, NO_ERRCODE, "omzmq3: send of %s failed: %s", msg, zmq_strerror(errno));
	    ABORT_FINALIZE(RS_RET_ERR);
	}
finalize_it:
	RETiRet;
}

static void
setInstParamDefaults(instanceData* pData) {
	pData->description     = NULL;
	pData->socket          = NULL;
	pData->tplName         = NULL;
	pData->type            = ZMQ_PUB;
	pData->action          = ACTION_BIND;
	pData->sndHWM          = -1;
	pData->rcvHWM          = -1;
	pData->identity        = NULL;
	pData->sndBuf          = -1;
	pData->rcvBuf          = -1;
	pData->linger          = -1;
	pData->backlog         = -1;
	pData->sndTimeout      = -1;
	pData->rcvTimeout      = -1;
	pData->maxMsgSize      = -1;
	pData->rate            = -1;
	pData->recoveryIVL     = -1;
	pData->multicastHops   = -1;
	pData->reconnectIVL    = -1;
	pData->reconnectIVLMax = -1;
	pData->ipv4Only        = -1;
	pData->affinity        =  1;
}


/* ----------------------------------------------------------------------------
 * Output Module Functions
 */

BEGINcreateInstance
CODESTARTcreateInstance
ENDcreateInstance


BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
ENDcreateWrkrInstance

BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	if(eFeat == sFEATURERepeatedMsgReduction)
		iRet = RS_RET_OK;
ENDisCompatibleWithFeature


BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
ENDdbgPrintInstInfo

BEGINfreeInstance
CODESTARTfreeInstance
	closeZMQ(pData);
	free(pData->description);
	free(pData->tplName);
	free(pData->identity);
ENDfreeInstance


BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance


BEGINtryResume
CODESTARTtryResume
	pthread_mutex_lock(&mutDoAct);
	if(NULL == pWrkrData->pData->socket)
		iRet = initZMQ(pWrkrData->pData);
	pthread_mutex_unlock(&mutDoAct);
ENDtryResume

BEGINdoAction
	instanceData *pData = pWrkrData->pData;
CODESTARTdoAction
	pthread_mutex_lock(&mutDoAct);
	iRet = writeZMQ(ppString[0], pData);
	pthread_mutex_unlock(&mutDoAct);
ENDdoAction


BEGINnewActInst
	struct cnfparamvals *pvals;
	int i;
CODESTARTnewActInst
	if ((pvals = nvlstGetParams(lst, &actpblk, NULL)) == NULL) {
	    ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	CHKiRet(createInstance(&pData));
	setInstParamDefaults(pData);

	CODE_STD_STRING_REQUESTnewActInst(1)
	for (i = 0; i < actpblk.nParams; ++i) {
	    if (!pvals[i].bUsed)
	        continue;
	    if (!strcmp(actpblk.descr[i].name, "description")) {
	        pData->description = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
	    } else if (!strcmp(actpblk.descr[i].name, "template")) {
	        pData->tplName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
	    } else if (!strcmp(actpblk.descr[i].name, "sockType")){
	        pData->type = getSocketType(es_str2cstr(pvals[i].val.d.estr, NULL));
	    } else if (!strcmp(actpblk.descr[i].name, "action")){
	        pData->action = getSocketAction(es_str2cstr(pvals[i].val.d.estr, NULL));
	    } else if (!strcmp(actpblk.descr[i].name, "sndHWM")) {
	        pData->sndHWM = (int) pvals[i].val.d.n;
	    } else if (!strcmp(actpblk.descr[i].name, "rcvHWM")) {
	        pData->rcvHWM = (int) pvals[i].val.d.n;
	    } else if (!strcmp(actpblk.descr[i].name, "identity")){
	        pData->identity = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
	    } else if (!strcmp(actpblk.descr[i].name, "sndBuf")) {
	        pData->sndBuf = (int) pvals[i].val.d.n;
	    } else if (!strcmp(actpblk.descr[i].name, "rcvBuf")) {
	        pData->rcvBuf = (int) pvals[i].val.d.n;
	    } else if(!strcmp(actpblk.descr[i].name, "linger")) {
	        pData->linger = (int) pvals[i].val.d.n;
	    } else if (!strcmp(actpblk.descr[i].name, "backlog")) {
	        pData->backlog = (int) pvals[i].val.d.n;
	    } else if (!strcmp(actpblk.descr[i].name, "sndTimeout")) {
	        pData->sndTimeout = (int) pvals[i].val.d.n;
	    } else if (!strcmp(actpblk.descr[i].name, "rcvTimeout")) {
	        pData->rcvTimeout = (int) pvals[i].val.d.n;
	    } else if (!strcmp(actpblk.descr[i].name, "maxMsgSize")) {
	        pData->maxMsgSize = (int) pvals[i].val.d.n;
	    } else if (!strcmp(actpblk.descr[i].name, "rate")) {
	        pData->rate = (int) pvals[i].val.d.n;
	    } else if (!strcmp(actpblk.descr[i].name, "recoveryIVL")) {
	        pData->recoveryIVL = (int) pvals[i].val.d.n;
	    } else if (!strcmp(actpblk.descr[i].name, "multicastHops")) {
	        pData->multicastHops = (int) pvals[i].val.d.n;
	    } else if (!strcmp(actpblk.descr[i].name, "reconnectIVL")) {
	        pData->reconnectIVL = (int) pvals[i].val.d.n;
	    } else if (!strcmp(actpblk.descr[i].name, "reconnectIVLMax")) {
	        pData->reconnectIVLMax = (int) pvals[i].val.d.n;
	    } else if (!strcmp(actpblk.descr[i].name, "ipv4Only")) {
	        pData->ipv4Only = (int) pvals[i].val.d.n;
	    } else if (!strcmp(actpblk.descr[i].name, "affinity")) {
	        pData->affinity = (int) pvals[i].val.d.n;
	    } else if (!strcmp(actpblk.descr[i].name, "globalWorkerThreads")) {
	        s_workerThreads = (int) pvals[i].val.d.n;
	    } else {
	        LogError(0, NO_ERRCODE, "omzmq3: program error, non-handled "
	                        "param '%s'\n", actpblk.descr[i].name);
	    }
	}

	if (pData->tplName == NULL) {
	    CHKiRet(OMSRsetEntry(*ppOMSR, 0, (uchar*)strdup("RSYSLOG_ForwardFormat"), OMSR_NO_RQD_TPL_OPTS));
	} else {
	    CHKiRet(OMSRsetEntry(*ppOMSR, 0, (uchar*)pData->tplName, OMSR_NO_RQD_TPL_OPTS));
	}
	if (NULL == pData->description) {
	    LogError(0, RS_RET_CONFIG_ERROR, "omzmq3: you didn't enter a description");
	    ABORT_FINALIZE(RS_RET_CONFIG_ERROR);
	}
	if (pData->type == -1) {
	    LogError(0, RS_RET_CONFIG_ERROR, "omzmq3: unknown socket type.");
	    ABORT_FINALIZE(RS_RET_CONFIG_ERROR);
	}
	if (pData->action == -1) {
	    LogError(0, RS_RET_CONFIG_ERROR, "omzmq3: unknown socket action");
	    ABORT_FINALIZE(RS_RET_CONFIG_ERROR);
	}

	CODE_STD_FINALIZERnewActInst
	cnfparamvalsDestruct(pvals, &actpblk);
ENDnewActInst

NO_LEGACY_CONF_parseSelectorAct

BEGINinitConfVars /* (re)set config variables to defaults */
CODESTARTinitConfVars
	s_workerThreads = -1;
ENDinitConfVars

BEGINmodExit
CODESTARTmodExit
	if (NULL != s_context) {
	    zctx_destroy(&s_context);
	    s_context=NULL;
	}
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
	CODEqueryEtryPt_STD_OMOD_QUERIES
	CODEqueryEtryPt_STD_CONF2_OMOD_QUERIES
	CODEqueryEtryPt_STD_OMOD8_QUERIES
ENDqueryEtryPt

BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* only supports rsyslog 6 configs */
CODEmodInit_QueryRegCFSLineHdlr
	LogError(0, RS_RET_DEPRECATED, "note: omzmq3 module is deprecated and will "
		"be removed soon. Do no longer use it, switch to imczmq. See "
		"https://github.com/rsyslog/rsyslog/issues/2103 for details.");
	INITChkCoreFeature(bCoreSupportsBatching, CORE_FEATURE_BATCHING);
	DBGPRINTF("omzmq3: module compiled with rsyslog version %s.\n", VERSION);

	INITLegCnfVars
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"omzmq3workerthreads", 0, eCmdHdlrInt, NULL, &s_workerThreads,
STD_LOADABLE_MODULE_ID));
ENDmodInit
