/*
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
#ifndef INCLUDED_DYNSTATS_H
#define INCLUDED_DYNSTATS_H

#include "hashtable.h"

typedef struct hashtable htable;

struct dynstats_ctr_s {
	STATSCOUNTER_DEF(ctr, mutCtr);
	ctr_t *pCtr;
	uchar *metric;
	/* linked list ptr */
	struct dynstats_ctr_s *next;
	struct dynstats_ctr_s *prev;
};

struct dynstats_bucket_s {
	htable *table;
	uchar *name;
	pthread_rwlock_t lock;
	statsobj_t *stats;
	STATSCOUNTER_DEF(ctrOpsOverflow, mutCtrOpsOverflow);
	ctr_t *pOpsOverflowCtr;
	STATSCOUNTER_DEF(ctrNewMetricAdd, mutCtrNewMetricAdd);
	ctr_t *pNewMetricAddCtr;
	STATSCOUNTER_DEF(ctrNoMetric, mutCtrNoMetric);
	ctr_t *pNoMetricCtr;
	STATSCOUNTER_DEF(ctrMetricsPurged, mutCtrMetricsPurged);
	ctr_t *pMetricsPurgedCtr;
	STATSCOUNTER_DEF(ctrOpsIgnored, mutCtrOpsIgnored);
	ctr_t *pOpsIgnoredCtr;
	STATSCOUNTER_DEF(ctrPurgeTriggered, mutCtrPurgeTriggered);
	ctr_t *pPurgeTriggeredCtr;
	struct dynstats_bucket_s *next; /* linked list ptr */
	struct dynstats_ctr_s *ctrs;
	/*survivor objects are used to keep counter values around for upto unused-ttl duration,
	  so in case it is accessed within (ttl - 2 * ttl) time-period we can re-store the
	  accumulator value from this */
	struct dynstats_ctr_s *survivor_ctrs;
	htable *survivor_table;
	
	uint32_t maxCardinality;
	uint32_t metricCount;
	pthread_mutex_t mutMetricCount;
	uint32_t unusedMetricLife;
	uint32_t lastResetTs;
	struct timespec metricCleanupTimeout;
	uint8_t resettable;
};

struct dynstats_buckets_s {
	struct dynstats_bucket_s *list;
	statsobj_t *global_stats;
	pthread_rwlock_t lock;
	uint8_t initialized;
};

rsRetVal dynstats_initCnf(dynstats_buckets_t *b);
rsRetVal dynstats_processCnf(struct cnfobj *o);
dynstats_bucket_t * dynstats_findBucket(const uchar* name);
rsRetVal dynstats_inc(dynstats_bucket_t *bucket, uchar* metric);
void dynstats_destroyAllBuckets(void);
void dynstats_resetExpired(void);
rsRetVal dynstatsClassInit(void);

#endif /* #ifndef INCLUDED_DYNSTATS_H */
