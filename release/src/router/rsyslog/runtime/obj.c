/* obj.c
 *
 * This file implements a generic object "class". All other classes can
 * use the service of this base class here to include auto-destruction and
 * other capabilities in a generic manner.
 *
 * As of 2008-02-29, I (rgerhards) am adding support for dynamically loadable
 * objects. In essence, each object will soon be available via its interface,
 * only. Before any object's code is accessed (including global static methods),
 * the caller needs to obtain an object interface. To do so, it needs to provide
 * the object name and the file where the object is expected to reside in. A
 * file may not be given, in which case the object is expected to reside in
 * the rsyslog core. The caller than receives an interface pointer which can
 * be utilized to access all the object's methods. This method enables rsyslog
 * to load library modules on demand. In order to keep overhead low, callers
 * should request object interface only once in the object Init function and
 * free them when they exit. The only exception is when a caller needs to
 * access an object only conditional, in which case a pointer to its interface
 * shall be aquired as need first arises but still be released only on exit
 * or when there definitely is no further need. The whole idea is to limit
 * the very performance-intense act of dynamically loading an objects library.
 * Of course, it is possible to violate this suggestion, but than you should
 * have very good reasoning to do so.
 *
 * Please note that there is one trick we need to do. Each object queries
 * the object interfaces and it does so via objUse(). objUse, however, is
 * part of the obj object's interface (implemented via the file you are
 * just reading). So in order to obtain a pointer to objUse, we need to
 * call it - obviously not possible. One solution would be that objUse is
 * hardcoded into all callers. That, however, would bring us into slight
 * trouble with actually dynamically loaded modules, as we should NOT
 * rely on the OS loader to resolve symbols back to the caller (this
 * is a feature not universally available and highly importable). Of course,
 * we can solve this with a pHostQueryEtryPoint() call. It still sounds
 * somewhat unnatural to call a regular interface function via a special
 * method. So what we do instead is define a special function called
 * objGetObjInterface() which delivers our own interface. That function
 * than will be defined global and be queriable via pHostQueryEtryPoint().
 * I agree, technically this is much the same, but from an architecture
 * point of view it looks cleaner (at least to me).
 *
 * Please note that there is another egg-hen problem: we use a linked list,
 * which is provided by the linkedList object. However, we need to
 * initialize the linked list before we can provide the UseObj()
 * functionality. That, in turn, would probably be required by the
 * linkedList object. So the solution is to use a backdoor just to
 * init the linked list and from then on use the usual interfaces.
 *
 * File begun on 2008-01-04 by RGerhards
 *
 * Copyright 2008-2016 Rainer Gerhards and Adiscon GmbH.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <pthread.h>

/* how many objects are supported by rsyslogd? */
#define OBJ_NUM_IDS 100 /* TODO change to a linked list?  info: 16 were currently in use 2008-02-29 */

#include "rsyslog.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "obj.h"
#include "stream.h"
#include "modules.h"
#include "errmsg.h"
#include "cfsysline.h"
#include "unicode-helper.h"
#include "datetime.h"

/* static data */
DEFobjCurrIf(obj) /* we define our own interface, as this is expected by some macros! */
DEFobjCurrIf(var)
DEFobjCurrIf(module)
DEFobjCurrIf(strm)
static objInfo_t *arrObjInfo[OBJ_NUM_IDS]; /* array with object information pointers */
pthread_mutex_t mutObjGlobalOp;	/* mutex to guard global operations of the object system */


/* cookies for serialized lines */
#define COOKIE_OBJLINE   '<'
#define COOKIE_PROPLINE  '+'
#define COOKIE_ENDLINE   '>'
#define COOKIE_BLANKLINE '.'

/* forward definitions */
static rsRetVal FindObjInfo(const char *szObjName, objInfo_t **ppInfo);

/* methods */

/* This is a dummy method to be used when a standard method has not been
 * implemented by an object. Having it allows us to simply call via the
 * jump table without any NULL pointer checks - which gains quite
 * some performance. -- rgerhards, 2008-01-04
 */
static rsRetVal objInfoNotImplementedDummy(void __attribute__((unused)) *pThis)
{
	return RS_RET_NOT_IMPLEMENTED;
}

/* and now the macro to check if something is not implemented
 * must be provided an objInfo_t pointer.
 */
#define objInfoIsImplemented(pThis, method) \
	(pThis->objMethods[method] != objInfoNotImplementedDummy)

/* construct an object Info object. Each class shall do this on init. The
 * resulting object shall be cached during the lifetime of the class and each
 * object shall receive a reference. A constructor and destructor MUST be provided for all
 * objects, thus they are in the parameter list.
 * pszID is the identifying object name and must point to constant pool memory. It is never freed.
 */
static rsRetVal
InfoConstruct(objInfo_t **ppThis, uchar *pszID, int iObjVers,
		rsRetVal (*pConstruct)(void *), rsRetVal (*pDestruct)(void *),
		rsRetVal (*pQueryIF)(interface_t*), modInfo_t *pModInfo)
{
	DEFiRet;
	int i;
	objInfo_t *pThis;

	assert(ppThis != NULL);

	if((pThis = calloc(1, sizeof(objInfo_t))) == NULL)
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);

	pThis->pszID = pszID;
	pThis->lenID = ustrlen(pszID);
	pThis->pszName = ustrdup(pszID); /* it's OK if we have NULL ptr, GetName() will deal with that! */
	pThis->iObjVers = iObjVers;
	pThis->QueryIF = pQueryIF;
	pThis->pModInfo = pModInfo;

	pThis->objMethods[0] = pConstruct;
	pThis->objMethods[1] = pDestruct;
	for(i = 2 ; i < OBJ_NUM_METHODS ; ++i) {
		pThis->objMethods[i] = objInfoNotImplementedDummy;
	}

	*ppThis = pThis;

finalize_it:
	RETiRet;
}


/* destruct the objInfo object - must be done only when no more instances exist.
 * rgerhards, 2008-03-10
 */
static rsRetVal
InfoDestruct(objInfo_t **ppThis)
{
	DEFiRet;
	objInfo_t *pThis;

	assert(ppThis != NULL);
	pThis = *ppThis;
	assert(pThis != NULL);

	free(pThis->pszName);
	free(pThis);
	*ppThis = NULL;

	RETiRet;
}


/* set a method handler */
static rsRetVal
InfoSetMethod(objInfo_t *pThis, objMethod_t objMethod, rsRetVal (*pHandler)(void*))
{
	pThis->objMethods[objMethod] = pHandler;
	return RS_RET_OK;
}

/* destruct the base object properties.
 * rgerhards, 2008-01-29
 */
static rsRetVal
DestructObjSelf(obj_t *pThis)
{
	DEFiRet;

	ISOBJ_assert(pThis);
	free(pThis->pszName);

	RETiRet;
}


/* --------------- object serializiation / deserialization support --------------- */


/* serialize the header of an object
 * pszRecType must be either "Obj" (Object) or "OPB" (Object Property Bag)
 */
static rsRetVal objSerializeHeader(strm_t *pStrm, obj_t *pObj, uchar *pszRecType)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pStrm, strm);
	ISOBJ_assert(pObj);
	assert(!strcmp((char*) pszRecType, "Obj") || !strcmp((char*) pszRecType, "OPB"));

	/* object cookie and serializer version (so far always 1) */
	CHKiRet(strm.WriteChar(pStrm, COOKIE_OBJLINE));
	CHKiRet(strm.Write(pStrm, (uchar*) pszRecType, 3)); /* record types are always 3 octets */
	CHKiRet(strm.WriteChar(pStrm, ':'));
	CHKiRet(strm.WriteChar(pStrm, '1'));

	/* object type, version and string length */
	CHKiRet(strm.WriteChar(pStrm, ':'));
	CHKiRet(strm.Write(pStrm, pObj->pObjInfo->pszID, pObj->pObjInfo->lenID));
	CHKiRet(strm.WriteChar(pStrm, ':'));
	CHKiRet(strm.WriteLong(pStrm, objGetVersion(pObj)));

	/* record trailer */
	CHKiRet(strm.WriteChar(pStrm, ':'));
	CHKiRet(strm.WriteChar(pStrm, '\n'));

finalize_it:
	RETiRet;
}


/* begin serialization of an object
 * rgerhards, 2008-01-06
 */
static rsRetVal
BeginSerialize(strm_t *pStrm, obj_t *pObj)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pStrm, strm);
	ISOBJ_assert(pObj);
	
	CHKiRet(strm.RecordBegin(pStrm));
	CHKiRet(objSerializeHeader(pStrm, pObj, (uchar*) "Obj"));

finalize_it:
	RETiRet;
}
	

/* begin serialization of an object's property bag
 * Note: a property bag is used to serialize some of an objects
 * properties, but not necessarily all. A good example is the queue
 * object, which at some stage needs to serialize a number of its
 * properties, but not the queue data itself. From the object point
 * of view, a property bag can not be used to re-instantiate an object.
 * Otherwise, the serialization is exactly the same.
 * rgerhards, 2008-01-11
 */
static rsRetVal
BeginSerializePropBag(strm_t *pStrm, obj_t *pObj)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pStrm, strm);
	ISOBJ_assert(pObj);
	
	CHKiRet(strm.RecordBegin(pStrm));
	CHKiRet(objSerializeHeader(pStrm, pObj, (uchar*) "OPB"));

finalize_it:
	RETiRet;
}


/* append a property
 */
static rsRetVal
SerializeProp(strm_t *pStrm, uchar *pszPropName, propType_t propType, void *pUsr)
{
	DEFiRet;
	uchar *pszBuf = NULL;
	size_t lenBuf = 0;
	uchar szBuf[64];
	varType_t vType = VARTYPE_NONE;

	ISOBJ_TYPE_assert(pStrm, strm);
	assert(pszPropName != NULL);

	/*dbgprintf("objSerializeProp: strm %p, propName '%s', type %d, pUsr %p\n",
		pStrm, pszPropName, propType, pUsr);*/
	/* if we have no user pointer, there is no need to write this property.
	 * TODO: think if that's the righ point of view
	 * rgerhards, 2008-01-06
	 */
	if(pUsr == NULL) {
		ABORT_FINALIZE(RS_RET_OK);
	}

	/* TODO: use the stream functions for data conversion here - should be quicker */

	switch(propType) {
		case PROPTYPE_PSZ:
			pszBuf = (uchar*) pUsr;
			lenBuf = ustrlen(pszBuf);
			vType = VARTYPE_STR;
			break;
		case PROPTYPE_SHORT:
			CHKiRet(srUtilItoA((char*) szBuf, sizeof(szBuf), (long) *((short*) pUsr)));
			pszBuf = szBuf;
			lenBuf = ustrlen(szBuf);
			vType = VARTYPE_NUMBER;
			break;
		case PROPTYPE_INT:
			CHKiRet(srUtilItoA((char*) szBuf, sizeof(szBuf), (long) *((int*) pUsr)));
			pszBuf = szBuf;
			lenBuf = ustrlen(szBuf);
			vType = VARTYPE_NUMBER;
			break;
		case PROPTYPE_LONG:
			CHKiRet(srUtilItoA((char*) szBuf, sizeof(szBuf), *((long*) pUsr)));
			pszBuf = szBuf;
			lenBuf = ustrlen(szBuf);
			vType = VARTYPE_NUMBER;
			break;
		case PROPTYPE_INT64:
			CHKiRet(srUtilItoA((char*) szBuf, sizeof(szBuf), *((int64*) pUsr)));
			pszBuf = szBuf;
			lenBuf = ustrlen(szBuf);
			vType = VARTYPE_NUMBER;
			break;
		case PROPTYPE_CSTR:
			pszBuf = rsCStrGetSzStrNoNULL((cstr_t *) pUsr);
			lenBuf = rsCStrLen((cstr_t*) pUsr);
			vType = VARTYPE_STR;
			break;
		case PROPTYPE_SYSLOGTIME:
			lenBuf = snprintf((char*) szBuf, sizeof(szBuf), "%d:%d:%d:%d:%d:%d:%d:%d:%d:%c:%d:%d",
					  ((syslogTime_t*)pUsr)->timeType,
					  ((syslogTime_t*)pUsr)->year,
					  ((syslogTime_t*)pUsr)->month,
					  ((syslogTime_t*)pUsr)->day,
					  ((syslogTime_t*)pUsr)->hour,
					  ((syslogTime_t*)pUsr)->minute,
					  ((syslogTime_t*)pUsr)->second,
					  ((syslogTime_t*)pUsr)->secfrac,
					  ((syslogTime_t*)pUsr)->secfracPrecision,
					  ((syslogTime_t*)pUsr)->OffsetMode,
					  ((syslogTime_t*)pUsr)->OffsetHour,
					  ((syslogTime_t*)pUsr)->OffsetMinute);
			if(lenBuf > sizeof(szBuf) - 1)
				ABORT_FINALIZE(RS_RET_PROVIDED_BUFFER_TOO_SMALL);
			vType = VARTYPE_SYSLOGTIME;
			pszBuf = szBuf;
			break;
		case PROPTYPE_NONE:
		default:
			dbgprintf("invalid PROPTYPE %d\n", propType);
			break;
	}

	/* cookie */
	CHKiRet(strm.WriteChar(pStrm, COOKIE_PROPLINE));
	/* name */
	CHKiRet(strm.Write(pStrm, pszPropName, ustrlen(pszPropName)));
	CHKiRet(strm.WriteChar(pStrm, ':'));
	/* type */
	CHKiRet(strm.WriteLong(pStrm, (int) vType));
	CHKiRet(strm.WriteChar(pStrm, ':'));
	/* length */
	CHKiRet(strm.WriteLong(pStrm, lenBuf));
	CHKiRet(strm.WriteChar(pStrm, ':'));

	/* data */
	CHKiRet(strm.Write(pStrm, (uchar*) pszBuf, lenBuf));

	/* trailer */
	CHKiRet(strm.WriteChar(pStrm, ':'));
	CHKiRet(strm.WriteChar(pStrm, '\n'));

finalize_it:
	RETiRet;
}


/* end serialization of an object. The caller receives a
 * standard C string, which he must free when no longer needed.
 */
static rsRetVal
EndSerialize(strm_t *pStrm)
{
	DEFiRet;

	assert(pStrm != NULL);

	CHKiRet(strm.WriteChar(pStrm, COOKIE_ENDLINE));
	CHKiRet(strm.Write(pStrm, (uchar*) "End\n", sizeof("END\n") - 1));
	CHKiRet(strm.WriteChar(pStrm, COOKIE_BLANKLINE));
	CHKiRet(strm.WriteChar(pStrm, '\n'));

	CHKiRet(strm.RecordEnd(pStrm));

finalize_it:
	RETiRet;
}


/* define a helper to make code below a bit cleaner (and quicker to write) */
#define NEXTC CHKiRet(strm.ReadChar(pStrm, &c))/*;dbgprintf("c: %c\n", c)*/


/* de-serialize an embedded, non-octect-counted string. This is useful
 * for deserializing the object name inside the header. The string is
 * terminated by the first occurence of the ':' character.
 * rgerhards, 2008-02-29
 */
static rsRetVal
objDeserializeEmbedStr(cstr_t **ppStr, strm_t *pStrm)
{
	DEFiRet;
	uchar c;
	cstr_t *pStr = NULL;

	assert(ppStr != NULL);

	CHKiRet(cstrConstruct(&pStr));

	NEXTC;
	while(c != ':') {
		CHKiRet(cstrAppendChar(pStr, c));
		NEXTC;
	}
	cstrFinalize(pStr);

	*ppStr = pStr;

finalize_it:
	if(iRet != RS_RET_OK && pStr != NULL)
		cstrDestruct(&pStr);

	RETiRet;
}


/* de-serialize a number */
static rsRetVal objDeserializeNumber(number_t *pNum, strm_t *pStrm)
{
	DEFiRet;
	number_t i;
	int bIsNegative;
	uchar c;

	assert(pNum != NULL);

	NEXTC;
	if(c == '-') {
		bIsNegative = 1;
		NEXTC;
	} else {
		bIsNegative = 0;
	}

	/* we check this so that we get more meaningful error codes */
	if(!isdigit(c)) ABORT_FINALIZE(RS_RET_INVALID_NUMBER);

	i = 0;
	while(isdigit(c)) {
		i = i * 10 + c - '0';
		NEXTC;
	}

	if(c != ':') ABORT_FINALIZE(RS_RET_INVALID_DELIMITER);

	if(bIsNegative)
		i *= -1;

	*pNum = i;
finalize_it:
	RETiRet;
}


/* de-serialize a string, length must be provided but may be 0 */
static rsRetVal objDeserializeStr(cstr_t **ppCStr, int iLen, strm_t *pStrm)
{
	DEFiRet;
	int i;
	uchar c;
	cstr_t *pCStr = NULL;

	assert(ppCStr != NULL);
	assert(iLen >= 0);

	CHKiRet(cstrConstruct(&pCStr));

	NEXTC;
	for(i = 0 ; i < iLen ; ++i) {
		CHKiRet(cstrAppendChar(pCStr, c));
		NEXTC;
	}
	cstrFinalize(pCStr);

	/* check terminator */
	if(c != ':') ABORT_FINALIZE(RS_RET_INVALID_DELIMITER);

	*ppCStr = pCStr;

finalize_it:
	if(iRet != RS_RET_OK && pCStr != NULL)
		cstrDestruct(&pCStr);

	RETiRet;
}


/* de-serialize a syslogTime -- rgerhards,2008-01-08 */
#define	GETVAL(var)  \
	CHKiRet(objDeserializeNumber(&l, pStrm)); \
	pTime->var = l;
static rsRetVal objDeserializeSyslogTime(syslogTime_t *pTime, strm_t *pStrm)
{
	DEFiRet;
	number_t l;
	uchar c;

	assert(pTime != NULL);

	GETVAL(timeType);
	GETVAL(year);
	GETVAL(month);
	GETVAL(day);
	GETVAL(hour);
	GETVAL(minute);
	GETVAL(second);
	GETVAL(secfrac);
	GETVAL(secfracPrecision);
	/* OffsetMode is a single character! */
	NEXTC; pTime->OffsetMode = c;
	NEXTC; if(c != ':') ABORT_FINALIZE(RS_RET_INVALID_DELIMITER);
	GETVAL(OffsetHour);
	GETVAL(OffsetMinute);

finalize_it:
	RETiRet;
}
#undef GETVAL

/* de-serialize an object header
 * rgerhards, 2008-01-07
 */
static rsRetVal objDeserializeHeader(uchar *pszRecType, cstr_t **ppstrID, int* poVers, strm_t *pStrm)
{
	DEFiRet;
	number_t oVers;
	uchar c;

	assert(ppstrID != NULL);
	assert(poVers != NULL);
	assert(!strcmp((char*) pszRecType, "Obj") || !strcmp((char*) pszRecType, "OPB"));

	/* check header cookie */
	NEXTC; if(c != COOKIE_OBJLINE) ABORT_FINALIZE(RS_RET_INVALID_HEADER);
	NEXTC; if(c != pszRecType[0]) ABORT_FINALIZE(RS_RET_INVALID_HEADER_RECTYPE);
	NEXTC; if(c != pszRecType[1]) ABORT_FINALIZE(RS_RET_INVALID_HEADER_RECTYPE);
	NEXTC; if(c != pszRecType[2]) ABORT_FINALIZE(RS_RET_INVALID_HEADER_RECTYPE);
	NEXTC; if(c != ':') ABORT_FINALIZE(RS_RET_INVALID_HEADER);
	NEXTC; if(c != '1') ABORT_FINALIZE(RS_RET_INVALID_HEADER_VERS);
	NEXTC; if(c != ':') ABORT_FINALIZE(RS_RET_INVALID_HEADER_VERS);

	/* object type and version */
	CHKiRet(objDeserializeEmbedStr(ppstrID, pStrm));
	CHKiRet(objDeserializeNumber(&oVers, pStrm));

	/* and now we skip over the rest until the delemiting \n */
	NEXTC;
	while(c != '\n') {
		NEXTC;
	}

	*poVers = oVers;

finalize_it:
	RETiRet;
}


/* Deserialize a single property. Pointer must be positioned at begin of line. Whole line
 * up until the \n is read.
 */
rsRetVal objDeserializeProperty(var_t *pProp, strm_t *pStrm)
{
	DEFiRet;
	number_t i;
	number_t iLen;
	uchar c;
	int step = 0; /* which step was successful? */
	int64 offs;

	assert(pProp != NULL);

	/* check cookie */
	NEXTC;
	if(c != COOKIE_PROPLINE) {
		/* oops, we've read one char that does not belong to use - unget it first */
		CHKiRet(strm.UnreadChar(pStrm, c));
		ABORT_FINALIZE(RS_RET_NO_PROPLINE);
	}

	/* get the property name first */
	CHKiRet(cstrConstruct(&pProp->pcsName));

	NEXTC;
	while(c != ':') {
		CHKiRet(cstrAppendChar(pProp->pcsName, c));
		NEXTC;
	}
	cstrFinalize(pProp->pcsName);
	step = 1;

	/* property type */
	CHKiRet(objDeserializeNumber(&i, pStrm));
	pProp->varType = i;
	step = 2;

	/* size (needed for strings) */
	CHKiRet(objDeserializeNumber(&iLen, pStrm));
	step = 3;

	/* we now need to deserialize the value */
	switch(pProp->varType) {
		case VARTYPE_STR:
			CHKiRet(objDeserializeStr(&pProp->val.pStr, iLen, pStrm));
			break;
		case VARTYPE_NUMBER:
			CHKiRet(objDeserializeNumber(&pProp->val.num, pStrm));
			break;
		case VARTYPE_SYSLOGTIME:
			CHKiRet(objDeserializeSyslogTime(&pProp->val.vSyslogTime, pStrm));
			break;
		case VARTYPE_NONE:
		default:
			dbgprintf("invalid VARTYPE %d\n", pProp->varType);
			break;
	}
	step = 4;

	/* we should now be at the end of the line. So the next char must be \n */
	NEXTC;
	if(c != '\n') ABORT_FINALIZE(RS_RET_INVALID_PROPFRAME);

finalize_it:
	/* ensure the type of var is reset back to VARTYPE_NONE since
	* the deconstruct method of var might free unallocated memory
	*/
	if(iRet != RS_RET_OK && iRet != RS_RET_NO_PROPLINE) {
		if(step <= 2) {
			pProp->varType = VARTYPE_NONE;
		}
	}
	if(Debug && iRet != RS_RET_OK && iRet != RS_RET_NO_PROPLINE) {
		strm.GetCurrOffset(pStrm, &offs);
		dbgprintf("error %d deserializing property name, offset %lld, step %d\n",
			  iRet, offs, step);
		strmDebugOutBuf(pStrm);
		if(step >= 1) {
			dbgprintf("error property name: '%s'\n", rsCStrGetSzStrNoNULL(pProp->pcsName));
		}
		if(step >= 2) {
			dbgprintf("error var type: '%d'\n", pProp->varType);
		}
		if(step >= 3) {
			dbgprintf("error len: '%d'\n", (int) iLen);
		}
		if(step >= 4) {
			switch(pProp->varType) {
				case VARTYPE_STR:
					dbgprintf("error data string: '%s'\n",
						  rsCStrGetSzStrNoNULL(pProp->val.pStr));
					break;
				case VARTYPE_NUMBER:
					dbgprintf("error number: %d\n", (int) pProp->val.num);
					break;
				case VARTYPE_SYSLOGTIME:
					dbgprintf("syslog time was successfully parsed (but "
					          "is not displayed\n");
					break;
				case VARTYPE_NONE:
				default:
					break;
			}
		}
	}
	RETiRet;
}


/* de-serialize an object trailer. This does not get any data but checks if the
 * format is ok.
 * rgerhards, 2008-01-07
 */
static rsRetVal objDeserializeTrailer(strm_t *pStrm)
{
	DEFiRet;
	uchar c;

	/* check header cookie */
	NEXTC; if(c != COOKIE_ENDLINE) ABORT_FINALIZE(RS_RET_INVALID_TRAILER);
	NEXTC; if(c != 'E')  ABORT_FINALIZE(RS_RET_INVALID_TRAILER);
	NEXTC; if(c != 'n')  ABORT_FINALIZE(RS_RET_INVALID_TRAILER);
	NEXTC; if(c != 'd')  ABORT_FINALIZE(RS_RET_INVALID_TRAILER);
	NEXTC; if(c != '\n') ABORT_FINALIZE(RS_RET_INVALID_TRAILER);
	NEXTC; if(c != COOKIE_BLANKLINE) ABORT_FINALIZE(RS_RET_INVALID_TRAILER);
	NEXTC; if(c != '\n') ABORT_FINALIZE(RS_RET_INVALID_TRAILER);

finalize_it:
	if(Debug && iRet != RS_RET_OK) {
		dbgprintf("objDeserializeTrailer fails with %d\n", iRet);
	}

	RETiRet;
}



/* This method tries to recover a serial store if it got out of sync.
 * To do so, it scans the line beginning cookies and waits for the object
 * cookie. If that is found, control is returned. If the store is exhausted,
 * we will receive an RS_RET_EOF error as part of NEXTC, which will also
 * terminate this function. So we may either return with somehting that
 * looks like a valid object or end of store.
 * rgerhards, 2008-01-07
 */
static rsRetVal objDeserializeTryRecover(strm_t *pStrm)
{
	DEFiRet;
	uchar c;
	int bWasNL;
	int bRun;

	assert(pStrm != NULL);
	bRun = 1;
	bWasNL = 0;

	while(bRun) {
		NEXTC;
		if(c == '\n')
			bWasNL = 1;
		else {
			if(bWasNL == 1 && c == COOKIE_OBJLINE)
				bRun = 0; /* we found it! */
			else
				bWasNL = 0;
		}
	}

	CHKiRet(strm.UnreadChar(pStrm, c));

finalize_it:
	dbgprintf("deserializer has possibly been able to re-sync and recover, state %d\n", iRet);
	RETiRet;
}


/* De-serialize the properties of an object. This includes processing
 * of the trailer. Header must already have been processed.
 * rgerhards, 2008-01-11
 */
static rsRetVal objDeserializeProperties(obj_t *pObj, rsRetVal (*objSetProperty)(), strm_t *pStrm)
{
	DEFiRet;
	var_t *pVar = NULL;

	ISOBJ_assert(pObj);
	ISOBJ_TYPE_assert(pStrm, strm);

	CHKiRet(var.Construct(&pVar));
	CHKiRet(var.ConstructFinalize(pVar));

	iRet = objDeserializeProperty(pVar, pStrm);
	while(iRet == RS_RET_OK) {
		CHKiRet(objSetProperty(pObj, pVar));
		/* re-init var object - TODO: method of var! */
		rsCStrDestruct(&pVar->pcsName); /* no longer needed */
		if(pVar->varType == VARTYPE_STR) {
			if(pVar->val.pStr != NULL)
				rsCStrDestruct(&pVar->val.pStr);
		}
		iRet = objDeserializeProperty(pVar, pStrm);
	}

	if(iRet != RS_RET_NO_PROPLINE)
		FINALIZE;

	CHKiRet(objDeserializeTrailer(pStrm)); /* do trailer checks */
finalize_it:
	if(pVar != NULL)
		var.Destruct(&pVar);

	RETiRet;
}


/* De-Serialize an object.
 * Params: Pointer to object Pointer (pObj) (like a obj_t**, but can not do that due to compiler warning)
 * expected object ID (to check against), a fixup function that can modify the object before it is finalized
 * and a user pointer that is to be passed to that function in addition to the object. The fixup function
 * pointer may be NULL, in which case none is called.
 * The caller must destruct the created object.
 * rgerhards, 2008-01-07
 */
static rsRetVal
Deserialize(void *ppObj, uchar *pszTypeExpected, strm_t *pStrm, rsRetVal (*fFixup)(obj_t*,void*), void *pUsr)
{
	DEFiRet;
	rsRetVal iRetLocal;
	obj_t *pObj = NULL;
	int oVers = 0;   /* keep compiler happy, but it is totally useless but takes up some execution time... */
	cstr_t *pstrID = NULL;
	objInfo_t *pObjInfo;

	assert(ppObj != NULL);
	assert(pszTypeExpected != NULL);
	ISOBJ_TYPE_assert(pStrm, strm);

	/* we de-serialize the header. if all goes well, we are happy. However, if
	 * we experience a problem, we try to recover. We do this by skipping to
	 * the next object header. This is defined via the line-start cookies. In
	 * worst case, we exhaust the queue, but then we receive EOF return state,
	 * from objDeserializeTryRecover(), what will cause us to ultimately give up.
	 * rgerhards, 2008-07-08
	 */
	do {
		iRetLocal = objDeserializeHeader((uchar*) "Obj", &pstrID, &oVers, pStrm);
		if(iRetLocal != RS_RET_OK) {
			dbgprintf("objDeserialize error %d during header processing - trying to recover\n", iRetLocal);
			CHKiRet(objDeserializeTryRecover(pStrm));
		}
	} while(iRetLocal != RS_RET_OK);

	if(rsCStrSzStrCmp(pstrID, pszTypeExpected, ustrlen(pszTypeExpected)))
	/* TODO: optimize strlen() - caller shall provide */
		ABORT_FINALIZE(RS_RET_INVALID_OID);

	CHKiRet(FindObjInfo((char*)cstrGetSzStrNoNULL(pstrID), &pObjInfo));

	CHKiRet(pObjInfo->objMethods[objMethod_CONSTRUCT](&pObj));

	/* we got the object, now we need to fill the properties */
	CHKiRet(objDeserializeProperties(pObj, pObjInfo->objMethods[objMethod_SETPROPERTY], pStrm));

	/* check if we need to call a fixup function that modifies the object
	 * before it is finalized. -- rgerhards, 2008-01-13
	 */
	if(fFixup != NULL)
		CHKiRet(fFixup(pObj, pUsr));

	/* we have a valid object, let's finalize our work and return */
	if(objInfoIsImplemented(pObjInfo, objMethod_CONSTRUCTION_FINALIZER))
		CHKiRet(pObjInfo->objMethods[objMethod_CONSTRUCTION_FINALIZER](pObj));

	*((obj_t**) ppObj) = pObj;

finalize_it:
	if(iRet != RS_RET_OK && pObj != NULL)
		free(pObj); /* TODO: check if we can call destructor 2008-01-13 rger */

	if(pstrID != NULL)
		rsCStrDestruct(&pstrID);

	RETiRet;
}


/* De-Serialize an object, with known constructur and destructor. Params like Deserialize().
 * Note: this is for the queue subsystem, and optimized for its use.
 * rgerhards, 2012-11-03
 */
rsRetVal
objDeserializeWithMethods(void *ppObj, uchar *pszTypeExpected, int lenTypeExpected, strm_t *pStrm,
rsRetVal (*fFixup)(obj_t*,void*), void *pUsr, rsRetVal (*objConstruct)(), rsRetVal (*objConstructFinalize)(),
rsRetVal (*objDeserialize)())
{
	DEFiRet;
	rsRetVal iRetLocal;
	obj_t *pObj = NULL;
	int oVers = 0;   /* keep compiler happy, but it is totally useless but takes up some execution time... */
	cstr_t *pstrID = NULL;

	assert(ppObj != NULL);
	assert(pszTypeExpected != NULL);
	ISOBJ_TYPE_assert(pStrm, strm);

	/* we de-serialize the header. if all goes well, we are happy. However, if
	 * we experience a problem, we try to recover. We do this by skipping to
	 * the next object header. This is defined via the line-start cookies. In
	 * worst case, we exhaust the queue, but then we receive EOF return state,
	 * from objDeserializeTryRecover(), what will cause us to ultimately give up.
	 * rgerhards, 2008-07-08
	 */
	do {
		iRetLocal = objDeserializeHeader((uchar*) "Obj", &pstrID, &oVers, pStrm);
		if(iRetLocal != RS_RET_OK) {
			dbgprintf("objDeserialize error %d during header processing - "
				  "trying to recover\n", iRetLocal);
			CHKiRet(objDeserializeTryRecover(pStrm));
		}
	} while(iRetLocal != RS_RET_OK);

	if(rsCStrSzStrCmp(pstrID, pszTypeExpected, lenTypeExpected))
		ABORT_FINALIZE(RS_RET_INVALID_OID);

	CHKiRet(objConstruct(&pObj));

	/* we got the object, now we need to fill the properties */
	CHKiRet(objDeserialize(pObj, pStrm));
	CHKiRet(objDeserializeTrailer(pStrm)); /* do trailer checks */

	/* check if we need to call a fixup function that modifies the object
	 * before it is finalized. -- rgerhards, 2008-01-13
	 */
	if(fFixup != NULL)
		CHKiRet(fFixup(pObj, pUsr));

	/* we have a valid object, let's finalize our work and return */
	if(objConstructFinalize != NULL) {
		CHKiRet(objConstructFinalize(pObj));
	}

	*((obj_t**) ppObj) = pObj;

finalize_it:
	if(iRet != RS_RET_OK && pObj != NULL)
		free(pObj); /* TODO: check if we can call destructor 2008-01-13 rger */

	if(pstrID != NULL)
		rsCStrDestruct(&pstrID);

	if(Debug && iRet != RS_RET_OK) {
		dbgprintf("objDeserializeWithMethods fails with %d, stream state:\n", iRet);
		strmDebugOutBuf(pStrm);
	}


	RETiRet;
}

/* This is a dummy deserializer, to be used for the delete queue reader
 * specifically. This is kind of a hack, but also to be replace (hopefully) soon
 * by totally different code. So let's make it as simple as possible...
 * rgerhards, 2012-11-06
 */
rsRetVal
objDeserializeDummy(obj_t __attribute__((unused)) *pObj, strm_t *pStrm)
{
	DEFiRet;
	var_t *pVar = NULL;

	CHKiRet(var.Construct(&pVar));
	CHKiRet(var.ConstructFinalize(pVar));

	iRet = objDeserializeProperty(pVar, pStrm);
	while(iRet == RS_RET_OK) {
		/* this loop does actually NOGHTING but read the file... */
		/* re-init var object - TODO: method of var! */
		rsCStrDestruct(&pVar->pcsName); /* no longer needed */
		if(pVar->varType == VARTYPE_STR) {
			if(pVar->val.pStr != NULL)
				rsCStrDestruct(&pVar->val.pStr);
		}
		iRet = objDeserializeProperty(pVar, pStrm);
	}
finalize_it:
	if(iRet == RS_RET_NO_PROPLINE)
		iRet = RS_RET_OK; /* NO_PROPLINE is OK and a kind of EOF! */
	if(pVar != NULL)
		var.Destruct(&pVar);
	RETiRet;
}


/* De-Serialize an object property bag. As a property bag contains only partial properties,
 * it is not instanciable. Thus, the caller must provide a pointer of an already-instanciated
 * object of the correct type.
 * Params: Pointer to object (pObj)
 * Pointer to be passed to the function
 * The caller must destruct the created object.
 * rgerhards, 2008-01-07
 */
static rsRetVal
DeserializePropBag(obj_t *pObj, strm_t *pStrm)
{
	DEFiRet;
	rsRetVal iRetLocal;
	cstr_t *pstrID = NULL;
	int oVers;
	objInfo_t *pObjInfo;

	ISOBJ_assert(pObj);
	ISOBJ_TYPE_assert(pStrm, strm);

	/* we de-serialize the header. if all goes well, we are happy. However, if
	 * we experience a problem, we try to recover. We do this by skipping to
	 * the next object header. This is defined via the line-start cookies. In
	 * worst case, we exhaust the queue, but then we receive EOF return state
	 * from objDeserializeTryRecover(), what will cause us to ultimately give up.
	 * rgerhards, 2008-07-08
	 */
	do {
		iRetLocal = objDeserializeHeader((uchar*) "OPB", &pstrID, &oVers, pStrm);
		if(iRetLocal != RS_RET_OK) {
			dbgprintf("objDeserializePropBag error %d during header - trying to recover\n", iRetLocal);
			CHKiRet(objDeserializeTryRecover(pStrm));
		}
	} while(iRetLocal != RS_RET_OK);

	if(rsCStrSzStrCmp(pstrID, pObj->pObjInfo->pszID, pObj->pObjInfo->lenID))
		ABORT_FINALIZE(RS_RET_INVALID_OID);

	CHKiRet(FindObjInfo((char*)cstrGetSzStrNoNULL(pstrID), &pObjInfo));

	/* we got the object, now we need to fill the properties */
	CHKiRet(objDeserializeProperties(pObj, pObjInfo->objMethods[objMethod_SETPROPERTY], pStrm));

finalize_it:
	if(pstrID != NULL)
		rsCStrDestruct(&pstrID);

	RETiRet;
}

#undef NEXTC /* undef helper macro */


/* --------------- end object serializiation / deserialization support --------------- */


/* set the object (instance) name
 * rgerhards, 2008-01-29
 * TODO: change the naming to a rsCStr obj! (faster)
 */
static rsRetVal
SetName(obj_t *pThis, uchar *pszName)
{
	DEFiRet;

	free(pThis->pszName);
	CHKmalloc(pThis->pszName = ustrdup(pszName));

finalize_it:
	RETiRet;
}


/* get the object (instance) name
 * Note that we use a non-standard calling convention. Thus function must never
 * fail, else we run into real big problems. So it must make sure that at least someting
 * is returned.
 * rgerhards, 2008-01-30
 */
uchar * ATTR_NONNULL()
objGetName(obj_t *const pThis)
{
	uchar *ret;
	uchar szName[128];

	BEGINfunc
	ISOBJ_assert(pThis);

	if(pThis->pszName == NULL) {
		snprintf((char*)szName, sizeof(szName), "%s %p", objGetClassName(pThis), pThis);
		SetName(pThis, szName);
		/* looks strange, but we NEED to re-check because if there was an
		 * error in objSetName(), the pointer may still be NULL
		 */
		if(pThis->pszName == NULL) {
			ret = objGetClassName(pThis);
		} else {
			ret = pThis->pszName;
		}
	} else {
		ret = pThis->pszName;
	}

	ENDfunc
	return ret;
}


/* Find the objInfo object for the current object
 * rgerhards, 2008-02-29
 */
static rsRetVal
FindObjInfo(const char *const __restrict__ strOID, objInfo_t **ppInfo)
{
	DEFiRet;
	int bFound;
	int i;

	bFound = 0;
	i = 0;
	while(!bFound && i < OBJ_NUM_IDS) {
		if(arrObjInfo[i] != NULL && !strcmp(strOID, (const char*)arrObjInfo[i]->pszID)) {
			bFound = 1;
			break;
		}
		++i;
	}

	if(!bFound)
		ABORT_FINALIZE(RS_RET_NOT_FOUND);

	*ppInfo = arrObjInfo[i];

finalize_it:
	if(iRet == RS_RET_OK) {
		/* DEV DEBUG ONLY dbgprintf("caller requested object '%s', found at index %d\n", (*ppInfo)->pszID, i);*/
		/*EMPTY BY INTENSION*/;
	} else {
		dbgprintf("caller requested object '%s', not found (iRet %d)\n", strOID, iRet);
	}

	RETiRet;
}


/* register a classes' info pointer, so that we can reference it later, if needed to
 * (e.g. for de-serialization support).
 * rgerhards, 2008-01-07
 * In this function, we look for a free space in the object table. While we do so, we
 * also detect if the same object has already been registered, which is not valid.
 * rgerhards, 2008-02-29
 */
static rsRetVal
RegisterObj(uchar *pszObjName, objInfo_t *pInfo)
{
	DEFiRet;
	int bFound;
	int i;

	assert(pszObjName != NULL);
	assert(pInfo != NULL);

	bFound = 0;
	i = 0;
	while(!bFound && i < OBJ_NUM_IDS && arrObjInfo[i] != NULL) {
		if(   arrObjInfo[i] != NULL
		   && !ustrcmp(arrObjInfo[i]->pszID, pszObjName)) {
			bFound = 1;
			break;
		}
		++i;
	}

	if(bFound)           ABORT_FINALIZE(RS_RET_OBJ_ALREADY_REGISTERED);
	if(i >= OBJ_NUM_IDS) ABORT_FINALIZE(RS_RET_OBJ_REGISTRY_OUT_OF_SPACE);

	arrObjInfo[i] = pInfo;
	/* DEV debug only: dbgprintf("object '%s' successfully registered with
	index %d, qIF %p\n", pszObjName, i, pInfo->QueryIF); */

finalize_it:
	if(iRet != RS_RET_OK) {
		LogError(0, NO_ERRCODE, "registering object '%s' failed with error code %d", pszObjName, iRet);
	}

	RETiRet;
}


/* deregister a classes' info pointer, usually called because the class is unloaded.
 * After deregistration, the class can no longer be accessed, except if it is reloaded.
 * rgerhards, 2008-03-10
 */
static rsRetVal
UnregisterObj(uchar *pszObjName)
{
	DEFiRet;
	int bFound;
	int i;

	assert(pszObjName != NULL);

	bFound = 0;
	i = 0;
	while(!bFound && i < OBJ_NUM_IDS) {
		if(   arrObjInfo[i] != NULL
		   && !ustrcmp(arrObjInfo[i]->pszID, pszObjName)) {
			bFound = 1;
			break;
		}
		++i;
	}

	if(!bFound)
		ABORT_FINALIZE(RS_RET_OBJ_NOT_REGISTERED);

	InfoDestruct(&arrObjInfo[i]);
	/* DEV debug only: dbgprintf("object '%s' successfully unregistered with index %d\n", pszObjName, i); */

finalize_it:
	if(iRet != RS_RET_OK) {
		dbgprintf("unregistering object '%s' failed with error code %d\n", pszObjName, iRet);
	}

	RETiRet;
}


/* This function shall be called by anyone who would like to use an object. It will
 * try to locate the object, load it into memory if not already present and return
 * a pointer to the objects interface.
 * rgerhards, 2008-02-29
 */
static rsRetVal
UseObj(const char *srcFile, uchar *pObjName, uchar *pObjFile, interface_t *pIf)
{
	DEFiRet;
	objInfo_t *pObjInfo;


	/* DEV debug only: dbgprintf("source file %s requests object '%s',
	ifIsLoaded %d\n", srcFile, pObjName, pIf->ifIsLoaded); */
	pthread_mutex_lock(&mutObjGlobalOp);

	if(pIf->ifIsLoaded == 1) {
		ABORT_FINALIZE(RS_RET_OK); /* we are already set */
	}
	if(pIf->ifIsLoaded == 2) {
		ABORT_FINALIZE(RS_RET_LOAD_ERROR); /* we had a load error and can not continue */
	}

	/* we must be careful that we do not enter in infinite loop if an error occurs during
	 * loading a module. ModLoad emits an error message in such cases and that potentially
	 * can trigger the same code here. So we initially set the module state to "load error"
	 * and set it to "fully initialized" when the load succeeded. It's a bit hackish, but
	 * looks like a good solution. -- rgerhards, 2008-03-07
	 */
	pIf->ifIsLoaded = 2;

	iRet = FindObjInfo((const char*)pObjName, &pObjInfo);
	if(iRet == RS_RET_NOT_FOUND) {
		/* in this case, we need to see if we can dynamically load the object */
		if(pObjFile == NULL) {
			FINALIZE; /* no chance, we have lost... */
		} else {
			CHKiRet(module.Load(pObjFile, 0, NULL));
			/* NOW, we must find it or we have a problem... */
			CHKiRet(FindObjInfo((const char*)pObjName, &pObjInfo));
		}
	} else if(iRet != RS_RET_OK) {
		FINALIZE; /* give up */
	}

	/* if we reach this point, we have a valid pObjInfo */
	if(pObjFile != NULL) { /* NULL means core module */
		module.Use(srcFile, pObjInfo->pModInfo); /* increase refcount */
	}

	CHKiRet(pObjInfo->QueryIF(pIf));
	pIf->ifIsLoaded = 1; /* we are happy */

finalize_it:
	pthread_mutex_unlock(&mutObjGlobalOp);
	RETiRet;
}


/* This function shall be called when a caller is done with an object. Its primary
 * purpose is to keep the reference count correct, which is highly important for
 * modules residing in loadable modules.
 * rgerhards, 2008-03-10
 */
static rsRetVal
ReleaseObj(const char *srcFile, uchar *pObjName, uchar *pObjFile, interface_t *pIf)
{
	DEFiRet;
	objInfo_t *pObjInfo;

	/* dev debug only dbgprintf("source file %s releasing object '%s',
	ifIsLoaded %d\n", srcFile, pObjName, pIf->ifIsLoaded); */
	pthread_mutex_lock(&mutObjGlobalOp);

	if(pObjFile == NULL)
		FINALIZE; /* if it is not a lodable module, we do not need to do anything... */

	if(pIf->ifIsLoaded == 0) {
		FINALIZE; /* we are not loaded - this is perfectly OK... */
	} else if(pIf->ifIsLoaded == 2) {
		pIf->ifIsLoaded = 0; /* clean up */
		FINALIZE; /* we had a load error and can not/must not continue */
	}

	CHKiRet(FindObjInfo((const char*)pObjName, &pObjInfo));

	/* if we reach this point, we have a valid pObjInfo */
	module.Release(srcFile, &pObjInfo->pModInfo); /* decrease refcount */

	pIf->ifIsLoaded = 0; /* indicated "no longer valid" */

finalize_it:
	pthread_mutex_unlock(&mutObjGlobalOp);

	RETiRet;
}


/* queryInterface function
 * rgerhards, 2008-02-29
 */
PROTOTYPEObjQueryInterface(obj);
BEGINobjQueryInterface(obj)
CODESTARTobjQueryInterface(obj)
	if(pIf->ifVersion != objCURR_IF_VERSION) { /* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->UseObj = UseObj;
	pIf->ReleaseObj = ReleaseObj;
	pIf->InfoConstruct = InfoConstruct;
	pIf->DestructObjSelf = DestructObjSelf;
	pIf->BeginSerializePropBag = BeginSerializePropBag;
	pIf->InfoSetMethod = InfoSetMethod;
	pIf->BeginSerialize = BeginSerialize;
	pIf->SerializeProp = SerializeProp;
	pIf->EndSerialize = EndSerialize;
	pIf->RegisterObj = RegisterObj;
	pIf->UnregisterObj = UnregisterObj;
	pIf->Deserialize = Deserialize;
	pIf->DeserializePropBag = DeserializePropBag;
	pIf->SetName = SetName;
	pIf->GetName = objGetName;
finalize_it:
ENDobjQueryInterface(obj)


/* This function returns a pointer to our own interface. It is used as the
 * hook that every object (including dynamically loaded ones) can use to
 * obtain a pointer to our interface which than can be used to obtain
 * pointers to any other interface in the system. This function must be
 * externally visible because of its special nature.
 * rgerhards, 2008-02-29 [nice - will have that date the next time in 4 years ;)]
 */
rsRetVal
objGetObjInterface(obj_if_t *pIf)
{
	DEFiRet;
	assert(pIf != NULL);
	objQueryInterface(pIf);
	RETiRet;
}


/* exit our class
 * rgerhards, 2008-03-11
 */
rsRetVal
objClassExit(void)
{
	DEFiRet;
	/* release objects we no longer need */
	objRelease(strm, CORE_COMPONENT);
	objRelease(var, CORE_COMPONENT);
	objRelease(module, CORE_COMPONENT);

	/* TODO: implement the class exits! */
#if 0
	cfsyslineExit(pModInfo);
	varClassExit(pModInfo);
#endif
	errmsgClassExit();
	moduleClassExit();
	RETiRet;
}


/* initialize our own class
 * Please note that this also initializes those classes that we rely on.
 * Though this is a bit dirty, we need to do it - otherwise we can't get
 * around that bootstrap problem. We need to face the fact the the obj
 * class is a little different from the rest of the system, as it provides
 * the core class loader functionality.
 * rgerhards, 2008-02-29
 */
rsRetVal
objClassInit(modInfo_t *pModInfo)
{
	pthread_mutexattr_t mutAttr;
	int i;
	DEFiRet;
	
	/* first, initialize the object system itself. This must be done
	 * before any other object is created.
	 */
	for(i = 0 ; i < OBJ_NUM_IDS ; ++i) {
		arrObjInfo[i] = NULL;
	}

	/* the mutex must be recursive, because objects may call into other
	 * object identifiers recursively.
	 */
	pthread_mutexattr_init(&mutAttr);
	pthread_mutexattr_settype(&mutAttr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&mutObjGlobalOp, &mutAttr);

	/* request objects we use */
	CHKiRet(objGetObjInterface(&obj)); /* get ourselves ;) */

	/* init classes we use (limit to as few as possible!) */
	CHKiRet(errmsgClassInit(pModInfo));
	CHKiRet(datetimeClassInit(pModInfo));
	CHKiRet(cfsyslineInit());
	CHKiRet(varClassInit(pModInfo));
	CHKiRet(moduleClassInit(pModInfo));
	CHKiRet(strmClassInit(pModInfo));
	CHKiRet(objUse(var, CORE_COMPONENT));
	CHKiRet(objUse(module, CORE_COMPONENT));
	CHKiRet(objUse(strm, CORE_COMPONENT));

finalize_it:
	RETiRet;
}

/* vi:set ai:
 */
