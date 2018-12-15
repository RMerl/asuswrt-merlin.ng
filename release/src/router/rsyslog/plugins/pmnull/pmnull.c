/* pmnull.c
 * This is a parser module for CISCO IOS "syslog" format.
 *
 * File begun on 2014-07-07 by RGerhards
 *
 * Copyright 2014-2015 Rainer Gerhards and Adiscon GmbH.
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
PARSER_NAME("rsyslog.pmnull")
MODULE_CNFNAME("pmnull")

/* internal structures */
DEF_PMOD_STATIC_DATA
DEFobjCurrIf(glbl)
DEFobjCurrIf(parser)
DEFobjCurrIf(datetime)


/* parser instance parameters */
static struct cnfparamdescr parserpdescr[] = {
	{ "tag", eCmdHdlrString, 0 },
	{ "syslogfacility", eCmdHdlrFacility, 0 },
	{ "syslogseverity", eCmdHdlrSeverity, 0 }
};
static struct cnfparamblk parserpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(parserpdescr)/sizeof(struct cnfparamdescr),
	  parserpdescr
	};

struct instanceConf_s {
	const char *tag;
	size_t lenTag;
	int pri;
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
	inst->tag = NULL;
	*pinst = inst;
finalize_it:
	RETiRet;
}


BEGINfreeParserInst
CODESTARTfreeParserInst
	dbgprintf("pmnull: free parser instance %p\n", pInst);
ENDfreeParserInst


BEGINnewParserInst
	struct cnfparamvals *pvals = NULL;
	int i;
	int syslogfacility = 1; /* default as of rfc3164 */
	int syslogseverity = 5; /* default as of rfc3164 */
CODESTARTnewParserInst
	DBGPRINTF("newParserInst (pmnull)\n");

	inst = NULL;
	CHKiRet(createInstance(&inst));

	if(lst == NULL)
		FINALIZE;  /* just set defaults, no param block! */

	if((pvals = nvlstGetParams(lst, &parserpblk, NULL)) == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	if(Debug) {
		dbgprintf("parser param blk in pmnull:\n");
		cnfparamsPrint(&parserpblk, pvals);
	}

	for(i = 0 ; i < parserpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(parserpblk.descr[i].name, "tag")) {
			inst->tag = (const char *) es_str2cstr(pvals[i].val.d.estr, NULL);
			inst->lenTag = strlen(inst->tag);
		} else if(!strcmp(parserpblk.descr[i].name, "syslogfacility")) {
			syslogfacility = pvals[i].val.d.n;
		} else if(!strcmp(parserpblk.descr[i].name, "syslogseverity")) {
			syslogseverity = pvals[i].val.d.n;
		} else {
			dbgprintf("pmnull: program error, non-handled "
				"param '%s'\n", parserpblk.descr[i].name);
		}
	}
	inst->pri = syslogfacility*8 + syslogseverity;
finalize_it:
CODE_STD_FINALIZERnewParserInst
	if(lst != NULL)
		cnfparamvalsDestruct(pvals, &parserpblk);
	if(iRet != RS_RET_OK)
		freeParserInst(inst);
ENDnewParserInst


BEGINparse2
CODESTARTparse2
	DBGPRINTF("Message will now be parsed by pmnull\n");
	if(pInst->tag != NULL) {
		MsgSetTAG(pMsg, (uchar *)pInst->tag, pInst->lenTag);
	}
	msgSetPRI(pMsg, pInst->pri);
	MsgSetMSGoffs(pMsg, 0);
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

	DBGPRINTF("pmnull parser init called\n");
ENDmodInit
