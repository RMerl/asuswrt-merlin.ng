/* imrelp.c
 *
 * This is the implementation of the RELP input module.
 *
 * File begun on 2008-03-13 by RGerhards
 *
 * Copyright 2008-2018 Adiscon GmbH.
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
#include <signal.h>
#include <librelp.h>
#include "rsyslog.h"
#include "dirty.h"
#include "errmsg.h"
#include "cfsysline.h"
#include "module-template.h"
#include "net.h"
#include "msg.h"
#include "unicode-helper.h"
#include "prop.h"
#include "ruleset.h"
#include "glbl.h"
#include "statsobj.h"
#include "srUtils.h"
#include "parserif.h"

MODULE_TYPE_INPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("imrelp")

/* static data */
DEF_IMOD_STATIC_DATA
DEFobjCurrIf(net)
DEFobjCurrIf(prop)
DEFobjCurrIf(ruleset)
DEFobjCurrIf(glbl)
DEFobjCurrIf(statsobj)

/* forward definitions */
static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal);


/* Module static data */
/* config vars for legacy config system */
static relpEngine_t *pRelpEngine;	/* our relp engine */

/* config settings */
typedef struct configSettings_s {
	uchar *pszBindRuleset;		/* name of Ruleset to bind to */
} configSettings_t;
static configSettings_t cs;

struct instanceConf_s {
	uchar *pszBindPort;		/* port to bind to */
	uchar *pszBindAddr;		/* address to bind to */
	uchar *pszBindRuleset;		/* name of ruleset to bind to */
	uchar *pszInputName;		/* value for inputname property */
	prop_t *pInputName;		/* InputName in property format for fast access */
	ruleset_t *pBindRuleset;	/* ruleset to bind listener to */
	sbool bKeepAlive;		/* support keep-alive packets */
	sbool bEnableTLS;
	sbool bEnableTLSZip;
	sbool bEnableLstn;		/* flag to permit disabling of listener in error case */
	int dhBits;
	size_t maxDataSize;
	int oversizeMode;
	uchar *pristring;		/* GnuTLS priority string (NULL if not to be provided) */
	uchar *authmode;		/* TLS auth mode */
	uchar *caCertFile;
	uchar *myCertFile;
	uchar *myPrivKeyFile;
	int iKeepAliveIntvl;
	int iKeepAliveProbes;
	int iKeepAliveTime;
	struct {
		int nmemb;
		uchar **name;
	} permittedPeers;

	struct instanceConf_s *next;
	/* with librelp, this module does not have any own specific session
	 * or listener active data item. As a "work-around", we keep some
	 * data items inside the configuration object. To keep things
	 * decently clean, we put them all into their dedicated struct. So
	 * it is easy to judge what is actual configuration and what is
	 * dynamic runtime data. -- rgerhards, 2013-06-18
	 */
	struct {
		statsobj_t *stats;	/* listener stats */
		STATSCOUNTER_DEF(ctrSubmit, mutCtrSubmit)
	} data;
};


struct modConfData_s {
	rsconf_t *pConf;		/* our overall config object */
	instanceConf_t *root, *tail;
	uchar *pszBindRuleset;		/* default name of Ruleset to bind to */
};

static modConfData_t *loadModConf = NULL;/* modConf ptr to use for the current load process */
static modConfData_t *runModConf = NULL;/* modConf ptr to use for the current load process */

/* module-global parameters */
static struct cnfparamdescr modpdescr[] = {
	{ "ruleset", eCmdHdlrGetWord, 0 },
};
static struct cnfparamblk modpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(modpdescr)/sizeof(struct cnfparamdescr),
	  modpdescr
	};

/* input instance parameters */
static struct cnfparamdescr inppdescr[] = {
	{ "port", eCmdHdlrString, CNFPARAM_REQUIRED },
	{ "address", eCmdHdlrString, 0 },
	{ "name", eCmdHdlrString, 0 },
	{ "ruleset", eCmdHdlrString, 0 },
	{ "keepalive", eCmdHdlrBinary, 0 },
	{ "keepalive.probes", eCmdHdlrInt, 0 },
	{ "keepalive.time", eCmdHdlrInt, 0 },
	{ "keepalive.interval", eCmdHdlrInt, 0 },
	{ "maxdatasize", eCmdHdlrSize, 0 },
	{ "oversizemode", eCmdHdlrString, 0 },
	{ "tls", eCmdHdlrBinary, 0 },
	{ "tls.permittedpeer", eCmdHdlrArray, 0 },
	{ "tls.authmode", eCmdHdlrString, 0 },
	{ "tls.dhbits", eCmdHdlrInt, 0 },
	{ "tls.prioritystring", eCmdHdlrString, 0 },
	{ "tls.cacert", eCmdHdlrString, 0 },
	{ "tls.mycert", eCmdHdlrString, 0 },
	{ "tls.myprivkey", eCmdHdlrString, 0 },
	{ "tls.compression", eCmdHdlrBinary, 0 }
};
static struct cnfparamblk inppblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(inppdescr)/sizeof(struct cnfparamdescr),
	  inppdescr
	};

#include "im-helper.h" /* must be included AFTER the type definitions! */
static int bLegacyCnfModGlobalsPermitted;/* are legacy module-global config parameters permitted? */

/* ------------------------------ callbacks ------------------------------ */

#if !defined(_AIX)
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif
static void __attribute__((format(printf, 1, 2)))
imrelp_dbgprintf(const char *fmt, ...)
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
	r_dbgprintf("imrelp.c", "%s", pszWriteBuf);
}
#if !defined(_AIX)
#pragma GCC diagnostic warning "-Wformat-nonliteral"
#endif


static void
onErr(void *pUsr, char *objinfo, char* errmesg, __attribute__((unused)) relpRetVal errcode)
{
	instanceConf_t *inst = (instanceConf_t*) pUsr;
	LogError(0, RS_RET_RELP_AUTH_FAIL, "imrelp[%s]: error '%s', object "
			" '%s' - input may not work as intended",
			inst->pszBindPort, errmesg, objinfo);
}

static void
onGenericErr(char *objinfo, char* errmesg, __attribute__((unused)) relpRetVal errcode)
{
	LogError(0, RS_RET_RELP_ERR, "imrelp: librelp error '%s', object "
			" '%s' - input may not work as intended", errmesg, objinfo);
}

static void
onAuthErr(void *pUsr, char *authinfo, char* errmesg, __attribute__((unused)) relpRetVal errcode)
{
	instanceConf_t *inst = (instanceConf_t*) pUsr;
	LogError(0, RS_RET_RELP_AUTH_FAIL, "imrelp[%s]: authentication error '%s', peer "
			"is '%s'", inst->pszBindPort, errmesg, authinfo);
}

/* callback for receiving syslog messages. This function is invoked from the
 * RELP engine when a syslog message arrived. It must return a relpRetVal,
 * with anything else but RELP_RET_OK terminating the relp session. Please note
 * that RELP_RET_OK is equal to RS_RET_OK and the other libRELP error codes
 * are different from our rsRetVal. So we can simply use our own iRet system
 * to fulfill the requirement.
 * rgerhards, 2008-03-21
 * Note: librelp 1.0.0 is required in order to receive the IP address, otherwise
 * we will only see the hostname (twice). -- rgerhards, 2009-10-14
 */
static relpRetVal
onSyslogRcv(void *pUsr, uchar *pHostname, uchar *pIP, uchar *msg, size_t lenMsg)
{
	prop_t *pProp = NULL;
	smsg_t *pMsg;
	instanceConf_t *inst = (instanceConf_t*) pUsr;
	DEFiRet;

	CHKiRet(msgConstruct(&pMsg));
	MsgSetInputName(pMsg, inst->pInputName);
	MsgSetRawMsg(pMsg, (char*)msg, lenMsg);
	MsgSetFlowControlType(pMsg, eFLOWCTL_LIGHT_DELAY);
	MsgSetRuleset(pMsg, inst->pBindRuleset);
	pMsg->msgFlags  = PARSE_HOSTNAME | NEEDS_PARSING;

	/* TODO: optimize this, we can store it inside the session */
	MsgSetRcvFromStr(pMsg, pHostname, ustrlen(pHostname), &pProp);
	CHKiRet(prop.Destruct(&pProp));
	CHKiRet(MsgSetRcvFromIPStr(pMsg, pIP, ustrlen(pIP), &pProp));
	CHKiRet(prop.Destruct(&pProp));
	CHKiRet(submitMsg2(pMsg));
	STATSCOUNTER_INC(inst->data.ctrSubmit, inst->data.mutCtrSubmit);

finalize_it:

	RETiRet;
}


/* ------------------------------ end callbacks ------------------------------ */

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
	inst->pszBindRuleset = NULL;
	inst->pszInputName = NULL;
	inst->pBindRuleset = NULL;
	inst->bKeepAlive = 0;
	inst->iKeepAliveIntvl = 0;
	inst->iKeepAliveProbes = 0;
	inst->iKeepAliveTime = 0;
	inst->bEnableTLS = 0;
	inst->bEnableTLSZip = 0;
	inst->bEnableLstn = 0;
	inst->dhBits = 0;
	inst->pristring = NULL;
	inst->authmode = NULL;
	inst->permittedPeers.nmemb = 0;
	inst->caCertFile = NULL;
	inst->myCertFile = NULL;
	inst->myPrivKeyFile = NULL;
	inst->maxDataSize = 0;
#ifdef HAVE_RELPSRVSETOVERSIZEMODE
	inst->oversizeMode = RELP_OVERSIZE_TRUNCATE;
#endif

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


/* function to generate an error message if the ruleset cannot be found */
static inline void
std_checkRuleset_genErrMsg(__attribute__((unused)) modConfData_t *modConf, instanceConf_t *inst)
{
	LogError(0, NO_ERRCODE, "imrelp[%s]: ruleset '%s' not found - "
			"using default ruleset instead",
			inst->pszBindPort, inst->pszBindRuleset);
}


/* This function is called when a new listener instance shall be added to
 * the current config object via the legacy config system. It just shuffles
 * all parameters to the listener in-memory instance.
 * rgerhards, 2011-05-04
 */
static rsRetVal addInstance(void __attribute__((unused)) *pVal, uchar *pNewVal)
{
	instanceConf_t *inst;
	DEFiRet;

	CHKiRet(createInstance(&inst));

	if(pNewVal == NULL || *pNewVal == '\0') {
		LogError(0, NO_ERRCODE, "imrelp: port number must be specified, listener ignored");
	}
	if((pNewVal == NULL) || (*pNewVal == '\0')) {
		inst->pszBindPort = NULL;
	} else {
		CHKmalloc(inst->pszBindPort = ustrdup(pNewVal));
	}
	if((cs.pszBindRuleset == NULL) || (cs.pszBindRuleset[0] == '\0')) {
		inst->pszBindRuleset = NULL;
	} else {
		CHKmalloc(inst->pszBindRuleset = ustrdup(cs.pszBindRuleset));
	}
	inst->pBindRuleset = NULL;
finalize_it:
	free(pNewVal);
	RETiRet;
}


static rsRetVal
addListner(modConfData_t __attribute__((unused)) *modConf, instanceConf_t *inst)
{
	relpSrv_t *pSrv;
	int relpRet;
	uchar statname[64];
	int i;
	DEFiRet;

	if(!inst->bEnableLstn) {
		DBGPRINTF("listener not started because it is disabled by config error\n");
		FINALIZE;
	}

	if(pRelpEngine == NULL) {
		CHKiRet(relpEngineConstruct(&pRelpEngine));
		CHKiRet(relpEngineSetDbgprint(pRelpEngine, (void (*)(char *, ...))imrelp_dbgprintf));
		CHKiRet(relpEngineSetFamily(pRelpEngine, glbl.GetDefPFFamily()));
		CHKiRet(relpEngineSetEnableCmd(pRelpEngine, (uchar*) "syslog", eRelpCmdState_Required));
		CHKiRet(relpEngineSetSyslogRcv2(pRelpEngine, onSyslogRcv));
		CHKiRet(relpEngineSetOnErr(pRelpEngine, onErr));
		CHKiRet(relpEngineSetOnGenericErr(pRelpEngine, onGenericErr));
		CHKiRet(relpEngineSetOnAuthErr(pRelpEngine, onAuthErr));
		if (!glbl.GetDisableDNS()) {
			CHKiRet(relpEngineSetDnsLookupMode(pRelpEngine, 1));
		}
	}

	CHKiRet(relpEngineListnerConstruct(pRelpEngine, &pSrv));
	CHKiRet(relpSrvSetLstnPort(pSrv, inst->pszBindPort));
	CHKiRet(relpSrvSetLstnAddr(pSrv, inst->pszBindAddr));
	CHKiRet(relpSrvSetMaxDataSize(pSrv, inst->maxDataSize));

#ifdef HAVE_RELPSRVSETOVERSIZEMODE
	CHKiRet(relpSrvSetOversizeMode(pSrv, inst->oversizeMode));
#endif
	inst->pszInputName = ustrdup((inst->pszInputName == NULL) ?  UCHAR_CONSTANT("imrelp") : inst->pszInputName);
	CHKiRet(prop.Construct(&inst->pInputName));
	CHKiRet(prop.SetString(inst->pInputName, inst->pszInputName, ustrlen(inst->pszInputName)));
	CHKiRet(prop.ConstructFinalize(inst->pInputName));
	/* support statistics gathering */
	CHKiRet(statsobj.Construct(&(inst->data.stats)));
	snprintf((char*)statname, sizeof(statname), "%s(%s)",
		 inst->pszInputName, inst->pszBindPort);
	statname[sizeof(statname)-1] = '\0'; /* just to be on the save side... */
	CHKiRet(statsobj.SetName(inst->data.stats, statname));
	CHKiRet(statsobj.SetOrigin(inst->data.stats, (uchar*)"imrelp"));
	STATSCOUNTER_INIT(inst->data.ctrSubmit, inst->data.mutCtrSubmit);
	CHKiRet(statsobj.AddCounter(inst->data.stats, UCHAR_CONSTANT("submitted"),
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &(inst->data.ctrSubmit)));
	CHKiRet(statsobj.ConstructFinalize(inst->data.stats));
	/* end stats counters */
	relpSrvSetUsrPtr(pSrv, inst);
	relpSrvSetKeepAlive(pSrv, inst->bKeepAlive, inst->iKeepAliveIntvl,
			    inst->iKeepAliveProbes, inst->iKeepAliveTime);
	if(inst->bEnableTLS) {
		relpRet = relpSrvEnableTLS2(pSrv);
		if(relpRet == RELP_RET_ERR_NO_TLS) {
			LogError(0, RS_RET_RELP_NO_TLS,
					"imrelp: could not activate relp TLS, librelp "
					"does not support it (most probably GnuTLS lib "
					"is too old)!");
			ABORT_FINALIZE(RS_RET_RELP_NO_TLS);
		} else if(relpRet == RELP_RET_ERR_NO_TLS_AUTH) {
			LogError(0, RS_RET_RELP_NO_TLS_AUTH,
					"imrelp: could not activate relp TLS with "
					"authentication, librelp does not support it "
					"(most probably GnuTLS lib is too old)! "
					"Note: anonymous TLS is probably supported.");
			ABORT_FINALIZE(RS_RET_RELP_NO_TLS_AUTH);
		} else if(relpRet != RELP_RET_OK) {
			LogError(0, RS_RET_RELP_ERR,
					"imrelp: could not activate relp TLS, code %d", relpRet);
			ABORT_FINALIZE(RS_RET_RELP_ERR);
		}
		if(inst->bEnableTLSZip) {
			relpSrvEnableTLSZip2(pSrv);
		}
		if(inst->dhBits) {
			relpSrvSetDHBits(pSrv, inst->dhBits);
		}
		relpSrvSetGnuTLSPriString(pSrv, (char*)inst->pristring);
		if(relpSrvSetAuthMode(pSrv, (char*)inst->authmode) != RELP_RET_OK) {
			LogError(0, RS_RET_RELP_ERR,
					"imrelp: invalid auth mode '%s'", inst->authmode);
			ABORT_FINALIZE(RS_RET_RELP_ERR);
		}
		if(relpSrvSetCACert(pSrv, (char*) inst->caCertFile) != RELP_RET_OK)
			ABORT_FINALIZE(RS_RET_RELP_ERR);
		if(relpSrvSetOwnCert(pSrv, (char*) inst->myCertFile) != RELP_RET_OK)
			ABORT_FINALIZE(RS_RET_RELP_ERR);
		if(relpSrvSetPrivKey(pSrv, (char*) inst->myPrivKeyFile) != RELP_RET_OK)
			ABORT_FINALIZE(RS_RET_RELP_ERR);
		for(i = 0 ; i <  inst->permittedPeers.nmemb ; ++i) {
			relpSrvAddPermittedPeer(pSrv, (char*)inst->permittedPeers.name[i]);
		}
	}
	relpRet = relpEngineListnerConstructFinalize(pRelpEngine, pSrv);
	/* re-check error TLS error codes. librelp seems to emit them only
	 * after finalize in some cases...
	 */
	if(relpRet == RELP_RET_ERR_NO_TLS) {
		LogError(0, RS_RET_RELP_NO_TLS,
				"imrelp: could not activate relp TLS listener, librelp "
				"does not support it (most probably GnuTLS lib "
				"is too old)!");
		ABORT_FINALIZE(RS_RET_RELP_NO_TLS);
	} else if(relpRet == RELP_RET_ERR_NO_TLS_AUTH) {
		LogError(0, RS_RET_RELP_NO_TLS_AUTH,
				"imrelp: could not activate relp TLS listener with "
				"authentication, librelp does not support it "
				"(most probably GnuTLS lib is too old)! "
				"Note: anonymous TLS is probably supported.");
		ABORT_FINALIZE(RS_RET_RELP_NO_TLS_AUTH);
	} else if(relpRet != RELP_RET_OK) {
		LogError(0, RS_RET_RELP_ERR,
				"imrelp: could not activate relp listener, code %d", relpRet);
		ABORT_FINALIZE(RS_RET_RELP_ERR);
	}

	DBGPRINTF("imrelp: max data size %zd\n", inst->maxDataSize);
	resetConfigVariables(NULL,NULL);

finalize_it:
	RETiRet;
}


BEGINnewInpInst
	struct cnfparamvals *pvals;
	instanceConf_t *inst = NULL;
	int i,j;
	FILE *fp;
CODESTARTnewInpInst
	DBGPRINTF("newInpInst (imrelp)\n");

	if((pvals = nvlstGetParams(lst, &inppblk, NULL)) == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	if(Debug) {
		dbgprintf("input param blk in imrelp:\n");
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
		} else if(!strcmp(inppblk.descr[i].name, "name")) {
			inst->pszInputName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "ruleset")) {
			inst->pszBindRuleset = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "maxdatasize")) {
			inst->maxDataSize = (size_t) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "oversizemode")) {
#ifdef HAVE_RELPSRVSETOVERSIZEMODE
			char *mode = es_str2cstr(pvals[i].val.d.estr, NULL);
			if(!strcmp(mode, "abort")) {
				inst->oversizeMode = RELP_OVERSIZE_ABORT;
			} else if(!strcmp(mode, "truncate")) {
				inst->oversizeMode = RELP_OVERSIZE_TRUNCATE;
			} else if(!strcmp(mode, "accept")) {
				inst->oversizeMode = RELP_OVERSIZE_ACCEPT;
			} else {
				parser_errmsg("imrelp: wrong oversizeMode parameter "
					"value %s, using default: truncate\n", mode);
				inst->oversizeMode = RELP_OVERSIZE_TRUNCATE;
			}
#else
			parser_errmsg("imrelp: parameter oversizeMode is not available in "
				"this relp version and is therefore disabled.");
#endif
		} else if(!strcmp(inppblk.descr[i].name, "keepalive")) {
			inst->bKeepAlive = (sbool) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "keepalive.probes")) {
			inst->iKeepAliveProbes = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "keepalive.time")) {
			inst->iKeepAliveTime = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "keepalive.interval")) {
			inst->iKeepAliveIntvl = (int) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "tls")) {
			inst->bEnableTLS = (unsigned) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "tls.dhbits")) {
			inst->dhBits = (unsigned) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "tls.prioritystring")) {
			inst->pristring = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "tls.authmode")) {
			inst->authmode = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "tls.compression")) {
			inst->bEnableTLSZip = (unsigned) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "tls.cacert")) {
			inst->caCertFile = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
			fp = fopen((const char*)inst->caCertFile, "r");
			if(fp == NULL) {
				char errStr[1024];
				rs_strerror_r(errno, errStr, sizeof(errStr));
				LogError(0, RS_RET_NO_FILE_ACCESS,
				"error: certificate file %s couldn't be accessed: %s\n",
				inst->caCertFile, errStr);
			} else {
				fclose(fp);
			}
		} else if(!strcmp(inppblk.descr[i].name, "tls.mycert")) {
			inst->myCertFile = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
			fp = fopen((const char*)inst->myCertFile, "r");
			if(fp == NULL) {
				char errStr[1024];
				rs_strerror_r(errno, errStr, sizeof(errStr));
				LogError(0, RS_RET_NO_FILE_ACCESS,
				"error: certificate file %s couldn't be accessed: %s\n",
				inst->myCertFile, errStr);
			} else {
				fclose(fp);
			}
		} else if(!strcmp(inppblk.descr[i].name, "tls.myprivkey")) {
			inst->myPrivKeyFile = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
			fp = fopen((const char*)inst->myPrivKeyFile, "r");
			if(fp == NULL) {
				char errStr[1024];
				rs_strerror_r(errno, errStr, sizeof(errStr));
				LogError(0, RS_RET_NO_FILE_ACCESS,
				"error: certificate file %s couldn't be accessed: %s\n",
				inst->myPrivKeyFile, errStr);
			} else {
				fclose(fp);
			}
		} else if(!strcmp(inppblk.descr[i].name, "tls.permittedpeer")) {
			inst->permittedPeers.nmemb = pvals[i].val.d.ar->nmemb;
			CHKmalloc(inst->permittedPeers.name =
				malloc(sizeof(uchar*) * inst->permittedPeers.nmemb));
			for(j = 0 ; j <  pvals[i].val.d.ar->nmemb ; ++j) {
				inst->permittedPeers.name[j] = (uchar*)es_str2cstr(pvals[i].val.d.ar->arr[j], NULL);
			}
		} else {
			dbgprintf("imrelp: program error, non-handled "
			  "param '%s'\n", inppblk.descr[i].name);
		}
	}

	if(inst->myCertFile  != NULL && inst->myPrivKeyFile == NULL) {
		LogError(0, RS_RET_ERR, "imrelp: certificate file given but no corresponding "
			"private key file - this is invalid, listener cannot be started");
		ABORT_FINALIZE(RS_RET_ERR);
	}
	if(inst->myCertFile  == NULL && inst->myPrivKeyFile != NULL) {
		LogError(0, RS_RET_ERR, "imrelp: private key file given but no corresponding "
			"certificate file - this is invalid, listener cannot be started");
		ABORT_FINALIZE(RS_RET_ERR);
	}

	inst->bEnableLstn = -1; /* all ok, ready to start up */

finalize_it:
CODE_STD_FINALIZERnewInpInst
	cnfparamvalsDestruct(pvals, &inppblk);
	if(iRet != RS_RET_OK) {
		if(inst != NULL) {
			free(inst->myCertFile);
			inst->myCertFile = NULL;
			free(inst->myPrivKeyFile);
			inst->myPrivKeyFile = NULL;
		}
	}
ENDnewInpInst


BEGINbeginCnfLoad
CODESTARTbeginCnfLoad
	loadModConf = pModConf;
	pModConf->pConf = pConf;
	pModConf->pszBindRuleset = NULL;
	/* init legacy config variables */
	cs.pszBindRuleset = NULL;
	bLegacyCnfModGlobalsPermitted = 1;
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
		dbgprintf("module (global) param blk for imrelp:\n");
		cnfparamsPrint(&modpblk, pvals);
	}

	for(i = 0 ; i < modpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(modpblk.descr[i].name, "ruleset")) {
			loadModConf->pszBindRuleset = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else {
			dbgprintf("imrelp: program error, non-handled "
			  "param '%s' in beginCnfLoad\n", modpblk.descr[i].name);
		}
	}
	/* remove all of our legacy module handlers, as they can not used in addition
	 * the the new-style config method.
	 */
	bLegacyCnfModGlobalsPermitted = 0;
finalize_it:
	if(pvals != NULL)
		cnfparamvalsDestruct(pvals, &modpblk);
ENDsetModCnf

BEGINendCnfLoad
CODESTARTendCnfLoad
	if(loadModConf->pszBindRuleset == NULL) {
		if((cs.pszBindRuleset == NULL) || (cs.pszBindRuleset[0] == '\0')) {
			loadModConf->pszBindRuleset = NULL;
		} else {
			CHKmalloc(loadModConf->pszBindRuleset = ustrdup(cs.pszBindRuleset));
		}
	} else {
		if((cs.pszBindRuleset != NULL) && (cs.pszBindRuleset[0] != '\0')) {
			LogError(0, RS_RET_DUP_PARAM, "imrelp: ruleset "
					"set via legacy directive ignored");
		}
	}
finalize_it:
	free(cs.pszBindRuleset);
	cs.pszBindRuleset = NULL;
	loadModConf = NULL; /* done loading */
ENDendCnfLoad

BEGINcheckCnf
	instanceConf_t *inst;
	size_t maxMessageSize;
CODESTARTcheckCnf
	for(inst = pModConf->root ; inst != NULL ; inst = inst->next) {
		if(inst->pszBindRuleset == NULL && pModConf->pszBindRuleset != NULL) {
			CHKmalloc(inst->pszBindRuleset = ustrdup(pModConf->pszBindRuleset));
		}
		std_checkRuleset(pModConf, inst);


		if(inst->maxDataSize == 0) {
			/* We set default value for maxDataSize here because
			 * otherwise the maxMessageSize isn't set.
			 */
			inst->maxDataSize = glbl.GetMaxLine();
		}
		maxMessageSize = (size_t)glbl.GetMaxLine();
		if(inst->maxDataSize < maxMessageSize) {
			LogError(0, RS_RET_INVALID_PARAMS, "error: "
					"maxDataSize (%zu) is smaller than global parameter "
					"maxMessageSize (%zu) - global parameter will be used.",
					inst->maxDataSize, maxMessageSize);
			inst->maxDataSize = maxMessageSize;
		}
	}

finalize_it:
ENDcheckCnf


BEGINactivateCnfPrePrivDrop
	instanceConf_t *inst;
CODESTARTactivateCnfPrePrivDrop
	runModConf = pModConf;
	for(inst = runModConf->root ; inst != NULL ; inst = inst->next) {
		addListner(pModConf, inst);
	}
	if(pRelpEngine == NULL) {
		LogError(0, RS_RET_NO_LSTN_DEFINED, "imrelp: no RELP listener defined, module can not run.");
		ABORT_FINALIZE(RS_RET_NO_RUN);
	}
finalize_it:
ENDactivateCnfPrePrivDrop

BEGINactivateCnf
CODESTARTactivateCnf
ENDactivateCnf


BEGINfreeCnf
	instanceConf_t *inst, *del;
	int i;
CODESTARTfreeCnf
	for(inst = pModConf->root ; inst != NULL ; ) {
		free(inst->pszBindPort);
		if (inst->pszBindAddr != NULL) {
			free(inst->pszBindAddr);
		}
		free(inst->pszBindRuleset);
		free(inst->pszInputName);
		free(inst->pristring);
		free(inst->authmode);
		for(i = 0 ; i <  inst->permittedPeers.nmemb ; ++i) {
			free(inst->permittedPeers.name[i]);
		}
		if(inst->bEnableLstn) {
			prop.Destruct(&inst->pInputName);
			statsobj.Destruct(&(inst->data.stats));
		}
		del = inst;
		inst = inst->next;
		free(del);
	}
	free(pModConf->pszBindRuleset);
ENDfreeCnf

/* This is used to terminate the plugin. Note that the signal handler blocks
 * other activity on the thread. As such, it is safe to request the stop. When
 * we terminate, relpEngine is called, and it's select() loop interrupted. But
 * only *after this function is done*. So we do not have a race!
 */
static void
doSIGTTIN(int __attribute__((unused)) sig)
{
	DBGPRINTF("imrelp: termination requested via SIGTTIN - telling RELP engine\n");
	relpEngineSetStop(pRelpEngine);
}


/* This function is called to gather input.
 */
BEGINrunInput
	sigset_t sigSet;
	struct sigaction sigAct;
CODESTARTrunInput
	/* we want to support non-cancel input termination. To do so, we must signal librelp
	 * when to stop. As we run on the same thread, we need to register as SIGTTIN handler,
	 * which will be used to put the terminating condition into librelp.
	 */
	sigfillset(&sigSet);
	pthread_sigmask(SIG_BLOCK, &sigSet, NULL);
	sigemptyset(&sigSet);
	sigaddset(&sigSet, SIGTTIN);
	pthread_sigmask(SIG_UNBLOCK, &sigSet, NULL);
	memset(&sigAct, 0, sizeof (sigAct));
	sigemptyset(&sigAct.sa_mask);
	sigAct.sa_handler = doSIGTTIN;
	sigaction(SIGTTIN, &sigAct, NULL);

	iRet = relpEngineRun(pRelpEngine);
ENDrunInput


BEGINwillRun
CODESTARTwillRun
ENDwillRun


BEGINafterRun
CODESTARTafterRun
	/* do cleanup here */
ENDafterRun


BEGINmodExit
CODESTARTmodExit
	if(pRelpEngine != NULL)
		iRet = relpEngineDestruct(&pRelpEngine);

	/* release objects we used */
	objRelease(statsobj, CORE_COMPONENT);
	objRelease(ruleset, CORE_COMPONENT);
	objRelease(glbl, CORE_COMPONENT);
	objRelease(prop, CORE_COMPONENT);
	objRelease(net, LM_NET_FILENAME);
ENDmodExit


static rsRetVal
resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal)
{
	free(cs.pszBindRuleset);
	cs.pszBindRuleset = NULL;
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
CODEqueryEtryPt_STD_CONF2_PREPRIVDROP_QUERIES
CODEqueryEtryPt_STD_CONF2_IMOD_QUERIES
CODEqueryEtryPt_STD_CONF2_setModCnf_QUERIES
CODEqueryEtryPt_IsCompatibleWithFeature_IF_OMOD_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	pRelpEngine = NULL;
	/* request objects we use */
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(prop, CORE_COMPONENT));
	CHKiRet(objUse(net, LM_NET_FILENAME));
	CHKiRet(objUse(ruleset, CORE_COMPONENT));
	CHKiRet(objUse(statsobj, CORE_COMPONENT));

	#ifndef HAVE_RELPSRVSETOVERSIZEMODE
		LogMsg(0, RS_RET_OK_WARN, LOG_WARNING, "imrelp: librelp too old, oversizemode "
			"defaults to \"abort\"");
	#endif

	/* register config file handlers */
	CHKiRet(regCfSysLineHdlr2((uchar*)"inputrelpserverbindruleset", 0, eCmdHdlrGetWord,
				   NULL, &cs.pszBindRuleset, STD_LOADABLE_MODULE_ID, &bLegacyCnfModGlobalsPermitted));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputrelpserverrun", 0, eCmdHdlrGetWord,
				   addInstance, NULL, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler,
		resetConfigVariables, NULL, STD_LOADABLE_MODULE_ID));
ENDmodInit


/* vim:set ai:
 */
