/* Definition of the generic obj class module.
 *
 * This module relies heavily on preprocessor macros in order to
 * provide fast execution time AND ease of use.
 *
 * Each object that uses this base class MUST provide a constructor with
 * the following interface:
 *
 * Destruct(pThis);
 *
 * A constructor is not necessary (except for some features, e.g. de-serialization).
 * If it is provided, it is a three-part constructor (to handle all cases with a
 * generic interface):
 *
 * Construct(&pThis);
 * SetProperty(pThis, property_t *);
 * ConstructFinalize(pThis);
 *
 * SetProperty() and ConstructFinalize() may also be called on an object
 * instance which has been Construct()'ed outside of this module.
 *
 * pThis always references to a pointer of the object.
 *
 * Copyright 2008-2012 Adiscon GmbH.
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

#ifndef OBJ_H_INCLUDED
#define OBJ_H_INCLUDED

#include "obj-types.h"
#include "var.h"
#include "stream.h"

/* macros */
/* the following one is a helper that prevents us from writing the
 * ever-same code at the end of Construct()
 */
#define OBJCONSTRUCT_CHECK_SUCCESS_AND_CLEANUP \
	if(iRet == RS_RET_OK) { \
		*ppThis = pThis; \
	} else { \
		if(pThis != NULL) \
			free(pThis); \
	}

#define objSerializeSCALAR_VAR(strm, propName, propType, var) \
	CHKiRet(obj.SerializeProp(strm, (uchar*) #propName, PROPTYPE_##propType, (void*) &var));
#define objSerializeSCALAR(strm, propName, propType) \
	CHKiRet(obj.SerializeProp(strm, (uchar*) #propName, PROPTYPE_##propType, (void*) &pThis->propName));
#define objSerializePTR(strm, propName, propType) \
	CHKiRet(obj.SerializeProp(strm, (uchar*) #propName, PROPTYPE_##propType, (void*) pThis->propName));
#define DEFobjStaticHelpers \
	static objInfo_t __attribute__((unused)) *pObjInfoOBJ = NULL; \
	DEFobjCurrIf(obj)


#define objGetClassName(pThis) (((obj_t*) (pThis))->pObjInfo->pszID)
#define objGetVersion(pThis) (((obj_t*) (pThis))->pObjInfo->iObjVers)
/* the next macro MUST be called in Constructors: */
#ifndef NDEBUG /* this means if debug... */
#	define objConstructSetObjInfo(pThis) \
		((obj_t*) (pThis))->pObjInfo = pObjInfoOBJ; \
		((obj_t*) (pThis))->pszName = NULL; \
		((obj_t*) (pThis))->iObjCooCKiE = 0xBADEFEE
#else
#	define objConstructSetObjInfo(pThis) \
		((obj_t*) (pThis))->pObjInfo = pObjInfoOBJ; \
		((obj_t*) (pThis))->pszName = NULL
#endif
#define objSerialize(pThis) (((obj_t*) (pThis))->pObjInfo->objMethods[objMethod_SERIALIZE])

#define OBJSetMethodHandler(methodID, pHdlr) \
	CHKiRet(obj.InfoSetMethod(pObjInfoOBJ, methodID, (rsRetVal (*)()) pHdlr))

/* interfaces */
BEGINinterface(obj) /* name must also be changed in ENDinterface macro! */
	rsRetVal (*UseObj)(const char *srcFile, uchar *pObjName, uchar *pObjFile, interface_t *pIf);
	rsRetVal (*ReleaseObj)(const char *srcFile, uchar *pObjName, uchar *pObjFile, interface_t *pIf);
	rsRetVal (*InfoConstruct)(objInfo_t **ppThis, uchar *pszID, int iObjVers,
		                  rsRetVal (*pConstruct)(void *), rsRetVal (*pDestruct)(void *),
	      			  rsRetVal (*pQueryIF)(interface_t*), modInfo_t*);
	rsRetVal (*DestructObjSelf)(obj_t *pThis);
	rsRetVal (*BeginSerializePropBag)(strm_t *pStrm, obj_t *pObj);
	rsRetVal (*InfoSetMethod)(objInfo_t *pThis, objMethod_t objMethod, rsRetVal (*pHandler)(void*));
	rsRetVal (*BeginSerialize)(strm_t *pStrm, obj_t *pObj);
	rsRetVal (*SerializeProp)(strm_t *pStrm, uchar *pszPropName, propType_t propType, void *pUsr);
	rsRetVal (*EndSerialize)(strm_t *pStrm);
	rsRetVal (*RegisterObj)(uchar *pszObjName, objInfo_t *pInfo);
	rsRetVal (*UnregisterObj)(uchar *pszObjName);
	rsRetVal (*Deserialize)(void *ppObj, uchar *pszTypeExpected, strm_t *pStrm, rsRetVal (*fFixup)(obj_t*,void*),
	void *pUsr);
	rsRetVal (*DeserializePropBag)(obj_t *pObj, strm_t *pStrm);
	rsRetVal (*SetName)(obj_t *pThis, uchar *pszName);
	uchar *  (*GetName)(obj_t *pThis);
ENDinterface(obj)
#define objCURR_IF_VERSION 2 /* increment whenever you change the interface structure! */


/* prototypes */
/* the following define *is* necessary, because it provides the root way of obtaining
 * interfaces (at some place we need to start our query...
 */
rsRetVal objGetObjInterface(obj_if_t *pIf);
PROTOTYPEObjClassInit(obj);
PROTOTYPEObjClassExit(obj);
rsRetVal objDeserializeWithMethods(void *ppObj, uchar *pszTypeExpected, int lenTypeExpected, strm_t *pStrm,
rsRetVal (*fFixup)(obj_t*,void*), void *pUsr, rsRetVal (*objConstruct)(), rsRetVal (*objConstructFinalize)(),
rsRetVal (*objDeserialize)());
rsRetVal objDeserializeProperty(var_t *pProp, strm_t *pStrm);
rsRetVal objDeserializeDummy(obj_t *pObj, strm_t *pStrm);
uchar *objGetName(obj_t *pThis);


/* the following definition is only for "friends" */
extern pthread_mutex_t mutObjGlobalOp;	/* mutex to guard global operations of the object system */

#endif /* #ifndef OBJ_H_INCLUDED */
