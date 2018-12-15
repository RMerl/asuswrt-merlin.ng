/* modules.c
 * This is the implementation of syslogd modules object.
 * This object handles plug-ins and build-in modules of all kind.
 *
 * Modules are reference-counted. Anyone who access a module must call
 * Use() before any function is accessed and Release() when he is done.
 * When the reference count reaches 0, rsyslog unloads the module (that
 * may be changed in the future to cache modules). Rsyslog does NOT
 * unload modules with a reference count > 0, even if the unload
 * method is called!
 *
 * File begun on 2007-07-22 by RGerhards
 *
 * Copyright 2007-2018 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of the rsyslog runtime library.
 *
 * The rsyslog runtime library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The rsyslog runtime library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the rsyslog runtime library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 * A copy of the LGPL can be found in the file "COPYING.LESSER" in this distribution.
 */
#include "config.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#ifdef	OS_BSD
#	include "libgen.h"
#endif

#include <dlfcn.h> /* TODO: replace this with the libtools equivalent! */

#include <unistd.h>
#include <sys/file.h>

#ifndef PATH_MAX
#	define PATH_MAX MAXPATHLEN
#endif

#include "rsyslog.h"
#include "rainerscript.h"
#include "cfsysline.h"
#include "rsconf.h"
#include "modules.h"
#include "errmsg.h"
#include "parser.h"
#include "strgen.h"

/* static data */
DEFobjStaticHelpers
DEFobjCurrIf(strgen)

static modInfo_t *pLoadedModules = NULL;	/* list of currently-loaded modules */
static modInfo_t *pLoadedModulesLast = NULL;	/* tail-pointer */

/* already dlopen()-ed libs */
static struct dlhandle_s *pHandles = NULL;

static uchar *pModDir;		/* directory where loadable modules are found */

/* tables for interfacing with the v6 config system */
/* action (instance) parameters */
static struct cnfparamdescr actpdescr[] = {
	{ "load", eCmdHdlrGetWord, 1 }
};
static struct cnfparamblk pblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(actpdescr)/sizeof(struct cnfparamdescr),
	  actpdescr
	};


/* we provide a set of dummy functions for modules that do not support the
 * some interfaces.
 * On the commit feature: As the modules do not support it, they commit each message they
 * receive, and as such the dummies can always return RS_RET_OK without causing
 * harm. This simplifies things as in action processing we do not need to check
 * if the transactional entry points exist.
 */
static rsRetVal
dummyBeginTransaction(__attribute__((unused)) void * dummy)
{
	return RS_RET_OK;
}
static rsRetVal
dummyEndTransaction(__attribute__((unused)) void * dummy)
{
	return RS_RET_OK;
}
static rsRetVal
dummyIsCompatibleWithFeature(__attribute__((unused)) syslogFeature eFeat)
{
	return RS_RET_INCOMPATIBLE;
}
static rsRetVal
dummynewActInst(uchar *modName, struct nvlst __attribute__((unused)) *dummy1,
		void __attribute__((unused)) **dummy2, omodStringRequest_t __attribute__((unused)) **dummy3)
{
	LogError(0, RS_RET_CONFOBJ_UNSUPPORTED, "config objects are not "
			"supported by module '%s' -- legacy config options "
			"MUST be used instead", modName);
	return RS_RET_CONFOBJ_UNSUPPORTED;
}

#ifdef DEBUG
/* we add some home-grown support to track our users (and detect who does not free us). In
 * the long term, this should probably be migrated into debug.c (TODO). -- rgerhards, 2008-03-11
 */

/* add a user to the current list of users (always at the root) */
static void
modUsrAdd(modInfo_t *pThis, const char *pszUsr)
{
	modUsr_t *pUsr;

	BEGINfunc
	if((pUsr = calloc(1, sizeof(modUsr_t))) == NULL)
		goto finalize_it;

	if((pUsr->pszFile = strdup(pszUsr)) == NULL) {
		free(pUsr);
		goto finalize_it;
	}

	if(pThis->pModUsrRoot != NULL) {
		pUsr->pNext = pThis->pModUsrRoot;
	}
	pThis->pModUsrRoot = pUsr;

finalize_it:
	ENDfunc;
}


/* remove a user from the current user list
 * rgerhards, 2008-03-11
 */
static void
modUsrDel(modInfo_t *pThis, const char *pszUsr)
{
	modUsr_t *pUsr;
	modUsr_t *pPrev = NULL;

	for(pUsr = pThis->pModUsrRoot ; pUsr != NULL ; pUsr = pUsr->pNext) {
		if(!strcmp(pUsr->pszFile, pszUsr))
			break;
		else
			pPrev = pUsr;
	}

	if(pUsr == NULL) {
		dbgprintf("oops - tried to delete user %s from module %s and it wasn't registered as one...\n",
			  pszUsr, pThis->pszName);
	} else {
		if(pPrev == NULL) {
			/* This was at the root! */
			pThis->pModUsrRoot = pUsr->pNext;
		} else {
			pPrev->pNext = pUsr->pNext;
		}
		/* free ressources */
		free(pUsr->pszFile);
		free(pUsr);
		pUsr = NULL; /* just to make sure... */
	}
}


/* print a short list all all source files using the module in question
 * rgerhards, 2008-03-11
 */
static void
modUsrPrint(modInfo_t *pThis)
{
	modUsr_t *pUsr;

	for(pUsr = pThis->pModUsrRoot ; pUsr != NULL ; pUsr = pUsr->pNext) {
		dbgprintf("\tmodule %s is currently in use by file %s\n",
			  pThis->pszName, pUsr->pszFile);
	}
}


/* print all loaded modules and who is accessing them. This is primarily intended
 * to be called at end of run to detect "module leaks" and who is causing them.
 * rgerhards, 2008-03-11
 */
static void
modUsrPrintAll(void)
{
	modInfo_t *pMod;

	BEGINfunc
	for(pMod = pLoadedModules ; pMod != NULL ; pMod = pMod->pNext) {
		dbgprintf("printing users of loadable module %s, refcount %u, ptr %p, type %d\n",
		pMod->pszName, pMod->uRefCnt, pMod, pMod->eType);
		modUsrPrint(pMod);
	}
	ENDfunc
}

#endif /* #ifdef DEBUG */


/* Construct a new module object
 */
static rsRetVal moduleConstruct(modInfo_t **pThis)
{
	modInfo_t *pNew;

	if((pNew = calloc(1, sizeof(modInfo_t))) == NULL)
		return RS_RET_OUT_OF_MEMORY;

	/* OK, we got the element, now initialize members that should
	 * not be zero-filled.
	 */

	*pThis = pNew;
	return RS_RET_OK;
}


/* Destructs a module object. The object must not be linked to the
 * linked list of modules. Please note that all other dependencies on this
 * modules must have been removed before (e.g. CfSysLineHandlers!)
 */
static void moduleDestruct(modInfo_t *pThis)
{
	assert(pThis != NULL);
	free(pThis->pszName);
	free(pThis->cnfName);
	if(pThis->pModHdlr != NULL) {
#	ifdef	VALGRIND
		DBGPRINTF("moduleDestruct: compiled with valgrind, do "
			"not unload module\n");
#	else
		if(glblUnloadModules) {
			if(pThis->eKeepType == eMOD_NOKEEP) {
				dlclose(pThis->pModHdlr);
			}
		} else {
			DBGPRINTF("moduleDestruct: not unloading module "
				"due to user configuration\n");
		}
#	endif
	}

	free(pThis);
}


/* This enables a module to query the core for specific features.
 * rgerhards, 2009-04-22
 */
static rsRetVal queryCoreFeatureSupport(int *pBool, unsigned uFeat)
{
	DEFiRet;

	if(pBool == NULL)
		ABORT_FINALIZE(RS_RET_PARAM_ERROR);

	*pBool = (uFeat & CORE_FEATURE_BATCHING) ? 1 : 0;

finalize_it:
	RETiRet;
}


/* The following function is the queryEntryPoint for host-based entry points.
 * Modules may call it to get access to core interface functions. Please note
 * that utility functions can be accessed via shared libraries - at least this
 * is my current shool of thinking.
 * Please note that the implementation as a query interface allows to take
 * care of plug-in interface version differences. -- rgerhards, 2007-07-31
 * ... but often it better not to use a new interface. So we now add core
 * functions here that a plugin may request. -- rgerhards, 2009-04-22
 */
static rsRetVal queryHostEtryPt(uchar *name, rsRetVal (**pEtryPoint)())
{
	DEFiRet;

	if((name == NULL) || (pEtryPoint == NULL))
		ABORT_FINALIZE(RS_RET_PARAM_ERROR);

	if(!strcmp((char*) name, "regCfSysLineHdlr")) {
		*pEtryPoint = regCfSysLineHdlr;
	} else if(!strcmp((char*) name, "objGetObjInterface")) {
		*pEtryPoint = objGetObjInterface;
	} else if(!strcmp((char*) name, "OMSRgetSupportedTplOpts")) {
		*pEtryPoint = OMSRgetSupportedTplOpts;
	} else if(!strcmp((char*) name, "queryCoreFeatureSupport")) {
		*pEtryPoint = queryCoreFeatureSupport;
	} else {
		*pEtryPoint = NULL; /* to  be on the safe side */
		ABORT_FINALIZE(RS_RET_ENTRY_POINT_NOT_FOUND);
	}

finalize_it:
	RETiRet;
}


/* get the name of a module
 */
uchar *
modGetName(modInfo_t *pThis)
{
	return((pThis->pszName == NULL) ? (uchar*) "" : pThis->pszName);
}


/* get the state-name of a module. The state name is its name
 * together with a short description of the module state (which
 * is pulled from the module itself.
 * rgerhards, 2007-07-24
 * TODO: the actual state name is not yet pulled
 */
static uchar *modGetStateName(modInfo_t *pThis)
{
	return(modGetName(pThis));
}


/* Add a module to the loaded module linked list
 */
static void
addModToGlblList(modInfo_t *pThis)
{
	assert(pThis != NULL);

	if(pLoadedModules == NULL) {
		pLoadedModules = pLoadedModulesLast = pThis;
	} else {
		/* there already exist entries */
		pThis->pPrev = pLoadedModulesLast;
		pLoadedModulesLast->pNext = pThis;
		pLoadedModulesLast = pThis;
	}
}


/* ready module for config processing. this includes checking if the module
 * is already in the config, so this function may return errors. Returns a
 * pointer to the last module inthe current config. That pointer needs to
 * be passed to addModToCnfLst() when it is called later in the process.
 */
rsRetVal
readyModForCnf(modInfo_t *pThis, cfgmodules_etry_t **ppNew, cfgmodules_etry_t **ppLast)
{
	cfgmodules_etry_t *pNew = NULL;
	cfgmodules_etry_t *pLast;
	DEFiRet;
	assert(pThis != NULL);

	if(loadConf == NULL) {
		FINALIZE; /* we are in an early init state */
	}

	/* check for duplicates and, as a side-activity, identify last node */
	pLast = loadConf->modules.root;
	if(pLast != NULL) {
		while(1) { /* loop broken inside */
			if(pLast->pMod == pThis) {
				DBGPRINTF("module '%s' already in this config\n", modGetName(pThis));
				if(strncmp((char*)modGetName(pThis), "builtin:", sizeof("builtin:")-1)) {
					LogError(0, RS_RET_MODULE_ALREADY_IN_CONF,
					   "module '%s' already in this config, cannot be added\n", modGetName(pThis));
					ABORT_FINALIZE(RS_RET_MODULE_ALREADY_IN_CONF);
				}
				FINALIZE;
			}
			if(pLast->next == NULL)
				break;
			pLast = pLast->next;
		}
	}

	/* if we reach this point, pLast is the tail pointer and this module is new
	 * inside the currently loaded config. So, iff it is an input module, let's
	 * pass it a pointer which it can populate with a pointer to its module conf.
	 */

	CHKmalloc(pNew = MALLOC(sizeof(cfgmodules_etry_t)));
	pNew->canActivate = 1;
	pNew->next = NULL;
	pNew->pMod = pThis;

	if(pThis->beginCnfLoad != NULL) {
		CHKiRet(pThis->beginCnfLoad(&pNew->modCnf, loadConf));
	}

	*ppLast = pLast;
	*ppNew = pNew;
finalize_it:
	if(iRet != RS_RET_OK) {
		if(pNew != NULL)
			free(pNew);
	}
	RETiRet;
}


/* abort the creation of a module entry without adding it to the
 * module list. Needed to prevent mem leaks.
 */
static inline void
abortCnfUse(cfgmodules_etry_t **pNew)
{
	if(pNew != NULL) {
		free(*pNew);
		*pNew = NULL;
	}
}


/* Add a module to the config module list for current loadConf.
 * Requires last pointer obtained by readyModForCnf().
 * The module pointer is handed over to this function. It is no
 * longer available to caller one we are called.
 */
rsRetVal ATTR_NONNULL(1)
addModToCnfList(cfgmodules_etry_t **const pNew, cfgmodules_etry_t *const pLast)
{
	DEFiRet;
	assert(*pNew != NULL);

	if(loadConf == NULL) {
		abortCnfUse(pNew);
		FINALIZE; /* we are in an early init state */
	}

	if(pLast == NULL) {
		loadConf->modules.root = *pNew;
	} else {
		/* there already exist entries */
		pLast->next = *pNew;
	}

finalize_it:
	*pNew = NULL;
	RETiRet;
}


/* Get the next module pointer - this is used to traverse the list.
 * The function returns the next pointer or NULL, if there is no next one.
 * The last object must be provided to the function. If NULL is provided,
 * it starts at the root of the list. Even in this case, NULL may be
 * returned - then, the list is empty.
 * rgerhards, 2007-07-23
 */
static modInfo_t *GetNxt(modInfo_t *pThis)
{
	modInfo_t *pNew;

	if(pThis == NULL)
		pNew = pLoadedModules;
	else
		pNew = pThis->pNext;

	return(pNew);
}


/* this function is like GetNxt(), but it returns pointers to
 * the configmodules entry, which than can be used to obtain the
 * actual module pointer. Note that it returns those for
 * modules of specific type only. Only modules from the provided
 * config are returned. Note that processing speed could be improved,
 * but this is really not relevant, as config file loading is not really
 * something we are concerned about in regard to runtime.
 */
static cfgmodules_etry_t
*GetNxtCnfType(rsconf_t *cnf, cfgmodules_etry_t *node, eModType_t rqtdType)
{
	if(node == NULL) { /* start at beginning of module list */
		node = cnf->modules.root;
	} else {
		node = node->next;
	}

	if(rqtdType != eMOD_ANY) { /* if any, we already have the right one! */
		while(node != NULL && node->pMod->eType != rqtdType) {
			node = node->next;
		}
	}

	return node;
}


/* Find a module with the given conf name and type. Returns NULL if none
 * can be found, otherwise module found.
 */
static modInfo_t *
FindWithCnfName(rsconf_t *cnf, uchar *name, eModType_t rqtdType)
{
	cfgmodules_etry_t *node;

	;
	for(  node = cnf->modules.root
	    ; node != NULL
	    ; node = node->next) {
		if(node->pMod->eType != rqtdType || node->pMod->cnfName == NULL)
			continue;
		if(!strcasecmp((char*)node->pMod->cnfName, (char*)name))
			break;
	}

	return node == NULL ? NULL : node->pMod;
}


/* Prepare a module for unloading.
 * This is currently a dummy, to be filled when we have a plug-in
 * interface - rgerhards, 2007-08-09
 * rgerhards, 2007-11-21:
 * When this function is called, all instance-data must already have
 * been destroyed. In the case of output modules, this happens when the
 * rule set is being destroyed. When we implement other module types, we
 * need to think how we handle it there (and if we have any instance data).
 * rgerhards, 2008-03-10: reject unload request if the module has a reference
 * count > 0.
 */
static rsRetVal
modPrepareUnload(modInfo_t *pThis)
{
	DEFiRet;
	void *pModCookie;

	assert(pThis != NULL);

	if(pThis->uRefCnt > 0) {
		dbgprintf("rejecting unload of module '%s' because it has a refcount of %d\n",
			  pThis->pszName, pThis->uRefCnt);
		ABORT_FINALIZE(RS_RET_MODULE_STILL_REFERENCED);
	}

	CHKiRet(pThis->modGetID(&pModCookie));
	pThis->modExit(); /* tell the module to get ready for unload */
	CHKiRet(unregCfSysLineHdlrs4Owner(pModCookie));

finalize_it:
	RETiRet;
}


/* Add an already-loaded module to the module linked list. This function does
 * everything needed to fully initialize the module.
 */
static rsRetVal
doModInit(rsRetVal (*modInit)(int, int*, rsRetVal(**)(), rsRetVal(*)(), modInfo_t*),
	  uchar *name, void *pModHdlr, modInfo_t **pNewModule)
{
	rsRetVal localRet;
	modInfo_t *pNew = NULL;
	uchar *pName;
	strgen_t *pStrgen; /* used for strgen modules */
	rsRetVal (*GetName)(uchar**);
	rsRetVal (*modGetType)(eModType_t *pType);
	rsRetVal (*modGetKeepType)(eModKeepType_t *pKeepType);
	struct dlhandle_s *pHandle = NULL;
	rsRetVal (*getModCnfName)(uchar **cnfName);
	uchar *cnfName;
	DEFiRet;

	assert(modInit != NULL);

	if((iRet = moduleConstruct(&pNew)) != RS_RET_OK) {
		pNew = NULL;
		FINALIZE;
	}

	CHKiRet((*modInit)(CURR_MOD_IF_VERSION, &pNew->iIFVers, &pNew->modQueryEtryPt, queryHostEtryPt, pNew));

	if(pNew->iIFVers != CURR_MOD_IF_VERSION) {
		ABORT_FINALIZE(RS_RET_MISSING_INTERFACE);
	}

	/* We now poll the module to see what type it is. We do this only once as this
	 * can never change in the lifetime of an module. -- rgerhards, 2007-12-14
	 */
	CHKiRet((*pNew->modQueryEtryPt)((uchar*)"getType", &modGetType));
	CHKiRet((*modGetType)(&pNew->eType));
	CHKiRet((*pNew->modQueryEtryPt)((uchar*)"getKeepType", &modGetKeepType));
	CHKiRet((*modGetKeepType)(&pNew->eKeepType));
	dbgprintf("module %s of type %d being loaded (keepType=%d).\n", name, pNew->eType, pNew->eKeepType);
	
	/* OK, we know we can successfully work with the module. So we now fill the
	 * rest of the data elements. First we load the interfaces common to all
	 * module types.
	 */
	CHKiRet((*pNew->modQueryEtryPt)((uchar*)"modGetID", &pNew->modGetID));
	CHKiRet((*pNew->modQueryEtryPt)((uchar*)"modExit", &pNew->modExit));
	localRet = (*pNew->modQueryEtryPt)((uchar*)"isCompatibleWithFeature", &pNew->isCompatibleWithFeature);
	if(localRet == RS_RET_MODULE_ENTRY_POINT_NOT_FOUND)
		pNew->isCompatibleWithFeature = dummyIsCompatibleWithFeature;
	else if(localRet != RS_RET_OK)
		ABORT_FINALIZE(localRet);
	localRet = (*pNew->modQueryEtryPt)((uchar*)"setModCnf", &pNew->setModCnf);
	if(localRet == RS_RET_MODULE_ENTRY_POINT_NOT_FOUND)
		pNew->setModCnf = NULL;
	else if(localRet != RS_RET_OK)
		ABORT_FINALIZE(localRet);

	/* optional calls for new config system */
	localRet = (*pNew->modQueryEtryPt)((uchar*)"getModCnfName", &getModCnfName);
	if(localRet == RS_RET_OK) {
		if(getModCnfName(&cnfName) == RS_RET_OK)
			pNew->cnfName = (uchar*) strdup((char*)cnfName);
			  /**< we do not care if strdup() fails, we can accept that */
		else
			pNew->cnfName = NULL;
		dbgprintf("module config name is '%s'\n", cnfName);
	}
	localRet = (*pNew->modQueryEtryPt)((uchar*)"beginCnfLoad", &pNew->beginCnfLoad);
	if(localRet == RS_RET_OK) {
		dbgprintf("module %s supports rsyslog v6 config interface\n", name);
		CHKiRet((*pNew->modQueryEtryPt)((uchar*)"endCnfLoad", &pNew->endCnfLoad));
		CHKiRet((*pNew->modQueryEtryPt)((uchar*)"freeCnf", &pNew->freeCnf));
		CHKiRet((*pNew->modQueryEtryPt)((uchar*)"checkCnf", &pNew->checkCnf));
		CHKiRet((*pNew->modQueryEtryPt)((uchar*)"activateCnf", &pNew->activateCnf));
		localRet = (*pNew->modQueryEtryPt)((uchar*)"activateCnfPrePrivDrop", &pNew->activateCnfPrePrivDrop);
		if(localRet == RS_RET_MODULE_ENTRY_POINT_NOT_FOUND) {
			pNew->activateCnfPrePrivDrop = NULL;
		} else {
			CHKiRet(localRet);
		}
	} else if(localRet == RS_RET_MODULE_ENTRY_POINT_NOT_FOUND) {
		pNew->beginCnfLoad = NULL; /* flag as non-present */
	} else {
		ABORT_FINALIZE(localRet);
	}
	/* ... and now the module-specific interfaces */
	switch(pNew->eType) {
		case eMOD_IN:
			CHKiRet((*pNew->modQueryEtryPt)((uchar*)"runInput", &pNew->mod.im.runInput));
			CHKiRet((*pNew->modQueryEtryPt)((uchar*)"willRun", &pNew->mod.im.willRun));
			CHKiRet((*pNew->modQueryEtryPt)((uchar*)"afterRun", &pNew->mod.im.afterRun));
			pNew->mod.im.bCanRun = 0;
			localRet = (*pNew->modQueryEtryPt)((uchar*)"newInpInst", &pNew->mod.im.newInpInst);
			if(localRet == RS_RET_MODULE_ENTRY_POINT_NOT_FOUND) {
				pNew->mod.im.newInpInst = NULL;
			} else if(localRet != RS_RET_OK) {
				ABORT_FINALIZE(localRet);
			}
			localRet = (*pNew->modQueryEtryPt)((uchar*)"doHUP", &pNew->doHUP);
			if(localRet != RS_RET_OK && localRet != RS_RET_MODULE_ENTRY_POINT_NOT_FOUND)
				ABORT_FINALIZE(localRet);

			break;
		case eMOD_OUT:
			CHKiRet((*pNew->modQueryEtryPt)((uchar*)"freeInstance", &pNew->freeInstance));
			CHKiRet((*pNew->modQueryEtryPt)((uchar*)"dbgPrintInstInfo", &pNew->dbgPrintInstInfo));
			CHKiRet((*pNew->modQueryEtryPt)((uchar*)"parseSelectorAct", &pNew->mod.om.parseSelectorAct));
			CHKiRet((*pNew->modQueryEtryPt)((uchar*)"tryResume", &pNew->tryResume));
			CHKiRet((*pNew->modQueryEtryPt)((uchar*)"createWrkrInstance",
				&pNew->mod.om.createWrkrInstance));
			CHKiRet((*pNew->modQueryEtryPt)((uchar*)"freeWrkrInstance",
				&pNew->mod.om.freeWrkrInstance));

			/* try load optional interfaces */
			localRet = (*pNew->modQueryEtryPt)((uchar*)"doHUP", &pNew->doHUP);
			if(localRet != RS_RET_OK && localRet != RS_RET_MODULE_ENTRY_POINT_NOT_FOUND)
				ABORT_FINALIZE(localRet);

			localRet = (*pNew->modQueryEtryPt)((uchar*)"doHUPWrkr", &pNew->doHUPWrkr);
			if(localRet != RS_RET_OK && localRet != RS_RET_MODULE_ENTRY_POINT_NOT_FOUND)
				ABORT_FINALIZE(localRet);

			localRet = (*pNew->modQueryEtryPt)((uchar*)"SetShutdownImmdtPtr",
				&pNew->mod.om.SetShutdownImmdtPtr);
			if(localRet != RS_RET_OK && localRet != RS_RET_MODULE_ENTRY_POINT_NOT_FOUND)
				ABORT_FINALIZE(localRet);

			pNew->mod.om.supportsTX = 1;
			localRet = (*pNew->modQueryEtryPt)((uchar*)"beginTransaction", &pNew->mod.om.beginTransaction);
			if(localRet == RS_RET_MODULE_ENTRY_POINT_NOT_FOUND) {
#ifdef _AIX
/* AIXPORT : typecaste the return type for AIX */
				pNew->mod.om.beginTransaction = (rsRetVal(*)(void*))dummyBeginTransaction;
#else
				pNew->mod.om.beginTransaction = dummyBeginTransaction;
#endif
				pNew->mod.om.supportsTX = 0;
			} else if(localRet != RS_RET_OK) {
				ABORT_FINALIZE(localRet);
			}

			localRet = (*pNew->modQueryEtryPt)((uchar*)"doAction",
				   &pNew->mod.om.doAction);
			if(localRet == RS_RET_MODULE_ENTRY_POINT_NOT_FOUND) {
				pNew->mod.om.doAction = NULL;
			} else if(localRet != RS_RET_OK) {
				ABORT_FINALIZE(localRet);
			}

			localRet = (*pNew->modQueryEtryPt)((uchar*)"commitTransaction",
				   &pNew->mod.om.commitTransaction);
			if(localRet == RS_RET_MODULE_ENTRY_POINT_NOT_FOUND) {
				pNew->mod.om.commitTransaction = NULL;
			} else if(localRet != RS_RET_OK) {
				ABORT_FINALIZE(localRet);
			}

			if(pNew->mod.om.doAction == NULL && pNew->mod.om.commitTransaction == NULL) {
				LogError(0, RS_RET_INVLD_OMOD,
					"module %s does neither provide doAction() "
					"nor commitTransaction() interface - cannot "
					"load", name);
				ABORT_FINALIZE(RS_RET_INVLD_OMOD);
			}

			if(pNew->mod.om.commitTransaction != NULL) {
				if(pNew->mod.om.doAction != NULL){
					LogError(0, RS_RET_INVLD_OMOD,
						"module %s provides both doAction() "
						"and commitTransaction() interface, using "
						"commitTransaction()", name);
					pNew->mod.om.doAction = NULL;
				}
				if(pNew->mod.om.beginTransaction == NULL){
					LogError(0, RS_RET_INVLD_OMOD,
						"module %s provides both commitTransaction() "
						"but does not provide beginTransaction() - "
						"cannot load", name);
					ABORT_FINALIZE(RS_RET_INVLD_OMOD);
				}
			}


			localRet = (*pNew->modQueryEtryPt)((uchar*)"endTransaction",
				   &pNew->mod.om.endTransaction);
			if(localRet == RS_RET_MODULE_ENTRY_POINT_NOT_FOUND) {
#ifdef _AIX
/* AIXPORT : typecaste the return type for AIX */
				pNew->mod.om.endTransaction = (rsRetVal(*)(void*))dummyEndTransaction;
#else
				pNew->mod.om.endTransaction = dummyEndTransaction;
#endif
			} else if(localRet != RS_RET_OK) {
				ABORT_FINALIZE(localRet);
			}

			localRet = (*pNew->modQueryEtryPt)((uchar*)"newActInst", &pNew->mod.om.newActInst);
			if(localRet == RS_RET_MODULE_ENTRY_POINT_NOT_FOUND) {
				pNew->mod.om.newActInst = dummynewActInst;
			} else if(localRet != RS_RET_OK) {
				ABORT_FINALIZE(localRet);
			}
			break;
		case eMOD_LIB:
			break;
		case eMOD_PARSER:
			localRet = (*pNew->modQueryEtryPt)((uchar*)"parse2",
				   &pNew->mod.pm.parse2);
			if(localRet == RS_RET_OK) {
				pNew->mod.pm.parse = NULL;
				CHKiRet((*pNew->modQueryEtryPt)((uchar*)"newParserInst",
					&pNew->mod.pm.newParserInst));
				CHKiRet((*pNew->modQueryEtryPt)((uchar*)"freeParserInst",
					&pNew->mod.pm.freeParserInst));
			} else if(localRet == RS_RET_MODULE_ENTRY_POINT_NOT_FOUND) {
				pNew->mod.pm.parse2 = NULL;
				pNew->mod.pm.newParserInst = NULL;
				pNew->mod.pm.freeParserInst = NULL;
				CHKiRet((*pNew->modQueryEtryPt)((uchar*)"parse", &pNew->mod.pm.parse));
			} else {
				ABORT_FINALIZE(localRet);
			}
			CHKiRet((*pNew->modQueryEtryPt)((uchar*)"GetParserName", &GetName));
			CHKiRet(GetName(&pName));
			CHKiRet(parserConstructViaModAndName(pNew, pName, NULL));
			break;
		case eMOD_STRGEN:
			/* first, we need to obtain the strgen object. We could not do that during
			 * init as that would have caused class bootstrap issues which are not
			 * absolutely necessary. Note that we can call objUse() multiple times, it
			 * handles that.
			 */
			CHKiRet(objUse(strgen, CORE_COMPONENT));
			CHKiRet((*pNew->modQueryEtryPt)((uchar*)"strgen", &pNew->mod.sm.strgen));
			CHKiRet((*pNew->modQueryEtryPt)((uchar*)"GetName", &GetName));
			CHKiRet(GetName(&pName));
			CHKiRet(strgen.Construct(&pStrgen));
			CHKiRet(strgen.SetName(pStrgen, pName));
			CHKiRet(strgen.SetModPtr(pStrgen, pNew));
			CHKiRet(strgen.ConstructFinalize(pStrgen));
			break;
		case eMOD_FUNCTION:
			CHKiRet((*pNew->modQueryEtryPt)((uchar*)"getFunctArray", &pNew->mod.fm.getFunctArray));
			int version;
			struct scriptFunct *functArray;
			pNew->mod.fm.getFunctArray(&version, &functArray);
			dbgprintf("LLL: %s\n", functArray[0].fname);
			addMod2List(version, functArray);
			break;
		case eMOD_ANY: /* this is mostly to keep the compiler happy! */
			DBGPRINTF("PROGRAM ERROR: eMOD_ANY set as module type\n");
			assert(0);
			break;
	}

	pNew->pszName = (uchar*) strdup((char*)name); /* we do not care if strdup() fails, we can accept that */
	pNew->pModHdlr = pModHdlr;
	if(pModHdlr == NULL) {
		pNew->eLinkType = eMOD_LINK_STATIC;
	} else {
		pNew->eLinkType = eMOD_LINK_DYNAMIC_LOADED;

		/* if we need to keep the linked module, save it */
		if (pNew->eKeepType == eMOD_KEEP) {
			/* see if we have this one already */
			for (pHandle = pHandles; pHandle; pHandle = pHandle->next) {
				if (!strcmp((char *)name, (char *)pHandle->pszName))
					break;
			}

			/* not found, create it */
			if (!pHandle) {
				if((pHandle = malloc(sizeof (*pHandle))) == NULL) {
					ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
				}
				if((pHandle->pszName = (uchar*) strdup((char*)name)) == NULL) {
					free(pHandle);
					ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
				}
				pHandle->pModHdlr = pModHdlr;
				pHandle->next = pHandles;

				pHandles = pHandle;
			}
		}
	}

	/* we initialized the structure, now let's add it to the linked list of modules */
	addModToGlblList(pNew);
	*pNewModule = pNew;

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pNew != NULL)
			moduleDestruct(pNew);
		*pNewModule = NULL;
	}

	RETiRet;
}

/* Print loaded modules. This is more or less a
 * debug or test aid, but anyhow I think it's worth it...
 * This only works if the dbgprintf() subsystem is initialized.
 * TODO: update for new input modules!
 */
static void modPrintList(void)
{
	modInfo_t *pMod;

	pMod = GetNxt(NULL);
	while(pMod != NULL) {
		dbgprintf("Loaded Module: Name='%s', IFVersion=%d, ",
			(char*) modGetName(pMod), pMod->iIFVers);
		dbgprintf("type=");
		switch(pMod->eType) {
		case eMOD_OUT:
			dbgprintf("output");
			break;
		case eMOD_IN:
			dbgprintf("input");
			break;
		case eMOD_LIB:
			dbgprintf("library");
			break;
		case eMOD_PARSER:
			dbgprintf("parser");
			break;
		case eMOD_STRGEN:
			dbgprintf("strgen");
			break;
		case eMOD_FUNCTION:
			dbgprintf("function");
			break;
		case eMOD_ANY: /* this is mostly to keep the compiler happy! */
			DBGPRINTF("PROGRAM ERROR: eMOD_ANY set as module type\n");
			assert(0);
			break;
		}
		dbgprintf(" module.\n");
		dbgprintf("Entry points:\n");
		dbgprintf("\tqueryEtryPt:        0x%lx\n", (unsigned long) pMod->modQueryEtryPt);
		dbgprintf("\tdbgPrintInstInfo:   0x%lx\n", (unsigned long) pMod->dbgPrintInstInfo);
		dbgprintf("\tfreeInstance:       0x%lx\n", (unsigned long) pMod->freeInstance);
		dbgprintf("\tbeginCnfLoad:       0x%lx\n", (unsigned long) pMod->beginCnfLoad);
		dbgprintf("\tSetModCnf:          0x%lx\n", (unsigned long) pMod->setModCnf);
		dbgprintf("\tcheckCnf:           0x%lx\n", (unsigned long) pMod->checkCnf);
		dbgprintf("\tactivateCnfPrePrivDrop: 0x%lx\n", (unsigned long) pMod->activateCnfPrePrivDrop);
		dbgprintf("\tactivateCnf:        0x%lx\n", (unsigned long) pMod->activateCnf);
		dbgprintf("\tfreeCnf:            0x%lx\n", (unsigned long) pMod->freeCnf);
		switch(pMod->eType) {
		case eMOD_OUT:
			dbgprintf("Output Module Entry Points:\n");
			dbgprintf("\tdoAction:           %p\n", pMod->mod.om.doAction);
			dbgprintf("\tparseSelectorAct:   %p\n", pMod->mod.om.parseSelectorAct);
			dbgprintf("\tnewActInst:         %p\n", (pMod->mod.om.newActInst == dummynewActInst) ?
								    NULL :  pMod->mod.om.newActInst);
			dbgprintf("\ttryResume:          %p\n", pMod->tryResume);
			dbgprintf("\tdoHUP:              %p\n", pMod->doHUP);
#ifdef _AIX
/* AIXPORT : typecaste the return type in AIX  */
			dbgprintf("\tBeginTransaction:   %p\n",
((pMod->mod.om.beginTransaction == (rsRetVal (*) (void*))dummyBeginTransaction) ?
								   NULL :  pMod->mod.om.beginTransaction));
			dbgprintf("\tEndTransaction:     %p\n",
			((pMod->mod.om.endTransaction == (rsRetVal (*)(void*))dummyEndTransaction) ?
				NULL :  pMod->mod.om.endTransaction));
#else
			dbgprintf("\tBeginTransaction:   %p\n",
				((pMod->mod.om.beginTransaction == dummyBeginTransaction) ?
				NULL :  pMod->mod.om.beginTransaction));
			dbgprintf("\tEndTransaction:     %p\n", ((pMod->mod.om.endTransaction == dummyEndTransaction) ?
				NULL :  pMod->mod.om.endTransaction));
#endif
			break;
		case eMOD_IN:
			dbgprintf("Input Module Entry Points\n");
			dbgprintf("\trunInput:           0x%lx\n", (unsigned long) pMod->mod.im.runInput);
			dbgprintf("\twillRun:            0x%lx\n", (unsigned long) pMod->mod.im.willRun);
			dbgprintf("\tafterRun:           0x%lx\n", (unsigned long) pMod->mod.im.afterRun);
			break;
		case eMOD_LIB:
			break;
		case eMOD_PARSER:
			dbgprintf("Parser Module Entry Points\n");
			dbgprintf("\tparse:              0x%lx\n", (unsigned long) pMod->mod.pm.parse);
			break;
		case eMOD_STRGEN:
			dbgprintf("Strgen Module Entry Points\n");
			dbgprintf("\tstrgen:            0x%lx\n", (unsigned long) pMod->mod.sm.strgen);
			break;
		case eMOD_FUNCTION:
			dbgprintf("Function Module Entry Points\n");
			dbgprintf("\tgetFunctArray:     0x%lx\n", (unsigned long) pMod->mod.fm.getFunctArray);
			break;
		case eMOD_ANY: /* this is mostly to keep the compiler happy! */
			break;
		}
		dbgprintf("\n");
		pMod = GetNxt(pMod); /* done, go next */
	}
}


/* HUP all modules that support it - except for actions, which
 * need (and have) specialised HUP handling.
 */
void
modDoHUP(void)
{
	modInfo_t *pMod;

	pMod = GetNxt(NULL);
	while(pMod != NULL) {
		if(pMod->eType != eMOD_OUT && pMod->doHUP != NULL) {
			DBGPRINTF("HUPing module %s\n", (char*) modGetName(pMod));
			pMod->doHUP(NULL);
		}
		pMod = GetNxt(pMod); /* done, go next */
	}
}


/* unlink and destroy a module. The caller must provide a pointer to the module
 * itself as well as one to its immediate predecessor.
 * rgerhards, 2008-02-26
 */
static rsRetVal
modUnlinkAndDestroy(modInfo_t **ppThis)
{
	DEFiRet;
	modInfo_t *pThis;

	assert(ppThis != NULL);
	pThis = *ppThis;
	assert(pThis != NULL);

	pthread_mutex_lock(&mutObjGlobalOp);

	/* first check if we are permitted to unload */
	if(pThis->eType == eMOD_LIB) {
		if(pThis->uRefCnt > 0) {
			dbgprintf("module %s NOT unloaded because it still has a refcount of %u\n",
				  pThis->pszName, pThis->uRefCnt);
#			ifdef DEBUG
			//modUsrPrintAll();
#			endif
			ABORT_FINALIZE(RS_RET_MODULE_STILL_REFERENCED);
		}
	}

	/* we need to unlink the module before we can destruct it -- rgerhards, 2008-02-26 */
	if(pThis->pPrev == NULL) {
		/* module is root, so we need to set a new root */
		pLoadedModules = pThis->pNext;
	} else {
		pThis->pPrev->pNext = pThis->pNext;
	}

	if(pThis->pNext == NULL) {
		pLoadedModulesLast = pThis->pPrev;
	} else {
		pThis->pNext->pPrev = pThis->pPrev;
	}

	/* finally, we are ready for the module to go away... */
	dbgprintf("Unloading module %s\n", modGetName(pThis));
	CHKiRet(modPrepareUnload(pThis));
	*ppThis = pThis->pNext;

	moduleDestruct(pThis);

finalize_it:
	pthread_mutex_unlock(&mutObjGlobalOp);
	RETiRet;
}


/* unload all loaded modules of a specific type (use eMOD_ALL if you want to
 * unload all module types). The unload happens only if the module is no longer
 * referenced. So some modules may survive this call.
 * rgerhards, 2008-03-11
 */
static rsRetVal
modUnloadAndDestructAll(eModLinkType_t modLinkTypesToUnload)
{
	DEFiRet;
	modInfo_t *pModCurr; /* module currently being processed */

	pModCurr = GetNxt(NULL);
	while(pModCurr != NULL) {
		if(modLinkTypesToUnload == eMOD_LINK_ALL || pModCurr->eLinkType == modLinkTypesToUnload) {
			if(modUnlinkAndDestroy(&pModCurr) == RS_RET_MODULE_STILL_REFERENCED) {
				pModCurr = GetNxt(pModCurr);
			} else {
				/* Note: if the module was successfully unloaded, it has updated the
				 * pModCurr pointer to the next module. However, the unload process may
				 * still have indirectly referenced the pointer list in a way that the
				 * unloaded module is not aware of. So we restart the unload process
				 * to make sure we do not fall into a trap (what we did ;)). The
				 * performance toll is minimal. -- rgerhards, 2008-04-28
				 */
				pModCurr = GetNxt(NULL);
			}
		} else {
			pModCurr = GetNxt(pModCurr);
		}
	}

#	ifdef DEBUG
	/* DEV DEBUG only!
		if(pLoadedModules != NULL) {
			dbgprintf("modules still loaded after module.UnloadAndDestructAll:\n");
			modUsrPrintAll();
		}
	*/
#	endif

	RETiRet;
}

/* find module with given name in global list */
static rsRetVal
findModule(uchar *pModName, int iModNameLen, modInfo_t **pMod)
{
	modInfo_t *pModInfo;
	uchar *pModNameCmp;
	DEFiRet;

	pModInfo = GetNxt(NULL);
	while(pModInfo != NULL) {
		if(!strncmp((char *) pModName, (char *) (pModNameCmp = modGetName(pModInfo)), iModNameLen) &&
		   (!*(pModNameCmp + iModNameLen) || !strcmp((char *) pModNameCmp + iModNameLen, ".so"))) {
			dbgprintf("Module '%s' found\n", pModName);
			break;
		}
		pModInfo = GetNxt(pModInfo);
	}
	*pMod = pModInfo;
	RETiRet;
}


/* load a module and initialize it, based on doModLoad() from conf.c
 * rgerhards, 2008-03-05
 * varmojfekoj added support for dynamically loadable modules on 2007-08-13
 * rgerhards, 2007-09-25: please note that the non-threadsafe function dlerror() is
 * called below. This is ok because modules are currently only loaded during
 * configuration file processing, which is executed on a single thread. Should we
 * change that design at any stage (what is unlikely), we need to find a
 * replacement.
 * rgerhards, 2011-04-27:
 * Parameter "bConfLoad" tells us if the load was triggered by a config handler, in
 * which case we need to tie the loaded module to the current config. If bConfLoad == 0,
 * the system loads a module for internal reasons, this is not directly tied to a
 * configuration. We could also think if it would be useful to add only certain types
 * of modules, but the current implementation at least looks simpler.
 * Note: pvals = NULL means legacy config system
 */
static rsRetVal ATTR_NONNULL(1)
Load(uchar *const pModName, const sbool bConfLoad, struct nvlst *const lst)
{
	size_t iPathLen, iModNameLen;
	int bHasExtension;
	void *pModHdlr, *pModInit;
	modInfo_t *pModInfo;
	cfgmodules_etry_t *pNew = NULL;
	cfgmodules_etry_t *pLast = NULL;
	uchar *pModDirCurr, *pModDirNext;
	int iLoadCnt;
	struct dlhandle_s *pHandle = NULL;
#	ifdef PATH_MAX
	uchar pathBuf[PATH_MAX+1];
#	else
	uchar pathBuf[4096];
#	endif
	uchar *pPathBuf = pathBuf;
	size_t lenPathBuf = sizeof(pathBuf);
	rsRetVal localRet;
	cstr_t *load_err_msg = NULL;
	DEFiRet;

	assert(pModName != NULL);
	DBGPRINTF("Requested to load module '%s'\n", pModName);

	iModNameLen = strlen((char*)pModName);
	/* overhead for a full path is potentially 1 byte for a slash,
	 * three bytes for ".so" and one byte for '\0'.
	 */
#	define PATHBUF_OVERHEAD 1 + iModNameLen + 3 + 1

	pthread_mutex_lock(&mutObjGlobalOp);

	if(iModNameLen > 3 && !strcmp((char *) pModName + iModNameLen - 3, ".so")) {
		iModNameLen -= 3;
		bHasExtension = RSTRUE;
	} else
		bHasExtension = RSFALSE;

	CHKiRet(findModule(pModName, iModNameLen, &pModInfo));
	if(pModInfo != NULL) {
		DBGPRINTF("Module '%s' already loaded\n", pModName);
		if(bConfLoad) {
			localRet = readyModForCnf(pModInfo, &pNew, &pLast);
			if(pModInfo->setModCnf != NULL && localRet == RS_RET_OK) {
				if(!strncmp((char*)pModName, "builtin:", sizeof("builtin:")-1)) {
					if(pModInfo->bSetModCnfCalled) {
						LogError(0, RS_RET_DUP_PARAM,
						    "parameters for built-in module %s already set - ignored\n",
						    pModName);
						ABORT_FINALIZE(RS_RET_DUP_PARAM);
					} else {
						/* for built-in moules, we need to call setModConf,
						 * because there is no way to set parameters at load
						 * time for obvious reasons...
						 */
						if(lst != NULL)
							pModInfo->setModCnf(lst);
						pModInfo->bSetModCnfCalled = 1;
					}
				} else {
					/* regular modules need to be added to conf list (for
					 * builtins, this happend during initial load).
					 */
					addModToCnfList(&pNew, pLast);
				}
			}
		}
		FINALIZE;
	}

	pModDirCurr = (uchar *)((pModDir == NULL) ? _PATH_MODDIR : (char *)pModDir);
	pModDirNext = NULL;
	pModHdlr    = NULL;
	iLoadCnt    = 0;
	do {	/* now build our load module name */
		if(*pModName == '/' || *pModName == '.') {
			if(lenPathBuf < PATHBUF_OVERHEAD) {
				if(pPathBuf != pathBuf) /* already malloc()ed memory? */
					free(pPathBuf);
				/* we always alloc enough memory for everything we potentiall need to add */
				lenPathBuf = PATHBUF_OVERHEAD;
				CHKmalloc(pPathBuf = malloc(lenPathBuf));
			}
			*pPathBuf = '\0';	/* we do not need to append the path - its already in the module name */
			iPathLen = 0;
		} else {
			*pPathBuf = '\0';

			iPathLen = strlen((char *)pModDirCurr);
			pModDirNext = (uchar *)strchr((char *)pModDirCurr, ':');
			if(pModDirNext)
				iPathLen = (size_t)(pModDirNext - pModDirCurr);

			if(iPathLen == 0) {
				if(pModDirNext) {
					pModDirCurr = pModDirNext + 1;
					continue;
				}
				break;
			} else if(iPathLen > lenPathBuf - PATHBUF_OVERHEAD) {
				if(pPathBuf != pathBuf) /* already malloc()ed memory? */
					free(pPathBuf);
				/* we always alloc enough memory for everything we potentiall need to add */
				lenPathBuf = iPathLen + PATHBUF_OVERHEAD;
				CHKmalloc(pPathBuf = malloc(lenPathBuf));
			}

			memcpy((char *) pPathBuf, (char *)pModDirCurr, iPathLen);
			if((pPathBuf[iPathLen - 1] != '/')) {
				/* we have space, made sure in previous check */
				pPathBuf[iPathLen++] = '/';
			}
			pPathBuf[iPathLen] = '\0';

			if(pModDirNext)
				pModDirCurr = pModDirNext + 1;
		}

		/* ... add actual name ... */
		strncat((char *) pPathBuf, (char *) pModName, lenPathBuf - iPathLen - 1);

		/* now see if we have an extension and, if not, append ".so" */
		if(!bHasExtension) {
			/* we do not have an extension and so need to add ".so"
			 * TODO: I guess this is highly importable, so we should change the
			 * algo over time... -- rgerhards, 2008-03-05
			 */
			strncat((char *) pPathBuf, ".so", lenPathBuf - strlen((char*) pPathBuf) - 1);
		}

		/* complete load path constructed, so ... GO! */
		dbgprintf("loading module '%s'\n", pPathBuf);

		/* see if we have this one already */
		for (pHandle = pHandles; pHandle; pHandle = pHandle->next) {
			if (!strcmp((char *)pModName, (char *)pHandle->pszName)) {
				pModHdlr = pHandle->pModHdlr;
				break;
			}
		}

		/* not found, try to dynamically link it */
		if (!pModHdlr) {
			pModHdlr = dlopen((char *) pPathBuf, RTLD_NOW);
		}

		if(pModHdlr == NULL) {
			char errmsg[4096];
			snprintf(errmsg, sizeof(errmsg), "%strying to load module %s: %s",
				(load_err_msg == NULL) ? "" : "  ////////  ",
				pPathBuf, dlerror());
			if(load_err_msg == NULL) {
				rsCStrConstructFromszStr(&load_err_msg, (uchar*)errmsg);
			} else {
				rsCStrAppendStr(load_err_msg, (uchar*)errmsg);
			}
		}

		iLoadCnt++;
	
	} while(pModHdlr == NULL && *pModName != '/' && pModDirNext);

	if(load_err_msg != NULL) {
		cstrFinalize(load_err_msg);
	}

	if(!pModHdlr) {
		LogError(0, RS_RET_MODULE_LOAD_ERR_DLOPEN, "could not load module '%s', errors: %s", pModName,
			(load_err_msg == NULL) ? "NONE SEEN???" : (const char*) cstrGetSzStrNoNULL(load_err_msg));
		ABORT_FINALIZE(RS_RET_MODULE_LOAD_ERR_DLOPEN);
	}
	if(!(pModInit = dlsym(pModHdlr, "modInit"))) {
		LogError(0, RS_RET_MODULE_LOAD_ERR_NO_INIT,
			 	"could not load module '%s', dlsym: %s\n", pPathBuf, dlerror());
		dlclose(pModHdlr);
		ABORT_FINALIZE(RS_RET_MODULE_LOAD_ERR_NO_INIT);
	}
	if((iRet = doModInit((rsRetVal(*)(int,int*,rsRetVal(**)(),rsRetVal(*)(),struct modInfo_s*))pModInit,
		(uchar*) pModName, pModHdlr, &pModInfo)) != RS_RET_OK) {
		LogError(0, RS_RET_MODULE_LOAD_ERR_INIT_FAILED,
			"could not load module '%s', rsyslog error %d\n", pPathBuf, iRet);
		dlclose(pModHdlr);
		ABORT_FINALIZE(RS_RET_MODULE_LOAD_ERR_INIT_FAILED);
	}

	if(bConfLoad) {
		readyModForCnf(pModInfo, &pNew, &pLast);
		if(pModInfo->setModCnf != NULL) {
			if(lst != NULL) {
				localRet = pModInfo->setModCnf(lst);
				if(localRet != RS_RET_OK) {
					LogError(0, localRet,
						"module '%s', failed processing config parameters",
						pPathBuf);
					ABORT_FINALIZE(localRet);
				}
			}
			pModInfo->bSetModCnfCalled = 1;
		}
		addModToCnfList(&pNew, pLast);
	}

finalize_it:
	if(load_err_msg != NULL) {
		cstrDestruct(&load_err_msg);
	}
	if(pPathBuf != pathBuf) /* used malloc()ed memory? */
		free(pPathBuf);
	if(iRet != RS_RET_OK)
		abortCnfUse(&pNew);
	pthread_mutex_unlock(&mutObjGlobalOp);
	RETiRet;
}


/* the v6+ way of loading modules: process a "module(...)" directive.
 * rgerhards, 2012-06-20
 */
rsRetVal
modulesProcessCnf(struct cnfobj *o)
{
	struct cnfparamvals *pvals;
	uchar *cnfModName = NULL;
	int typeIdx;
	DEFiRet;

	pvals = nvlstGetParams(o->nvlst, &pblk, NULL);
	if(pvals == NULL) {
		ABORT_FINALIZE(RS_RET_ERR);
	}
	DBGPRINTF("modulesProcessCnf params:\n");
	cnfparamsPrint(&pblk, pvals);
	typeIdx = cnfparamGetIdx(&pblk, "load");
	if(pvals[typeIdx].bUsed == 0) {
		LogError(0, RS_RET_CONF_RQRD_PARAM_MISSING, "module type missing");
		ABORT_FINALIZE(RS_RET_CONF_RQRD_PARAM_MISSING);
	}

	cnfModName = (uchar*)es_str2cstr(pvals[typeIdx].val.d.estr, NULL);
	iRet = Load(cnfModName, 1, o->nvlst);
	
finalize_it:
	free(cnfModName);
	cnfparamvalsDestruct(pvals, &pblk);
	RETiRet;
}


/* set the default module load directory. A NULL value may be provided, in
 * which case any previous value is deleted but no new one set. The caller-provided
 * string is duplicated. If it needs to be freed, that's the caller's duty.
 * rgerhards, 2008-03-07
 */
static rsRetVal
SetModDir(uchar *pszModDir)
{
	DEFiRet;

	dbgprintf("setting default module load directory '%s'\n", pszModDir);
	if(pModDir != NULL) {
		free(pModDir);
	}

	pModDir = (uchar*) strdup((char*)pszModDir);

	RETiRet;
}


/* Reference-Counting object access: add 1 to the current reference count. Must be
 * called by anyone interested in using a module. -- rgerhards, 20080-03-10
 */
static rsRetVal
Use(const char *srcFile, modInfo_t *pThis)
{
	DEFiRet;

	assert(pThis != NULL);
	pThis->uRefCnt++;
	dbgprintf("source file %s requested reference for module '%s', reference count now %u\n",
		  srcFile, pThis->pszName, pThis->uRefCnt);

#	ifdef DEBUG
	modUsrAdd(pThis, srcFile);
#	endif

	RETiRet;

}


/* Reference-Counting object access: subract one from the current refcount. Must
 * by called by anyone who no longer needs a module. If count reaches 0, the
 * module is unloaded. -- rgerhards, 20080-03-10
 */
static rsRetVal
Release(const char *srcFile, modInfo_t **ppThis)
{
	DEFiRet;
	modInfo_t *pThis;

	assert(ppThis != NULL);
	pThis = *ppThis;
	assert(pThis != NULL);
	if(pThis->uRefCnt == 0) {
		/* oops, we are already at 0? */
		dbgprintf("internal error: module '%s' already has a refcount of 0 (released by %s)!\n",
			  pThis->pszName, srcFile);
	} else {
		--pThis->uRefCnt;
		dbgprintf("file %s released module '%s', reference count now %u\n",
			  srcFile, pThis->pszName, pThis->uRefCnt);
#		ifdef DEBUG
		modUsrDel(pThis, srcFile);
		modUsrPrint(pThis);
#		endif
	}

	if(pThis->uRefCnt == 0) {
		/* we have a zero refcount, so we must unload the module */
		dbgprintf("module '%s' has zero reference count, unloading...\n", pThis->pszName);
		modUnlinkAndDestroy(&pThis);
		/* we must NOT do a *ppThis = NULL, because ppThis now points into freed memory!
		 * If in doubt, see obj.c::ReleaseObj() for how we are called.
		 */
	}

	RETiRet;

}


/* exit our class
 * rgerhards, 2008-03-11
 */
BEGINObjClassExit(module, OBJ_IS_LOADABLE_MODULE) /* CHANGE class also in END MACRO! */
CODESTARTObjClassExit(module)
	/* release objects we no longer need */
	free(pModDir);
#	ifdef DEBUG
	modUsrPrintAll(); /* debug aid - TODO: integrate with debug.c, at least the settings! */
#	endif
ENDObjClassExit(module)


/* queryInterface function
 * rgerhards, 2008-03-05
 */
BEGINobjQueryInterface(module)
CODESTARTobjQueryInterface(module)
	if(pIf->ifVersion != moduleCURR_IF_VERSION) { /* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->GetNxt = GetNxt;
	pIf->GetNxtCnfType = GetNxtCnfType;
	pIf->GetName = modGetName;
	pIf->GetStateName = modGetStateName;
	pIf->PrintList = modPrintList;
	pIf->FindWithCnfName = FindWithCnfName;
	pIf->UnloadAndDestructAll = modUnloadAndDestructAll;
	pIf->doModInit = doModInit;
	pIf->SetModDir = SetModDir;
	pIf->Load = Load;
	pIf->Use = Use;
	pIf->Release = Release;
finalize_it:
ENDobjQueryInterface(module)


/* Initialize our class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-03-05
 */
BEGINAbstractObjClassInit(module, 1, OBJ_IS_CORE_MODULE) /* class, version - CHANGE class also in END MACRO! */
	uchar *pModPath;

	/* use any module load path specified in the environment */
	if((pModPath = (uchar*) getenv("RSYSLOG_MODDIR")) != NULL) {
		SetModDir(pModPath);
	}

	/* now check if another module path was set via the command line (-M)
	 * if so, that overrides the environment. Please note that we must use
	 * a global setting here because the command line parser can NOT call
	 * into the module object, because it is not initialized at that point. So
	 * instead a global setting is changed and we pick it up as soon as we
	 * initialize -- rgerhards, 2008-04-04
	 */
	if(glblModPath != NULL) {
		SetModDir(glblModPath);
	}

	/* request objects we use */
ENDObjClassInit(module)

/* vi:set ai:
 */
