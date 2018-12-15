/* Definition of globally-accessible data items.
 *
 * This module provides access methods to items of global scope. Most often,
 * these globals serve as defaults to initialize local settings. Currently,
 * many of them are either constants or global variable references. However,
 * this module provides the necessary hooks to change that at any time.
 *
 * Please note that there currently is no glbl.c file as we do not yet
 * have any implementations.
 *
 * Copyright 2008-2018 Rainer Gerhards and Adiscon GmbH.
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

#ifndef GLBL_H_INCLUDED
#define GLBL_H_INCLUDED

#include <sys/types.h>
#ifdef ENABLE_LIBLOGGING_STDLOG
#include <liblogging/stdlog.h>
#endif
#include "rainerscript.h"
#include "prop.h"

#define glblGetIOBufSize() 4096 /* size of the IO buffer, e.g. for strm class */
#define glblOversizeMsgInputMode_Truncate 0
#define glblOversizeMsgInputMode_Split 1
#define glblOversizeMsgInputMode_Accept 2


extern pid_t glbl_ourpid;
extern int bProcessInternalMessages;
extern int bPermitSlashInProgramname;
#ifdef ENABLE_LIBLOGGING_STDLOG
extern stdlog_channel_t stdlog_hdl;
#endif

/* interfaces */
BEGINinterface(glbl) /* name must also be changed in ENDinterface macro! */
	uchar* (*GetWorkDir)(void);
	int (*GetMaxLine)(void);
#define SIMP_PROP(name, dataType) \
	dataType (*Get##name)(void); \
	rsRetVal (*Set##name)(dataType);
	SIMP_PROP(OptimizeUniProc, int)
	SIMP_PROP(PreserveFQDN, int)
	SIMP_PROP(DefPFFamily, int)
	SIMP_PROP(DropMalPTRMsgs, int)
	SIMP_PROP(Option_DisallowWarning, int)
	SIMP_PROP(DisableDNS, int)
	SIMP_PROP(LocalFQDNName, uchar*)
	SIMP_PROP(mainqCnfObj, struct cnfobj*)
	SIMP_PROP(LocalHostName, uchar*)
	SIMP_PROP(LocalDomain, uchar*)
	SIMP_PROP(StripDomains, char**)
	SIMP_PROP(LocalHosts, char**)
	SIMP_PROP(DfltNetstrmDrvr, uchar*)
	SIMP_PROP(DfltNetstrmDrvrCAF, uchar*)
	SIMP_PROP(DfltNetstrmDrvrKeyFile, uchar*)
	SIMP_PROP(DfltNetstrmDrvrCertFile, uchar*)
	SIMP_PROP(ParserControlCharacterEscapePrefix, uchar)
	SIMP_PROP(ParserDropTrailingLFOnReception, int)
	SIMP_PROP(ParserEscapeControlCharactersOnReceive, int)
	SIMP_PROP(ParserSpaceLFOnReceive, int)
	SIMP_PROP(ParserEscape8BitCharactersOnReceive, int)
	SIMP_PROP(ParserEscapeControlCharacterTab, int)
	SIMP_PROP(ParserEscapeControlCharactersCStyle, int)

	/* added v3, 2009-06-30 */
	rsRetVal (*GenerateLocalHostNameProperty)(void);
	prop_t* (*GetLocalHostNameProp)(void);
	/* added v4, 2009-07-20 */
	int (*GetGlobalInputTermState)(void);
	void (*SetGlobalInputTermination)(void);
	/* added v5, 2009-11-03 */
	SIMP_PROP(ParseHOSTNAMEandTAG, int)
	/* note: v4, v5 are already used by more recent versions, so we need to skip them! */
	/* added v6, 2009-11-16 as part of varmojfekoj's "unlimited select()" patch
	 * Note that it must be always present, otherwise the interface would have different
	 * versions depending on compile settings, what is not acceptable.
	 * Use this property with care, it is only truly available if UNLIMITED_SELECT is enabled
	 * (I did not yet further investigate the details, because that code hopefully can be removed
	 * at some later stage).
	 */
	SIMP_PROP(FdSetSize, int)
	/* v7: was neeeded to mean v5+v6 - do NOT add anything else for that version! */
	/* next change is v9! */
	/* v8 - 2012-03-21 */
	prop_t* (*GetLocalHostIP)(void);
	uchar* (*GetSourceIPofLocalClient)(void);		/* [ar] */
	rsRetVal (*SetSourceIPofLocalClient)(uchar*);		/* [ar] */
	/* v9 - 2015-01-12  SetMaxLine method removed */
#undef	SIMP_PROP
ENDinterface(glbl)
#define glblCURR_IF_VERSION 9 /* increment whenever you change the interface structure! */
/* version 2 had PreserveFQDN added - rgerhards, 2008-12-08 */

/* the remaining prototypes */
PROTOTYPEObj(glbl);

extern int glblDebugOnShutdown;	/* start debug log when we are shut down */
extern int glblReportNewSenders;
extern int glblReportGoneAwaySenders;
extern int glblSenderStatsTimeout;
extern int glblSenderKeepTrack;
extern int glblUnloadModules;
extern short janitorInterval;
extern int glblIntMsgRateLimitItv;
extern int glblIntMsgRateLimitBurst;
extern char** glblDbgFiles;
extern size_t glblDbgFilesNum;
extern int glblDbgWhitelist;
extern int glblPermitCtlC;
extern int glblInputTimeoutShutdown;

/* Developer options enable some strange things for developer-only testing.
 * These should never be enabled in a user build, except if explicitly told
 * by a developer. The options are acutally flags, so they should be powers
 * of two. Flag assignment may change between versions, **backward
 * compatibility is NOT necessary**.
 * rgerhards, 2018-04-28
 */
#define DEV_OPTION_KEEP_RUNNING_ON_HARD_CONF_ERROR 1
extern uint64_t glblDevOptions;

#define glblGetOurPid() glbl_ourpid
#define glblSetOurPid(pid) { glbl_ourpid = (pid); }

void glblPrepCnf(void);
void glblProcessCnf(struct cnfobj *o);
void glblProcessTimezone(struct cnfobj *o);
void glblProcessMainQCnf(struct cnfobj *o);
void glblDestructMainqCnfObj(void);
rsRetVal glblDoneLoadCnf(void);
const uchar * glblGetWorkDirRaw(void);
tzinfo_t* glblFindTimezoneInfo(char *id);
int GetGnuTLSLoglevel(void);
int glblGetMaxLine(void);
int bs_arrcmp_glblDbgFiles(const void *s1, const void *s2);
uchar* glblGetOversizeMsgErrorFile(void);
int glblGetOversizeMsgInputMode(void);
int glblReportOversizeMessage(void);

#endif /* #ifndef GLBL_H_INCLUDED */
