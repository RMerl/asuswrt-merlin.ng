/* module-template.h
 * This header contains macros that can be used to implement the
 * plumbing of modules.
 *
 * File begun on 2007-07-25 by RGerhards
 *
 * Copyright 2007-2015 Adiscon GmbH.
 *
 * This file is part of the rsyslog runtime library.
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
#ifndef	MODULE_TEMPLATE_H_INCLUDED
#define	MODULE_TEMPLATE_H_INCLUDED 1

#include "modules.h"
#include "obj.h"
#include "objomsr.h"
#include "threads.h"

/* macro to define standard output-module static data members
 */
#define DEF_MOD_STATIC_DATA \
	static __attribute__((unused)) rsRetVal (*omsdRegCFSLineHdlr)(uchar *pCmdName, int bChainingPermitted, \
	ecslCmdHdrlType eType, rsRetVal (*pHdlr)(), void *pData, void *pOwnerCookie);

#define DEF_OMOD_STATIC_DATA \
	DEF_MOD_STATIC_DATA \
	DEFobjCurrIf(obj) \
	static __attribute__((unused)) int bCoreSupportsBatching;
#define DEF_IMOD_STATIC_DATA \
	DEF_MOD_STATIC_DATA \
	DEFobjCurrIf(obj)
#define DEF_LMOD_STATIC_DATA \
	DEF_MOD_STATIC_DATA
#define DEF_PMOD_STATIC_DATA \
	DEFobjCurrIf(obj) \
	DEF_MOD_STATIC_DATA
#define DEF_SMOD_STATIC_DATA \
	DEFobjCurrIf(obj) \
	DEF_MOD_STATIC_DATA
#define DEF_FMOD_STATIC_DATA \
	DEFobjCurrIf(obj) \
	DEF_MOD_STATIC_DATA


/* Macro to define the module type. Each module can only have a single type. If
 * a module provides multiple types, several separate modules must be created which
 * then should share a single library containing the majority of code. This macro
 * must be present in each module. -- rgerhards, 2007-12-14
 * Note that MODULE_TYPE_TESTBENCH is reserved for testbenches, but
 * declared in their own header files (because the rest does not need these
 * defines). -- rgerhards, 2008-06-13
 */
#define MODULE_TYPE(x)\
static rsRetVal modGetType(eModType_t *modType) \
	{ \
		*modType = x; \
		return RS_RET_OK;\
	}

#define MODULE_TYPE_INPUT MODULE_TYPE(eMOD_IN)
#define MODULE_TYPE_OUTPUT MODULE_TYPE(eMOD_OUT)
#define MODULE_TYPE_PARSER MODULE_TYPE(eMOD_PARSER)
#define MODULE_TYPE_STRGEN MODULE_TYPE(eMOD_STRGEN)
#define MODULE_TYPE_FUNCTION MODULE_TYPE(eMOD_FUNCTION)
#define MODULE_TYPE_LIB \
	DEF_LMOD_STATIC_DATA \
	MODULE_TYPE(eMOD_LIB)

/* Macro to define whether the module should be kept dynamically linked.
 */
#define MODULE_KEEP_TYPE(x)\
static rsRetVal modGetKeepType(eModKeepType_t *modKeepType) \
	{ \
		*modKeepType = x; \
		return RS_RET_OK;\
	}
#define MODULE_TYPE_NOKEEP MODULE_KEEP_TYPE(eMOD_NOKEEP)
#define MODULE_TYPE_KEEP MODULE_KEEP_TYPE(eMOD_KEEP)

/* macro to define a unique module id. This must be able to fit in a void*. The
 * module id must be unique inside a running rsyslogd application. It is used to
 * track ownership of several objects. Most importantly, when the module is
 * unloaded the module id value is used to find what needs to be destroyed.
 * We currently use a pointer to modExit() as the module id. This sounds to be
 * reasonable save, as each module must have this entry point AND there is no valid
 * reason for twice this entry point being in memory.
 * rgerhards, 2007-11-21
 */
#define STD_LOADABLE_MODULE_ID ((void*) modExit)


/* macro to implement the "modGetID()" interface function
 * rgerhards 2007-11-21
 */
#define DEFmodGetID \
static rsRetVal modGetID(void **pID) \
	{ \
		*pID = STD_LOADABLE_MODULE_ID;\
		return RS_RET_OK;\
	}

/* macro to provide the v6 config system module name
 */
#define MODULE_CNFNAME(name) \
static rsRetVal modGetCnfName(uchar **cnfName) \
	{ \
		*cnfName = (uchar*) name; \
		return RS_RET_OK;\
	}


/* to following macros are used to generate function headers and standard
 * functionality. It works as follows (described on the sample case of
 * createInstance()):
 *
 * BEGINcreateInstance
 * ... custom variable definitions (on stack) ... (if any)
 * CODESTARTcreateInstance
 * ... custom code ... (if any)
 * ENDcreateInstance
 */

/* createInstance()
 */
#define BEGINcreateInstance \
static rsRetVal createInstance(instanceData **ppData)\
	{\
	DEFiRet; /* store error code here */\
	instanceData *pData; /* use this to point to data elements */

#define CODESTARTcreateInstance \
	if((pData = calloc(1, sizeof(instanceData))) == NULL) {\
		*ppData = NULL;\
		ENDfunc \
		return RS_RET_OUT_OF_MEMORY;\
	}

#define ENDcreateInstance \
	*ppData = pData;\
	RETiRet;\
}

/* freeInstance()
 * This is the cleanup function for the module instance. It is called immediately before
 * the module instance is destroyed (unloaded). The module should do any cleanup
 * here, e.g. close file, free instantance heap memory and the like. Control will
 * not be passed back to the module once this function is finished. Keep in mind,
 * however, that other instances may still be loaded and used. So do not destroy
 * anything that may be used by another instance. If you have such a ressource, you
 * currently need to do the instance counting yourself.
 */
#define BEGINfreeInstance \
static rsRetVal freeInstance(void* pModData)\
{\
	DEFiRet;\
	instanceData *pData;

#define CODESTARTfreeInstance \
	pData = (instanceData*) pModData;

#define ENDfreeInstance \
	if(pData != NULL)\
		free(pData); /* we need to free this in any case */\
	RETiRet;\
}

/* createWrkrInstance()
 */
#define BEGINcreateWrkrInstance \
static rsRetVal createWrkrInstance(wrkrInstanceData_t **ppWrkrData, instanceData *pData)\
	{\
	DEFiRet; /* store error code here */\
	wrkrInstanceData_t *pWrkrData; /* use this to point to data elements */

#define CODESTARTcreateWrkrInstance \
	if((pWrkrData = calloc(1, sizeof(wrkrInstanceData_t))) == NULL) {\
		*ppWrkrData = NULL;\
		ENDfunc \
		return RS_RET_OUT_OF_MEMORY;\
	} \
	pWrkrData->pData = pData;

#define ENDcreateWrkrInstance \
	*ppWrkrData = pWrkrData;\
	RETiRet;\
}

/* freeWrkrInstance */
#define BEGINfreeWrkrInstance \
static rsRetVal freeWrkrInstance(void* pd)\
{\
	DEFiRet;\
	wrkrInstanceData_t *pWrkrData;

#define CODESTARTfreeWrkrInstance \
	pWrkrData = (wrkrInstanceData_t*) pd;

#define ENDfreeWrkrInstance \
	if(pWrkrData != NULL)\
		free(pWrkrData); /* we need to free this in any case */\
	RETiRet;\
}


/* isCompatibleWithFeature()
 */
#define BEGINisCompatibleWithFeature \
static rsRetVal isCompatibleWithFeature(syslogFeature __attribute__((unused)) eFeat)\
{\
	rsRetVal iRet = RS_RET_INCOMPATIBLE; \
	BEGINfunc

#define CODESTARTisCompatibleWithFeature

#define ENDisCompatibleWithFeature \
	RETiRet;\
}


/* beginTransaction()
 * introduced in v4.3.3 -- rgerhards, 2009-04-27
 */
#define BEGINbeginTransaction \
static rsRetVal beginTransaction(wrkrInstanceData_t __attribute__((unused)) *pWrkrData)\
{\
	DEFiRet;

#define CODESTARTbeginTransaction /* currently empty, but may be extended */

#define ENDbeginTransaction \
	RETiRet;\
}


/* commitTransaction()
 * Commits a transaction. Note that beginTransaction() must have been
 * called before this entry point. It receives the full batch of messages
 * to be processed in pParam parameter.
 * introduced in v8.1.3 -- rgerhards, 2013-12-04
 */
#define BEGINcommitTransaction \
static rsRetVal commitTransaction(wrkrInstanceData_t __attribute__((unused)) *const pWrkrData, \
	actWrkrIParams_t *const pParams, const unsigned nParams)\
{\
	DEFiRet;

#define CODESTARTcommitTransaction /* currently empty, but may be extended */

#define ENDcommitTransaction \
	RETiRet;\
}

/* endTransaction()
 * introduced in v4.3.3 -- rgerhards, 2009-04-27
 */
#define BEGINendTransaction \
static rsRetVal endTransaction(wrkrInstanceData_t __attribute__((unused)) *pWrkrData)\
{\
	DEFiRet;

#define CODESTARTendTransaction /* currently empty, but may be extended */

#define ENDendTransaction \
	RETiRet;\
}


/* doAction()
 */
#define BEGINdoAction \
static rsRetVal doAction(void * pMsgData, wrkrInstanceData_t __attribute__((unused)) *pWrkrData)\
{\
	uchar **ppString = (uchar **) pMsgData; \
	DEFiRet;

#define CODESTARTdoAction \
	/* ppString may be NULL if the output module requested no strings */

#define ENDdoAction \
	RETiRet;\
}

/* below is a variant of doAction where the passed-in data is not the common
 * case of string.
 */
#define BEGINdoAction_NoStrings \
static rsRetVal doAction(void * pMsgData, wrkrInstanceData_t __attribute__((unused)) *pWrkrData)\
{\
	DEFiRet;


/* dbgPrintInstInfo()
 * Extra comments:
 * Print debug information about this instance.
 */
#define BEGINdbgPrintInstInfo \
static rsRetVal dbgPrintInstInfo(void *pModData)\
{\
	DEFiRet;\
	instanceData *pData = NULL;

#define CODESTARTdbgPrintInstInfo \
	pData = (instanceData*) pModData; \
	(void)pData; /* prevent compiler warning if unused! */

#define ENDdbgPrintInstInfo \
	RETiRet;\
}


/* parseSelectorAct()
 * Extra comments:
 * try to process a selector action line. Checks if the action
 * applies to this module and, if so, processed it. If not, it
 * is left untouched. The driver will then call another module.
 * On exit, ppModData must point to instance data. Also, a string
 * request object must be created and filled. A macro is defined
 * for that.
 * For the most usual case, we have defined a macro below.
 * If more than one string is requested, the macro can be used together
 * with own code that overwrites the entry count. In this case, the
 * macro must come before the own code. It is recommended to be
 * placed right after CODESTARTparseSelectorAct.
 */
#define BEGINparseSelectorAct \
static rsRetVal parseSelectorAct(uchar **pp, void **ppModData, omodStringRequest_t **ppOMSR)\
{\
	DEFiRet;\
	uchar *p;\
	instanceData *pData = NULL;

#define CODESTARTparseSelectorAct \
	assert(pp != NULL);\
	assert(ppModData != NULL);\
	assert(ppOMSR != NULL);\
	p = *pp;

#define CODE_STD_STRING_REQUESTparseSelectorAct(NumStrReqEntries) \
	CHKiRet(OMSRconstruct(ppOMSR, NumStrReqEntries));

#define CODE_STD_FINALIZERparseSelectorAct \
finalize_it: ATTR_UNUSED; /* semi-colon needed according to gcc doc! */\
	if(iRet == RS_RET_OK || iRet == RS_RET_OK_WARN || iRet == RS_RET_SUSPENDED) {\
		*ppModData = pData;\
		*pp = p;\
	} else {\
		/* cleanup, we failed */\
		if(*ppOMSR != NULL) {\
			OMSRdestruct(*ppOMSR);\
			*ppOMSR = NULL;\
		}\
		if(pData != NULL) {\
			freeInstance(pData);\
		} \
	}

#define ENDparseSelectorAct \
	RETiRet;\
}

/* a special replacement macro for modules that do not support legacy config at all */
#define NO_LEGACY_CONF_parseSelectorAct \
static rsRetVal parseSelectorAct(uchar **pp ATTR_UNUSED, void **ppModData ATTR_UNUSED, \
	omodStringRequest_t **ppOMSR ATTR_UNUSED)\
{\
	return RS_RET_LEGA_ACT_NOT_SUPPORTED;\
}

/* newActInst()
 * Extra comments:
 * This creates a new instance of a the action that implements the call.
 * This is part of the conf2 (rsyslog v6) config system. It is called by
 * the core when an action object has been obtained. The output module
 * must then verify parameters and create a new action instance (if
 * parameters are acceptable) or return an error code.
 * On exit, ppModData must point to instance data. Also, a string
 * request object must be created and filled. A macro is defined
 * for that.
 * For the most usual case, we have defined a macro below.
 * If more than one string is requested, the macro can be used together
 * with own code that overwrites the entry count. In this case, the
 * macro must come before the own code. It is recommended to be
 * placed right after CODESTARTnewActInst.
 */
#define BEGINnewActInst \
static rsRetVal newActInst(uchar __attribute__((unused)) *modName, \
	struct nvlst __attribute__((unused)) *lst, void **ppModData, \
	omodStringRequest_t **ppOMSR)\
{\
	DEFiRet;\
	instanceData *pData = NULL; \
	*ppOMSR = NULL;

#define CODESTARTnewActInst \

#define CODE_STD_STRING_REQUESTnewActInst(NumStrReqEntries) \
	CHKiRet(OMSRconstruct(ppOMSR, NumStrReqEntries));

#define CODE_STD_FINALIZERnewActInst \
finalize_it:\
	if(iRet == RS_RET_OK || iRet == RS_RET_SUSPENDED) {\
		*ppModData = pData;\
	} else {\
		/* cleanup, we failed */\
		if(*ppOMSR != NULL) {\
			OMSRdestruct(*ppOMSR);\
			*ppOMSR = NULL;\
		}\
		if(pData != NULL) {\
			freeInstance(pData);\
		} \
	}

#define ENDnewActInst \
	RETiRet;\
}


/* newInpInst()
 * This is basically the equivalent to newActInst() for creating input
 * module (listener) instances.
 */
#define BEGINnewInpInst \
static rsRetVal newInpInst(struct nvlst *lst)\
{\
	DEFiRet;

#define CODESTARTnewInpInst \

#define CODE_STD_FINALIZERnewInpInst

#define ENDnewInpInst \
	RETiRet;\
}



/* newParserInst()
 * This is basically the equivalent to newActInst() for creating parser
 * module (listener) instances.
 */
#define BEGINnewParserInst \
static rsRetVal newParserInst(struct nvlst *lst, void *pinst)\
{\
	instanceConf_t *inst; \
	DEFiRet;

#define CODESTARTnewParserInst \

#define CODE_STD_FINALIZERnewParserInst

#define ENDnewParserInst \
	if(iRet == RS_RET_OK) \
		*((instanceConf_t**)pinst) = inst; \
	RETiRet;\
}


/* freeParserInst */
#define BEGINfreeParserInst \
static rsRetVal freeParserInst(void* pi)\
{\
	DEFiRet;\
	instanceConf_t *pInst;

#define CODESTARTfreeParserInst\
	pInst = (instanceConf_t*) pi;

#define ENDfreeParserInst\
	if(pInst != NULL)\
		free(pInst);\
	RETiRet;\
}

/* tryResume()
 * This entry point is called to check if a module can resume operations. This
 * happens when a module requested that it be suspended. In suspended state,
 * the engine periodically tries to resume the module. If that succeeds, normal
 * processing continues. If not, the module will not be called unless a
 * tryResume() call succeeds.
 * Returns RS_RET_OK, if resumption succeeded, RS_RET_SUSPENDED otherwise
 * rgerhard, 2007-08-02
 */
#define BEGINtryResume \
static rsRetVal tryResume(wrkrInstanceData_t __attribute__((unused)) *pWrkrData)\
{\
	DEFiRet;

#define CODESTARTtryResume \
	assert(pWrkrData != NULL);

#define ENDtryResume \
	RETiRet;\
}


/* initConfVars() - initialize pre-v6.3-config variables
 */
#define BEGINinitConfVars \
static rsRetVal initConfVars(void)\
{\
	DEFiRet;

#define CODESTARTinitConfVars

#define ENDinitConfVars \
	RETiRet;\
}
	

/* queryEtryPt()
 */
#define BEGINqueryEtryPt \
DEFmodGetID \
static rsRetVal queryEtryPt(uchar *name, rsRetVal (**pEtryPoint)())\
{\
	DEFiRet;

#define CODESTARTqueryEtryPt \
	if((name == NULL) || (pEtryPoint == NULL)) {\
		ENDfunc \
		return RS_RET_PARAM_ERROR;\
	} \
	*pEtryPoint = NULL;

#define ENDqueryEtryPt \
	if(iRet == RS_RET_OK)\
		if(*pEtryPoint == NULL) { \
			dbgprintf("entry point '%s' not present in module\n", name); \
			iRet = RS_RET_MODULE_ENTRY_POINT_NOT_FOUND;\
		} \
	RETiRet;\
}

/* the following definition is the standard block for queryEtryPt for all types
 * of modules. It should be included in any module, and typically is so by calling
 * the module-type specific macros.
 */
#define CODEqueryEtryPt_STD_MOD_QUERIES \
	if(!strcmp((char*) name, "modExit")) {\
		*pEtryPoint = modExit;\
	} else if(!strcmp((char*) name, "modGetID")) {\
		*pEtryPoint = modGetID;\
	} else if(!strcmp((char*) name, "getType")) {\
		*pEtryPoint = modGetType;\
	} else if(!strcmp((char*) name, "getKeepType")) {\
		*pEtryPoint = modGetKeepType;\
	}

/* the following definition is the standard block for queryEtryPt for output
 * modules WHICH DO NOT SUPPORT TRANSACTIONS.
 */
#define CODEqueryEtryPt_STD_OMOD_QUERIES \
	CODEqueryEtryPt_STD_MOD_QUERIES \
	else if(!strcmp((char*) name, "doAction")) {\
		*pEtryPoint = doAction;\
	} else if(!strcmp((char*) name, "dbgPrintInstInfo")) {\
		*pEtryPoint = dbgPrintInstInfo;\
	} else if(!strcmp((char*) name, "freeInstance")) {\
		*pEtryPoint = freeInstance;\
	} else if(!strcmp((char*) name, "parseSelectorAct")) {\
		*pEtryPoint = parseSelectorAct;\
	} else if(!strcmp((char*) name, "isCompatibleWithFeature")) {\
		*pEtryPoint = isCompatibleWithFeature;\
	} else if(!strcmp((char*) name, "tryResume")) {\
		*pEtryPoint = tryResume;\
	}

/* the following definition is the standard block for queryEtryPt for output
 * modules using the transaction interface.
 */
#define CODEqueryEtryPt_STD_OMODTX_QUERIES \
	CODEqueryEtryPt_STD_MOD_QUERIES \
	else if(!strcmp((char*) name, "beginTransaction")) {\
		*pEtryPoint = beginTransaction;\
	} else if(!strcmp((char*) name, "commitTransaction")) {\
		*pEtryPoint = commitTransaction;\
	} else if(!strcmp((char*) name, "dbgPrintInstInfo")) {\
		*pEtryPoint = dbgPrintInstInfo;\
	} else if(!strcmp((char*) name, "freeInstance")) {\
		*pEtryPoint = freeInstance;\
	} else if(!strcmp((char*) name, "parseSelectorAct")) {\
		*pEtryPoint = parseSelectorAct;\
	} else if(!strcmp((char*) name, "isCompatibleWithFeature")) {\
		*pEtryPoint = isCompatibleWithFeature;\
	} else if(!strcmp((char*) name, "tryResume")) {\
		*pEtryPoint = tryResume;\
	}

/* standard queries for output module interface in rsyslog v8+ */
#define CODEqueryEtryPt_STD_OMOD8_QUERIES \
	else if(!strcmp((char*) name, "createWrkrInstance")) {\
		*pEtryPoint = createWrkrInstance;\
	} else if(!strcmp((char*) name, "freeWrkrInstance")) {\
		*pEtryPoint = freeWrkrInstance;\
	}

/* the following definition is queryEtryPt block that must be added
 * if an output module supports the transactional interface.
 * rgerhards, 2009-04-27
 */
#define CODEqueryEtryPt_TXIF_OMOD_QUERIES \
	  else if(!strcmp((char*) name, "beginTransaction")) {\
		*pEtryPoint = beginTransaction;\
	} else if(!strcmp((char*) name, "endTransaction")) {\
		*pEtryPoint = endTransaction;\
	}


/* the following definition is a queryEtryPt block that must be added
 * if a non-output module supports "isCompatibleWithFeature".
 * rgerhards, 2009-07-20
 */
#define CODEqueryEtryPt_IsCompatibleWithFeature_IF_OMOD_QUERIES \
	  else if(!strcmp((char*) name, "isCompatibleWithFeature")) {\
		*pEtryPoint = isCompatibleWithFeature;\
	}


/* the following definition is the standard block for queryEtryPt for INPUT
 * modules. This can be used if no specific handling (e.g. to cover version
 * differences) is needed.
 */
#define CODEqueryEtryPt_STD_IMOD_QUERIES \
	CODEqueryEtryPt_STD_MOD_QUERIES \
	else if(!strcmp((char*) name, "runInput")) {\
		*pEtryPoint = runInput;\
	} else if(!strcmp((char*) name, "willRun")) {\
		*pEtryPoint = willRun;\
	} else if(!strcmp((char*) name, "afterRun")) {\
		*pEtryPoint = afterRun;\
	}


/* the following block is to be added for modules that support the v2
 * config system. The config name is also provided.
 */
#define CODEqueryEtryPt_STD_CONF2_QUERIES \
	  else if(!strcmp((char*) name, "beginCnfLoad")) {\
		*pEtryPoint = beginCnfLoad;\
	} else if(!strcmp((char*) name, "endCnfLoad")) {\
		*pEtryPoint = endCnfLoad;\
	} else if(!strcmp((char*) name, "checkCnf")) {\
		*pEtryPoint = checkCnf;\
	} else if(!strcmp((char*) name, "activateCnf")) {\
		*pEtryPoint = activateCnf;\
	} else if(!strcmp((char*) name, "freeCnf")) {\
		*pEtryPoint = freeCnf;\
	} \
	CODEqueryEtryPt_STD_CONF2_CNFNAME_QUERIES

/* the following block is to be added for modules that support v2
 * module global parameters [module(...)]
 */
#define CODEqueryEtryPt_STD_CONF2_setModCnf_QUERIES \
	  else if(!strcmp((char*) name, "setModCnf")) {\
		*pEtryPoint = setModCnf;\
	} \

/* the following block is to be added for output modules that support the v2
 * config system. The config name is also provided.
 */
#define CODEqueryEtryPt_STD_CONF2_OMOD_QUERIES \
	  else if(!strcmp((char*) name, "newActInst")) {\
		*pEtryPoint = newActInst;\
	} \
	CODEqueryEtryPt_STD_CONF2_CNFNAME_QUERIES


/* the following block is to be added for input modules that support the v2
 * config system. The config name is also provided.
 */
#define CODEqueryEtryPt_STD_CONF2_IMOD_QUERIES \
	  else if(!strcmp((char*) name, "newInpInst")) {\
		*pEtryPoint = newInpInst;\
	} \
	CODEqueryEtryPt_STD_CONF2_CNFNAME_QUERIES


/* the following block is to be added for modules that require
 * pre priv drop activation support.
 */
#define CODEqueryEtryPt_STD_CONF2_PREPRIVDROP_QUERIES \
	  else if(!strcmp((char*) name, "activateCnfPrePrivDrop")) {\
		*pEtryPoint = activateCnfPrePrivDrop;\
	}

/* the following block is to be added for modules that support
 * their config name. This is required for the rsyslog v6 config
 * system, especially for outout modules which do not require
 * the new set of begin/end config settings.
 */
#define CODEqueryEtryPt_STD_CONF2_CNFNAME_QUERIES \
	  else if(!strcmp((char*) name, "getModCnfName")) {\
		*pEtryPoint = modGetCnfName;\
	}

/* the following definition is the standard block for queryEtryPt for LIBRARY
 * modules. This can be used if no specific handling (e.g. to cover version
 * differences) is needed.
 */
#define CODEqueryEtryPt_STD_LIB_QUERIES \
	CODEqueryEtryPt_STD_MOD_QUERIES

/* the following definition is the standard block for queryEtryPt for PARSER
 * modules. This can be used if no specific handling (e.g. to cover version
 * differences) is needed.
 */
#define CODEqueryEtryPt_STD_PMOD_QUERIES \
	CODEqueryEtryPt_STD_MOD_QUERIES \
	else if(!strcmp((char*) name, "parse")) {\
		*pEtryPoint = parse;\
	} else if(!strcmp((char*) name, "GetParserName")) {\
		*pEtryPoint = GetParserName;\
	}

/* the following definition is the standard block for queryEtryPt for PARSER
 * modules obeying the v2+ config interface.
 */
#define CODEqueryEtryPt_STD_PMOD2_QUERIES \
	CODEqueryEtryPt_STD_MOD_QUERIES \
	else if(!strcmp((char*) name, "parse2")) {\
		*pEtryPoint = parse2;\
	} else if(!strcmp((char*) name, "GetParserName")) {\
		*pEtryPoint = GetParserName;\
	} else if(!strcmp((char*) name, "newParserInst")) {\
		*pEtryPoint = newParserInst;\
	} else if(!strcmp((char*) name, "freeParserInst")) {\
		*pEtryPoint = freeParserInst;\
	} \
	CODEqueryEtryPt_STD_CONF2_CNFNAME_QUERIES



/* the following definition is the standard block for queryEtryPt for rscript function
 * modules. This can be used if no specific handling (e.g. to cover version
 * differences) is needed.
 */
#define CODEqueryEtryPt_STD_FMOD_QUERIES \
	CODEqueryEtryPt_STD_MOD_QUERIES \
	else if(!strcmp((char*) name, "getFunctArray")) {\
		*pEtryPoint = getFunctArray;\
	}

/* the following definition is the standard block for queryEtryPt for Strgen
 * modules. This can be used if no specific handling (e.g. to cover version
 * differences) is needed.
 */
#define CODEqueryEtryPt_STD_SMOD_QUERIES \
	CODEqueryEtryPt_STD_MOD_QUERIES \
	else if(!strcmp((char*) name, "strgen")) {\
		*pEtryPoint = strgen;\
	} else if(!strcmp((char*) name, "GetName")) {\
		*pEtryPoint = GetStrgenName;\
	}

/* modInit()
 * This has an extra parameter, which is the specific name of the modInit
 * function. That is needed for built-in modules, which must have unique
 * names in order to link statically. Please note that this is always only
 * the case with modInit() and NO other entry point. The reason is that only
 * modInit() is visible form a linker/loader point of view. All other entry
 * points are passed via rsyslog-internal query functions and are defined
 * static inside the modules source. This is an important concept, as it allows
 * us to support different interface versions within a single module. (Granted,
 * we do not currently have different interface versions, so we can not put
 * it to a test - but our firm believe is that we can do all abstraction needed...)
 *
 * Extra Comments:
 * initialize the module
 *
 * Later, much more must be done. So far, we only return a pointer
 * to the queryEtryPt() function
 * TODO: do interface version checking & handshaking
 * iIfVersRequetsed is the version of the interface specification that the
 * caller would like to see being used. ipIFVersProvided is what we
 * decide to provide.
 * rgerhards, 2007-11-21: see modExit() comment below for important information
 * on the need to initialize static data with code. modInit() may be called on a
 * cached, left-in-memory copy of a previous incarnation.
 */
#define BEGINmodInit(uniqName) \
rsRetVal __attribute__((unused)) modInit##uniqName(int iIFVersRequested __attribute__((unused)), \
int *ipIFVersProvided, rsRetVal (**pQueryEtryPt)(), rsRetVal (*pHostQueryEtryPt)(uchar*, rsRetVal (**)()), \
modInfo_t __attribute__((unused)) *pModInfo);\
rsRetVal __attribute__((unused)) modInit##uniqName(int iIFVersRequested __attribute__((unused)), \
int *ipIFVersProvided, rsRetVal (**pQueryEtryPt)(), rsRetVal (*pHostQueryEtryPt)(uchar*, rsRetVal (**)()), \
modInfo_t __attribute__((unused)) *pModInfo)\
{\
	DEFiRet; \
	rsRetVal (*pObjGetObjInterface)(obj_if_t *pIf);

#define CODESTARTmodInit \
	assert(pHostQueryEtryPt != NULL);\
	iRet = pHostQueryEtryPt((uchar*)"objGetObjInterface", &pObjGetObjInterface); \
	if((iRet != RS_RET_OK) || (pQueryEtryPt == NULL) || (ipIFVersProvided == NULL) || \
		(pObjGetObjInterface == NULL)) { \
		ENDfunc \
		return (iRet == RS_RET_OK) ? RS_RET_PARAM_ERROR : iRet; \
	} \
	/* now get the obj interface so that we can access other objects */ \
	CHKiRet(pObjGetObjInterface(&obj));

/* do those initializations necessary for legacy config variables */
#define INITLegCnfVars \
	initConfVars();

#define ENDmodInit \
finalize_it:\
	*pQueryEtryPt = queryEtryPt;\
	RETiRet;\
}


/* now come some check functions, which enable a standard way of obtaining feature
 * information from the core. feat is the to-be-tested feature and featVar is a
 * variable that receives the result (0-not support, 1-supported).
 * This must be a macro, so that it is put into the output's code. Otherwise, we
 * would need to rely on a library entry point, which is what we intend to avoid ;)
 * rgerhards, 2009-04-27
 */
#define INITChkCoreFeature(featVar, feat) \
{ \
	rsRetVal MACRO_Ret; \
	rsRetVal (*pQueryCoreFeatureSupport)(int*, unsigned); \
	int bSupportsIt; \
	featVar = 0; \
	MACRO_Ret = pHostQueryEtryPt((uchar*)"queryCoreFeatureSupport", &pQueryCoreFeatureSupport); \
	if(MACRO_Ret == RS_RET_OK) { \
		/* found entry point, so let's see if core supports it */ \
		CHKiRet((*pQueryCoreFeatureSupport)(&bSupportsIt, feat)); \
		if(bSupportsIt) \
			featVar = 1; \
	} else if(MACRO_Ret != RS_RET_ENTRY_POINT_NOT_FOUND) { \
		ABORT_FINALIZE(MACRO_Ret); /* Something else went wrong, what is not acceptable */ \
	} \
}



/* definitions for host API queries */
#define CODEmodInit_QueryRegCFSLineHdlr \
	CHKiRet(pHostQueryEtryPt((uchar*)"regCfSysLineHdlr", &omsdRegCFSLineHdlr));


/* modExit()
 * This is the counterpart to modInit(). It destroys a module and makes it ready for
 * unloading. It is similiar to freeInstance() for the instance data. Please note that
 * this entry point needs to free any module-global data structures and registrations.
 * For example, the CfSysLineHandlers a module has registered need to be unregistered
 * here. This entry point is only called immediately before unloading of the module. So
 * it is likely to be destroyed. HOWEVER, the caller may decide to keep the module cached.
 * So a module must never assume that it is actually destroyed. A call to modInit() may
 * happen immediately after modExit(). So a module can NOT assume that static data elements
 * are being re-initialized by the loader - this must always be done by module code itself.
 * It is suggested to do this in modInit(). - rgerhards, 2007-11-21
 */
#define BEGINmodExit \
static rsRetVal modExit(void)\
{\
	DEFiRet;

#define CODESTARTmodExit

#define ENDmodExit \
	RETiRet;\
}


/* beginCnfLoad()
 * This is a function tells an input module that a new config load begins.
 * The core passes in a handle to the new module-specific module conf to
 * the module. -- rgerards, 2011-05-03
 */
#define BEGINbeginCnfLoad \
static rsRetVal beginCnfLoad(modConfData_t **ptr, __attribute__((unused)) rsconf_t *pConf)\
{\
	modConfData_t *pModConf; \
	DEFiRet;

#define CODESTARTbeginCnfLoad \
	if((pModConf = calloc(1, sizeof(modConfData_t))) == NULL) {\
		*ptr = NULL;\
		ENDfunc \
		return RS_RET_OUT_OF_MEMORY;\
	}

#define ENDbeginCnfLoad \
	*ptr = pModConf;\
	RETiRet;\
}


/* setModCnf()
 * This function permits to set module global parameters via the v2 config
 * interface. It may be called multiple times, but parameters must not be
 * set in a conflicting way. The module must use its current config load
 * context when processing the directives.
 * Note that lst may be NULL, especially if the module is loaded via the
 * legacy config system. The module must check for this.
 * NOTE: This entry point must only be implemented if module global
 * parameters are actually required.
 */
#define BEGINsetModCnf \
static rsRetVal setModCnf(struct nvlst *lst)\
{\
	DEFiRet;

#define CODESTARTsetModCnf

#define ENDsetModCnf \
	RETiRet;\
}


/* endCnfLoad()
 * This is a function tells an input module that the current config load ended.
 * It gets a last chance to make changes to its in-memory config object. After
 * this call, the config object must no longer be changed.
 * The pModConf pointer passed into the module must no longer be used.
 * rgerards, 2011-05-03
 */
#define BEGINendCnfLoad \
static rsRetVal endCnfLoad(modConfData_t *ptr)\
{\
	modConfData_t __attribute__((unused)) *pModConf = (modConfData_t*) ptr; \
	DEFiRet;

#define CODESTARTendCnfLoad

#define ENDendCnfLoad \
	RETiRet;\
}


/* checkCnf()
 * Check the provided config object for errors, inconsistencies and other things
 * that do not work out.
 * NOTE: no part of the config must be activated, so some checks that require
 * activation can not be done in this entry point. They must be done in the
 * activateConf() stage, where the caller must also be prepared for error
 * returns.
 * rgerhards, 2011-05-03
 */
#define BEGINcheckCnf \
static rsRetVal checkCnf(modConfData_t *ptr)\
{\
	modConfData_t __attribute__((unused)) *pModConf = (modConfData_t*) ptr; \
	DEFiRet;

#define CODESTARTcheckCnf

#define ENDcheckCnf \
	RETiRet;\
}


/* activateCnfPrePrivDrop()
 * Initial config activation, before dropping privileges. This is an optional
 * entry points that should only be implemented by those module that really need
 * it. Processing should be limited to the minimum possible. Main activation
 * should happen in the normal activateCnf() call.
 * rgerhards, 2011-05-06
 */
#define BEGINactivateCnfPrePrivDrop \
static rsRetVal activateCnfPrePrivDrop(modConfData_t *ptr)\
{\
	modConfData_t *pModConf = (modConfData_t*) ptr; \
	DEFiRet;

#define CODESTARTactivateCnfPrePrivDrop

#define ENDactivateCnfPrePrivDrop \
	RETiRet;\
}


/* activateCnf()
 * This activates the provided config, and may report errors if they are detected
 * during activation.
 * rgerhards, 2011-05-03
 */
#define BEGINactivateCnf \
static rsRetVal activateCnf(modConfData_t *ptr)\
{\
	modConfData_t __attribute__((unused)) *pModConf = (modConfData_t*) ptr; \
	DEFiRet;

#define CODESTARTactivateCnf

#define ENDactivateCnf \
	RETiRet;\
}


/* freeCnf()
 * This is a function tells an input module that it must free all data
 * associated with the passed-in module config.
 * rgerhards, 2011-05-03
 */
#define BEGINfreeCnf \
static rsRetVal freeCnf(void *ptr)\
{\
	modConfData_t *pModConf = (modConfData_t*) ptr; \
	DEFiRet;

#define CODESTARTfreeCnf

#define ENDfreeCnf \
	if(pModConf != NULL)\
		free(pModConf); /* we need to free this in any case */\
	RETiRet;\
}


/* runInput()
 * This is the main function for input modules. It is used to gather data from the
 * input source and submit it to the message queue. Each runInput() instance has its own
 * thread. This is handled by the rsyslog engine. It needs to spawn off new threads only
 * if there is a module-internal need to do so.
 */
#define BEGINrunInput \
static rsRetVal runInput(thrdInfo_t __attribute__((unused)) *pThrd)\
{\
	DEFiRet;

#define CODESTARTrunInput \
	dbgSetThrdName((uchar*)__FILE__); /* we need to provide something better later */

#define ENDrunInput \
	RETiRet;\
}


/* willRun()
 * This is a function that will be replaced in the longer term. It is used so
 * that a module can tell the caller if it will run or not. This is to be replaced
 * when we introduce input module instances. However, these require config syntax
 * changes and I may (or may not... ;)) hold that until another config file
 * format is available. -- rgerhards, 2007-12-17
 * returns RS_RET_NO_RUN if it will not run (RS_RET_OK or error otherwise)
 */
#define BEGINwillRun \
static rsRetVal willRun(void)\
{\
	DEFiRet;

#define CODESTARTwillRun

#define ENDwillRun \
	RETiRet;\
}


/* afterRun()
 * This function is called after an input module has been run and its thread has
 * been terminated. It shall do any necessary cleanup.
 * This is expected to evolve into a freeInstance type of call once the input module
 * interface evolves to support multiple instances.
 * rgerhards, 2007-12-17
 */
#define BEGINafterRun \
static rsRetVal afterRun(void)\
{\
	DEFiRet;

#define CODESTARTafterRun

#define ENDafterRun \
	RETiRet;\
}


/* doHUP()
 * This function is optional. Currently, it is available to output plugins
 * only, but may be made available to other types of plugins in the future.
 * A plugin does not need to define this entry point. If if does, it gets
 * called when a HUP at the action level is to be done. A plugin should register
 * this function so that it can close files, connection or other ressources
 * on HUP - if it can be assume the user wanted to do this as a part of HUP
 * processing. Note that the name "HUP" has historical reasons, it stems back
 * to the infamous SIGHUP which was sent to restart a syslogd. We still retain
 * that legacy, but may move this to a different signal.
 * rgerhards, 2008-10-22
 */
#define CODEqueryEtryPt_doHUP \
	else if(!strcmp((char*) name, "doHUP")) {\
		*pEtryPoint = doHUP;\
	}
#define BEGINdoHUP \
static rsRetVal doHUP(instanceData __attribute__((unused)) *pData)\
{\
	DEFiRet;

#define CODESTARTdoHUP

#define ENDdoHUP \
	RETiRet;\
}


/* doHUPWrkr()
 * This is like doHUP(), but on an action worker level.
 * rgerhards, 2015-03-25
 */
#define CODEqueryEtryPt_doHUPWrkr \
	else if(!strcmp((char*) name, "doHUPWrkr")) {\
		*pEtryPoint = doHUPWrkr;\
	}
#define BEGINdoHUPWrkr \
static rsRetVal doHUPWrkr(wrkrInstanceData_t __attribute__((unused)) *pWrkrData)\
{\
	DEFiRet;

#define CODESTARTdoHUPWrkr

#define ENDdoHUPWrkr \
	RETiRet;\
}


/* SetShutdownImmdtPtr()
 * This function is optional. If defined by an output plugin, it is called
 * each time the action is invoked to set the "ShutdownImmediate" pointer,
 * which is used during termination to indicate the action should shutdown
 * as quickly as possible.
 */
#define CODEqueryEtryPt_SetShutdownImmdtPtr \
	else if(!strcmp((char*) name, "SetShutdownImmdtPtr")) {\
		*pEtryPoint = SetShutdownImmdtPtr;\
	}
#define BEGINSetShutdownImmdtPtr \
static rsRetVal SetShutdownImmdtPtr(instanceData __attribute__((unused)) *pData, int *pPtr)\
{\
	DEFiRet;

#define CODESTARTSetShutdownImmdtPtr

#define ENDSetShutdownImmdtPtr \
	RETiRet;\
}


/* parse() - main entry point of parser modules (v1 config interface)
 */
#define BEGINparse \
static rsRetVal parse(smsg_t *pMsg)\
{\
	DEFiRet;

#define CODESTARTparse \
	assert(pMsg != NULL);

#define ENDparse \
	RETiRet;\
}


/* parse2() - main entry point of parser modules (v2+ config interface)
 */
#define BEGINparse2 \
static rsRetVal parse2(instanceConf_t *const pInst, smsg_t *pMsg)\
{\
	DEFiRet;

#define CODESTARTparse2 \
	assert(pInst != NULL);\
	assert(pMsg != NULL);

#define ENDparse2 \
	RETiRet;\
}


/* strgen() - main entry point of parser modules
 * Note that we do NOT use size_t as this permits us to store the
 * values directly into optimized heap structures.
 * ppBuf is the buffer pointer
 * pLenBuf is the current max size of this buffer
 * pStrLen is an output parameter that MUST hold the length
 *         of the generated string on exit (this is cached)
 */
#define BEGINstrgen \
static rsRetVal strgen(smsg_t *const pMsg, actWrkrIParams_t *const iparam) \
{\
	DEFiRet;

#define CODESTARTstrgen \
	assert(pMsg != NULL);

#define ENDstrgen \
	RETiRet;\
}



/* getFunctArray() - main entry point of parser modules
 * Note that we do NOT use size_t as this permits us to store the
 * values directly into optimized heap structures.
 * ppBuf is the buffer pointer
 * pLenBuf is the current max size of this buffer
 * pStrLen is an output parameter that MUST hold the length
 *         of the generated string on exit (this is cached)
 */
#define BEGINgetFunctArray \
static rsRetVal getFunctArray(int *const version, const struct scriptFunct**const functArray) \
{\
	DEFiRet;

#define CODESTARTgetFunctArray

#define ENDgetFunctArray \
	RETiRet;\
}


/* function to specify the parser name. This is done via a single command which
 * receives a ANSI string as parameter.
 */
#define PARSER_NAME(x) \
static rsRetVal GetParserName(uchar **ppSz)\
{\
	*ppSz = UCHAR_CONSTANT(x);\
	return RS_RET_OK;\
}



/* function to specify the strgen name. This is done via a single command which
 * receives a ANSI string as parameter.
 */
#define STRGEN_NAME(x) \
static rsRetVal GetStrgenName(uchar **ppSz)\
{\
	*ppSz = UCHAR_CONSTANT(x);\
	return RS_RET_OK;\
}

#endif /* #ifndef MODULE_TEMPLATE_H_INCLUDED */

/* vim:set ai:
 */
