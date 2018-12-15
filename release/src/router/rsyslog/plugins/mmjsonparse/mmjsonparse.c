/* mmjsonparse.c
 * This is a message modification module. If give, it extracts JSON data
 * and populates the EE event structure with it.
 *
 * NOTE: read comments in module-template.h for details on the calling interface!
 *
 * File begun on 2012-02-20 by RGerhards
 *
 * Copyright 2012-2018 Adiscon GmbH.
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
#include <ctype.h>
#include <json.h>
#include "conf.h"
#include "syslogd-types.h"
#include "template.h"
#include "module-template.h"
#include "errmsg.h"
#include "cfsysline.h"
#include "parserif.h"
#include "dirty.h"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("mmjsonparse")

static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal);

/* static data */

/* internal structures
 */
DEF_OMOD_STATIC_DATA

typedef struct _instanceData {
	sbool bUseRawMsg;     /**< use %rawmsg% instead of %msg% */
	char *cookie;
	uchar *container;
	int lenCookie;
	/* REMOVE dummy when real data items are to be added! */
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
	struct json_tokener *tokener;
} wrkrInstanceData_t;

struct modConfData_s {
	rsconf_t *pConf;	/* our overall config object */
};
static modConfData_t *loadModConf = NULL;/* modConf ptr to use for the current load process */
static modConfData_t *runModConf = NULL;/* modConf ptr to use for the current exec process */

/* tables for interfacing with the v6 config system */
/* action (instance) parameters */
static struct cnfparamdescr actpdescr[] = {
	{ "cookie", eCmdHdlrString, 0 },
	{ "container", eCmdHdlrString, 0 },
	{ "userawmsg", eCmdHdlrBinary, 0 }
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
	CHKmalloc(pData->container = (uchar*)strdup("!"));
	CHKmalloc(pData->cookie = strdup(CONST_CEE_COOKIE));
	pData->lenCookie = CONST_LEN_CEE_COOKIE;
finalize_it:
ENDcreateInstance

BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
	pWrkrData->tokener = json_tokener_new();
	if(pWrkrData->tokener == NULL) {
		LogError(0, RS_RET_ERR, "error: could not create json "
				"tokener, cannot activate instance");
		ABORT_FINALIZE(RS_RET_ERR);
	}
finalize_it:
ENDcreateWrkrInstance


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
ENDisCompatibleWithFeature


BEGINfreeInstance
CODESTARTfreeInstance
	free(pData->cookie);
	free(pData->container);
ENDfreeInstance

BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
	if(pWrkrData->tokener != NULL)
		json_tokener_free(pWrkrData->tokener);
ENDfreeWrkrInstance


BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
	DBGPRINTF("mmjsonparse\n");
ENDdbgPrintInstInfo


BEGINtryResume
CODESTARTtryResume
ENDtryResume


static rsRetVal
processJSON(wrkrInstanceData_t *pWrkrData, smsg_t *pMsg, char *buf, size_t lenBuf)
{
	struct json_object *json;
	const char *errMsg;
	DEFiRet;

	assert(pWrkrData->tokener != NULL);
	DBGPRINTF("mmjsonparse: toParse: '%s'\n", buf);
	json_tokener_reset(pWrkrData->tokener);

	json = json_tokener_parse_ex(pWrkrData->tokener, buf, lenBuf);
	if(Debug) {
		errMsg = NULL;
		if(json == NULL) {
			enum json_tokener_error err;

			err = pWrkrData->tokener->err;
			if(err != json_tokener_continue)
				errMsg = json_tokener_error_desc(err);
			else
				errMsg = "Unterminated input";
		} else if((size_t)pWrkrData->tokener->char_offset < lenBuf)
			errMsg = "Extra characters after JSON object";
		else if(!json_object_is_type(json, json_type_object))
			errMsg = "JSON value is not an object";
		if(errMsg != NULL) {
			DBGPRINTF("mmjsonparse: Error parsing JSON '%s': %s\n",
					buf, errMsg);
		}
	}
	if(json == NULL
	   || ((size_t)pWrkrData->tokener->char_offset < lenBuf)
	   || (!json_object_is_type(json, json_type_object))) {
		if(json != NULL) {
			/* Release json object as we are not going to add it to pMsg */
			json_object_put(json);
		}
		ABORT_FINALIZE(RS_RET_NO_CEE_MSG);
	}

	msgAddJSON(pMsg, pWrkrData->pData->container, json, 0, 0);
finalize_it:
	RETiRet;
}

BEGINdoAction_NoStrings
	smsg_t **ppMsg = (smsg_t **) pMsgData;
	smsg_t *pMsg = ppMsg[0];
	uchar *buf;
	rs_size_t len;
	int bSuccess = 0;
	struct json_object *jval;
	struct json_object *json;
	instanceData *pData;
CODESTARTdoAction
	pData = pWrkrData->pData;
	/* note that we can performance-optimize the interface, but this also
	 * requires changes to the libraries. For now, we accept message
	 * duplication. -- rgerhards, 2010-12-01
	 */
	if(pWrkrData->pData->bUseRawMsg)
		getRawMsg(pMsg, &buf, &len);
	else
		buf = getMSG(pMsg);

	while(*buf && isspace(*buf)) {
		++buf;
	}

	if(*buf == '\0' || strncmp((char*)buf, pData->cookie, pData->lenCookie)) {
		DBGPRINTF("mmjsonparse: no JSON cookie: '%s'\n", buf);
		ABORT_FINALIZE(RS_RET_NO_CEE_MSG);
	}
	buf += pData->lenCookie;
	CHKiRet(processJSON(pWrkrData, pMsg, (char*) buf, strlen((char*)buf)));
	bSuccess = 1;
finalize_it:
	if(iRet == RS_RET_NO_CEE_MSG) {
		/* add buf as msg */
		json = json_object_new_object();
		jval = json_object_new_string((char*)buf);
		json_object_object_add(json, "msg", jval);
		msgAddJSON(pMsg, pData->container, json, 0, 0);
		iRet = RS_RET_OK;
	}
	MsgSetParseSuccess(pMsg, bSuccess);
ENDdoAction

static inline void
setInstParamDefaults(instanceData *pData)
{
	pData->bUseRawMsg = 0;
}

BEGINnewActInst
	struct cnfparamvals *pvals;
	int i;
CODESTARTnewActInst
	DBGPRINTF("newActInst (mmjsonparse)\n");
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
		if(!strcmp(actpblk.descr[i].name, "cookie")) {
			free(pData->cookie);
			pData->cookie = es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "container")) {
			free(pData->container);
			size_t lenvar = es_strlen(pvals[i].val.d.estr);
			pData->container = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
			if(pData->container[0] == '$') {
				/* pre 8.35, the container name needed to be specified without
				 * the leading $. This was confusing, so we now require a full
				 * variable name. Nevertheless, we still need to support the
				 * version without $. -- rgerhards, 2018-05-16
				 */
				/* copy lenvar size because of \0 string terminator */
				memmove(pData->container, pData->container+1,  lenvar);
				--lenvar;
			}
			if(   (lenvar == 0)
			   || (  !(   pData->container[0] == '!'
			           || pData->container[0] == '.'
			           || pData->container[0] == '/' ) )
			   ) {
			parser_errmsg("mmjsonparse: invalid container name '%s', name must "
				"start with either '$!', '$.', or '$/'", pData->container);
			ABORT_FINALIZE(RS_RET_INVALID_VAR);
		}
		} else if(!strcmp(actpblk.descr[i].name, "userawmsg")) {
			pData->bUseRawMsg = (int) pvals[i].val.d.n;
		} else {
			dbgprintf("mmjsonparse: program error, non-handled param '%s'\n", actpblk.descr[i].name);
		}
	}

	if(pData->container == NULL)
		CHKmalloc(pData->container = (uchar*) strdup("!"));
	pData->lenCookie = strlen(pData->cookie);
CODE_STD_FINALIZERnewActInst
	cnfparamvalsDestruct(pvals, &actpblk);
ENDnewActInst

BEGINparseSelectorAct
CODESTARTparseSelectorAct
CODE_STD_STRING_REQUESTparseSelectorAct(1)
	/* first check if this config line is actually for us */
	if(strncmp((char*) p, ":mmjsonparse:", sizeof(":mmjsonparse:") - 1)) {
		ABORT_FINALIZE(RS_RET_CONFLINE_UNPROCESSED);
	}

	/* ok, if we reach this point, we have something for us */
	p += sizeof(":mmjsonparse:") - 1; /* eat indicator sequence  (-1 because of '\0'!) */
	CHKiRet(createInstance(&pData));

	/* check if a non-standard template is to be applied */
	if(*(p-1) == ';')
		--p;
	/* we call the function below because we need to call it via our interface definition. However,
	 * the format specified (if any) is always ignored.
	 */
	iRet = cflineParseTemplateName(&p, *ppOMSR, 0, OMSR_TPL_AS_MSG, (uchar*) "RSYSLOG_FileFormat");
CODE_STD_FINALIZERparseSelectorAct
ENDparseSelectorAct


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



/* Reset config variables for this module to default values.
 */
static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal)
{
	DEFiRet;
	RETiRet;
}


BEGINmodInit()
	rsRetVal localRet;
	rsRetVal (*pomsrGetSupportedTplOpts)(unsigned long *pOpts);
	unsigned long opts;
	int bMsgPassingSupported;
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION;
		/* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	DBGPRINTF("mmjsonparse: module compiled with rsyslog version %s.\n", VERSION);
	/* check if the rsyslog core supports parameter passing code */
	bMsgPassingSupported = 0;
	localRet = pHostQueryEtryPt((uchar*)"OMSRgetSupportedTplOpts",
			&pomsrGetSupportedTplOpts);
	if(localRet == RS_RET_OK) {
		/* found entry point, so let's see if core supports msg passing */
		CHKiRet((*pomsrGetSupportedTplOpts)(&opts));
		if(opts & OMSR_TPL_AS_MSG)
			bMsgPassingSupported = 1;
	} else if(localRet != RS_RET_ENTRY_POINT_NOT_FOUND) {
		ABORT_FINALIZE(localRet); /* Something else went wrong, not acceptable */
	}
	
	if(!bMsgPassingSupported) {
		DBGPRINTF("mmjsonparse: msg-passing is not supported by rsyslog core, "
			  "can not continue.\n");
		ABORT_FINALIZE(RS_RET_NO_MSG_PASSING);
	}

	
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler,
				    resetConfigVariables, NULL, STD_LOADABLE_MODULE_ID));
ENDmodInit

/* vi:set ai:
 */
