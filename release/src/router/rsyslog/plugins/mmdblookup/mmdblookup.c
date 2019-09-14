/* mmdblookup.c
 * Parse ipaddress field of the message into structured data using
 * MaxMindDB.
 *
 * Copyright 2013 Rao Chenlin.
 * Copyright 2017 Rainer Gerhards and Adiscon GmbH.
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
#include <pthread.h>
#include "conf.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "template.h"
#include "module-template.h"
#include "errmsg.h"
#include "parserif.h"

#include "maxminddb.h"

#define JSON_IPLOOKUP_NAME "!iplocation"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("mmdblookup")


DEF_OMOD_STATIC_DATA

/* config variables */
typedef struct _instanceData {
	char *pszKey;
	char *pszMmdbFile;
	struct {
		int     nmemb;
		char **name;
		char **varname;
	} fieldList;
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
	MMDB_s        mmdb;
} wrkrInstanceData_t;

struct modConfData_s {
	/* our overall config object */
	rsconf_t *pConf;
	const char *container;
};

/* modConf ptr to use for the current load process */
static modConfData_t *loadModConf = NULL;
/* modConf ptr to use for the current exec process */
static modConfData_t *runModConf  = NULL;


/* module-global parameters */
static struct cnfparamdescr modpdescr[] = {
	{ "container", eCmdHdlrGetWord, 0 },
};
static struct cnfparamblk modpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(modpdescr)/sizeof(struct cnfparamdescr),
	  modpdescr
	};

/* tables for interfacing with the v6 config system
 * action (instance) parameters */
static struct cnfparamdescr actpdescr[] = {
	{ "key",      eCmdHdlrGetWord, CNFPARAM_REQUIRED },
	{ "mmdbfile", eCmdHdlrGetWord, CNFPARAM_REQUIRED },
	{ "fields",   eCmdHdlrArray,   CNFPARAM_REQUIRED },
};
static struct cnfparamblk actpblk = {
	CNFPARAMBLK_VERSION,
	sizeof(actpdescr)/sizeof(struct cnfparamdescr),
	actpdescr
};


/* protype functions */
void str_split(char **membuf);


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
	free((void*)runModConf->container);
ENDfreeCnf


BEGINcreateInstance
CODESTARTcreateInstance
ENDcreateInstance

BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
	int status = MMDB_open(pData->pszMmdbFile, MMDB_MODE_MMAP, &pWrkrData->mmdb);
	if (MMDB_SUCCESS != status) {
		dbgprintf("Can't open %s - %s\n", pData->pszMmdbFile, MMDB_strerror(status));
		if (MMDB_IO_ERROR == status) {
			dbgprintf("  IO error: %s\n", strerror(errno));
		}
		LogError(0, RS_RET_SUSPENDED, "can not initialize maxminddb");
		/* ABORT_FINALIZE(RS_RET_SUSPENDED); */
	}
ENDcreateWrkrInstance


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
ENDisCompatibleWithFeature


BEGINfreeInstance
CODESTARTfreeInstance
	if(pData->fieldList.name != NULL) {
		for(int i = 0 ; i < pData->fieldList.nmemb ; ++i) {
			free(pData->fieldList.name[i]);
			free(pData->fieldList.varname[i]);
		}
		free(pData->fieldList.name);
		free(pData->fieldList.varname);
	}
	free(pData->pszKey);
	free(pData->pszMmdbFile);
ENDfreeInstance


BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
	MMDB_close(&pWrkrData->mmdb);
ENDfreeWrkrInstance


BEGINsetModCnf
	struct cnfparamvals *pvals = NULL;
	int i;
CODESTARTsetModCnf
	loadModConf->container = NULL;
	pvals = nvlstGetParams(lst, &modpblk, NULL);
	if(pvals == NULL) {
		LogError(0, RS_RET_MISSING_CNFPARAMS, "mmdblookup: error processing module "
						"config parameters missing [module(...)]");
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	if(Debug) {
		dbgprintf("module (global) param blk for mmdblookup:\n");
		cnfparamsPrint(&modpblk, pvals);
	}

	for(i = 0 ; i < modpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(modpblk.descr[i].name, "container")) {
			loadModConf->container = es_str2cstr(pvals[i].val.d.estr, NULL);
		} else {
			dbgprintf("mmdblookup: program error, non-handled "
					  "param '%s' in setModCnf\n", modpblk.descr[i].name);
		}
	}

	if(loadModConf->container == NULL) {
		CHKmalloc(loadModConf->container = strdup(JSON_IPLOOKUP_NAME));
	}

finalize_it:
	if(pvals != NULL)
		cnfparamvalsDestruct(pvals, &modpblk);
ENDsetModCnf

static inline void
setInstParamDefaults(instanceData *pData)
{
	pData->pszKey = NULL;
	pData->pszMmdbFile = NULL;
	pData->fieldList.nmemb = 0;
}

BEGINnewActInst
	struct cnfparamvals *pvals;
	int i;
CODESTARTnewActInst
	dbgprintf("newActInst (mmdblookup)\n");
	if ((pvals = nvlstGetParams(lst, &actpblk, NULL)) == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	CODE_STD_STRING_REQUESTnewActInst(1)
	CHKiRet(OMSRsetEntry(*ppOMSR, 0, NULL, OMSR_TPL_AS_MSG));
	CHKiRet(createInstance(&pData));
	setInstParamDefaults(pData);

	for (i = 0; i < actpblk.nParams; ++i) {
		if (!pvals[i].bUsed)
			continue;
		if (!strcmp(actpblk.descr[i].name, "key")) {
			pData->pszKey = es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if (!strcmp(actpblk.descr[i].name, "mmdbfile")) {
			pData->pszMmdbFile = es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if (!strcmp(actpblk.descr[i].name, "fields")) {
			pData->fieldList.nmemb = pvals[i].val.d.ar->nmemb;
			CHKmalloc(pData->fieldList.name = calloc(pData->fieldList.nmemb, sizeof(char *)));
			CHKmalloc(pData->fieldList.varname = calloc(pData->fieldList.nmemb, sizeof(char *)));
			for (int j = 0; j <  pvals[i].val.d.ar->nmemb; ++j) {
				char *const param = es_str2cstr(pvals[i].val.d.ar->arr[j], NULL);
				char *varname = NULL;
				char *name;
				if(*param == ':') {
					char *b = strchr(param+1, ':');
					if(b == NULL) {
						parser_errmsg("mmdblookup: missing closing colon: '%s'", param);
						ABORT_FINALIZE(RS_RET_ERR);
					}
					*b = '\0'; /* split name & varname */
					varname = param+1;
					name = b+1;
				} else {
					name = param;
				}
				if(*name == '!')
					++name;
				CHKmalloc(pData->fieldList.name[j] = strdup(name));
				char vnamebuf[1024];
				snprintf(vnamebuf, sizeof(vnamebuf),
					"%s!%s", loadModConf->container,
					(varname == NULL) ? name : varname);
				CHKmalloc(pData->fieldList.varname[j] = strdup(vnamebuf));
				free(param);
			}
		} else {
			dbgprintf("mmdblookup: program error, non-handled"
				" param '%s'\n", actpblk.descr[i].name);
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


void
str_split(char **membuf)
{
	char *buf  = *membuf;
	char tempbuf[strlen(buf)];
	memset(tempbuf, 0, strlen(buf));

	while (*buf++ != '\0') {
		if (*buf == '\n' || *buf == '\t' || *buf == ' ')
			continue;
		else {
			if (*buf == '<') {
				char *p = strchr(buf, '>');
				buf = buf + (int)(p - buf);
				strcat(tempbuf, ",");
			} else if (*buf == '}')
				strcat(tempbuf, "},");
			else
				strncat(tempbuf, buf, 1);
		}
	}

	tempbuf[strlen(tempbuf) + 1] = '\n';
	memcpy(*membuf, tempbuf, strlen(tempbuf));
}


BEGINdoAction_NoStrings
	smsg_t **ppMsg = (smsg_t **) pMsgData;
	smsg_t *pMsg   = ppMsg[0];
	struct json_object *keyjson = NULL;
	const char *pszValue;
	instanceData *const pData = pWrkrData->pData;
	json_object *total_json = NULL;
	MMDB_entry_data_list_s *entry_data_list = NULL;
CODESTARTdoAction
	/* key is given, so get the property json */
	msgPropDescr_t pProp;
	msgPropDescrFill(&pProp, (uchar*)pData->pszKey, strlen(pData->pszKey));
	rsRetVal localRet = msgGetJSONPropJSON(pMsg, &pProp, &keyjson);
	msgPropDescrDestruct(&pProp);

	if (localRet != RS_RET_OK) {
		/* key not found in the message. nothing to do */
		ABORT_FINALIZE(RS_RET_OK);
	}
	/* key found, so get the value */
	pszValue = (char*)json_object_get_string(keyjson);
	if(pszValue == NULL) { /* json null object returns NULL! */
		pszValue = "";
	}

	int gai_err, mmdb_err;
	MMDB_lookup_result_s result = MMDB_lookup_string(&pWrkrData->mmdb, pszValue, &gai_err, &mmdb_err);

	if (0 != gai_err) {
		dbgprintf("Error from call to getaddrinfo for %s - %s\n", pszValue, gai_strerror(gai_err));
		ABORT_FINALIZE(RS_RET_OK);
	}
	if (MMDB_SUCCESS != mmdb_err) {
		dbgprintf("Got an error from the maxminddb library: %s\n", MMDB_strerror(mmdb_err));
		ABORT_FINALIZE(RS_RET_OK);
	}

	int status  = MMDB_get_entry_data_list(&result.entry, &entry_data_list);

	if (MMDB_SUCCESS != status) {
		dbgprintf("Got an error looking up the entry data - %s\n", MMDB_strerror(status));
		ABORT_FINALIZE(RS_RET_OK);
	}

	size_t  memlen;
	char   *membuf;
	FILE   *memstream;
	CHKmalloc(memstream = open_memstream(&membuf, &memlen));

	if (entry_data_list != NULL && memstream != NULL) {
		MMDB_dump_entry_data_list(memstream, entry_data_list, 2);
		fflush(memstream);
		str_split(&membuf);
	}

	DBGPRINTF("maxmindb returns: '%s'\n", membuf);
	total_json = json_tokener_parse(membuf);
	fclose(memstream);
	free(membuf);

	/* extract and amend fields (to message) as configured */
	for (int i = 0 ; i <  pData->fieldList.nmemb; ++i) {
		char *strtok_save;
		char buf[(strlen((char *)(pData->fieldList.name[i])))+1];
		strcpy(buf, (char *)pData->fieldList.name[i]);

		json_object *temp_json = total_json;
		json_object *sub_obj   = temp_json;
		int j = 0;
		const char *SEP = "!";

		/* find lowest level JSON object */
		char *s = strtok_r(buf, SEP, &strtok_save);
		for (; s != NULL; j++) {
			json_object_object_get_ex(temp_json, s, &sub_obj);
			temp_json = sub_obj;
			s = strtok_r(NULL, SEP, &strtok_save);
		}
		/* temp_json now contains the value we want to have, so set it */
		json_object_get(temp_json);
		msgAddJSON(pMsg, (uchar *)pData->fieldList.varname[i], temp_json, 0, 0);
	}

finalize_it:
	if(entry_data_list != NULL)
		MMDB_free_entry_data_list(entry_data_list);
	json_object_put(keyjson);
	if(total_json != NULL)
		json_object_put(total_json);
ENDdoAction


NO_LEGACY_CONF_parseSelectorAct


BEGINmodExit
CODESTARTmodExit
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_OMOD_QUERIES
CODEqueryEtryPt_STD_OMOD8_QUERIES
CODEqueryEtryPt_STD_CONF2_setModCnf_QUERIES
CODEqueryEtryPt_STD_CONF2_OMOD_QUERIES
CODEqueryEtryPt_STD_CONF2_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	/* we only support the current interface specification */
	*ipIFVersProvided = CURR_MOD_IF_VERSION;
CODEmodInit_QueryRegCFSLineHdlr
	dbgprintf("mmdblookup: module compiled with rsyslog version %s.\n", VERSION);
ENDmodInit
