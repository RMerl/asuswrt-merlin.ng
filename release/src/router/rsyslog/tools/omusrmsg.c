/* omusrmsg.c
 * This is the implementation of the build-in output module for sending
 * user messages.
 *
 * NOTE: read comments in module-template.h to understand how this file
 *       works!
 *
 * File begun on 2007-07-20 by RGerhards (extracted from syslogd.c, which at the
 * time of the fork from sysklogd was under BSD license)
 *
 * Copyright 2007-2018 Adiscon GmbH.
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
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <ctype.h>
#include <sys/param.h>
#ifdef HAVE_UTMP_H
#  include <utmp.h>
#  define STRUCTUTMP struct utmp
#  define UTNAME ut_name
#else
#  include <utmpx.h>
#  define STRUCTUTMP struct utmpx
#  define UTNAME ut_user
#endif
#include <unistd.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <errno.h>
#if HAVE_FCNTL_H
#include <fcntl.h>
#else
#include <sys/msgbuf.h>
#endif
#if HAVE_PATHS_H
#include <paths.h>
#endif
#include "rsyslog.h"
#include "srUtils.h"
#include "stringbuf.h"
#include "syslogd-types.h"
#include "conf.h"
#include "omusrmsg.h"
#include "module-template.h"
#include "errmsg.h"


/* portability: */
#ifndef _PATH_DEV
#	define _PATH_DEV	"/dev/"
#endif

#ifdef UT_NAMESIZE
# define UNAMESZ	UT_NAMESIZE	/* length of a login name */
#else
# define UNAMESZ	32	/* length of a login name, 32 seems current (2018) good bet */
#endif
#define MAXUNAMES	20	/* maximum number of user names */

#ifdef OS_SOLARIS
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("omusrmsg")

/* internal structures
 */
DEF_OMOD_STATIC_DATA

typedef struct _instanceData {
	int bIsWall; /* 1- is wall, 0 - individual users */
	char uname[MAXUNAMES][UNAMESZ+1];
	uchar *tplName;
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
} wrkrInstanceData_t;

typedef struct configSettings_s {
	EMPTY_STRUCT
} configSettings_t;
static configSettings_t __attribute__((unused)) cs;


/* tables for interfacing with the v6 config system */
/* action (instance) parameters */
static struct cnfparamdescr actpdescr[] = {
	{ "users", eCmdHdlrString, CNFPARAM_REQUIRED },
	{ "template", eCmdHdlrGetWord, 0 }
};
static struct cnfparamblk actpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(actpdescr)/sizeof(struct cnfparamdescr),
	  actpdescr
	};

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
	if(eFeat == sFEATURERepeatedMsgReduction)
		iRet = RS_RET_OK;
ENDisCompatibleWithFeature


BEGINfreeInstance
CODESTARTfreeInstance
	free(pData->tplName);
ENDfreeInstance


BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance


BEGINdbgPrintInstInfo
	register int i;
CODESTARTdbgPrintInstInfo
	for (i = 0; i < MAXUNAMES && *pData->uname[i]; i++)
		dbgprintf("%s, ", pData->uname[i]);
ENDdbgPrintInstInfo


/**
 * BSD setutent/getutent() replacement routines
 * The following routines emulate setutent() and getutent() under
 * BSD because they are not available there. We only emulate what we actually
 * need! rgerhards 2005-03-18
 */
#ifdef OS_BSD
/* Since version 900007, FreeBSD has a POSIX compliant <utmpx.h> */
#if defined(__FreeBSD__) && (__FreeBSD_version >= 900007)
#  define setutent(void) setutxent(void)
#  define getutent(void) getutxent(void)
#  define endutent(void) endutxent(void)
#else
static FILE *BSD_uf = NULL;
void setutent(void)
{
	assert(BSD_uf == NULL);
	if ((BSD_uf = fopen(_PATH_UTMP, "r")) == NULL) {
		LogError(errno, NO_ERRCODE, "error opening utmp %s", _PATH_UTMP);
		return;
	}
}

STRUCTUTMP* getutent(void)
{
	static STRUCTUTMP st_utmp;

	if(fread((char *)&st_utmp, sizeof(st_utmp), 1, BSD_uf) != 1)
		return NULL;

	return(&st_utmp);
}

void endutent(void)
{
	fclose(BSD_uf);
	BSD_uf = NULL;
}
#endif /* if defined(__FreeBSD__) */
#endif  /* #ifdef OS_BSD */


/*  WALLMSG -- Write a message to the world at large
 *
 *	Write the specified message to either the entire
 *	world, or a list of approved users.
 *
 * rgerhards, 2005-10-19: applying the following sysklogd patch:
 * Tue May  4 16:52:01 CEST 2004: Solar Designer <solar@openwall.com>
 *	Adjust the size of a variable to prevent a buffer overflow
 *	should _PATH_DEV ever contain something different than "/dev/".
 * rgerhards, 2008-07-04: changing the function to no longer use fork() but
 * 	continue run on its thread instead.
 */
static rsRetVal wallmsg(uchar* pMsg, instanceData *pData)
{

	uchar szErr[512];
	char p[sizeof(_PATH_DEV) + UNAMESZ];
	register int i;
	int errnoSave;
	int ttyf;
	int wrRet;
	STRUCTUTMP ut;
	STRUCTUTMP *uptr;
	struct stat statb;
	DEFiRet;

	assert(pMsg != NULL);

	/* open the user login file */
	setutent();

	/* scan the user login file */
	while((uptr = getutent())) {
		memcpy(&ut, uptr, sizeof(ut));
		/* is this slot used? */
		if(ut.UTNAME[0] == '\0')
			continue;
#ifndef OS_BSD
		if(ut.ut_type != USER_PROCESS)
			continue;
#endif
		if(!(memcmp (ut.UTNAME,"LOGIN", 6))) /* paranoia */
			continue;

		/* should we send the message to this user? */
		if(pData->bIsWall == 0) {
			for(i = 0; i < MAXUNAMES; i++) {
				if(!pData->uname[i][0]) {
					i = MAXUNAMES;
					break;
				}
				if(strncmp(pData->uname[i], ut.UTNAME, UNAMESZ) == 0)
					break;
			}
			if(i == MAXUNAMES) /* user not found? */
				continue; /* on to next user! */
		}

		/* compute the device name */
		strcpy(p, _PATH_DEV);
		memcpy(p, ut.ut_line, UNAMESZ);

		/* we must be careful when writing to the terminal. A terminal may block
		 * (for example, a user has pressed <ctl>-s). In that case, we can not
		 * wait indefinitely. So we need to use non-blocking I/O. In case we would
		 * block, we simply do not send the message, because that's the best we can
		 * do. -- rgerhards, 2008-07-04
		 */

		/* open the terminal */
		if((ttyf = open(p, O_WRONLY|O_NOCTTY|O_NONBLOCK)) >= 0) {
			if(fstat(ttyf, &statb) == 0 && (statb.st_mode & S_IWRITE)) {
				wrRet = write(ttyf, pMsg, strlen((char*)pMsg));
				if(Debug && wrRet == -1) {
					/* we record the state to the debug log */
					errnoSave = errno;
					rs_strerror_r(errno, (char*)szErr, sizeof(szErr));
					dbgprintf("write to terminal '%s' failed with [%d]:%s\n",
						  p, errnoSave, szErr);
				}
			}
			close(ttyf);
		}
	}

	/* close the user login file */
	endutent();
	RETiRet;
}


BEGINtryResume
CODESTARTtryResume
ENDtryResume

BEGINdoAction
CODESTARTdoAction
	dbgprintf("\n");
	iRet = wallmsg(ppString[0], pWrkrData->pData);
ENDdoAction


static void
populateUsers(instanceData *pData, es_str_t *usrs)
{
	int i;
	int iDst;
	es_size_t iUsr;
	es_size_t len;
	uchar *c;

	len = es_strlen(usrs);
	c = es_getBufAddr(usrs);
	pData->bIsWall = 0; /* write to individual users */
	iUsr = 0;
	for(i = 0 ; i < MAXUNAMES && iUsr < len ; ++i) {
		for(  iDst = 0
		    ; iDst < UNAMESZ && iUsr < len && c[iUsr] != ','
		    ; ++iDst, ++iUsr) {
			pData->uname[i][iDst] = c[iUsr];
		}
		pData->uname[i][iDst] = '\0';
		DBGPRINTF("omusrmsg: send to user '%s'\n", pData->uname[i]);
		if(iUsr < len && c[iUsr] != ',') {
			LogError(0, RS_RET_ERR, "user name '%s...' too long - "
				"ignored", pData->uname[i]);
			--i;
			++iUsr;
			while(iUsr < len && c[iUsr] != ',')
				++iUsr; /* skip to next name */
		} else if(iDst == 0) {
			LogError(0, RS_RET_ERR, "no user name given - "
				"ignored");
			--i;
			++iUsr;
			while(iUsr < len && c[iUsr] != ',')
				++iUsr; /* skip to next name */
		}
		if(iUsr < len) {
			++iUsr; /* skip "," */
			while(iUsr < len && isspace(c[iUsr]))
				++iUsr; /* skip whitespace */
		}
	}
	if(i == MAXUNAMES && iUsr != len) {
		LogError(0, RS_RET_ERR, "omusrmsg supports only up to %d "
			"user names in a single action - all others have been ignored",
			MAXUNAMES);
	}
}


static inline void
setInstParamDefaults(instanceData *pData)
{
	pData->bIsWall = 0;
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
		if(!strcmp(actpblk.descr[i].name, "users")) {
			if(!es_strbufcmp(pvals[i].val.d.estr, (uchar*)"*", 1)) {
				pData->bIsWall = 1;
			} else {
				populateUsers(pData, pvals[i].val.d.estr);
			}
		} else if(!strcmp(actpblk.descr[i].name, "template")) {
			pData->tplName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else {
			dbgprintf("omusrmsg: program error, non-handled "
			  "param '%s'\n", actpblk.descr[i].name);
		}
	}

	if(pData->tplName == NULL) {
		CHKiRet(OMSRsetEntry(*ppOMSR, 0,
			(uchar*) strdup(pData->bIsWall ? " WallFmt" : " StdUsrMsgFmt"),
			OMSR_NO_RQD_TPL_OPTS));
	} else {
		CHKiRet(OMSRsetEntry(*ppOMSR, 0,
			(uchar*) strdup((char*) pData->tplName),
			OMSR_NO_RQD_TPL_OPTS));
	}
CODE_STD_FINALIZERnewActInst
	cnfparamvalsDestruct(pvals, &actpblk);
ENDnewActInst


BEGINparseSelectorAct
	es_str_t *usrs;
	int bHadWarning;
CODESTARTparseSelectorAct
CODE_STD_STRING_REQUESTparseSelectorAct(1)
	bHadWarning = 0;
	if(!strncmp((char*) p, ":omusrmsg:", sizeof(":omusrmsg:") - 1)) {
		p += sizeof(":omusrmsg:") - 1; /* eat indicator sequence  (-1 because of '\0'!) */
	} else {
		if(!*p || !((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')
	   || (*p >= '0' && *p <= '9') || *p == '_' || *p == '.' || *p == '*')) {
			ABORT_FINALIZE(RS_RET_CONFLINE_UNPROCESSED);
		} else {
			LogMsg(0, RS_RET_OUTDATED_STMT, LOG_WARNING,
				"action '%s' treated as ':omusrmsg:%s' - please "
				"use ':omusrmsg:%s' syntax instead, '%s' will "
				"not be supported in the future",
				p, p, p, p);
			bHadWarning = 1;
		}
	}

	CHKiRet(createInstance(&pData));

	if(*p == '*') { /* wall */
		dbgprintf("write-all");
		++p; /* eat '*' */
		pData->bIsWall = 1; /* write to all users */
		CHKiRet(cflineParseTemplateName(&p, *ppOMSR, 0, OMSR_NO_RQD_TPL_OPTS, (uchar*) " WallFmt"));
	} else {
		/* everything else is currently treated as a user name */
		usrs = es_newStr(128);
		while(*p && *p != ';') {
			es_addChar(&usrs, *p);
			++p;
		}
		populateUsers(pData, usrs);
		es_deleteStr(usrs);
		if((iRet = cflineParseTemplateName(&p, *ppOMSR, 0, OMSR_NO_RQD_TPL_OPTS, (uchar*)" StdUsrMsgFmt"))
			!= RS_RET_OK)
			goto finalize_it;
	}
	if(iRet == RS_RET_OK && bHadWarning)
		iRet = RS_RET_OK_WARN;
CODE_STD_FINALIZERparseSelectorAct
ENDparseSelectorAct


BEGINmodExit
CODESTARTmodExit
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_OMOD_QUERIES
CODEqueryEtryPt_STD_OMOD8_QUERIES
CODEqueryEtryPt_STD_CONF2_OMOD_QUERIES
ENDqueryEtryPt


BEGINmodInit(UsrMsg)
CODESTARTmodInit
INITLegCnfVars
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
ENDmodInit

/* vim:set ai:
 */
