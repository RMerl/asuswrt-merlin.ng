/* immark.c
 * This is the implementation of the build-in mark message input module.
 *
 * NOTE: read comments in module-template.h to understand how this file
 *       works!
 *
 * File begun on 2007-07-20 by RGerhards (extracted from syslogd.c)
 * This file is under development and has not yet arrived at being fully
 * self-contained and a real object. So far, it is mostly an excerpt
 * of the "old" message code without any modifications. However, it
 * helps to have things at the right place one we go to the meat of it.
 *
 * Copyright 2007-2012 Adiscon GmbH.
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
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include "dirty.h"
#include "cfsysline.h"
#include "module-template.h"
#include "errmsg.h"
#include "msg.h"
#include "srUtils.h"
#include "glbl.h"

MODULE_TYPE_INPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("immark")

/* defines */
#define DEFAULT_MARK_PERIOD (20 * 60)

/* Module static data */
DEF_IMOD_STATIC_DATA
DEFobjCurrIf(glbl)

static int iMarkMessagePeriod = DEFAULT_MARK_PERIOD;
struct modConfData_s {
	rsconf_t *pConf;	/* our overall config object */
	int iMarkMessagePeriod;
	sbool configSetViaV2Method;
};

/* module-global parameters */
static struct cnfparamdescr modpdescr[] = {
	{ "interval", eCmdHdlrInt, 0 }
};
static struct cnfparamblk modpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(modpdescr)/sizeof(struct cnfparamdescr),
	  modpdescr
	};


static modConfData_t *loadModConf = NULL;/* modConf ptr to use for the current load process */
static int bLegacyCnfModGlobalsPermitted;/* are legacy module-global config parameters permitted? */


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	if(eFeat == sFEATURENonCancelInputTermination)
		iRet = RS_RET_OK;
ENDisCompatibleWithFeature


BEGINafterRun
CODESTARTafterRun
ENDafterRun


BEGINbeginCnfLoad
CODESTARTbeginCnfLoad
	loadModConf = pModConf;
	pModConf->pConf = pConf;
	/* init our settings */
	pModConf->iMarkMessagePeriod = DEFAULT_MARK_PERIOD;
	loadModConf->configSetViaV2Method = 0;
	bLegacyCnfModGlobalsPermitted = 1;
ENDbeginCnfLoad


BEGINsetModCnf
	struct cnfparamvals *pvals = NULL;
	int i;
CODESTARTsetModCnf
	pvals = nvlstGetParams(lst, &modpblk, NULL);
	if(pvals == NULL) {
		LogError(0, RS_RET_MISSING_CNFPARAMS, "error processing module "
				"config parameters [module(...)]");
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	if(Debug) {
		dbgprintf("module (global) param blk for immark:\n");
		cnfparamsPrint(&modpblk, pvals);
	}

	for(i = 0 ; i < modpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(modpblk.descr[i].name, "interval")) {
			loadModConf->iMarkMessagePeriod = (int) pvals[i].val.d.n;
		} else {
			dbgprintf("immark: program error, non-handled "
			  "param '%s' in beginCnfLoad\n", modpblk.descr[i].name);
		}
	}

	/* disable legacy module-global config directives */
	bLegacyCnfModGlobalsPermitted = 0;
	loadModConf->configSetViaV2Method = 1;

finalize_it:
	if(pvals != NULL)
		cnfparamvalsDestruct(pvals, &modpblk);
ENDsetModCnf


BEGINendCnfLoad
CODESTARTendCnfLoad
	if(!loadModConf->configSetViaV2Method) {
		pModConf->iMarkMessagePeriod = iMarkMessagePeriod;
	}
ENDendCnfLoad


BEGINcheckCnf
CODESTARTcheckCnf
	if(pModConf->iMarkMessagePeriod == 0) {
		LogError(0, NO_ERRCODE, "immark: mark message period must not be 0, can not run");
		ABORT_FINALIZE(RS_RET_NO_RUN);	/* we can not run with this error */
	}
finalize_it:
ENDcheckCnf


BEGINactivateCnf
CODESTARTactivateCnf
	MarkInterval = pModConf->iMarkMessagePeriod;
	DBGPRINTF("immark set MarkInterval to %d\n", MarkInterval);
ENDactivateCnf


BEGINfreeCnf
CODESTARTfreeCnf
ENDfreeCnf


/* This function is called to gather input. It must terminate only
 * a) on failure (iRet set accordingly)
 * b) on termination of the input module (as part of the unload process)
 * Code begun 2007-12-12 rgerhards
 *
 * This code must simply spawn emit a mark message at each mark interval.
 * We are running on our own thread, so this is extremely easy: we just
 * sleep MarkInterval seconds and each time we awake, we inject the message.
 * Please note that we do not do the other fancy things that sysklogd
 * (and pre 1.20.2 releases of rsyslog) did in mark procesing. They simply
 * do not belong here.
 */
BEGINrunInput
CODESTARTrunInput
	/* this is an endless loop - it is terminated when the thread is
	 * signalled to do so. This, however, is handled by the framework,
	 * right into the sleep below.
	 */
	while(1) {
		srSleep(MarkInterval, 0); /* seconds, micro seconds */

		if(glbl.GetGlobalInputTermState() == 1)
			break; /* terminate input! */

		dbgprintf("immark: injecting mark message\n");
		logmsgInternal(NO_ERRCODE, LOG_SYSLOG|LOG_INFO, (uchar*)"-- MARK --", MARK);
	}
ENDrunInput


BEGINwillRun
CODESTARTwillRun
ENDwillRun


BEGINmodExit
CODESTARTmodExit
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_IMOD_QUERIES
CODEqueryEtryPt_STD_CONF2_QUERIES
CODEqueryEtryPt_STD_CONF2_setModCnf_QUERIES
CODEqueryEtryPt_IsCompatibleWithFeature_IF_OMOD_QUERIES
ENDqueryEtryPt

static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal)
{
	iMarkMessagePeriod = DEFAULT_MARK_PERIOD;
	return RS_RET_OK;
}

BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	CHKiRet(objUse(glbl, CORE_COMPONENT));

	/* legacy config handlers */
	CHKiRet(regCfSysLineHdlr2((uchar *)"markmessageperiod", 0, eCmdHdlrInt, NULL,
		&iMarkMessagePeriod, STD_LOADABLE_MODULE_ID, &bLegacyCnfModGlobalsPermitted));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler,
		resetConfigVariables, NULL, STD_LOADABLE_MODULE_ID));
ENDmodInit
/* vi:set ai:
 */
