/* omrelp.c
 *
 * This is the implementation of the RELP output module.
 *
 * Note that when multiple action workers are activated, we currently
 * also create multiple actions. This may be the source of some mild
 * message loss (!) if the worker instance is shut down while the
 * connection to the remote system is in retry state.
 * TODO: think if we should implement a mode where we do NOT
 *       support multiple action worker instances. This would be
 *       slower, but not have this loss opportunity. But it should
 *       definitely be optional and by default off due to the
 *       performance implications (and given the fact that message
 *       loss is pretty unlikely in usual cases).
 *
 *
 * File begun on 2008-03-13 by RGerhards
 *
 * Copyright 2008-2016 Adiscon GmbH.
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
#include "rsyslog.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <librelp.h>
#include "conf.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "cfsysline.h"
#include "module-template.h"
#include "glbl.h"
#include "errmsg.h"
#include "debug.h"
#include "unicode-helper.h"

#ifndef RELP_DFLT_PT
#	define RELP_DFLT_PT "514"
#endif

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("omrelp")

/* internal structures
 */
DEF_OMOD_STATIC_DATA
DEFobjCurrIf(glbl)

#define DFLT_ENABLE_TLS 0
#define DFLT_ENABLE_TLSZIP 0

static relpEngine_t *pRelpEngine;	/* our relp engine */

typedef struct _instanceData {
	uchar *target;
	uchar *port;
	int sizeWindow;		/**< the RELP window size - 0=use default */
	unsigned timeout;
	int connTimeout;
	unsigned rebindInterval;
	sbool bEnableTLS;
	sbool bEnableTLSZip;
	sbool bHadAuthFail;	/**< set on auth failure, will cause retry to disable action */
	uchar *pristring;		/* GnuTLS priority string (NULL if not to be provided) */
	uchar *authmode;
	uchar *caCertFile;
	uchar *myCertFile;
	uchar *myPrivKeyFile;
	uchar *tplName;
	uchar *localClientIP;
	struct {
		int nmemb;
		uchar **name;
	} permittedPeers;
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
	int bInitialConnect; /* is this the initial connection request of our module? (0-no, 1-yes) */
	int bIsConnected; /* currently connected to server? 0 - no, 1 - yes */
	relpClt_t *pRelpClt; /* relp client for this instance */
	unsigned nSent; /* number msgs sent - for rebind support */
} wrkrInstanceData_t;

typedef struct configSettings_s {
	EMPTY_STRUCT
} configSettings_t;
static configSettings_t __attribute__((unused)) cs;

static rsRetVal doCreateRelpClient(instanceData *pData, relpClt_t **pRelpClt);

/* tables for interfacing with the v6 config system */
/* action (instance) parameters */
static struct cnfparamdescr actpdescr[] = {
	{ "target", eCmdHdlrGetWord, 1 },
	{ "tls", eCmdHdlrBinary, 0 },
	{ "tls.compression", eCmdHdlrBinary, 0 },
	{ "tls.prioritystring", eCmdHdlrString, 0 },
	{ "tls.cacert", eCmdHdlrString, 0 },
	{ "tls.mycert", eCmdHdlrString, 0 },
	{ "tls.myprivkey", eCmdHdlrString, 0 },
	{ "tls.authmode", eCmdHdlrString, 0 },
	{ "tls.permittedpeer", eCmdHdlrArray, 0 },
	{ "port", eCmdHdlrGetWord, 0 },
	{ "rebindinterval", eCmdHdlrInt, 0 },
	{ "windowsize", eCmdHdlrInt, 0 },
	{ "timeout", eCmdHdlrInt, 0 },
	{ "conn.timeout", eCmdHdlrInt, 0 },
	{ "localclientip", eCmdHdlrGetWord, 0 },
	{ "template", eCmdHdlrGetWord, 0 }
};
static struct cnfparamblk actpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(actpdescr)/sizeof(struct cnfparamdescr),
	  actpdescr
	};

BEGINinitConfVars		/* (re)set config variables to default values */
CODESTARTinitConfVars
ENDinitConfVars

/* We may change the implementation to try to lookup the port
 * if it is unspecified. So far, we use 514 as default (what probably
 * is not a really bright idea, but kept for backward compatibility).
 */

#if !defined(_AIX)
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif
static void __attribute__((format(printf, 1, 2)))
omrelp_dbgprintf(const char *fmt, ...)
{
	va_list ap;
	char pszWriteBuf[32*1024+1]; //this function has to be able to
					/*generate a buffer longer than that of r_dbgprintf, so
					r_dbgprintf can properly truncate*/
	if(!(Debug && debugging_on)) {
		return;
	}

	va_start(ap, fmt);
	vsnprintf(pszWriteBuf, sizeof(pszWriteBuf), fmt, ap);
	va_end(ap);
	r_dbgprintf("omrelp.c", "%s", pszWriteBuf);
}
#if !defined(_AIX)
#pragma GCC diagnostic warning "-Wformat-nonliteral"
#endif


static uchar *getRelpPt(instanceData *pData)
{
	assert(pData != NULL);
	if(pData->port == NULL)
		return((uchar*)RELP_DFLT_PT);
	else
		return(pData->port);
}

static void
onErr(void *pUsr, char *objinfo, char* errmesg, __attribute__((unused)) relpRetVal errcode)
{
	wrkrInstanceData_t *pWrkrData = (wrkrInstanceData_t*) pUsr;
	LogError(0, RS_RET_RELP_AUTH_FAIL, "omrelp[%s:%s]: error '%s', object "
			" '%s' - action may not work as intended",
			pWrkrData->pData->target, pWrkrData->pData->port, errmesg, objinfo);
}

static void
onGenericErr(char *objinfo, char* errmesg, __attribute__((unused)) relpRetVal errcode)
{
	LogError(0, RS_RET_RELP_ERR, "omrelp: librelp error '%s', object "
			"'%s' - action may not work as intended",
			errmesg, objinfo);
}

static void
onAuthErr(void *pUsr, char *authinfo, char* errmesg, __attribute__((unused)) relpRetVal errcode)
{
	instanceData *pData = ((wrkrInstanceData_t*) pUsr)->pData;
	LogError(0, RS_RET_RELP_AUTH_FAIL, "omrelp[%s:%s]: authentication error '%s', peer "
			"is '%s' - DISABLING action", pData->target, pData->port, errmesg, authinfo);
	pData->bHadAuthFail = 1;
}

static rsRetVal
doCreateRelpClient(instanceData *pData, relpClt_t **pRelpClt)
{
	int i;
	DEFiRet;

	if(relpEngineCltConstruct(pRelpEngine, pRelpClt) != RELP_RET_OK)
		ABORT_FINALIZE(RS_RET_RELP_ERR);
	if(relpCltSetTimeout(*pRelpClt, pData->timeout) != RELP_RET_OK)
		ABORT_FINALIZE(RS_RET_RELP_ERR);
	if(relpCltSetConnTimeout(*pRelpClt, pData->connTimeout) != RELP_RET_OK) {
		ABORT_FINALIZE(RS_RET_RELP_ERR);
	}
	if(relpCltSetWindowSize(*pRelpClt, pData->sizeWindow) != RELP_RET_OK)
		ABORT_FINALIZE(RS_RET_RELP_ERR);
	if(pData->bEnableTLS) {
		if(relpCltEnableTLS(*pRelpClt) != RELP_RET_OK)
			ABORT_FINALIZE(RS_RET_RELP_ERR);
		if(pData->bEnableTLSZip) {
			if(relpCltEnableTLSZip(*pRelpClt) != RELP_RET_OK)
				ABORT_FINALIZE(RS_RET_RELP_ERR);
		}
		if(relpCltSetGnuTLSPriString(*pRelpClt, (char*) pData->pristring) != RELP_RET_OK)
			ABORT_FINALIZE(RS_RET_RELP_ERR);


		if(relpCltSetAuthMode(*pRelpClt, (char*) pData->authmode) != RELP_RET_OK) {
			LogError(0, RS_RET_RELP_ERR,
					"omrelp: invalid auth mode '%s'\n", pData->authmode);
			ABORT_FINALIZE(RS_RET_RELP_ERR);
		}

		if(relpCltSetCACert(*pRelpClt, (char*) pData->caCertFile) != RELP_RET_OK)
			ABORT_FINALIZE(RS_RET_RELP_ERR);
		if(relpCltSetOwnCert(*pRelpClt, (char*) pData->myCertFile) != RELP_RET_OK)
			ABORT_FINALIZE(RS_RET_RELP_ERR);
		if(relpCltSetPrivKey(*pRelpClt, (char*) pData->myPrivKeyFile) != RELP_RET_OK)
			ABORT_FINALIZE(RS_RET_RELP_ERR);
		for(i = 0 ; i <  pData->permittedPeers.nmemb ; ++i) {
			relpCltAddPermittedPeer(*pRelpClt, (char*)pData->permittedPeers.name[i]);
		}
	}
	if(pData->localClientIP != NULL) {
		if(relpCltSetClientIP(*pRelpClt, pData->localClientIP) != RELP_RET_OK)
			ABORT_FINALIZE(RS_RET_RELP_ERR);
	}
finalize_it:

	RETiRet;
}

BEGINcreateInstance
CODESTARTcreateInstance
	pData->sizeWindow = 0;
	pData->timeout = 90;
	pData->connTimeout = 10;
	pData->rebindInterval = 0;
	pData->bEnableTLS = DFLT_ENABLE_TLS;
	pData->bEnableTLSZip = DFLT_ENABLE_TLSZIP;
	pData->bHadAuthFail = 0;
	pData->pristring = NULL;
	pData->authmode = NULL;
	pData->localClientIP = NULL;
	pData->caCertFile = NULL;
	pData->myCertFile = NULL;
	pData->myPrivKeyFile = NULL;
	pData->permittedPeers.nmemb = 0;
ENDcreateInstance

BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
	pWrkrData->pRelpClt = NULL;
	iRet = doCreateRelpClient(pWrkrData->pData, &pWrkrData->pRelpClt);
	if(relpCltSetUsrPtr(pWrkrData->pRelpClt, pWrkrData) != RELP_RET_OK)
		LogError(0, RS_RET_NO_ERRCODE, "omrelp: error when creating relp client");
	pWrkrData->bInitialConnect = 1;
	pWrkrData->nSent = 0;
ENDcreateWrkrInstance

BEGINfreeInstance
	int i;
CODESTARTfreeInstance
	free(pData->target);
	free(pData->port);
	free(pData->tplName);
	free(pData->pristring);
	free(pData->authmode);
	free(pData->localClientIP);
	free(pData->caCertFile);
	free(pData->myCertFile);
	free(pData->myPrivKeyFile);
	if(pData->permittedPeers.name != NULL) {
		for(i = 0 ; i <  pData->permittedPeers.nmemb ; ++i) {
			free(pData->permittedPeers.name[i]);
		}
	}
ENDfreeInstance

BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
	if(pWrkrData->pRelpClt != NULL)
		relpEngineCltDestruct(pRelpEngine, &pWrkrData->pRelpClt);
ENDfreeWrkrInstance

static void
setInstParamDefaults(instanceData *pData)
{
	pData->target = NULL;
	pData->port = NULL;
	pData->tplName = NULL;
	pData->timeout = 90;
	pData->connTimeout = 10;
	pData->sizeWindow = 0;
	pData->rebindInterval = 0;
	pData->bEnableTLS = DFLT_ENABLE_TLS;
	pData->bEnableTLSZip = DFLT_ENABLE_TLSZIP;
	pData->pristring = NULL;
	pData->authmode = NULL;
	if(glbl.GetSourceIPofLocalClient() == NULL)
		pData->localClientIP = NULL;
	else
		pData->localClientIP = (uchar*)strdup((char*)glbl.GetSourceIPofLocalClient());
	pData->caCertFile = NULL;
	pData->myCertFile = NULL;
	pData->myPrivKeyFile = NULL;
	pData->permittedPeers.name = NULL;
	pData->permittedPeers.nmemb = 0;
}


BEGINnewActInst
	struct cnfparamvals *pvals;
	int i,j;
	FILE *fp;
	relpClt_t *pRelpClt = NULL;
CODESTARTnewActInst
	if((pvals = nvlstGetParams(lst, &actpblk, NULL)) == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	CHKiRet(createInstance(&pData));
	setInstParamDefaults(pData);

	for(i = 0 ; i < actpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(actpblk.descr[i].name, "target")) {
			pData->target = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "port")) {
			pData->port = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "template")) {
			pData->tplName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "localclientip")) {
			pData->localClientIP = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "timeout")) {
			pData->timeout = (unsigned) pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "conn.timeout")) {
			pData->connTimeout = (int) pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "rebindinterval")) {
			pData->rebindInterval = (unsigned) pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "windowsize")) {
			pData->sizeWindow = (int) pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "tls")) {
			pData->bEnableTLS = (unsigned) pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "tls.compression")) {
			pData->bEnableTLSZip = (unsigned) pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "tls.prioritystring")) {
			pData->pristring = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "tls.cacert")) {
			pData->caCertFile = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
			fp = fopen((const char*)pData->caCertFile, "r");
			if(fp == NULL) {
				char errStr[1024];
				rs_strerror_r(errno, errStr, sizeof(errStr));
				LogError(0, RS_RET_NO_FILE_ACCESS,
				"error: certificate file %s couldn't be accessed: %s\n",
				pData->caCertFile, errStr);
			} else {
				fclose(fp);
			}
		} else if(!strcmp(actpblk.descr[i].name, "tls.mycert")) {
			pData->myCertFile = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
			fp = fopen((const char*)pData->myCertFile, "r");
			if(fp == NULL) {
				char errStr[1024];
				rs_strerror_r(errno, errStr, sizeof(errStr));
				LogError(0, RS_RET_NO_FILE_ACCESS,
				"error: certificate file %s couldn't be accessed: %s\n",
				pData->myCertFile, errStr);
			} else {
				fclose(fp);
			}
		} else if(!strcmp(actpblk.descr[i].name, "tls.myprivkey")) {
			pData->myPrivKeyFile = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
			fp = fopen((const char*)pData->myPrivKeyFile, "r");
			if(fp == NULL) {
				char errStr[1024];
				rs_strerror_r(errno, errStr, sizeof(errStr));
				LogError(0, RS_RET_NO_FILE_ACCESS,
				"error: certificate file %s couldn't be accessed: %s\n",
				pData->myPrivKeyFile, errStr);
			} else {
				fclose(fp);
			}
		} else if(!strcmp(actpblk.descr[i].name, "tls.authmode")) {
			pData->authmode = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "tls.permittedpeer")) {
			pData->permittedPeers.nmemb = pvals[i].val.d.ar->nmemb;
			CHKmalloc(pData->permittedPeers.name =
				malloc(sizeof(uchar*) * pData->permittedPeers.nmemb));
			for(j = 0 ; j <  pData->permittedPeers.nmemb ; ++j) {
				pData->permittedPeers.name[j] = (uchar*)es_str2cstr(pvals[i].val.d.ar->arr[j], NULL);
			}
		} else {
			dbgprintf("omrelp: program error, non-handled "
			  "param '%s'\n", actpblk.descr[i].name);
		}
	}
	
	CODE_STD_STRING_REQUESTnewActInst(1)

	CHKiRet(OMSRsetEntry(*ppOMSR, 0, (uchar*)strdup((pData->tplName == NULL) ?
			    "RSYSLOG_ForwardFormat" : (char*)pData->tplName),
	   		    OMSR_NO_RQD_TPL_OPTS));

	iRet = doCreateRelpClient(pData, &pRelpClt);
	if(pRelpClt != NULL)
		relpEngineCltDestruct(pRelpEngine, &pRelpClt);

CODE_STD_FINALIZERnewActInst
	if(pvals != NULL)
		cnfparamvalsDestruct(pvals, &actpblk);
ENDnewActInst

BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	if(eFeat == sFEATURERepeatedMsgReduction)
		iRet = RS_RET_OK;
ENDisCompatibleWithFeature

BEGINSetShutdownImmdtPtr
CODESTARTSetShutdownImmdtPtr
	relpEngineSetShutdownImmdtPtr(pRelpEngine, pPtr);
	DBGPRINTF("omrelp: shutdownImmediate ptr now is %p\n", pPtr);
ENDSetShutdownImmdtPtr


BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
	dbgprintf("RELP/%s", pData->target);
ENDdbgPrintInstInfo


/* try to connect to server
 * rgerhards, 2008-03-21
 */
static rsRetVal ATTR_NONNULL()
doConnect(wrkrInstanceData_t *const pWrkrData)
{
	DEFiRet;

	if(pWrkrData->bInitialConnect) {
		iRet = relpCltConnect(pWrkrData->pRelpClt, glbl.GetDefPFFamily(),
				      getRelpPt(pWrkrData->pData), pWrkrData->pData->target);
		if(iRet == RELP_RET_OK)
			pWrkrData->bInitialConnect = 0;
	} else {
		iRet = relpCltReconnect(pWrkrData->pRelpClt);
	}

	if(iRet == RELP_RET_OK) {
		pWrkrData->bIsConnected = 1;
	} else if(iRet == RELP_RET_ERR_NO_TLS) {
		LogError(0, iRet, "omrelp: Could not connect, librelp does NOT "
				"does not support TLS (most probably GnuTLS lib "
				"is too old)!");
		FINALIZE;
	} else if(iRet == RELP_RET_ERR_NO_TLS_AUTH) {
		LogError(0, iRet,
				"omrelp: could not activate relp TLS with "
				"authentication, librelp does not support it "
				"(most probably GnuTLS lib is too old)! "
				"Note: anonymous TLS is probably supported.");
		FINALIZE;
	} else {
		pWrkrData->bIsConnected = 0;
		iRet = RS_RET_SUSPENDED;
	}

finalize_it:
	RETiRet;
}


BEGINtryResume
CODESTARTtryResume
	if(pWrkrData->pData->bHadAuthFail) {
		ABORT_FINALIZE(RS_RET_DISABLE_ACTION);
	}
	iRet = doConnect(pWrkrData);
finalize_it:
ENDtryResume

static rsRetVal
doRebind(wrkrInstanceData_t *pWrkrData)
{
	DEFiRet;
	DBGPRINTF("omrelp: destructing relp client due to rebindInterval\n");
	CHKiRet(relpEngineCltDestruct(pRelpEngine, &pWrkrData->pRelpClt));
	pWrkrData->bIsConnected = 0;
	CHKiRet(doCreateRelpClient(pWrkrData->pData, &pWrkrData->pRelpClt));
	if(relpCltSetUsrPtr(pWrkrData->pRelpClt, pWrkrData) != RELP_RET_OK)
		LogError(0, RS_RET_NO_ERRCODE, "omrelp: error when creating relp client");
	pWrkrData->bInitialConnect = 1;
	pWrkrData->nSent = 0;
finalize_it:
	RETiRet;
}

BEGINbeginTransaction
CODESTARTbeginTransaction
	DBGPRINTF("omrelp: beginTransaction\n");
	if(!pWrkrData->bIsConnected) {
		CHKiRet(doConnect(pWrkrData));
	}
	relpCltHintBurstBegin(pWrkrData->pRelpClt);
finalize_it:
ENDbeginTransaction

BEGINdoAction
	uchar *pMsg; /* temporary buffering */
	size_t lenMsg;
	relpRetVal ret;
	instanceData *pData;
CODESTARTdoAction
	pData = pWrkrData->pData;
	dbgprintf(" %s:%s/RELP\n", pData->target, getRelpPt(pData));

	if(!pWrkrData->bIsConnected) {
		CHKiRet(doConnect(pWrkrData));
	}

	pMsg = ppString[0];
	lenMsg = strlen((char*) pMsg); /* TODO: don't we get this? */

	/* we need to truncate oversize msgs - no way around that... */
	if((int) lenMsg > glbl.GetMaxLine())
		lenMsg = glbl.GetMaxLine();

	/* forward */
	ret = relpCltSendSyslog(pWrkrData->pRelpClt, (uchar*) pMsg, lenMsg);
	if(ret != RELP_RET_OK) {
		/* error! */
		dbgprintf("error forwarding via relp, suspending\n");
		ABORT_FINALIZE(RS_RET_SUSPENDED);
	}

	if(pData->rebindInterval != 0 &&
	   (++pWrkrData->nSent >= pData->rebindInterval)) {
	   	doRebind(pWrkrData);
	}
finalize_it:
	if(pData->bHadAuthFail)
		iRet = RS_RET_DISABLE_ACTION;
	if(iRet == RS_RET_OK) {
		/* we mimic non-commit, as otherwise our endTransaction handler
		 * will not get called. While this is not 100% correct, the worst
		 * that can happen is some message duplication, something that
		 * rsyslog generally accepts and prefers over message loss.
		 */
		iRet = RS_RET_PREVIOUS_COMMITTED;
	}
ENDdoAction


BEGINendTransaction
CODESTARTendTransaction
	DBGPRINTF("omrelp: endTransaction, connected %d\n", pWrkrData->bIsConnected);
	if(pWrkrData->bIsConnected) {
		relpCltHintBurstEnd(pWrkrData->pRelpClt);
	}
ENDendTransaction

BEGINparseSelectorAct
	uchar *q;
	int i;
	int bErr;
CODESTARTparseSelectorAct
CODE_STD_STRING_REQUESTparseSelectorAct(1)
	if(!strncmp((char*) p, ":omrelp:", sizeof(":omrelp:") - 1)) {
		p += sizeof(":omrelp:") - 1; /* eat indicator sequence (-1 because of '\0'!) */
	} else {
		ABORT_FINALIZE(RS_RET_CONFLINE_UNPROCESSED);
	}

	/* ok, if we reach this point, we have something for us */
	if((iRet = createInstance(&pData)) != RS_RET_OK)
		FINALIZE;

	/* extract the host first (we do a trick - we replace the ';' or ':' with a '\0')
	 * now skip to port and then template name. rgerhards 2005-07-06
	 */
	if(*p == '[') { /* everything is hostname upto ']' */
		++p; /* skip '[' */
		for(q = p ; *p && *p != ']' ; ++p)
			/* JUST SKIP */;
		if(*p == ']') {
			*p = '\0'; /* trick to obtain hostname (later)! */
			++p; /* eat it */
		}
	} else { /* traditional view of hostname */
		for(q = p ; *p && *p != ';' && *p != ':' && *p != '#' ; ++p)
			/* JUST SKIP */;
	}

	pData->port = NULL;
	if(*p == ':') { /* process port */
		uchar * tmp;

		*p = '\0'; /* trick to obtain hostname (later)! */
		tmp = ++p;
		for(i=0 ; *p && isdigit((int) *p) ; ++p, ++i)
			/* SKIP AND COUNT */;
		pData->port = MALLOC(i + 1);
		if(pData->port == NULL) {
			LogError(0, NO_ERRCODE, "Could not get memory to store relp port, "
				 "using default port, results may not be what you intend\n");
			/* we leave f_forw.port set to NULL, this is then handled by getRelpPt() */
		} else {
			memcpy(pData->port, tmp, i);
			*(pData->port + i) = '\0';
		}
	}
	
	/* now skip to template */
	bErr = 0;
	while(*p && *p != ';') {
		if(*p && *p != ';' && !isspace((int) *p)) {
			if(bErr == 0) { /* only 1 error msg! */
				bErr = 1;
				errno = 0;
				LogError(0, NO_ERRCODE, "invalid selector line (port), probably not doing "
					 "what was intended");
			}
		}
		++p;
	}

	if(*p == ';') {
		*p = '\0'; /* trick to obtain hostname (later)! */
		CHKmalloc(pData->target = ustrdup(q));
		*p = ';';
	} else {
		CHKmalloc(pData->target = ustrdup(q));
	}

	/* process template */
	CHKiRet(cflineParseTemplateName(&p, *ppOMSR, 0, OMSR_NO_RQD_TPL_OPTS, (uchar*) "RSYSLOG_ForwardFormat"));

CODE_STD_FINALIZERparseSelectorAct
ENDparseSelectorAct


BEGINmodExit
CODESTARTmodExit
	relpEngineDestruct(&pRelpEngine);

	/* release what we no longer need */
	objRelease(glbl, CORE_COMPONENT);
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_OMOD_QUERIES
CODEqueryEtryPt_STD_OMOD8_QUERIES
CODEqueryEtryPt_STD_CONF2_CNFNAME_QUERIES
CODEqueryEtryPt_STD_CONF2_OMOD_QUERIES
CODEqueryEtryPt_TXIF_OMOD_QUERIES
CODEqueryEtryPt_SetShutdownImmdtPtr
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
INITLegCnfVars
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	/* create our relp engine */
	CHKiRet(relpEngineConstruct(&pRelpEngine));
	CHKiRet(relpEngineSetDbgprint(pRelpEngine, (void (*)(char *, ...))omrelp_dbgprintf));
	CHKiRet(relpEngineSetOnAuthErr(pRelpEngine, onAuthErr));
	CHKiRet(relpEngineSetOnGenericErr(pRelpEngine, onGenericErr));
	CHKiRet(relpEngineSetOnErr(pRelpEngine, onErr));
	CHKiRet(relpEngineSetEnableCmd(pRelpEngine, (uchar*) "syslog", eRelpCmdState_Required));

	/* tell which objects we need */
	CHKiRet(objUse(glbl, CORE_COMPONENT));
ENDmodInit
