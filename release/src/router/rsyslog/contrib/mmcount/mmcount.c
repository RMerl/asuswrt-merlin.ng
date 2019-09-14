/* mmcount.c
 * count messages by priority or json property of given app-name.
 *
 * Copyright 2013 Red Hat Inc.
 * Copyright 2014 Rainer Gerhards
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
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <json.h>
#include "conf.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "template.h"
#include "module-template.h"
#include "errmsg.h"
#include "hashtable.h"


#define JSON_COUNT_NAME "!mmcount"
#define SEVERITY_COUNT 8

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("mmcount")


DEF_OMOD_STATIC_DATA

/* config variables */

typedef struct _instanceData {
	char *pszAppName;
	int severity[SEVERITY_COUNT];
	char *pszKey;
	char *pszValue;
	int valueCounter;
	struct hashtable *ht;
	pthread_mutex_t mut;
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
} wrkrInstanceData_t;

struct modConfData_s {
	rsconf_t *pConf;	/* our overall config object */
};
static modConfData_t *loadModConf = NULL;/* modConf ptr to use for the current load process */
static modConfData_t *runModConf = NULL;/* modConf ptr to use for the current exec process */


/* tables for interfacing with the v6 config system */
/* action (instance) parameters */
static struct cnfparamdescr actpdescr[] = {
	{ "appname", eCmdHdlrGetWord, 0 },
	{ "key", eCmdHdlrGetWord, 0 },
	{ "value", eCmdHdlrGetWord, 0 },
};
static struct cnfparamblk actpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(actpdescr)/sizeof(struct cnfparamdescr),
	  actpdescr
	};

BEGINbeginCnfLoad
CODESTARTbeginCnfLoad
	loadModConf = pModConf;
	pModConf->pConf = pConf;
ENDbeginCnfLoad

BEGINendCnfLoad
CODESTARTendCnfLoad
ENDendCnfLoad

BEGINcheckCnf
CODESTARTcheckCnf
ENDcheckCnf

BEGINactivateCnf
CODESTARTactivateCnf
	runModConf = pModConf;
ENDactivateCnf

BEGINfreeCnf
CODESTARTfreeCnf
ENDfreeCnf


BEGINcreateInstance
CODESTARTcreateInstance
	pthread_mutex_init(&pData->mut, NULL);
ENDcreateInstance

BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
ENDcreateWrkrInstance


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
ENDisCompatibleWithFeature


BEGINfreeInstance
CODESTARTfreeInstance
ENDfreeInstance


BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance

static inline void
setInstParamDefaults(instanceData *pData)
{
	int i;

	pData->pszAppName = NULL;
	for (i = 0; i < SEVERITY_COUNT; i++)
	        pData->severity[i] = 0;
	pData->pszKey = NULL;
	pData->pszValue = NULL;
	pData->valueCounter = 0;
	pData->ht = NULL;
}

static unsigned int
hash_from_key_fn(void *k)
{
	return *(unsigned int *)k;
}

static int
key_equals_fn(void *k1, void *k2)
{
	return (*(unsigned int *)k1 == *(unsigned int *)k2);
}

BEGINnewActInst
	struct cnfparamvals *pvals;
	int i;
CODESTARTnewActInst
	DBGPRINTF("newActInst (mmcount)\n");
	if((pvals = nvlstGetParams(lst, &actpblk, NULL)) == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	CODE_STD_STRING_REQUESTnewActInst(1)
	CHKiRet(OMSRsetEntry(*ppOMSR, 0, NULL, OMSR_TPL_AS_MSG));
	CHKiRet(createInstance(&pData));
	setInstParamDefaults(pData);

	for(i = 0 ; i < actpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(actpblk.descr[i].name, "appname")) {
			pData->pszAppName = es_str2cstr(pvals[i].val.d.estr, NULL);
			continue;
		}
		if(!strcmp(actpblk.descr[i].name, "key")) {
			pData->pszKey = es_str2cstr(pvals[i].val.d.estr, NULL);
			continue;
		}
		if(!strcmp(actpblk.descr[i].name, "value")) {
			pData->pszValue = es_str2cstr(pvals[i].val.d.estr, NULL);
			continue;
		}
		dbgprintf("mmcount: program error, non-handled "
			  "param '%s'\n", actpblk.descr[i].name);
	}

	if(pData->pszAppName == NULL) {
		dbgprintf("mmcount: action requires a appname");
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	if(pData->pszKey != NULL && pData->pszValue == NULL) {
		if(NULL == (pData->ht = create_hashtable(100, hash_from_key_fn, key_equals_fn, NULL))) {
			DBGPRINTF("mmcount: error creating hash table!\n");
			ABORT_FINALIZE(RS_RET_ERR);
		}
	}
CODE_STD_FINALIZERnewActInst
	cnfparamvalsDestruct(pvals, &actpblk);
ENDnewActInst


BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
ENDdbgPrintInstInfo


BEGINtryResume
CODESTARTtryResume
ENDtryResume

static int *
getCounter(struct hashtable *ht, const char *str) {
	unsigned int key;
	int *pCounter;
	unsigned int *pKey;

	/* we dont store str as key, instead we store hash of the str
	   as key to reduce memory usage */
	key = hash_from_string((char*)str);
	pCounter = hashtable_search(ht, &key);
	if(pCounter) {
		return pCounter;
	}

	/* counter is not found for the str, so add new entry and
	   return the counter */
	if(NULL == (pKey = (unsigned int*)malloc(sizeof(unsigned int)))) {
		DBGPRINTF("mmcount: memory allocation for key failed\n");
		return NULL;
	}
	*pKey = key;

	if(NULL == (pCounter = (int*)malloc(sizeof(int)))) {
		DBGPRINTF("mmcount: memory allocation for value failed\n");
		free(pKey);
		return NULL;
	}
	*pCounter = 0;

	if(!hashtable_insert(ht, pKey, pCounter)) {
		DBGPRINTF("mmcount: inserting element into hashtable failed\n");
		free(pKey);
		free(pCounter);
		return NULL;
	}
	return pCounter;
}

BEGINdoAction_NoStrings
	smsg_t **ppMsg = (smsg_t **) pMsgData;
	smsg_t *pMsg = ppMsg[0];
	char *appname;
	struct json_object *json = NULL;
	struct json_object *keyjson = NULL;
	const char *pszValue;
	int *pCounter;
	instanceData *const pData = pWrkrData->pData;
CODESTARTdoAction
	appname = getAPPNAME(pMsg, LOCK_MUTEX);

	pthread_mutex_lock(&pData->mut);
	if(0 != strcmp(appname, pData->pszAppName)) {
		/* we are not working for this appname. nothing to do */
		ABORT_FINALIZE(RS_RET_OK);
	}

	if(!pData->pszKey) {
		/* no key given for count, so we count severity */
		if(pMsg->iSeverity < SEVERITY_COUNT) {
			pData->severity[pMsg->iSeverity]++;
			json = json_object_new_int(pData->severity[pMsg->iSeverity]);
		}
		ABORT_FINALIZE(RS_RET_OK);
	}

	/* key is given, so get the property json */
	msgPropDescr_t pProp;
	msgPropDescrFill(&pProp, (uchar*)pData->pszKey, strlen(pData->pszKey));
	rsRetVal localRet = msgGetJSONPropJSON(pMsg, &pProp, &keyjson);
	msgPropDescrDestruct(&pProp);
	if(localRet != RS_RET_OK) {
		/* key not found in the message. nothing to do */
		ABORT_FINALIZE(RS_RET_OK);
	}

	/* key found, so get the value */
	pszValue = (char*)json_object_get_string(keyjson);
	if(pszValue == NULL) { /* json null object returns NULL! */
		pszValue = "";
	}

	if(pData->pszValue) {
		/* value also given for count */
		if(!strcmp(pszValue, pData->pszValue)) {
			/* count for (value and key and appname) matched */
			pData->valueCounter++;
			json = json_object_new_int(pData->valueCounter);
		}
		ABORT_FINALIZE(RS_RET_OK);
	}

	/* value is not given, so we count for each value of given key */
	pCounter = getCounter(pData->ht, pszValue);
	if(pCounter) {
		(*pCounter)++;
		json = json_object_new_int(*pCounter);
	}
finalize_it:
	pthread_mutex_unlock(&pData->mut);

	if(json) {
		msgAddJSON(pMsg, (uchar *)JSON_COUNT_NAME, json, 0, 0);
	}
ENDdoAction


NO_LEGACY_CONF_parseSelectorAct


BEGINmodExit
CODESTARTmodExit
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_OMOD_QUERIES
CODEqueryEtryPt_STD_OMOD8_QUERIES
CODEqueryEtryPt_STD_CONF2_OMOD_QUERIES
CODEqueryEtryPt_STD_CONF2_QUERIES
ENDqueryEtryPt



BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	DBGPRINTF("mmcount: module compiled with rsyslog version %s.\n", VERSION);
ENDmodInit
