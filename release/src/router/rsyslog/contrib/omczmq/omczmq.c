/* omczmq.c
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
MODULE_CNFNAME("omczmq")

DEF_OMOD_STATIC_DATA

static pthread_mutex_t mutDoAct = PTHREAD_MUTEX_INITIALIZER;

static struct cnfparamdescr modpdescr[] = {
	{ "authenticator", eCmdHdlrBinary, 0 },
	{ "authtype", eCmdHdlrGetWord, 0 },
	{ "clientcertpath", eCmdHdlrGetWord, 0 },
	{ "servercertpath", eCmdHdlrGetWord, 0 }
};

static struct cnfparamblk modpblk = {
	CNFPARAMBLK_VERSION,
	sizeof(modpdescr)/sizeof(struct cnfparamdescr),
	modpdescr
};

struct modConfData_s {
	rsconf_t *pConf;
	uchar *tplName;
	int authenticator;
	char *authType;
	char *serverCertPath;
	char *clientCertPath;
};

static modConfData_t *runModConf = NULL;
static zactor_t *authActor;

typedef struct _instanceData {
	zsock_t *sock;
	bool serverish;
	int sendTimeout;
	zlist_t *topics;
	bool sendError;
	char *sockEndpoints;
	int sockType;
	int sendHWM;
#if(CZMQ_VERSION_MAJOR >= 4 && ZMQ_VERSION_MAJOR >=4 && ZMQ_VERSION_MINOR >=2)
	int heartbeatIvl;
	int heartbeatTimeout;
	int heartbeatTTL;
	int connectTimeout;
#endif
	uchar *tplName;
	sbool topicFrame;
	sbool dynaTopic;
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
} wrkrInstanceData_t;

static struct cnfparamdescr actpdescr[] = {
	{ "endpoints", eCmdHdlrGetWord, 1 },
	{ "socktype", eCmdHdlrGetWord, 1 },
	{ "sendhwm", eCmdHdlrGetWord, 0 },
#if(CZMQ_VERSION_MAJOR >= 4 && ZMQ_VERSION_MAJOR >=4 && ZMQ_VERSION_MINOR >=2)
	{ "heartbeatttl", eCmdHdlrGetWord, 0},
	{ "heartbeativl", eCmdHdlrGetWord, 0},
	{ "heartbeattimeout", eCmdHdlrGetWord, 0},
	{ "connecttimeout", eCmdHdlrGetWord, 0},
#endif
	{ "sendtimeout", eCmdHdlrGetWord, 0 },
	{ "template", eCmdHdlrGetWord, 0 },
	{ "topics", eCmdHdlrGetWord, 0 },
	{ "topicframe", eCmdHdlrGetWord, 0},
	{ "dynatopic", eCmdHdlrBinary, 0 }
};

static struct cnfparamblk actpblk = {
	CNFPARAMBLK_VERSION,
	sizeof(actpdescr) / sizeof(struct cnfparamdescr),
	actpdescr
};

static rsRetVal initCZMQ(instanceData* pData) {
	DEFiRet;
	putenv((char*)"ZSYS_SIGHANDLER=false");
	pData->sock = zsock_new(pData->sockType);
	if(!pData->sock) {
		LogError(0, RS_RET_NO_ERRCODE,
				"omczmq: new socket failed for endpoints: %s",
				pData->sockEndpoints);
		ABORT_FINALIZE(RS_RET_SUSPENDED);
	}

	zsock_set_sndtimeo(pData->sock, pData->sendTimeout);

#if(CZMQ_VERSION_MAJOR >= 4 && ZMQ_VERSION_MAJOR >=4 && ZMQ_VERSION_MINOR >=2)
	if(pData->heartbeatIvl > 0 && pData->heartbeatTimeout > 0 && pData->heartbeatTTL > 0) {
		zsock_set_heartbeat_ivl(pData->sock, pData->heartbeatIvl);
		zsock_set_heartbeat_timeout(pData->sock, pData->heartbeatTimeout);
		zsock_set_heartbeat_ttl(pData->sock, pData->heartbeatTTL);
	}
#endif

	if(runModConf->authType) {
		if (!strcmp(runModConf->authType, "CURVESERVER")) {
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

	switch(pData->sockType) {
		case ZMQ_PUB:
#if defined(ZMQ_RADIO)
		case ZMQ_RADIO:
#endif
			pData->serverish = true;
			break;
		case ZMQ_PUSH:
#if defined(ZMQ_SCATTER)
		case ZMQ_SCATTER:
#endif
		case ZMQ_DEALER:
#if defined(ZMQ_CLIENT)
		case ZMQ_CLIENT:
#endif
			pData->serverish = false;
			break;
	}

	int rc = zsock_attach(pData->sock, pData->sockEndpoints, pData->serverish);
	if(rc == -1) {
		LogError(0, NO_ERRCODE, "zsock_attach to %s failed",
				pData->sockEndpoints);
		ABORT_FINALIZE(RS_RET_SUSPENDED);
	}

finalize_it:
	RETiRet;
}

static rsRetVal outputCZMQ(uchar** ppString, instanceData* pData) {
	DEFiRet;

	if(NULL == pData->sock) {
		CHKiRet(initCZMQ(pData));
	}

	/* if we are using a PUB (or RADIO) socket and we have a topic list then we
	 * need some special care and attention */
#if defined(ZMQ_RADIO)
	DBGPRINTF("omczmq: ZMQ_RADIO is defined...\n");
	if((pData->sockType == ZMQ_PUB || pData->sockType == ZMQ_RADIO) && pData->topics) {
#else
	DBGPRINTF("omczmq: ZMQ_RADIO is NOT defined...\n");
	if(pData->sockType == ZMQ_PUB && pData->topics) {
#endif
		int templateIndex = 1;
		const char *topic = (const char *)zlist_first(pData->topics);
		while(topic) {
			int rc;
			/* if dynaTopic is true, the topic is constructed by rsyslog
			 * by applying the supplied template to the message properties */
			if(pData->dynaTopic)
				topic = (const char*)ppString[templateIndex];

			if (pData->sockType == ZMQ_PUB) {
				/* if topicFrame is true, send the topic as a separate zmq frame */
				if(pData->topicFrame) {
					rc = zstr_sendx(pData->sock, topic, (char*)ppString[0], NULL);
				}

				/* if topicFrame is false, concatenate the topic with the
				 * message in the same frame */
				else {
					rc = zstr_sendf(pData->sock, "%s%s", topic, (char*)ppString[0]);
				}

				/* if we have a send error notify rsyslog */
				if(rc != 0) {
					pData->sendError = true;
					ABORT_FINALIZE(RS_RET_SUSPENDED);
				}
			}
#if defined(ZMQ_RADIO)
			else if(pData->sockType == ZMQ_RADIO) {
				DBGPRINTF("omczmq: sending on RADIO socket...\n");
				zframe_t *frame = zframe_from((char*)ppString[0]);
				if (!frame) {
					DBGPRINTF("omczmq: failed to create frame...\n");
					pData->sendError = true;
					ABORT_FINALIZE(RS_RET_SUSPENDED);
				}
				rc = zframe_set_group(frame, topic);
				if (rc != 0) {
					DBGPRINTF("omczmq: failed to set group '%d'...\n", rc);
					pData->sendError = true;
					ABORT_FINALIZE(RS_RET_SUSPENDED);
				}
				DBGPRINTF("omczmq: set RADIO group to '%s'\n", topic);
				rc = zframe_send(&frame, pData->sock, 0);
				if(rc != 0) {
					pData->sendError = true;
					ABORT_FINALIZE(RS_RET_SUSPENDED);
				}
			}
#endif

			/* get the next topic from the list, and increment
			 * our topic index */
			topic = zlist_next(pData->topics);
			templateIndex++;
		}
	}

	/* we aren't a PUB socket and we don't have a topic list - this means
	 * we can just send the message using the rsyslog template */
	else {
		int rc = zstr_send(pData->sock, (char*)ppString[0]);
		if(rc != 0) {
			pData->sendError = true;
			DBGPRINTF("omczmq: send error: %d", rc);
			ABORT_FINALIZE(RS_RET_SUSPENDED);
		}
	}
finalize_it:
	RETiRet;
}

static inline void
setInstParamDefaults(instanceData* pData) {
	pData->sockEndpoints = NULL;
	pData->sock = NULL;
	pData->sendError = false;
	pData->serverish = false;
	pData->tplName = NULL;
	pData->sockType = -1;
	pData->sendTimeout = -1;
	pData->topics = NULL;
	pData->topicFrame = false;
#if(CZMQ_VERSION_MAJOR >= 4 && ZMQ_VERSION_MAJOR >=4 && ZMQ_VERSION_MINOR >=2)
	pData->heartbeatIvl = 0;
	pData->heartbeatTimeout = 0;
	pData->heartbeatTTL = 0;
#endif
}


BEGINcreateInstance
CODESTARTcreateInstance
ENDcreateInstance

BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
ENDcreateWrkrInstance

BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	if(eFeat == sFEATURERepeatedMsgReduction) {
		iRet = RS_RET_OK;
	}
ENDisCompatibleWithFeature
BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
ENDdbgPrintInstInfo

BEGINfreeInstance
CODESTARTfreeInstance
	zlist_destroy(&pData->topics);
	zsock_destroy(&pData->sock);
	free(pData->sockEndpoints);
	free(pData->tplName);
ENDfreeInstance


BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance

BEGINtryResume
	instanceData *pData;
CODESTARTtryResume
	pthread_mutex_lock(&mutDoAct);
	pData = pWrkrData->pData;
	DBGPRINTF("omczmq: trying to resume...\n");
	zsock_destroy(&pData->sock);
	iRet = initCZMQ(pData);
	pthread_mutex_unlock(&mutDoAct);
ENDtryResume

BEGINbeginCnfLoad
CODESTARTbeginCnfLoad
	runModConf = pModConf;
	runModConf->pConf = pConf;
	runModConf->authenticator = 0;
	runModConf->authType = NULL;
	runModConf->serverCertPath = NULL;
	runModConf->clientCertPath = NULL;
ENDbeginCnfLoad

BEGINcheckCnf
CODESTARTcheckCnf
ENDcheckCnf

BEGINactivateCnf
CODESTARTactivateCnf
	runModConf = pModConf;
	if(runModConf->authenticator == 1) {
		if(!authActor) {
			DBGPRINTF("omczmq: starting authActor\n");
			authActor = zactor_new(zauth, NULL);
			if(!strcmp(runModConf->clientCertPath, "*")) {
				zstr_sendx(authActor, "CURVE", CURVE_ALLOW_ANY, NULL);
			}
			else {
				zstr_sendx(authActor, "CURVE", runModConf->clientCertPath, NULL);
			}
			zsock_wait(authActor);
		}
	}
ENDactivateCnf

BEGINfreeCnf
CODESTARTfreeCnf
	free(pModConf->tplName);
	free(pModConf->authType);
	free(pModConf->serverCertPath);
	free(pModConf->clientCertPath);
	DBGPRINTF("omczmq: stopping authActor\n");
	zactor_destroy(&authActor);
ENDfreeCnf

BEGINsetModCnf
	struct cnfparamvals *pvals = NULL;
	int i;
CODESTARTsetModCnf
	pvals = nvlstGetParams(lst, &modpblk, NULL);
	if (pvals == NULL) {
		LogError(0, RS_RET_MISSING_CNFPARAMS, "error processing module");
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	for (i=0; i<modpblk.nParams; ++i) {
		if(!pvals[i].bUsed) {
			DBGPRINTF("omczmq: pvals[i].bUSed continuing\n");
			continue;
		}
		if(!strcmp(modpblk.descr[i].name, "authenticator")) {
			runModConf->authenticator = (int)pvals[i].val.d.n;
		}
		else if(!strcmp(modpblk.descr[i].name, "authtype")) {
			runModConf->authType = es_str2cstr(pvals[i].val.d.estr, NULL);
			DBGPRINTF("omczmq: authtype set to %s\n", runModConf->authType);
		}
		else if(!strcmp(modpblk.descr[i].name, "servercertpath")) {
			runModConf->serverCertPath = es_str2cstr(pvals[i].val.d.estr, NULL);
			DBGPRINTF("omczmq: serverCertPath set to %s\n", runModConf->serverCertPath);
		}
		else if(!strcmp(modpblk.descr[i].name, "clientcertpath")) {
			runModConf->clientCertPath = es_str2cstr(pvals[i].val.d.estr, NULL);
			DBGPRINTF("omczmq: clientCertPath set to %s\n", runModConf->clientCertPath);
		}
		else {
			LogError(0, RS_RET_INVALID_PARAMS,
						"omczmq: config error, unknown "
						"param %s in setModCnf\n",
						modpblk.descr[i].name);
		}
	}

	DBGPRINTF("omczmq: authenticator set to %d\n", runModConf->authenticator);
	DBGPRINTF("omczmq: authType set to %s\n", runModConf->authType);
	DBGPRINTF("omczmq: serverCertPath set to %s\n", runModConf->serverCertPath);
	DBGPRINTF("omczmq: clientCertPath set to %s\n", runModConf->clientCertPath);

finalize_it:
		if(pvals != NULL)
			cnfparamvalsDestruct(pvals, &modpblk);
ENDsetModCnf

BEGINendCnfLoad
CODESTARTendCnfLoad
	runModConf = NULL;
ENDendCnfLoad


BEGINdoAction
	instanceData *pData;
CODESTARTdoAction
	pthread_mutex_lock(&mutDoAct);
	pData = pWrkrData->pData;
	iRet = outputCZMQ(ppString, pData);
	pthread_mutex_unlock(&mutDoAct);
ENDdoAction


BEGINnewActInst
	struct cnfparamvals *pvals;
	int i;
	int iNumTpls;
CODESTARTnewActInst
	if ((pvals = nvlstGetParams(lst, &actpblk, NULL)) == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	CHKiRet(createInstance(&pData));
	setInstParamDefaults(pData);

	for(i = 0; i < actpblk.nParams; ++i) {
		if(!pvals[i].bUsed) {
			continue;
		}

		if(!strcmp(actpblk.descr[i].name, "endpoints")) {
			pData->sockEndpoints = es_str2cstr(pvals[i].val.d.estr, NULL);
			DBGPRINTF("omczmq: sockEndPoints set to '%s'\n", pData->sockEndpoints);
		}
		else if(!strcmp(actpblk.descr[i].name, "template")) {
			pData->tplName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
			DBGPRINTF("omczmq: template set to '%s'\n", pData->tplName);
		}
		else if(!strcmp(actpblk.descr[i].name, "dynatopic")) {
			pData->dynaTopic = pvals[i].val.d.n;
			DBGPRINTF("omczmq: dynaTopic set to %s\n", pData->dynaTopic ? "true" : "false");
		}
		else if(!strcmp(actpblk.descr[i].name, "sendtimeout")) {
			pData->sendTimeout = atoi(es_str2cstr(pvals[i].val.d.estr, NULL));
			DBGPRINTF("omczmq: sendTimeout set to %d\n", pData->sendTimeout);
		}
		else if(!strcmp(actpblk.descr[i].name, "sendhwm")) {
			pData->sendTimeout = atoi(es_str2cstr(pvals[i].val.d.estr, NULL));
			DBGPRINTF("omczmq: sendHWM set to %d\n", pData->sendHWM);
		}
#if (CZMQ_VERSION_MAJOR >= 4 && ZMQ_VERSION_MAJOR >=4 && ZMQ_VERSION_MINOR >=2)
		else if(!strcmp(actpblk.descr[i].name, "heartbeativl")) {
			pData->heartbeatIvl = atoi(es_str2cstr(pvals[i].val.d.estr, NULL));
			DBGPRINTF("omczmq: heartbeatbeatIvl set to %d\n", pData->heartbeatIvl);
		}
		else if(!strcmp(actpblk.descr[i].name, "heartbeattimeout")) {
			pData->heartbeatTimeout = atoi(es_str2cstr(pvals[i].val.d.estr, NULL));
			DBGPRINTF("omczmq: heartbeatTimeout set to %d\n", pData->heartbeatTimeout);
		}
		else if(!strcmp(actpblk.descr[i].name, "heartbeatttl")) {
			pData->heartbeatTimeout = atoi(es_str2cstr(pvals[i].val.d.estr, NULL));
			DBGPRINTF("omczmq: heartbeatTTL set to %d\n", pData->heartbeatTTL);
		}
#endif
		else if(!strcmp(actpblk.descr[i].name, "socktype")){
			char *stringType = es_str2cstr(pvals[i].val.d.estr, NULL);
			if(stringType != NULL){
				if(!strcmp("PUB", stringType)) {
					pData->sockType = ZMQ_PUB;
					DBGPRINTF("omczmq: sockType set to ZMQ_PUB\n");
				}
#if defined(ZMQ_RADIO)
				else if(!strcmp("RADIO", stringType)) {
					pData->sockType = ZMQ_RADIO;
					DBGPRINTF("omczmq: sockType set to ZMQ_RADIO\n");
				}
#endif
				else if(!strcmp("PUSH", stringType)) {
					pData->sockType = ZMQ_PUSH;
					DBGPRINTF("omczmq: sockType set to ZMQ_PUSH\n");
				}
#if defined(ZMQ_SCATTER)
				else if(!strcmp("SCATTER", stringType)) {
					pData->sockType = ZMQ_SCATTER;
					DBGPRINTF("omczmq: sockType set to ZMQ_SCATTER\n");
				}
#endif
				else if(!strcmp("DEALER", stringType)) {
					pData->sockType = ZMQ_DEALER;
					DBGPRINTF("omczmq: sockType set to ZMQ_DEALER\n");
				}
#if defined(ZMQ_CLIENT)
				else if(!strcmp("CLIENT", stringType)) {
					pData->sockType = ZMQ_CLIENT;
					DBGPRINTF("omczmq: sockType set to ZMQ_CLIENT\n");
				}
#endif
				free(stringType);
			}
			else{
				LogError(0, RS_RET_OUT_OF_MEMORY,
						"omczmq: out of memory");
				ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
			}
		}
		else if(!strcmp(actpblk.descr[i].name, "topicframe")) {
			pData->topicFrame = pvals[i].val.d.n;
			DBGPRINTF("omczmq: topicFrame set to %s\n", pData->topicFrame ? "true" : "false");
		}
		else if(!strcmp(actpblk.descr[i].name, "topics")) {
			pData->topics = zlist_new();
			char *topics = es_str2cstr(pvals[i].val.d.estr, NULL);
			DBGPRINTF("omczmq: topics set to %s\n", topics);
			char *topics_org = topics;
			char topic[256];
			if(topics == NULL){
				LogError(0, RS_RET_OUT_OF_MEMORY,
					"out of memory");
				ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
			}

			while(*topics) {
				char *delimiter = strchr(topics, ',');
				if (!delimiter) {
					delimiter = topics + strlen(topics);
				}
				memcpy (topic, topics, delimiter - topics);
				topic[delimiter-topics] = 0;
				char *current_topic = strdup(topic);
				zlist_append (pData->topics, current_topic);
				if(*delimiter == 0) {
					break;
				}
				topics = delimiter + 1;
			}
			free(topics_org);

		}
		else {
			LogError(0, NO_ERRCODE,
					"omczmq: config error - '%s' is not a valid option",
					actpblk.descr[i].name);
			ABORT_FINALIZE(RS_RET_CONFIG_ERROR);
		}
	}

	iNumTpls = 1;
	if (pData->dynaTopic) {
		iNumTpls = zlist_size (pData->topics) + iNumTpls;
	}
	CODE_STD_STRING_REQUESTnewActInst(iNumTpls)
	
	if (pData->tplName == NULL) {
		CHKiRet(OMSRsetEntry(*ppOMSR, 0, (uchar*)strdup("RSYSLOG_ForwardFormat"),
					OMSR_NO_RQD_TPL_OPTS));
	}
	else {
		CHKiRet(OMSRsetEntry(*ppOMSR, 0, (uchar*)pData->tplName, OMSR_NO_RQD_TPL_OPTS));
	}

	i = 1;
	if (pData->dynaTopic) {
		char *topic = zlist_first(pData->topics);
		while (topic) {
			CHKiRet(OMSRsetEntry(*ppOMSR, i, (uchar*)strdup(topic), OMSR_NO_RQD_TPL_OPTS));
			i++;
			topic = zlist_next(pData->topics);
		}
	}

	CODE_STD_FINALIZERnewActInst
	cnfparamvalsDestruct(pvals, &actpblk);
ENDnewActInst

BEGINinitConfVars
CODESTARTinitConfVars
ENDinitConfVars

NO_LEGACY_CONF_parseSelectorAct

BEGINmodExit
CODESTARTmodExit
ENDmodExit

BEGINqueryEtryPt
CODESTARTqueryEtryPt
	CODEqueryEtryPt_STD_OMOD_QUERIES
	CODEqueryEtryPt_STD_CONF2_OMOD_QUERIES
	CODEqueryEtryPt_STD_CONF2_QUERIES
	CODEqueryEtryPt_STD_CONF2_setModCnf_QUERIES
	CODEqueryEtryPt_STD_OMOD8_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION;
CODEmodInit_QueryRegCFSLineHdlr
	INITChkCoreFeature(bCoreSupportsBatching, CORE_FEATURE_BATCHING);
	DBGPRINTF("omczmq: module compiled with rsyslog version %s.\n", VERSION);

	INITLegCnfVars
ENDmodInit
