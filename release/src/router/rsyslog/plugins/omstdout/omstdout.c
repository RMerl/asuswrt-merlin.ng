/* omstdout.c
 * send all output to stdout - this is primarily a test driver (but may
 * be used for weired use cases). Not tested for robustness!
 *
 * NOTE: read comments in module-template.h for more specifics!
 *
 * File begun on 2009-03-19 by RGerhards
 *
 * Copyright 2009-2017 Adiscon GmbH.
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
#include "conf.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "template.h"
#include "module-template.h"
#include "errmsg.h"
#include "cfsysline.h"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("omstdout")

static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal);

/* static data */

/* internal structures
 */
DEF_OMOD_STATIC_DATA

/* config variables */

typedef struct _instanceData {
	int bUseArrayInterface;		/* uses action use array instead of string template interface? */
	int bEnsureLFEnding;		/* ensure that a linefeed is written at the end of EACH
					record (test aid for nettester) */
	uchar *templateName;
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
} wrkrInstanceData_t;

typedef struct configSettings_s {
	int bUseArrayInterface;		/* shall action use array instead of string template interface? */
	int bEnsureLFEnding;
	int templateName;
} configSettings_t;
static configSettings_t cs;

/* tables for interfacing with the v6 config system */
/* action (instance) parameters */
static struct cnfparamdescr actpdescr[] = {
	{ "ensurelfending", eCmdHdlrBinary, 0 },
	{ "template", eCmdHdlrGetWord, 0 }
};
static struct cnfparamblk actpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(actpdescr)/sizeof(struct cnfparamdescr),
	  actpdescr
	};

struct modConfData_s {
	rsconf_t *pConf;	/* our overall config object */
};

static modConfData_t *loadModConf = NULL;/* modConf ptr to use for the current load process */
static modConfData_t *runModConf = NULL;/* modConf ptr to use for the current exec process */



BEGINinitConfVars		/* (re)set config variables to default values */
CODESTARTinitConfVars
	resetConfigVariables(NULL, NULL);
ENDinitConfVars

BEGINcreateInstance
CODESTARTcreateInstance
ENDcreateInstance


BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
ENDcreateWrkrInstance


BEGINbeginCnfLoad
CODESTARTbeginCnfLoad
	loadModConf = pModConf;
	pModConf->pConf = pConf;
ENDbeginCnfLoad


BEGINendCnfLoad
CODESTARTendCnfLoad
	loadModConf = NULL; /* done loading */
ENDendCnfLoad

BEGINcheckCnf
CODESTARTcheckCnf
ENDcheckCnf

BEGINactivateCnf
CODESTARTactivateCnf
	runModConf = pModConf;
ENDactivateCnf

BEGINfreeCnf
CODESTARTfreeCnf
ENDfreeCnf


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
	dbgprintf("omstdout\n");
	dbgprintf("\tensureLFEnding='%d'\n", pData->bEnsureLFEnding);
	dbgprintf("\ttemplate='%s'\n", pData->templateName);
ENDdbgPrintInstInfo


BEGINtryResume
CODESTARTtryResume
ENDtryResume

BEGINdoAction
	char **szParams;
	char *toWrite;
	int iParamVal;
	int iParam;
	int iBuf;
	char szBuf[65564];
	size_t len;
	int r;
CODESTARTdoAction
dbgprintf("omstdout: in doAction\n");
	if(pWrkrData->pData->bUseArrayInterface) {
dbgprintf("omstdout: in ArrayInterface\n");
		/* if we use array passing, we need to put together a string
		 * ourselves. At this point, please keep in mind that omstdout is
		 * primarily a testing aid. Other modules may do different processing
		 * if they would like to support downlevel versions which do not support
		 * array-passing, but also use that interface on cores who do...
		 * So this code here is also more or less an example of how to do that.
		 * rgerhards, 2009-04-03
		 */
		szParams = (char**)(void*) (ppString[0]);
		/* In array-passing mode, ppString[] contains a NULL-terminated array
		 * of char *pointers.
		 */
		iParam = 0;
		iBuf = 0;
		while(szParams[iParam] != NULL && iBuf < (int)sizeof(szBuf)-1) {
			if(iParam > 0)
				szBuf[iBuf++] = ','; /* all but first need a delimiter */
			iParamVal = 0;
			while(szParams[iParam][iParamVal] != '\0' && iBuf < (int) sizeof(szBuf)-1) {
				szBuf[iBuf++] = szParams[iParam][iParamVal++];
			}
			++iParam;
		}
		szBuf[iBuf] = '\0';
		toWrite = szBuf;
	} else {
dbgprintf("omstdout: in else\n");
		toWrite = (char*) ppString[0];
	}
	len = strlen(toWrite);
	/* the following if's are just to silence compiler warnings. If someone
	 * actually intends to use this module in production (why???), this code
	 * needs to be more solid. -- rgerhards, 2012-11-28
	 */
dbgprintf("omstdout: len: %d, toWrite: %s\n", (int) len, toWrite);
	if((r = write(1, toWrite, len)) != (int) len) { /* 1 is stdout! */
		DBGPRINTF("omstdout: error %d writing to stdout[%zd]: %s\n",
			r, len, toWrite);
	}
	if(pWrkrData->pData->bEnsureLFEnding && toWrite[len-1] != '\n') {
		if((r = write(1, "\n", 1)) != 1) { /* write missing LF */
			DBGPRINTF("omstdout: error %d writing \\n to stdout\n",
				r);
		}
	}
ENDdoAction

static void
setInstParamDefaults(instanceData *pData)
{
	pData->bEnsureLFEnding = 1;
	pData->templateName = (uchar*) "RSYSLOG_FileFormat";
	pData->bUseArrayInterface = 0;
}


BEGINnewActInst
	struct cnfparamvals *pvals;
	int i;
	int bDestructPValsOnExit;
	uchar *tplToUse;
CODESTARTnewActInst
	DBGPRINTF("newActInst (omstdout)\n");

	bDestructPValsOnExit = 0;
	pvals = nvlstGetParams(lst, &actpblk, NULL);
	if(pvals == NULL) {
		LogError(0, RS_RET_MISSING_CNFPARAMS, "omstdout: error reading "
				"config parameters");
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}
	bDestructPValsOnExit = 1;

	if(Debug) {
		dbgprintf("action param blk in omstdout:\n");
		cnfparamsPrint(&actpblk, pvals);
	}

	CHKiRet(createInstance(&pData));
	setInstParamDefaults(pData);

	for(i = 0 ; i < actpblk.nParams ; ++i) {
		if(!pvals[i].bUsed) {
			continue;
		} else if(!strcmp(actpblk.descr[i].name, "ensurelfending")) {
			pData->bEnsureLFEnding = (int) pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "template")) {
			pData->templateName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else {
			DBGPRINTF("omstdout: program error, non-handled "
			  "param '%s'\n", actpblk.descr[i].name);
		}
	}


	CODE_STD_STRING_REQUESTnewActInst(1)
	//TODO: make the template a parameter
	tplToUse = (uchar*) strdup((pData->templateName == NULL) ? "RSYSLOG_FileFormat" : (char *)pData->templateName);
	CHKiRet(OMSRsetEntry(*ppOMSR, 0, tplToUse, OMSR_NO_RQD_TPL_OPTS));
CODE_STD_FINALIZERnewActInst
	if(bDestructPValsOnExit)
		cnfparamvalsDestruct(pvals, &actpblk);
ENDnewActInst



BEGINparseSelectorAct
	int iTplOpts;
CODESTARTparseSelectorAct
CODE_STD_STRING_REQUESTparseSelectorAct(1)
	/* first check if this config line is actually for us */
	if(strncmp((char*) p, ":omstdout:", sizeof(":omstdout:") - 1)) {
		ABORT_FINALIZE(RS_RET_CONFLINE_UNPROCESSED);
	}

	/* ok, if we reach this point, we have something for us */
	p += sizeof(":omstdout:") - 1; /* eat indicator sequence  (-1 because of '\0'!) */
	CHKiRet(createInstance(&pData));

	/* check if a non-standard template is to be applied */
	if(*(p-1) == ';')
		--p;
	iTplOpts = (cs.bUseArrayInterface == 0) ? 0 : OMSR_TPL_AS_ARRAY;
	CHKiRet(cflineParseTemplateName(&p, *ppOMSR, 0, iTplOpts, (uchar*) "RSYSLOG_FileFormat"));
	pData->bUseArrayInterface = cs.bUseArrayInterface;
	pData->bEnsureLFEnding = cs.bEnsureLFEnding;
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
CODEqueryEtryPt_STD_CONF2_QUERIES
CODEqueryEtryPt_STD_CONF2_OMOD_QUERIES
ENDqueryEtryPt



/* Reset config variables for this module to default values.
 */
static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal)
{
	DEFiRet;
	cs.bUseArrayInterface = 0;
	cs.bEnsureLFEnding = 1;
	RETiRet;
}


BEGINmodInit()
	rsRetVal localRet;
	rsRetVal (*pomsrGetSupportedTplOpts)(unsigned long *pOpts);
	unsigned long opts;
	int bArrayPassingSupported;		/* does core support template passing as an array? */
CODESTARTmodInit
INITLegCnfVars
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	/* check if the rsyslog core supports parameter passing code */
	bArrayPassingSupported = 0;
	localRet = pHostQueryEtryPt((uchar*)"OMSRgetSupportedTplOpts", &pomsrGetSupportedTplOpts);
	if(localRet == RS_RET_OK) {
		/* found entry point, so let's see if core supports array passing */
		CHKiRet((*pomsrGetSupportedTplOpts)(&opts));
		if(opts & OMSR_TPL_AS_ARRAY)
			bArrayPassingSupported = 1;
	} else if(localRet != RS_RET_ENTRY_POINT_NOT_FOUND) {
		ABORT_FINALIZE(localRet); /* Something else went wrong, what is not acceptable */
	}
	DBGPRINTF("omstdout: array-passing is %ssupported by rsyslog core.\n", bArrayPassingSupported ? "" : "not ");

	if(bArrayPassingSupported) {
		/* enable config comand only if core supports it */
		CHKiRet(omsdRegCFSLineHdlr((uchar *)"actionomstdoutarrayinterface", 0, eCmdHdlrBinary, NULL,
			                   &cs.bUseArrayInterface, STD_LOADABLE_MODULE_ID));
	}
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"actionomstdoutensurelfending", 0, eCmdHdlrBinary, NULL,
				   &cs.bEnsureLFEnding, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler,
				    resetConfigVariables, NULL, STD_LOADABLE_MODULE_ID));
ENDmodInit

/* vi:set ai:
 */
