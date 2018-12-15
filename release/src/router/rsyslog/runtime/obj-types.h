/* Some type definitions and macros for the obj object.
 * I needed to move them out of the main obj.h, because obj.h's
 * prototypes use other data types. However, their .h's rely
 * on some of the obj.h data types and macros. So I needed to break
 * that loop somehow and I've done that by moving the typedefs
 * into this file here.
 *
 * Copyright 2008-2012 Rainer Gerhards and Adiscon GmbH.
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

#ifndef OBJ_TYPES_H_INCLUDED
#define OBJ_TYPES_H_INCLUDED

#include "stringbuf.h"
#include "syslogd-types.h"

/* property types for obj[De]Serialize() */
typedef enum {
	PROPTYPE_NONE = 0, /* currently no value set */
	PROPTYPE_PSZ = 1,
	PROPTYPE_SHORT = 2,
	PROPTYPE_INT = 3,
	PROPTYPE_LONG = 4,
	PROPTYPE_INT64 = 5,
	PROPTYPE_CSTR = 6,
	PROPTYPE_SYSLOGTIME = 7
} propType_t;

typedef unsigned objID_t;

typedef enum {	/* IDs of base methods supported by all objects - used for jump table, so
		 * they must start at zero and be incremented. -- rgerhards, 2008-01-04
		 */
	objMethod_CONSTRUCT = 0,
	objMethod_DESTRUCT = 1,
	objMethod_SERIALIZE = 2,
	objMethod_DESERIALIZE = 3,
	objMethod_SETPROPERTY = 4,
	objMethod_CONSTRUCTION_FINALIZER = 5,
	objMethod_GETSEVERITY = 6,
	objMethod_DEBUGPRINT = 7
} objMethod_t;
#define OBJ_NUM_METHODS 8	/* must be updated to contain the max number of methods supported */


/* the base data type for interfaces
 * This MUST be in sync with the ifBEGIN macro
 */
struct interface_s {
	int ifVersion;	/* must be set to version requested */
	int ifIsLoaded;
	/* is the interface loaded? (0-no, 1-yes, 2-load failed; if not 1, functions can NOT be called! */
};


struct objInfo_s {
	uchar *pszID; /* the object ID as a string */
	size_t lenID; /* length of the ID string */
	int iObjVers;
	uchar *pszName;
	rsRetVal (*objMethods[OBJ_NUM_METHODS])();
	rsRetVal (*QueryIF)(interface_t*);
	struct modInfo_s *pModInfo;
};


struct obj_s {	/* the dummy struct that each derived class can be casted to */
	objInfo_t *pObjInfo;
#ifndef NDEBUG /* this means if debug... */
	unsigned int iObjCooCKiE; /* must always be 0xBADEFEE for a valid object */
#endif
	uchar *pszName;		/* the name of *this* specific object instance */
};


/* macros which must be gloablly-visible (because they are used during definition of
 * other objects.
 */
#ifndef NDEBUG /* this means if debug... */
#include <string.h>
#	define BEGINobjInstance \
		obj_t objData
#	define ISOBJ_assert(pObj) \
		do { \
		ASSERT((pObj) != NULL); \
		ASSERT((unsigned) ((obj_t*)(pObj))->iObjCooCKiE == (unsigned) 0xBADEFEE); \
		} while(0);
#	define ISOBJ_TYPE_assert(pObj, objType) \
		do { \
		ASSERT(pObj != NULL); \
		if(strcmp((char*)(((obj_t*)pObj)->pObjInfo->pszID), #objType)) { \
			dbgprintf("%s:%d ISOBJ assert failure: invalid object type, expected '%s' " \
				  "actual '%s', cookie: %X\n", __FILE__, __LINE__, #objType, \
				  (((obj_t*)pObj)->pObjInfo->pszID), ((obj_t*)(pObj))->iObjCooCKiE); \
			fprintf(stderr, "%s:%d ISOBJ assert failure: invalid object type, expected '%s' " \
				  "actual '%s', cookie: %X\n", __FILE__, __LINE__, #objType, \
				  (((obj_t*)pObj)->pObjInfo->pszID), ((obj_t*)(pObj))->iObjCooCKiE); \
			fflush(stderr); \
			assert(!strcmp((char*)(((obj_t*)pObj)->pObjInfo->pszID), #objType)); \
		} \
		ASSERT((unsigned) ((obj_t*)(pObj))->iObjCooCKiE == (unsigned) 0xBADEFEE); \
		} while(0)
	/* now the same for pointers to "regular" objects (like wrkrInstanceData) */
#	define PTR_ASSERT_DEF unsigned int _Assert_type;
#	define PTR_ASSERT_SET_TYPE(_ptr, _type) _ptr->_Assert_type = _type
#	define PTR_ASSERT_CHK(_ptr, _type) do { \
		assert(_ptr != NULL); \
		if(_ptr->_Assert_type != _type) {\
			dbgprintf("%s:%d PTR_ASSERT_CHECK failure: invalid pointer type %x, " \
				"expected %x\n", __FILE__, __LINE__, _ptr->_Assert_type, _type); \
			fprintf(stderr, "%s:%d PTR_ASSERT_CHECK failure: invalid pointer type %x, " \
				"expected %x\n", __FILE__, __LINE__, _ptr->_Assert_type, _type); \
			assert(_ptr->_Assert_type == _type); \
		} \
	} while(0)
#else /* non-debug mode, no checks but much faster */
#	define BEGINobjInstance obj_t objData
#	define ISOBJ_TYPE_assert(pObj, objType)
#	define ISOBJ_assert(pObj)

#	define PTR_ASSERT_DEF
#	define PTR_ASSERT_SET_TYPE(_ptr, _type)
#	define PTR_ASSERT_CHK(_ptr, _type)
#endif

/* a set method for *very simple* object accesses. Note that this does
 * NOT conform to the standard calling conventions and should be
 * used only if actually nothing can go wrong! -- rgerhards, 2008-04-17
 */
#define DEFpropGetMeth(obj, prop, dataType)\
	dataType obj##Get##prop(void)\
	{ \
		return pThis->prop = pVal; \
	}

#define DEFpropSetMethPTR(obj, prop, dataType)\
	rsRetVal obj##Set##prop(obj##_t *pThis, dataType *pVal)\
	{ \
		/* DEV debug: dbgprintf("%sSet%s()\n", #obj, #prop); */\
		pThis->prop = pVal; \
		return RS_RET_OK; \
	}
#define PROTOTYPEpropSetMethPTR(obj, prop, dataType)\
	rsRetVal obj##Set##prop(obj##_t *pThis, dataType*)
#define DEFpropSetMethFP(obj, prop, dataType)\
	rsRetVal obj##Set##prop(obj##_t *pThis, dataType)\
	{ \
		/* DEV debug: dbgprintf("%sSet%s()\n", #obj, #prop); */\
		pThis->prop = pVal; \
		return RS_RET_OK; \
	}
#define PROTOTYPEpropSetMethFP(obj, prop, dataType)\
	rsRetVal obj##Set##prop(obj##_t *pThis, dataType)
#define DEFpropSetMeth(obj, prop, dataType)\
	rsRetVal obj##Set##prop(obj##_t *pThis, dataType pVal);\
	rsRetVal obj##Set##prop(obj##_t *pThis, dataType pVal)\
	{ \
		/* DEV debug: dbgprintf("%sSet%s()\n", #obj, #prop); */\
		pThis->prop = pVal; \
		return RS_RET_OK; \
	}
#define PROTOTYPEpropSetMeth(obj, prop, dataType)\
	rsRetVal obj##Set##prop(obj##_t *pThis, dataType pVal)
#define INTERFACEpropSetMeth(obj, prop, dataType)\
	rsRetVal (*Set##prop)(obj##_t *pThis, dataType)
/* class initializer */
#define PROTOTYPEObjClassInit(objName) rsRetVal objName##ClassInit(struct modInfo_s*)
/* below: objName must be the object name (e.g. vm, strm, ...) and ISCORE must be
 * 1 if the module is a statically linked core module and 0 if it is a
 * dynamically loaded one. -- rgerhards, 2008-02-29
 */
#define OBJ_IS_CORE_MODULE 1 /* This should better be renamed to something like "OBJ_IS_NOT_LIBHEAD" or so... ;) */
#define OBJ_IS_LOADABLE_MODULE 0
#define BEGINObjClassInit(objName, objVers, objType) \
rsRetVal objName##ClassInit(struct modInfo_s *pModInfo) \
{ \
	DEFiRet; \
	if(objType == OBJ_IS_CORE_MODULE) { /* are we a core module? */ \
		CHKiRet(objGetObjInterface(&obj)); /* this provides the root pointer for all other queries */ \
	} \
	CHKiRet(obj.InfoConstruct(&pObjInfoOBJ, (uchar*) #objName, objVers, \
	                         (rsRetVal (*)(void*))objName##Construct,\
				 (rsRetVal (*)(void*))objName##Destruct,\
				 (rsRetVal (*)(interface_t*))objName##QueryInterface, pModInfo)); \

#define ENDObjClassInit(objName) \
	iRet = obj.RegisterObj((uchar*)#objName, pObjInfoOBJ); \
finalize_it: \
	RETiRet; \
}

/* ... and now the same for abstract classes.
 * TODO: consolidate the two -- rgerhards, 2008-02-29
 */
#define BEGINAbstractObjClassInit(objName, objVers, objType) \
rsRetVal objName##ClassInit(struct modInfo_s *pModInfo) \
{ \
	DEFiRet; \
	if(objType == OBJ_IS_CORE_MODULE) { /* are we a core module? */ \
		CHKiRet(objGetObjInterface(&obj)); /* this provides the root pointer for all other queries */ \
	} \
	CHKiRet(obj.InfoConstruct(&pObjInfoOBJ, (uchar*) #objName, objVers, \
	                         NULL,\
				 NULL,\
				 (rsRetVal (*)(interface_t*))objName##QueryInterface, pModInfo));

#define ENDObjClassInit(objName) \
	iRet = obj.RegisterObj((uchar*)#objName, pObjInfoOBJ); \
finalize_it: \
	RETiRet; \
}


/* now come the class exit. This is to be called immediately before the class is
 * unloaded (actual unload for plugins, program termination for core modules)
 * gerhards, 2008-03-10
 */
#define PROTOTYPEObjClassExit(objName) rsRetVal objName##ClassExit(void)
#define BEGINObjClassExit(objName, objType) \
rsRetVal objName##ClassExit(void) \
{ \
	DEFiRet;

#define CODESTARTObjClassExit(objName)

#define ENDObjClassExit(objName) \
	iRet = obj.UnregisterObj((uchar*)#objName); \
	RETiRet; \
}

/* this defines both the constructor and initializer
 * rgerhards, 2008-01-10
 */
#define BEGINobjConstruct(obj) \
	static rsRetVal obj##Initialize(obj##_t __attribute__((unused)) *pThis) \
	{ \
		DEFiRet;

#define ENDobjConstruct(obj) \
		/* use finalize_it: before calling the macro (if you need it)! */ \
		RETiRet; \
	} \
	rsRetVal obj##Construct(obj##_t **ppThis); \
	rsRetVal obj##Construct(obj##_t **ppThis) \
	{ \
		DEFiRet; \
		obj##_t *pThis; \
	 \
		ASSERT(ppThis != NULL); \
	 \
		if((pThis = (obj##_t *)calloc(1, sizeof(obj##_t))) == NULL) { \
			ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY); \
		} \
		objConstructSetObjInfo(pThis); \
	 \
		obj##Initialize(pThis); \
	\
	finalize_it: \
		OBJCONSTRUCT_CHECK_SUCCESS_AND_CLEANUP \
		RETiRet; \
	}


/* this defines the destructor. The important point is that the base object
 * destructor is called. The upper-level class shall destruct all of its
 * properties, but not the instance itself. This is freed here by the
 * framework (we need an intact pointer because we need to free the
 * obj_t structures inside it). A pointer to the object pointer must be
 * parse, because it is re-set to NULL (this, for example, is important in
 * cancellation handlers). The object pointer is always named pThis.
 * The object is always freed, even if there is some error while
 * Cancellation is blocked during destructors, as this could have fatal
 * side-effects. However, this also means the upper-level object should
 * not perform any lenghty processing.
 * IMPORTANT: if the upper level object requires some situations where the
 * object shall not be destructed (e.g. via reference counting), then
 * it shall set pThis to NULL, which prevents destruction of the
 * object.
 * processing.
 * rgerhards, 2008-01-30
 */
#define PROTOTYPEobjDestruct(OBJ) \
	rsRetVal OBJ##Destruct(OBJ##_t __attribute__((unused)) **ppThis)
/* note: we generate a prototype in any case, as this does not hurt but
 * many modules otherwise seem to miss one, which generates compiler
 * warnings.
 */
#define BEGINobjDestruct(OBJ) \
	rsRetVal OBJ##Destruct(OBJ##_t __attribute__((unused)) **ppThis);\
	rsRetVal OBJ##Destruct(OBJ##_t __attribute__((unused)) **ppThis) \
	{ \
		DEFiRet; \
		OBJ##_t *pThis;

#define CODESTARTobjDestruct(OBJ) \
		ASSERT(ppThis != NULL); \
		pThis = *ppThis; \
		ISOBJ_TYPE_assert(pThis, OBJ);

/* note: there was a long-time bug in the macro below that lead to *ppThis = NULL
 * only when the object was actually destructed. I discovered this issue during
 * introduction of the pRcvFrom property in smsg_t, but it potentially had other
 * effects, too. I am not sure if some experienced instability resulted from this
 * bug OR if its fix will cause harm to so-far "correctly" running code. The later
 * may very well be. Thus I will change it only for the current branch and also
 * the beta, but not in all old builds. Let's see how things evolve.
 * rgerhards, 2009-06-30
 */
#define ENDobjDestruct(OBJ) \
	 	goto finalize_it; /* prevent compiler warning ;) */ \
	 	/* no more code here! */ \
	finalize_it: \
		if(pThis != NULL) { \
			obj.DestructObjSelf((obj_t*) pThis); \
			free(pThis); \
		} \
		*ppThis = NULL; \
		RETiRet; \
	}


/* this defines the debug print entry point. DebugPrint is optional. If
 * it is provided, the object should output some meaningful information
 * via the debug system.
 * rgerhards, 2008-02-20
 */
#define PROTOTYPEObjDebugPrint(obj) rsRetVal obj##DebugPrint(obj##_t *pThis)
#define INTERFACEObjDebugPrint(obj) rsRetVal (*DebugPrint)(obj##_t *pThis)
#define BEGINobjDebugPrint(obj) \
	rsRetVal obj##DebugPrint(obj##_t __attribute__((unused)) *pThis);\
	rsRetVal obj##DebugPrint(obj##_t __attribute__((unused)) *pThis) \
	{ \
		DEFiRet; \

#define CODESTARTobjDebugPrint(obj) \
		ASSERT(pThis != NULL); \
		ISOBJ_TYPE_assert(pThis, obj); \

#define ENDobjDebugPrint(obj) \
		RETiRet; \
	}

/* ------------------------------ object loader system ------------------------------ *
 * The following code builds a dynamic object loader system. The
 * root idea is that all objects are dynamically loadable,
 * which is necessary to get a clean plug-in interface where every plugin can access
 * rsyslog's rich object model via simple and quite portable methods.
 *
 * To do so, each object defines one or more interfaces. They are essentially structures
 * with function (method) pointers. Anyone interested in calling an object must first
 * obtain the interface and can then call through it.
 *
 * The interface data type must always be called <obj>_if_t, as this is expected
 * by the macros. Having consitent naming is also easier for the programmer. By default,
 * macros create a static variable named like the object in each calling objects
 * static data block.
 *
 * rgerhards, 2008-02-21 (initial implementation), 2008-04-17 (update of this note)
 */

/* this defines the QueryInterface print entry point. Over time, it should be
 * present in all objects.
 */
#define BEGINobjQueryInterface(obj) \
	rsRetVal obj##QueryInterface(obj##_if_t *pIf);\
	rsRetVal obj##QueryInterface(obj##_if_t *pIf) \
	{ \
		DEFiRet; \

#define CODESTARTobjQueryInterface(obj) \
		ASSERT(pIf != NULL);

#define ENDobjQueryInterface(obj) \
		RETiRet; \
	}

#define PROTOTYPEObjQueryInterface(obj) rsRetVal obj##QueryInterface(obj##_if_t *pIf)


/* the following macros should be used to define interfaces inside the
 * header files.
 */
#define BEGINinterface(obj) \
	typedef struct obj##_if_s {\
		ifBEGIN		/* This MUST always be the first interface member */
#define ENDinterface(obj) \
	} obj##_if_t;
	
/* the following macro is used to get access to an object (not an instance,
 * just the class itself!). It must be called before any of the object's
 * methods can be accessed. The MYLIB part is the name of my library, or NULL if
 * the caller is a core module. Using the right value here is important to get
 * the reference counting correct (object accesses from the same library must
 * not be counted because that would cause a library plugin to never unload, as
 * its ClassExit() entry points are only called if no object is referenced, which
 * would never happen as the library references itself.
 * rgerhards, 2008-03-11
 */
#define CORE_COMPONENT NULL /* use this to indicate this is a core component */
#define DONT_LOAD_LIB NULL /* do not load a library to obtain object interface (currently same as CORE_COMPONENT) */
#define objUse(objName, FILENAME) \
	obj.UseObj(__FILE__, (uchar*)#objName, (uchar*)FILENAME, (void*) &objName)
#define objRelease(objName, FILENAME) \
	obj.ReleaseObj(__FILE__, (uchar*)#objName, (uchar*) FILENAME, (void*) &objName)

/* defines data that must always be present at the very begin of the interface structure */
#define ifBEGIN \
	int ifVersion;	/* must be set to version requested */ \
	int ifIsLoaded; /* is the interface loaded? (0-no, 1-yes; if no, functions can NOT be called! */


/* use the following define some place in your static data (suggested right at
 * the beginning
 */
#define DEFobjCurrIf(obj) \
		static obj##_if_t obj = { .ifVersion = obj##CURR_IF_VERSION, .ifIsLoaded = 0 };

/* define the prototypes for a class - when we use interfaces, we just have few
 * functions that actually need to be non-static.
 */
#define PROTOTYPEObj(obj) \
	PROTOTYPEObjClassInit(obj); \
	PROTOTYPEObjClassExit(obj); \
	PROTOTYPEObjQueryInterface(obj)

/* ------------------------------ end object loader system ------------------------------ */


#include "modules.h"

#endif /* #ifndef OBJ_TYPES_H_INCLUDED */
