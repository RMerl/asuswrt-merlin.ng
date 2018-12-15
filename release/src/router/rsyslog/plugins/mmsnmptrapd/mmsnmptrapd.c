/* mmsnmptrapd.c
 * This is a message modification module. It takes messages generated
 * from snmptrapd and modifies them so that the look like they
 * originated from the real originator.
 *
 * NOTE: read comments in module-template.h for details on the calling interface!
 *
 * File begun on 2011-05-05 by RGerhards
 *
 * Copyright 2011-2017 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of rsyslog.
 *
 * Rsyslog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rsyslog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rsyslog.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
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
#include "conf.h"
#include "msg.h"
#include "syslogd-types.h"
#include "template.h"
#include "module-template.h"
#include "errmsg.h"
#include "cfsysline.h"
#include "unicode-helper.h"
#include "dirty.h"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("mmsnmptrapd")

static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal);

/* static data */

/* internal structures
 */
DEF_OMOD_STATIC_DATA

struct severMap_s {
	uchar *name;
	int code;
	struct severMap_s *next;
};

typedef struct _instanceData {
	uchar *pszTagName;
	uchar *pszTagID;	/* cached: name plus trailing shlash (for compares) */
	int lenTagID;		/* cached: length of tag ID, for performance reasons */
	struct severMap_s *severMap;
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
} wrkrInstanceData_t;

typedef struct configSettings_s {
	uchar *pszTagName;	/**< name of tag start value that indicates snmptrapd initiated message */
	uchar *pszSeverityMapping; /**< severitystring to numerical code mapping for snmptrapd string */
} configSettings_t;
static configSettings_t cs;

BEGINinitConfVars		/* (re)set config variables to default values */
CODESTARTinitConfVars
	cs.pszTagName = NULL;
	cs.pszSeverityMapping = NULL;
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


BEGINfreeInstance
	struct severMap_s *node, *nodeDel;
CODESTARTfreeInstance
	for(node = pData->severMap ; node != NULL ; ) {
		nodeDel = node;
		node = node->next;
		free(nodeDel->name);
		free(nodeDel);
	}
	free(pData->pszTagName);
	free(pData->pszTagID);
ENDfreeInstance

BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance


BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
	dbgprintf("mmsnmptrapd\n");
ENDdbgPrintInstInfo


BEGINtryResume
CODESTARTtryResume
ENDtryResume


/* check if a string is numeric (int) */
static int
isNumeric(uchar *str)
{
	int r = 1;
	if(*str == '-' || *str == '+')
		++str;
	while(*str) {
		if(!isdigit(*str)) {
			r = 0;
			goto done;
		}
		++str;
	}
done:
	return r;
}

/* get a substring delimited by a character (or end of string). The
 * string is trimmed, that is leading and trailing spaces are removed.
 * The caller must provide a buffer which shall receive the substring.
 * String length is returned as result. The input string is updated
 * on exit, so that it may be used for another query starting at that
 * position.
 */
static int
getSubstring(uchar **psrc, uchar delim, uchar *dst, int lenDst)
{
	uchar *dstwrk = dst;
	uchar *src = *psrc;
	while(*src && isspace(*src)) {
		++src;	/* trim leading spaces */
	}
	while(*src && *src != delim && --lenDst > 0) {
		*dstwrk++ = *src++;
	}
	dstwrk--;
	while(dstwrk > dst && isspace(*dst))
		--dstwrk; /* trim trailing spaces */
	*++dstwrk = '\0';
	
	/* final results */
	if(*src == delim)
		++src;
	*psrc = src;
	return(dstwrk - dst);
}


/* get string up to the next SP or '/'. Stops at max size.
 * dst, lenDst (receive buffer) must be given. lenDst is
 * max length on entry and actual length on exit.
 */
static int ATTR_NONNULL()
getTagComponent(uchar *tag, uchar *const dst, int *const lenDst)
{
	int end = *lenDst - 1; /* -1 for NUL-char! */
	int i;

	i = 0;
	if(tag[i] == '/') {
		++tag;
		while(i < end && tag[i] != '\0' && tag[i] != ' ' && tag[i] != '/') {
			dst[i] = tag[i];
			++i;
		}
	}
	dst[i] = '\0';
	*lenDst = i;
	return i;
}


/* lookup severity code based on provided severity
 * returns -1 if severity could not be found.
 */
static int
lookupSeverityCode(instanceData *pData, uchar *sever)
{
	struct severMap_s *node;
	int sevCode = -1;

	for(node = pData->severMap ; node != NULL ; node = node->next) {
		if(!ustrcmp(node->name, sever)) {
			sevCode = node->code;
			break;
		}
	}
	return sevCode;
}


BEGINdoAction_NoStrings
	smsg_t **ppMsg = (smsg_t **) pMsgData;
	smsg_t *pMsg = ppMsg[0];
	int lenTAG;
	int lenSever;
	int lenHost;
	int sevCode;
	uchar *pszTag;
	uchar pszSever[512];
	uchar pszHost[512];
	instanceData *pData;
CODESTARTdoAction
	pData = pWrkrData->pData;
	getTAG(pMsg, &pszTag, &lenTAG);
	if(strncmp((char*)pszTag, (char*)pData->pszTagID, pData->lenTagID)) {
		DBGPRINTF("tag '%s' not matching, mmsnmptrapd ignoring this message\n",
			  pszTag);
		FINALIZE;
	}

	lenSever = sizeof(pszSever);
	getTagComponent(pszTag+pData->lenTagID-1, pszSever, &lenSever);
	lenHost = sizeof(pszHost);
	getTagComponent(pszTag+pData->lenTagID+lenSever, pszHost, &lenHost);
	DBGPRINTF("mmsnmptrapd: sever '%s'(%d), host '%s'(%d)\n", pszSever, lenSever, pszHost,lenHost);

	if(lenHost > 0 && pszHost[lenHost-1] == ':') {
		pszHost[lenHost-1] = '\0';
		--lenHost;
	}
	sevCode = lookupSeverityCode(pData, pszSever);
	/* now apply new settings */
	MsgSetTAG(pMsg, pData->pszTagName, pData->lenTagID);
	MsgSetHOSTNAME(pMsg, pszHost, lenHost);
	if(sevCode != -1)
		pMsg->iSeverity = sevCode; /* we update like the parser does! */
finalize_it:
ENDdoAction


/* Build the severity mapping table based on user-provided configuration
 * settings.
 */
static rsRetVal ATTR_NONNULL()
buildSeverityMapping(instanceData *const pData)
{
	uchar pszSev[512];
	uchar pszSevCode[512];
	int sevCode;
	uchar *mapping;
	struct severMap_s *node = NULL;
	DEFiRet;

	mapping = cs.pszSeverityMapping;

	while(1) {	/* broken inside when all entries are processed */
		if(getSubstring(&mapping, '/', pszSev, sizeof(pszSev)) == 0) {
			FINALIZE;
		}
		if(getSubstring(&mapping, ',', pszSevCode, sizeof(pszSevCode)) == 0) {
			LogError(0, RS_RET_ERR, "error: invalid severity mapping, cannot "
					"extract code. given: '%s'\n", cs.pszSeverityMapping);
			ABORT_FINALIZE(RS_RET_ERR);
		}
		sevCode = atoi((char*) pszSevCode);
		if(!isNumeric(pszSevCode))
			sevCode = -1;
		if(sevCode < 0 || sevCode > 7) {
			LogError(0, RS_RET_ERR, "error: severity code %d outside of valid "
					"range 0..7 (was string '%s')\n", sevCode, pszSevCode);
			ABORT_FINALIZE(RS_RET_ERR);
		}
		CHKmalloc(node = MALLOC(sizeof(struct severMap_s)));
		CHKmalloc(node->name = ustrdup(pszSev));
		node->code = sevCode;
		/* we enqueue at the top, so the two lines below do all we need! */
		node->next = pData->severMap;
		pData->severMap = node;
		node = NULL;
		DBGPRINTF("mmsnmptrapd: severity string '%s' mapped to code %d\n",
			  pszSev, sevCode);
	}

finalize_it:
	if(iRet != RS_RET_OK) {
		free(node);
	}
	RETiRet;
}


BEGINparseSelectorAct
CODESTARTparseSelectorAct
CODE_STD_STRING_REQUESTparseSelectorAct(1)
	/* first check if this config line is actually for us */
	if(strncmp((char*) p, ":mmsnmptrapd:", sizeof(":mmsnmptrapd:") - 1)) {
		ABORT_FINALIZE(RS_RET_CONFLINE_UNPROCESSED);
	}

	/* ok, if we reach this point, we have something for us */
	p += sizeof(":mmsnmptrapd:") - 1; /* eat indicator sequence  (-1 because of '\0'!) */
	CHKiRet(createInstance(&pData));

	/* check if a non-standard template is to be applied */
	if(*(p-1) == ';')
		--p;
	/* we call the function below because we need to call it via our interface definition. However,
	 * the format specified (if any) is always ignored.
	 */
	CHKiRet(cflineParseTemplateName(&p, *ppOMSR, 0, OMSR_TPL_AS_MSG, (uchar*) "RSYSLOG_FileFormat"));

	/* finally build the instance */
	if(cs.pszTagName == NULL) {
		CHKmalloc(pData->pszTagName = (uchar*) strdup("snmptrapd:"));
		CHKmalloc(pData->pszTagID = (uchar*) strdup("snmptrapd/"));
	} else {
		int lenTag = ustrlen(cs.pszTagName);
		/* new tag value (with colon at the end) */
		CHKmalloc(pData->pszTagName = MALLOC(lenTag + 2));
		memcpy(pData->pszTagName, cs.pszTagName, lenTag);
		memcpy(pData->pszTagName+lenTag, ":", 2);
		/* tag ID for comparisions */
		CHKmalloc(pData->pszTagID = MALLOC(lenTag + 2));
		memcpy(pData->pszTagID, cs.pszTagName, lenTag);
		memcpy(pData->pszTagID+lenTag, "/", 2);
		free(cs.pszTagName); /* no longer needed */
	}
	pData->lenTagID = ustrlen(pData->pszTagID);
	if(cs.pszSeverityMapping != NULL) {
		CHKiRet(buildSeverityMapping(pData));
	}

	/* all config vars auto-reset! */
	cs.pszTagName = NULL;
	free(cs.pszSeverityMapping);
	cs.pszSeverityMapping = NULL;
CODE_STD_FINALIZERparseSelectorAct
ENDparseSelectorAct


BEGINmodExit
CODESTARTmodExit
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
	free(cs.pszTagName);
	cs.pszTagName = NULL;
	free(cs.pszSeverityMapping);
	cs.pszSeverityMapping = NULL;
	RETiRet;
}


BEGINmodInit()
	rsRetVal localRet;
	rsRetVal (*pomsrGetSupportedTplOpts)(unsigned long *pOpts);
	unsigned long opts;
	int bMsgPassingSupported;
CODESTARTmodInit
INITLegCnfVars
	*ipIFVersProvided = CURR_MOD_IF_VERSION;
		/* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
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
		DBGPRINTF("mmsnmptrapd: msg-passing is not supported by rsyslog core, "
			  "can not continue.\n");
		ABORT_FINALIZE(RS_RET_NO_MSG_PASSING);
	}


	/* TODO: config vars ininit can be replaced by commented-out code above in v6 */
	cs.pszTagName = NULL;
	cs.pszSeverityMapping = NULL;
	
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"mmsnmptrapdtag", 0, eCmdHdlrGetWord,
				    NULL, &cs.pszTagName, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"mmsnmptrapdseveritymapping", 0, eCmdHdlrGetWord,
				    NULL, &cs.pszSeverityMapping, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler,
				    resetConfigVariables, NULL, STD_LOADABLE_MODULE_ID));
ENDmodInit

/* vi:set ai:
 */
