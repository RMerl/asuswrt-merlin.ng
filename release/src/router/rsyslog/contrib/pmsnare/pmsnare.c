/* pmsnare.c
 *
 * this detects logs sent by Snare and cleans them up so that they can be processed by the normal parser
 *
 * there are two variations of this, if the client is set to 'syslog' mode it sends
 *
 * <pri>timestamp<sp>hostname<sp>tag<tab>otherstuff
 *
 * if the client is not set to syslog it sends
 *
 * hostname<tab>tag<tab>otherstuff
 *
 * The tabs can be represented in different ways. This module will auto-detect the tab representation based on
 * the global config settings, but they can be overridden for each instance in the config file if needed.
 *
 * ToDo, take advantage of items in the message itself to set more friendly information
 * where the normal parser will find it by re-writing more of the message
 *
 * Interesting information includes:
 *
 * in the case of windows snare messages:
 *   the system hostname is field 12
 *   the severity is field 3 (criticality ranging form 0 to 4)
 *   the source of the log is field 4 and may be able to be mapped to facility
 *
 *
 * created 2010-12-13 by David Lang based on pmlastmsg
 * Modified 2017-05-29 by Shane Lawrence.
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
PARSER_NAME("rsyslog.snare")
MODULE_CNFNAME("pmsnare")

/* internal structures
 */
DEF_PMOD_STATIC_DATA
DEFobjCurrIf(glbl)
DEFobjCurrIf(parser)
DEFobjCurrIf(datetime)


/* static data */
static int bParseHOSTNAMEandTAG;	/* cache for the equally-named global param - performance enhancement */

/* Keep a list of parser instances so we can apply global settings after config is loaded. */
typedef struct modInstances_s {
	instanceConf_t *root;
	instanceConf_t *tail;
} modInstances_t;
static modInstances_t *modInstances = NULL;

struct modConfData_s {
	rsconf_t *pConf;	/* our overall config object */
};
static modConfData_t *modConf = NULL;

/* Per-instance config settings. */
static struct cnfparamdescr parserpdescr[] = {
	{ "parser.controlcharacterescapeprefix", eCmdHdlrGetChar, 0 },
	{ "parser.escapecontrolcharactersonreceive", eCmdHdlrBinary, 0 },
	{ "parser.escapecontrolcharactertab", eCmdHdlrBinary, 0},
	{ "parser.escapecontrolcharacterscstyle", eCmdHdlrBinary, 0 }
};
static struct cnfparamblk parserpblk = {
	CNFPARAMBLK_VERSION,
	sizeof(parserpdescr)/sizeof(struct cnfparamdescr),
	parserpdescr
};
struct instanceConf_s {
	int bEscapeCCOnRcv;
	int bEscapeTab;
	int bParserEscapeCCCStyle;
	uchar cCCEscapeChar;
	int tabLength;
	char tabRepresentation[5];
	struct instanceConf_s *next;
};

/* Creates the instance and adds it to the list of instances. */
static rsRetVal createInstance(instanceConf_t **pinst) {
	instanceConf_t *inst;
	DEFiRet;
	CHKmalloc(inst = MALLOC(sizeof(instanceConf_t)));
	inst->next = NULL;
	*pinst = inst;
	
	/*  Add to list of instances. */
	if(modInstances == NULL) {
		CHKmalloc(modInstances = MALLOC(sizeof(modInstances_t)));
		modInstances->tail = modInstances->root = NULL;
	}
	if (modInstances->tail == NULL) {
		modInstances->tail = modInstances->root = inst;
	} else {
		modInstances->tail->next = inst;
		modInstances->tail = inst;
	}

	finalize_it:
	RETiRet;
}

BEGINnewParserInst
	struct cnfparamvals *pvals = NULL;
	int i;
CODESTARTnewParserInst
	DBGPRINTF("newParserInst (pmsnare)\n");
	inst = NULL;
	CHKiRet(createInstance(&inst));

	/* Mark these as unset so we know if they should be overridden later. */
	inst->bEscapeCCOnRcv = -1;
	inst->bEscapeTab = -1;
	inst->bParserEscapeCCCStyle = -1;
	inst->cCCEscapeChar = '\0';

	/* If using the old config, just use global settings for each instance. */
	if (lst == NULL)
		FINALIZE;
	
	/* If using the new config, process module settings for this instance. */
	if((pvals = nvlstGetParams(lst, &parserpblk, NULL)) == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	if(Debug) {
		dbgprintf("pmsnare: parser param blk:\n");
		cnfparamsPrint(&parserpblk, pvals);
	}

	for(i = 0 ; i < parserpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(parserpblk.descr[i].name, "parser.escapecontrolcharactersonreceive")) {
			inst->bEscapeCCOnRcv = pvals[i].val.d.n;
		} else if(!strcmp(parserpblk.descr[i].name, "parser.escapecontrolcharactertab")) {
			inst->bEscapeTab = pvals[i].val.d.n;
		} else if(!strcmp(parserpblk.descr[i].name, "parser.escapecontrolcharacterscstyle")) {
			inst->bParserEscapeCCCStyle = pvals[i].val.d.n;
		} else if(!strcmp(parserpblk.descr[i].name, "parser.controlcharacterescapeprefix")) {
			inst->cCCEscapeChar = (uchar) *es_str2cstr(pvals[i].val.d.estr, NULL);
		} else {
			dbgprintf("pmsnare: program error, non-handled param '%s'\n", parserpblk.descr[i].name);
		}
	}
	
finalize_it:
CODE_STD_FINALIZERnewParserInst
	if(lst != NULL)
		cnfparamvalsDestruct(pvals, &parserpblk);
	if(iRet != RS_RET_OK)
		free(inst);
ENDnewParserInst

BEGINfreeParserInst
CODESTARTfreeParserInst
	dbgprintf("pmsnare: free parser instance %p\n", pInst);
ENDfreeParserInst

BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	if(eFeat == sFEATUREAutomaticSanitazion)
		iRet = RS_RET_OK;
	if(eFeat == sFEATUREAutomaticPRIParsing)
		iRet = RS_RET_OK;
ENDisCompatibleWithFeature

/* Interface with the global config. */
BEGINbeginCnfLoad
CODESTARTbeginCnfLoad
	modConf = pModConf;
	pModConf->pConf = pConf;
ENDbeginCnfLoad

BEGINsetModCnf
CODESTARTsetModCnf
	/* Could use module-globals here, but not global globals. */
	(void) lst;
ENDsetModCnf

BEGINendCnfLoad
	instanceConf_t *inst;
CODESTARTendCnfLoad
	dbgprintf("pmsnare: Begin endCnfLoad\n");
	/* Loop through each parser instance and apply global settings to any option that hasn't been overridden.
	 * This can't be done any earlier because the config wasn't fully loaded until now. */
	for(inst = modInstances->root; inst != NULL; inst = inst->next) {
		if(inst->bEscapeCCOnRcv == -1)
			inst->bEscapeCCOnRcv = glbl.GetParserEscapeControlCharactersOnReceive();
		if(inst->bEscapeTab == -1)
			inst->bEscapeTab = glbl.GetParserEscapeControlCharacterTab();
		if(inst->bParserEscapeCCCStyle == -1)
			inst->bParserEscapeCCCStyle = glbl.GetParserEscapeControlCharactersCStyle();
		if(inst->cCCEscapeChar == '\0')
			inst->cCCEscapeChar = glbl.GetParserControlCharacterEscapePrefix();

		/* Determine tab representation. Possible options:
		 *		"#011"	escape on, escapetabs on, no change to prefix (default)
		 *		"?011"	prefix changed in config
		 *		"\\t"	C style
		 *		'\t' 	escape turned off
		 */
		if (inst->bEscapeCCOnRcv && inst->bEscapeTab) {
			if (inst->bParserEscapeCCCStyle) {
				strncpy(inst->tabRepresentation, "\\t", 5);
			} else {
				strncpy(inst->tabRepresentation, "#011", 5);
				inst->tabRepresentation[0] = inst->cCCEscapeChar;
			}
		} else {
			strncpy(inst->tabRepresentation, "\t", 5);
		}
		inst->tabLength=strlen(inst->tabRepresentation);
		/* TODO: This debug message would be more useful if it told which Snare instance! */
		dbgprintf("pmsnare: Snare parser will treat '%s' as tab.\n", inst->tabRepresentation);
	}

	assert(pModConf == modConf);
ENDendCnfLoad

BEGINcheckCnf
CODESTARTcheckCnf
ENDcheckCnf

BEGINactivateCnf
CODESTARTactivateCnf
ENDactivateCnf

BEGINfreeCnf
	instanceConf_t *inst, *del;
CODESTARTfreeCnf
	for(inst = modInstances->root ; inst != NULL ; ) {
		del = inst;
		inst = inst->next;
		free(del);
	}
	free(modInstances);
ENDfreeCnf

BEGINparse2
	uchar *p2parse;
	int lenMsg;
	int snaremessage; /* 0 means not a snare message, otherwise it's the index of the tab after the tag  */
	
CODESTARTparse2
	dbgprintf("Message will now be parsed by fix Snare parser.\n");
	assert(pMsg != NULL);
	assert(pMsg->pszRawMsg != NULL);

	/* check if this message is of the type we handle in this (very limited) parser
	 *
	 * - Find out if the first separator is a tab.
	 * - If it is, see if the second word is one of our expected tags.
	 *   - If so, flag as Snare and replace the first tab with space so that
	 *     hostname and syslog tag are going to be parsed properly
	 *   - Else not a snare message, abort.
	 * - Else assume valid 3164 timestamp, move over to the syslog tag.
	 * - See if syslog header is followed by tab and one of our expected tags.
	 *   - If so, flag as Snare.
	 * - See if either type flagged as Snare.
	 *   - If so, replace the tab with a space so that it will be parsed properly.
	 */

	snaremessage=0;
	/* note: offAfterPRI is already the number of PRI chars (do not add one!) */
	lenMsg = pMsg->iLenRawMsg - pMsg->offAfterPRI;
	/* point to start of text, after PRI */
	p2parse = pMsg->pszRawMsg + pMsg->offAfterPRI;
	dbgprintf("pmsnare: msg to look at: [%d]'%s'\n", lenMsg, p2parse);
	if((unsigned) lenMsg < 30) {
		/* too short, can not be "our" message */
		dbgprintf("pmsnare: Message is too short to be Snare!\n");
		ABORT_FINALIZE(RS_RET_COULD_NOT_PARSE);
	}

	/* Find the first separator and check if it's a tab. */
	while(lenMsg && *p2parse != ' ' && *p2parse != '\t' && *p2parse != pInst->tabRepresentation[0]) {
		--lenMsg;
		++p2parse;
	}
	if ((lenMsg > pInst->tabLength) && (strncasecmp((char *)p2parse, pInst->tabRepresentation,
			pInst->tabLength) == 0)) {
		dbgprintf("pmsnare: tab separated message\n");
		dbgprintf("pmsnare: tab [%d]'%s'	msg at the first separator: [%d]'%s'\n",
			pInst->tabLength, pInst->tabRepresentation, lenMsg, p2parse);

		/* Look for the Snare tag. */
		if(strncasecmp((char*)(p2parse + pInst->tabLength), "MSWinEventLog", 13) == 0) {
			dbgprintf("Found a non-syslog Windows Snare message.\n");
			snaremessage = p2parse - pMsg->pszRawMsg + pInst->tabLength + 13;
		}
		else if(strncasecmp((char*) (p2parse + pInst->tabLength), "LinuxKAudit", 11) == 0) {
			dbgprintf("Found a non-syslog Linux Snare message.\n");
			snaremessage = p2parse - pMsg->pszRawMsg + pInst->tabLength + 11;
		} else {
			/* Tab-separated but no Snare tag-> can't be Snare! */
			ABORT_FINALIZE(RS_RET_COULD_NOT_PARSE);
		}

		/* This is a non-syslog Snare message. Example:
		 * other.lab.home	MSWinEventLog	1	Security	606129	Wed May 17 02:25:10 2017
		 */

		/* Remove the tab between the hostname and Snare tag. */
		*p2parse = ' ';
		p2parse++;
		lenMsg--;
		lenMsg -= (pInst->tabLength-1); /* size of tab goes from tabLength to 1, so shorten
						the message by the difference */
		memmove(p2parse, p2parse+(pInst->tabLength-1), lenMsg);
		/* move the message portion up to overwrite the tab */
		*(p2parse + lenMsg)	= '\0';
		pMsg->iLenRawMsg -= (pInst->tabLength-1);
		pMsg->iLenMSG -= (pInst->tabLength-1);
		snaremessage -= (pInst->tabLength-1);
	} else {
		/* The first separator is not a tab. Look for a syslog Snare message. Example:
		 * <14>May 17 02:25:10 syslog.lab.home MSWinEventLog     1    Security  606129
			Wed May 17 02:25:10 2017
		 */

		/* go back to the beginning of the message */
		lenMsg = pMsg->iLenRawMsg - pMsg->offAfterPRI;
		/* offAfterPRI is already the number of PRI chars (do not add one!) */
		p2parse = pMsg->pszRawMsg + pMsg->offAfterPRI;

		/* skip over timestamp and space (15 chars + space). */
		lenMsg -=16;
		p2parse +=16;
		/* skip over what should be the hostname and space */
		while(lenMsg && *p2parse != ' ') {
			--lenMsg;
			++p2parse;
		}
		if (lenMsg){
			--lenMsg;
			++p2parse;
		}
		dbgprintf("pmsnare: tab [%d]'%s'	msg after the timestamp and hostname: [%d]'%s'\n",
				pInst->tabLength,pInst->tabRepresentation,lenMsg, p2parse);

		/* Look for the Snare tag. */
		if(lenMsg > 13 && strncasecmp((char*) p2parse, "MSWinEventLog", 13) == 0) {
			dbgprintf("Found a syslog Windows Snare message.\n");
			snaremessage = p2parse - pMsg->pszRawMsg + 13;
		}
		else if(lenMsg > 11 && strncasecmp((char*) p2parse, "LinuxKAudit", 11) == 0) {
			dbgprintf("pmsnare: Found a syslog Linux Snare message.\n");
			snaremessage = p2parse - pMsg->pszRawMsg + 11;
		}
	}
	
	if(snaremessage) {
		/* Skip to the end of the tag. */
		p2parse = pMsg->pszRawMsg + snaremessage;
		lenMsg = pMsg->iLenRawMsg - snaremessage;

		/* Remove the tab after the tag. */
		*p2parse = ' ';
		p2parse++;
		lenMsg--;
		lenMsg -= (pInst->tabLength-1); /* size of tab goes from tabLength to 1, so shorten
						the message by the difference */
		memmove(p2parse, p2parse+(pInst->tabLength-1), lenMsg);
		/* move the message portion up to overwrite the tab */
		*(p2parse + lenMsg) = '\0';
		pMsg->iLenRawMsg -= (pInst->tabLength-1);
		pMsg->iLenMSG -= (pInst->tabLength-1);

		DBGPRINTF("pmsnare: new message: [%d]'%s'\n", lenMsg, pMsg->pszRawMsg + pMsg->offAfterPRI);
	}
	
	ABORT_FINALIZE(RS_RET_COULD_NOT_PARSE);

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
CODEqueryEtryPt_STD_MOD_QUERIES
CODEqueryEtryPt_STD_CONF2_QUERIES
CODEqueryEtryPt_STD_CONF2_setModCnf_QUERIES
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

	DBGPRINTF("snare parser init called, compiled with version %s\n", VERSION);
	bParseHOSTNAMEandTAG = glbl.GetParseHOSTNAMEandTAG();
	/* cache value, is set only during rsyslogd option processing */
ENDmodInit

/* vim:set ai:
 */
