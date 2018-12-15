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
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <assert.h>

#include "rsyslog.h"
#include "srUtils.h"
#include "errmsg.h"
#include "rsconf.h"
#include "unicode-helper.h"

/* definitions for objects we access */
DEFobjStaticHelpers
DEFobjCurrIf(statsobj)

#define DYNSTATS_PARAM_NAME "name"
#define DYNSTATS_PARAM_RESETTABLE "resettable"
#define DYNSTATS_PARAM_MAX_CARDINALITY "maxCardinality"
#define DYNSTATS_PARAM_UNUSED_METRIC_LIFE "unusedMetricLife" /* in seconds */

#define DYNSTATS_DEFAULT_RESETTABILITY 1
#define DYNSTATS_DEFAULT_MAX_CARDINALITY 2000
#define DYNSTATS_DEFAULT_UNUSED_METRIC_LIFE 3600 /* seconds */

#define DYNSTATS_MAX_BUCKET_NS_METRIC_LENGTH 100
#define DYNSTATS_METRIC_NAME_SEPARATOR '.'
#define DYNSTATS_HASHTABLE_SIZE_OVERPROVISIONING 1.25

static struct cnfparamdescr modpdescr[] = {
	{ DYNSTATS_PARAM_NAME, eCmdHdlrString, CNFPARAM_REQUIRED },
	{ DYNSTATS_PARAM_RESETTABLE, eCmdHdlrBinary, 0 },
	{ DYNSTATS_PARAM_MAX_CARDINALITY, eCmdHdlrPositiveInt, 0},
	{ DYNSTATS_PARAM_UNUSED_METRIC_LIFE, eCmdHdlrPositiveInt, 0} /* in minutes */
};

static struct cnfparamblk modpblk =
{
	CNFPARAMBLK_VERSION,
	sizeof(modpdescr)/sizeof(struct cnfparamdescr),
	modpdescr
};

rsRetVal
dynstatsClassInit(void) {
	DEFiRet;
	CHKiRet(objGetObjInterface(&obj));
	CHKiRet(objUse(statsobj, CORE_COMPONENT));
finalize_it:
	RETiRet;
}

static void
dynstats_destroyCtr(dynstats_ctr_t *ctr) {
	statsobj.DestructUnlinkedCounter(ctr->pCtr);
	free(ctr->metric);
	free(ctr);
}

static void /* assumes exclusive access to bucket */
dynstats_destroyCountersIn(dynstats_bucket_t *b, htable *table, dynstats_ctr_t *ctrs) {
	dynstats_ctr_t *ctr;
	int ctrs_purged = 0;
	hashtable_destroy(table, 0);
	while (ctrs != NULL) {
		ctr = ctrs;
		ctrs = ctrs->next;
		dynstats_destroyCtr(ctr);
		ctrs_purged++;
	}
	STATSCOUNTER_ADD(b->ctrMetricsPurged, b->mutCtrMetricsPurged, ctrs_purged);
	ATOMIC_SUB_unsigned(&b->metricCount, ctrs_purged, &b->mutMetricCount);
}

static void /* assumes exclusive access to bucket */
dynstats_destroyCounters(dynstats_bucket_t *b) {
	statsobj.UnlinkAllCounters(b->stats);
	dynstats_destroyCountersIn(b, b->table, b->ctrs);
}

static void
dynstats_destroyBucket(dynstats_bucket_t* b) {
	dynstats_buckets_t *bkts;

	bkts = &loadConf->dynstats_buckets;

	pthread_rwlock_wrlock(&b->lock);
	dynstats_destroyCounters(b);
	dynstats_destroyCountersIn(b, b->survivor_table, b->survivor_ctrs);
	statsobj.Destruct(&b->stats);
	free(b->name);
	pthread_rwlock_unlock(&b->lock);
	pthread_rwlock_destroy(&b->lock);
	pthread_mutex_destroy(&b->mutMetricCount);
	statsobj.DestructCounter(bkts->global_stats, b->pOpsOverflowCtr);
	statsobj.DestructCounter(bkts->global_stats, b->pNewMetricAddCtr);
	statsobj.DestructCounter(bkts->global_stats, b->pNoMetricCtr);
	statsobj.DestructCounter(bkts->global_stats, b->pMetricsPurgedCtr);
	statsobj.DestructCounter(bkts->global_stats, b->pOpsIgnoredCtr);
	statsobj.DestructCounter(bkts->global_stats, b->pPurgeTriggeredCtr);
	free(b);
}

static rsRetVal
dynstats_addBucketMetrics(dynstats_buckets_t *bkts, dynstats_bucket_t *b, const uchar* name) {
	uchar *metric_name_buff, *metric_suffix;
	const uchar *suffix_litteral;
	int name_len;
	DEFiRet;

	name_len = ustrlen(name);
	CHKmalloc(metric_name_buff = malloc((name_len + DYNSTATS_MAX_BUCKET_NS_METRIC_LENGTH + 1) * sizeof(uchar)));

	strcpy((char*)metric_name_buff, (char*)name);
	metric_suffix = metric_name_buff + name_len;
	*metric_suffix = DYNSTATS_METRIC_NAME_SEPARATOR;
	metric_suffix++;

	suffix_litteral = UCHAR_CONSTANT("ops_overflow");
	ustrncpy(metric_suffix, suffix_litteral, DYNSTATS_MAX_BUCKET_NS_METRIC_LENGTH);
	STATSCOUNTER_INIT(b->ctrOpsOverflow, b->mutCtrOpsOverflow);
	CHKiRet(statsobj.AddManagedCounter(bkts->global_stats, metric_name_buff, ctrType_IntCtr,
									   CTR_FLAG_RESETTABLE,
										&(b->ctrOpsOverflow),
										&b->pOpsOverflowCtr, 1));

	suffix_litteral = UCHAR_CONSTANT("new_metric_add");
	ustrncpy(metric_suffix, suffix_litteral, DYNSTATS_MAX_BUCKET_NS_METRIC_LENGTH);
	STATSCOUNTER_INIT(b->ctrNewMetricAdd, b->mutCtrNewMetricAdd);
	CHKiRet(statsobj.AddManagedCounter(bkts->global_stats, metric_name_buff, ctrType_IntCtr,
									   CTR_FLAG_RESETTABLE,
										&(b->ctrNewMetricAdd),
										&b->pNewMetricAddCtr, 1));

	suffix_litteral = UCHAR_CONSTANT("no_metric");
	ustrncpy(metric_suffix, suffix_litteral, DYNSTATS_MAX_BUCKET_NS_METRIC_LENGTH);
	STATSCOUNTER_INIT(b->ctrNoMetric, b->mutCtrNoMetric);
	CHKiRet(statsobj.AddManagedCounter(bkts->global_stats, metric_name_buff, ctrType_IntCtr,
									   CTR_FLAG_RESETTABLE,
										&(b->ctrNoMetric),
										&b->pNoMetricCtr, 1));

	suffix_litteral = UCHAR_CONSTANT("metrics_purged");
	ustrncpy(metric_suffix, suffix_litteral, DYNSTATS_MAX_BUCKET_NS_METRIC_LENGTH);
	STATSCOUNTER_INIT(b->ctrMetricsPurged, b->mutCtrMetricsPurged);
	CHKiRet(statsobj.AddManagedCounter(bkts->global_stats, metric_name_buff, ctrType_IntCtr,
									   CTR_FLAG_RESETTABLE,
										&(b->ctrMetricsPurged),
										&b->pMetricsPurgedCtr, 1));

	suffix_litteral = UCHAR_CONSTANT("ops_ignored");
	ustrncpy(metric_suffix, suffix_litteral, DYNSTATS_MAX_BUCKET_NS_METRIC_LENGTH);
	STATSCOUNTER_INIT(b->ctrOpsIgnored, b->mutCtrOpsIgnored);
	CHKiRet(statsobj.AddManagedCounter(bkts->global_stats, metric_name_buff, ctrType_IntCtr,
									   CTR_FLAG_RESETTABLE,
										&(b->ctrOpsIgnored),
										&b->pOpsIgnoredCtr, 1));

	suffix_litteral = UCHAR_CONSTANT("purge_triggered");
	ustrncpy(metric_suffix, suffix_litteral, DYNSTATS_MAX_BUCKET_NS_METRIC_LENGTH);
	STATSCOUNTER_INIT(b->ctrPurgeTriggered, b->mutCtrPurgeTriggered);
	CHKiRet(statsobj.AddManagedCounter(bkts->global_stats, metric_name_buff, ctrType_IntCtr,
									   CTR_FLAG_RESETTABLE,
										&(b->ctrPurgeTriggered),
										&b->pPurgeTriggeredCtr, 1));

finalize_it:
	free(metric_name_buff);
	if (iRet != RS_RET_OK) {
		if (b->pOpsOverflowCtr != NULL) {
			statsobj.DestructCounter(bkts->global_stats, b->pOpsOverflowCtr);
		}
		if (b->pNewMetricAddCtr != NULL) {
			statsobj.DestructCounter(bkts->global_stats, b->pNewMetricAddCtr);
		}
		if (b->pNoMetricCtr != NULL) {
			statsobj.DestructCounter(bkts->global_stats, b->pNoMetricCtr);
		}
		if (b->pMetricsPurgedCtr != NULL) {
			statsobj.DestructCounter(bkts->global_stats, b->pMetricsPurgedCtr);
		}
		if (b->pOpsIgnoredCtr != NULL) {
			statsobj.DestructCounter(bkts->global_stats, b->pOpsIgnoredCtr);
		}
		if (b->pPurgeTriggeredCtr != NULL) {
			statsobj.DestructCounter(bkts->global_stats, b->pPurgeTriggeredCtr);
		}
	}
	RETiRet;
}

static void
no_op_free(void __attribute__((unused)) *ignore)  {}

static rsRetVal  /* assumes exclusive access to bucket */
dynstats_rebuildSurvivorTable(dynstats_bucket_t *b) {
	htable *survivor_table = NULL;
	htable *new_table = NULL;
	size_t htab_sz;
	DEFiRet;
	
	htab_sz = (size_t) (DYNSTATS_HASHTABLE_SIZE_OVERPROVISIONING * b->maxCardinality + 1);
	if (b->table == NULL) {
		CHKmalloc(survivor_table = create_hashtable(htab_sz, hash_from_string, key_equals_string,
			no_op_free));
	}
	CHKmalloc(new_table = create_hashtable(htab_sz, hash_from_string, key_equals_string, no_op_free));
	statsobj.UnlinkAllCounters(b->stats);
	if (b->survivor_table != NULL) {
		dynstats_destroyCountersIn(b, b->survivor_table, b->survivor_ctrs);
	}
	b->survivor_table = (b->table == NULL) ? survivor_table : b->table;
	b->survivor_ctrs = b->ctrs;
	b->table = new_table;
	b->ctrs = NULL;
finalize_it:
	if (iRet != RS_RET_OK) {
		LogError(errno, RS_RET_INTERNAL_ERROR, "error trying to evict "
			"TTL-expired metrics of dyn-stats bucket named: %s", b->name);
		if (new_table == NULL) {
			LogError(errno, RS_RET_INTERNAL_ERROR, "error trying to "
				"initialize hash-table for dyn-stats bucket named: %s", b->name);
		} else {
			assert(0); /* "can" not happen -- triggers Coverity CID 184307:
			hashtable_destroy(new_table, 0);
			We keep this as guard should code above change in the future */
		}
		if (b->table == NULL) {
			if (survivor_table == NULL) {
				LogError(errno, RS_RET_INTERNAL_ERROR, "error trying to initialize "
				"ttl-survivor hash-table for dyn-stats bucket named: %s", b->name);
			} else {
				hashtable_destroy(survivor_table, 0);
			}
		}
	}
	RETiRet;
}

static rsRetVal
dynstats_resetBucket(dynstats_bucket_t *b) {
	DEFiRet;
	pthread_rwlock_wrlock(&b->lock);
	CHKiRet(dynstats_rebuildSurvivorTable(b));
	STATSCOUNTER_INC(b->ctrPurgeTriggered, b->mutCtrPurgeTriggered);
	timeoutComp(&b->metricCleanupTimeout, b->unusedMetricLife);
finalize_it:
	pthread_rwlock_unlock(&b->lock);
	RETiRet;
}

static void
dynstats_resetIfExpired(dynstats_bucket_t *b) {
	long timeout;
	pthread_rwlock_rdlock(&b->lock);
	timeout = timeoutVal(&b->metricCleanupTimeout);
	pthread_rwlock_unlock(&b->lock);
	if (timeout == 0) {
		LogMsg(0, RS_RET_TIMED_OUT, LOG_INFO, "dynstats: bucket '%s' is being reset", b->name);
		dynstats_resetBucket(b);
	}
}

static void
dynstats_readCallback(statsobj_t __attribute__((unused)) *ignore, void *b) {
	dynstats_buckets_t *bkts;
	bkts = &loadConf->dynstats_buckets;

	pthread_rwlock_rdlock(&bkts->lock);
	dynstats_resetIfExpired((dynstats_bucket_t *) b);
	pthread_rwlock_unlock(&bkts->lock);
}

static rsRetVal
dynstats_initNewBucketStats(dynstats_bucket_t *b) {
	DEFiRet;
	
	CHKiRet(statsobj.Construct(&b->stats));
	CHKiRet(statsobj.SetOrigin(b->stats, UCHAR_CONSTANT("dynstats.bucket")));
	CHKiRet(statsobj.SetName(b->stats, b->name));
	CHKiRet(statsobj.SetReportingNamespace(b->stats, UCHAR_CONSTANT("values")));
	statsobj.SetReadNotifier(b->stats, dynstats_readCallback, b);
	CHKiRet(statsobj.ConstructFinalize(b->stats));
	
finalize_it:
	RETiRet;
}

static rsRetVal
dynstats_newBucket(const uchar* name, uint8_t resettable, uint32_t maxCardinality, uint32_t unusedMetricLife) {
	dynstats_bucket_t *b;
	dynstats_buckets_t *bkts;
	uint8_t lock_initialized, metric_count_mutex_initialized;
	pthread_rwlockattr_t bucket_lock_attr;
	DEFiRet;

	lock_initialized = metric_count_mutex_initialized = 0;
	b = NULL;
	
	bkts = &loadConf->dynstats_buckets;

	if (bkts->initialized) {
		CHKmalloc(b = calloc(1, sizeof(dynstats_bucket_t)));
		b->resettable = resettable;
		b->maxCardinality = maxCardinality;
		b->unusedMetricLife = 1000 * unusedMetricLife;
		CHKmalloc(b->name = ustrdup(name));

		pthread_rwlockattr_init(&bucket_lock_attr);
#ifdef HAVE_PTHREAD_RWLOCKATTR_SETKIND_NP
		pthread_rwlockattr_setkind_np(&bucket_lock_attr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
#endif

		pthread_rwlock_init(&b->lock, &bucket_lock_attr);
		lock_initialized = 1;
		pthread_mutex_init(&b->mutMetricCount, NULL);
		metric_count_mutex_initialized = 1;

		CHKiRet(dynstats_initNewBucketStats(b));

		CHKiRet(dynstats_resetBucket(b));

		CHKiRet(dynstats_addBucketMetrics(bkts, b, name));

		pthread_rwlock_wrlock(&bkts->lock);
		if (bkts->list == NULL) {
			bkts->list = b;
		} else {
			b->next = bkts->list;
			bkts->list = b;
		}
		pthread_rwlock_unlock(&bkts->lock);
	} else {
		LogError(0, RS_RET_INTERNAL_ERROR, "dynstats: bucket creation failed, as "
		"global-initialization of buckets was unsuccessful");
		ABORT_FINALIZE(RS_RET_INTERNAL_ERROR);
	}
finalize_it:
	if (iRet != RS_RET_OK) {
		if (metric_count_mutex_initialized) {
			pthread_mutex_destroy(&b->mutMetricCount);
		}
		if (lock_initialized) {
			pthread_rwlock_destroy(&b->lock);
		}
		if (b != NULL) {
			dynstats_destroyBucket(b);
		}
	}
	RETiRet;
}

rsRetVal
dynstats_processCnf(struct cnfobj *o) {
	struct cnfparamvals *pvals;
	short i;
	uchar *name = NULL;
	uint8_t resettable = DYNSTATS_DEFAULT_RESETTABILITY;
	uint32_t maxCardinality = DYNSTATS_DEFAULT_MAX_CARDINALITY;
	uint32_t unusedMetricLife = DYNSTATS_DEFAULT_UNUSED_METRIC_LIFE;
	DEFiRet;

	pvals = nvlstGetParams(o->nvlst, &modpblk, NULL);
	if(pvals == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}
	
	for(i = 0 ; i < modpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(modpblk.descr[i].name, DYNSTATS_PARAM_NAME)) {
			CHKmalloc(name = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL));
		} else if (!strcmp(modpblk.descr[i].name, DYNSTATS_PARAM_RESETTABLE)) {
			resettable = (pvals[i].val.d.n != 0);
		} else if (!strcmp(modpblk.descr[i].name, DYNSTATS_PARAM_MAX_CARDINALITY)) {
			maxCardinality = (uint32_t) pvals[i].val.d.n;
		} else if (!strcmp(modpblk.descr[i].name, DYNSTATS_PARAM_UNUSED_METRIC_LIFE)) {
			unusedMetricLife = (uint32_t) pvals[i].val.d.n;
		} else {
			dbgprintf("dyn_stats: program error, non-handled "
					  "param '%s'\n", modpblk.descr[i].name);
		}
	}
	if (name != NULL) {
		CHKiRet(dynstats_newBucket(name, resettable, maxCardinality, unusedMetricLife));
	}

finalize_it:
	free(name);
	cnfparamvalsDestruct(pvals, &modpblk);
	RETiRet;
}

rsRetVal
dynstats_initCnf(dynstats_buckets_t *bkts) {
	DEFiRet;

	bkts->initialized = 0;
	
	bkts->list = NULL;
	CHKiRet(statsobj.Construct(&bkts->global_stats));
	CHKiRet(statsobj.SetOrigin(bkts->global_stats, UCHAR_CONSTANT("dynstats")));
	CHKiRet(statsobj.SetName(bkts->global_stats, UCHAR_CONSTANT("global")));
	CHKiRet(statsobj.SetReportingNamespace(bkts->global_stats, UCHAR_CONSTANT("values")));
	CHKiRet(statsobj.ConstructFinalize(bkts->global_stats));
	pthread_rwlock_init(&bkts->lock, NULL);

	bkts->initialized = 1;
	
finalize_it:
	if (iRet != RS_RET_OK) {
		statsobj.Destruct(&bkts->global_stats);
	}
	RETiRet;
}

void
dynstats_destroyAllBuckets(void) {
	dynstats_buckets_t *bkts;
	dynstats_bucket_t *b;
	bkts = &loadConf->dynstats_buckets;
	if (bkts->initialized) {
		pthread_rwlock_wrlock(&bkts->lock);
		while(1) {
			b = bkts->list;
			if (b == NULL) {
				break;
			} else {
				bkts->list = b->next;
				dynstats_destroyBucket(b);
			}
		}
		statsobj.Destruct(&bkts->global_stats);
		pthread_rwlock_unlock(&bkts->lock);
		pthread_rwlock_destroy(&bkts->lock);
	}
}

dynstats_bucket_t *
dynstats_findBucket(const uchar* name) {
	dynstats_buckets_t *bkts;
	dynstats_bucket_t *b;
	bkts = &loadConf->dynstats_buckets;
	if (bkts->initialized) {
		pthread_rwlock_rdlock(&bkts->lock);
		b = bkts->list;
		while(b != NULL) {
			if (! ustrcmp(name, b->name)) {
				break;
			}
			b = b->next;
		}
		pthread_rwlock_unlock(&bkts->lock);
	} else {
		b = NULL;
		LogError(0, RS_RET_INTERNAL_ERROR, "dynstats: bucket lookup failed, as global-initialization "
		"of buckets was unsuccessful");
	}

	return b;
}

static rsRetVal
dynstats_createCtr(dynstats_bucket_t *b, const uchar* metric, dynstats_ctr_t **ctr) {
	DEFiRet;
	
	CHKmalloc(*ctr = calloc(1, sizeof(dynstats_ctr_t)));
	CHKmalloc((*ctr)->metric = ustrdup(metric));
	STATSCOUNTER_INIT((*ctr)->ctr, (*ctr)->mutCtr);
	CHKiRet(statsobj.AddManagedCounter(b->stats, metric, ctrType_IntCtr,
				b->resettable ? CTR_FLAG_MUST_RESET : CTR_FLAG_NONE,
				&(*ctr)->ctr, &(*ctr)->pCtr, 0));
finalize_it:
	if (iRet != RS_RET_OK) {
		if ((*ctr) != NULL) {
			free((*ctr)->metric);
			free(*ctr);
			*ctr = NULL;
		}
	}
	RETiRet;
}

#if !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized" /* TODO: how can we fix these warnings? */
#endif
static rsRetVal
dynstats_addNewCtr(dynstats_bucket_t *b, const uchar* metric, uint8_t doInitialIncrement) {
	dynstats_ctr_t *ctr;
	dynstats_ctr_t *found_ctr, *survivor_ctr, *effective_ctr;
	int created;
	uchar *copy_of_key = NULL;
	DEFiRet;

	created = 0;
	ctr = NULL;

	if ((unsigned) ATOMIC_FETCH_32BIT_unsigned(&b->metricCount, &b->mutMetricCount) >= b->maxCardinality) {
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
	}
	
	CHKiRet(dynstats_createCtr(b, metric, &ctr));

	pthread_rwlock_wrlock(&b->lock);
	found_ctr = (dynstats_ctr_t*) hashtable_search(b->table, ctr->metric);
	if (found_ctr != NULL) {
		if (doInitialIncrement) {
			STATSCOUNTER_INC(found_ctr->ctr, found_ctr->mutCtr);
		}
	} else {
		copy_of_key = ustrdup(ctr->metric);
		if (copy_of_key != NULL) {
			survivor_ctr = (dynstats_ctr_t*) hashtable_search(b->survivor_table, ctr->metric);
			if (survivor_ctr == NULL) {
				effective_ctr = ctr;
			} else {
				effective_ctr = survivor_ctr;
				if (survivor_ctr->prev != NULL) {
					survivor_ctr->prev->next = survivor_ctr->next;
				}
				if (survivor_ctr->next != NULL) {
					survivor_ctr->next->prev = survivor_ctr->prev;
				}
				if (survivor_ctr == b->survivor_ctrs) {
					b->survivor_ctrs = survivor_ctr->next;
				}
			}
			if ((created = hashtable_insert(b->table, copy_of_key, effective_ctr))) {
				statsobj.AddPreCreatedCtr(b->stats, effective_ctr->pCtr);
			}
		}
		if (created) {
			if (b->ctrs != NULL) {
				b->ctrs->prev = effective_ctr;
			}
			effective_ctr->prev = NULL;
			effective_ctr->next = b->ctrs;
			b->ctrs = effective_ctr;
			if (doInitialIncrement) {
				STATSCOUNTER_INC(effective_ctr->ctr, effective_ctr->mutCtr);
			}
		}
	}
	pthread_rwlock_unlock(&b->lock);

	if (found_ctr != NULL) {
		//ignore
	} else if (created && (effective_ctr != survivor_ctr)) {
		ATOMIC_INC(&b->metricCount, &b->mutMetricCount);
		STATSCOUNTER_INC(b->ctrNewMetricAdd, b->mutCtrNewMetricAdd);
	} else if (! created) {
		if (copy_of_key != NULL) {
			free(copy_of_key);
		}
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
	}
	
finalize_it:
	if (((! created) || (effective_ctr != ctr)) && (ctr != NULL)) {
		dynstats_destroyCtr(ctr);
	}
	RETiRet;
}
#if !defined(__clang__)
#pragma GCC diagnostic pop
#endif

rsRetVal
dynstats_inc(dynstats_bucket_t *b, uchar* metric) {
	dynstats_ctr_t *ctr;
	DEFiRet;

	if (! GatherStats) {
		FINALIZE;
	}

	if (ustrlen(metric) == 0) {
		STATSCOUNTER_INC(b->ctrNoMetric, b->mutCtrNoMetric);
		FINALIZE;
	}

	if (pthread_rwlock_tryrdlock(&b->lock) == 0) {
		ctr = (dynstats_ctr_t *) hashtable_search(b->table, metric);
		if (ctr != NULL) {
			STATSCOUNTER_INC(ctr->ctr, ctr->mutCtr);
		}
		pthread_rwlock_unlock(&b->lock);
	} else {
		ABORT_FINALIZE(RS_RET_NOENTRY);
	}

	if (ctr == NULL) {
		CHKiRet(dynstats_addNewCtr(b, metric, 1));
	}
finalize_it:
	if (iRet != RS_RET_OK) {
		if (iRet == RS_RET_NOENTRY) {
			/* NOTE: this is not tested (because it requires very strong orchestration to
			gurantee contended lock for testing) */
			STATSCOUNTER_INC(b->ctrOpsIgnored, b->mutCtrOpsIgnored);
		} else {
			STATSCOUNTER_INC(b->ctrOpsOverflow, b->mutCtrOpsOverflow);
		}
	}
	RETiRet;
}

