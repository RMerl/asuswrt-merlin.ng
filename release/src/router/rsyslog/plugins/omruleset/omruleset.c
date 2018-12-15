/* omruleset.c
 * This is a very special output module. It permits to pass a message object
 * to another rule set. While this is a very simple action, it enables very
 * complex configurations, e.g. it supports high-speed "and" conditions, sending
 * data to the same file in a non-racy way, include functionality as well as
 * some high-performance optimizations (in case the rule sets have the necessary
 * queue definitions). So while this code is small, it is pretty important.
 *
 * NOTE: read comments in module-template.h for details on the calling interface!
 *
 * File begun on 2009-11-02 by RGerhards
 *
 * Copyright 2009-2016 Rainer Gerhards and Adiscon GmbH.
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
#include "conf.h"
#include "syslogd-types.h"
#include "template.h"
#include "module-template.h"
#include "errmsg.h"
#include "ruleset.h"
#include "cfsysline.h"
#include "dirty.h"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("omruleset")

static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal);

/* static data */
DEFobjCurrIf(ruleset);

/* internal structures
 */
DEF_OMOD_STATIC_DATA

/* config variables */


typedef struct _instanceData {
	ruleset_t *pRuleset;	/* ruleset to enqueue to */
	uchar *pszRulesetName;	/* primarily for debugging/display purposes */
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
} wrkrInstanceData_t;

typedef struct configSettings_s {
	ruleset_t *pRuleset;	/* ruleset to enqueue message to (NULL = Default, not recommended) */
	uchar *pszRulesetName;
} configSettings_t;
static configSettings_t cs;

BEGINinitConfVars		/* (re)set config variables to default values */
CODESTARTinitConfVars
	resetConfigVariables(NULL, NULL);
ENDinitConfVars


BEGINcreateInstance
CODESTARTcreateInstance
ENDcreateInstance


BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
ENDcreateWrkrInstance


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
ENDisCompatibleWithFeature


BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance


BEGINfreeInstance
CODESTARTfreeInstance
	free(pData->pszRulesetName);
ENDfreeInstance


BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
	dbgprintf("omruleset target %s[%p]\n", (char*) pData->pszRulesetName, pData->pRuleset);
ENDdbgPrintInstInfo


BEGINtryResume
CODESTARTtryResume
ENDtryResume

/* Note that we change the flow control type to "no delay", because at this point in
 * rsyslog procesing we can not really slow down the producer any longer, as we already
 * work off a queue. So a delay would just block out execution for longer than needed.
 */
BEGINdoAction_NoStrings
	smsg_t **ppMsg = (smsg_t **) pMsgData;
	smsg_t *pMsg;
CODESTARTdoAction
	CHKmalloc(pMsg = MsgDup(ppMsg[0]));
	DBGPRINTF(":omruleset: forwarding message %p to ruleset %s[%p]\n", pMsg,
		  (char*) pWrkrData->pData->pszRulesetName, pWrkrData->pData->pRuleset);
	MsgSetFlowControlType(pMsg, eFLOWCTL_NO_DELAY);
	MsgSetRuleset(pMsg, pWrkrData->pData->pRuleset);
	/* Note: we intentionally use submitMsg2() here, as we process messages
	 * that were already run through the rate-limiter. So it is (at least)
	 * questionable if they were rate-limited again.
	 */
	submitMsg2(pMsg);
finalize_it:
ENDdoAction

/* set the ruleset name */
static rsRetVal
setRuleset(void __attribute__((unused)) *pVal, uchar *pszName)
{
	rsRetVal localRet;
	DEFiRet;

	localRet = ruleset.GetRuleset(ourConf, &cs.pRuleset, pszName);
	if(localRet == RS_RET_NOT_FOUND) {
		LogError(0, RS_RET_RULESET_NOT_FOUND, "error: ruleset '%s' not found - ignored", pszName);
	}
	CHKiRet(localRet);
	cs.pszRulesetName = pszName; /* save for later display purposes */

finalize_it:
	if(iRet != RS_RET_OK) { /* cleanup needed? */
		free(pszName);
	}
	RETiRet;
}


BEGINparseSelectorAct
	int iTplOpts;
CODESTARTparseSelectorAct
CODE_STD_STRING_REQUESTparseSelectorAct(1)
	/* first check if this config line is actually for us */
	if(strncmp((char*) p, ":omruleset:", sizeof(":omruleset:") - 1)) {
		ABORT_FINALIZE(RS_RET_CONFLINE_UNPROCESSED);
	}

	if(cs.pRuleset == NULL) {
		LogError(0, RS_RET_NO_RULESET, "error: no ruleset was specified, use "
				"$ActionOmrulesetRulesetName directive first!");
		ABORT_FINALIZE(RS_RET_NO_RULESET);
	}

	/* ok, if we reach this point, we have something for us */
	p += sizeof(":omruleset:") - 1; /* eat indicator sequence  (-1 because of '\0'!) */
	CHKiRet(createInstance(&pData));

	LogMsg(0, RS_RET_DEPRECATED, LOG_WARNING,
			"warning: omruleset is deprecated, consider "
			"using the 'call' statement instead");

	/* check if a non-standard template is to be applied */
	if(*(p-1) == ';')
		--p;
	iTplOpts = OMSR_TPL_AS_MSG;
	/* we call the message below because we need to call it via our interface definition. However,
	 * the format specified (if any) is always ignored.
	 */
	CHKiRet(cflineParseTemplateName(&p, *ppOMSR, 0, iTplOpts, (uchar*) "RSYSLOG_FileFormat"));
	pData->pRuleset = cs.pRuleset;
	pData->pszRulesetName = cs.pszRulesetName;
	cs.pRuleset = NULL;
/* re-set, because there is a high risk of unwanted behavior if we leave it in! */
	cs.pszRulesetName = NULL;
/* note: we must not free, as we handed over this pointer to the instanceDat to the instanceDataa! */
CODE_STD_FINALIZERparseSelectorAct
ENDparseSelectorAct


BEGINmodExit
CODESTARTmodExit
	free(cs.pszRulesetName);
	objRelease(ruleset, CORE_COMPONENT);
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_OMOD_QUERIES
CODEqueryEtryPt_STD_OMOD8_QUERIES
CODEqueryEtryPt_STD_CONF2_CNFNAME_QUERIES
ENDqueryEtryPt



/* Reset config variables for this module to default values.
 */
static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal)
{
	DEFiRet;
	cs.pRuleset = NULL;
	free(cs.pszRulesetName);
	cs.pszRulesetName = NULL;
	RETiRet;
}


BEGINmodInit()
	rsRetVal localRet;
	rsRetVal (*pomsrGetSupportedTplOpts)(unsigned long *pOpts);
	unsigned long opts;
	int bMsgPassingSupported;		/* does core support template passing as an array? */
CODESTARTmodInit
INITLegCnfVars
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	/* check if the rsyslog core supports parameter passing code */
	bMsgPassingSupported = 0;
	localRet = pHostQueryEtryPt((uchar*)"OMSRgetSupportedTplOpts", &pomsrGetSupportedTplOpts);
	if(localRet == RS_RET_OK) {
		/* found entry point, so let's see if core supports msg passing */
		CHKiRet((*pomsrGetSupportedTplOpts)(&opts));
		if(opts & OMSR_TPL_AS_MSG)
			bMsgPassingSupported = 1;
	} else if(localRet != RS_RET_ENTRY_POINT_NOT_FOUND) {
		ABORT_FINALIZE(localRet); /* Something else went wrong, what is not acceptable */
	}
	
	if(!bMsgPassingSupported) {
		DBGPRINTF("omruleset: msg-passing is not supported by rsyslog core, can not continue.\n");
		ABORT_FINALIZE(RS_RET_NO_MSG_PASSING);
	}

	CHKiRet(objUse(ruleset, CORE_COMPONENT));

	LogMsg(0, RS_RET_DEPRECATED, LOG_WARNING,
			"warning: omruleset is deprecated, consider "
			"using the 'call' statement instead");

	CHKiRet(omsdRegCFSLineHdlr((uchar *)"actionomrulesetrulesetname", 0, eCmdHdlrGetWord,
				    setRuleset, NULL, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler,
				    resetConfigVariables, NULL, STD_LOADABLE_MODULE_ID));
ENDmodInit

/* vi:set ai:
 */
