/* The config file handler (not yet a real object)
 *
 * This file is based on an excerpt from syslogd.c, which dates back
 * much later. I began the file on 2008-02-19 as part of the modularization
 * effort. Over time, a clean abstration will become even more important
 * because the config file handler will by dynamically be loaded and be
 * kept in memory only as long as the config file is actually being
 * processed. Thereafter, it shall be unloaded. -- rgerhards
 * Please note that the original syslogd.c source was under BSD license
 * at the time of the rsyslog fork from sysklogd.
 *
 * Copyright 2008-2016 Rainer Gerhards and Adiscon GmbH.
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
#define CFGLNSIZ 64*1024 /* the maximum size of a configuraton file line, after re-combination */
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <dirent.h>
#include <glob.h>
#include <sys/types.h>
#ifdef HAVE_LIBGEN_H
#	ifndef OS_SOLARIS
#		include <libgen.h>
#	endif
#endif

#include "rsyslog.h"
#include "dirty.h"
#include "parse.h"
#include "action.h"
#include "template.h"
#include "cfsysline.h"
#include "modules.h"
#include "outchannel.h"
#include "stringbuf.h"
#include "conf.h"
#include "stringbuf.h"
#include "srUtils.h"
#include "errmsg.h"
#include "net.h"
#include "ruleset.h"
#include "rsconf.h"
#include "unicode-helper.h"
#include "rainerscript.h"

#ifdef OS_SOLARIS
#	define NAME_MAX MAXNAMELEN
#endif

/* forward definitions */


/* static data */
DEFobjStaticHelpers
DEFobjCurrIf(module)
DEFobjCurrIf(net)
DEFobjCurrIf(ruleset)

int bConfStrictScoping = 0;	/* force strict scoping during config processing? */


/* The following module-global variables are used for building
 * tag and host selector lines during startup and config reload.
 * This is stored as a global variable pool because of its ease. It is
 * also fairly compatible with multi-threading as the stratup code must
 * be run in a single thread anyways. So there can be no race conditions.
 * rgerhards 2005-10-18
 */
EHostnameCmpMode eDfltHostnameCmpMode = HN_NO_COMP;
cstr_t *pDfltHostnameCmp = NULL;
cstr_t *pDfltProgNameCmp = NULL;


/* process a $ModLoad config line.  */
static rsRetVal
doModLoad(uchar **pp, __attribute__((unused)) void* pVal)
{
	DEFiRet;
	uchar szName[512];
	uchar *pModName;

	ASSERT(pp != NULL);
	ASSERT(*pp != NULL);

	skipWhiteSpace(pp); /* skip over any whitespace */
	if(getSubString(pp, (char*) szName, sizeof(szName), ' ')  != 0) {
		LogError(0, RS_RET_NOT_FOUND, "could not extract module name");
		ABORT_FINALIZE(RS_RET_NOT_FOUND);
	}
	skipWhiteSpace(pp); /* skip over any whitespace */

	/* this below is a quick and dirty hack to provide compatibility with the
	 * $ModLoad MySQL forward compatibility statement. This needs to be supported
	 * for legacy format.
	 */
	if(!strcmp((char*) szName, "MySQL"))
		pModName = (uchar*) "ommysql.so";
	else
		pModName = szName;

	CHKiRet(module.Load(pModName, 1, NULL));

finalize_it:
	RETiRet;
}


/* remove leading spaces from name; this "fixes" some anomalies in
 * getSubString(), but I was not brave enough to fix the former as
 * it has many other callers... -- rgerhards, 2013-05-27
 */
static void
ltrim(char *src)
{
	char *dst = src;
	while(isspace(*src))
		++src; /*SKIP*/;
	if(dst != src) {
		while(*src != '\0')
			*dst++ = *src++;
		*dst = '\0';
	}
}

/* parse and interpret a $-config line that starts with
 * a name (this is common code). It is parsed to the name
 * and then the proper sub-function is called to handle
 * the actual directive.
 * rgerhards 2004-11-17
 * rgerhards 2005-06-21: previously only for templates, now
 *    generalized.
 */
static rsRetVal
doNameLine(uchar **pp, void* pVal)
{
	DEFiRet;
	uchar *p;
	enum eDirective eDir;
	char szName[128];

	ASSERT(pp != NULL);
	p = *pp;
	ASSERT(p != NULL);

	eDir = (enum eDirective) pVal;	/* this time, it actually is NOT a pointer! */

	if(getSubString(&p, szName, sizeof(szName), ',')  != 0) {
		LogError(0, RS_RET_NOT_FOUND, "Invalid config line: could not extract name - line ignored");
		ABORT_FINALIZE(RS_RET_NOT_FOUND);
	}
	ltrim(szName);
	if(*p == ',')
		++p; /* comma was eaten */
	
	/* we got the name - now we pass name & the rest of the string
	 * to the subfunction. It makes no sense to do further
	 * parsing here, as this is in close interaction with the
	 * respective subsystem. rgerhards 2004-11-17
	 */
	
	switch(eDir) {
		case DIR_TEMPLATE:
			tplAddLine(loadConf, szName, &p);
			break;
		case DIR_OUTCHANNEL:
			ochAddLine(szName, &p);
			break;
		case DIR_ALLOWEDSENDER:
			net.addAllowedSenderLine(szName, &p);
			break;
		default:/* we do this to avoid compiler warning - not all
			 * enum values call this function, so an incomplete list
			 * is quite ok (but then we should not run into this code,
			 * so at least we log a debug warning).
			 */
			dbgprintf("INTERNAL ERROR: doNameLine() called with invalid eDir %d.\n",
				eDir);
			break;
	}

	*pp = p;

finalize_it:
	RETiRet;
}


/* Parse and interpret a system-directive in the config line
 * A system directive is one that starts with a "$" sign. It offers
 * extended configuration parameters.
 * 2004-11-17 rgerhards
 */
static rsRetVal
cfsysline(uchar *p)
{
	DEFiRet;
	uchar szCmd[64];

	ASSERT(p != NULL);
	errno = 0;
	if(getSubString(&p, (char*) szCmd, sizeof(szCmd), ' ')  != 0) {
		LogError(0, RS_RET_NOT_FOUND, "Invalid $-configline "
			"- could not extract command - line ignored\n");
		ABORT_FINALIZE(RS_RET_NOT_FOUND);
	}

	/* we now try and see if we can find the command in the registered
	 * list of cfsysline handlers. -- rgerhards, 2007-07-31
	 */
	CHKiRet(processCfSysLineCommand(szCmd, &p));

	/* now check if we have some extra characters left on the line - that
	 * should not be the case. Whitespace is OK, but everything else should
	 * trigger a warning (that may be an indication of undesired behaviour).
	 * An exception, of course, are comments (starting with '#').
	 * rgerhards, 2007-07-04
	 */
	skipWhiteSpace(&p);

	if(*p && *p != '#') { /* we have a non-whitespace, so let's complain */
		LogError(0, NO_ERRCODE,
		         "error: extra characters in config line ignored: '%s'", p);
	}

finalize_it:
	RETiRet;
}


/* Helper to cfline() and its helpers. Parses a template name
 * from an "action" line. Must be called with the Line pointer
 * pointing to the first character after the semicolon.
 * rgerhards 2004-11-19
 * changed function to work with OMSR. -- rgerhards, 2007-07-27
 * the default template is to be used when no template is specified.
 */
rsRetVal cflineParseTemplateName(uchar** pp, omodStringRequest_t *pOMSR, int iEntry, int iTplOpts, uchar *dfltTplName)
{
	uchar *p;
	uchar *tplName = NULL;
	cstr_t *pStrB = NULL;
	DEFiRet;

	ASSERT(pp != NULL);
	ASSERT(*pp != NULL);
	ASSERT(pOMSR != NULL);

	p =*pp;
	/* a template must follow - search it and complain, if not found */
	skipWhiteSpace(&p);
	if(*p == ';')
		++p; /* eat it */
	else if(*p != '\0' && *p != '#') {
		LogError(0, RS_RET_ERR, "invalid character in selector line - ';template' expected");
		ABORT_FINALIZE(RS_RET_ERR);
	}

	skipWhiteSpace(&p); /* go to begin of template name */

	if(*p == '\0' || *p == '#') {
		/* no template specified, use the default */
		/* TODO: check NULL ptr */
		tplName = (uchar*) strdup((char*)dfltTplName);
	} else {
		/* template specified, pick it up */
		CHKiRet(cstrConstruct(&pStrB));

		/* now copy the string */
		while(*p && *p != '#' && !isspace((int) *p)) {
			CHKiRet(cstrAppendChar(pStrB, *p));
			++p;
		}
		cstrFinalize(pStrB);
		CHKiRet(cstrConvSzStrAndDestruct(&pStrB, &tplName, 0));
	}

	CHKiRet(OMSRsetEntry(pOMSR, iEntry, tplName, iTplOpts));

finalize_it:
	if(iRet != RS_RET_OK) {
		free(tplName);
		if(pStrB != NULL)
			cstrDestruct(&pStrB);
	}

	*pp = p;

	RETiRet;
}

/* Helper to cfline(). Parses a file name up until the first
 * comma and then looks for the template specifier. Tries
 * to find that template.
 * rgerhards 2004-11-18
 * parameter pFileName must point to a buffer large enough
 * to hold the largest possible filename.
 * rgerhards, 2007-07-25
 * updated to include OMSR pointer -- rgerhards, 2007-07-27
 * updated to include template name -- rgerhards, 2008-03-28
 * rgerhards, 2010-01-19: file names end at the first space
 */
rsRetVal
cflineParseFileName(uchar* p, uchar *pFileName, omodStringRequest_t *pOMSR, int iEntry, int iTplOpts, uchar *pszTpl)
{
	register uchar *pName;
	int i;
	DEFiRet;

	ASSERT(pOMSR != NULL);

	pName = pFileName;
	i = 1; /* we start at 1 so that we reseve space for the '\0'! */
	while(*p && *p != ';' && *p != ' ' && i < MAXFNAME) {
		*pName++ = *p++;
		++i;
	}
	*pName = '\0';

	iRet = cflineParseTemplateName(&p, pOMSR, iEntry, iTplOpts, pszTpl);

	RETiRet;
}


/* Decode a traditional PRI filter */
/* GPLv3 - stems back to sysklogd */
rsRetVal DecodePRIFilter(uchar *pline, uchar pmask[])
{
	uchar *p;
	register uchar *q;
	register int i, i2;
	uchar *bp;
	int pri; /* this MUST be int, as -1 is used to convey an error state */
	int singlpri = 0;
	int ignorepri = 0;
	uchar buf[2048]; /* buffer for facility and priority names */
	uchar xbuf[200];
	DEFiRet;

	ASSERT(pline != NULL);

	dbgprintf("Decoding traditional PRI filter '%s'\n", pline);

	for (i = 0; i <= LOG_NFACILITIES; i++) {
		pmask[i] = TABLE_NOPRI;
	}

	/* scan through the list of selectors */
	for (p = pline; *p && *p != '\t' && *p != ' ';) {
		/* find the end of this facility name list */
		for (q = p; *q && *q != '\t' && *q++ != '.'; )
			continue;

		/* collect priority name */
		for (bp = buf; *q && !strchr("\t ,;", *q) && bp < buf+sizeof(buf)-1 ; )
			*bp++ = *q++;
		*bp = '\0';

		/* skip cruft */
		if(*q) {
			while (strchr(",;", *q))
				q++;
		}

		/* decode priority name */
		if ( *buf == '!' ) {
			ignorepri = 1;
			/* copy below is ok, we can NOT go off the allocated area */
			for (bp=buf; *(bp+1); bp++)
				*bp=*(bp+1);
			*bp='\0';
		} else {
			ignorepri = 0;
		}
		if ( *buf == '=' ) {
			singlpri = 1;
			pri = decodeSyslogName(&buf[1], syslogPriNames);
		}
		else { singlpri = 0;
			pri = decodeSyslogName(buf, syslogPriNames);
		}

		if (pri < 0) {
			snprintf((char*) xbuf, sizeof(xbuf), "unknown priority name \"%s\"", buf);
			LogError(0, RS_RET_ERR, "%s", xbuf);
			return RS_RET_ERR;
		}

		/* scan facilities */
		while (*p && !strchr("\t .;", *p)) {
			for (bp = buf; *p && !strchr("\t ,;.", *p) && bp < buf+sizeof(buf)-1 ; )
				*bp++ = *p++;
			*bp = '\0';
			if (*buf == '*') {
				for (i = 0; i <= LOG_NFACILITIES; i++) {
					if ( pri == INTERNAL_NOPRI ) {
						if ( ignorepri )
							pmask[i] = TABLE_ALLPRI;
						else
							pmask[i] = TABLE_NOPRI;
					}
					else if ( singlpri ) {
						if ( ignorepri )
				  			pmask[i] &= ~(1<<pri);
						else
				  			pmask[i] |= (1<<pri);
					} else {
						if ( pri == TABLE_ALLPRI ) {
							if ( ignorepri )
								pmask[i] = TABLE_NOPRI;
							else
								pmask[i] = TABLE_ALLPRI;
						} else {
							if ( ignorepri )
								for (i2= 0; i2 <= pri; ++i2)
									pmask[i] &= ~(1<<i2);
							else
								for (i2= 0; i2 <= pri; ++i2)
									pmask[i] |= (1<<i2);
						}
					}
				}
			} else {
				i = decodeSyslogName(buf, syslogFacNames);
				if (i < 0) {

					snprintf((char*) xbuf, sizeof(xbuf), "unknown facility name \"%s\"", buf);
					LogError(0, RS_RET_ERR, "%s", xbuf);
					return RS_RET_ERR;
				}

				if ( pri == INTERNAL_NOPRI ) {
					if ( ignorepri )
						pmask[i >> 3] = TABLE_ALLPRI;
					else
						pmask[i >> 3] = TABLE_NOPRI;
				} else if ( singlpri ) {
					if ( ignorepri )
						pmask[i >> 3] &= ~(1<<pri);
					else
						pmask[i >> 3] |= (1<<pri);
				} else {
					if ( pri == TABLE_ALLPRI ) {
						if ( ignorepri )
							pmask[i >> 3] = TABLE_NOPRI;
						else
							pmask[i >> 3] = TABLE_ALLPRI;
					} else {
						if ( ignorepri )
							for (i2= 0; i2 <= pri; ++i2)
								pmask[i >> 3] &= ~(1<<i2);
						else
							for (i2= 0; i2 <= pri; ++i2)
								pmask[i >> 3] |= (1<<i2);
					}
				}
			}
			while (*p == ',' || *p == ' ')
				p++;
		}

		p = q;
	}

	RETiRet;
}


/* process the action part of a selector line
 * rgerhards, 2007-08-01
 */
rsRetVal cflineDoAction(rsconf_t *conf, uchar **p, action_t **ppAction)
{
	modInfo_t *pMod;
	cfgmodules_etry_t *node;
	omodStringRequest_t *pOMSR;
	int bHadWarning = 0;
	action_t *pAction = NULL;
	void *pModData;
	DEFiRet;

	ASSERT(p != NULL);
	ASSERT(ppAction != NULL);

	/* loop through all modules and see if one picks up the line */
	node = module.GetNxtCnfType(conf, NULL, eMOD_OUT);
	/* Note: clang static analyzer reports that node maybe == NULL. However, this is
	 * not possible, because we have the built-in output modules which are always
	 * present. Anyhow, we guard this by an assert. -- rgerhards, 2010-12-16
	 */
	assert(node != NULL);
	while(node != NULL) {
		pOMSR = NULL;
		pMod = node->pMod;
		iRet = pMod->mod.om.parseSelectorAct(p, &pModData, &pOMSR);
		dbgprintf("tried selector action for %s: %d\n", module.GetName(pMod), iRet);
		if(iRet == RS_RET_OK_WARN) {
			bHadWarning = 1;
			iRet = RS_RET_OK;
		}
		if(iRet == RS_RET_OK) {
			if((iRet = addAction(&pAction, pMod, pModData, pOMSR, NULL, NULL)) == RS_RET_OK) {
				/* here check if the module is compatible with select features
				 * (currently, we have no such features!) */
				conf->actions.nbrActions++;	/* one more active action! */
			}
			break;
		} else if(iRet != RS_RET_CONFLINE_UNPROCESSED) {
			/* In this case, the module would have handled the config
			 * line, but some error occured while doing so. This error should
			 * already by reported by the module. We do not try any other
			 * modules on this line, because we found the right one.
			 * rgerhards, 2007-07-24
			 */
			dbgprintf("error %d parsing config line\n", (int) iRet);
			break;
		}
		node = module.GetNxtCnfType(conf, node, eMOD_OUT);
	}

	*ppAction = pAction;
	if(iRet == RS_RET_OK && bHadWarning)
		iRet = RS_RET_OK_WARN;
	RETiRet;
}


/* return the current number of active actions
 * rgerhards, 2008-07-28
 */
static rsRetVal
GetNbrActActions(rsconf_t *conf, int *piNbrActions)
{
	DEFiRet;
	assert(piNbrActions != NULL);
	*piNbrActions = conf->actions.nbrActions;
	RETiRet;
}


/* queryInterface function
 * rgerhards, 2008-02-29
 */
BEGINobjQueryInterface(conf)
CODESTARTobjQueryInterface(conf)
	if(pIf->ifVersion != confCURR_IF_VERSION) { /* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->doNameLine = doNameLine;
	pIf->cfsysline = cfsysline;
	pIf->doModLoad = doModLoad;
	pIf->GetNbrActActions = GetNbrActActions;

finalize_it:
ENDobjQueryInterface(conf)


/* Reset config variables to default values.
 * rgerhards, 2010-07-23
 */
static rsRetVal
resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal)
{
	bConfStrictScoping = 0;
	return RS_RET_OK;
}


/* exit our class
 * rgerhards, 2008-03-11
 */
BEGINObjClassExit(conf, OBJ_IS_CORE_MODULE) /* CHANGE class also in END MACRO! */
CODESTARTObjClassExit(conf)
	/* free no-longer needed module-global variables */
	if(pDfltHostnameCmp != NULL) {
		rsCStrDestruct(&pDfltHostnameCmp);
	}

	if(pDfltProgNameCmp != NULL) {
		rsCStrDestruct(&pDfltProgNameCmp);
	}

	/* release objects we no longer need */
	objRelease(module, CORE_COMPONENT);
	objRelease(net, LM_NET_FILENAME);
	objRelease(ruleset, CORE_COMPONENT);
ENDObjClassExit(conf)


/* Initialize our class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-02-29
 */
BEGINAbstractObjClassInit(conf, 1, OBJ_IS_CORE_MODULE) /* class, version - CHANGE class also in END MACRO! */
	/* request objects we use */
	CHKiRet(objUse(module, CORE_COMPONENT));
	CHKiRet(objUse(net, LM_NET_FILENAME)); /* TODO: make this dependcy go away! */
	CHKiRet(objUse(ruleset, CORE_COMPONENT));

	/* These commands will NOT be supported -- the new v6.3 config system provides
	 * far better methods. We will remove the related code soon. -- rgerhards, 2012-01-09
	 */
	CHKiRet(regCfSysLineHdlr((uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler, resetConfigVariables,
NULL, NULL));
ENDObjClassInit(conf)

/* vi:set ai:
 */
