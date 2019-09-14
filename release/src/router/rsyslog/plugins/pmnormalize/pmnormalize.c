/* pmnormalize.c
 * This is a parser module for parsing incoming messages using liblognorm.
 *
 * File begun on 2017-03-03 by Pascal Withopf.
 *
 * Copyright 2014-2018 Adiscon GmbH.
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
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <liblognorm.h>
#include <json.h>
#include "conf.h"
#include "syslogd-types.h"
#include "template.h"
#include "msg.h"
#include "module-template.h"
#include "glbl.h"
#include "errmsg.h"
#include "parser.h"
#include "datetime.h"
#include "unicode-helper.h"

MODULE_TYPE_PARSER
MODULE_TYPE_NOKEEP
PARSER_NAME("rsyslog.pmnormalize")
MODULE_CNFNAME("pmnormalize")

/* internal structures */
DEF_PMOD_STATIC_DATA
DEFobjCurrIf(glbl)
DEFobjCurrIf(parser)
DEFobjCurrIf(datetime)


/* parser instance parameters */
static struct cnfparamdescr parserpdescr[] = {
	{ "rulebase", eCmdHdlrGetWord, 0 },
	{ "rule", eCmdHdlrArray, 0 },
	{ "undefinedpropertyerror", eCmdHdlrBinary, 0 }
};
static struct cnfparamblk parserpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(parserpdescr)/sizeof(struct cnfparamdescr),
	  parserpdescr
	};

struct instanceConf_s {
	sbool undefPropErr;
	char *rulebase;
	char *rule;
	ln_ctx ctxln;		/*context to be used for liblognorm*/
	char *pszPath;		/*path of normalized data*/
};

BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	if(eFeat == sFEATUREAutomaticSanitazion)
		iRet = RS_RET_OK;
	if(eFeat == sFEATUREAutomaticPRIParsing)
		iRet = RS_RET_OK;
ENDisCompatibleWithFeature

/* create input instance, set default parameters, and
 * add it to the list of instances.
 */
static rsRetVal
createInstance(instanceConf_t **pinst)
{
	instanceConf_t *inst;
	DEFiRet;
	CHKmalloc(inst = MALLOC(sizeof(instanceConf_t)));
	inst->undefPropErr = 0;
	inst->rulebase = NULL;
	inst->rule = NULL;
	inst->ctxln = NULL;
	*pinst = inst;
finalize_it:
	RETiRet;
}

/* callback for liblognorm error messages */
static void
errCallBack(void __attribute__((unused)) *cookie, const char *msg,
	    size_t __attribute__((unused)) lenMsg)
{
	LogError(0, RS_RET_ERR_LIBLOGNORM, "liblognorm error: %s", msg);
}

/* to be called to build the liblognorm part of the instance ONCE ALL PARAMETERS ARE CORRECT
 * (and set within inst!).
 */
static rsRetVal
buildInstance(instanceConf_t *inst)
{
	DEFiRet;
	if((inst->ctxln = ln_initCtx()) == NULL) {
		LogError(0, RS_RET_ERR_LIBLOGNORM_INIT, "error: could not initialize "
				"liblognorm ctx, cannot activate action");
		ABORT_FINALIZE(RS_RET_ERR_LIBLOGNORM_INIT);
	}
	ln_setErrMsgCB(inst->ctxln, errCallBack, NULL);

	if(inst->rule != NULL && inst->rulebase == NULL) {
		if(ln_loadSamplesFromString(inst->ctxln, inst->rule) !=0) {
			LogError(0, RS_RET_NO_RULEBASE, "error: normalization rulebase '%s' "
					"could not be loaded cannot activate action", inst->rulebase);
			ln_exitCtx(inst->ctxln);
			ABORT_FINALIZE(RS_RET_ERR_LIBLOGNORM_SAMPDB_LOAD);
		}
		free(inst->rule);
		inst->rule = NULL;
	} else if(inst->rulebase != NULL && inst->rule == NULL) {
		if(ln_loadSamples(inst->ctxln, (char*) inst->rulebase) != 0) {
			LogError(0, RS_RET_NO_RULEBASE, "error: normalization rulebase '%s' "
					"could not be loaded cannot activate action", inst->rulebase);
			ln_exitCtx(inst->ctxln);
			ABORT_FINALIZE(RS_RET_ERR_LIBLOGNORM_SAMPDB_LOAD);
		}
	}
finalize_it:
	RETiRet;
}


BEGINfreeParserInst
CODESTARTfreeParserInst
	dbgprintf("pmnormalize: free parser instance %p\n", pInst);
	if(pInst->ctxln != NULL) {
		ln_exitCtx(pInst->ctxln);
	}
ENDfreeParserInst


BEGINnewParserInst
	struct cnfparamvals *pvals = NULL;
	int i;
CODESTARTnewParserInst
	DBGPRINTF("newParserInst (pmnormalize)\n");

	inst = NULL;
	CHKiRet(createInstance(&inst));

	if(lst == NULL)
		FINALIZE;  /* just set defaults, no param block! */

	if((pvals = nvlstGetParams(lst, &parserpblk, NULL)) == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	if(Debug) {
		dbgprintf("parser param blk in pmnormalize:\n");
		cnfparamsPrint(&parserpblk, pvals);
	}

	for(i = 0 ; i < parserpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(parserpblk.descr[i].name, "undefinedpropertyerror")) {
			inst->undefPropErr = (int) pvals[i].val.d.n;
		} else if(!strcmp(parserpblk.descr[i].name, "rulebase")) {
			inst->rulebase = (char *) es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(parserpblk.descr[i].name, "rule")) {
			es_str_t *rules;
			CHKmalloc(rules = es_newStr(128));
			for(int j=0; j < pvals[i].val.d.ar->nmemb; ++j) {
				CHKiRet(es_addStr(&rules, pvals[i].val.d.ar->arr[j]));
				CHKiRet(es_addChar(&rules, '\n'));
			}
			inst->rule = (char*)es_str2cstr(rules, NULL);
			if(rules != NULL)
				es_deleteStr(rules);
		} else {
			LogError(0, RS_RET_INTERNAL_ERROR ,
				"pmnormalize: program error, non-handled param '%s'",
				parserpblk.descr[i].name);
		}
	}
	if(!inst->rulebase && !inst->rule) {
		LogError(0, RS_RET_CONFIG_ERROR, "pmnormalize: rulebase needed. "
				"Use option rulebase or rule.");
	}
	if(inst->rulebase && inst->rule) {
		LogError(0, RS_RET_CONFIG_ERROR, "pmnormalize: only one rulebase "
				"possible, rulebase can't be used with rule");
	}

	iRet = buildInstance(inst);
finalize_it:
CODE_STD_FINALIZERnewParserInst
	if(lst != NULL)
		cnfparamvalsDestruct(pvals, &parserpblk);
	if(iRet != RS_RET_OK)
		freeParserInst(inst);
ENDnewParserInst


BEGINparse2
	uchar *buf;
	rs_size_t len;
	int r;
	struct json_object *json = NULL;
CODESTARTparse2
	DBGPRINTF("Message will now be parsed by pmnormalize\n");
	/*Msg OffSet needs to be set*/
	MsgSetMSGoffs(pMsg, 0);

	getRawMsg(pMsg, &buf, &len);
	r = ln_normalize(pInst->ctxln, (char*)buf, len, &json);
	if(r != 0) {
		DBGPRINTF("error %d during ln_normalize\n", r);
		if(pInst->undefPropErr) {
			LogError(0, RS_RET_ERR, "error %d during ln_normalize; "
					"json: %s\n", r, fjson_object_to_json_string(json));
		}
	} else {
		iRet = MsgSetPropsViaJSON_Object(pMsg, json);
	}

ENDparse2


BEGINmodExit
CODESTARTmodExit
	/* release what we no longer need */
	objRelease(glbl, CORE_COMPONENT);
	objRelease(parser, CORE_COMPONENT);
	objRelease(datetime, CORE_COMPONENT);
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_PMOD2_QUERIES
CODEqueryEtryPt_IsCompatibleWithFeature_IF_OMOD_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(parser, CORE_COMPONENT));
	CHKiRet(objUse(datetime, CORE_COMPONENT));

	DBGPRINTF("pmnormalize parser init called\n");
ENDmodInit
