/* mmaudit.c
 * This is a message modification module supporting Linux audit format
 * in various settings. The module tries to identify the provided
 * message as being a Linux audit record and, if so, converts it into
 * cee-enhanced syslog format.
 *
 * NOTE WELL:
 * Right now, we do not do any trust checks. So it is possible that a
 * malicous user emits something that looks like an audit record and
 * tries to fool the system with that. Solving this trust issue is NOT
 * an easy thing to do. This will be worked on, as the lumberjack effort
 * continues. Please consider the module in its current state as a proof
 * of concept.
 *
 * File begun on 2012-02-23 by RGerhards
 *
 * Copyright 2013-2016 Adiscon GmbH.
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
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <json.h>
#include "conf.h"
#include "syslogd-types.h"
#include "template.h"
#include "module-template.h"
#include "errmsg.h"
#include "cfsysline.h"
#include "dirty.h"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP


/* static data */

/* internal structures
 */
DEF_OMOD_STATIC_DATA

typedef struct _instanceData {
	int dummy; /* remove when the first real parameter is needed */
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
} wrkrInstanceData_t;


BEGINinitConfVars		/* (re)set config variables to default values */
CODESTARTinitConfVars
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
CODESTARTfreeInstance
ENDfreeInstance

BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance


BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
	dbgprintf("mmaudit\n");
ENDdbgPrintInstInfo


BEGINtryResume
CODESTARTtryResume
ENDtryResume


static void
skipWhitespace(uchar **buf)
{
	while(**buf && isspace(**buf))
		++(*buf);
}


static rsRetVal
parseName(uchar **buf, char *name, unsigned lenName)
{
	unsigned i;
	skipWhitespace(buf);
	--lenName; /* reserve space for '\0' */
	i = 0;
	while(**buf && **buf != '=' && lenName) {
//dbgprintf("parseNAme, buf: %s\n", *buf);
		name[i++] = **buf;
		++(*buf), --lenName;
	}
	name[i] = '\0';
	return RS_RET_OK;
}


static rsRetVal
parseValue(uchar **buf, char *val, unsigned lenval)
{
	char termc;
	unsigned i;
	DEFiRet;

	--lenval; /* reserve space for '\0' */
	i = 0;
	if(**buf == '\0') {
		FINALIZE;
	} else if(**buf == '\'') {
		termc = '\'';
		++(*buf);
	} else if(**buf == '"') {
		termc = '"';
		++(*buf);
	} else {
		termc = ' ';
	}

	while(**buf && **buf != termc && lenval) {
//dbgprintf("parseValue, termc '%c', buf: %s\n", termc, *buf);
		val[i++] = **buf;
		++(*buf), --lenval;
	}
	val[i] = '\0';

finalize_it:
	RETiRet;
}


/* parse the audit record and create libee structure
 */
static rsRetVal
audit_parse(uchar *buf, struct json_object **jsonRoot)
{
	struct json_object *json;
	struct json_object *jval;
	char name[1024];
	char val[1024];
	DEFiRet;

	*jsonRoot = json_object_new_object();
	if(*jsonRoot == NULL) {
		ABORT_FINALIZE(RS_RET_ERR);
	}
	json = json_object_new_object();
	json_object_object_add(*jsonRoot, "data", json);

	while(*buf) {
		CHKiRet(parseName(&buf, name, sizeof(name)));
		if(*buf != '=') {
			ABORT_FINALIZE(RS_RET_ERR);
		}
		++buf;
		CHKiRet(parseValue(&buf, val, sizeof(val)));
		jval = json_object_new_string(val);
		json_object_object_add(json, name, jval);
	}
	

finalize_it:
	RETiRet;
}


BEGINdoAction_NoStrings
	smsg_t **ppMsg = (smsg_t **) pMsgData;
	smsg_t *pMsg = ppMsg[0];
	uchar *buf;
	int typeID;
	struct json_object *jsonRoot;
	struct json_object *json;
	struct json_object *jval;
	int i;
	char auditID[1024];
	int bSuccess = 0;
CODESTARTdoAction
	/* note that we can performance-optimize the interface, but this also
	 * requires changes to the libraries. For now, we accept message
	 * duplication. -- rgerhards, 2010-12-01
	 */
	buf = getMSG(pMsg);

	while(*buf && isspace(*buf)) {
		++buf;
	}

	if(*buf == '\0' || strncmp((char*)buf, "type=", 5)) {
		DBGPRINTF("mmaudit: type= undetected: '%s'\n", buf);
		FINALIZE;
	}
	buf += 5;

	typeID = 0;
	while(*buf && isdigit(*buf)) {
		typeID = typeID * 10 + *buf - '0';
		++buf;
	}

	if(*buf == '\0' || strncmp((char*)buf, " audit(", sizeof(" audit(")-1)) {
		DBGPRINTF("mmaudit: audit( header not found: %s'\n", buf);
		FINALIZE;
	}
	buf += sizeof(" audit(");

	for(i = 0 ; i < (int) (sizeof(auditID)-2) && *buf && *buf != ')' ; ++i) {
		auditID[i] = *buf++;
	}
	auditID[i] = '\0';
	if(*buf != ')' || *(buf+1) != ':') {
		DBGPRINTF("mmaudit: trailer '):' not found, no audit record: %s'\n", buf);
		FINALIZE;
	}
	buf += 2;

	audit_parse(buf, &jsonRoot);
	if(jsonRoot == NULL) {
		DBGPRINTF("mmaudit: audit parse error, assuming no "
			  "audit message: '%s'\n", buf);
		FINALIZE;
	}

	/* we now need to shuffle the "outer" properties into that stream */
	json = json_object_new_object();
	json_object_object_add(jsonRoot, "hdr", json);
	jval = json_object_new_string(auditID);
	json_object_object_add(json, "auditid", jval);
	jval = json_object_new_int(typeID);
	json_object_object_add(json, "type", jval);

	msgAddJSON(pMsg, (uchar*)"!audit", jsonRoot, 0, 0);
	bSuccess = 1;

finalize_it:
	MsgSetParseSuccess(pMsg, bSuccess);
ENDdoAction


BEGINparseSelectorAct
CODESTARTparseSelectorAct
CODE_STD_STRING_REQUESTparseSelectorAct(1)
	/* first check if this config line is actually for us */
	if(strncmp((char*) p, ":mmaudit:", sizeof(":mmaudit:") - 1)) {
		ABORT_FINALIZE(RS_RET_CONFLINE_UNPROCESSED);
	}

	/* ok, if we reach this point, we have something for us */
	p += sizeof(":mmaudit:") - 1; /* eat indicator sequence  (-1 because of '\0'!) */
	CHKiRet(createInstance(&pData));

	/* check if a non-standard template is to be applied */
	if(*(p-1) == ';')
		--p;
	/* we call the function below because we need to call it via our interface definition. However,
	 * the format specified (if any) is always ignored.
	 */
	iRet = cflineParseTemplateName(&p, *ppOMSR, 0, OMSR_TPL_AS_MSG, (uchar*) "RSYSLOG_FileFormat");
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
		DBGPRINTF("mmaudit: msg-passing is not supported by rsyslog core, "
			  "can not continue.\n");
		ABORT_FINALIZE(RS_RET_NO_MSG_PASSING);
	}

ENDmodInit

/* vi:set ai:
 */
