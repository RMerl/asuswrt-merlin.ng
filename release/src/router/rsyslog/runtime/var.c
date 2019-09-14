/* var.c - a typeless variable class
 *
 * This class is used to represent variable values, which may have any type.
 * Among others, it will be used inside rsyslog's expression system, but
 * also internally at any place where a typeless variable is needed.
 *
 * Module begun 2008-02-20 by Rainer Gerhards, with some code taken
 * from the obj.c/.h files.
 *
 * Copyright 2007-2016 Rainer Gerhards and Adiscon GmbH.
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
#include <stdlib.h>
#include <assert.h>

#include "rsyslog.h"
#include "obj.h"
#include "srUtils.h"
#include "var.h"

/* static data */
DEFobjStaticHelpers


/* Standard-Constructor
 */
BEGINobjConstruct(var) /* be sure to specify the object type also in END macro! */
ENDobjConstruct(var)


/* ConstructionFinalizer
 * rgerhards, 2008-01-09
 */
static rsRetVal
varConstructFinalize(var_t __attribute__((unused)) *pThis)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, var);

	RETiRet;
}


/* destructor for the var object */
BEGINobjDestruct(var) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDestruct(var)
	if(pThis->pcsName != NULL)
		rsCStrDestruct(&pThis->pcsName);
	if(pThis->varType == VARTYPE_STR) {
		if(pThis->val.pStr != NULL)
			rsCStrDestruct(&pThis->val.pStr);
	}
ENDobjDestruct(var)


/* DebugPrint support for the var object */
BEGINobjDebugPrint(var) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDebugPrint(var)
	switch(pThis->varType) {
		case VARTYPE_STR:
			dbgoprint((obj_t*) pThis, "type: cstr, val '%s'\n", rsCStrGetSzStrNoNULL(pThis->val.pStr));
			break;
		case VARTYPE_NUMBER:
			dbgoprint((obj_t*) pThis, "type: number, val %lld\n", pThis->val.num);
			break;
		case VARTYPE_SYSLOGTIME:
		case VARTYPE_NONE:
		default:
			dbgoprint((obj_t*) pThis, "type %d currently not suppored in debug output\n", pThis->varType);
			break;
	}
ENDobjDebugPrint(var)


/* queryInterface function
 * rgerhards, 2008-02-21
 */
BEGINobjQueryInterface(var)
CODESTARTobjQueryInterface(var)
	if(pIf->ifVersion != varCURR_IF_VERSION) { /* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->Construct = varConstruct;
	pIf->ConstructFinalize = varConstructFinalize;
	pIf->Destruct = varDestruct;
	pIf->DebugPrint = varDebugPrint;
finalize_it:
ENDobjQueryInterface(var)


/* Initialize the var class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-02-19
 */
BEGINObjClassInit(var, 1, OBJ_IS_CORE_MODULE) /* class, version */
	/* request objects we use */

	/* now set our own handlers */
	OBJSetMethodHandler(objMethod_DEBUGPRINT, varDebugPrint);
	OBJSetMethodHandler(objMethod_CONSTRUCTION_FINALIZER, varConstructFinalize);
ENDObjClassInit(var)

/* vi:set ai:
 */
