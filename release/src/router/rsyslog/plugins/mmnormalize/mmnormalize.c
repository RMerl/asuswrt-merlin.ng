/* mmnormalize.c
 * This is a message modification module. It normalizes the input message with
 * the help of liblognorm. The message's JSON variables are updated.
 *
 * NOTE: read comments in module-template.h for details on the calling interface!
 *
 * File begun on 2010-01-01 by RGerhards
 *
 * Copyright 2010-2015 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of rsyslog.
 *
 * Rsyslog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rsyslog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rsyslog.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
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
#include <libestr.h>
#include <json.h>
#include <liblognorm.h>
#include "conf.h"
#include "syslogd-types.h"
#include "template.h"
#include "module-template.h"
#include "errmsg.h"
#include "cfsysline.h"
#include "dirty.h"
#include "unicode-helper.h"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("mmnormalize")

static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal);

/* static data */

/* internal structures
 */
DEF_OMOD_STATIC_DATA

static struct cnfparamdescr modpdescr[] = {
	{ "allowregex", eCmdHdlrBinary, 0 }
};

static struct cnfparamblk modpblk = {
	CNFPARAMBLK_VERSION,
	sizeof(modpdescr)/sizeof(struct cnfparamdescr),
	modpdescr
};

typedef struct _instanceData {
	sbool bUseRawMsg;	/**< use %rawmsg% instead of %msg% */
	uchar   *rule;		/* rule to use */
	uchar 	*rulebase;	/**< name of rulebase to use */
	ln_ctx ctxln;		/**< context to be used for liblognorm */
	char *pszPath;		/**< path of normalized data */
	msgPropDescr_t *varDescr;/**< name of variable to use */
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
} wrkrInstanceData_t;

typedef struct configSettings_s {
	uchar *rulebase;		/**< name of normalization rulebase to use */
	uchar *rule;
	int bUseRawMsg;	/**< use %rawmsg% instead of %msg% */
} configSettings_t;
static configSettings_t cs;

/* tables for interfacing with the v6 config system */
/* action (instance) parameters */
static struct cnfparamdescr actpdescr[] = {
	{ "rulebase", eCmdHdlrGetWord, 0 },
	{ "rule", eCmdHdlrArray, 0 },
	{ "path", eCmdHdlrGetWord, 0 },
	{ "userawmsg", eCmdHdlrBinary, 0 },
	{ "variable", eCmdHdlrGetWord, 0 }
};
static struct cnfparamblk actpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(actpdescr)/sizeof(struct cnfparamdescr),
	  actpdescr
	};

struct modConfData_s {
	rsconf_t *pConf;	/* our overall config object */
	int allow_regex;
};

static modConfData_t *loadModConf = NULL;/* modConf ptr to use for the current load process */
static modConfData_t *runModConf = NULL;/* modConf ptr to use for the current exec process */

/* callback for liblognorm error messages */
static void
errCallBack(void __attribute__((unused)) *cookie, const char *msg,
	    size_t __attribute__((unused)) lenMsg)
{
	LogError(0, RS_RET_ERR_LIBLOGNORM, "liblognorm error: %s", msg);
}

/* to be called to build the liblognorm part of the instance ONCE ALL PARAMETERS ARE CORRECT
 * (and set within pData!).
 */
static rsRetVal
buildInstance(instanceData *pData)
{
	DEFiRet;
	if((pData->ctxln = ln_initCtx()) == NULL) {
		LogError(0, RS_RET_ERR_LIBLOGNORM_INIT, "error: could not initialize "
				"liblognorm ctx, cannot activate action");
		ABORT_FINALIZE(RS_RET_ERR_LIBLOGNORM_INIT);
	}
	ln_setCtxOpts(pData->ctxln, loadModConf->allow_regex);
	ln_setErrMsgCB(pData->ctxln, errCallBack, NULL);
	if(pData->rule !=NULL && pData->rulebase == NULL) {
		if(ln_loadSamplesFromString(pData->ctxln, (char*) pData->rule) !=0) {
			LogError(0, RS_RET_NO_RULEBASE, "error: normalization rule '%s' "
					"could not be loaded cannot activate action", pData->rule);
			ln_exitCtx(pData->ctxln);
			ABORT_FINALIZE(RS_RET_ERR_LIBLOGNORM_SAMPDB_LOAD);
		}
		free(pData->rule);
		pData->rule = NULL;
	} else if(pData->rule ==NULL && pData->rulebase != NULL) {
		if(ln_loadSamples(pData->ctxln, (char*) pData->rulebase) != 0) {
			LogError(0, RS_RET_NO_RULEBASE, "error: normalization rulebase '%s' "
					"could not be loaded cannot activate action", pData->rulebase);
			ln_exitCtx(pData->ctxln);
			ABORT_FINALIZE(RS_RET_ERR_LIBLOGNORM_SAMPDB_LOAD);
		}
	}
finalize_it:
	RETiRet;
}


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
	/* free legacy config vars */
	free(cs.rulebase);
	free(cs.rule);
	cs.rulebase = NULL;
	cs.rule = NULL;
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
ENDisCompatibleWithFeature


BEGINfreeInstance
CODESTARTfreeInstance
	free(pData->rulebase);
	free(pData->rule);
	ln_exitCtx(pData->ctxln);
	free(pData->pszPath);
	msgPropDescrDestruct(pData->varDescr);
	free(pData->varDescr);
ENDfreeInstance


BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance


BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
	dbgprintf("mmnormalize\n");
	dbgprintf("\tvariable='%s'\n", pData->varDescr->name);
	dbgprintf("\trulebase='%s'\n", pData->rulebase);
	dbgprintf("\trule='%s'\n", pData->rule);
	dbgprintf("\tpath='%s'\n", pData->pszPath);
	dbgprintf("\tbUseRawMsg='%d'\n", pData->bUseRawMsg);
ENDdbgPrintInstInfo


BEGINtryResume
CODESTARTtryResume
ENDtryResume

BEGINdoAction_NoStrings
	smsg_t **ppMsg = (smsg_t **) pMsgData;
	smsg_t *pMsg = ppMsg[0];
	uchar *buf;
	rs_size_t len;
	int r;
	struct json_object *json = NULL;
	unsigned short freeBuf = 0;
CODESTARTdoAction
	if(pWrkrData->pData->bUseRawMsg) {
		getRawMsg(pMsg, &buf, &len);
	} else if (pWrkrData->pData->varDescr) {
		buf = MsgGetProp(pMsg, NULL, pWrkrData->pData->varDescr, &len, &freeBuf, NULL);
	} else {
		buf = getMSG(pMsg);
		len = getMSGLen(pMsg);
	}
	r = ln_normalize(pWrkrData->pData->ctxln, (char*)buf, len, &json);
	if (freeBuf) {
		free(buf);
		buf = NULL;
	}
	if(r != 0) {
		DBGPRINTF("error %d during ln_normalize\n", r);
		MsgSetParseSuccess(pMsg, 0);
	} else {
		MsgSetParseSuccess(pMsg, 1);
	}

	msgAddJSON(pMsg, (uchar*)pWrkrData->pData->pszPath + 1, json, 0, 0);

ENDdoAction


static void
setInstParamDefaults(instanceData *pData)
{
	pData->rulebase = NULL;
	pData->rule = NULL;
	pData->bUseRawMsg = 0;
	pData->pszPath = strdup("$!");
	pData->varDescr = NULL;
}

BEGINsetModCnf
	struct cnfparamvals *pvals = NULL;
	int i;
CODESTARTsetModCnf
	pvals = nvlstGetParams(lst, &modpblk, NULL);
	if(pvals == NULL) {
		LogError(0, RS_RET_MISSING_CNFPARAMS, "mmnormalize: error processing module "
						"config parameters missing [module(...)]");
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}
	
	if(Debug) {
		dbgprintf("module (global) param blk for mmnormalize:\n");
		cnfparamsPrint(&modpblk, pvals);
	}
	
	for(i = 0 ; i < modpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(modpblk.descr[i].name, "allowregex")) {
			loadModConf->allow_regex = (int) pvals[i].val.d.n;
		} else {
			dbgprintf("mmnormalize: program error, non-handled "
					  "param '%s' in setModCnf\n", modpblk.descr[i].name);
		}
	}
	
finalize_it:
	if(pvals != NULL)
		cnfparamvalsDestruct(pvals, &modpblk);
ENDsetModCnf


BEGINnewActInst
	struct cnfparamvals *pvals;
	int i;
	int bDestructPValsOnExit;
	char *cstr;
	char *varName = NULL;
	char *buffer;
	char *tStr;
	int size = 0;
CODESTARTnewActInst
	DBGPRINTF("newActInst (mmnormalize)\n");

	bDestructPValsOnExit = 0;
	pvals = nvlstGetParams(lst, &actpblk, NULL);
	if(pvals == NULL) {
		LogError(0, RS_RET_MISSING_CNFPARAMS, "mmnormalize: error reading "
				"config parameters");
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}
	bDestructPValsOnExit = 1;

	if(Debug) {
		dbgprintf("action param blk in mmnormalize:\n");
		cnfparamsPrint(&actpblk, pvals);
	}

	CHKiRet(createInstance(&pData));
	setInstParamDefaults(pData);

	for(i = 0 ; i < actpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(actpblk.descr[i].name, "rulebase")) {
			pData->rulebase = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "rule")) {
			for(int j=0; j < pvals[i].val.d.ar->nmemb; ++j) {
				tStr = (char*)es_str2cstr(pvals[i].val.d.ar->arr[j], NULL);
				size += strlen(tStr);
				free(tStr);
			}
			buffer = malloc(size + pvals[i].val.d.ar->nmemb + 1);
			tStr = (char*)es_str2cstr(pvals[i].val.d.ar->arr[0], NULL);
			strcpy(buffer, tStr);
			free(tStr);
			strcat(buffer, "\n");
			for(int j=1; j < pvals[i].val.d.ar->nmemb; ++j) {
				tStr = (char*)es_str2cstr(pvals[i].val.d.ar->arr[j], NULL);
				strcat(buffer, tStr);
				free(tStr);
				strcat(buffer, "\n");
			}
			strcat(buffer, "\0");
			pData->rule = (uchar*)buffer;
		} else if(!strcmp(actpblk.descr[i].name, "userawmsg")) {
			pData->bUseRawMsg = (int) pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "variable")) {
			varName = es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "path")) {
			cstr = es_str2cstr(pvals[i].val.d.estr, NULL);
			if (strlen(cstr) < 2) {
				LogError(0, RS_RET_VALUE_NOT_SUPPORTED,
						"mmnormalize: valid path name should be at least "
						"2 symbols long, got %s",	cstr);
				free(cstr);
			} else if (cstr[0] != '$') {
				LogError(0, RS_RET_VALUE_NOT_SUPPORTED,
						"mmnormalize: valid path name should start with $,"
						"got %s", cstr);
				free(cstr);
			} else {
				free(pData->pszPath);
				pData->pszPath = cstr;
			}
			continue;
		} else {
			DBGPRINTF("mmnormalize: program error, non-handled "
			  "param '%s'\n", actpblk.descr[i].name);
		}
	}

	if (varName) {
		if(pData->bUseRawMsg) {
			LogError(0, RS_RET_CONFIG_ERROR,
			                "mmnormalize: 'variable' param can't be used with 'useRawMsg'. "
			                "Ignoring 'variable', will use raw message.");
		} else {
			CHKmalloc(pData->varDescr = MALLOC(sizeof(msgPropDescr_t)));
			CHKiRet(msgPropDescrFill(pData->varDescr, (uchar*) varName, strlen(varName)));
		}
		free(varName);
		varName = NULL;
	}
	if(!pData->rulebase) {
		if(!pData->rule) {
			LogError(0, RS_RET_CONFIG_ERROR, "mmnormalize: rulebase needed. "
					"Use option rulebase or rule.");
		}
	}
	if(pData->rulebase) {
		if(pData->rule) {
			LogError(0, RS_RET_CONFIG_ERROR,
					"mmnormalize: only one rulebase possible, rulebase "
					"can't be used with rule");
		}
	}

	CODE_STD_STRING_REQUESTnewActInst(1)
	CHKiRet(OMSRsetEntry(*ppOMSR, 0, NULL, OMSR_TPL_AS_MSG));
	iRet = buildInstance(pData);
CODE_STD_FINALIZERnewActInst
	if(bDestructPValsOnExit)
		cnfparamvalsDestruct(pvals, &actpblk);
ENDnewActInst


BEGINparseSelectorAct
CODESTARTparseSelectorAct
CODE_STD_STRING_REQUESTparseSelectorAct(1)
	/* first check if this config line is actually for us */
	if(strncmp((char*) p, ":mmnormalize:", sizeof(":mmnormalize:") - 1)) {
		ABORT_FINALIZE(RS_RET_CONFLINE_UNPROCESSED);
	}

	if(cs.rulebase == NULL && cs.rule == NULL) {
		LogError(0, RS_RET_NO_RULEBASE, "error: no normalization rulebase was specified, use "
				"$MMNormalizeSampleDB directive first!");
		ABORT_FINALIZE(RS_RET_NO_RULEBASE);
	}

	/* ok, if we reach this point, we have something for us */
	p += sizeof(":mmnormalize:") - 1; /* eat indicator sequence  (-1 because of '\0'!) */
	CHKiRet(createInstance(&pData));

	pData->rulebase = cs.rulebase;
	pData->rule = cs.rule;
	pData->bUseRawMsg = cs.bUseRawMsg;
	pData->pszPath = strdup("$!"); /* old interface does not support this feature */
	/* all config vars auto-reset! */
	cs.bUseRawMsg = 0;
	cs.rulebase = NULL; /* we used it up! */
	cs.rule = NULL;

	/* check if a non-standard template is to be applied */
	if(*(p-1) == ';')
		--p;
	/* we call the function below because we need to call it via our interface definition. However,
	 * the format specified (if any) is always ignored.
	 */
	CHKiRet(cflineParseTemplateName(&p, *ppOMSR, 0, OMSR_TPL_AS_MSG, (uchar*) "RSYSLOG_FileFormat"));
	CHKiRet(buildInstance(pData));
CODE_STD_FINALIZERparseSelectorAct
ENDparseSelectorAct


BEGINmodExit
CODESTARTmodExit
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_OMOD_QUERIES
CODEqueryEtryPt_STD_OMOD8_QUERIES
CODEqueryEtryPt_STD_CONF2_QUERIES
CODEqueryEtryPt_STD_CONF2_setModCnf_QUERIES
CODEqueryEtryPt_STD_CONF2_OMOD_QUERIES
ENDqueryEtryPt



/* Reset config variables for this module to default values.
 */
static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal)
{
	DEFiRet;
	cs.rulebase = NULL;
	cs.rule = NULL;
	cs.bUseRawMsg = 0;
	RETiRet;
}

/* set the rulebase name */
static rsRetVal
setRuleBase(void __attribute__((unused)) *pVal, uchar *pszName)
{
	DEFiRet;
	cs.rulebase = pszName;
	pszName = NULL;
	RETiRet;
}

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
	DBGPRINTF("mmnormalize: module compiled with rsyslog version %s.\n", VERSION);
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
		DBGPRINTF("mmnormalize: msg-passing is not supported by rsyslog core, "
			  "can not continue.\n");
		ABORT_FINALIZE(RS_RET_NO_MSG_PASSING);
	}

	CHKiRet(omsdRegCFSLineHdlr((uchar *)"mmnormalizerulebase", 0, eCmdHdlrGetWord,
				    setRuleBase, NULL, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"mmnormalizerule", 0, eCmdHdlrGetWord, NULL,
				NULL, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"mmnormalizeuserawmsg", 0, eCmdHdlrBinary,
				NULL, &cs.bUseRawMsg, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler,
				    resetConfigVariables, NULL, STD_LOADABLE_MODULE_ID));
ENDmodInit

/* vi:set ai:
 */
