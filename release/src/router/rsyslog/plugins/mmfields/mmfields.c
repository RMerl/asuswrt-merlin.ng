/* mmfields.c
 * Parse all fields of the message into structured data inside the
 * JSON tree.
 *
 * Copyright 2013 Adiscon GmbH.
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
#include "conf.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "template.h"
#include "module-template.h"
#include "errmsg.h"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("mmfields")


DEF_OMOD_STATIC_DATA

/* config variables */

/* define operation modes we have */
#define SIMPLE_MODE 0	 /* just overwrite */
#define REWRITE_MODE 1	 /* rewrite IP address, canoninized */
typedef struct _instanceData {
	char separator;
	uchar *jsonRoot;	/**< container where to store fields */
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
	{ "separator", eCmdHdlrGetChar, 0 },
	{ "jsonroot", eCmdHdlrString, 0 }
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
ENDcreateInstance

BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
ENDcreateWrkrInstance


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
ENDisCompatibleWithFeature


BEGINfreeInstance
CODESTARTfreeInstance
	free(pData->jsonRoot);
ENDfreeInstance

BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance


static inline void
setInstParamDefaults(instanceData *pData)
{
	pData->separator = ',';
	pData->jsonRoot = NULL;
}

BEGINnewActInst
	struct cnfparamvals *pvals;
	int i;
CODESTARTnewActInst
	DBGPRINTF("newActInst (mmfields)\n");
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
		if(!strcmp(actpblk.descr[i].name, "separator")) {
			pData->separator = es_getBufAddr(pvals[i].val.d.estr)[0];
		} else if(!strcmp(actpblk.descr[i].name, "jsonroot")) {
			pData->jsonRoot = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else {
			dbgprintf("mmfields: program error, non-handled "
			  "param '%s'\n", actpblk.descr[i].name);
		}
	}
	if(pData->jsonRoot == NULL) {
		CHKmalloc(pData->jsonRoot = (uchar*) strdup("!"));
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


static rsRetVal
extractField(instanceData *pData, uchar *msgtext, int lenMsg, int *curridx, uchar *fieldbuf)
{
	int i, j;
	DEFiRet;
	i = *curridx;
	j = 0;
	while(i < lenMsg && msgtext[i] != pData->separator) {
		fieldbuf[j++] = msgtext[i++];
	}
	fieldbuf[j] = '\0';
	if(i < lenMsg)
		++i;
	*curridx = i;

	RETiRet;
}


static rsRetVal
parse_fields(instanceData *pData, smsg_t *pMsg, uchar *msgtext, int lenMsg)
{
	uchar fieldbuf[32*1024];
	uchar fieldname[512];
	struct json_object *json;
	struct json_object *jval;
	int field;
	uchar *buf;
	int currIdx = 0;
	DEFiRet;

	if(lenMsg < (int) sizeof(fieldbuf)) {
		buf = fieldbuf;
	} else {
		CHKmalloc(buf = malloc(lenMsg+1));
	}

	json =  json_object_new_object();
	if(json == NULL) {
		ABORT_FINALIZE(RS_RET_ERR);
	}
	field = 1;
	while(currIdx < lenMsg) {
		CHKiRet(extractField(pData, msgtext, lenMsg, &currIdx, buf));
		DBGPRINTF("mmfields: field %d: '%s'\n", field, buf);
		snprintf((char*)fieldname, sizeof(fieldname), "f%d", field);
		fieldname[sizeof(fieldname)-1] = '\0';
		jval = json_object_new_string((char*)buf);
		json_object_object_add(json, (char*)fieldname, jval);
		field++;
	}
	msgAddJSON(pMsg, pData->jsonRoot, json, 0, 0);
finalize_it:
	if(buf != fieldbuf)
		free(buf);
	RETiRet;
}


BEGINdoAction_NoStrings
	smsg_t **ppMsg = (smsg_t **) pMsgData;
	smsg_t *pMsg = ppMsg[0];
	uchar *msg;
	int lenMsg;
CODESTARTdoAction
	lenMsg = getMSGLen(pMsg);
	msg = getMSG(pMsg);
	iRet = parse_fields(pWrkrData->pData, pMsg, msg, lenMsg);
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
	DBGPRINTF("mmfields: module compiled with rsyslog version %s.\n", VERSION);
ENDmodInit
