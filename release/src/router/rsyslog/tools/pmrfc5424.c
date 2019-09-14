/* pmrfc5424.c
 * This is a parser module for RFC5424-formatted messages.
 *
 * NOTE: read comments in module-template.h to understand how this file
 *       works!
 *
 * File begun on 2009-11-03 by RGerhards
 *
 * Copyright 2007-2015 Rainer Gerhards and Adiscon GmbH.
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
#include "syslogd.h"
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

#ifdef _AIX
#endif
MODULE_TYPE_PARSER
MODULE_TYPE_NOKEEP
PARSER_NAME("rsyslog.rfc5424")

/* internal structures
 */
DEF_PMOD_STATIC_DATA
DEFobjCurrIf(glbl)
DEFobjCurrIf(parser)
DEFobjCurrIf(datetime)


/* config data */


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	if(eFeat == sFEATUREAutomaticSanitazion)
		iRet = RS_RET_OK;
	if(eFeat == sFEATUREAutomaticPRIParsing)
		iRet = RS_RET_OK;
ENDisCompatibleWithFeature


/* Helper to parseRFCSyslogMsg. This function parses a field up to
 * (and including) the SP character after it. The field contents is
 * returned in a caller-provided buffer. The parsepointer is advanced
 * to after the terminating SP. The caller must ensure that the
 * provided buffer is large enough to hold the to be extracted value.
 * Returns 0 if everything is fine or 1 if either the field is not
 * SP-terminated or any other error occurs. -- rger, 2005-11-24
 * The function now receives the size of the string and makes sure
 * that it does not process more than that. The *pLenStr counter is
 * updated on exit. -- rgerhards, 2009-09-23
 */
static int parseRFCField(uchar **pp2parse, uchar *pResult, int *pLenStr)
{
	uchar *p2parse;
	int iRet = 0;

	assert(pp2parse != NULL);
	assert(*pp2parse != NULL);
	assert(pResult != NULL);

	p2parse = *pp2parse;

	/* this is the actual parsing loop */
	while(*pLenStr > 0  && *p2parse != ' ') {
		*pResult++ = *p2parse++;
		--(*pLenStr);
	}

	if(*pLenStr > 0 && *p2parse == ' ') {
		++p2parse; /* eat SP, but only if not at end of string */
		--(*pLenStr);
	} else {
		iRet = 1; /* there MUST be an SP! */
	}
	*pResult = '\0';

	/* set the new parse pointer */
	*pp2parse = p2parse;
	return iRet;
}


/* Helper to parseRFCSyslogMsg. This function parses the structured
 * data field of a message. It does NOT parse inside structured data,
 * just gets the field as whole. Parsing the single entities is left
 * to other functions. The parsepointer is advanced
 * to after the terminating SP. The caller must ensure that the
 * provided buffer is large enough to hold the to be extracted value.
 * Returns 0 if everything is fine or 1 if either the field is not
 * SP-terminated or any other error occurs. -- rger, 2005-11-24
 * The function now receives the size of the string and makes sure
 * that it does not process more than that. The *pLenStr counter is
 * updated on exit. -- rgerhards, 2009-09-23
 */
static int parseRFCStructuredData(uchar **pp2parse, uchar *pResult, int *pLenStr)
{
	uchar *p2parse;
	int bCont = 1;
	int iRet = 0;
	int lenStr;

	assert(pp2parse != NULL);
	assert(*pp2parse != NULL);
	assert(pResult != NULL);

	p2parse = *pp2parse;
	lenStr = *pLenStr;

	/* this is the actual parsing loop
	 * Remeber: structured data starts with [ and includes any characters
	 * until the first ] followed by a SP. There may be spaces inside
	 * structured data. There may also be \] inside the structured data, which
	 * do NOT terminate an element.
	 */
	if(lenStr == 0 || (*p2parse != '[' && *p2parse != '-'))
		return 1; /* this is NOT structured data! */

	if(*p2parse == '-') { /* empty structured data? */
		*pResult++ = '-';
		++p2parse;
		--lenStr;
	} else {
		while(bCont) {
			if(lenStr < 2) {
				/* we now need to check if we have only structured data */
				if(lenStr > 0 && *p2parse == ']') {
					*pResult++ = *p2parse;
					p2parse++;
					lenStr--;
					bCont = 0;
				} else {
					iRet = 1; /* this is not valid! */
					bCont = 0;
				}
			} else if(*p2parse == '\\' && *(p2parse+1) == ']') {
				/* this is escaped, need to copy both */
				*pResult++ = *p2parse++;
				*pResult++ = *p2parse++;
				lenStr -= 2;
			} else if(*p2parse == ']' && *(p2parse+1) == ' ') {
				/* found end, just need to copy the ] and eat the SP */
				*pResult++ = *p2parse;
				p2parse += 2;
				lenStr -= 2;
				bCont = 0;
			} else {
				*pResult++ = *p2parse++;
				--lenStr;
			}
		}
	}

	if(lenStr > 0 && *p2parse == ' ') {
		++p2parse; /* eat SP, but only if not at end of string */
		--lenStr;
	} else {
		iRet = 1; /* there MUST be an SP! */
	}
	*pResult = '\0';

	/* set the new parse pointer */
	*pp2parse = p2parse;
	*pLenStr = lenStr;
	return iRet;
}

/* parse a RFC5424-formatted syslog message. This function returns
 * 0 if processing of the message shall continue and 1 if something
 * went wrong and this messe should be ignored. This function has been
 * implemented in the effort to support syslog-protocol. Please note that
 * the name (parse *RFC*) stems from the hope that syslog-protocol will
 * some time become an RFC. Do not confuse this with informational
 * RFC 3164 (which is legacy syslog).
 *
 * currently supported format:
 *
 * <PRI>VERSION SP TIMESTAMP SP HOSTNAME SP APP-NAME SP PROCID SP MSGID SP [SD-ID]s SP MSG
 *
 * <PRI> is already stripped when this function is entered. VERSION already
 * has been confirmed to be "1", but has NOT been stripped from the message.
 *
 * rger, 2005-11-24
 */
BEGINparse
	uchar *p2parse;
	uchar *pBuf = NULL;
	int lenMsg;
	int bContParse = 1;
CODESTARTparse
	assert(pMsg != NULL);
	assert(pMsg->pszRawMsg != NULL);
	p2parse = pMsg->pszRawMsg + pMsg->offAfterPRI; /* point to start of text, after PRI */
	lenMsg = pMsg->iLenRawMsg - pMsg->offAfterPRI;

	/* check if we are the right parser */
	if(lenMsg < 2 || p2parse[0] != '1' || p2parse[1] != ' ') {
		ABORT_FINALIZE(RS_RET_COULD_NOT_PARSE);
	}
	DBGPRINTF("Message has RFC5424/syslog-protocol format.\n");
	setProtocolVersion(pMsg, MSG_RFC5424_PROTOCOL);
	p2parse += 2;
	lenMsg -= 2;

	/* Now get us some memory we can use as a work buffer while parsing.
	 * We simply allocated a buffer sufficiently large to hold all of the
	 * message, so we can not run into any troubles. I think this is
	 * wiser than to use individual buffers.
	 */
	CHKmalloc(pBuf = MALLOC(lenMsg + 1));

	/* IMPORTANT NOTE:
	 * Validation is not actually done below nor are any errors handled. I have
	 * NOT included this for the current proof of concept. However, it is strongly
	 * advisable to add it when this code actually goes into production.
	 * rgerhards, 2005-11-24
	 */

	/* TIMESTAMP */
	if(lenMsg >= 2 && p2parse[0] == '-' && p2parse[1] == ' ') {
		memcpy(&pMsg->tTIMESTAMP, &pMsg->tRcvdAt, sizeof(struct syslogTime));
		p2parse += 2;
		lenMsg -= 2;
	} else if(datetime.ParseTIMESTAMP3339(&(pMsg->tTIMESTAMP),  &p2parse, &lenMsg) == RS_RET_OK) {
		if(pMsg->msgFlags & IGNDATE) {
			/* we need to ignore the msg data, so simply copy over reception date */
			memcpy(&pMsg->tTIMESTAMP, &pMsg->tRcvdAt, sizeof(struct syslogTime));
		}
	} else {
		DBGPRINTF("no TIMESTAMP detected!\n");
		bContParse = 0;
	}

	/* HOSTNAME */
	if(bContParse) {
		parseRFCField(&p2parse, pBuf, &lenMsg);
		MsgSetHOSTNAME(pMsg, pBuf, ustrlen(pBuf));
	}

	/* APP-NAME */
	if(bContParse) {
		parseRFCField(&p2parse, pBuf, &lenMsg);
		MsgSetAPPNAME(pMsg, (char*)pBuf);
	}

	/* PROCID */
	if(bContParse) {
		parseRFCField(&p2parse, pBuf, &lenMsg);
		MsgSetPROCID(pMsg, (char*)pBuf);
	}

	/* MSGID */
	if(bContParse) {
		parseRFCField(&p2parse, pBuf, &lenMsg);
		MsgSetMSGID(pMsg, (char*)pBuf);
	}

	/* STRUCTURED-DATA */
	if(bContParse) {
		parseRFCStructuredData(&p2parse, pBuf, &lenMsg);
		MsgSetStructuredData(pMsg, (char*)pBuf);
	}

	/* MSG */
	MsgSetMSGoffs(pMsg, p2parse - pMsg->pszRawMsg);

finalize_it:
	if(pBuf != NULL)
		free(pBuf);
ENDparse


BEGINmodExit
CODESTARTmodExit
	/* release what we no longer need */
	objRelease(glbl, CORE_COMPONENT);
	objRelease(parser, CORE_COMPONENT);
	objRelease(datetime, CORE_COMPONENT);
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_PMOD_QUERIES
CODEqueryEtryPt_IsCompatibleWithFeature_IF_OMOD_QUERIES
ENDqueryEtryPt


BEGINmodInit(pmrfc5424)
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(parser, CORE_COMPONENT));
	CHKiRet(objUse(datetime, CORE_COMPONENT));

	dbgprintf("rfc5424 parser init called\n");
	dbgprintf("GetParserName addr %p\n", GetParserName);
ENDmodInit

/* vim:set ai:
 */
