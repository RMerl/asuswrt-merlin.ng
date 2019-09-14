/* omkafka.c
 * This output plugin make rsyslog talk to Apache Kafka.
 *
 * Copyright 2014-2017 by Adiscon GmbH.
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
#include "rsyslog.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/uio.h>
#include <sys/queue.h>
#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
#	include <sys/stat.h>
#endif
#include <unistd.h>
#include <librdkafka/rdkafka.h>
#include "syslogd-types.h"
#include "srUtils.h"
#include "template.h"
#include "module-template.h"
#include "errmsg.h"
#include "atomic.h"
#include "statsobj.h"
#include "unicode-helper.h"
#include "datetime.h"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("omkafka")

/* internal structures
 */
DEF_OMOD_STATIC_DATA
DEFobjCurrIf(datetime)
DEFobjCurrIf(strm)
DEFobjCurrIf(statsobj)

statsobj_t *kafkaStats;
STATSCOUNTER_DEF(ctrQueueSize, mutCtrQueueSize);
STATSCOUNTER_DEF(ctrTopicSubmit, mutCtrTopicSubmit);
STATSCOUNTER_DEF(ctrKafkaFail, mutCtrKafkaFail);
STATSCOUNTER_DEF(ctrCacheMiss, mutCtrCacheMiss);
STATSCOUNTER_DEF(ctrCacheEvict, mutCtrCacheEvict);
STATSCOUNTER_DEF(ctrCacheSkip, mutCtrCacheSkip);
STATSCOUNTER_DEF(ctrKafkaAck, mutCtrKafkaAck);
STATSCOUNTER_DEF(ctrKafkaMsgTooLarge, mutCtrKafkaMsgTooLarge);
STATSCOUNTER_DEF(ctrKafkaUnknownTopic, mutCtrKafkaUnknownTopic);
STATSCOUNTER_DEF(ctrKafkaQueueFull, mutCtrKafkaQueueFull);
STATSCOUNTER_DEF(ctrKafkaUnknownPartition, mutCtrKafkaUnknownPartition);
STATSCOUNTER_DEF(ctrKafkaOtherErrors, mutCtrKafkaOtherErrors);
STATSCOUNTER_DEF(ctrKafkaRespTimedOut, mutCtrKafkaRespTimedOut);
STATSCOUNTER_DEF(ctrKafkaRespTransport, mutCtrKafkaRespTransport);
STATSCOUNTER_DEF(ctrKafkaRespBrokerDown, mutCtrKafkaRespBrokerDown);
STATSCOUNTER_DEF(ctrKafkaRespAuth, mutCtrKafkaRespAuth);
STATSCOUNTER_DEF(ctrKafkaRespOther, mutCtrKafkaRespOther);

#define MAX_ERRMSG 1024 /* max size of error messages that we support */

#define NO_FIXED_PARTITION -1	/* signifies that no fixed partition config exists */

struct kafka_params {
	const char *name;
	const char *val;
};

#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif

/* flags for writeKafka: shall we resubmit a failed message? */
#define RESUBMIT	1
#define NO_RESUBMIT	0

#ifdef HAVE_ATOMIC_BUILTINS64
static uint64 clockTopicAccess = 0;
#else
static unsigned clockTopicAccess = 0;
#endif
/* and the "tick" function */
#ifndef HAVE_ATOMIC_BUILTINS
static pthread_mutex_t mutClock;
#endif
static uint64
getClockTopicAccess(void)
{
#ifdef HAVE_ATOMIC_BUILTINS64
	return ATOMIC_INC_AND_FETCH_uint64(&clockTopicAccess, &mutClock);
#else
	return ATOMIC_INC_AND_FETCH_unsigned(&clockTopicAccess, &mutClock);
#endif
}

/* Needed for Kafka timestamp librdkafka > 0.9.4 */
#define KAFKA_TimeStamp "\"%timestamp:::date-unixtimestamp%\""

static int closeTimeout = 1000;
static pthread_mutex_t closeTimeoutMut = PTHREAD_MUTEX_INITIALIZER;

/* stats callback window metrics */
static uint64 rtt_avg_usec;
static uint64 throttle_avg_msec;
static uint64 int_latency_avg_usec;

/* dynamic topic cache */
struct s_dynaTopicCacheEntry {
	uchar *pName;
	rd_kafka_topic_t *pTopic;
	uint64 clkTickAccessed;
	pthread_rwlock_t lock;
};
typedef struct s_dynaTopicCacheEntry dynaTopicCacheEntry;

/* Struct for Failed Messages Listitems */
struct s_failedmsg_entry {
	uchar* payload;
	uchar* topicname;
	SLIST_ENTRY(s_failedmsg_entry) entries;	/*	List. */
} ;
typedef struct s_failedmsg_entry failedmsg_entry;

typedef struct _instanceData {
	uchar *topic;
	sbool dynaTopic;
	dynaTopicCacheEntry **dynCache;
	pthread_mutex_t mutDynCache;
	rd_kafka_topic_t *pTopic;
	int iCurrElt;
	int iCurrCacheSize;
	int bReportErrs;
	int iDynaTopicCacheSize;
	uchar *tplName;		/* assigned output template */
	char *brokers;
	sbool autoPartition;
	int fixedPartition;
	int nPartitions;
	uint32_t currPartition;
	DEF_ATOMIC_HELPER_MUT(mutCurrPartition);
	int nConfParams;
	struct kafka_params *confParams;
	int nTopicConfParams;
	struct kafka_params *topicConfParams;
	uchar *errorFile;
	uchar *key;
	int bReopenOnHup;
	int bResubmitOnFailure;	/* Resubmit failed messages into kafka queue*/
	int bKeepFailedMessages;/* Keep Failed messages in memory,
							only works if bResubmitOnFailure is enabled */
	uchar *failedMsgFile;	/* file in which failed messages are being stored on
							shutdown and loaded on startup */

	int fdErrFile;		/* error file fd or -1 if not open */
	pthread_mutex_t mutErrFile;
	int bIsOpen;
	int bIsSuspended;	/* when broker fail, we need to suspend the action */
	pthread_rwlock_t rkLock;
	pthread_mutex_t mut_doAction; /* make sure one wrkr instance max in parallel */
	rd_kafka_t *rk;
	int closeTimeout;
	SLIST_HEAD(failedmsg_listhead, s_failedmsg_entry) failedmsg_head;
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
} wrkrInstanceData_t;


/* tables for interfacing with the v6 config system */
/* action (instance) parameters */
static struct cnfparamdescr actpdescr[] = {
	{ "topic", eCmdHdlrString, CNFPARAM_REQUIRED },
	{ "dynatopic", eCmdHdlrBinary, 0 },
	{ "dynatopic.cachesize", eCmdHdlrInt, 0 },
	{ "partitions.auto", eCmdHdlrBinary, 0 },	/* use librdkafka's automatic partitioning function */
	{ "partitions.number", eCmdHdlrPositiveInt, 0 },
	{ "partitions.usefixed", eCmdHdlrNonNegInt, 0 }, /* expert parameter, "nails" partition */
	{ "broker", eCmdHdlrArray, 0 },
	{ "confparam", eCmdHdlrArray, 0 },
	{ "topicconfparam", eCmdHdlrArray, 0 },
	{ "errorfile", eCmdHdlrGetWord, 0 },
	{ "key", eCmdHdlrGetWord, 0 },
	{ "template", eCmdHdlrGetWord, 0 },
	{ "closetimeout", eCmdHdlrPositiveInt, 0 },
	{ "reopenonhup", eCmdHdlrBinary, 0 },
	{ "resubmitonfailure", eCmdHdlrBinary, 0 },	/* Resubmit message into kafaj queue on failure */
	{ "keepfailedmessages", eCmdHdlrBinary, 0 },
	{ "failedmsgfile", eCmdHdlrGetWord, 0 }
};
static struct cnfparamblk actpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(actpdescr)/sizeof(struct cnfparamdescr),
	  actpdescr
	};

BEGINinitConfVars		/* (re)set config variables to default values */
CODESTARTinitConfVars
ENDinitConfVars

static uint32_t
getPartition(instanceData *const __restrict__ pData)
{
	if (pData->autoPartition) {
		return RD_KAFKA_PARTITION_UA;
	} else {
		return (pData->fixedPartition == NO_FIXED_PARTITION) ?
		          ATOMIC_INC_AND_FETCH_unsigned(&pData->currPartition,
			      &pData->mutCurrPartition) % pData->nPartitions
			:  (unsigned) pData->fixedPartition;
	}
}

/* must always be called with appropriate locks taken */
static void
d_free_topic(rd_kafka_topic_t **topic)
{
	if (*topic != NULL) {
		DBGPRINTF("omkafka: closing topic %s\n", rd_kafka_topic_name(*topic));
		rd_kafka_topic_destroy(*topic);
		*topic = NULL;
	}
}

static void ATTR_NONNULL(1)
failedmsg_entry_destruct(failedmsg_entry *const __restrict__ fmsgEntry) {
	free(fmsgEntry->payload);
	free(fmsgEntry->topicname);
	free(fmsgEntry);
}

/* note: we need the length of message as we need to deal with
 * non-NUL terminated strings under some circumstances.
 */
static failedmsg_entry * ATTR_NONNULL()
failedmsg_entry_construct(const char *const msg, const size_t msglen, const char *const topicname)
{
	failedmsg_entry *etry = NULL;

	if((etry = malloc(sizeof(struct s_failedmsg_entry))) == NULL) {
		return NULL;
	}
	if((etry->payload = (uchar*)malloc(msglen+1)) == NULL) {
		free(etry);
		return NULL;
	}
	memcpy(etry->payload, msg, msglen);
	etry->payload[msglen] = '\0';
	if((etry->topicname = (uchar*)strdup(topicname)) == NULL) {
		free(etry->payload);
		free(etry);
		return NULL;
	}
	return etry;
}

/* destroy topic item */
/* must be called with write(rkLock) */
static void
closeTopic(instanceData *__restrict__ const pData)
{
	d_free_topic(&pData->pTopic);
}

/* these dynaTopic* functions are only slightly modified versions of those found in omfile.c.
 * check the sources in omfile.c for more descriptive comments about each of these functions.
 * i will only put the bare descriptions in this one. 2015-01-09 - Tait Clarridge
 */

/* delete a cache entry from the dynamic topic cache */
/* must be called with lock(mutDynCache) */
static rsRetVal
dynaTopicDelCacheEntry(instanceData *__restrict__ const pData, const int iEntry, const int bFreeEntry)
{
	dynaTopicCacheEntry **pCache = pData->dynCache;
	DEFiRet;
	ASSERT(pCache != NULL);

	if(pCache[iEntry] == NULL)
		FINALIZE;
	pthread_rwlock_wrlock(&pCache[iEntry]->lock);

	DBGPRINTF("Removing entry %d for topic '%s' from dynaCache.\n", iEntry,
		pCache[iEntry]->pName == NULL ? UCHAR_CONSTANT("[OPEN FAILED]") : pCache[iEntry]->pName);

	if(pCache[iEntry]->pName != NULL) {
		d_free(pCache[iEntry]->pName);
		pCache[iEntry]->pName = NULL;
	}

	pthread_rwlock_unlock(&pCache[iEntry]->lock);

	if(bFreeEntry) {
		pthread_rwlock_destroy(&pCache[iEntry]->lock);
		d_free(pCache[iEntry]);
		pCache[iEntry] = NULL;
	}

finalize_it:
	RETiRet;
}

/* clear the entire dynamic topic cache */
static void
dynaTopicFreeCacheEntries(instanceData *__restrict__ const pData)
{
	register int i;
	ASSERT(pData != NULL);

	BEGINfunc;
	pthread_mutex_lock(&pData->mutDynCache);
	for(i = 0 ; i < pData->iCurrCacheSize ; ++i) {
		dynaTopicDelCacheEntry(pData, i, 1);
	}
	pData->iCurrElt = -1; /* invalidate current element */
	pthread_mutex_unlock(&pData->mutDynCache);
	ENDfunc;
}

/* create the topic object */
/* must be called with _atleast_ read(rkLock) */
static rsRetVal
createTopic(instanceData *__restrict__ const pData, const uchar *__restrict__ const newTopicName,
rd_kafka_topic_t** topic) {
/* Get a new topic conf */
	rd_kafka_topic_conf_t *const topicconf = rd_kafka_topic_conf_new();
	char errstr[MAX_ERRMSG];
	rd_kafka_topic_t *rkt = NULL;
	DEFiRet;

	*topic = NULL;

	if(topicconf == NULL) {
		LogError(0, RS_RET_KAFKA_ERROR,
			"omkafka: error creating kafka topic conf obj: %s\n",
			rd_kafka_err2str(rd_kafka_last_error()));
		ABORT_FINALIZE(RS_RET_KAFKA_ERROR);
	}
	for(int i = 0 ; i < pData->nTopicConfParams ; ++i) {
		DBGPRINTF("omkafka: setting custom topic configuration parameter: %s:%s\n",
			pData->topicConfParams[i].name,
			pData->topicConfParams[i].val);
		if(rd_kafka_topic_conf_set(topicconf, pData->topicConfParams[i].name,
		   pData->topicConfParams[i].val, errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
			if(pData->bReportErrs) {
				LogError(0, RS_RET_PARAM_ERROR, "error in kafka "
					"topic conf parameter '%s=%s': %s",
					pData->topicConfParams[i].name,
					pData->topicConfParams[i].val, errstr);
			} else {
				DBGPRINTF("omkafka: setting custom topic configuration parameter '%s=%s': %s",
					pData->topicConfParams[i].name,
					pData->topicConfParams[i].val, errstr);
			}
			ABORT_FINALIZE(RS_RET_PARAM_ERROR);
		}
	}
	rkt = rd_kafka_topic_new(pData->rk, (char *)newTopicName, topicconf);
	if(rkt == NULL) {
		LogError(0, RS_RET_KAFKA_ERROR,
			"omkafka: error creating kafka topic: %s\n",
			rd_kafka_err2str(rd_kafka_last_error()));
		ABORT_FINALIZE(RS_RET_KAFKA_ERROR);
	}
	*topic = rkt;
finalize_it:
	RETiRet;
}

/* create the topic object */
/* must be called with write(rkLock) */
static rsRetVal
prepareTopic(instanceData *__restrict__ const pData, const uchar *__restrict__ const newTopicName)
{
	DEFiRet;
	iRet = createTopic(pData, newTopicName, &pData->pTopic);
	if(iRet != RS_RET_OK) {
		if(pData->pTopic != NULL) {
			closeTopic(pData);
		}
	}
	RETiRet;
}

/* check dynamic topic cache for existence of the already created topic.
 * if it does not exist, create a new one, or if we are currently using it
 * as of the last message, keep using it.
 *
 * must be called with read(rkLock)
 * must be called with mutDynCache locked
 */
static rsRetVal ATTR_NONNULL()
prepareDynTopic(instanceData *__restrict__ const pData, const uchar *__restrict__ const newTopicName,
				rd_kafka_topic_t** topic, pthread_rwlock_t** lock)
{
	uint64 ctOldest;
	int iOldest;
	int i;
	int iFirstFree;
	rsRetVal localRet;
	dynaTopicCacheEntry **pCache;
	dynaTopicCacheEntry *entry = NULL;
	rd_kafka_topic_t *tmpTopic = NULL;
	DEFiRet;
	ASSERT(pData != NULL);
	ASSERT(newTopicName != NULL);

	pCache = pData->dynCache;
	/* first check, if we still have the current topic */
	if ((pData->iCurrElt != -1)
		&& !ustrcmp(newTopicName, pCache[pData->iCurrElt]->pName)) {
			/* great, we are all set */
			pCache[pData->iCurrElt]->clkTickAccessed = getClockTopicAccess();
			entry = pCache[pData->iCurrElt];
			STATSCOUNTER_INC(ctrCacheSkip, mutCtrCacheSkip);
			FINALIZE;
	}

	/* ok, no luck. Now let's search the table if we find a matching spot.
	 * While doing so, we also prepare for creation of a new one.
	 */
	pData->iCurrElt = -1;
	iFirstFree = -1;
	iOldest = 0;
	ctOldest = getClockTopicAccess();
	for(i = 0 ; i < pData->iCurrCacheSize ; ++i) {
		if(pCache[i] == NULL || pCache[i]->pName == NULL) {
			if(iFirstFree == -1)
				iFirstFree = i;
		} else { /*got an element, let's see if it matches */
			if(!ustrcmp(newTopicName, pCache[i]->pName)) {
				/* we found our element! */
				entry = pCache[i];
				pData->iCurrElt = i;
				/* update "timestamp" for LRU */
				pCache[i]->clkTickAccessed = getClockTopicAccess();
				FINALIZE;
			}
			/* did not find it - so lets keep track of the counters for LRU */
			if(pCache[i]->clkTickAccessed < ctOldest) {
				ctOldest = pCache[i]->clkTickAccessed;
				iOldest = i;
			}
		}
	}
	STATSCOUNTER_INC(ctrCacheMiss, mutCtrCacheMiss);

	/* invalidate iCurrElt as we may error-exit out of this function when the currrent
	 * iCurrElt has been freed or otherwise become unusable. This is a precaution, and
	 * performance-wise it may be better to do that in each of the exits. However, that
	 * is error-prone, so I prefer to do it here. -- rgerhards, 2010-03-02
	 */
	pData->iCurrElt = -1;

	if(iFirstFree == -1 && (pData->iCurrCacheSize < pData->iDynaTopicCacheSize)) {
		/* there is space left, so set it to that index */
		iFirstFree = pData->iCurrCacheSize++;
	}

	if(iFirstFree == -1) {
		dynaTopicDelCacheEntry(pData, iOldest, 0);
		STATSCOUNTER_INC(ctrCacheEvict, mutCtrCacheEvict);
		iFirstFree = iOldest; /* this one *is* now free ;) */
	} else {
		pCache[iFirstFree] = NULL;
	}
	/* we need to allocate memory for the cache structure */
	if(pCache[iFirstFree] == NULL) {
		CHKmalloc(pCache[iFirstFree] =
			(dynaTopicCacheEntry*) calloc(1, sizeof(dynaTopicCacheEntry)));
		CHKiRet(pthread_rwlock_init(&pCache[iFirstFree]->lock, NULL));
	}

	/* Ok, we finally can open the topic */
	localRet = createTopic(pData, newTopicName, &tmpTopic);

	if(localRet != RS_RET_OK) {
		LogError(0, localRet, "Could not open dynamic topic '%s' "
			"[state %d] - discarding message",
		newTopicName, localRet);
		ABORT_FINALIZE(localRet);
	}

	if((pCache[iFirstFree]->pName = ustrdup(newTopicName)) == NULL) {
		d_free_topic(&tmpTopic);
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
	}
	pCache[iFirstFree]->pTopic = tmpTopic;
	pCache[iFirstFree]->clkTickAccessed = getClockTopicAccess();
	entry = pCache[iFirstFree];
	pData->iCurrElt = iFirstFree;
	DBGPRINTF("Added new entry %d for topic cache, topic '%s'.\n", iFirstFree, newTopicName);

finalize_it:
	if (iRet == RS_RET_OK) {
		*topic = entry->pTopic;
		*lock = &entry->lock;
	}
	RETiRet;
}

/* write data error request/replies to separate error file
 * Note: we open the file but never close it before exit. If it
 * needs to be closed, HUP must be sent.
 */
static rsRetVal
writeDataError(instanceData *const pData,
	const char *const __restrict__ data,
	const size_t lenData,
	const int kafkaErr)
{
	int bLocked = 0;
	struct json_object *json = NULL;
	DEFiRet;

	if(pData->errorFile == NULL) {
		FINALIZE;
	}

	json = json_object_new_object();
	if(json == NULL) {
		ABORT_FINALIZE(RS_RET_ERR);
	}
	struct json_object *jval;
	jval = json_object_new_int(kafkaErr);
	json_object_object_add(json, "errcode", jval);
	jval = json_object_new_string(rd_kafka_err2str(kafkaErr));
	json_object_object_add(json, "errmsg", jval);
	jval = json_object_new_string_len(data, lenData);
	json_object_object_add(json, "data", jval);

	struct iovec iov[2];
	iov[0].iov_base = (void*) json_object_get_string(json);
	iov[0].iov_len = strlen(iov[0].iov_base);
	iov[1].iov_base = (char *) "\n";
	iov[1].iov_len = 1;

	/* we must protect the file write do operations due to other wrks & HUP */
	pthread_mutex_lock(&pData->mutErrFile);
	bLocked = 1;
	if(pData->fdErrFile == -1) {
		pData->fdErrFile = open((char*)pData->errorFile,
					O_WRONLY|O_CREAT|O_APPEND|O_LARGEFILE|O_CLOEXEC,
					S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
		if(pData->fdErrFile == -1) {
			LogError(errno, RS_RET_ERR, "omkafka: error opening error file %s",
				pData->errorFile);
			ABORT_FINALIZE(RS_RET_ERR);
		}
	}

	/* Note: we do not do real error-handling on the err file, as this
	 * complicates things way to much.
	 */
	const ssize_t nwritten = writev(pData->fdErrFile, iov, sizeof(iov)/sizeof(struct iovec));
	if(nwritten != (ssize_t) iov[0].iov_len + 1) {
		LogError(errno, RS_RET_ERR,
			"omkafka: error writing error file, write returns %lld\n",
			(long long) nwritten);
	}

finalize_it:
	if(bLocked)
		pthread_mutex_unlock(&pData->mutErrFile);
	if(json != NULL)
		json_object_put(json);
	RETiRet;
}

/* identify and count specific types of kafka failures.
 */
static rsRetVal
updateKafkaFailureCounts(rd_kafka_resp_err_t err) {
	DEFiRet;
	if (err == RD_KAFKA_RESP_ERR_MSG_SIZE_TOO_LARGE) {
		STATSCOUNTER_INC(ctrKafkaMsgTooLarge, mutCtrKafkaMsgTooLarge);
	} else if (err == RD_KAFKA_RESP_ERR__UNKNOWN_TOPIC) {
		STATSCOUNTER_INC(ctrKafkaUnknownTopic, mutCtrKafkaUnknownTopic);
	} else if (err == RD_KAFKA_RESP_ERR__QUEUE_FULL) {
		STATSCOUNTER_INC(ctrKafkaQueueFull, mutCtrKafkaQueueFull);
	} else if (err == RD_KAFKA_RESP_ERR__UNKNOWN_PARTITION) {
		STATSCOUNTER_INC(ctrKafkaUnknownPartition, mutCtrKafkaUnknownPartition);
	} else {
		STATSCOUNTER_INC(ctrKafkaOtherErrors, mutCtrKafkaOtherErrors);
	}

	RETiRet;
}

/* must be called with read(rkLock)
 * b_do_resubmit tells if we shall resubmit on error or not. This is needed
 *               when we submit already resubmitted messages.
 */
static rsRetVal ATTR_NONNULL(1, 2)
writeKafka(instanceData *const pData, uchar *const msg,
	uchar *const msgTimestamp, uchar *const topic, const int b_do_resubmit)
{
	DEFiRet;
	const int partition = getPartition(pData);
	rd_kafka_topic_t *rkt = NULL;
	pthread_rwlock_t *dynTopicLock = NULL;
	failedmsg_entry* fmsgEntry;
	int topic_mut_locked = 0;
	rd_kafka_resp_err_t msg_kafka_response;
#if RD_KAFKA_VERSION >= 0x00090400
	int64_t ttMsgTimestamp;
#else
	int msg_enqueue_status = 0;
#endif

	DBGPRINTF("omkafka: trying to send: key:'%s', msg:'%s', timestamp:'%s'\n",
		pData->key, msg, msgTimestamp);

	if(pData->dynaTopic) {
		DBGPRINTF("omkafka: topic to insert to: %s\n", topic);
		/* ensure locking happens all inside this function */
		pthread_mutex_lock(&pData->mutDynCache);
		const rsRetVal localRet = prepareDynTopic(pData, topic, &rkt, &dynTopicLock);
		if (localRet == RS_RET_OK) {
			pthread_rwlock_rdlock(dynTopicLock);
			topic_mut_locked = 1;
		}
		pthread_mutex_unlock(&pData->mutDynCache);
		CHKiRet(localRet);
	} else {
		rkt = pData->pTopic;
	}

#if RD_KAFKA_VERSION >= 0x00090400
	if (msgTimestamp == NULL) {
		/* Resubmitted items don't have a timestamp */
		ttMsgTimestamp = 0;
	} else {
		ttMsgTimestamp = atoi((char*)msgTimestamp); /* Convert timestamp into int */
		ttMsgTimestamp *= 1000; /* Timestamp in Milliseconds for kafka */
	}
	DBGPRINTF("omkafka: rd_kafka_producev timestamp=%s/%" PRId64 "\n", msgTimestamp, ttMsgTimestamp);

	/* Using new kafka producev API, includes Timestamp! */
	if (pData->key == NULL) {
		msg_kafka_response = rd_kafka_producev(pData->rk,
						RD_KAFKA_V_RKT(rkt),
						RD_KAFKA_V_PARTITION(partition),
						RD_KAFKA_V_VALUE(msg, strlen((char*)msg)),
						RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
						RD_KAFKA_V_TIMESTAMP(ttMsgTimestamp),
						RD_KAFKA_V_KEY(NULL, 0),
						RD_KAFKA_V_END);
	} else {
		DBGPRINTF("omkafka: rd_kafka_producev key=%s\n", pData->key);
		msg_kafka_response = rd_kafka_producev(pData->rk,
						RD_KAFKA_V_RKT(rkt),
						RD_KAFKA_V_PARTITION(partition),
						RD_KAFKA_V_VALUE(msg, strlen((char*)msg)),
						RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
						RD_KAFKA_V_TIMESTAMP(ttMsgTimestamp),
						RD_KAFKA_V_KEY(pData->key,strlen((char*)pData->key)),
						RD_KAFKA_V_END);
	}

	if (msg_kafka_response != RD_KAFKA_RESP_ERR_NO_ERROR ) {
		updateKafkaFailureCounts(msg_kafka_response);

		/* Put into kafka queue, again if configured! */
		if (pData->bResubmitOnFailure && b_do_resubmit) {
			DBGPRINTF("omkafka: Failed to produce to topic '%s' (rd_kafka_producev)"
				"partition %d: '%d/%s' - adding MSG '%s' to failed for RETRY!\n",
				rd_kafka_topic_name(rkt), partition, msg_kafka_response,
				rd_kafka_err2str(msg_kafka_response), msg);
			CHKmalloc(fmsgEntry = failedmsg_entry_construct((char*) msg, strlen((char*)msg),
				rd_kafka_topic_name(rkt)));
			SLIST_INSERT_HEAD(&pData->failedmsg_head, fmsgEntry, entries);
		} else {
			LogError(0, RS_RET_KAFKA_PRODUCE_ERR,
				"omkafka: Failed to produce to topic '%s' (rd_kafka_producev)"
				"partition %d: %d/%s - MSG '%s'\n",
				rd_kafka_topic_name(rkt), partition, msg_kafka_response,
				rd_kafka_err2str(msg_kafka_response), msg);
		}
	}
#else

	DBGPRINTF("omkafka: rd_kafka_produce\n");
	/* Using old kafka produce API */
	msg_enqueue_status = rd_kafka_produce(rkt, partition, RD_KAFKA_MSG_F_COPY,
				  msg, strlen((char*)msg), pData->key,
				  pData->key == NULL ? 0 : strlen((char*)pData->key),
				  NULL);
	if(msg_enqueue_status == -1) {
		msg_kafka_response = rd_kafka_last_error();
		updateKafkaFailureCounts(msg_kafka_response);

		/* Put into kafka queue, again if configured! */
		if (pData->bResubmitOnFailure && b_do_resubmit) {
		   	DBGPRINTF("omkafka: Failed to produce to topic '%s' (rd_kafka_produce)"
				"partition %d: '%d/%s' - adding MSG '%s' to failed for RETRY!\n",
				rd_kafka_topic_name(rkt), partition, msg_kafka_response,
				rd_kafka_err2str(rd_kafka_errno2err(errno)), msg);
			CHKmalloc(fmsgEntry = failedmsg_entry_construct((char*) msg, strlen((char*)msg),
				rd_kafka_topic_name(rkt)));
			SLIST_INSERT_HEAD(&pData->failedmsg_head, fmsgEntry, entries);
		} else {
			LogError(0, RS_RET_KAFKA_PRODUCE_ERR,
				"omkafka: Failed to produce to topic '%s' (rd_kafka_produce) "
				"partition %d: %d/%s - MSG '%s'\n",
				rd_kafka_topic_name(rkt), partition, msg_kafka_response,
				rd_kafka_err2str(msg_kafka_response), msg);
		}
	}
#endif

	const int callbacksCalled = rd_kafka_poll(pData->rk, 0); /* call callbacks */
	DBGPRINTF("omkafka: writeKafka kafka outqueue length: %d, callbacks called %d\n",
			  rd_kafka_outq_len(pData->rk), callbacksCalled);

#if RD_KAFKA_VERSION >= 0x00090400
	if (msg_kafka_response != RD_KAFKA_RESP_ERR_NO_ERROR) {
#else
	if (msg_enqueue_status == -1) {
#endif
		STATSCOUNTER_INC(ctrKafkaFail, mutCtrKafkaFail);
		ABORT_FINALIZE(RS_RET_KAFKA_PRODUCE_ERR);
		/* ABORT_FINALIZE isn't absolutely necessary as of now,
		   because this is the last line anyway, but its useful to ensure
		   correctness in case we add more stuff below this line at some point*/
	}

finalize_it:
	if(topic_mut_locked) {
		pthread_rwlock_unlock(dynTopicLock);
	}
	DBGPRINTF("omkafka: writeKafka returned %d\n", iRet);
	if(iRet != RS_RET_OK) {
		iRet = RS_RET_SUSPENDED;
	}
	STATSCOUNTER_SETMAX_NOMUT(ctrQueueSize, (unsigned) rd_kafka_outq_len(pData->rk));
	STATSCOUNTER_INC(ctrTopicSubmit, mutCtrTopicSubmit);
	RETiRet;
}

static void
deliveryCallback(rd_kafka_t __attribute__((unused)) *rk,
	const rd_kafka_message_t *rkmessage,
	void *opaque)
{
	instanceData *const pData = (instanceData *) opaque;
	failedmsg_entry* fmsgEntry;
	DEFiRet;

	if (rkmessage->err) {
		updateKafkaFailureCounts(rkmessage->err);

		/* Put into kafka queue, again if configured! */
		if (pData->bResubmitOnFailure) {
			DBGPRINTF("omkafka: kafka delivery FAIL on Topic '%s', msg '%.*s', key '%.*s' -"
				" adding to FAILED MSGs for RETRY!\n",
				rd_kafka_topic_name(rkmessage->rkt),
				(int)(rkmessage->len-1), (char*)rkmessage->payload,
				(int)(rkmessage->key_len), (char*)rkmessage->key);
			CHKmalloc(fmsgEntry = failedmsg_entry_construct(rkmessage->payload, rkmessage->len,
				rd_kafka_topic_name(rkmessage->rkt)));
			SLIST_INSERT_HEAD(&pData->failedmsg_head, fmsgEntry, entries);
		} else {
			LogError(0, RS_RET_ERR,
				"omkafka: kafka delivery FAIL on Topic '%s', msg '%.*s', key '%.*s'\n",
				rd_kafka_topic_name(rkmessage->rkt),
				(int)(rkmessage->len-1), (char*)rkmessage->payload,
				(int)(rkmessage->key_len), (char*)rkmessage->key);
			writeDataError(pData, (char*) rkmessage->payload, rkmessage->len, rkmessage->err);
		}
		STATSCOUNTER_INC(ctrKafkaFail, mutCtrKafkaFail);
	} else {
		DBGPRINTF("omkafka: kafka delivery SUCCESS on msg '%.*s'\n", (int)(rkmessage->len-1),
			(char*)rkmessage->payload);
		STATSCOUNTER_INC(ctrKafkaAck, mutCtrKafkaAck);
	}
finalize_it:
	if(iRet != RS_RET_OK) {
		DBGPRINTF("omkafka: deliveryCallback returned failure %d\n", iRet);
	}
}

/**
 * This function looks for a json object that corresponds to the
 * passed name and returns it is found. Otherwise returns NULL.
 * It will be used for processing stats callback json object.
 */
static struct fjson_object *
get_object(struct fjson_object *fj_obj, const char * name) {
	struct fjson_object_iterator it = fjson_object_iter_begin(fj_obj);
	struct fjson_object_iterator itEnd = fjson_object_iter_end(fj_obj);
	while (!fjson_object_iter_equal (&it, &itEnd)) {
		const char * key = fjson_object_iter_peek_name (&it);
		struct fjson_object * val = fjson_object_iter_peek_value(&it);
		if(!strncmp(key, name, strlen(name))){
			return val;
		}
		fjson_object_iter_next (&it);
	}

	return NULL;
}

/**
 * This function performs a two level search in stats callback json
 * object. It iterates over broker objects and for each broker object
 * returns desired level2 value (such as avg/min/max) for specified
 * level1 window statistic (such as rtt/throttle/int_latency). Threshold
 * allows skipping values that are too small, so that they don't
 * impact on aggregate averaged value that is returned.
 */
static uint64
jsonExtractWindoStats(struct fjson_object * stats_object,
	const char * level1_obj_name, const char * level2_obj_name,
	unsigned long skip_threshold) {
	uint64 level2_val;
	uint64 agg_val = 0;
	uint64 ret_val = 0;
	int active_brokers = 0;

	struct fjson_object * brokers_obj = get_object(stats_object, "brokers");
	if (brokers_obj == NULL) {
		LogMsg(0, NO_ERRCODE, LOG_ERR, "jsonExtractWindowStat: failed to find brokers object");
		return ret_val;
	}

	/* iterate over borkers to get level1 window objects at level2 (min, max, avg, etc.) */
	struct fjson_object_iterator it = fjson_object_iter_begin(brokers_obj);
	struct fjson_object_iterator itEnd = fjson_object_iter_end(brokers_obj);
	while (!fjson_object_iter_equal (&it, &itEnd)) {
		struct fjson_object * val = fjson_object_iter_peek_value(&it);
		struct fjson_object * level1_obj = get_object(val, level1_obj_name);
		if(level1_obj == NULL)
			return ret_val;

		struct fjson_object * level2_obj = get_object(level1_obj, level2_obj_name);
		if(level2_obj == NULL)
			return ret_val;

		level2_val = fjson_object_get_int64(level2_obj);
		if (level2_val > skip_threshold) {
			agg_val += level2_val;
			active_brokers++;
		}
		fjson_object_iter_next (&it);
	}
	if(active_brokers > 0) {
		ret_val = agg_val/active_brokers;
	}

	return ret_val;
}

/**
 * librdkafka will call this function after every statistics.interval.ms
 * interval, which is specified in confParam. See the explanation at:
 * https://github.com/edenhill/librdkafka/wiki/Statistics
 *
 * Here we have extracted windows stats: rtt, throttle time, and internal
 * latency averages. These values will be logged as impstats messages.
 */
static int
statsCallback(rd_kafka_t __attribute__((unused)) *rk,
	char *json, size_t __attribute__((unused)) json_len,
	void __attribute__((unused)) *opaque) {
	char buf[2048];
	char handler_name[1024] = "unknown";
	int replyq = 0;
	int msg_cnt = 0;
	int msg_size = 0;
	uint64 msg_max = 0;
	uint64 msg_size_max = 0;

	struct fjson_object * stats_object = NULL;
	struct fjson_object * fj_obj = NULL;

	DBGPRINTF("omkafka: librdkafka stats callback: %s\n", json);

	/* prepare fjson object from stats callback for parsing */
	stats_object = fjson_tokener_parse(json);
	if (stats_object == NULL) {
		LogMsg(0, NO_ERRCODE, LOG_ERR, "statsCallback: fjson tokenizer failed:");
		return 0;
	}
	enum fjson_type type = fjson_object_get_type(stats_object);
	if (type != fjson_type_object) {
		LogMsg(0, NO_ERRCODE, LOG_ERR, "statsCallback: json is not of type object; can't process statsCB\n");
		return 0;
	}

	/* top level stats extraction through libfastjson based parsing */
	fj_obj = get_object(stats_object, "name");
	if (fj_obj != NULL)
		snprintf(handler_name, sizeof(handler_name), "%s", (char *)fjson_object_get_string(fj_obj));
	fj_obj = get_object(stats_object, "replyq");
	replyq = (fj_obj == NULL) ? 0 : fjson_object_get_int(fj_obj);
	fj_obj = get_object(stats_object, "msg_cnt");
	msg_cnt = (fj_obj == NULL) ? 0 : fjson_object_get_int(fj_obj);
	fj_obj = get_object(stats_object, "msg_size");
	msg_size = (fj_obj == NULL) ? 0 : fjson_object_get_int(fj_obj);
	fj_obj = get_object(stats_object, "msg_max");
	msg_max = (fj_obj == NULL) ? 0 : fjson_object_get_int64(fj_obj);
	fj_obj = get_object(stats_object, "msg_size_max");
	msg_size_max = (fj_obj == NULL) ? 0 : fjson_object_get_int64(fj_obj);

	/* window stats extraction to be picked up by impstats counters */
	rtt_avg_usec = jsonExtractWindoStats(stats_object, "rtt", "avg", 100);
	throttle_avg_msec = jsonExtractWindoStats(stats_object, "throttle", "avg", 0);
	int_latency_avg_usec = jsonExtractWindoStats(stats_object, "int_latency", "avg", 0);
	json_object_put (stats_object);

	/* emit a log line to get stats visibility per librdkafka client */
	snprintf(buf, sizeof(buf),
		"statscb_window_stats: handler_name=%s replyq=%d msg_cnt=%d msg_size=%d "
		"msg_max=%lld msg_size_max=%lld rtt_avg_usec=%lld throttle_avg_msec=%lld "
		"int_latency_avg_usec=%lld",
		handler_name, replyq, msg_cnt, msg_size, msg_max, msg_size_max,
		rtt_avg_usec, throttle_avg_msec, int_latency_avg_usec);
	LogMsg(0, NO_ERRCODE, LOG_INFO, "%s\n", buf);

	return 0;
}

static void
kafkaLogger(const rd_kafka_t __attribute__((unused)) *rk, int level,
	    const char *fac, const char *buf)
{
	DBGPRINTF("omkafka: kafka log message [%d,%s]: %s\n",
		  level, fac, buf);
}

/* should be called with write(rkLock) */
static void
do_rd_kafka_destroy(instanceData *const __restrict__ pData)
{
	if (pData->rk == NULL) {
		DBGPRINTF("omkafka: onDestroy can't close, handle wasn't open\n");
		goto done;
	}
	int queuedCount = rd_kafka_outq_len(pData->rk);
	DBGPRINTF("omkafka: onDestroy closing - items left in outqueue: %d\n", queuedCount);

	struct timespec tOut;
	timeoutComp(&tOut, pData->closeTimeout);

	while (timeoutVal(&tOut) > 0) {
		queuedCount = rd_kafka_outq_len(pData->rk);
		if (queuedCount > 0) {
			/* Flush all remaining kafka messages (rd_kafka_poll is called inside) */
			const int flushStatus = rd_kafka_flush(pData->rk, pData->closeTimeout);
			if (flushStatus == RD_KAFKA_RESP_ERR_NO_ERROR) {
				DBGPRINTF("omkafka: onDestroyflushed remaining '%d' messages "
					"to kafka topic '%s'\n", queuedCount,
					rd_kafka_topic_name(pData->pTopic));

				/* Trigger callbacks a last time before shutdown */
				const int callbacksCalled = rd_kafka_poll(pData->rk, 0); /* call callbacks */
				DBGPRINTF("omkafka: onDestroy kafka outqueue length: %d, "
					"callbacks called %d\n", rd_kafka_outq_len(pData->rk),
					callbacksCalled);
			} else /* TODO: Handle unsend messages here! */ {
				/* timeout = RD_KAFKA_RESP_ERR__TIMED_OUT */
				LogError(0, RS_RET_KAFKA_ERROR, "omkafka: onDestroy "
						"Failed to send remaing '%d' messages to "
						"topic '%s' on shutdown with error: '%s'",
						queuedCount,
						rd_kafka_topic_name(pData->pTopic),
						rd_kafka_err2str(flushStatus));
			}
		} else {
			break;
		}
	}
	if (queuedCount > 0) {
		LogMsg(0, RS_RET_ERR, LOG_WARNING,
				"omkafka: queue-drain for close timed-out took too long, "
				"items left in outqueue: %d -- this may indicate data loss",
				rd_kafka_outq_len(pData->rk));
	}
	if (pData->dynaTopic) {
		dynaTopicFreeCacheEntries(pData);
	} else {
		closeTopic(pData);
	}

	/* Final destroy of kafka!*/
	rd_kafka_destroy(pData->rk);

# if RD_KAFKA_VERSION < 0x00090001
	/* Wait for kafka being destroyed in old API */
	if (rd_kafka_wait_destroyed(10000) < 0)	{
		LogError(0, RS_RET_ERR, "omkafka: rd_kafka_destroy did not finish after grace timeout (10s)!");
	} else {
		DBGPRINTF("omkafka: rd_kafka_destroy successfully finished\n");
	}
# endif

	pData->rk = NULL;
done:	return;
}

/* should be called with write(rkLock) */
static void
closeKafka(instanceData *const __restrict__ pData)
{
	if(pData->bIsOpen) {
		do_rd_kafka_destroy(pData);
		pData->bIsOpen = 0;
	}
}

static void
errorCallback(rd_kafka_t __attribute__((unused)) *rk,
	int __attribute__((unused)) err,
	const char *reason,
	void __attribute__((unused)) *opaque)
{
	/* Get InstanceData pointer */
	instanceData *const pData = (instanceData *) opaque;

	/* count kafka transport errors that cause action suspension */
	if (err == RD_KAFKA_RESP_ERR__MSG_TIMED_OUT) {
		STATSCOUNTER_INC(ctrKafkaRespTimedOut, mutCtrKafkaRespTimedOut);
	} else if (err == RD_KAFKA_RESP_ERR__TRANSPORT) {
		STATSCOUNTER_INC(ctrKafkaRespTransport, mutCtrKafkaRespTransport);
	} else if (err == RD_KAFKA_RESP_ERR__ALL_BROKERS_DOWN) {
		STATSCOUNTER_INC(ctrKafkaRespBrokerDown, mutCtrKafkaRespBrokerDown);
	} else if (err == RD_KAFKA_RESP_ERR__AUTHENTICATION) {
		STATSCOUNTER_INC(ctrKafkaRespAuth, mutCtrKafkaRespAuth);
	} else {
		STATSCOUNTER_INC(ctrKafkaRespOther, mutCtrKafkaRespOther);
	}

	/* Handle common transport error codes*/
	if (err == RD_KAFKA_RESP_ERR__MSG_TIMED_OUT ||
		err == RD_KAFKA_RESP_ERR__TRANSPORT ||
		err == RD_KAFKA_RESP_ERR__ALL_BROKERS_DOWN ||
		err == RD_KAFKA_RESP_ERR__AUTHENTICATION) {
		/* Broker transport error, we need to disable the action for now!*/
		pData->bIsSuspended = 1;
		LogMsg(0, RS_RET_KAFKA_ERROR, LOG_WARNING,
			"omkafka: action will suspended due to kafka error %d: %s",
			err, rd_kafka_err2str(err));
	} else {
		LogError(0, RS_RET_KAFKA_ERROR, "omkafka: kafka error message: %d,'%s','%s'",
			err, rd_kafka_err2str(err), reason);
	}
}



#if 0 /* the stock librdkafka version in Ubuntu 14.04 LTS does NOT support metadata :-( */
/* Note: this is a skeleton, with some code missing--> add it when it is actually implemented. */
static int
getConfiguredPartitions()
{
	struct rd_kafka_metadata *pMetadata;
	if(rd_kafka_metadata(pData->rk, 0, rkt, &pMetadata, 8)
		== RD_KAFKA_RESP_ERR_NO_ERROR) {
		dbgprintf("omkafka: topic '%s' has %d partitions\n",
			  pData->topic, pMetadata->topics[0]->partition_cnt);
		rd_kafka_metadata_destroy(pMetadata);
	} else {
		dbgprintf("omkafka: error reading metadata\n");
		// TODO: handle this gracefull **when** we actually need
		// the metadata -- or remove completely. 2014-12-12 rgerhards
	}
}
#endif

/* should be called with write(rkLock) */
static rsRetVal
openKafka(instanceData *const __restrict__ pData)
{
	char errstr[MAX_ERRMSG];
	int nBrokers = 0;
	DEFiRet;

	if(pData->bIsOpen)
		FINALIZE;

	pData->pTopic = NULL;

	/* main conf */
	rd_kafka_conf_t *const conf = rd_kafka_conf_new();
	if(conf == NULL) {
		LogError(0, RS_RET_KAFKA_ERROR, "omkafka: error creating kafka conf obj: %s\n",
			rd_kafka_err2str(rd_kafka_last_error()));
		ABORT_FINALIZE(RS_RET_KAFKA_ERROR);
	}

#ifdef DEBUG
	/* enable kafka debug output */
	if(rd_kafka_conf_set(conf, "debug", RD_KAFKA_DEBUG_CONTEXTS,
		errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
		LogError(0, RS_RET_KAFKA_ERROR, "omkafka: error setting kafka debug option: %s\n", errstr);
		/* DO NOT ABORT IN THIS CASE! */
	}
#endif

	for(int i = 0 ; i < pData->nConfParams ; ++i) {
		DBGPRINTF("omkafka: setting custom configuration parameter: %s:%s\n",
			pData->confParams[i].name,
			pData->confParams[i].val);
		if(rd_kafka_conf_set(conf, pData->confParams[i].name,
			pData->confParams[i].val, errstr, sizeof(errstr))
	 	   != RD_KAFKA_CONF_OK) {
			if(pData->bReportErrs) {
				LogError(0, RS_RET_PARAM_ERROR, "error setting custom configuration "
					"parameter '%s=%s': %s",
					pData->confParams[i].name,
					pData->confParams[i].val, errstr);
			} else {
				DBGPRINTF("omkafka: error setting custom configuration parameter '%s=%s': %s",
					pData->confParams[i].name,
					pData->confParams[i].val, errstr);
			}
			ABORT_FINALIZE(RS_RET_PARAM_ERROR);
		}
	}
	rd_kafka_conf_set_opaque(conf, (void *) pData);
	rd_kafka_conf_set_dr_msg_cb(conf, deliveryCallback);
	rd_kafka_conf_set_error_cb(conf, errorCallback);
	rd_kafka_conf_set_stats_cb(conf, statsCallback);
#	if RD_KAFKA_VERSION >= 0x00090001
	rd_kafka_conf_set_log_cb(conf, kafkaLogger);
#	endif

	char kafkaErrMsg[1024];
	pData->rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf, kafkaErrMsg, sizeof(kafkaErrMsg));
	if(pData->rk == NULL) {
		LogError(0, RS_RET_KAFKA_ERROR,
			"omkafka: error creating kafka handle: %s\n", kafkaErrMsg);
		ABORT_FINALIZE(RS_RET_KAFKA_ERROR);
	}

#	if RD_KAFKA_VERSION < 0x00090001
	rd_kafka_conf_set_log_cb(pData->rk, kafkaLogger);
#	endif
	DBGPRINTF("omkafka setting brokers: '%s'n", pData->brokers);
	if((nBrokers = rd_kafka_brokers_add(pData->rk, (char*)pData->brokers)) == 0) {
		LogError(0, RS_RET_KAFKA_NO_VALID_BROKERS,
			"omkafka: no valid brokers specified: %s\n", pData->brokers);
		ABORT_FINALIZE(RS_RET_KAFKA_NO_VALID_BROKERS);
	}

	pData->bIsOpen = 1;
finalize_it:
	if(iRet == RS_RET_OK) {
		pData->bReportErrs = 1;
	} else {
		pData->bReportErrs = 0;
		if(pData->rk != NULL) {
			do_rd_kafka_destroy(pData);
		}
	}
	RETiRet;
}

static rsRetVal
setupKafkaHandle(instanceData *const __restrict__ pData, int recreate)
{
	DEFiRet;
	pthread_rwlock_wrlock(&pData->rkLock);
	if (recreate) {
		closeKafka(pData);
	}
	CHKiRet(openKafka(pData));
	if (! pData->dynaTopic) {
		if( pData->pTopic == NULL)
			CHKiRet(prepareTopic(pData, pData->topic));
	}
finalize_it:
	if (iRet != RS_RET_OK) {
		if (pData->rk != NULL) {
			closeKafka(pData);
		}

		/* Parameter Error's cannot be resumed, so we need to disable the action */
		if (iRet == RS_RET_PARAM_ERROR) {
			iRet = RS_RET_DISABLE_ACTION;
			LogError(0, iRet, "omkafka: action will be disabled due invalid "
				"kafka configuration parameters\n");
		}

	}
	pthread_rwlock_unlock(&pData->rkLock);
	RETiRet;
}

static rsRetVal
checkFailedMessages(instanceData *const __restrict__ pData)
{
	failedmsg_entry* fmsgEntry;
	DEFiRet;

	/* Loop through failed messages, reprocess them first! */
	while (!SLIST_EMPTY(&pData->failedmsg_head)) {
		fmsgEntry = SLIST_FIRST(&pData->failedmsg_head);
		assert(fmsgEntry != NULL);
		/* Put back into kafka! */
		iRet = writeKafka(pData, (uchar*) fmsgEntry->payload, NULL, fmsgEntry->topicname, NO_RESUBMIT);
		if(iRet != RS_RET_OK) {
			LogMsg(0, RS_RET_SUSPENDED, LOG_WARNING,
				"omkafka: failed to deliver failed msg '%.*s' with status %d. "
				"- suspending AGAIN!",
				(int)(strlen((char*)fmsgEntry->payload)-1),
				(char*)fmsgEntry->payload, iRet);
			ABORT_FINALIZE(RS_RET_SUSPENDED);
		} else {
			DBGPRINTF("omkafka: successfully delivered failed msg '%.*s'.\n",
				(int)(strlen((char*)fmsgEntry->payload)-1),
				(char*)fmsgEntry->payload);
			/* Note: we can use SLIST even though it is o(n), because the element
			 * in question is always either the root or the next element and
			 * SLIST_REMOVE iterates only until the element to be deleted is found.
			 * We cannot use SLIST_REMOVE_HEAD() as new elements may have been
			 * added in the delivery callback!
			 * TODO: sounds like bad logic -- why do we add and remove, just simply
			 * keep it in queue?
			 */
			SLIST_REMOVE(&pData->failedmsg_head, fmsgEntry, s_failedmsg_entry, entries);
			failedmsg_entry_destruct(fmsgEntry);
		}
	}

finalize_it:
	RETiRet;
}

/* This function persists failed messages into a data file, so they can
 * be resend on next startup.
 * alorbach, 2017-06-02
 */
static rsRetVal ATTR_NONNULL(1)
persistFailedMsgs(instanceData *const __restrict__ pData)
{
	DEFiRet;
	int fdMsgFile = -1;
	ssize_t nwritten;

	if(SLIST_EMPTY(&pData->failedmsg_head)) {
		DBGPRINTF("omkafka: persistFailedMsgs: We do not need to persist failed messages.\n");
		FINALIZE;
	}

	fdMsgFile = open((char*)pData->failedMsgFile,
				O_WRONLY|O_CREAT|O_APPEND|O_LARGEFILE|O_CLOEXEC,
				S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	if(fdMsgFile == -1) {
		LogError(errno, RS_RET_ERR, "omkafka: persistFailedMsgs error opening failed msg file %s",
			pData->failedMsgFile);
		ABORT_FINALIZE(RS_RET_ERR);
	}

	while (!SLIST_EMPTY(&pData->failedmsg_head)) {
		failedmsg_entry* fmsgEntry = SLIST_FIRST(&pData->failedmsg_head);
		assert(fmsgEntry != NULL);
		nwritten = write(fdMsgFile, fmsgEntry->topicname, ustrlen(fmsgEntry->topicname) );
		if(nwritten != -1)
			nwritten = write(fdMsgFile, "\t", 1);
		if(nwritten != -1)
			nwritten = write(fdMsgFile, fmsgEntry->payload, ustrlen(fmsgEntry->payload) );
		if(nwritten == -1) {
			LogError(errno, RS_RET_ERR, "omkafka: persistFailedMsgs error writing failed msg file");
			ABORT_FINALIZE(RS_RET_ERR);
		} else {
			DBGPRINTF("omkafka: persistFailedMsgs successfully written loaded msg '%.*s' for "
				"topic '%s'\n", (int)(strlen((char*)fmsgEntry->payload)-1),
				fmsgEntry->payload, fmsgEntry->topicname);
		}
		SLIST_REMOVE_HEAD(&pData->failedmsg_head, entries);
		failedmsg_entry_destruct(fmsgEntry);
	}

finalize_it:
	if(fdMsgFile != -1) {
		close(fdMsgFile);
	}
	if(iRet != RS_RET_OK) {
		LogError(0, iRet, "omkafka: could not persist failed messages "
			"file %s - failed messages will be lost.",
			(char*)pData->failedMsgFile);
	}
	RETiRet;
}

/* This function loads failed messages from a data file, so they can
 * be resend after action startup.
 * alorbach, 2017-06-06
 */
static rsRetVal
loadFailedMsgs(instanceData *const __restrict__ pData)
{
	DEFiRet;
	struct stat stat_buf;
	failedmsg_entry* fmsgEntry;
	strm_t *pstrmFMSG = NULL;
	cstr_t *pCStr = NULL;
	uchar *puStr;
	char *pStrTabPos;

	assert(pData->failedMsgFile != NULL);

	/* check if the file exists */
	if(stat((char*) pData->failedMsgFile, &stat_buf) == -1) {
		if(errno == ENOENT) {
			DBGPRINTF("omkafka: loadFailedMsgs failed messages file %s wasn't found, "
				"continue startup\n", pData->failedMsgFile);
			ABORT_FINALIZE(RS_RET_FILE_NOT_FOUND);
		} else {
			LogError(errno, RS_RET_IO_ERROR,
				"omkafka: loadFailedMsgs could not open failed messages file %s",
				pData->failedMsgFile);
			ABORT_FINALIZE(RS_RET_IO_ERROR);
		}
	} else {
		DBGPRINTF("omkafka: loadFailedMsgs found failed message file %s.\n",
			pData->failedMsgFile);
	}

	/* File exists, we can load and process it */
	CHKiRet(strm.Construct(&pstrmFMSG));
	CHKiRet(strm.SettOperationsMode(pstrmFMSG, STREAMMODE_READ));
	CHKiRet(strm.SetsType(pstrmFMSG, STREAMTYPE_FILE_SINGLE));
	CHKiRet(strm.SetFName(pstrmFMSG, pData->failedMsgFile, ustrlen(pData->failedMsgFile)));
	CHKiRet(strm.ConstructFinalize(pstrmFMSG));

	while(strm.ReadLine(pstrmFMSG, &pCStr, 0, 0, 0, NULL) == RS_RET_OK) {
		if(rsCStrLen(pCStr) == 0) {
			/* we do not process empty lines */
			DBGPRINTF("omkafka: loadFailedMsgs msg was empty!");
		} else {
			puStr = rsCStrGetSzStrNoNULL(pCStr);
			pStrTabPos = index((char*)puStr, '\t');
			if (pStrTabPos != NULL) {
				DBGPRINTF("omkafka: loadFailedMsgs successfully loaded msg '%s' for "
					"topic '%.*s':%d\n",
					pStrTabPos+1, (int)(pStrTabPos-(char*)puStr), (char*)puStr,
					(int)(pStrTabPos-(char*)puStr));
				*pStrTabPos = '\0'; /* split string into two */
				CHKmalloc(fmsgEntry = failedmsg_entry_construct(pStrTabPos+1,
					strlen(pStrTabPos+1), (char*)puStr));
				SLIST_INSERT_HEAD(&pData->failedmsg_head, fmsgEntry, entries);
			} else {
				LogError(0, RS_RET_ERR, "omkafka: loadFailedMsgs droping invalid msg found: %s",
					(char*)rsCStrGetSzStrNoNULL(pCStr));
			}
		}

		rsCStrDestruct(&pCStr); /* discard string (must be done by us!) */
	}
finalize_it:
	if(pstrmFMSG != NULL) {
		strm.Destruct(&pstrmFMSG);
	}

	if(iRet != RS_RET_OK) {
		/* We ignore FILE NOT FOUND here */
		if (iRet != RS_RET_FILE_NOT_FOUND) {
			LogError(0, iRet, "omkafka: could not load failed messages "
			"from file %s error %d - failed messages will not be resend.",
			(char*)pData->failedMsgFile, iRet);
		}
	} else {
		DBGPRINTF("omkafka: loadFailedMsgs unlinking '%s'\n", (char*)pData->failedMsgFile);
		/* Delete file if still exists! */
		const int r = unlink((char*)pData->failedMsgFile);
		if(r != 0 && r != ENOENT) {
			LogError(errno, RS_RET_ERR, "omkafka: loadFailedMsgs failed to remove "
				"file \"%s\"", (char*)pData->failedMsgFile);
		}
	}

	RETiRet;
}

BEGINdoHUP
CODESTARTdoHUP
	pthread_mutex_lock(&pData->mutErrFile);
	if(pData->fdErrFile != -1) {
		close(pData->fdErrFile);
		pData->fdErrFile = -1;
	}
	pthread_mutex_unlock(&pData->mutErrFile);
	if (pData->bReopenOnHup) {
		CHKiRet(setupKafkaHandle(pData, 1));
	}
finalize_it:
ENDdoHUP

BEGINcreateInstance
CODESTARTcreateInstance
	pData->currPartition = 0;
	pData->bIsOpen = 0;
	pData->bIsSuspended = 0;
	pData->fdErrFile = -1;
	pData->pTopic = NULL;
	pData->bReportErrs = 1;
	pData->bReopenOnHup = 1;
	pData->bResubmitOnFailure = 0;
	pData->bKeepFailedMessages = 0;
	pData->failedMsgFile = NULL;
	SLIST_INIT(&pData->failedmsg_head);
	CHKiRet(pthread_mutex_init(&pData->mut_doAction, NULL));
	CHKiRet(pthread_mutex_init(&pData->mutErrFile, NULL));
	CHKiRet(pthread_rwlock_init(&pData->rkLock, NULL));
	CHKiRet(pthread_mutex_init(&pData->mutDynCache, NULL));
	INIT_ATOMIC_HELPER_MUT(pData->mutCurrPartition);
finalize_it:
ENDcreateInstance


BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
ENDcreateWrkrInstance


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
ENDisCompatibleWithFeature


BEGINfreeInstance
CODESTARTfreeInstance
	/* Helpers for Failed Msg List */
	failedmsg_entry* fmsgEntry1;
	failedmsg_entry* fmsgEntry2;
	if(pData->fdErrFile != -1)
		close(pData->fdErrFile);
	/* Closing Kafka first! */
	pthread_rwlock_wrlock(&pData->rkLock);
	closeKafka(pData);
	if(pData->dynaTopic && pData->dynCache != NULL) {
		d_free(pData->dynCache);
		pData->dynCache = NULL;
	}
	/* Persist failed messages */
	if (pData->bResubmitOnFailure && pData->bKeepFailedMessages && pData->failedMsgFile != NULL) {
		persistFailedMsgs(pData);
	}
	pthread_rwlock_unlock(&pData->rkLock);

	/* Delete Linked List for failed msgs */
	fmsgEntry1 = SLIST_FIRST(&pData->failedmsg_head);
	while (fmsgEntry1 != NULL)	{
		fmsgEntry2 = SLIST_NEXT(fmsgEntry1, entries);
		failedmsg_entry_destruct(fmsgEntry1);
		fmsgEntry1 = fmsgEntry2;
	}
	SLIST_INIT(&pData->failedmsg_head);
	/* Free other mem */
	free(pData->errorFile);
	free(pData->failedMsgFile);
	free(pData->topic);
	free(pData->brokers);
	free(pData->tplName);
	for(int i = 0 ; i < pData->nConfParams ; ++i) {
		free((void*) pData->confParams[i].name);
		free((void*) pData->confParams[i].val);
	}
	free(pData->confParams);
	for(int i = 0 ; i < pData->nTopicConfParams ; ++i) {
		free((void*) pData->topicConfParams[i].name);
		free((void*) pData->topicConfParams[i].val);
	}
	free(pData->topicConfParams);
	DESTROY_ATOMIC_HELPER_MUT(pData->mutCurrPartition);
	pthread_rwlock_destroy(&pData->rkLock);
	pthread_mutex_destroy(&pData->mut_doAction);
	pthread_mutex_destroy(&pData->mutErrFile);
	pthread_mutex_destroy(&pData->mutDynCache);
ENDfreeInstance

BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance


BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
ENDdbgPrintInstInfo


BEGINtryResume
	int iKafkaRet;
	const struct rd_kafka_metadata *metadata;
CODESTARTtryResume
	pthread_mutex_lock(&pWrkrData->pData->mut_doAction); /* see doAction header comment! */
	CHKiRet(setupKafkaHandle(pWrkrData->pData, 0));

	if ((iKafkaRet = rd_kafka_metadata(pWrkrData->pData->rk, 0, NULL, &metadata, 1000))
			!= RD_KAFKA_RESP_ERR_NO_ERROR) {
		DBGPRINTF("omkafka: tryResume failed, brokers down %d,%s\n", iKafkaRet, rd_kafka_err2str(iKafkaRet));
		ABORT_FINALIZE(RS_RET_SUSPENDED);
	} else {
		DBGPRINTF("omkafka: tryResume success, %d brokers UP\n", metadata->broker_cnt);
		/* Reset suspended state */
		pWrkrData->pData->bIsSuspended = 0;
		/* free mem*/
		rd_kafka_metadata_destroy(metadata);
	}

finalize_it:
	pthread_mutex_unlock(&pWrkrData->pData->mut_doAction); /* see doAction header comment! */
	DBGPRINTF("omkafka: tryResume returned %d\n", iRet);
ENDtryResume


/* IMPORTANT NOTE on multithreading:
 * librdkafka creates background threads itself. So omkafka basically needs to move
 * memory buffers over to librdkafka, which then does the heavy hauling. As such, we
 * think that it is best to run max one wrkr instance of omkafka -- otherwise we just
 * get additional locking (contention) overhead without any real gain. As such,
 * we use a global mutex for doAction which ensures only one worker can be active
 * at any given time. That mutex is also used to guard utility functions (like
 * tryResume) which may also be accessed by multiple workers in parallel.
 * Note: shall this method be changed, the kafka connection/suspension handling needs
 * to be refactored. The current code assumes that all workers share state information
 * including librdkafka handles.
 */
BEGINdoAction
CODESTARTdoAction
	failedmsg_entry* fmsgEntry;
	instanceData *const pData = pWrkrData->pData;
	int need_unlock = 0;

	pthread_mutex_lock(&pData->mut_doAction);
	if (! pData->bIsOpen)
		CHKiRet(setupKafkaHandle(pData, 0));

	/* Lock here to prevent msg loss */
	pthread_rwlock_rdlock(&pData->rkLock);
	need_unlock = 1;

	/* We need to trigger callbacks first in order to suspend the Action properly on failure */
	const int callbacksCalled = rd_kafka_poll(pData->rk, 0); /* call callbacks */
	DBGPRINTF("omkafka: doAction kafka outqueue length: %d, callbacks called %d\n",
		rd_kafka_outq_len(pData->rk), callbacksCalled);

	/* Reprocess failed messages! */
	if (pData->bResubmitOnFailure) {
		iRet = checkFailedMessages(pData);
		if(iRet != RS_RET_OK) {
			DBGPRINTF("omkafka: doAction failed to submit FAILED messages with status %d\n", iRet);

			if (pData->bResubmitOnFailure) {
				DBGPRINTF("omkafka: also adding MSG '%.*s' for topic '%s' to failed for RETRY!\n",
					(int)(strlen((char*)ppString[0])-1), ppString[0],
					pData->dynaTopic ? ppString[2] : pData->topic);
				CHKmalloc(fmsgEntry = failedmsg_entry_construct((char*)ppString[0],
					strlen((char*)ppString[0]),
					(char*) (pData->dynaTopic ? ppString[2] : pData->topic)));
				SLIST_INSERT_HEAD(&pData->failedmsg_head, fmsgEntry, entries);
			}
			ABORT_FINALIZE(iRet);
		}
	}

	/* support dynamic topic */
	iRet = writeKafka(pData, ppString[0], ppString[1], pData->dynaTopic ? ppString[2] : pData->topic, RESUBMIT);

finalize_it:
	if(need_unlock) {
		pthread_rwlock_unlock(&pData->rkLock);
	}

	if(iRet != RS_RET_OK) {
		DBGPRINTF("omkafka: doAction failed with status %d\n", iRet);
	}

	/* Suspend Action if broker problems were reported in error callback */
	if (pData->bIsSuspended) {
		DBGPRINTF("omkafka: doAction broker failure detected, suspending action\n");
		iRet = RS_RET_SUSPENDED;
	}
	pthread_mutex_unlock(&pData->mut_doAction); /* must be after last pData access! */
ENDdoAction


static void
setInstParamDefaults(instanceData *pData)
{
	pData->topic = NULL;
	pData->dynaTopic = 0;
	pData->iDynaTopicCacheSize = 50;
	pData->brokers = NULL;
	pData->autoPartition = 0;
	pData->fixedPartition = NO_FIXED_PARTITION;
	pData->nPartitions = 1;
	pData->nConfParams = 0;
	pData->confParams = NULL;
	pData->nTopicConfParams = 0;
	pData->topicConfParams = NULL;
	pData->errorFile = NULL;
	pData->failedMsgFile = NULL;
	pData->key = NULL;
	pData->closeTimeout = 2000;
}

static rsRetVal
processKafkaParam(char *const param,
	const char **const name,
	const char **const paramval)
{
	DEFiRet;
	char *val = strstr(param, "=");
	if(val == NULL) {
		LogError(0, RS_RET_PARAM_ERROR, "missing equal sign in "
				"parameter '%s'", param);
		ABORT_FINALIZE(RS_RET_PARAM_ERROR);
	}
	*val = '\0'; /* terminates name */
	++val; /* now points to begin of value */
	CHKmalloc(*name = strdup(param));
	CHKmalloc(*paramval = strdup(val));
finalize_it:
	RETiRet;
}

BEGINnewActInst
	struct cnfparamvals *pvals;
	int i;
	int iNumTpls;
CODESTARTnewActInst
	if((pvals = nvlstGetParams(lst, &actpblk, NULL)) == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	CHKiRet(createInstance(&pData));
	setInstParamDefaults(pData);

	for(i = 0 ; i < actpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(actpblk.descr[i].name, "topic")) {
			pData->topic = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "dynatopic")) {
			pData->dynaTopic = pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "dynatopic.cachesize")) {
			pData->iDynaTopicCacheSize = pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "closetimeout")) {
			pData->closeTimeout = pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "partitions.auto")) {
			pData->autoPartition = pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "partitions.number")) {
			pData->nPartitions = pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "partitions.usefixed")) {
			pData->fixedPartition = pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "broker")) {
			es_str_t *es = es_newStr(128);
			int bNeedComma = 0;
			for(int j = 0 ; j <  pvals[i].val.d.ar->nmemb ; ++j) {
				if(bNeedComma)
					es_addChar(&es, ',');
				es_addStr(&es, pvals[i].val.d.ar->arr[j]);
				bNeedComma = 1;
			}
			pData->brokers = es_str2cstr(es, NULL);
			es_deleteStr(es);
		} else if(!strcmp(actpblk.descr[i].name, "confparam")) {
			pData->nConfParams = pvals[i].val.d.ar->nmemb;
			CHKmalloc(pData->confParams = malloc(sizeof(struct kafka_params) *
			                                      pvals[i].val.d.ar->nmemb ));
			for(int j = 0 ; j <  pvals[i].val.d.ar->nmemb ; ++j) {
				char *cstr = es_str2cstr(pvals[i].val.d.ar->arr[j], NULL);
				CHKiRet(processKafkaParam(cstr, &pData->confParams[j].name,
					&pData->confParams[j].val));
				free(cstr);
			}
		} else if(!strcmp(actpblk.descr[i].name, "topicconfparam")) {
			pData->nTopicConfParams = pvals[i].val.d.ar->nmemb;
			CHKmalloc(pData->topicConfParams = malloc(sizeof(struct kafka_params) *
			                                      pvals[i].val.d.ar->nmemb ));
			for(int j = 0 ; j <  pvals[i].val.d.ar->nmemb ; ++j) {
				char *cstr = es_str2cstr(pvals[i].val.d.ar->arr[j], NULL);
				CHKiRet(processKafkaParam(cstr, &pData->topicConfParams[j].name,
					&pData->topicConfParams[j].val));
				free(cstr);
			}
		} else if(!strcmp(actpblk.descr[i].name, "errorfile")) {
			pData->errorFile = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "key")) {
			pData->key = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "template")) {
			pData->tplName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(actpblk.descr[i].name, "reopenonhup")) {
			pData->bReopenOnHup = pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "resubmitonfailure")) {
			pData->bResubmitOnFailure = pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "keepfailedmessages")) {
			pData->bKeepFailedMessages = pvals[i].val.d.n;
		} else if(!strcmp(actpblk.descr[i].name, "failedmsgfile")) {
			pData->failedMsgFile = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else {
			LogError(0, RS_RET_INTERNAL_ERROR,
				"omkafka: program error, non-handled param '%s'\n", actpblk.descr[i].name);
		}
	}

	if(pData->brokers == NULL) {
		CHKmalloc(pData->brokers = strdup("localhost:9092"));
		LogMsg(0, NO_ERRCODE, LOG_INFO, "imkafka: \"broker\" parameter not specified "
			"using default of localhost:9092 -- this may not be what you want!");
	}

	if(pData->dynaTopic && pData->topic == NULL) {
		LogError(0, RS_RET_CONFIG_ERROR,
			"omkafka: requested dynamic topic, but no "
			"name for topic template given - action definition invalid");
		ABORT_FINALIZE(RS_RET_CONFIG_ERROR);
	}

	iNumTpls = 2;
	if(pData->dynaTopic) ++iNumTpls;
	CODE_STD_STRING_REQUESTnewActInst(iNumTpls);
	CHKiRet(OMSRsetEntry(*ppOMSR, 0, (uchar*)strdup((pData->tplName == NULL) ?
						"RSYSLOG_FileFormat" : (char*)pData->tplName),
						OMSR_NO_RQD_TPL_OPTS));

	CHKiRet(OMSRsetEntry(*ppOMSR, 1, (uchar*)strdup(" KAFKA_TimeStamp"),
						OMSR_NO_RQD_TPL_OPTS));
	if(pData->dynaTopic) {
		CHKiRet(OMSRsetEntry(*ppOMSR, 2, ustrdup(pData->topic), OMSR_NO_RQD_TPL_OPTS));
		CHKmalloc(pData->dynCache = (dynaTopicCacheEntry**)
			calloc(pData->iDynaTopicCacheSize, sizeof(dynaTopicCacheEntry*)));
		pData->iCurrElt = -1;
	}

	pthread_mutex_lock(&closeTimeoutMut);
	if (closeTimeout < pData->closeTimeout) {
		closeTimeout = pData->closeTimeout;
	}
	pthread_mutex_unlock(&closeTimeoutMut);

	/* Load failed messages here (If enabled), do NOT check for IRET!*/
	if (pData->bKeepFailedMessages && pData->failedMsgFile != NULL) {
		loadFailedMsgs(pData);
	}

CODE_STD_FINALIZERnewActInst
	cnfparamvalsDestruct(pvals, &actpblk);
ENDnewActInst


BEGINmodExit
CODESTARTmodExit
	statsobj.Destruct(&kafkaStats);
	CHKiRet(objRelease(statsobj, CORE_COMPONENT));
	DESTROY_ATOMIC_HELPER_MUT(mutClock);

	pthread_mutex_lock(&closeTimeoutMut);
	int timeout = closeTimeout;
	pthread_mutex_unlock(&closeTimeoutMut);
	pthread_mutex_destroy(&closeTimeoutMut);
	if (rd_kafka_wait_destroyed(timeout) != 0) {
		LogMsg(0, RS_RET_OK, LOG_WARNING,
			"omkafka: could not terminate librdkafka gracefully, "
			"%d threads still remain.\n", rd_kafka_thread_cnt());
	}
finalize_it:
ENDmodExit


NO_LEGACY_CONF_parseSelectorAct
BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_OMOD_QUERIES
CODEqueryEtryPt_STD_OMOD8_QUERIES
CODEqueryEtryPt_STD_CONF2_CNFNAME_QUERIES
CODEqueryEtryPt_STD_CONF2_OMOD_QUERIES
CODEqueryEtryPt_doHUP
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	uchar *pTmp;
INITLegCnfVars
	*ipIFVersProvided = CURR_MOD_IF_VERSION;
CODEmodInit_QueryRegCFSLineHdlr
	CHKiRet(objUse(datetime, CORE_COMPONENT));
	CHKiRet(objUse(strm, CORE_COMPONENT));
	CHKiRet(objUse(statsobj, CORE_COMPONENT));

	INIT_ATOMIC_HELPER_MUT(mutClock);

	DBGPRINTF("omkafka %s using librdkafka version %s, 0x%x\n",
	          VERSION, rd_kafka_version_str(), rd_kafka_version());
	CHKiRet(statsobj.Construct(&kafkaStats));
	CHKiRet(statsobj.SetName(kafkaStats, (uchar *)"omkafka"));
	CHKiRet(statsobj.SetOrigin(kafkaStats, (uchar*)"omkafka"));
	STATSCOUNTER_INIT(ctrTopicSubmit, mutCtrTopicSubmit);
	CHKiRet(statsobj.AddCounter(kafkaStats, (uchar *)"submitted",
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &ctrTopicSubmit));
	STATSCOUNTER_INIT(ctrQueueSize, mutCtrQueueSize);
	CHKiRet(statsobj.AddCounter(kafkaStats, (uchar *)"maxoutqsize",
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &ctrQueueSize));
	STATSCOUNTER_INIT(ctrKafkaFail, mutCtrKafkaFail);
	CHKiRet(statsobj.AddCounter(kafkaStats, (uchar *)"failures",
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &ctrKafkaFail));
	STATSCOUNTER_INIT(ctrCacheSkip, mutCtrCacheSkip);
	CHKiRet(statsobj.AddCounter(kafkaStats, (uchar *)"topicdynacache.skipped",
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &ctrCacheSkip));
	STATSCOUNTER_INIT(ctrCacheMiss, mutCtrCacheMiss);
	CHKiRet(statsobj.AddCounter(kafkaStats, (uchar *)"topicdynacache.miss",
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &ctrCacheMiss));
	STATSCOUNTER_INIT(ctrCacheEvict, mutCtrCacheEvict);
	CHKiRet(statsobj.AddCounter(kafkaStats, (uchar *)"topicdynacache.evicted",
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &ctrCacheEvict));
	STATSCOUNTER_INIT(ctrKafkaAck, mutCtrKafkaAck);
	CHKiRet(statsobj.AddCounter(kafkaStats, (uchar *)"acked",
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &ctrKafkaAck));
	STATSCOUNTER_INIT(ctrKafkaMsgTooLarge, mutCtrKafkaMsgTooLarge);
	CHKiRet(statsobj.AddCounter(kafkaStats, (uchar *)"failures_msg_too_large",
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &ctrKafkaMsgTooLarge));
	STATSCOUNTER_INIT(ctrKafkaUnknownTopic, mutCtrKafkaUnknownTopic);
	CHKiRet(statsobj.AddCounter(kafkaStats, (uchar *)"failures_unknown_topic",
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &ctrKafkaUnknownTopic));
	STATSCOUNTER_INIT(ctrKafkaQueueFull, mutCtrKafkaQueueFull);
	CHKiRet(statsobj.AddCounter(kafkaStats, (uchar *)"failures_queue_full",
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &ctrKafkaQueueFull));
	STATSCOUNTER_INIT(ctrKafkaUnknownPartition, mutCtrKafkaUnknownPartition);
	CHKiRet(statsobj.AddCounter(kafkaStats, (uchar *)"failures_unknown_partition",
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &ctrKafkaUnknownPartition));
	STATSCOUNTER_INIT(ctrKafkaOtherErrors, mutCtrKafkaOtherErrors);
	CHKiRet(statsobj.AddCounter(kafkaStats, (uchar *)"failures_other",
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &ctrKafkaOtherErrors));
	STATSCOUNTER_INIT(ctrKafkaRespTimedOut, mutCtrKafkaRespTimedOut);
	CHKiRet(statsobj.AddCounter(kafkaStats, (uchar *)"errors_timed_out",
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &ctrKafkaRespTimedOut));
	STATSCOUNTER_INIT(ctrKafkaRespTransport, mutCtrKafkaRespTransport);
	CHKiRet(statsobj.AddCounter(kafkaStats, (uchar *)"errors_transport",
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &ctrKafkaRespTransport));
	STATSCOUNTER_INIT(ctrKafkaRespBrokerDown, mutCtrKafkaRespBrokerDown);
	CHKiRet(statsobj.AddCounter(kafkaStats, (uchar *)"errors_broker_down",
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &ctrKafkaRespBrokerDown));
	STATSCOUNTER_INIT(ctrKafkaRespAuth, mutCtrKafkaRespAuth);
	CHKiRet(statsobj.AddCounter(kafkaStats, (uchar *)"errors_auth",
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &ctrKafkaRespAuth));
	STATSCOUNTER_INIT(ctrKafkaRespOther, mutCtrKafkaRespOther);
	CHKiRet(statsobj.AddCounter(kafkaStats, (uchar *)"errors_other",
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &ctrKafkaRespOther));
	CHKiRet(statsobj.AddCounter(kafkaStats, UCHAR_CONSTANT("rtt_avg_usec"),
		ctrType_Int, CTR_FLAG_NONE, &rtt_avg_usec));
	CHKiRet(statsobj.AddCounter(kafkaStats, UCHAR_CONSTANT("throttle_avg_msec"),
		ctrType_Int, CTR_FLAG_NONE, &throttle_avg_msec));
	CHKiRet(statsobj.AddCounter(kafkaStats, UCHAR_CONSTANT("int_latency_avg_usec"),
		ctrType_Int, CTR_FLAG_NONE, &int_latency_avg_usec));
	CHKiRet(statsobj.ConstructFinalize(kafkaStats));

	DBGPRINTF("omkafka: Add KAFKA_TimeStamp to template system ONCE\n");
	pTmp = (uchar*) KAFKA_TimeStamp;
	tplAddLine(ourConf, " KAFKA_TimeStamp", &pTmp);
ENDmodInit
