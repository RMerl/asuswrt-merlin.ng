/* ompipe.c
 * This is the implementation of the build-in pipe output module.
 * Note that this module stems back to the "old" (4.4.2 and below)
 * omfile. There were some issues with the new omfile code and pipes
 * (namely in regard to xconsole), so we took out the pipe code and moved
 * that to a separate module. That a) immediately solves the issue for a
 * less common use case and probably makes it much easier to enhance
 * file and pipe support (now independently) in the future (we always
 * needed to think about pipes in omfile so far, what we now no longer
 * need to, hopefully resulting in reduction of complexity).
 *
 * NOTE: read comments in module-template.h to understand how this pipe
 *       works!
 *
 * Copyright 2007-2018 Rainer Gerhards and Adiscon GmbH.
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
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>

#include "rsyslog.h"
#include "syslogd.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "template.h"
#include "ompipe.h"
#include "omfile.h" /* for dirty trick: access to $ActionFileDefaultTemplate value */
#include "cfsysline.h"
#include "module-template.h"
#include "conf.h"
#include "errmsg.h"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("ompipe")

/* internal structures
 */
DEF_OMOD_STATIC_DATA


typedef struct _instanceData {
	uchar	*pipe;	/* pipe or template name (display only) */
	uchar	*tplName;       /* format template to use */
	short	fd;		/* pipe descriptor for (current) pipe */
	pthread_mutex_t mutWrite; /* guard against multiple instances writing to same pipe */
	sbool	bHadError;	/* did we already have/report an error on this pipe? */
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
} wrkrInstanceData_t;

typedef struct configSettings_s {
	EMPTY_STRUCT
} configSettings_t;
static configSettings_t __attribute__((unused)) cs;

/* tables for interfacing with the v6 config system */
/* module-global parameters */
static struct cnfparamdescr modpdescr[] = {
	{ "template", eCmdHdlrGetWord, 0 },
};
static struct cnfparamblk modpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(modpdescr)/sizeof(struct cnfparamdescr),
	  modpdescr
	};

/* action (instance) parameters */
static struct cnfparamdescr actpdescr[] = {
	{ "pipe", eCmdHdlrString, CNFPARAM_REQUIRED },
	{ "template", eCmdHdlrGetWord, 0 }
};
static struct cnfparamblk actpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(actpdescr)/sizeof(struct cnfparamdescr),
	  actpdescr
	};

struct modConfData_s {
	rsconf_t *pConf;	/* our overall config object */
	uchar 	*tplName;	/* default template */
};

static modConfData_t *loadModConf = NULL;/* modConf ptr to use for the current load process */
static modConfData_t *runModConf = NULL;/* modConf ptr to use for the current exec process */

/* this function gets the default template */
static uchar*
getDfltTpl(void)
{
	if(loadModConf != NULL && loadModConf->tplName != NULL)
		return loadModConf->tplName;
	else
		return (uchar*)"RSYSLOG_FileFormat";
}


BEGINinitConfVars		/* (re)set config variables to default values */
CODESTARTinitConfVars
ENDinitConfVars


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	if(eFeat == sFEATURERepeatedMsgReduction)
		iRet = RS_RET_OK;
ENDisCompatibleWithFeature


BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
	dbgprintf("pipe %s", pData->pipe);
	if (pData->fd == -1)
		dbgprintf(" (unused)");
ENDdbgPrintInstInfo


/* This is now shared code for all types of files. It simply prepares
 * pipe access, which, among others, means the the pipe wil be opened
 * and any directories in between will be created (based on config, of
 * course). -- rgerhards, 2008-10-22
 * changed to iRet interface - 2009-03-19
 */
static rsRetVal
preparePipe(instanceData *pData)
{
	DEFiRet;
	pData->fd = open((char*) pData->pipe, O_RDWR|O_NONBLOCK|O_CLOEXEC);
	if(pData->fd < 0 ) {
		pData->fd = -1;
		if(!pData->bHadError) {
			LogError(errno, RS_RET_NO_FILE_ACCESS, "Could not open output pipe '%s':",
				        pData->pipe);
			pData->bHadError = 1;
		}
		DBGPRINTF("Error opening log pipe: %s\n", pData->pipe);
	}
	RETiRet;
}


/* rgerhards 2004-11-11: write to a pipe output. This
 * will be called for all outputs using pipe semantics,
 * for example also for pipes.
 */
static rsRetVal writePipe(uchar **ppString, instanceData *pData)
{
	int iLenWritten;
	DEFiRet;

	ASSERT(pData != NULL);

	if(pData->fd == -1) {
		rsRetVal iRetLocal;
		iRetLocal = preparePipe(pData);
		if((iRetLocal != RS_RET_OK) || (pData->fd == -1))
			ABORT_FINALIZE(RS_RET_SUSPENDED); /* whatever the failure was, we need to retry */
	}

	/* create the message based on format specified */
	iLenWritten = write(pData->fd, ppString[0], strlen((char*)ppString[0]));
	if(iLenWritten < 0) {
		const int e = errno;
		/* If a named pipe is full, we suspend this action for a while */
		if(e == EAGAIN)
			ABORT_FINALIZE(RS_RET_SUSPENDED);

		close(pData->fd);
		pData->fd = -1; /* tell that fd is no longer open! */
		iRet = RS_RET_SUSPENDED;
		LogError(e, NO_ERRCODE, "write error on pipe %s", pData->pipe);
	}

finalize_it:
	RETiRet;
}


BEGINbeginCnfLoad
CODESTARTbeginCnfLoad
	loadModConf = pModConf;
	pModConf->pConf = pConf;
	pModConf->tplName = NULL;
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
		dbgprintf("module (global) param blk for ompipe:\n");
		cnfparamsPrint(&modpblk, pvals);
	}

	for(i = 0 ; i < modpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(modpblk.descr[i].name, "template")) {
			loadModConf->tplName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
			if(pszFileDfltTplName != NULL) {
				LogError(0, RS_RET_DUP_PARAM, "ompipe: warning: default template "
						"was already set via legacy directive - may lead to inconsistent "
						"results.");
			}
		} else {
			dbgprintf("ompipe: program error, non-handled "
			  "param '%s' in beginCnfLoad\n", modpblk.descr[i].name);
		}
	}
finalize_it:
	if(pvals != NULL)
		cnfparamvalsDestruct(pvals, &modpblk);
ENDsetModCnf

BEGINendCnfLoad
CODESTARTendCnfLoad
	loadModConf = NULL; /* done loading */
	/* free legacy config vars */
	free(pszFileDfltTplName);
	pszFileDfltTplName = NULL;
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
	free(pModConf->tplName);
ENDfreeCnf

BEGINcreateInstance
CODESTARTcreateInstance
	pData->pipe = NULL;
	pData->fd = -1;
	pData->bHadError = 0;
	pthread_mutex_init(&pData->mutWrite, NULL);
ENDcreateInstance


BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
ENDcreateWrkrInstance


BEGINfreeInstance
CODESTARTfreeInstance
	pthread_mutex_destroy(&pData->mutWrite);
	free(pData->pipe);
	if(pData->fd != -1)
		close(pData->fd);
ENDfreeInstance


BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance


BEGINtryResume
	instanceData *__restrict__ const pData = pWrkrData->pData;
	fd_set wrds;
	struct timeval tv;
	int ready;
CODESTARTtryResume
	if(pData->fd == -1) {
		rsRetVal iRetLocal;
		iRetLocal = preparePipe(pData);
		if((iRetLocal != RS_RET_OK) || (pData->fd == -1))
			ABORT_FINALIZE(RS_RET_SUSPENDED);
	} else {
		/* we can reach this if the pipe is full, so we need
		 * to check if we can write again. /dev/xconsole is the
		 * ugly example of why this is necessary.
		 */
		FD_ZERO(&wrds);
		FD_SET(pData->fd, &wrds);
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		ready = select(pData->fd+1, NULL, &wrds, NULL, &tv);
		DBGPRINTF("ompipe: tryResume: ready to write fd %d: %d\n", pData->fd, ready);
		if(ready != 1)
			ABORT_FINALIZE(RS_RET_SUSPENDED);
	}
finalize_it:
ENDtryResume

BEGINdoAction
	instanceData *pData;
CODESTARTdoAction
	pData = pWrkrData->pData;
	DBGPRINTF("ompipe: writing to %s\n", pData->pipe);
	/* this module is single-threaded by nature */
	pthread_mutex_lock(&pData->mutWrite);
	iRet = writePipe(ppString, pData);
	pthread_mutex_unlock(&pData->mutWrite);
ENDdoAction


static inline void
setInstParamDefaults(instanceData *pData)
{
	pData->tplName = NULL;
}

BEGINnewActInst
	struct cnfparamvals *pvals;
	int i;
CODESTARTnewActInst
	if((pvals = nvlstGetParams(lst, &actpblk, NULL)) == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	CHKiRet(createInstance(&pData));
	setInstParamDefaults(pData);

	CODE_STD_STRING_REQUESTnewActInst(1)
	for(i = 0 ; i < actpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(actpblk.descr[i].name, "pipe")) {
			pData->pipe = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "template")) {
			pData->tplName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else {
			dbgprintf("ompipe: program error, non-handled "
			  "param '%s'\n", actpblk.descr[i].name);
		}
	}

	CHKiRet(OMSRsetEntry(*ppOMSR, 0, (uchar*)strdup((pData->tplName == NULL) ?
						"RSYSLOG_FileFormat" : (char*)pData->tplName),
						OMSR_NO_RQD_TPL_OPTS));
CODE_STD_FINALIZERnewActInst
	cnfparamvalsDestruct(pvals, &actpblk);
ENDnewActInst

BEGINparseSelectorAct
CODESTARTparseSelectorAct
	/* yes, the if below is redundant, but I need it now. Will go away as
	 * the code further changes.  -- rgerhards, 2007-07-25
	 */
	if(*p == '|') {
		if((iRet = createInstance(&pData)) != RS_RET_OK) {
			ENDfunc
			return iRet; /* this can not use RET_iRet! */
		}
	} else {
		/* this is not clean, but we need it for the time being
		 * TODO: remove when cleaning up modularization
		 */
		ENDfunc
		return RS_RET_CONFLINE_UNPROCESSED;
	}

	CODE_STD_STRING_REQUESTparseSelectorAct(1)
	CHKmalloc(pData->pipe = malloc(512));
	++p;
	CHKiRet(cflineParseFileName(p, (uchar*) pData->pipe, *ppOMSR, 0, OMSR_NO_RQD_TPL_OPTS,
				       getDfltTpl()));

CODE_STD_FINALIZERparseSelectorAct
ENDparseSelectorAct


BEGINdoHUP
CODESTARTdoHUP
	if(pData->fd != -1) {
		close(pData->fd);
		pData->fd = -1;
	}
ENDdoHUP


BEGINmodExit
CODESTARTmodExit
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_OMOD_QUERIES
CODEqueryEtryPt_STD_OMOD8_QUERIES
CODEqueryEtryPt_doHUP
CODEqueryEtryPt_STD_CONF2_QUERIES
CODEqueryEtryPt_STD_CONF2_CNFNAME_QUERIES
CODEqueryEtryPt_STD_CONF2_setModCnf_QUERIES
CODEqueryEtryPt_STD_CONF2_OMOD_QUERIES
ENDqueryEtryPt


BEGINmodInit(Pipe)
CODESTARTmodInit
INITLegCnfVars
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
ENDmodInit
/* vi:set ai:
 */
