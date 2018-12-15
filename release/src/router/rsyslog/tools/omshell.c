/* omshell.c
 * This is the implementation of the build-in shell output module.
 *
 * ************* DO NOT EXTEND THIS MODULE **************
 * This is pure legacy, omprog has much better and more
 * secure functionality than this module. It is NOT
 * recommended to base new work on it!
 * 2012-01-19 rgerhards
 * ******************************************************
 *
 * NOTE: read comments in module-template.h to understand how this file
 *       works!
 *
 * shell support was initially written by bkalkbrenner 2005-09-20
 *
 * File begun on 2007-07-20 by RGerhards (extracted from syslogd.c)
 * This file is under development and has not yet arrived at being fully
 * self-contained and a real object. So far, it is mostly an excerpt
 * of the "old" message code without any modifications. However, it
 * helps to have things at the right place one we go to the meat of it.
 *
 * Copyright 2007-2016 Adiscon GmbH.
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
#include "conf.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "omshell.h"
#include "module-template.h"
#include "errmsg.h"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP

/* internal structures
 */
DEF_OMOD_STATIC_DATA

typedef struct _instanceData {
	uchar	progName[MAXFNAME]; /* program  to execute */
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
} wrkrInstanceData_t;

BEGINcreateInstance
CODESTARTcreateInstance
ENDcreateInstance


BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
ENDcreateWrkrInstance


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	if(eFeat == sFEATURERepeatedMsgReduction)
		iRet = RS_RET_OK;
ENDisCompatibleWithFeature


BEGINfreeInstance
CODESTARTfreeInstance
ENDfreeInstance


BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance


BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
	printf("%s", pData->progName);
ENDdbgPrintInstInfo


BEGINtryResume
CODESTARTtryResume
ENDtryResume

BEGINdoAction
CODESTARTdoAction
	dbgprintf("\n");
	if(execProg((uchar*) pWrkrData->pData->progName, 1, ppString[0]) == 0)
	 	LogError(0, NO_ERRCODE, "Executing program '%s' failed", (char*)pWrkrData->pData->progName);
ENDdoAction


BEGINparseSelectorAct
CODESTARTparseSelectorAct
CODE_STD_STRING_REQUESTparseSelectorAct(1)
	/* yes, the if below is redundant, but I need it now. Will go away as
	 * the code further changes.  -- rgerhards, 2007-07-25
	 */
	if(*p == '^') {
		if((iRet = createInstance(&pData)) != RS_RET_OK)
			goto finalize_it;
	}


	switch (*p)
	{
	case '^': /* bkalkbrenner 2005-09-20: execute shell command */
		dbgprintf("exec\n");
		++p;
		iRet = cflineParseFileName(p, (uchar*) pData->progName, *ppOMSR, 0, OMSR_NO_RQD_TPL_OPTS,
			                   (uchar*)"RSYSLOG_TraditionalFileFormat");
		break;
	default:
		iRet = RS_RET_CONFLINE_UNPROCESSED;
		break;
	}

CODE_STD_FINALIZERparseSelectorAct
ENDparseSelectorAct


BEGINmodExit
CODESTARTmodExit
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_OMOD_QUERIES
CODEqueryEtryPt_STD_OMOD8_QUERIES
ENDqueryEtryPt


BEGINmodInit(Shell)
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
ENDmodInit

/*
 * vi:set ai:
 */
