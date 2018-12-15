/* The regexp object.
 *
 * Module begun 2008-03-05 by Rainer Gerhards, based on some code
 * from syslogd.c
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

#include "config.h"
#include <pthread.h>
#include <regex.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "rsyslog.h"
#include "module-template.h"
#include "obj.h"
#include "regexp.h"
#include "errmsg.h"
#include "hashtable.h"
#include "hashtable_itr.h"

MODULE_TYPE_LIB
MODULE_TYPE_NOKEEP

/* static data */
DEFobjStaticHelpers

/* When using glibc, we enable per-thread regex to avoid lock contention.
 * See:
 * - https://github.com/rsyslog/rsyslog/issues/2759
 * - https://github.com/rsyslog/rsyslog/pull/2786
 * - https://sourceware.org/bugzilla/show_bug.cgi?id=11159
 *
 * This should not affect BSD as they don't seem to take a lock in regexec.
 */
#ifdef __GLIBC__
#define USE_PERTHREAD_REGEX 1
#else
#define USE_PERTHREAD_REGEX 0
#endif

static pthread_mutex_t mut_regexp;

// Map a regex_t to its associated uncompiled parameters.
static struct hashtable *regex_to_uncomp = NULL;

// Map a (regexp_t, pthead_t) to a perthread_regex.
static struct hashtable *perthread_regexs = NULL;


/*
 * This stores un-compiled regex to allow further
 * call to regexec to re-compile a new regex dedicated
 * to the calling thread.
 */
typedef struct uncomp_regex {
	char *regex;
	int cflags;
	regex_t *preg;
} uncomp_regex_t;

/*
 * This stores a regex dedicated to a single thread.
 */
typedef struct perthread_regex {
	const regex_t *original_preg;
	regex_t preg;
	int ret;
	pthread_mutex_t lock;
	pthread_t thread;
} perthread_regex_t;


static unsigned __attribute__((nonnull(1))) int hash_from_regex(void *k) {
	return (uintptr_t)*(regex_t **)k;
}

static int key_equals_regex(void *key1, void *key2) {
	return *(regex_t **)key1 == *(regex_t **)key2;
}

static unsigned __attribute__((nonnull(1))) int hash_from_tregex(void *k) {
	perthread_regex_t *entry = k;
	// Cast to (void*) is ok here because already used in other parts of the code.
	uintptr_t thread_id = (uintptr_t)(void *)entry->thread;

	return thread_id ^ (uintptr_t)entry->original_preg;
}

static int key_equals_tregex(void *key1, void *key2) {
	perthread_regex_t *entry1 = key1;
	perthread_regex_t *entry2 = key2;

	return (pthread_equal(entry1->thread, entry2->thread) &&
					entry1->original_preg == entry2->original_preg);
}


/* ------------------------------ methods ------------------------------ */


// Create a copy of preg to be used by this thread only.
static perthread_regex_t *create_perthread_regex(const regex_t *preg, uncomp_regex_t *uncomp) {
	perthread_regex_t *entry = NULL;

	if (Debug) {
		DBGPRINTF("Creating new regex_t for thread %p original regexp_t %p (pattern: %s, cflags: %x)\n",
							(void *)pthread_self(), preg,
							uncomp->regex, uncomp->cflags);
	}
	entry = calloc(1, sizeof(*entry));
	if (!entry)
		return entry;
	entry->original_preg = preg;
	DBGPRINTF("regexp: regcomp %p %p\n", entry, &entry->preg);
	entry->ret = regcomp(&entry->preg, uncomp->regex, uncomp->cflags);
	pthread_mutex_init(&entry->lock, NULL);
	entry->thread = pthread_self();
	return entry;
}

// Get (or create) a regex_t to be used by the current thread.
static perthread_regex_t *get_perthread_regex(const regex_t *preg) {
	perthread_regex_t *entry = NULL;
	perthread_regex_t key = { .original_preg = preg, .thread = pthread_self() };

	pthread_mutex_lock(&mut_regexp);
	entry = hashtable_search(perthread_regexs, (void *)&key);
	if (!entry) {
		uncomp_regex_t *uncomp = hashtable_search(regex_to_uncomp, (void *)&preg);

		if (uncomp) {
			entry = create_perthread_regex(preg, uncomp);
			if(!hashtable_insert(perthread_regexs, (void *)entry, entry)) {
				LogError(0, RS_RET_INTERNAL_ERROR,
					"error trying to insert thread-regexp into hash-table - things "
					"will not work 100%% correctly (mostly probably out of memory issue)");
			}
		}
	}
	if (entry) {
		pthread_mutex_lock(&entry->lock);
	}
	pthread_mutex_unlock(&mut_regexp);
	return entry;
}

static void remove_uncomp_regexp(regex_t *preg) {
	uncomp_regex_t *uncomp = NULL;

	pthread_mutex_lock(&mut_regexp);
	uncomp = hashtable_remove(regex_to_uncomp, (void *)&preg);

	if (uncomp) {
		if (Debug) {
			DBGPRINTF("Removing everything linked to regexp_t %p (pattern: %s, cflags: %x)\n",
								preg, uncomp->regex, uncomp->cflags);
		}
		free(uncomp->regex);
		free(uncomp);
	}
	pthread_mutex_unlock(&mut_regexp);
}

static void _regfree(regex_t *preg) {
	int ret = 0;
	struct hashtable_itr *itr = NULL;

	if (!preg)
		return;

	regfree(preg);
	remove_uncomp_regexp(preg);

	pthread_mutex_lock(&mut_regexp);
	if (!hashtable_count(perthread_regexs)) {
		pthread_mutex_unlock(&mut_regexp);
		return;
	}

	// This can be long to iterate other all regexps, but regfree doesn't get called
	// a lot during processing.
	itr = hashtable_iterator(perthread_regexs);
	do {
		perthread_regex_t *entry = (perthread_regex_t *)hashtable_iterator_value(itr);

		// Do it before freeing the entry.
		ret = hashtable_iterator_advance(itr);

		if (entry->original_preg == preg) {
			// This allows us to avoid freeing this while somebody is still using it.
			pthread_mutex_lock(&entry->lock);
			// We can unlock immediately after because mut_regexp is locked.
			pthread_mutex_unlock(&entry->lock);
			pthread_mutex_destroy(&entry->lock);
			regfree(&entry->preg);

			// Do it last because it will free entry.
			hashtable_remove(perthread_regexs, (void *)entry);
		}
	} while (ret);
	free(itr);

	pthread_mutex_unlock(&mut_regexp);
}

static int _regcomp(regex_t *preg, const char *regex, int cflags) {
	int ret = 0;
	regex_t **ppreg = NULL;
	uncomp_regex_t *uncomp;

	// Remove previous data if caller forgot to call regfree().
	remove_uncomp_regexp(preg);

	// Make sure preg itself it correctly initalized.
	ret = regcomp(preg, regex, cflags);
	if (ret != 0)
		return ret;

	uncomp = calloc(1, sizeof(*uncomp));
	if (!uncomp)
		return REG_ESPACE;

	uncomp->preg = preg;
	uncomp->regex = strdup(regex);
	uncomp->cflags = cflags;
	pthread_mutex_lock(&mut_regexp);

	// We need to allocate the key because hashtable will free it on remove.
	ppreg = malloc(sizeof(regex_t *));
	*ppreg = preg;
	ret = hashtable_insert(regex_to_uncomp, (void *)ppreg, uncomp);
	pthread_mutex_unlock(&mut_regexp);
	if (ret == 0) {
		free(uncomp->regex);
		free(uncomp);
		return REG_ESPACE;
	}

	perthread_regex_t *entry = get_perthread_regex(preg);
	if (entry) {
		ret = entry->ret;
		pthread_mutex_unlock(&entry->lock);
	} else {
		ret = REG_ESPACE;
	}
	return ret;
}

static int _regexec(const regex_t *preg, const char *string, size_t nmatch, regmatch_t pmatch[], int eflags) {
	perthread_regex_t *entry = get_perthread_regex(preg);
	int ret = REG_NOMATCH;
	if(entry != NULL) {
		ret = regexec(&entry->preg, string, nmatch, pmatch, eflags);
		pthread_mutex_unlock(&entry->lock);
	}
	return ret;
}

static size_t _regerror(int errcode, const regex_t *preg, char *errbuf, size_t errbuf_size) {
	perthread_regex_t *entry = get_perthread_regex(preg);

	if (entry)
		preg = &entry->preg;

	size_t ret = regerror(errcode, preg, errbuf, errbuf_size);

	if (entry)
		pthread_mutex_unlock(&entry->lock);

	return ret;
}

/* queryInterface function
 * rgerhards, 2008-03-05
 */
BEGINobjQueryInterface(regexp)
CODESTARTobjQueryInterface(regexp)
	if(pIf->ifVersion != regexpCURR_IF_VERSION) { /* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	if (USE_PERTHREAD_REGEX) {
		pIf->regcomp = _regcomp;
		pIf->regexec = _regexec;
		pIf->regerror = _regerror;
		pIf->regfree = _regfree;
	} else {
		pIf->regcomp = regcomp;
		pIf->regexec = regexec;
		pIf->regerror = regerror;
		pIf->regfree = regfree;
	}

finalize_it:
ENDobjQueryInterface(regexp)


/* Initialize the regexp class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-02-19
 */
BEGINAbstractObjClassInit(regexp, 1, OBJ_IS_LOADABLE_MODULE) /* class, version */
	/* request objects we use */

	if (USE_PERTHREAD_REGEX) {
		pthread_mutex_init(&mut_regexp, NULL);

		regex_to_uncomp = create_hashtable(100, hash_from_regex, key_equals_regex, NULL);
		perthread_regexs = create_hashtable(100, hash_from_tregex, key_equals_tregex, NULL);
		if(regex_to_uncomp == NULL || perthread_regexs == NULL) {
			LogError(0, RS_RET_INTERNAL_ERROR, "error trying to initialize hash-table "
							 "for regexp table. regexp will be disabled.");
			if (regex_to_uncomp) hashtable_destroy(regex_to_uncomp, 1);
			if (perthread_regexs) hashtable_destroy(perthread_regexs, 1);
			regex_to_uncomp = NULL;
			perthread_regexs = NULL;
			ABORT_FINALIZE(RS_RET_INTERNAL_ERROR);
		}
	}

ENDObjClassInit(regexp)


/* Exit the class.
 */
BEGINObjClassExit(regexp, OBJ_IS_LOADABLE_MODULE) /* class, version */
	if (USE_PERTHREAD_REGEX) {
		/* release objects we no longer need */
		pthread_mutex_destroy(&mut_regexp);
		if (regex_to_uncomp)
			hashtable_destroy(regex_to_uncomp, 1);
		if (perthread_regexs)
			hashtable_destroy(perthread_regexs, 1);
	}
ENDObjClassExit(regexp)


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

	CHKiRet(regexpClassInit(pModInfo)); /* must be done after tcps_sess, as we use it */
	/* Initialize all classes that are in our module - this includes ourselfs */
ENDmodInit
/* vi:set ai:
 */
