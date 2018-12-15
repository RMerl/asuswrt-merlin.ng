/* The statsobj object.
 *
 * This object provides a statistics-gathering facility inside rsyslog. This
 * functionality will be pragmatically implemented and extended.
 *
 * Copyright 2010-2017 Adiscon GmbH.
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
#include <stdarg.h>
#include <inttypes.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <json.h>

#include "rsyslog.h"
#include "unicode-helper.h"
#include "obj.h"
#include "statsobj.h"
#include "srUtils.h"
#include "stringbuf.h"
#include "errmsg.h"
#include "hashtable.h"
#include "hashtable_itr.h"


/* externally-visiable data (see statsobj.h for explanation) */
int GatherStats = 0;

/* static data */
DEFobjStaticHelpers

/* doubly linked list of stats objects. Object is automatically linked to it
 * upon construction. Enqueue always happens at the front (simplifies logic).
 */
static statsobj_t *objRoot = NULL;
static statsobj_t *objLast = NULL;

static pthread_mutex_t mutStats;
static pthread_mutex_t mutSenders;

static struct hashtable *stats_senders = NULL;

/* ------------------------------ statsobj linked list maintenance  ------------------------------ */

static void
addToObjList(statsobj_t *pThis)
{
	pthread_mutex_lock(&mutStats);
	if (pThis->flags && STATSOBJ_FLAG_DO_PREPEND) {
		pThis->next = objRoot;
		if (objRoot != NULL) {
			objRoot->prev = pThis;
		}
		objRoot = pThis;
		if (objLast == NULL)
			objLast = pThis;
	} else {
		pThis->prev = objLast;
		if(objLast != NULL)
			objLast->next = pThis;
		objLast = pThis;
		if(objRoot == NULL)
			objRoot = pThis;
	}
	pthread_mutex_unlock(&mutStats);
}


static void
removeFromObjList(statsobj_t *pThis)
{
	pthread_mutex_lock(&mutStats);
	if(pThis->prev != NULL)
		pThis->prev->next = pThis->next;
	if(pThis->next != NULL)
		pThis->next->prev = pThis->prev;
	if(objLast == pThis)
		objLast = pThis->prev;
	if(objRoot == pThis)
		objRoot = pThis->next;
	pthread_mutex_unlock(&mutStats);
}


static void
addCtrToList(statsobj_t *pThis, ctr_t *pCtr)
{
	pthread_mutex_lock(&pThis->mutCtr);
	pCtr->prev = pThis->ctrLast;
	if(pThis->ctrLast != NULL)
		pThis->ctrLast->next = pCtr;
	pThis->ctrLast = pCtr;
	if(pThis->ctrRoot == NULL)
		pThis->ctrRoot = pCtr;
	pthread_mutex_unlock(&pThis->mutCtr);
}

/* ------------------------------ methods ------------------------------ */


/* Standard-Constructor
 */
BEGINobjConstruct(statsobj) /* be sure to specify the object type also in END macro! */
	pthread_mutex_init(&pThis->mutCtr, NULL);
	pThis->ctrLast = NULL;
	pThis->ctrRoot = NULL;
	pThis->read_notifier = NULL;
	pThis->flags = 0;
ENDobjConstruct(statsobj)


/* ConstructionFinalizer
 */
static rsRetVal
statsobjConstructFinalize(statsobj_t *pThis)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, statsobj);
	addToObjList(pThis);
	RETiRet;
}

/* set read_notifier (a function which is invoked after stats are read).
 */
static rsRetVal
setReadNotifier(statsobj_t *pThis, statsobj_read_notifier_t notifier, void* ctx)
{
	DEFiRet;
	pThis->read_notifier = notifier;
	pThis->read_notifier_ctx = ctx;
	RETiRet;
}


/* set origin (module name, etc).
 * Note that we make our own copy of the memory, caller is
 * responsible to free up name it passes in (if required).
 */
static rsRetVal
setOrigin(statsobj_t *pThis, uchar *origin)
{
	DEFiRet;
	CHKmalloc(pThis->origin = ustrdup(origin));
finalize_it:
	RETiRet;
}


/* set name. Note that we make our own copy of the memory, caller is
 * responsible to free up name it passes in (if required).
 */
static rsRetVal
setName(statsobj_t *pThis, uchar *name)
{
	DEFiRet;
	CHKmalloc(pThis->name = ustrdup(name));
finalize_it:
	RETiRet;
}

static void
setStatsObjFlags(statsobj_t *pThis, int flags) {
	pThis->flags = flags;
}

static rsRetVal
setReportingNamespace(statsobj_t *pThis, uchar *ns)
{
	DEFiRet;
	CHKmalloc(pThis->reporting_ns = ustrdup(ns));
finalize_it:
	RETiRet;
}

/* add a counter to an object
 * ctrName is duplicated, caller must free it if requried
 * NOTE: The counter is READ-ONLY and MUST NOT be modified (most
 * importantly, it must not be initialized, so the caller must
 * ensure the counter is properly initialized before AddCounter()
 * is called.
 */
static rsRetVal
addManagedCounter(statsobj_t *pThis, const uchar *ctrName, statsCtrType_t ctrType, int8_t flags, void *pCtr,
ctr_t **entryRef, int8_t linked)
{
	ctr_t *ctr;
	DEFiRet;

	*entryRef = NULL;

	CHKmalloc(ctr = calloc(1, sizeof(ctr_t)));
	ctr->next = NULL;
	ctr->prev = NULL;
	if((ctr->name = ustrdup(ctrName)) == NULL) {
		DBGPRINTF("addCounter: OOM in strdup()\n");
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
	}
	ctr->flags = flags;
	ctr->ctrType = ctrType;
	switch(ctrType) {
	case ctrType_IntCtr:
		ctr->val.pIntCtr = (intctr_t*) pCtr;
		break;
	case ctrType_Int:
		ctr->val.pInt = (int*) pCtr;
		break;
	}
	if (linked) {
		addCtrToList(pThis, ctr);
	}
	*entryRef = ctr;

finalize_it:
	if (iRet != RS_RET_OK) {
		if (ctr != NULL) {
			free(ctr->name);
			free(ctr);
		}
	}
	RETiRet;
}

static void
addPreCreatedCounter(statsobj_t *pThis, ctr_t *pCtr)
{
	pCtr->next = NULL;
	pCtr->prev = NULL;
	addCtrToList(pThis, pCtr);
}

static rsRetVal
addCounter(statsobj_t *pThis, const uchar *ctrName, statsCtrType_t ctrType, int8_t flags, void *pCtr)
{
	ctr_t *ctr;
	DEFiRet;
	iRet = addManagedCounter(pThis, ctrName, ctrType, flags, pCtr, &ctr, 1);
	RETiRet;
}

static void
destructUnlinkedCounter(ctr_t *ctr) {
	free(ctr->name);
	free(ctr);
}

static void
destructCounter(statsobj_t *pThis, ctr_t *pCtr)
{
	pthread_mutex_lock(&pThis->mutCtr);
	if (pCtr->prev != NULL) {
		pCtr->prev->next = pCtr->next;
	}
	if (pCtr->next != NULL) {
		pCtr->next->prev = pCtr->prev;
	}
	if (pThis->ctrLast == pCtr) {
		pThis->ctrLast = pCtr->prev;
	}
	if (pThis->ctrRoot == pCtr) {
		pThis->ctrRoot = pCtr->next;
	}
	pthread_mutex_unlock(&pThis->mutCtr);
	destructUnlinkedCounter(pCtr);
}

static void
resetResettableCtr(ctr_t *pCtr, int8_t bResetCtrs)
{
	if ((bResetCtrs && (pCtr->flags & CTR_FLAG_RESETTABLE)) ||
		(pCtr->flags & CTR_FLAG_MUST_RESET)) {
		switch(pCtr->ctrType) {
		case ctrType_IntCtr:
			*(pCtr->val.pIntCtr) = 0;
			break;
		case ctrType_Int:
			*(pCtr->val.pInt) = 0;
			break;
		}
	}
}

static rsRetVal
addCtrForReporting(json_object *to, const uchar* field_name, intctr_t value) {
	json_object *v;
	DEFiRet;

	/*We should migrate libfastjson to support uint64_t in addition to int64_t.
	  Although no counter is likely to grow to int64 max-value, this is theoritically
	  incorrect (as intctr_t is uint64)*/
	CHKmalloc(v = json_object_new_int64((int64_t) value));

	json_object_object_add(to, (const char*) field_name, v);
finalize_it:
	/* v cannot be NULL in error case, as this would only happen during malloc fail,
	 * which itself sets it to NULL -- so not doing cleanup here.
	 */
	RETiRet;
}

static rsRetVal
addContextForReporting(json_object *to, const uchar* field_name, const uchar* value) {
	json_object *v;
	DEFiRet;

	CHKmalloc(v = json_object_new_string((const char*) value));

	json_object_object_add(to, (const char*) field_name, v);
finalize_it:
	RETiRet;
}

static intctr_t
accumulatedValue(ctr_t *pCtr) {
	switch(pCtr->ctrType) {
	case ctrType_IntCtr:
		return *(pCtr->val.pIntCtr);
	case ctrType_Int:
		return *(pCtr->val.pInt);
	}
	return -1;
}


/* get all the object's countes together as CEE. */
static rsRetVal
getStatsLineCEE(statsobj_t *pThis, cstr_t **ppcstr, const statsFmtType_t fmt, const int8_t bResetCtrs)
{
	cstr_t *pcstr = NULL;
	ctr_t *pCtr;
	json_object *root, *values;
	int locked = 0;
	DEFiRet;

	root = values = NULL;

	CHKiRet(cstrConstruct(&pcstr));

	if (fmt == statsFmt_CEE)
		CHKiRet(rsCStrAppendStrWithLen(pcstr, UCHAR_CONSTANT(CONST_CEE_COOKIE" "), CONST_LEN_CEE_COOKIE + 1));

	CHKmalloc(root = json_object_new_object());

	CHKiRet(addContextForReporting(root, UCHAR_CONSTANT("name"), pThis->name));
	
	if(pThis->origin != NULL) {
		CHKiRet(addContextForReporting(root, UCHAR_CONSTANT("origin"), pThis->origin));
	}

	if (pThis->reporting_ns == NULL) {
		values = json_object_get(root);
	} else {
		CHKmalloc(values = json_object_new_object());
		json_object_object_add(root, (const char*) pThis->reporting_ns, json_object_get(values));
	}

	/* now add all counters to this line */
	pthread_mutex_lock(&pThis->mutCtr);
	locked = 1;
	for(pCtr = pThis->ctrRoot ; pCtr != NULL ; pCtr = pCtr->next) {
		if (fmt == statsFmt_JSON_ES) {
			/* work-around for broken Elasticsearch JSON implementation:
			 * we need to replace dots by a different char, we use bang.
			 * Note: ES 2.0 does not longer accept dot in name
			 */
			uchar esbuf[256];
			strncpy((char*)esbuf, (char*)pCtr->name, sizeof(esbuf)-1);
			esbuf[sizeof(esbuf)-1] = '\0';
			for(uchar *c = esbuf ; *c ; ++c) {
				if(*c == '.')
					*c = '!';
			}
			CHKiRet(addCtrForReporting(values, esbuf, accumulatedValue(pCtr)));
		} else {
			CHKiRet(addCtrForReporting(values, pCtr->name, accumulatedValue(pCtr)));
		}
		resetResettableCtr(pCtr, bResetCtrs);
	}
	pthread_mutex_unlock(&pThis->mutCtr);
	locked = 0;
	CHKiRet(rsCStrAppendStr(pcstr, (const uchar*) json_object_to_json_string(root)));

	cstrFinalize(pcstr);
	*ppcstr = pcstr;
	pcstr = NULL;

finalize_it:
	if(locked) {
		pthread_mutex_unlock(&pThis->mutCtr);
	}

	if (pcstr != NULL) {
		cstrDestruct(&pcstr);
	}
	if (root != NULL) {
		json_object_put(root);
	}
	if (values != NULL) {
		json_object_put(values);
	}
	
	RETiRet;
}

/* get all the object's countes together with object name as one line.
 */
static rsRetVal
getStatsLine(statsobj_t *pThis, cstr_t **ppcstr, int8_t bResetCtrs)
{
	cstr_t *pcstr;
	ctr_t *pCtr;
	DEFiRet;

	CHKiRet(cstrConstruct(&pcstr));
	rsCStrAppendStr(pcstr, pThis->name);
	rsCStrAppendStrWithLen(pcstr, UCHAR_CONSTANT(": "), 2);

	if(pThis->origin != NULL) {
		rsCStrAppendStrWithLen(pcstr, UCHAR_CONSTANT("origin="), 7);
		rsCStrAppendStr(pcstr, pThis->origin);
		cstrAppendChar(pcstr, ' ');
	}

	/* now add all counters to this line */
	pthread_mutex_lock(&pThis->mutCtr);
	for(pCtr = pThis->ctrRoot ; pCtr != NULL ; pCtr = pCtr->next) {
		rsCStrAppendStr(pcstr, pCtr->name);
		cstrAppendChar(pcstr, '=');
		switch(pCtr->ctrType) {
		case ctrType_IntCtr:
			rsCStrAppendInt(pcstr, *(pCtr->val.pIntCtr)); // TODO: OK?????
			break;
		case ctrType_Int:
			rsCStrAppendInt(pcstr, *(pCtr->val.pInt));
			break;
		}
		cstrAppendChar(pcstr, ' ');
		resetResettableCtr(pCtr, bResetCtrs);
	}
	pthread_mutex_unlock(&pThis->mutCtr);

	cstrFinalize(pcstr);
	*ppcstr = pcstr;

finalize_it:
	RETiRet;
}



/* this function obtains all sender stats. hlper to getAllStatsLines()
 * We need to keep this looked to avoid resizing of the hash table
 * (what could otherwise cause a segfault).
 */
static void
getSenderStats(rsRetVal(*cb)(void*, const char*),
	void *usrptr,
	statsFmtType_t fmt,
	const int8_t bResetCtrs)
{
	struct hashtable_itr *itr = NULL;
	struct sender_stats *stat;
	char fmtbuf[2048];

	pthread_mutex_lock(&mutSenders);

	/* Iterator constructor only returns a valid iterator if
	 * the hashtable is not empty
	 */
	if(hashtable_count(stats_senders) > 0) {
		itr = hashtable_iterator(stats_senders);
		do {
			stat = (struct sender_stats*)hashtable_iterator_value(itr);
			if(fmt == statsFmt_Legacy) {
				snprintf(fmtbuf, sizeof(fmtbuf),
					"_sender_stat: sender=%s messages=%"
					PRIu64,
					stat->sender, stat->nMsgs);
			} else {
				snprintf(fmtbuf, sizeof(fmtbuf),
					"{ \"name\":\"_sender_stat\", "
					"\"sender\":\"%s\", \"messages\":\"%"
					PRIu64 "\"}",
					stat->sender, stat->nMsgs);
			}
			fmtbuf[sizeof(fmtbuf)-1] = '\0';
			cb(usrptr, fmtbuf);
			if(bResetCtrs)
				stat->nMsgs = 0;
		} while (hashtable_iterator_advance(itr));
	}

	free(itr);
	pthread_mutex_unlock(&mutSenders);
}


/* this function can be used to obtain all stats lines. In this case,
 * a callback must be provided. This module than iterates over all objects and
 * submits each stats line to the callback. The callback has two parameters:
 * the first one is a caller-provided void*, the second one the cstr_t with the
 * line. If the callback reports an error, processing is stopped.
 */
static rsRetVal
getAllStatsLines(rsRetVal(*cb)(void*, const char*), void *const usrptr, statsFmtType_t fmt, const int8_t bResetCtrs)
{
	statsobj_t *o;
	cstr_t *cstr = NULL;
	DEFiRet;

	for(o = objRoot ; o != NULL ; o = o->next) {
		switch(fmt) {
		case statsFmt_Legacy:
			CHKiRet(getStatsLine(o, &cstr, bResetCtrs));
			break;
		case statsFmt_CEE:
		case statsFmt_JSON:
		case statsFmt_JSON_ES:
			CHKiRet(getStatsLineCEE(o, &cstr, fmt, bResetCtrs));
			break;
		}
		CHKiRet(cb(usrptr, (const char*)cstrGetSzStrNoNULL(cstr)));
		rsCStrDestruct(&cstr);
		if (o->read_notifier != NULL) {
			o->read_notifier(o, o->read_notifier_ctx);
		}
	}

	getSenderStats(cb, usrptr, fmt, bResetCtrs);

finalize_it:
	if(cstr != NULL) {
		rsCStrDestruct(&cstr);
	}
	RETiRet;
}

/* Enable statistics gathering. currently there is no function to disable it
 * again, as this is right now not needed.
 */
static rsRetVal
enableStats(void)
{
	GatherStats = 1;
	return RS_RET_OK;
}


rsRetVal
statsRecordSender(const uchar *sender, unsigned nMsgs, time_t lastSeen)
{
	struct sender_stats *stat;
	int mustUnlock = 0;
	DEFiRet;

	if(stats_senders == NULL)
		FINALIZE;	/* unlikely: we could not init our hash table */

	pthread_mutex_lock(&mutSenders);
	mustUnlock = 1;
	stat = hashtable_search(stats_senders, (void*)sender);
	if(stat == NULL) {
		DBGPRINTF("statsRecordSender: sender '%s' not found, adding\n",
			sender);
		CHKmalloc(stat = calloc(1, sizeof(struct sender_stats)));
		stat->sender = (const uchar*)strdup((const char*)sender);
		stat->nMsgs = 0;
		if(glblReportNewSenders) {
			LogMsg(0, RS_RET_SENDER_APPEARED,
				LOG_INFO, "new sender '%s'", stat->sender);
		}
		if(hashtable_insert(stats_senders, (void*)stat->sender,
			(void*)stat) == 0) {
			LogError(errno, RS_RET_INTERNAL_ERROR,
				"error inserting sender '%s' into sender "
				"hash table", sender);
			ABORT_FINALIZE(RS_RET_INTERNAL_ERROR);
		}
	}

	stat->nMsgs += nMsgs;
	stat->lastSeen = lastSeen;
	DBGPRINTF("DDDDD: statsRecordSender: '%s', nmsgs %u [%llu], lastSeen %llu\n", sender, nMsgs,
	(long long unsigned) stat->nMsgs, (long long unsigned) lastSeen);

finalize_it:
	if(mustUnlock)
		pthread_mutex_unlock(&mutSenders);
	RETiRet;
}

static ctr_t*
unlinkAllCounters(statsobj_t *pThis) {
	ctr_t *ctr;
	pthread_mutex_lock(&pThis->mutCtr);
	ctr = pThis->ctrRoot;
	pThis->ctrLast = NULL;
	pThis->ctrRoot = NULL;
	pthread_mutex_unlock(&pThis->mutCtr);
	return ctr;
}

static void
destructUnlinkedCounters(ctr_t *ctr) {
	ctr_t *ctrToDel;

	while(ctr != NULL) {
		ctrToDel = ctr;
		ctr = ctr->next;
		destructUnlinkedCounter(ctrToDel);
	}
}

/* check if a sender has not sent info to us for an extended period
 * of time.
 */
void
checkGoneAwaySenders(const time_t tCurr)
{
	struct hashtable_itr *itr = NULL;
	struct sender_stats *stat;
	const time_t rqdLast = tCurr - glblSenderStatsTimeout;
	struct tm tm;

	pthread_mutex_lock(&mutSenders);

	/* Iterator constructor only returns a valid iterator if
	 * the hashtable is not empty
	 */
	if(hashtable_count(stats_senders) > 0) {
		itr = hashtable_iterator(stats_senders);
		do {
			stat = (struct sender_stats*)hashtable_iterator_value(itr);
			if(stat->lastSeen < rqdLast) {
				if(glblReportGoneAwaySenders) {
					localtime_r(&stat->lastSeen, &tm);
					LogMsg(0, RS_RET_SENDER_GONE_AWAY,
						LOG_WARNING,
						"removing sender '%s' from connection "
						"table, last seen at "
						"%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d",
						stat->sender,
						tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
						tm.tm_hour, tm.tm_min, tm.tm_sec);
				}
				hashtable_remove(stats_senders, (void*)stat->sender);
			}
		} while (hashtable_iterator_advance(itr));
	}

	pthread_mutex_unlock(&mutSenders);
	free(itr);
}

/* destructor for the statsobj object */
BEGINobjDestruct(statsobj) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDestruct(statsobj)
	removeFromObjList(pThis);

	/* destruct counters */
	destructUnlinkedCounters(unlinkAllCounters(pThis));

	pthread_mutex_destroy(&pThis->mutCtr);
	free(pThis->name);
	free(pThis->origin);
	free(pThis->reporting_ns);
ENDobjDestruct(statsobj)


/* debugprint for the statsobj object */
BEGINobjDebugPrint(statsobj) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDebugPrint(statsobj)
	dbgoprint((obj_t*) pThis, "statsobj object, currently no state info available\n");
ENDobjDebugPrint(statsobj)


/* queryInterface function
 */
BEGINobjQueryInterface(statsobj)
CODESTARTobjQueryInterface(statsobj)
	if(pIf->ifVersion != statsobjCURR_IF_VERSION) { /* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->Construct = statsobjConstruct;
	pIf->ConstructFinalize = statsobjConstructFinalize;
	pIf->Destruct = statsobjDestruct;
	pIf->DebugPrint = statsobjDebugPrint;
	pIf->SetName = setName;
	pIf->SetOrigin = setOrigin;
	pIf->SetReadNotifier = setReadNotifier;
	pIf->SetReportingNamespace = setReportingNamespace;
	pIf->SetStatsObjFlags = setStatsObjFlags;
	pIf->GetAllStatsLines = getAllStatsLines;
	pIf->AddCounter = addCounter;
	pIf->AddManagedCounter = addManagedCounter;
	pIf->AddPreCreatedCtr = addPreCreatedCounter;
	pIf->DestructCounter = destructCounter;
	pIf->DestructUnlinkedCounter = destructUnlinkedCounter;
	pIf->UnlinkAllCounters = unlinkAllCounters;
	pIf->EnableStats = enableStats;
finalize_it:
ENDobjQueryInterface(statsobj)


/* Initialize the statsobj class. Must be called as the very first method
 * before anything else is called inside this class.
 */
BEGINAbstractObjClassInit(statsobj, 1, OBJ_IS_CORE_MODULE) /* class, version */
	/* request objects we use */

	/* set our own handlers */
	OBJSetMethodHandler(objMethod_DEBUGPRINT, statsobjDebugPrint);
	OBJSetMethodHandler(objMethod_CONSTRUCTION_FINALIZER, statsobjConstructFinalize);

	/* init other data items */
	pthread_mutex_init(&mutStats, NULL);
	pthread_mutex_init(&mutSenders, NULL);

	if((stats_senders = create_hashtable(100, hash_from_string, key_equals_string, NULL)) == NULL) {
		LogError(0, RS_RET_INTERNAL_ERROR, "error trying to initialize hash-table "
			"for sender table. Sender statistics and warnings are disabled.");
		ABORT_FINALIZE(RS_RET_INTERNAL_ERROR);
	}
ENDObjClassInit(statsobj)

/* Exit the class.
 */
BEGINObjClassExit(statsobj, OBJ_IS_CORE_MODULE) /* class, version */
	/* release objects we no longer need */
	pthread_mutex_destroy(&mutStats);
	pthread_mutex_destroy(&mutSenders);
	hashtable_destroy(stats_senders, 1);
ENDObjClassExit(statsobj)
