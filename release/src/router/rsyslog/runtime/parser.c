/* parser.c
 * This module contains functions for message parsers. It still needs to be
 * converted into an object (and much extended).
 *
 * Module begun 2008-10-09 by Rainer Gerhards (based on previous code from syslogd.c)
 *
 * Copyright 2008-2016 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of the rsyslog runtime library.
 *
 * The rsyslog runtime library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The rsyslog runtime library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the rsyslog runtime library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 * A copy of the LGPL can be found in the file "COPYING.LESSER" in this distribution.
 */
#include "config.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <zlib.h>

#include "rsyslog.h"
#include "dirty.h"
#include "msg.h"
#include "obj.h"
#include "datetime.h"
#include "errmsg.h"
#include "parser.h"
#include "ruleset.h"
#include "unicode-helper.h"
#include "dirty.h"
#include "cfsysline.h"

/* some defines */
#define DEFUPRI		(LOG_USER|LOG_NOTICE)

/* definitions for objects we access */
DEFobjStaticHelpers
DEFobjCurrIf(glbl)
DEFobjCurrIf(datetime)
DEFobjCurrIf(ruleset)

/* static data */

static char hexdigit[16] =
	{'0', '1', '2', '3', '4', '5', '6', '7', '8',
	 '9', 'A', 'B', 'C', 'D', 'E', 'F' };

/* This is the list of all parsers known to us.
 * This is also used to unload all modules on shutdown.
 */
parserList_t *pParsLstRoot = NULL;

/* this is the list of the default parsers, to be used if no others
 * are specified.
 */
parserList_t *pDfltParsLst = NULL;


/* intialize (but NOT allocate) a parser list. Primarily meant as a hook
 * which can be used to extend the list in the future. So far, just sets
 * it to NULL.
 */
static rsRetVal
InitParserList(parserList_t **pListRoot)
{
	*pListRoot = NULL;
	return RS_RET_OK;
}


/* destruct a parser list. The list elements are destroyed, but the parser objects
 * themselves are not modified. (That is done at a late stage during rsyslogd
 * shutdown and need not be considered here.)
 */
static rsRetVal
DestructParserList(parserList_t **ppListRoot)
{
	parserList_t *pParsLst;
	parserList_t *pParsLstDel;

	pParsLst = *ppListRoot;
	while(pParsLst != NULL) {
		pParsLstDel = pParsLst;
		pParsLst = pParsLst->pNext;
		free(pParsLstDel);
	}
	*ppListRoot = NULL;
	return RS_RET_OK;
}


/* Add a parser to the list. We use a VERY simple and ineffcient algorithm,
 * but it is employed only for a few milliseconds during config processing. So
 * I prefer to keep it very simple and with simple data structures. Unfortunately,
 * we need to preserve the order, but I don't like to add a tail pointer as that
 * would require a container object. So I do the extra work to skip to the tail
 * when adding elements...
 * rgerhards, 2009-11-03
 */
static rsRetVal
AddParserToList(parserList_t **ppListRoot, parser_t *pParser)
{
	parserList_t *pThis;
	parserList_t *pTail;
	DEFiRet;

	CHKmalloc(pThis = MALLOC(sizeof(parserList_t)));
	pThis->pParser = pParser;
	pThis->pNext = NULL;

	if(*ppListRoot == NULL) {
		pThis->pNext = *ppListRoot;
		*ppListRoot = pThis;
	} else {
		/* find tail first */
		for(pTail = *ppListRoot ; pTail->pNext != NULL ; pTail = pTail->pNext)
			/* just search, do nothing else */;
		/* add at tail */
		pTail->pNext = pThis;
	}
DBGPRINTF("DDDDD: added parser '%s' to list %p\n", pParser->pName, ppListRoot);
finalize_it:
	RETiRet;
}

void
printParserList(parserList_t *pList)
{
	while(pList != NULL) {
		dbgprintf("parser: %s\n", pList->pParser->pName);
		pList = pList->pNext;
	}
}

/* find a parser based on the provided name */
static rsRetVal
FindParser(parser_t **ppParser, uchar *pName)
{
	parserList_t *pThis;
	DEFiRet;
	
	for(pThis = pParsLstRoot ; pThis != NULL ; pThis = pThis->pNext) {
		if(ustrcmp(pThis->pParser->pName, pName) == 0) {
			*ppParser = pThis->pParser;
			FINALIZE;	/* found it, iRet still eq. OK! */
		}
	}

	iRet = RS_RET_PARSER_NOT_FOUND;

finalize_it:
	RETiRet;
}


/* --- END helper functions for parser list handling --- */

/* Add a an already existing parser to the default list. As usual, order
 * of calls is important (most importantly, that means the legacy parser,
 * which can process everything, MUST be added last!).
 * rgerhards, 2009-11-04
 */
static rsRetVal
AddDfltParser(uchar *pName)
{
	parser_t *pParser;
	DEFiRet;

	CHKiRet(FindParser(&pParser, pName));
	CHKiRet(AddParserToList(&pDfltParsLst, pParser));
	DBGPRINTF("Parser '%s' added to default parser set.\n", pName);
	
finalize_it:
	RETiRet;
}


/* set the parser name - string is copied over, call can continue to use it,
 * but must free it if desired.
 */
static rsRetVal
SetName(parser_t *pThis, uchar *name)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, parser);
	assert(name != NULL);

	if(pThis->pName != NULL) {
		free(pThis->pName);
		pThis->pName = NULL;
	}

	CHKmalloc(pThis->pName = ustrdup(name));

finalize_it:
	RETiRet;
}


/* set a pointer to "our" module. Note that no module
 * pointer must already be set.
 */
static rsRetVal
SetModPtr(parser_t *pThis, modInfo_t *pMod)
{
	ISOBJ_TYPE_assert(pThis, parser);
	assert(pMod != NULL);
	assert(pThis->pModule == NULL);
	pThis->pModule = pMod;
	return RS_RET_OK;
}


/* Specify if we should do standard PRI parsing before we pass the data
 * down to the parser module.
 */
static rsRetVal
SetDoPRIParsing(parser_t *pThis, int bDoIt)
{
	ISOBJ_TYPE_assert(pThis, parser);
	pThis->bDoPRIParsing = bDoIt;
	return RS_RET_OK;
}




BEGINobjConstruct(parser) /* be sure to specify the object type also in END macro! */
ENDobjConstruct(parser)

/* ConstructionFinalizer. The most important chore is to add the parser object
 * to our global list of available parsers.
 * rgerhards, 2009-11-03
 */
static rsRetVal parserConstructFinalize(parser_t *pThis)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, parser);
	CHKiRet(AddParserToList(&pParsLstRoot, pThis));
	DBGPRINTF("Parser '%s' added to list of available parsers.\n", pThis->pName);

finalize_it:
	RETiRet;
}


/* construct a parser object via a pointer to the parser module
 * and the name. This is a separate function because we need it
 * in multiple spots inside the code.
 */
rsRetVal
parserConstructViaModAndName(modInfo_t *__restrict__ pMod, uchar *const __restrict__ pName, void *pInst)
{
	rsRetVal localRet;
	parser_t *pParser = NULL;
	DEFiRet;

	if(pInst == NULL && pMod->mod.pm.newParserInst != NULL) {
		/* this happens for the default instance on ModLoad time */
		CHKiRet(pMod->mod.pm.newParserInst(NULL, &pInst));
	}
	CHKiRet(parserConstruct(&pParser));
	/* check some features */
	localRet = pMod->isCompatibleWithFeature(sFEATUREAutomaticSanitazion);
	if(localRet == RS_RET_OK){
		pParser->bDoSanitazion = RSTRUE;
	}
	localRet = pMod->isCompatibleWithFeature(sFEATUREAutomaticPRIParsing);
	if(localRet == RS_RET_OK){
		CHKiRet(SetDoPRIParsing(pParser, RSTRUE));
	}

	CHKiRet(SetName(pParser, pName));
	CHKiRet(SetModPtr(pParser, pMod));
	pParser->pInst = pInst;
	CHKiRet(parserConstructFinalize(pParser));
finalize_it:
	if(iRet != RS_RET_OK)
		free(pParser);
	RETiRet;
}
BEGINobjDestruct(parser) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDestruct(parser)
	DBGPRINTF("destructing parser '%s'\n", pThis->pName);
	if(pThis->pInst != NULL) {
		pThis->pModule->mod.pm.freeParserInst(pThis->pInst);
	}
	free(pThis->pName);
ENDobjDestruct(parser)


/* uncompress a received message if it is compressed.
 * pMsg->pszRawMsg buffer is updated.
 * rgerhards, 2008-10-09
 */
static rsRetVal uncompressMessage(smsg_t *pMsg)
{
	DEFiRet;
	uchar *deflateBuf = NULL;
	uLongf iLenDefBuf;
	uchar *pszMsg;
	size_t lenMsg;
	
	assert(pMsg != NULL);
	pszMsg = pMsg->pszRawMsg;
	lenMsg = pMsg->iLenRawMsg;

	/* we first need to check if we have a compressed record. If so,
	 * we must decompress it.
	 */
	if(lenMsg > 0 && *pszMsg == 'z') { /* compressed data present? (do NOT change order if conditions!) */
		/* we have compressed data, so let's deflate it. We support a maximum
		 * message size of iMaxLine. If it is larger, an error message is logged
		 * and the message is dropped. We do NOT try to decompress larger messages
		 * as such might be used for denial of service. It might happen to later
		 * builds that such functionality be added as an optional, operator-configurable
		 * feature.
		 */
		int ret;
		iLenDefBuf = glbl.GetMaxLine();
		CHKmalloc(deflateBuf = MALLOC(iLenDefBuf + 1));
		ret = uncompress((uchar *) deflateBuf, &iLenDefBuf, (uchar *) pszMsg+1, lenMsg-1);
		DBGPRINTF("Compressed message uncompressed with status %d, length: new %ld, old %d.\n",
		        ret, (long) iLenDefBuf, (int) (lenMsg-1));
		/* Now check if the uncompression worked. If not, there is not much we can do. In
		 * that case, we log an error message but ignore the message itself. Storing the
		 * compressed text is dangerous, as it contains control characters. So we do
		 * not do this. If someone would like to have a copy, this code here could be
		 * modified to do a hex-dump of the buffer in question. We do not include
		 * this functionality right now.
		 * rgerhards, 2006-12-07
		 */
		if(ret != Z_OK) {
			LogError(0, NO_ERRCODE, "Uncompression of a message failed with return code %d "
			            "- enable debug logging if you need further information. "
				    "Message ignored.", ret);
			FINALIZE; /* unconditional exit, nothing left to do... */
		}
		MsgSetRawMsg(pMsg, (char*)deflateBuf, iLenDefBuf);
	}
finalize_it:
	if(deflateBuf != NULL)
		free(deflateBuf);

	RETiRet;
}


/* sanitize a received message
 * if a message gets to large during sanitization, it is truncated. This is
 * as specified in the upcoming syslog RFC series.
 * rgerhards, 2008-10-09
 * We check if we have a NUL character at the very end of the
 * message. This seems to be a frequent problem with a number of senders.
 * So I have now decided to drop these NULs. However, if they are intentional,
 * that may cause us some problems, e.g. with syslog-sign. On the other hand,
 * current code always has problems with intentional NULs (as it needs to escape
 * them to prevent problems with the C string libraries), so that does not
 * really matter. Just to be on the save side, we'll log destruction of such
 * NULs in the debug log.
 * rgerhards, 2007-09-14
 */
static rsRetVal
SanitizeMsg(smsg_t *pMsg)
{
	DEFiRet;
	uchar *pszMsg;
	uchar *pDst; /* destination for copy job */
	size_t lenMsg;
	size_t iSrc;
	size_t iDst;
	size_t iMaxLine;
	size_t maxDest;
	uchar pc;
	sbool bUpdatedLen = RSFALSE;
	uchar szSanBuf[32*1024]; /* buffer used for sanitizing a string */

	assert(pMsg != NULL);
	assert(pMsg->iLenRawMsg > 0);

	pszMsg = pMsg->pszRawMsg;
	lenMsg = pMsg->iLenRawMsg;

	/* remove NUL character at end of message (see comment in function header)
	 * Note that we do not need to add a NUL character in this case, because it
	 * is already present ;)
	 */
	if(pszMsg[lenMsg-1] == '\0') {
		DBGPRINTF("dropped NUL at very end of message\n");
		bUpdatedLen = RSTRUE;
		lenMsg--;
	}

	/* then we check if we need to drop trailing LFs, which often make
	 * their way into syslog messages unintentionally. In order to remain
	 * compatible to recent IETF developments, we allow the user to
	 * turn on/off this handling.  rgerhards, 2007-07-23
	 */
	if(glbl.GetParserDropTrailingLFOnReception()
	   && lenMsg > 0 && pszMsg[lenMsg-1] == '\n') {
		DBGPRINTF("dropped LF at very end of message (DropTrailingLF is set)\n");
		lenMsg--;
		pszMsg[lenMsg] = '\0';
		bUpdatedLen = RSTRUE;
	}

	/* it is much quicker to sweep over the message and see if it actually
	 * needs sanitation than to do the sanitation in any case. So we first do
	 * this and terminate when it is not needed - which is expectedly the case
	 * for the vast majority of messages. -- rgerhards, 2009-06-15
	 * Note that we do NOT check here if tab characters are to be escaped or
	 * not. I expect this functionality to be seldomly used and thus I do not
	 * like to pay the performance penalty. So the penalty is only with those
	 * that actually use it, because we may call the sanitizer without actual
	 * need below (but it then still will work perfectly well!). -- rgerhards, 2009-11-27
	 */
	int bNeedSanitize = 0;
	for(iSrc = 0 ; iSrc < lenMsg ; iSrc++) {
		if(pszMsg[iSrc] < 32) {
			if(glbl.GetParserSpaceLFOnReceive() && pszMsg[iSrc] == '\n') {
				pszMsg[iSrc] = ' ';
			} else if(pszMsg[iSrc] == '\0' || glbl.GetParserEscapeControlCharactersOnReceive()) {
				bNeedSanitize = 1;
				if (!glbl.GetParserSpaceLFOnReceive()) {
					break;
			    }
			}
		} else if(pszMsg[iSrc] > 127 && glbl.GetParserEscape8BitCharactersOnReceive()) {
			bNeedSanitize = 1;
			break;
		}
	}

	if(!bNeedSanitize) {
		if(bUpdatedLen == RSTRUE)
			MsgSetRawMsgSize(pMsg, lenMsg);
		FINALIZE;
	}

	/* now copy over the message and sanitize it. Note that up to iSrc-1 there was
	 * obviously no need to sanitize, so we can go over that quickly...
	 */
	iMaxLine = glbl.GetMaxLine();
	maxDest = lenMsg * 4; /* message can grow at most four-fold */

	if(maxDest > iMaxLine)
		maxDest = iMaxLine;	/* but not more than the max size! */
	if(maxDest < sizeof(szSanBuf))
		pDst = szSanBuf;
	else
		CHKmalloc(pDst = MALLOC(maxDest + 1));
	if(iSrc > 0) {
		iSrc--; /* go back to where everything is OK */
		if(iSrc > maxDest) {
			DBGPRINTF("parser.Sanitize: have oversize index %zd, "
				"max %zd - corrected, but should not happen\n",
				iSrc, maxDest);
			iSrc = maxDest;
		}
		memcpy(pDst, pszMsg, iSrc); /* fast copy known good */
	}
	iDst = iSrc;
	while(iSrc < lenMsg && iDst < maxDest - 3) { /* leave some space if last char must be escaped */
		if((pszMsg[iSrc] < 32) && (pszMsg[iSrc] != '\t' || glbl.GetParserEscapeControlCharacterTab())) {
			/* note: \0 must always be escaped, the rest of the code currently
			 * can not handle it! -- rgerhards, 2009-08-26
			 */
			if(pszMsg[iSrc] == '\0' || glbl.GetParserEscapeControlCharactersOnReceive()) {
				/* we are configured to escape control characters. Please note
				 * that this most probably break non-western character sets like
				 * Japanese, Korean or Chinese. rgerhards, 2007-07-17
				 */
				if (glbl.GetParserEscapeControlCharactersCStyle()) {
					pDst[iDst++] = '\\';

					switch (pszMsg[iSrc]) {
					case '\0':
						pDst[iDst++] = '0';
						break;
					case '\a':
						pDst[iDst++] = 'a';
						break;
					case '\b':
						pDst[iDst++] = 'b';
						break;
					case '\e':
						pDst[iDst++] = 'e';
						break;
					case '\f':
						pDst[iDst++] = 'f';
						break;
					case '\n':
						pDst[iDst++] = 'n';
						break;
					case '\r':
						pDst[iDst++] = 'r';
						break;
					case '\t':
						pDst[iDst++] = 't';
						break;
					case '\v':
						pDst[iDst++] = 'v';
						break;
					default:
						pDst[iDst++] = 'x';

						pc = pszMsg[iSrc];
						pDst[iDst++] = hexdigit[(pc & 0xF0) >> 4];
						pDst[iDst++] = hexdigit[pc & 0xF];

						break;
					}

				} else {
					pDst[iDst++] = glbl.GetParserControlCharacterEscapePrefix();
					pDst[iDst++] = '0' + ((pszMsg[iSrc] & 0300) >> 6);
					pDst[iDst++] = '0' + ((pszMsg[iSrc] & 0070) >> 3);
					pDst[iDst++] = '0' + ((pszMsg[iSrc] & 0007));
				}
			}

		} else if(pszMsg[iSrc] > 127 && glbl.GetParserEscape8BitCharactersOnReceive()) {
			if (glbl.GetParserEscapeControlCharactersCStyle()) {
				pDst[iDst++] = '\\';
				pDst[iDst++] = 'x';

				pc = pszMsg[iSrc];
				pDst[iDst++] = hexdigit[(pc & 0xF0) >> 4];
				pDst[iDst++] = hexdigit[pc & 0xF];

			} else {
				/* In this case, we also do the conversion. Note that this most
				 * probably breaks European languages. -- rgerhards, 2010-01-27
				 */
				pDst[iDst++] = glbl.GetParserControlCharacterEscapePrefix();
				pDst[iDst++] = '0' + ((pszMsg[iSrc] & 0300) >> 6);
				pDst[iDst++] = '0' + ((pszMsg[iSrc] & 0070) >> 3);
				pDst[iDst++] = '0' + ((pszMsg[iSrc] & 0007));
			}
		} else {
			pDst[iDst++] = pszMsg[iSrc];
		}
		++iSrc;
	}
	pDst[iDst] = '\0';

	MsgSetRawMsg(pMsg, (char*)pDst, iDst); /* save sanitized string */

	if(pDst != szSanBuf)
		free(pDst);

finalize_it:
	RETiRet;
}

/* A standard parser to parse out the PRI. This is made available in
 * this module as it is expected that allmost all parsers will need
 * that functionality and so they do not need to implement it themsleves.
 */
static rsRetVal
ParsePRI(smsg_t *pMsg)
{
	syslog_pri_t pri;
	uchar *msg;
	int lenMsg;
	DEFiRet;

	/* pull PRI */
	lenMsg = pMsg->iLenRawMsg;
	msg = pMsg->pszRawMsg;
	pri = DEFUPRI;
	if(pMsg->msgFlags & NO_PRI_IN_RAW) {
		/* In this case, simply do so as if the pri would be right at top */
		MsgSetAfterPRIOffs(pMsg, 0);
	} else {
		if(*msg == '<') {
			pri = 0;
			while(--lenMsg > 0 && isdigit((int) *++msg) && pri <= LOG_MAXPRI) {
				pri = 10 * pri + (*msg - '0');
			}
			if(*msg == '>') {
				++msg;
			} else {
				pri = LOG_PRI_INVLD;
			}
			if(pri > LOG_MAXPRI)
				pri = LOG_PRI_INVLD;
		}
		msgSetPRI(pMsg, pri);
		MsgSetAfterPRIOffs(pMsg, (pri == LOG_PRI_INVLD) ? 0 : msg - pMsg->pszRawMsg);
	}
	RETiRet;
}


/* Parse a received message. The object's rawmsg property is taken and
 * parsed according to the relevant standards. This can later be
 * extended to support configured parsers.
 * rgerhards, 2008-10-09
 */
static rsRetVal
ParseMsg(smsg_t *pMsg)
{
	rsRetVal localRet = RS_RET_ERR;
	parserList_t *pParserList;
	parser_t *pParser;
	sbool bIsSanitized;
	sbool bPRIisParsed;
	static int iErrMsgRateLimiter = 0;
	DEFiRet;

	if(pMsg->iLenRawMsg == 0)
		ABORT_FINALIZE(RS_RET_EMPTY_MSG);

	CHKiRet(uncompressMessage(pMsg));

	/* we take the risk to print a non-sanitized string, because this is the best we can get
	 * (and that functionality is too important for debugging to drop it...).
	 */
	DBGPRINTF("msg parser: flags %x, from '%s', msg '%.60s'\n", pMsg->msgFlags,
		  (pMsg->msgFlags & NEEDS_DNSRESOL) ? UCHAR_CONSTANT("~NOTRESOLVED~") : getRcvFrom(pMsg),
		  pMsg->pszRawMsg);

	/* we now need to go through our list of parsers and see which one is capable of
	 * parsing the message. Note that the first parser that requires message sanitization
	 * will cause it to happen. After that, access to the unsanitized message is no
	 * loger possible.
	 */
	pParserList = ruleset.GetParserList(ourConf, pMsg);
	if(pParserList == NULL) {
		pParserList = pDfltParsLst;
	}
	DBGPRINTF("parse using parser list %p%s.\n", pParserList,
		  (pParserList == pDfltParsLst) ? " (the default list)" : "");

	bIsSanitized = RSFALSE;
	bPRIisParsed = RSFALSE;
	while(pParserList != NULL) {
		pParser = pParserList->pParser;
		if(pParser->bDoSanitazion && bIsSanitized == RSFALSE) {
			CHKiRet(SanitizeMsg(pMsg));
			if(pParser->bDoPRIParsing && bPRIisParsed == RSFALSE) {
				CHKiRet(ParsePRI(pMsg));
				bPRIisParsed = RSTRUE;
			}
			bIsSanitized = RSTRUE;
		}
		if(pParser->pModule->mod.pm.parse2 == NULL)
			localRet = pParser->pModule->mod.pm.parse(pMsg);
		else
			localRet = pParser->pModule->mod.pm.parse2(pParser->pInst, pMsg);
		DBGPRINTF("Parser '%s' returned %d\n", pParser->pName, localRet);
		if(localRet != RS_RET_COULD_NOT_PARSE)
			break;
		pParserList = pParserList->pNext;
	}

	/* We need to log a warning message and drop the message if we did not find a parser.
	 * Note that we log at most the first 1000 message, as this may very well be a problem
	 * that causes a message generation loop. We do not synchronize that counter, it doesn't
	 * matter if we log a handful messages more than we should...
	 */
	if(localRet != RS_RET_OK) {
		if(++iErrMsgRateLimiter < 1000) {
			LogError(0, localRet, "Error: one message could not be processed by "
				"any parser, message is being discarded (start of raw msg: '%.60s')",
				pMsg->pszRawMsg);
		}
		DBGPRINTF("No parser could process the message (state %d), we need to discard it.\n", localRet);
		ABORT_FINALIZE(localRet);
	}

	/* "finalize" message object */
	pMsg->msgFlags &= ~NEEDS_PARSING; /* this message is now parsed */

finalize_it:
	RETiRet;
}
/* queryInterface function-- rgerhards, 2009-11-03
 */
BEGINobjQueryInterface(parser)
CODESTARTobjQueryInterface(parser)
	if(pIf->ifVersion != parserCURR_IF_VERSION) { /* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->Construct = parserConstruct;
	pIf->ConstructFinalize = parserConstructFinalize;
	pIf->Destruct = parserDestruct;
	pIf->SetName = SetName;
	pIf->SetModPtr = SetModPtr;
	pIf->SetDoPRIParsing = SetDoPRIParsing;
	pIf->ParseMsg = ParseMsg;
	pIf->SanitizeMsg = SanitizeMsg;
	pIf->InitParserList = InitParserList;
	pIf->DestructParserList = DestructParserList;
	pIf->AddParserToList = AddParserToList;
	pIf->AddDfltParser = AddDfltParser;
	pIf->FindParser = FindParser;
finalize_it:
ENDobjQueryInterface(parser)

/* This destroys the master parserlist and all of its parser entries. MUST only be
 * done when the module is shut down. Parser modules are NOT unloaded, rsyslog
 * does that at a later stage for all dynamically loaded modules.
 */
static void
destroyMasterParserList(void)
{
	parserList_t *pParsLst;
	parserList_t *pParsLstDel;

	pParsLst = pParsLstRoot;
	while(pParsLst != NULL) {
		parserDestruct(&pParsLst->pParser);
		pParsLstDel = pParsLst;
		pParsLst = pParsLst->pNext;
		free(pParsLstDel);
	}
}

/* Exit our class.
 * rgerhards, 2009-11-04
 */
BEGINObjClassExit(parser, OBJ_IS_CORE_MODULE) /* class, version */
	DestructParserList(&pDfltParsLst);
	destroyMasterParserList();
	objRelease(glbl, CORE_COMPONENT);
	objRelease(datetime, CORE_COMPONENT);
	objRelease(ruleset, CORE_COMPONENT);
ENDObjClassExit(parser)


/* Initialize the parser class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2009-11-02
 */
BEGINObjClassInit(parser, 1, OBJ_IS_CORE_MODULE) /* class, version */
	/* request objects we use */
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(datetime, CORE_COMPONENT));
	CHKiRet(objUse(ruleset, CORE_COMPONENT));

	InitParserList(&pParsLstRoot);
	InitParserList(&pDfltParsLst);
ENDObjClassInit(parser)
