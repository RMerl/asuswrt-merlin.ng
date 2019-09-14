/* omdiscard.c
 * This is the implementation of the built-in discard output module.
 *
 * NOTE: read comments in module-template.h to understand how this file
 *       works!
 *
 * File begun on 2007-07-24 by RGerhards
 *
 * Copyright 2007-2013 Adiscon GmbH.
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
#include "syslogd.h"
#include "syslogd-types.h"
#include "omdiscard.h"
#include "module-template.h"
#include "errmsg.h"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP

/* internal structures
 */
DEF_OMOD_STATIC_DATA

typedef struct _instanceData {
	EMPTY_STRUCT
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
} wrkrInstanceData_t;

/* we do not need a createInstance()!
BEGINcreateInstance
CODESTARTcreateInstance
ENDcreateInstance
*/


BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
ENDcreateWrkrInstance


BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
	/* do nothing */
ENDdbgPrintInstInfo


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	/* we are not compatible with repeated msg reduction feature, so do not allow it */
ENDisCompatibleWithFeature


BEGINtryResume
CODESTARTtryResume
ENDtryResume

BEGINdoAction_NoStrings
CODESTARTdoAction
	(void)pMsgData; /* Suppress compiler warning on unused var */
	dbgprintf("\n");
	iRet = RS_RET_DISCARDMSG;
ENDdoAction


BEGINfreeInstance
CODESTARTfreeInstance
	/* we do not have instance data, so we do not need to
	 * do anything here. -- rgerhards, 2007-07-25
	 */
ENDfreeInstance


BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance


BEGINparseSelectorAct
CODESTARTparseSelectorAct
CODE_STD_STRING_REQUESTparseSelectorAct(0)
	pData = NULL; /* this action does not have any instance data */
	p = *pp;

	if(*p == '~') {
		dbgprintf("discard\n");
		LogMsg(0, RS_RET_DEPRECATED, LOG_WARNING,
			"warning: ~ action is deprecated, consider "
			"using the 'stop' statement instead");
	} else {
		iRet = RS_RET_CONFLINE_UNPROCESSED;
	}
/* we do not use the macro
 * CODE_STD_FINALIZERparseSelectorAct
 * here as this causes a Coverity ID "false positive" (CID 185431).
 * We don't see an issue with using the copy&pasted code as it is unlikly
 * to change for this (outdated) module.
 */
finalize_it: ATTR_UNUSED; /* semi-colon needed according to gcc doc! */
	if(iRet == RS_RET_OK || iRet == RS_RET_OK_WARN || iRet == RS_RET_SUSPENDED) {
		*ppModData = pData;
		*pp = p;
	} else {
		/* cleanup, we failed */
		if(*ppOMSR != NULL) {
			OMSRdestruct(*ppOMSR);
			*ppOMSR = NULL;
		}
	}
/* END modified macro text */
ENDparseSelectorAct


BEGINmodExit
CODESTARTmodExit
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_OMOD_QUERIES
CODEqueryEtryPt_STD_OMOD8_QUERIES
ENDqueryEtryPt


BEGINmodInit(Discard)
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
ENDmodInit
/*
 * vi:set ai:
 */
