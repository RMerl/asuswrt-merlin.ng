/* Definitions for config file handling (not yet an object).
 *
 * Copyright 2008-2012 Adiscon GmbH.
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
#ifndef INCLUDED_CONF_H
#define INCLUDED_CONF_H
#include "action.h"

/* definitions used for doNameLine to differentiate between different command types
 * (with otherwise identical code). This is a left-over from the previous config
 * system. It stays, because it is still useful. So do not wonder why it looks
 * somewhat strange (at least its name). -- rgerhards, 2007-08-01
 */
enum eDirective { DIR_TEMPLATE = 0, DIR_OUTCHANNEL = 1, DIR_ALLOWEDSENDER = 2};
extern ecslConfObjType currConfObj;
extern int bConfStrictScoping;	/* force strict scoping during config processing? */

/* interfaces */
BEGINinterface(conf) /* name must also be changed in ENDinterface macro! */
	rsRetVal (*doNameLine)(uchar **pp, void* pVal);
	rsRetVal (*cfsysline)(uchar *p);
	rsRetVal (*doModLoad)(uchar **pp, __attribute__((unused)) void* pVal);
	rsRetVal (*GetNbrActActions)(rsconf_t *conf, int *);
	/* version 4 -- 2010-07-23 rgerhards */
	/* "just" added global variables
	 * FYI: we reconsider repacking as a non-object, as only the core currently
	 * accesses this module. The current object structure complicates things without
	 * any real benefit.
	 */
	/* version 5 -- 2011-04-19 rgerhards */
	/* complete revamp, we now use the rsconf object */
	/* version 6 -- 2011-07-06 rgerhards */
	/* again a complete revamp, using flex/bison based parser now */
ENDinterface(conf)
#define confCURR_IF_VERSION 6 /* increment whenever you change the interface structure! */
/* in Version 3, entry point "ReInitConf()" was removed, as we do not longer need
 * to support restart-type HUP -- rgerhards, 2009-07-15
 */


/* prototypes */
PROTOTYPEObj(conf);


/* TODO: the following 2 need to go in conf obj interface... */
rsRetVal cflineParseTemplateName(uchar** pp, omodStringRequest_t *pOMSR, int iEntry, int iTplOpts,
	uchar *dfltTplName);
rsRetVal cflineParseFileName(uchar* p, uchar *pFileName, omodStringRequest_t *pOMSR, int iEntry, int iTplOpts,
	uchar *pszTpl);

rsRetVal DecodePRIFilter(uchar *pline, uchar pmask[]);
rsRetVal cflineDoAction(rsconf_t *conf, uchar **p, action_t **ppAction);
extern EHostnameCmpMode eDfltHostnameCmpMode;
extern cstr_t *pDfltHostnameCmp;
extern cstr_t *pDfltProgNameCmp;

#endif /* #ifndef INCLUDED_CONF_H */
