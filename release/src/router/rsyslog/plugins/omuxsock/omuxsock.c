/* omuxsock.c
 * This is the implementation of datgram unix domain socket forwarding.
 *
 * NOTE: read comments in module-template.h to understand how this file
 *       works!
 *
 * Copyright 2010-2016 Adiscon GmbH.
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
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include "conf.h"
#include "srUtils.h"
#include "template.h"
#include "msg.h"
#include "cfsysline.h"
#include "module-template.h"
#include "glbl.h"
#include "errmsg.h"
#include "unicode-helper.h"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("omuxsock")

/* internal structures
 */
DEF_OMOD_STATIC_DATA
DEFobjCurrIf(glbl)

#define INVLD_SOCK -1

typedef struct _instanceData {
	permittedPeers_t *pPermPeers;
	uchar *sockName;
	int sock;
	struct sockaddr_un addr;
} instanceData;


typedef struct wrkrInstanceData {
	instanceData *pData;
} wrkrInstanceData_t;

/* config data */
typedef struct configSettings_s {
	uchar *tplName; /* name of the default template to use */
	uchar *sockName; /* name of the default template to use */
} configSettings_t;
static configSettings_t cs;

/* module-global parameters */
static struct cnfparamdescr modpdescr[] = {
	{ "template", eCmdHdlrGetWord, 0 },
};
static struct cnfparamblk modpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(modpdescr)/sizeof(struct cnfparamdescr),
	  modpdescr
	};

struct modConfData_s {
	rsconf_t *pConf;	/* our overall config object */
	uchar 	*tplName;	/* default template */
};

static modConfData_t *loadModConf = NULL;/* modConf ptr to use for the current load process */
static modConfData_t *runModConf = NULL;/* modConf ptr to use for the current exec process */


static pthread_mutex_t mutDoAct = PTHREAD_MUTEX_INITIALIZER;

BEGINinitConfVars		/* (re)set config variables to default values */
CODESTARTinitConfVars
	cs.tplName = NULL;
	cs.sockName = NULL;
ENDinitConfVars


static rsRetVal doTryResume(instanceData *pData);


/* this function gets the default template. It coordinates action between
 * old-style and new-style configuration parts.
 */
static uchar*
getDfltTpl(void)
{
	if(loadModConf != NULL && loadModConf->tplName != NULL)
		return loadModConf->tplName;
	else if(cs.tplName == NULL)
		return (uchar*)"RSYSLOG_TraditionalForwardFormat";
	else
		return cs.tplName;
}

/* set the default template to be used
 * This is a module-global parameter, and as such needs special handling. It needs to
 * be coordinated with values set via the v2 config system (rsyslog v6+). What we do
 * is we do not permit this directive after the v2 config system has been used to set
 * the parameter.
 */
static rsRetVal
setLegacyDfltTpl(void __attribute__((unused)) *pVal, uchar* newVal)
{
	DEFiRet;

	if(loadModConf != NULL && loadModConf->tplName != NULL) {
		free(newVal);
		LogError(0, RS_RET_ERR, "omuxsock default template already set via module "
			"global parameter - can no longer be changed");
		ABORT_FINALIZE(RS_RET_ERR);
	}
	free(cs.tplName);
	cs.tplName = newVal;
finalize_it:
	RETiRet;
}


static rsRetVal
closeSocket(instanceData *pData)
{
	DEFiRet;
	if(pData->sock != INVLD_SOCK) {
		close(pData->sock);
		pData->sock = INVLD_SOCK;
	}
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
		dbgprintf("module (global) param blk for omuxsock:\n");
		cnfparamsPrint(&modpblk, pvals);
	}

	for(i = 0 ; i < modpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(modpblk.descr[i].name, "template")) {
			loadModConf->tplName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
			if(cs.tplName != NULL) {
				LogError(0, RS_RET_DUP_PARAM, "omuxsock: default template "
						"was already set via legacy directive - may lead to inconsistent "
						"results.");
			}
		} else {
			dbgprintf("omuxsock: program error, non-handled "
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
	free(cs.tplName);
	cs.tplName = NULL;
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
	pData->sock = INVLD_SOCK;
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
	/* final cleanup */
	closeSocket(pData);
	free(pData->sockName);
ENDfreeInstance

BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance


BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
	DBGPRINTF("%s", pData->sockName);
ENDdbgPrintInstInfo


/* Send a message via UDP
 * rgehards, 2007-12-20
 */
static rsRetVal sendMsg(instanceData *pData, char *msg, size_t len)
{
	DEFiRet;
	unsigned lenSent = 0;

	if(pData->sock == INVLD_SOCK) {
		CHKiRet(doTryResume(pData));
	}

	if(pData->sock != INVLD_SOCK) {
		lenSent = sendto(pData->sock, msg, len, 0, (const struct sockaddr *)&pData->addr,
			sizeof(pData->addr));
		if(lenSent != len) {
			int eno = errno;
			char errStr[1024];
			DBGPRINTF("omuxsock suspending: sendto(), socket %d, error: %d = %s.\n",
				pData->sock, eno, rs_strerror_r(eno, errStr, sizeof(errStr)));
		}
	}

finalize_it:
	RETiRet;
}


/* open socket to remote system
 */
static rsRetVal
openSocket(instanceData *pData)
{
	DEFiRet;
	assert(pData->sock == INVLD_SOCK);

	if((pData->sock = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
		char errStr[1024];
		int eno = errno;
		DBGPRINTF("error %d creating AF_UNIX/SOCK_DGRAM: %s.\n",
			eno, rs_strerror_r(eno, errStr, sizeof(errStr)));
		pData->sock = INVLD_SOCK;
		ABORT_FINALIZE(RS_RET_NO_SOCKET);

	}

	/* set up server address structure */
	memset(&pData->addr, 0, sizeof(pData->addr));
	pData->addr.sun_family = AF_UNIX;
	strncpy(pData->addr.sun_path, (char*)pData->sockName, sizeof(pData->addr.sun_path));
	pData->addr.sun_path[sizeof(pData->addr.sun_path)-1] = '\0';

finalize_it:
	if(iRet != RS_RET_OK) {
		closeSocket(pData);
	}
	RETiRet;
}



/* try to resume connection if it is not ready
 */
static rsRetVal doTryResume(instanceData *pData)
{
	DEFiRet;

	DBGPRINTF("omuxsock trying to resume\n");
	closeSocket(pData);
	iRet = openSocket(pData);

	if(iRet != RS_RET_OK) {
		iRet = RS_RET_SUSPENDED;
	}

	RETiRet;
}


BEGINtryResume
CODESTARTtryResume
	iRet = doTryResume(pWrkrData->pData);
ENDtryResume

BEGINdoAction
	char *psz = NULL; /* temporary buffering */
	register unsigned l;
	int iMaxLine;
CODESTARTdoAction
	pthread_mutex_lock(&mutDoAct);
	CHKiRet(doTryResume(pWrkrData->pData));

	iMaxLine = glbl.GetMaxLine();

	DBGPRINTF(" omuxsock:%s\n", pWrkrData->pData->sockName);

	psz = (char*) ppString[0];
	l = strlen((char*) psz);
	if((int) l > iMaxLine)
		l = iMaxLine;

	CHKiRet(sendMsg(pWrkrData->pData, psz, l));

finalize_it:
	pthread_mutex_unlock(&mutDoAct);
ENDdoAction


BEGINparseSelectorAct
CODESTARTparseSelectorAct
CODE_STD_STRING_REQUESTparseSelectorAct(1)

	/* first check if this config line is actually for us */
	if(strncmp((char*) p, ":omuxsock:", sizeof(":omuxsock:") - 1)) {
		ABORT_FINALIZE(RS_RET_CONFLINE_UNPROCESSED);
	}

	/* ok, if we reach this point, we have something for us */
	p += sizeof(":omuxsock:") - 1; /* eat indicator sequence  (-1 because of '\0'!) */
	CHKiRet(createInstance(&pData));

	/* check if a non-standard template is to be applied */
	if(*(p-1) == ';')
		--p;
	CHKiRet(cflineParseTemplateName(&p, *ppOMSR, 0, 0, getDfltTpl()));
	
	if(cs.sockName == NULL) {
		LogError(0, RS_RET_NO_SOCK_CONFIGURED, "No output socket configured for omuxsock\n");
		ABORT_FINALIZE(RS_RET_NO_SOCK_CONFIGURED);
	}

	pData->sockName = cs.sockName;
	cs.sockName = NULL; /* pData is now owner and will fee it */

CODE_STD_FINALIZERparseSelectorAct
ENDparseSelectorAct


/* a common function to free our configuration variables - used both on exit
 * and on $ResetConfig processing. -- rgerhards, 2008-05-16
 */
static void
freeConfigVars(void)
{
	free(cs.tplName);
	cs.tplName = NULL;
	free(cs.sockName);
	cs.sockName = NULL;
}


BEGINmodExit
CODESTARTmodExit
	/* release what we no longer need */
	objRelease(glbl, CORE_COMPONENT);

	freeConfigVars();
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_OMOD_QUERIES
CODEqueryEtryPt_STD_OMOD8_QUERIES
CODEqueryEtryPt_STD_CONF2_QUERIES
CODEqueryEtryPt_STD_CONF2_setModCnf_QUERIES
ENDqueryEtryPt


/* Reset config variables for this module to default values.
 * rgerhards, 2008-03-28
 */
static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal)
{
	freeConfigVars();
	return RS_RET_OK;
}


BEGINmodInit()
CODESTARTmodInit
INITLegCnfVars
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	CHKiRet(objUse(glbl, CORE_COMPONENT));

	CHKiRet(regCfSysLineHdlr((uchar *)"omuxsockdefaulttemplate", 0, eCmdHdlrGetWord, setLegacyDfltTpl,
		NULL, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"omuxsocksocket", 0, eCmdHdlrGetWord, NULL, &cs.sockName, NULL));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler, resetConfigVariables,
	NULL, STD_LOADABLE_MODULE_ID));
ENDmodInit

/* vim:set ai:
 */
