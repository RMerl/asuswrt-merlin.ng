/* mmsequence.c
 * Generate a number based on some sequence.
 *
 * Copyright 2013 pavel@levshin.spb.ru.
 *
 * Based on: mmcount.c
 * Copyright 2013 Red Hat Inc.
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
#include <time.h>
#include <limits.h>
#include <json.h>
#include <pthread.h>
#include "conf.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "template.h"
#include "module-template.h"
#include "errmsg.h"
#include "hashtable.h"

#define JSON_VAR_NAME "$!mmsequence"

enum mmSequenceModes {
	mmSequenceRandom,
	mmSequencePerInstance,
	mmSequencePerKey
};

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("mmsequence")


DEF_OMOD_STATIC_DATA

/* config variables */

typedef struct _instanceData {
	enum mmSequenceModes mode;
	int valueFrom;
	int valueTo;
	int step;
	unsigned int seed;
	int value;
	char *pszKey;
	char *pszVar;
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
	{ "mode", eCmdHdlrGetWord, 0 },
	{ "from", eCmdHdlrNonNegInt, 0 },
	{ "to", eCmdHdlrPositiveInt, 0 },
	{ "step", eCmdHdlrNonNegInt, 0 },
	{ "key", eCmdHdlrGetWord, 0 },
	{ "var", eCmdHdlrGetWord, 0 },
};
static struct cnfparamblk actpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(actpdescr)/sizeof(struct cnfparamdescr),
	  actpdescr
	};

/* table for key-counter pairs */
static struct hashtable *ght;
static pthread_mutex_t ght_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t inst_mutex = PTHREAD_MUTEX_INITIALIZER;
	
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
	pData->mode = mmSequencePerInstance;
	pData->valueFrom = 0;
	pData->valueTo = INT_MAX;
	pData->step = 1;
	pData->pszKey = (char*)"";
	pData->pszVar = (char*)JSON_VAR_NAME;
}

BEGINnewActInst
	struct cnfparamvals *pvals;
	int i;
	char *cstr;
CODESTARTnewActInst
	DBGPRINTF("newActInst (mmsequence)\n");
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
		if(!strcmp(actpblk.descr[i].name, "mode")) {
			if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"random",
					 sizeof("random")-1)) {
				pData->mode = mmSequenceRandom;
			} else if (!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"instance",
					 sizeof("instance")-1)) {
				pData->mode = mmSequencePerInstance;
			} else if (!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"key",
					 sizeof("key")-1)) {
				pData->mode = mmSequencePerKey;
			} else {
				cstr = es_str2cstr(pvals[i].val.d.estr, NULL);
				LogError(0, RS_RET_INVLD_MODE,
					"mmsequence: invalid mode '%s' - ignored",
					cstr);
				free(cstr);
			}
			continue;
		}
		if(!strcmp(actpblk.descr[i].name, "from")) {
			pData->valueFrom = pvals[i].val.d.n;
			continue;
		}
		if(!strcmp(actpblk.descr[i].name, "to")) {
			pData->valueTo = pvals[i].val.d.n;
			continue;
		}
		if(!strcmp(actpblk.descr[i].name, "step")) {
			pData->step = pvals[i].val.d.n;
			continue;
		}
		if(!strcmp(actpblk.descr[i].name, "key")) {
			pData->pszKey = es_str2cstr(pvals[i].val.d.estr, NULL);
			continue;
		}
		if(!strcmp(actpblk.descr[i].name, "var")) {
			cstr = es_str2cstr(pvals[i].val.d.estr, NULL);
			if (strlen(cstr) < 3) {
				LogError(0, RS_RET_VALUE_NOT_SUPPORTED,
						"mmsequence: valid variable name should be at least "
						"3 symbols long, got %s",	cstr);
				free(cstr);
			} else if (cstr[0] != '$') {
				LogError(0, RS_RET_VALUE_NOT_SUPPORTED,
						"mmsequence: valid variable name should start with $,"
						"got %s", cstr);
				free(cstr);
			} else {
				pData->pszVar = cstr;
			}
			continue;
		}
		dbgprintf("mmsequence: program error, non-handled "
			  "param '%s'\n", actpblk.descr[i].name);
	}
	switch(pData->mode) {
	case mmSequenceRandom:
		pData->seed = (unsigned int)(intptr_t)pData ^ (unsigned int)time(NULL);
		break;
	case mmSequencePerInstance:
		pData->value = pData->valueTo;
		break;
	case mmSequencePerKey:
		if (pthread_mutex_lock(&ght_mutex)) {
			DBGPRINTF("mmsequence: mutex lock has failed!\n");
			ABORT_FINALIZE(RS_RET_ERR);
		}
		if (ght == NULL) {
			if(NULL == (ght = create_hashtable(100, hash_from_string, key_equals_string, NULL))) {
				pthread_mutex_unlock(&ght_mutex);
				DBGPRINTF("mmsequence: error creating hash table!\n");
				ABORT_FINALIZE(RS_RET_ERR);
			}
		}
		pthread_mutex_unlock(&ght_mutex);
		break;
	default:
		LogError(0, RS_RET_INVLD_MODE,
				"mmsequence: this mode is not currently implemented");
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
getCounter(struct hashtable *ht, char *str, int initial) {
	int *pCounter;
	char *pStr;

	pCounter = hashtable_search(ht, str);
	if(pCounter) {
		return pCounter;
	}

	/* counter is not found for the str, so add new entry and
	   return the counter */
	if(NULL == (pStr = strdup(str))) {
		DBGPRINTF("mmsequence: memory allocation for key failed\n");
		return NULL;
	}

	if(NULL == (pCounter = (int*)malloc(sizeof(*pCounter)))) {
		DBGPRINTF("mmsequence: memory allocation for value failed\n");
		free(pStr);
		return NULL;
	}
	*pCounter = initial;

	if(!hashtable_insert(ht, pStr, pCounter)) {
		DBGPRINTF("mmsequence: inserting element into hashtable failed\n");
		free(pStr);
		free(pCounter);
		return NULL;
	}
	return pCounter;
}


BEGINdoAction_NoStrings
	smsg_t **ppMsg = (smsg_t **) pMsgData;
	smsg_t *pMsg = ppMsg[0];
	struct json_object *json;
	int val = 0;
	int *pCounter;
	instanceData *pData;
CODESTARTdoAction
	pData = pWrkrData->pData;

	switch(pData->mode) {
	case mmSequenceRandom:
		val = pData->valueFrom + (rand_r(&pData->seed) %
				(pData->valueTo - pData->valueFrom));
		break;
	case mmSequencePerInstance:
		if (!pthread_mutex_lock(&inst_mutex)) {
			if (pData->value >= pData->valueTo - pData->step) {
				pData->value = pData->valueFrom;
			} else {
				pData->value += pData->step;
			}
			val = pData->value;
			pthread_mutex_unlock(&inst_mutex);
		} else {
			LogError(0, RS_RET_ERR,
					"mmsequence: mutex lock has failed!");
		}
		break;
	case mmSequencePerKey:
		if (!pthread_mutex_lock(&ght_mutex)) {
			pCounter = getCounter(ght, pData->pszKey, pData->valueTo);
			if(pCounter) {
				if (*pCounter >= pData->valueTo - pData->step
						|| *pCounter < pData->valueFrom ) {
					*pCounter = pData->valueFrom;
				} else {
					*pCounter += pData->step;
				}
				val = *pCounter;
			} else {
				LogError(0, RS_RET_NOT_FOUND,
						"mmsequence: unable to fetch the counter from hash");
			}
			pthread_mutex_unlock(&ght_mutex);
		} else {
			LogError(0, RS_RET_ERR,
					"mmsequence: mutex lock has failed!");
		}

		break;
	default:
		LogError(0, RS_RET_NOT_IMPLEMENTED,
				"mmsequence: this mode is not currently implemented");
	}

	/* finalize_it: */
	json = json_object_new_int(val);
	if (json == NULL) {
		LogError(0, RS_RET_OBJ_CREATION_FAILED,
				"mmsequence: unable to create JSON");
	} else if (RS_RET_OK != msgAddJSON(pMsg, (uchar *)pData->pszVar + 1, json, 0, 0)) {
		LogError(0, RS_RET_OBJ_CREATION_FAILED,
				"mmsequence: unable to pass out the value");
		json_object_put(json);
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
	DBGPRINTF("mmsequence: module compiled with rsyslog version %s.\n", VERSION);
ENDmodInit
