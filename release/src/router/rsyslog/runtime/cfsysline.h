/* Definition of the cfsysline (config file system line) object.
 *
 * Copyright 2007-2012 Adiscon GmbH.
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

#ifndef CFSYSLINE_H_INCLUDED
#define CFSYSLINE_H_INCLUDED

#include "linkedlist.h"

/* this is a single entry for a parse routine. It describes exactly
 * one entry point/handler.
 * The short name is cslch (Configfile SysLine CommandHandler)
 */
struct cslCmdHdlr_s { /* config file sysline parse entry */
	ecslConfObjType __attribute__((deprecated)) eConfObjType;		/* which config object is this for? */
	ecslCmdHdrlType eType;			/* which type of handler is this? */
	rsRetVal (*cslCmdHdlr)();		/* function pointer to use with handler (params depending on eType) */
	void *pData;				/* user-supplied data pointer */
	int *permitted;				/* is this parameter currently permitted? (NULL=don't check) */
};
typedef struct cslCmdHdlr_s cslCmdHdlr_t;


/* this is the list of known configuration commands with pointers to
 * their handlers.
 * The short name is cslc (Configfile SysLine Command)
 */
struct cslCmd_s { /* config file sysline parse entry */
	int bChainingPermitted;			/* may multiple handlers be chained for this command? */
	linkedList_t llCmdHdlrs;	/* linked list of command handlers */
};
typedef struct cslCmd_s cslCmd_t;

/* prototypes */
rsRetVal regCfSysLineHdlr(const uchar *pCmdName, int bChainingPermitted, ecslCmdHdrlType eType, rsRetVal (*pHdlr)(),
	void *pData, void *pOwnerCookie);
rsRetVal regCfSysLineHdlr2(const uchar *pCmdName, int bChainingPermitted, ecslCmdHdrlType eType, rsRetVal (*pHdlr)(),
	void *pData, void *pOwnerCookie, int *permitted);
rsRetVal unregCfSysLineHdlrs(void);
rsRetVal unregCfSysLineHdlrs4Owner(void *pOwnerCookie);
rsRetVal processCfSysLineCommand(uchar *pCmd, uchar **p);
rsRetVal cfsyslineInit(void);
void dbgPrintCfSysLineHandlers(void);

#endif /* #ifndef CFSYSLINE_H_INCLUDED */
