/* pmrciscoios.c
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
PARSER_NAME("rsyslog.ciscoios")
MODULE_CNFNAME("pmciscoios")

/* internal structures */
DEF_PMOD_STATIC_DATA
DEFobjCurrIf(glbl)
DEFobjCurrIf(parser)
DEFobjCurrIf(datetime)


/* parser instance parameters */
static struct cnfparamdescr parserpdescr[] = {
	{ "present.origin", eCmdHdlrBinary, 0 },
	{ "present.xr", eCmdHdlrBinary, 0 }
};
static struct cnfparamblk parserpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(parserpdescr)/sizeof(struct cnfparamdescr),
	  parserpdescr
	};

struct instanceConf_s {
	int bOriginPresent; /* is ORIGIN field present? */
	int bXrPresent; /* is XR? */
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
	inst->bOriginPresent = 0;
	inst->bXrPresent = 0;
	*pinst = inst;
finalize_it:
	RETiRet;
}


BEGINfreeParserInst
CODESTARTfreeParserInst
	dbgprintf("pmciscoios: free parser instance %p\n", pInst);
ENDfreeParserInst


BEGINnewParserInst
	struct cnfparamvals *pvals = NULL;
	int i;
CODESTARTnewParserInst
	DBGPRINTF("newParserInst (pmciscoios)\n");

	inst = NULL;
	CHKiRet(createInstance(&inst));

	if(lst == NULL)
		FINALIZE;  /* just set defaults, no param block! */

	if((pvals = nvlstGetParams(lst, &parserpblk, NULL)) == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	if(Debug) {
		dbgprintf("parser param blk in pmciscoios:\n");
		cnfparamsPrint(&parserpblk, pvals);
	}

	for(i = 0 ; i < parserpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(parserpblk.descr[i].name, "present.origin")) {
			inst->bOriginPresent = (int) pvals[i].val.d.n;
		} else if(!strcmp(parserpblk.descr[i].name, "present.xr")) {
			inst->bXrPresent = (int) pvals[i].val.d.n;
		} else {
			dbgprintf("pmciscoios: program error, non-handled "
				"param '%s'\n", parserpblk.descr[i].name);
		}
	}
finalize_it:
CODE_STD_FINALIZERnewParserInst
	if(lst != NULL)
		cnfparamvalsDestruct(pvals, &parserpblk);
	if(iRet != RS_RET_OK)
		freeParserInst(inst);
ENDnewParserInst


BEGINparse2
	uchar *p2parse;
	long long msgcounter;
	int lenMsg;
	int i;
	int iHostname = 0;
	uchar bufParseTAG[512];
	uchar bufParseHOSTNAME[CONF_HOSTNAME_MAXSIZE]; /* used by origin */
CODESTARTparse2
	DBGPRINTF("Message will now be parsed by pmciscoios\n");
	assert(pMsg != NULL);
	assert(pMsg->pszRawMsg != NULL);
	lenMsg = pMsg->iLenRawMsg - pMsg->offAfterPRI;
	/* note: offAfterPRI is already the number of PRI chars (do not add one!) */
	p2parse = pMsg->pszRawMsg + pMsg->offAfterPRI; /* point to start of text, after PRI */

	/* first obtain the MESSAGE COUNTER. It must be numeric up until
	 * the ": " terminator sequence
	 */
	msgcounter = 0;
	while(lenMsg > 0 && (*p2parse >= '0' && *p2parse <= '9') ) {
		msgcounter = msgcounter * 10 + *p2parse - '0';
		++p2parse, --lenMsg;
	}
	DBGPRINTF("pmciscoios: msgcntr %lld\n", msgcounter);

	/* delimiter check */
	if(lenMsg < 2 || *p2parse != ':' || *(p2parse+1) != ' ') {
		DBGPRINTF("pmciscoios: fail after seqno: '%s'\n", p2parse);
		ABORT_FINALIZE(RS_RET_COULD_NOT_PARSE);
	}
	p2parse += 2;

	/* ORIGIN (optional) */
	if(pInst->bOriginPresent) {
		iHostname = 0;
		while(   lenMsg > 1
		      && !(*p2parse == ':' && *(p2parse+1) == ' ')  /* IPv6 is e.g. "::1" (loopback) */
		      && iHostname < (int) sizeof(bufParseHOSTNAME) - 1 ) {
			bufParseHOSTNAME[iHostname++] = *p2parse++;
			--lenMsg;
		}
		bufParseHOSTNAME[iHostname] = '\0';
		/* delimiter check */
		if(lenMsg < 2 || *(p2parse+1) != ' ') {
			DBGPRINTF("pmciscoios: fail after origin: '%s'\n", p2parse);
			ABORT_FINALIZE(RS_RET_COULD_NOT_PARSE);
		}
		p2parse += 2;
	}

	/* XR RSP (optional) */
	if(pInst->bXrPresent) {
		while(   lenMsg > 1
			&& !(*p2parse == ':')) {
			--lenMsg;
			++p2parse;
		}
		/* delimiter check */
		if(lenMsg < 2) {
			DBGPRINTF("pmciscoios: fail after XR: '%s'\n", p2parse);
			ABORT_FINALIZE(RS_RET_COULD_NOT_PARSE);
		}
		p2parse += 1;
	}

	/* TIMESTAMP */
	if(p2parse[0] == '*' || p2parse[0] == '.') p2parse++;
	if(datetime.ParseTIMESTAMP3164(&(pMsg->tTIMESTAMP), &p2parse, &lenMsg, PARSE3164_TZSTRING,
	NO_PERMIT_YEAR_AFTER_TIME) == RS_RET_OK) {
		if(pMsg->dfltTZ[0] != '\0')
			applyDfltTZ(&pMsg->tTIMESTAMP, pMsg->dfltTZ);
	} else {
		DBGPRINTF("pmciscoios: fail at timestamp: '%s'\n", p2parse);
		ABORT_FINALIZE(RS_RET_COULD_NOT_PARSE);
	}
	/* Note: date parser strips ": ", so we cannot do the delimiter check here */

	/* XR RSP (optional) */
	if(pInst->bXrPresent) {
		while(   lenMsg > 1
			&& !(*p2parse == '%')) {
			--lenMsg;
			p2parse++;
		}
		/* delimiter check */
		if(lenMsg < 2) {
			DBGPRINTF("pmciscoios: fail after XR tag search: '%s'\n", p2parse);
			ABORT_FINALIZE(RS_RET_COULD_NOT_PARSE);
		}
	}

	/* parse SYSLOG TAG. must always start with '%', else we have a field mismatch */
	if(lenMsg < 1 || *p2parse != '%') {
		DBGPRINTF("pmciscoios: fail at tag begin (no '%%'): '%s'\n", p2parse);
		ABORT_FINALIZE(RS_RET_COULD_NOT_PARSE);
	}

	i = 0;
	while(lenMsg > 0 && *p2parse != ':' && *p2parse != ' ' && i < (int) sizeof(bufParseTAG) - 2) {
		bufParseTAG[i++] = *p2parse++;
		--lenMsg;
	}
	/* delimiter check */
	if(pInst->bXrPresent) p2parse++;
	if(lenMsg < 2 || *p2parse != ':' || *(p2parse+1) != ' ') {
		DBGPRINTF("pmciscoios: fail after tag: '%s'\n", p2parse);
		ABORT_FINALIZE(RS_RET_COULD_NOT_PARSE);
	}

	++p2parse;
	bufParseTAG[i++] = ':';
	bufParseTAG[i] = '\0';	/* terminate string */

	/* if we reach this point, we have a wellformed message and can persist the values */
	MsgSetTAG(pMsg, bufParseTAG, i);
	/* if bOriginPresent !=0 iHostname gets initialized */
	if(pInst->bOriginPresent)
		MsgSetHOSTNAME(pMsg, bufParseHOSTNAME, iHostname);
	MsgSetMSGoffs(pMsg, p2parse - pMsg->pszRawMsg);
	setProtocolVersion(pMsg, MSG_LEGACY_PROTOCOL);
finalize_it:
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

	DBGPRINTF("pmciscoios parser init called\n");
ENDmodInit
