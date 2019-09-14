/* The zlibwrap object.
 *
 * This is an rsyslog object wrapper around zlib.
 *
 * Copyright 2009-2012 Rainer Gerhards and Adiscon GmbH.
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

#include "config.h"
#include <string.h>
#include <assert.h>
#include <zlib.h>

#include "rsyslog.h"
#include "module-template.h"
#include "obj.h"
#include "zlibw.h"

MODULE_TYPE_LIB
MODULE_TYPE_NOKEEP

/* static data */
DEFobjStaticHelpers


/* ------------------------------ methods ------------------------------ */

/* zlib make strong use of macros for its interface functions, so we can not simply
 * pass function pointers to them. Instead, we create very small wrappers which call
 * the relevant entry points.
 */

static int myDeflateInit(z_streamp strm, int level)
{
	return deflateInit(strm, level);
}

static int myDeflateInit2(z_streamp strm, int level, int method, int windowBits, int memLevel, int strategy)
{
	return deflateInit2(strm, level, method, windowBits, memLevel, strategy);
}

static int myDeflateEnd(z_streamp strm)
{
	return deflateEnd(strm);
}

static int myDeflate(z_streamp strm, int flush)
{
	return deflate(strm, flush);
}


/* queryInterface function
 * rgerhards, 2008-03-05
 */
BEGINobjQueryInterface(zlibw)
CODESTARTobjQueryInterface(zlibw)
	if(pIf->ifVersion != zlibwCURR_IF_VERSION) { /* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->DeflateInit = myDeflateInit;
	pIf->DeflateInit2 = myDeflateInit2;
	pIf->Deflate     = myDeflate;
	pIf->DeflateEnd  = myDeflateEnd;
finalize_it:
ENDobjQueryInterface(zlibw)


/* Initialize the zlibw class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-02-19
 */
BEGINAbstractObjClassInit(zlibw, 1, OBJ_IS_LOADABLE_MODULE) /* class, version */
	/* request objects we use */

	/* set our own handlers */
ENDObjClassInit(zlibw)


/* --------------- here now comes the plumbing that makes as a library module --------------- */


BEGINmodExit
CODESTARTmodExit
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_LIB_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */

	CHKiRet(zlibwClassInit(pModInfo)); /* must be done after tcps_sess, as we use it */
	/* Initialize all classes that are in our module - this includes ourselfs */
ENDmodInit
/* vi:set ai:
 */
