/* imczmq.c
 * Copyright (C) 2016 Brian Knox
 * Copyright (C) 2014 Rainer Gerhards
 *
 * Author: Brian Knox <bknox@digitalocean.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "config.h"
#include "rsyslog.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cfsysline.h"
#include "dirty.h"
#include "errmsg.h"
#include "glbl.h"
#include "module-template.h"
#include "msg.h"
#include "net.h"
#include "parser.h"
#include "prop.h"
#include "ruleset.h"
#include "srUtils.h"
#include "unicode-helper.h"
#include <czmq.h>

MODULE_TYPE_INPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("imczmq");

DEF_IMOD_STATIC_DATA
DEFobjCurrIf(glbl)
DEFobjCurrIf(prop)
DEFobjCurrIf(ruleset)

static struct cnfparamdescr modpdescr[] = {
	{ "authenticator", eCmdHdlrBinary, 0 },
	{ "authtype", eCmdHdlrString, 0 },
	{ "servercertpath", eCmdHdlrString, 0 },
	{ "clientcertpath", eCmdHdlrString, 0 },
};

static struct cnfparamblk modpblk = {
	CNFPARAMBLK_VERSION,
	sizeof(modpdescr)/sizeof(struct cnfparamdescr),
	modpdescr
};

struct modConfData_s {
	rsconf_t *pConf;
	instanceConf_t *root;
	instanceConf_t *tail;
	int authenticator;
	char *authType;
	char *serverCertPath;
	char *clientCertPath;
};

struct instanceConf_s {
	bool serverish;
	int sockType;
	char *sockEndpoints;
	char *topics;
	uchar *pszBindRuleset;
	ruleset_t *pBindRuleset;
	struct instanceConf_s *next;
};

struct listener_t {
	zsock_t *sock;
	ruleset_t *ruleset;
};

static zlist_t *listenerList;
static modConfData_t *runModConf = NULL;
static prop_t *s_namep = NULL;

static struct cnfparamdescr inppdescr[] = {
	{ "endpoints", eCmdHdlrGetWord, 1 },
	{ "socktype", eCmdHdlrGetWord, 1 },
	{ "ruleset", eCmdHdlrGetWord, 0 },
	{ "topics", eCmdHdlrGetWord, 0 },
};

#include "im-helper.h"

static struct cnfparamblk inppblk = {
	CNFPARAMBLK_VERSION,
	sizeof(inppdescr) / sizeof(struct cnfparamdescr),
	inppdescr
};

static void setDefaults(instanceConf_t* iconf) {
	iconf->serverish = true;
	iconf->sockType = -1;
	iconf->sockEndpoints = NULL;
	iconf->topics = NULL;
	iconf->pszBindRuleset = NULL;
	iconf->pBindRuleset = NULL;
	iconf->next = NULL;
};

static rsRetVal createInstance(instanceConf_t** pinst) {
	DEFiRet;
	instanceConf_t* inst;
	CHKmalloc(inst = MALLOC(sizeof(instanceConf_t)));
	
	setDefaults(inst);
	
	if(runModConf->root == NULL || runModConf->tail == NULL) {
		runModConf->tail = runModConf->root = inst;
	}
	else {
		runModConf->tail->next = inst;
		runModConf->tail = inst;
	}
	*pinst = inst;
finalize_it:
	RETiRet;
}

static rsRetVal addListener(instanceConf_t* iconf){
	DEFiRet;
	
	DBGPRINTF("imczmq: addListener called..\n");
	struct listener_t* pData = NULL;
	CHKmalloc(pData=(struct listener_t*)MALLOC(sizeof(struct listener_t)));
	pData->ruleset = iconf->pBindRuleset;

	pData->sock = zsock_new(iconf->sockType);
	if(!pData->sock) {
		LogError(0, RS_RET_NO_ERRCODE,
				"imczmq: new socket failed for endpoints: %s",
				iconf->sockEndpoints);
		ABORT_FINALIZE(RS_RET_NO_ERRCODE);
	}

	DBGPRINTF("imczmq: created socket of type %d..\n", iconf->sockType);

	if(runModConf->authType) {
		if(!strcmp(runModConf->authType, "CURVESERVER")) {
			DBGPRINTF("imczmq: we are a CURVESERVER\n");
			zcert_t *serverCert = zcert_load(runModConf->serverCertPath);
			if(!serverCert) {
				LogError(0, NO_ERRCODE, "could not load cert %s",
					runModConf->serverCertPath);
				ABORT_FINALIZE(RS_RET_ERR);
			}
			zsock_set_zap_domain(pData->sock, "global");
			zsock_set_curve_server(pData->sock, 1);
			zcert_apply(serverCert, pData->sock);
			zcert_destroy(&serverCert);
		}
		else if(!strcmp(runModConf->authType, "CURVECLIENT")) {
			DBGPRINTF("imczmq: we are a CURVECLIENT\n");
			zcert_t *serverCert = zcert_load(runModConf->serverCertPath);
			if(!serverCert) {
				LogError(0, NO_ERRCODE, "could not load cert %s",
					runModConf->serverCertPath);
				ABORT_FINALIZE(RS_RET_ERR);
			}
			const char *server_key = zcert_public_txt(serverCert);
			zcert_destroy(&serverCert);
			zsock_set_curve_serverkey(pData->sock, server_key);

			zcert_t *clientCert = zcert_load(runModConf->clientCertPath);
			if(!clientCert) {
				LogError(0, NO_ERRCODE, "could not load cert %s",
					runModConf->clientCertPath);
				ABORT_FINALIZE(RS_RET_ERR);
			}

			zcert_apply(clientCert, pData->sock);
			zcert_destroy(&clientCert);
		}

	}

	switch(iconf->sockType) {
		case ZMQ_SUB:
#if defined(ZMQ_DISH)
		case ZMQ_DISH:
#endif
			iconf->serverish = false;
			break;
		case ZMQ_PULL:
#if defined(ZMQ_GATHER)
		case ZMQ_GATHER:
#endif
		case ZMQ_ROUTER:
#if defined(ZMQ_SERVER)
		case ZMQ_SERVER:
#endif
			iconf->serverish = true;
			break;
	}

	if(iconf->topics) {
		// A zero-length topic means subscribe to everything
		if(!*iconf->topics && iconf->sockType == ZMQ_SUB) {
			DBGPRINTF("imczmq: subscribing to all topics\n");
			zsock_set_subscribe(pData->sock, "");
		}

		char topic[256];
		while(*iconf->topics) {
			char *delimiter = strchr(iconf->topics, ',');
			if(!delimiter) {
				delimiter = iconf->topics + strlen(iconf->topics);
			}
			memcpy (topic, iconf->topics, delimiter - iconf->topics);
			topic[delimiter-iconf->topics] = 0;
			DBGPRINTF("imczmq: subscribing to %s\n", topic);
			if(iconf->sockType == ZMQ_SUB) {
				zsock_set_subscribe (pData->sock, topic);
			}
#if defined(ZMQ_DISH)
			else if(iconf->sockType == ZMQ_DISH) {
				int rc = zsock_join (pData->sock, topic);
				if(rc != 0) {
					LogError(0, NO_ERRCODE, "could not join group %s", topic);
					ABORT_FINALIZE(RS_RET_ERR);
				}
			}
#endif
			if(*delimiter == 0) {
				break;
			}
			iconf->topics = delimiter + 1;
		}
	}

	int rc = zsock_attach(pData->sock, (const char*)iconf->sockEndpoints,
			iconf->serverish);
	if (rc == -1) {
		LogError(0, NO_ERRCODE, "zsock_attach to %s failed",
				iconf->sockEndpoints);
		ABORT_FINALIZE(RS_RET_ERR);
	}

	DBGPRINTF("imczmq: attached socket to %s\n", iconf->sockEndpoints);

	rc = zlist_append(listenerList, (void *)pData);
	if(rc != 0) {
		LogError(0, NO_ERRCODE, "could not append listener");
		ABORT_FINALIZE(RS_RET_ERR);
	}
finalize_it:
	if(iRet != RS_RET_OK) {
		free(pData);
	}
	RETiRet;
}

static rsRetVal rcvData(void){
	DEFiRet;
	
	if(!listenerList) {
		listenerList = zlist_new();
		if(!listenerList) {
			LogError(0, NO_ERRCODE, "could not allocate list");
			ABORT_FINALIZE(RS_RET_ERR);
		}
	}

	zactor_t *authActor = NULL;

	if(runModConf->authenticator == 1) {
		authActor = zactor_new(zauth, NULL);
		zstr_sendx(authActor, "CURVE", runModConf->clientCertPath, NULL);
		zsock_wait(authActor);
	}

	instanceConf_t *inst;
	for(inst = runModConf->root; inst != NULL; inst=inst->next) {
		CHKiRet(addListener(inst));
	}
	
	zpoller_t *poller = zpoller_new(NULL);
	if(!poller) {
		LogError(0, NO_ERRCODE, "could not create poller");
			ABORT_FINALIZE(RS_RET_ERR);
	}
	DBGPRINTF("imczmq: created poller\n");

	struct listener_t *pData;

	pData = zlist_first(listenerList);
	if(!pData) {
		LogError(0, NO_ERRCODE, "imczmq: no listeners were "
						"started, input not activated.\n");
		ABORT_FINALIZE(RS_RET_NO_RUN);
	}

	while(pData) {
		int rc = zpoller_add(poller, pData->sock);
		if(rc != 0) {
			LogError(0, NO_ERRCODE, "imczmq: could not add "
						"socket to poller, input not activated.\n");
			ABORT_FINALIZE(RS_RET_NO_RUN);
		}
		pData = zlist_next(listenerList);
	}

	zframe_t *frame;
	zsock_t *which = (zsock_t *)zpoller_wait(poller, -1);
	while(which) {
		if (zpoller_terminated(poller)) {
				break;
		}
		pData = zlist_first(listenerList);
		while(pData->sock != which) {
			pData = zlist_next(listenerList);
		}
	
		if(which == pData->sock) {
			DBGPRINTF("imczmq: found matching socket\n");
		}

		frame = zframe_recv(which);
		char *buf = zframe_strdup(frame);

		if(buf == NULL) {
			DBGPRINTF("imczmq: null buffer\n");
			continue;
		}
		smsg_t *pMsg;
		if(msgConstruct(&pMsg) == RS_RET_OK) {
			MsgSetRawMsg(pMsg, buf, strlen(buf));
			MsgSetInputName(pMsg, s_namep);
			MsgSetHOSTNAME(pMsg, glbl.GetLocalHostName(), ustrlen(glbl.GetLocalHostName()));
			MsgSetRcvFrom(pMsg, glbl.GetLocalHostNameProp());
			MsgSetRcvFromIP(pMsg, glbl.GetLocalHostIP());
			MsgSetMSGoffs(pMsg, 0);
			MsgSetFlowControlType(pMsg, eFLOWCTL_NO_DELAY);
			MsgSetRuleset(pMsg, pData->ruleset);
			pMsg->msgFlags = NEEDS_PARSING | PARSE_HOSTNAME;
			submitMsg2(pMsg);
		}

		free(buf);
		which = (zsock_t *)zpoller_wait(poller, -1);
	}
finalize_it:
	zframe_destroy(&frame);
	zpoller_destroy(&poller);
	pData = zlist_first(listenerList);
	while(pData) {
		zsock_destroy(&pData->sock);
		free(pData->ruleset);
		pData = zlist_next(listenerList);
	}
	zlist_destroy(&listenerList);
	zactor_destroy(&authActor);
	RETiRet;
}

BEGINrunInput
CODESTARTrunInput
	iRet = rcvData();
ENDrunInput


BEGINwillRun
CODESTARTwillRun
	CHKiRet(prop.Construct(&s_namep));
	CHKiRet(prop.SetString(s_namep,
		UCHAR_CONSTANT("imczmq"),
		sizeof("imczmq") - 1));

	CHKiRet(prop.ConstructFinalize(s_namep));

finalize_it:
ENDwillRun


BEGINafterRun
CODESTARTafterRun
	if(s_namep != NULL) {
		prop.Destruct(&s_namep);
	}
ENDafterRun


BEGINmodExit
CODESTARTmodExit
	objRelease(glbl, CORE_COMPONENT);
	objRelease(prop, CORE_COMPONENT);
	objRelease(ruleset, CORE_COMPONENT);
ENDmodExit


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	if(eFeat == sFEATURENonCancelInputTermination) {
		iRet = RS_RET_OK;
	}
ENDisCompatibleWithFeature


BEGINbeginCnfLoad
CODESTARTbeginCnfLoad
	runModConf = pModConf;
	runModConf->pConf = pConf;
	runModConf->authenticator = 0;
	runModConf->authType = NULL;
	runModConf->serverCertPath = NULL;
	runModConf->clientCertPath = NULL;
ENDbeginCnfLoad


BEGINsetModCnf
	struct cnfparamvals* pvals = NULL;
	int i;
CODESTARTsetModCnf
	pvals = nvlstGetParams(lst, &modpblk, NULL);
	if(NULL == pvals) {
		LogError(0, RS_RET_MISSING_CNFPARAMS,
				"imczmq: error processing module "
				"config parameters ['module(...)']");

		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	for(i=0; i < modpblk.nParams; ++i) {
		if(!pvals[i].bUsed) {
			continue;
		}
		if(!strcmp(modpblk.descr[i].name, "authenticator")) {
			runModConf->authenticator = (int)pvals[i].val.d.n;
		}
		else if(!strcmp(modpblk.descr[i].name, "authtype")) {
			runModConf->authType = es_str2cstr(pvals[i].val.d.estr, NULL);
		}
		else if(!strcmp(modpblk.descr[i].name, "servercertpath")) {
			runModConf->serverCertPath = es_str2cstr(pvals[i].val.d.estr, NULL);
		}
		else if(!strcmp(modpblk.descr[i].name, "clientcertpath")) {
			runModConf->clientCertPath = es_str2cstr(pvals[i].val.d.estr, NULL);
		}
		else {
			LogError(0, RS_RET_INVALID_PARAMS,
						"imczmq: config error, unknown "
						"param %s in setModCnf\n",
						modpblk.descr[i].name);
		}
	}

	DBGPRINTF("imczmq: authenticator set to %d\n", runModConf->authenticator);
	DBGPRINTF("imczmq: authType set to %s\n", runModConf->authType);
	DBGPRINTF("imczmq: serverCertPath set to %s\n", runModConf->serverCertPath);
	DBGPRINTF("imczmq: clientCertPath set to %s\n", runModConf->clientCertPath);

finalize_it:
	if(pvals != NULL) {
		cnfparamvalsDestruct(pvals, &modpblk);
	}
ENDsetModCnf


BEGINendCnfLoad
CODESTARTendCnfLoad
ENDendCnfLoad


static inline void
std_checkRuleset_genErrMsg(__attribute__((unused)) modConfData_t *modConf, instanceConf_t *inst)
{
	LogError(0, NO_ERRCODE,
			"imczmq: ruleset '%s' for socket %s not found - "
			"using default ruleset instead", inst->pszBindRuleset,
			inst->sockEndpoints);
}


BEGINcheckCnf
instanceConf_t* inst;
CODESTARTcheckCnf
	for(inst = pModConf->root; inst!=NULL; inst=inst->next) {
		std_checkRuleset(pModConf, inst);
	}
ENDcheckCnf


BEGINactivateCnfPrePrivDrop
CODESTARTactivateCnfPrePrivDrop
	runModConf = pModConf;
	putenv((char*)"ZSYS_SIGHANDLER=false");
ENDactivateCnfPrePrivDrop


BEGINactivateCnf
CODESTARTactivateCnf
ENDactivateCnf


BEGINfreeCnf
	instanceConf_t *inst, *inst_r;
CODESTARTfreeCnf
	free(pModConf->authType);
	free(pModConf->serverCertPath);
	free(pModConf->clientCertPath);
	for (inst = pModConf->root ; inst != NULL ; ) {
		free(inst->pszBindRuleset);
		free(inst->sockEndpoints);
		inst_r = inst;
		inst = inst->next;
		free(inst_r);
	}

ENDfreeCnf


BEGINnewInpInst
	struct cnfparamvals* pvals;
	instanceConf_t* inst;
	int i;
CODESTARTnewInpInst
	DBGPRINTF("newInpInst (imczmq)\n");
	
	pvals = nvlstGetParams(lst, &inppblk, NULL);
	if(NULL==pvals) {
		LogError(0, RS_RET_MISSING_CNFPARAMS,
						"imczmq: required parameters are missing\n");
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	if(Debug) {
		DBGPRINTF("imczmq: input param blk:\n");
		cnfparamsPrint(&inppblk, pvals);
	}
	
	CHKiRet(createInstance(&inst));
	
	for(i = 0 ; i < inppblk.nParams ; ++i) {
		if(!pvals[i].bUsed) {
			continue;
		}

		if(!strcmp(inppblk.descr[i].name, "ruleset")) {
			inst->pszBindRuleset = (uchar *)es_str2cstr(pvals[i].val.d.estr, NULL);
		}
		else if(!strcmp(inppblk.descr[i].name, "endpoints")) {
			inst->sockEndpoints = es_str2cstr(pvals[i].val.d.estr, NULL);
		}
		else if(!strcmp(inppblk.descr[i].name, "topics")) {
			inst->topics = es_str2cstr(pvals[i].val.d.estr, NULL);
		}
		else if(!strcmp(inppblk.descr[i].name, "socktype")){
			char *stringType = es_str2cstr(pvals[i].val.d.estr, NULL);
			if( NULL == stringType ){
				LogError(0, RS_RET_CONFIG_ERROR,
					"imczmq: '%s' is invalid sockType", stringType);
				ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
			}

			if(!strcmp("PULL", stringType)) {
				inst->sockType = ZMQ_PULL;
			}
#if defined(ZMQ_GATHER)
			else if(!strcmp("GATHER", stringType)) {
				inst->sockType = ZMQ_GATHER;
			}
#endif
			else if(!strcmp("SUB", stringType)) {
				inst->sockType = ZMQ_SUB;
			}
#if defined(ZMQ_DISH)
			else if(!strcmp("DISH", stringType)) {
				inst->sockType = ZMQ_DISH;
			}
#endif
			else if(!strcmp("ROUTER", stringType)) {
				inst->sockType = ZMQ_ROUTER;
			}
#if defined(ZMQ_SERVER)
			else if(!strcmp("SERVER", stringType)) {
				inst->sockType = ZMQ_SERVER;
			}
#endif
			free(stringType);

		}
		else {
			LogError(0, NO_ERRCODE,
					"imczmq: program error, non-handled "
					"param '%s'\n", inppblk.descr[i].name);
		}
	}
finalize_it:
CODE_STD_FINALIZERnewInpInst
	cnfparamvalsDestruct(pvals, &inppblk);
ENDnewInpInst


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
	*ipIFVersProvided = CURR_MOD_IF_VERSION;
CODEmodInit_QueryRegCFSLineHdlr
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(prop, CORE_COMPONENT));
	CHKiRet(objUse(ruleset, CORE_COMPONENT));
ENDmodInit

