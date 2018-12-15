/* smfile.c
 * This is a strgen module for the traditional file format.
 *
 * Format generated:
 * "%TIMESTAMP:::date-rfc3339% %HOSTNAME% %syslogtag%%msg:::sp-if-no-1st-sp%%msg:::drop-last-lf%\n"
 * Note that this is the same as smtradfile.c, except that we do have a RFC3339 timestamp. However,
 * we have copied over the code from there, it is too simple to go through all the hassle
 * of having a single code base.
 *
 * NOTE: read comments in module-template.h to understand how this file
 *       works!
 *
 * File begun on 2010-06-01 by RGerhards
 *
 * Copyright 2010-2014 Rainer Gerhards and Adiscon GmbH.
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
#include "unicode-helper.h"

MODULE_TYPE_STRGEN
MODULE_TYPE_NOKEEP
STRGEN_NAME("RSYSLOG_FileFormat")

/* internal structures
 */
DEF_SMOD_STATIC_DATA


/* config data */


/* This strgen tries to minimize the amount of reallocs be first obtaining pointers to all strings
 * needed (including their length) and then calculating the actual space required. So when we
 * finally copy, we know exactly what we need. So we do at most one alloc.
 */
BEGINstrgen
	register int iBuf;
	uchar *pTimeStamp;
	size_t lenTimeStamp;
	uchar *pHOSTNAME;
	size_t lenHOSTNAME;
	uchar *pTAG;
	int lenTAG;
	uchar *pMSG;
	size_t lenMSG;
	size_t lenTotal;
CODESTARTstrgen
	/* first obtain all strings and their length (if not fixed) */
	pTimeStamp = (uchar*) getTimeReported(pMsg, tplFmtRFC3339Date);
	lenTimeStamp = ustrlen(pTimeStamp);
	pHOSTNAME = (uchar*) getHOSTNAME(pMsg);
	lenHOSTNAME = getHOSTNAMELen(pMsg);
	getTAG(pMsg, &pTAG, &lenTAG);
	pMSG = getMSG(pMsg);
	lenMSG = getMSGLen(pMsg);

	/* calculate len, constants for spaces and similar fixed strings */
	lenTotal = lenTimeStamp + 1 + lenHOSTNAME + 1 + lenTAG + lenMSG + 2;
	if(pMSG[0] != ' ')
		++lenTotal; /* then we need to introduce one additional space */

	/* now make sure buffer is large enough */
	if(lenTotal  >= iparam->lenBuf)
		CHKiRet(ExtendBuf(iparam, lenTotal));

	/* and concatenate the resulting string */
	memcpy(iparam->param, pTimeStamp, lenTimeStamp);
	iBuf = lenTimeStamp;
	iparam->param[iBuf++] = ' ';

	memcpy(iparam->param + iBuf, pHOSTNAME, lenHOSTNAME);
	iBuf += lenHOSTNAME;
	iparam->param[iBuf++] = ' ';

	memcpy(iparam->param + iBuf, pTAG, lenTAG);
	iBuf += lenTAG;

	if(pMSG[0] != ' ')
		iparam->param[iBuf++] = ' ';
	memcpy(iparam->param + iBuf, pMSG, lenMSG);
	iBuf += lenMSG;

	/* trailer */
	iparam->param[iBuf++] = '\n';
	iparam->param[iBuf] = '\0';

	iparam->lenStr = lenTotal - 1; /* do not count \0! */

finalize_it:
ENDstrgen


BEGINmodExit
CODESTARTmodExit
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_SMOD_QUERIES
ENDqueryEtryPt


BEGINmodInit(smfile)
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr

	dbgprintf("rsyslog standard file format strgen init called, compiled with version %s\n", VERSION);
ENDmodInit
