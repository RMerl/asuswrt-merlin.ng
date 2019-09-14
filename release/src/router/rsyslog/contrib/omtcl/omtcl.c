/* omtcl.c
 * invoke a tcl procedure for every message
 *
 * NOTE: read comments in module-template.h for more specifics!
 *
 * File begun on 2016-05-16 by fcr
 *
 * Copyright 2016 Francisco Castro <fcr@adinet.com.uy>
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
/* work around gcc-7 build problems - acceptable for contributed module */
#pragma GCC diagnostic ignored "-Wundef"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include "conf.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "template.h"
#include "module-template.h"
#include "errmsg.h"
#include "cfsysline.h"
#include "tcl.h"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP

DEF_OMOD_STATIC_DATA

typedef struct _instanceData {
	Tcl_Interp * interp;
	Tcl_Obj * cmdName;
} instanceData;

typedef struct wrkrInstanceData {
	instanceData * pData;
} wrkrInstanceData_t;

BEGINinitConfVars
CODESTARTinitConfVars
ENDinitConfVars

BEGINcreateInstance
CODESTARTcreateInstance
	pData->interp = Tcl_CreateInterp();
	pData->cmdName = NULL;
ENDcreateInstance

BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
ENDcreateWrkrInstance

BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
/* not compatible with message reduction */
ENDisCompatibleWithFeature

BEGINfreeInstance
CODESTARTfreeInstance
	if(pData->cmdName != NULL)
		Tcl_DecrRefCount(pData->cmdName);
	Tcl_DeleteInterp(pData->interp);
ENDfreeInstance

BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance

BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
ENDdbgPrintInstInfo

BEGINtryResume
CODESTARTtryResume
ENDtryResume

BEGINdoAction
	Tcl_Obj * objv[2];
CODESTARTdoAction
	objv[0] = pWrkrData->pData->cmdName;
	objv[1] = Tcl_NewStringObj((char*) ppString[0], -1);
	if (Tcl_EvalObjv(pWrkrData->pData->interp, 2, objv, 0) != TCL_OK) {
		iRet = RS_RET_ERR;
		DBGPRINTF("omtcl: %s", Tcl_GetStringResult(pWrkrData->pData->interp));
	}
ENDdoAction

BEGINparseSelectorAct
	char fileName[PATH_MAX+1];
	char buffer[4096];
CODESTARTparseSelectorAct
CODE_STD_STRING_REQUESTparseSelectorAct(1)
	if(strncmp((char*) p, ":omtcl:", sizeof(":omtcl:") - 1)) {
		ABORT_FINALIZE(RS_RET_CONFLINE_UNPROCESSED);
	}
	p += sizeof(":omtcl:") - 1;

	if(getSubString(&p, fileName, PATH_MAX+1, ',') || getSubString(&p, buffer, 4096, ';') || !strlen(buffer)) {
		LogError(0, RS_RET_INVALID_PARAMS, "Invalid OmTcl parameters");
		ABORT_FINALIZE(RS_RET_INVALID_PARAMS);
	}

	if (*(p-1) == ';')
		--p;

	CHKiRet(cflineParseTemplateName(&p, *ppOMSR, 0, 0, (uchar*) "RSYSLOG_FileFormat"));

	CHKiRet(createInstance(&pData));
	pData->cmdName = Tcl_NewStringObj(buffer, -1);
	Tcl_IncrRefCount(pData->cmdName);

	// TODO parse arguments: file,procname
	if (Tcl_EvalFile(pData->interp, fileName) == TCL_ERROR) {
		LogError(0, RS_RET_CONFIG_ERROR, "Loading Tcl script: %s", Tcl_GetStringResult(pData->interp));
		ABORT_FINALIZE(RS_RET_CONFIG_ERROR);
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


BEGINmodInit()
CODESTARTmodInit
INITLegCnfVars
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	DBGPRINTF("omtcl: module compiled with rsyslog version %s.\n", VERSION);

ENDmodInit

