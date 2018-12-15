/* lookup.c
 * Support for lookup tables in RainerScript.
 *
 * Copyright 2013-2018 Adiscon GmbH.
 *
 * This file is part of the rsyslog runtime library.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *		 http://www.apache.org/licenses/LICENSE-2.0
 *		 -or-
 *		 see COPYING.ASL20 in the source distribution
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
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <json.h>
#include <assert.h>

#include "rsyslog.h"
#include "srUtils.h"
#include "errmsg.h"
#include "lookup.h"
#include "msg.h"
#include "rsconf.h"
#include "dirty.h"
#include "unicode-helper.h"

#if !defined(_AIX)
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

/* definitions for objects we access */
DEFobjStaticHelpers
DEFobjCurrIf(glbl)

/* forward definitions */
static rsRetVal lookupReadFile(lookup_t *pThis, const uchar* name, const uchar* filename);
static void lookupDestruct(lookup_t *pThis);

/* static data */
/* tables for interfacing with the v6 config system (as far as we need to) */
static struct cnfparamdescr modpdescr[] = {
	{ "name", eCmdHdlrString, CNFPARAM_REQUIRED },
	{ "file", eCmdHdlrString, CNFPARAM_REQUIRED },
	{ "reloadOnHUP", eCmdHdlrBinary, 0 }
};
static struct cnfparamblk modpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(modpdescr)/sizeof(struct cnfparamdescr),
	  modpdescr
	};

/* internal data-types */
typedef struct uint32_index_val_s {
	uint32_t index;
	uchar *val;
} uint32_index_val_t;

const char * reloader_prefix = "lkp_tbl_reloader:";

static void *
lookupTableReloader(void *self);

static void
lookupStopReloader(lookup_ref_t *pThis);

/* create a new lookup table object AND include it in our list of
 * lookup tables.
 */
static rsRetVal
lookupNew(lookup_ref_t **ppThis)
{
	lookup_ref_t *pThis = NULL;
	lookup_t *t = NULL;
	int initialized = 0;
	DEFiRet;

	CHKmalloc(pThis = calloc(1, sizeof(lookup_ref_t)));
	CHKmalloc(t = calloc(1, sizeof(lookup_t)));
	CHKiConcCtrl(pthread_rwlock_init(&pThis->rwlock, NULL));
	initialized++; /*1*/
	CHKiConcCtrl(pthread_mutex_init(&pThis->reloader_mut, NULL));
	initialized++; /*2*/
	CHKiConcCtrl(pthread_cond_init(&pThis->run_reloader, NULL));
	initialized++; /*3*/
	CHKiConcCtrl(pthread_attr_init(&pThis->reloader_thd_attr));
	initialized++; /*4*/
	pThis->do_reload = pThis->do_stop = 0;
	pThis->reload_on_hup = 1; /*DO reload on HUP (default)*/
	CHKiConcCtrl(pthread_create(&pThis->reloader, &pThis->reloader_thd_attr,
		lookupTableReloader, pThis));
	initialized++; /*5*/

	pThis->next = NULL;
	if(loadConf->lu_tabs.root == NULL) {
		loadConf->lu_tabs.root = pThis;
	} else {
		loadConf->lu_tabs.last->next = pThis;
	}
	loadConf->lu_tabs.last = pThis;

	pThis->self = t;

	*ppThis = pThis;
finalize_it:
	if(iRet != RS_RET_OK) {
		LogError(errno, iRet, "a lookup table could not be initialized: "
			"failed at init-step %d (please enable debug logs for details)",
			initialized);
		/* Can not happen with current code, but might occur in the future when
		 * an error-condition as added after step 5. If we leave it in, Coverity
		 * scan complains. So we comment it out but do not remove the code.
		 * Triggered by CID 185426
		if (initialized > 4) lookupStopReloader(pThis);
		*/
		if (initialized > 3) pthread_attr_destroy(&pThis->reloader_thd_attr);
		if (initialized > 2) pthread_cond_destroy(&pThis->run_reloader);
		if (initialized > 1) pthread_mutex_destroy(&pThis->reloader_mut);
		if (initialized > 0) pthread_rwlock_destroy(&pThis->rwlock);
		free(t);
		free(pThis);
	}
	RETiRet;
}

/*must be called with reloader_mut acquired*/
static void ATTR_NONNULL()
freeStubValueForReloadFailure(lookup_ref_t *const pThis)
{
	if (pThis->stub_value_for_reload_failure != NULL) {
		free(pThis->stub_value_for_reload_failure);
		pThis->stub_value_for_reload_failure = NULL;
	}
}

static void
lookupStopReloader(lookup_ref_t *pThis) {
	pthread_mutex_lock(&pThis->reloader_mut);
	freeStubValueForReloadFailure(pThis);
	pThis->do_reload = 0;
	pThis->do_stop = 1;
	pthread_cond_signal(&pThis->run_reloader);
	pthread_mutex_unlock(&pThis->reloader_mut);
	pthread_join(pThis->reloader, NULL);
}

static void
lookupRefDestruct(lookup_ref_t *pThis)
{
	lookupStopReloader(pThis);
	pthread_mutex_destroy(&pThis->reloader_mut);
	pthread_cond_destroy(&pThis->run_reloader);
	pthread_attr_destroy(&pThis->reloader_thd_attr);

	pthread_rwlock_destroy(&pThis->rwlock);
	lookupDestruct(pThis->self);
	free(pThis->name);
	free(pThis->filename);
	free(pThis);
}

static void
destructTable_str(lookup_t *pThis) {
	uint32_t i = 0;
	lookup_string_tab_entry_t *entries = pThis->table.str->entries;
	for (i = 0; i < pThis->nmemb; i++) {
		free(entries[i].key);
	}
	free(entries);
	free(pThis->table.str);
}


static void
destructTable_arr(lookup_t *pThis) {
	free(pThis->table.arr->interned_val_refs);
	free(pThis->table.arr);
}

static void
destructTable_sparseArr(lookup_t *pThis) {
	free(pThis->table.sprsArr->entries);
	free(pThis->table.sprsArr);
}

static void
lookupDestruct(lookup_t *pThis) {
	uint32_t i;

	if (pThis == NULL) return;

	if (pThis->type == STRING_LOOKUP_TABLE) {
		destructTable_str(pThis);
	} else if (pThis->type == ARRAY_LOOKUP_TABLE) {
		destructTable_arr(pThis);
	} else if (pThis->type == SPARSE_ARRAY_LOOKUP_TABLE) {
		destructTable_sparseArr(pThis);
	} else if (pThis->type == STUBBED_LOOKUP_TABLE) {
		/*nothing to be done*/
	}

	for (i = 0; i < pThis->interned_val_count; i++) {
		free(pThis->interned_vals[i]);
	}
	free(pThis->interned_vals);
	free(pThis->nomatch);
	free(pThis);
}

void
lookupInitCnf(lookup_tables_t *lu_tabs)
{
	lu_tabs->root = NULL;
	lu_tabs->last = NULL;
}

void
lookupDestroyCnf(void)
{
	lookup_ref_t *luref, *luref_next;
	for(luref = loadConf->lu_tabs.root ; luref != NULL ; ) {
		luref_next = luref->next;
		lookupRefDestruct(luref);
		luref = luref_next;
	}
}

/* comparison function for qsort() */
static int
qs_arrcmp_strtab(const void *s1, const void *s2)
{
	return ustrcmp(((lookup_string_tab_entry_t*)s1)->key, ((lookup_string_tab_entry_t*)s2)->key);
}

static int
qs_arrcmp_ustrs(const void *s1, const void *s2)
{
	return ustrcmp(*(uchar**)s1, *(uchar**)s2);
}

static int
qs_arrcmp_uint32_index_val(const void *s1, const void *s2)
{
	uint32_t first_value = ((uint32_index_val_t*)s1)->index;
	uint32_t second_value = ((uint32_index_val_t*)s2)->index;
	if (first_value < second_value) {
		return -1;
	}
	return first_value - second_value;
}

static int
qs_arrcmp_sprsArrtab(const void *s1, const void *s2)
{
	uint32_t first_value = ((lookup_sparseArray_tab_entry_t*)s1)->key;
	uint32_t second_value = ((lookup_sparseArray_tab_entry_t*)s2)->key;
	if (first_value < second_value) {
		return -1;
	}
	return first_value - second_value;
}

/* comparison function for bsearch() and string array compare
 * this is for the string lookup table type
 */
static int
bs_arrcmp_strtab(const void *s1, const void *s2)
{
	return strcmp((char*)s1, (char*)((lookup_string_tab_entry_t*)s2)->key);
}

static int
bs_arrcmp_str(const void *s1, const void *s2)
{
	return ustrcmp((uchar*)s1, *(uchar**)s2);
}

static int
bs_arrcmp_sprsArrtab(const void *s1, const void *s2)
{
	uint32_t key = *(uint32_t*)s1;
	uint32_t array_member_value = ((lookup_sparseArray_tab_entry_t*)s2)->key;
	if (key < array_member_value) {
		return -1;
	}
	return key - array_member_value;
}

static inline const char*
defaultVal(lookup_t *pThis) {
	return (pThis->nomatch == NULL) ? "" : (const char*) pThis->nomatch;
}

/* lookup_fn for different types of tables */
static es_str_t*
lookupKey_stub(lookup_t *pThis, lookup_key_t __attribute__((unused)) key) {
	return es_newStrFromCStr((char*) pThis->nomatch, ustrlen(pThis->nomatch));
}

static es_str_t*
lookupKey_str(lookup_t *pThis, lookup_key_t key) {
	lookup_string_tab_entry_t *entry;
	const char *r;
	if(pThis->nmemb == 0) {
		entry = NULL;
	} else {
		assert(pThis->table.str->entries);
		entry = bsearch(key.k_str, pThis->table.str->entries, pThis->nmemb,
			sizeof(lookup_string_tab_entry_t), bs_arrcmp_strtab);
	}
	if(entry == NULL) {
		r = defaultVal(pThis);
	} else {
		r = (const char*)entry->interned_val_ref;
	}
	return es_newStrFromCStr(r, strlen(r));
}

static es_str_t*
lookupKey_arr(lookup_t *pThis, lookup_key_t key) {
	const char *r;
	uint32_t uint_key = key.k_uint;
	if ((pThis->nmemb == 0) || (uint_key < pThis->table.arr->first_key)) {
		r = defaultVal(pThis);
	} else {
		uint32_t idx = uint_key - pThis->table.arr->first_key;
		if (idx >= pThis->nmemb) {
			r = defaultVal(pThis);
		} else {
		    r = (char*) pThis->table.arr->interned_val_refs[idx];
		}
	}

	return es_newStrFromCStr(r, strlen(r));
}

typedef int (comp_fn_t)(const void *s1, const void *s2);

static void *
bsearch_lte(const void *key, const void *base, size_t nmemb, size_t size, comp_fn_t *comp_fn)
{
	size_t l, u, idx;
	const void *p;
	int comparison;

	l = 0;
	u = nmemb;
	if (l == u) {
		return NULL;
	}
	while (l < u) {
		idx = (l + u) / 2;
		p = (void *) (((const char *) base) + (idx * size));
		comparison = (*comp_fn)(key, p);
		if (comparison < 0)
			u = idx;
		else if (comparison > 0)
			l = idx + 1;
		else
			return (void *) p;
	}
	if (comparison < 0) {
		if (idx == 0) {
			return NULL;
		}
		idx--;
	}
	return (void *) (((const char *) base) + ( idx * size));
}

static es_str_t*
lookupKey_sprsArr(lookup_t *pThis, lookup_key_t key) {
	lookup_sparseArray_tab_entry_t *entry;
	const char *r;
	if (pThis->nmemb == 0) {
		entry = NULL;
	} else {
		entry = bsearch_lte(&key.k_uint, pThis->table.sprsArr->entries, pThis->nmemb,
			sizeof(lookup_sparseArray_tab_entry_t), bs_arrcmp_sprsArrtab);
	}

	if(entry == NULL) {
		r = defaultVal(pThis);
	} else {
		r = (const char*)entry->interned_val_ref;
	}
	return es_newStrFromCStr(r, strlen(r));
}

/* builders for different table-types */

#define NO_INDEX_ERROR(type, name)				\
	LogError(0, RS_RET_INVALID_VALUE, "'%s' lookup table named: '%s' has record(s) without 'index' "\
"field", type, name); \
	ABORT_FINALIZE(RS_RET_INVALID_VALUE);

static rsRetVal
build_StringTable(lookup_t *pThis, struct json_object *jtab, const uchar* name) {
	uint32_t i;
	struct json_object *jrow, *jindex, *jvalue;
	uchar *value, *canonicalValueRef;
	DEFiRet;

	pThis->table.str = NULL;
	CHKmalloc(pThis->table.str = calloc(1, sizeof(lookup_string_tab_t)));
	if (pThis->nmemb > 0) {
		CHKmalloc(pThis->table.str->entries = calloc(pThis->nmemb, sizeof(lookup_string_tab_entry_t)));

		for(i = 0; i < pThis->nmemb; i++) {
			jrow = json_object_array_get_idx(jtab, i);
			jindex = json_object_object_get(jrow, "index");
			jvalue = json_object_object_get(jrow, "value");
			if (jindex == NULL || json_object_is_type(jindex, json_type_null)) {
				NO_INDEX_ERROR("string", name);
			}
			CHKmalloc(pThis->table.str->entries[i].key = ustrdup((uchar*) json_object_get_string(jindex)));
			value = (uchar*) json_object_get_string(jvalue);
			uchar **found  = (uchar**) bsearch(value, pThis->interned_vals,
				pThis->interned_val_count, sizeof(uchar*), bs_arrcmp_str);
			if(found == NULL) {
				LogError(0, RS_RET_INTERNAL_ERROR, "lookup.c:build_StringTable(): "
					"internal error, bsearch returned NULL for '%s'", value);
				ABORT_FINALIZE(RS_RET_INTERNAL_ERROR);
			}
			// I give up, I see no way to remove false positive -- rgerhards, 2017-10-24
			#ifndef __clang_analyzer__
			canonicalValueRef = *found;
			if(canonicalValueRef == NULL) {
				LogError(0, RS_RET_INTERNAL_ERROR, "lookup.c:build_StringTable(): "
					"internal error, canonicalValueRef returned from bsearch "
					"is NULL for '%s'", value);
				ABORT_FINALIZE(RS_RET_INTERNAL_ERROR);
			}
			pThis->table.str->entries[i].interned_val_ref = canonicalValueRef;
			#endif
		}
		qsort(pThis->table.str->entries, pThis->nmemb, sizeof(lookup_string_tab_entry_t), qs_arrcmp_strtab);
	}

	pThis->lookup = lookupKey_str;
	pThis->key_type = LOOKUP_KEY_TYPE_STRING;
finalize_it:
	RETiRet;
}

static rsRetVal
build_ArrayTable(lookup_t *pThis, struct json_object *jtab, const uchar *name) {
	uint32_t i;
	struct json_object *jrow, *jindex, *jvalue;
	uchar *canonicalValueRef;
	uint32_t prev_index, index;
	uint8_t prev_index_set;
	uint32_index_val_t *indexes = NULL;
	DEFiRet;

	prev_index_set = 0;

	pThis->table.arr = NULL;
	CHKmalloc(pThis->table.arr = calloc(1, sizeof(lookup_array_tab_t)));
	if (pThis->nmemb > 0) {
		CHKmalloc(indexes = calloc(pThis->nmemb, sizeof(uint32_index_val_t)));
		CHKmalloc(pThis->table.arr->interned_val_refs = calloc(pThis->nmemb, sizeof(uchar*)));

		for(i = 0; i < pThis->nmemb; i++) {
			jrow = json_object_array_get_idx(jtab, i);
			jindex = json_object_object_get(jrow, "index");
			jvalue = json_object_object_get(jrow, "value");
			if (jindex == NULL || json_object_is_type(jindex, json_type_null)) {
				NO_INDEX_ERROR("array", name);
			}
			indexes[i].index = (uint32_t) json_object_get_int(jindex);
			indexes[i].val = (uchar*) json_object_get_string(jvalue);
		}
		qsort(indexes, pThis->nmemb, sizeof(uint32_index_val_t), qs_arrcmp_uint32_index_val);
		for(i = 0; i < pThis->nmemb; i++) {
			index = indexes[i].index;
			if (prev_index_set == 0) {
				prev_index = index;
				prev_index_set = 1;
				pThis->table.arr->first_key = index;
			} else {
				if (index != ++prev_index) {
					LogError(0, RS_RET_INVALID_VALUE, "'array' lookup table name: '%s' "
					"has non-contiguous members between index '%d' and '%d'",
									name, prev_index, index);
					ABORT_FINALIZE(RS_RET_INVALID_VALUE);
				}
			}
			uchar *const *const canonicalValueRef_ptr = bsearch(indexes[i].val, pThis->interned_vals,
				pThis->interned_val_count, sizeof(uchar*), bs_arrcmp_str);
			if(canonicalValueRef_ptr == NULL) {
				LogError(0, RS_RET_ERR, "BUG: canonicalValueRef not found in "
					"build_ArrayTable(), %s:%d", __FILE__, __LINE__);
				ABORT_FINALIZE(RS_RET_ERR);
			}
			canonicalValueRef = *canonicalValueRef_ptr;
			assert(canonicalValueRef != NULL);
			pThis->table.arr->interned_val_refs[i] = canonicalValueRef;
		}
	}

	pThis->lookup = lookupKey_arr;
	pThis->key_type = LOOKUP_KEY_TYPE_UINT;

finalize_it:
	free(indexes);
	RETiRet;
}

static rsRetVal
build_SparseArrayTable(lookup_t *pThis, struct json_object *jtab, const uchar* name) {
	uint32_t i;
	struct json_object *jrow, *jindex, *jvalue;
	uchar *value, *canonicalValueRef;
	DEFiRet;

	pThis->table.str = NULL;
	CHKmalloc(pThis->table.sprsArr = calloc(1, sizeof(lookup_sparseArray_tab_t)));
	if (pThis->nmemb > 0) {
		CHKmalloc(pThis->table.sprsArr->entries = calloc(pThis->nmemb, sizeof(lookup_sparseArray_tab_entry_t)));

		for(i = 0; i < pThis->nmemb; i++) {
			jrow = json_object_array_get_idx(jtab, i);
			jindex = json_object_object_get(jrow, "index");
			jvalue = json_object_object_get(jrow, "value");
			if (jindex == NULL || json_object_is_type(jindex, json_type_null)) {
				NO_INDEX_ERROR("sparseArray", name);
			}
			pThis->table.sprsArr->entries[i].key = (uint32_t) json_object_get_int(jindex);
			value = (uchar*) json_object_get_string(jvalue);
			uchar *const *const canonicalValueRef_ptr = bsearch(value, pThis->interned_vals,
				pThis->interned_val_count, sizeof(uchar*), bs_arrcmp_str);
			if(canonicalValueRef_ptr == NULL) {
				LogError(0, RS_RET_ERR, "BUG: canonicalValueRef not found in "
					"build_SparseArrayTable(), %s:%d", __FILE__, __LINE__);
				ABORT_FINALIZE(RS_RET_ERR);
			}
			canonicalValueRef = *canonicalValueRef_ptr;
			assert(canonicalValueRef != NULL);
			pThis->table.sprsArr->entries[i].interned_val_ref = canonicalValueRef;
		}
		qsort(pThis->table.sprsArr->entries, pThis->nmemb, sizeof(lookup_sparseArray_tab_entry_t),
				qs_arrcmp_sprsArrtab);
	}

	pThis->lookup = lookupKey_sprsArr;
	pThis->key_type = LOOKUP_KEY_TYPE_UINT;

finalize_it:
	RETiRet;
}

static rsRetVal
lookupBuildStubbedTable(lookup_t *pThis, const uchar* stub_val) {
	DEFiRet;

	CHKmalloc(pThis->nomatch = ustrdup(stub_val));
	pThis->lookup = lookupKey_stub;
	pThis->type = STUBBED_LOOKUP_TABLE;
	pThis->key_type = LOOKUP_KEY_TYPE_NONE;

finalize_it:
	RETiRet;
}

static rsRetVal
lookupBuildTable_v1(lookup_t *pThis, struct json_object *jroot, const uchar* name) {
	struct json_object *jnomatch, *jtype, *jtab;
	struct json_object *jrow, *jvalue;
	const char *table_type, *nomatch_value;
	const uchar **all_values;
	const uchar *curr, *prev;
	uint32_t i, j;
	uint32_t uniq_values;

	DEFiRet;
	all_values = NULL;

	jnomatch = json_object_object_get(jroot, "nomatch");
	jtype = json_object_object_get(jroot, "type");
	jtab = json_object_object_get(jroot, "table");
	if (jtab == NULL || !json_object_is_type(jtab, json_type_array)) {
		LogError(0, RS_RET_INVALID_VALUE, "lookup table named: '%s' has invalid table definition", name);
		ABORT_FINALIZE(RS_RET_INVALID_VALUE);
	}
	pThis->nmemb = json_object_array_length(jtab);
	table_type = json_object_get_string(jtype);
	if (table_type == NULL) {
		table_type = "string";
	}

	CHKmalloc(all_values = malloc(pThis->nmemb * sizeof(uchar*)));

	/* before actual table can be loaded, prepare all-value list and remove duplicates*/
	for(i = 0; i < pThis->nmemb; i++) {
		jrow = json_object_array_get_idx(jtab, i);
		jvalue = json_object_object_get(jrow, "value");
		if (jvalue == NULL || json_object_is_type(jvalue, json_type_null)) {
			LogError(0, RS_RET_INVALID_VALUE, "'%s' lookup table named: '%s' has record(s) "
			"without 'value' field", table_type, name);
			ABORT_FINALIZE(RS_RET_INVALID_VALUE);
		}
		all_values[i] = (const uchar*) json_object_get_string(jvalue);
	}
	qsort(all_values, pThis->nmemb, sizeof(uchar*), qs_arrcmp_ustrs);
	uniq_values = 1;
	for(i = 1; i < pThis->nmemb; i++) {
		curr = all_values[i];
		prev = all_values[i - 1];
		if (ustrcmp(prev, curr) != 0) {
			uniq_values++;
		}
	}

	if (pThis->nmemb > 0)  {
		CHKmalloc(pThis->interned_vals = malloc(uniq_values * sizeof(uchar*)));
		j = 0;
		CHKmalloc(pThis->interned_vals[j++] = ustrdup(all_values[0]));
		for(i = 1; i < pThis->nmemb ; ++i) {
			curr = all_values[i];
			prev = all_values[i - 1];
			if (ustrcmp(prev, curr) != 0) {
				CHKmalloc(pThis->interned_vals[j++] = ustrdup(all_values[i]));
			}
		}
		pThis->interned_val_count = uniq_values;
	}
	/* uniq values captured (sorted) */

	nomatch_value = json_object_get_string(jnomatch);
	if (nomatch_value != NULL) {
		CHKmalloc(pThis->nomatch = (uchar*) strdup(nomatch_value));
	}

	if (strcmp(table_type, "array") == 0) {
		pThis->type = ARRAY_LOOKUP_TABLE;
		CHKiRet(build_ArrayTable(pThis, jtab, name));
	} else if (strcmp(table_type, "sparseArray") == 0) {
		pThis->type = SPARSE_ARRAY_LOOKUP_TABLE;
		CHKiRet(build_SparseArrayTable(pThis, jtab, name));
	} else if (strcmp(table_type, "string") == 0) {
		pThis->type = STRING_LOOKUP_TABLE;
		CHKiRet(build_StringTable(pThis, jtab, name));
	} else {
		LogError(0, RS_RET_INVALID_VALUE, "lookup table named: '%s' uses unupported "
				"type: '%s'", name, table_type);
		ABORT_FINALIZE(RS_RET_INVALID_VALUE);
	}
finalize_it:
	if (all_values != NULL) free(all_values);
	RETiRet;
}

static rsRetVal
lookupBuildTable(lookup_t *pThis, struct json_object *jroot, const uchar* name)
{
	struct json_object *jversion;
	int version = 1;

	DEFiRet;

	jversion = json_object_object_get(jroot, "version");
	if (jversion != NULL && !json_object_is_type(jversion, json_type_null)) {
		version = json_object_get_int(jversion);
	} else {
		LogError(0, RS_RET_INVALID_VALUE, "lookup table named: '%s' doesn't specify version "
		"(will use default value: %d)", name, version);
	}
	if (version == 1) {
		CHKiRet(lookupBuildTable_v1(pThis, jroot, name));
	} else {
		LogError(0, RS_RET_INVALID_VALUE, "lookup table named: '%s' uses unsupported "
				"version: %d", name, version);
		ABORT_FINALIZE(RS_RET_INVALID_VALUE);
	}

finalize_it:
	RETiRet;
}


/* find a lookup table. This is a naive O(n) algo, but this really
 * doesn't matter as it is called only a few times during config
 * load. The function returns either a pointer to the requested
 * table or NULL, if not found.
 */
lookup_ref_t * ATTR_NONNULL()
lookupFindTable(uchar *name)
{
	lookup_ref_t *curr;

	for(curr = loadConf->lu_tabs.root ; curr != NULL ; curr = curr->next) {
		if(!ustrcmp(curr->name, name))
			break;
	}
	return curr;
}


/* this reloads a lookup table. This is done while the engine is running,
 * as such the function must ensure proper locking and proper order of
 * operations (so that nothing can interfere). If the table cannot be loaded,
 * the old table is continued to be used.
 */
static rsRetVal
lookupReloadOrStub(lookup_ref_t *pThis, const uchar* stub_val) {
	lookup_t *newlu, *oldlu; /* dummy to be able to use support functions without
								affecting current settings. */
	DEFiRet;

	oldlu = pThis->self;
	newlu = NULL;

	DBGPRINTF("reload requested for lookup table '%s'\n", pThis->name);
	CHKmalloc(newlu = calloc(1, sizeof(lookup_t)));
	if (stub_val == NULL) {
		CHKiRet(lookupReadFile(newlu, pThis->name, pThis->filename));
	} else {
		CHKiRet(lookupBuildStubbedTable(newlu, stub_val));
	}
	/* all went well, copy over data members */
	pthread_rwlock_wrlock(&pThis->rwlock);
	pThis->self = newlu;
	pthread_rwlock_unlock(&pThis->rwlock);
finalize_it:
	if (iRet != RS_RET_OK) {
		if (stub_val == NULL) {
			LogError(0, RS_RET_INTERNAL_ERROR,
					"lookup table '%s' could not be reloaded from file '%s'",
					pThis->name, pThis->filename);
		} else {
			LogError(0, RS_RET_INTERNAL_ERROR,
					"lookup table '%s' could not be stubbed with value '%s'",
					pThis->name, stub_val);
		}
		lookupDestruct(newlu);
	} else {
		if (stub_val == NULL) {
			LogMsg(0, RS_RET_OK, LOG_INFO, "lookup table '%s' reloaded from file '%s'",
					pThis->name, pThis->filename);
		} else {
			LogError(0, RS_RET_OK, "lookup table '%s' stubbed with value '%s'",
					pThis->name, stub_val);
		}
		lookupDestruct(oldlu);
	}
	RETiRet;
}

static rsRetVal
lookupDoStub(lookup_ref_t *pThis, const uchar* stub_val)
{
	int already_stubbed = 0;
	DEFiRet;
	pthread_rwlock_rdlock(&pThis->rwlock);
	if (pThis->self->type == STUBBED_LOOKUP_TABLE &&
		ustrcmp(pThis->self->nomatch, stub_val) == 0)
		already_stubbed = 1;
	pthread_rwlock_unlock(&pThis->rwlock);
	if (! already_stubbed) {
		LogError(0, RS_RET_OK, "stubbing lookup table '%s' with value '%s'",
			pThis->name, stub_val);
		CHKiRet(lookupReloadOrStub(pThis, stub_val));
	} else {
		LogError(0, RS_RET_OK, "lookup table '%s' is already stubbed with value '%s'",
			pThis->name, stub_val);
	}
finalize_it:
	RETiRet;
}

static uint8_t
lookupIsReloadPending(lookup_ref_t *pThis) {
	uint8_t reload_pending;
	pthread_mutex_lock(&pThis->reloader_mut);
	reload_pending = pThis->do_reload;
	pthread_mutex_unlock(&pThis->reloader_mut);
	return reload_pending;
}

/* note: stub_val_if_reload_fails may or may not be NULL */
rsRetVal ATTR_NONNULL(1)
lookupReload(lookup_ref_t *const pThis, const uchar *const stub_val_if_reload_fails)
{
	uint8_t locked = 0;
	int lock_errno = 0;
	DEFiRet;
	assert(pThis != NULL);
	if ((lock_errno = pthread_mutex_trylock(&pThis->reloader_mut)) == 0) {
		locked = 1;
		/*so it doesn't leak memory in situation where 2 reload requests are issued back to back*/
		freeStubValueForReloadFailure(pThis);
		if (stub_val_if_reload_fails != NULL) {
			CHKmalloc(pThis->stub_value_for_reload_failure = ustrdup(stub_val_if_reload_fails));
		}
		pThis->do_reload = 1;
		pthread_cond_signal(&pThis->run_reloader);
	} else {
		LogError(lock_errno, RS_RET_INTERNAL_ERROR, "attempt to trigger "
			"reload of lookup table '%s' failed (not stubbing)", pThis->name);
		ABORT_FINALIZE(RS_RET_INTERNAL_ERROR);
		/* we can choose to stub the table here, but it'll hurt because
		   the table reloader may take time to complete the reload
		   and stubbing because of a concurrent reload message may
		   not be desirable (except in very tightly controled environments
		   where reload-triggering messages pushed are timed accurately
		   and an idempotency-filter is used to reject re-deliveries) */
	}
finalize_it:
	if (locked) {
		pthread_mutex_unlock(&pThis->reloader_mut);
	}
	RETiRet;
}

static rsRetVal ATTR_NONNULL()
lookupDoReload(lookup_ref_t *pThis)
{
	DEFiRet;
	iRet = lookupReloadOrStub(pThis, NULL);
	if ((iRet != RS_RET_OK) &&
		(pThis->stub_value_for_reload_failure != NULL)) {
		iRet = lookupDoStub(pThis, pThis->stub_value_for_reload_failure);
	}
	freeStubValueForReloadFailure(pThis);
	RETiRet;
}

void *
lookupTableReloader(void *self)
{
	lookup_ref_t *pThis = (lookup_ref_t*) self;
	pthread_mutex_lock(&pThis->reloader_mut);
	while(1) {
		if (pThis->do_stop) {
			break;
		} else if (pThis->do_reload) {
			pThis->do_reload = 0;
			lookupDoReload(pThis);
		} else {
			pthread_cond_wait(&pThis->run_reloader, &pThis->reloader_mut);
		}
	}
	pthread_mutex_unlock(&pThis->reloader_mut);
	return NULL;
}

/* reload all lookup tables on HUP */
void
lookupDoHUP(void)
{
	lookup_ref_t *luref;
	for(luref = loadConf->lu_tabs.root ; luref != NULL ; luref = luref->next) {
		if (luref->reload_on_hup) {
			lookupReload(luref, NULL);
		}
	}
}

uint
lookupPendingReloadCount(void)
{
	uint pending_reload_count = 0;
	lookup_ref_t *luref;
	for(luref = loadConf->lu_tabs.root ; luref != NULL ; luref = luref->next) {
		if (lookupIsReloadPending(luref)) {
			pending_reload_count++;
		}
	}
	return pending_reload_count;
}


/* returns either a pointer to the value (read only!) or NULL
 * if either the key could not be found or an error occured.
 * Note that an estr_t object is returned. The caller is
 * responsible for freeing it.
 */
es_str_t *
lookupKey(lookup_ref_t *pThis, lookup_key_t key)
{
	es_str_t *estr;
	lookup_t *t;
	pthread_rwlock_rdlock(&pThis->rwlock);
	t = pThis->self;
	estr = t->lookup(t, key);
	pthread_rwlock_unlock(&pThis->rwlock);
	return estr;
}


/* note: widely-deployed json_c 0.9 does NOT support incremental
 * parsing. In order to keep compatible with e.g. Ubuntu 12.04LTS,
 * we read the file into one big memory buffer and parse it at once.
 * While this is not very elegant, it will not pose any real issue
 * for "reasonable" lookup tables (and "unreasonably" large ones
 * will probably have other issues as well...).
 */
static rsRetVal ATTR_NONNULL()
lookupReadFile(lookup_t *const pThis, const uchar *const name, const uchar *const filename)
{
	struct json_tokener *tokener = NULL;
	struct json_object *json = NULL;
	char *iobuf = NULL;
	int fd = -1;
	ssize_t nread;
	struct stat sb;
	DEFiRet;


	if((fd = open((const char*) filename, O_RDONLY)) == -1) {
		LogError(errno, RS_RET_FILE_NOT_FOUND,
			"lookup table file '%s' could not be opened", filename);
		ABORT_FINALIZE(RS_RET_FILE_NOT_FOUND);
	}

	if(fstat(fd, &sb) == -1) {
		LogError(errno, RS_RET_FILE_NOT_FOUND,
			"lookup table file '%s' stat failed", filename);
		ABORT_FINALIZE(RS_RET_FILE_NOT_FOUND);
	}

	CHKmalloc(iobuf = malloc(sb.st_size));

	tokener = json_tokener_new();
	nread = read(fd, iobuf, sb.st_size);
	if(nread != (ssize_t) sb.st_size) {
		LogError(errno, RS_RET_READ_ERR,
			"lookup table file '%s' read error", filename);
		ABORT_FINALIZE(RS_RET_READ_ERR);
	}

	json = json_tokener_parse_ex(tokener, iobuf, sb.st_size);
	if(json == NULL) {
		LogError(0, RS_RET_JSON_PARSE_ERR,
			"lookup table file '%s' json parsing error",
			filename);
		ABORT_FINALIZE(RS_RET_JSON_PARSE_ERR);
	}
	free(iobuf); /* early free to sever resources*/
	iobuf = NULL; /* make sure no double-free */

	/* got json object, now populate our own in-memory structure */
	CHKiRet(lookupBuildTable(pThis, json, name));

finalize_it:
	if (fd != -1) {
		close(fd);
	}
	free(iobuf);
	if(tokener != NULL)
		json_tokener_free(tokener);
	if(json != NULL)
		json_object_put(json);
	RETiRet;
}


rsRetVal
lookupTableDefProcessCnf(struct cnfobj *o)
{
	struct cnfparamvals *pvals;
	lookup_ref_t *lu;
	short i;
#ifdef HAVE_PTHREAD_SETNAME_NP
	char *reloader_thd_name = NULL;
	int thd_name_len = 0;
#endif
	DEFiRet;
	lu = NULL;

	pvals = nvlstGetParams(o->nvlst, &modpblk, NULL);
	if(pvals == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}
	DBGPRINTF("lookupTableDefProcessCnf params:\n");
	cnfparamsPrint(&modpblk, pvals);

	CHKiRet(lookupNew(&lu));

	for(i = 0 ; i < modpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(modpblk.descr[i].name, "file")) {
			CHKmalloc(lu->filename = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL));
		} else if(!strcmp(modpblk.descr[i].name, "name")) {
			CHKmalloc(lu->name = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL));
		} else if(!strcmp(modpblk.descr[i].name, "reloadOnHUP")) {
			lu->reload_on_hup = (pvals[i].val.d.n != 0);
		} else {
			dbgprintf("lookup_table: program error, non-handled "
			  "param '%s'\n", modpblk.descr[i].name);
		}
	}
#ifdef HAVE_PTHREAD_SETNAME_NP
	thd_name_len = ustrlen(lu->name) + strlen(reloader_prefix) + 1;
	CHKmalloc(reloader_thd_name = malloc(thd_name_len));
	strcpy(reloader_thd_name, reloader_prefix);
	strcpy(reloader_thd_name + strlen(reloader_prefix), (char*) lu->name);
	reloader_thd_name[thd_name_len - 1] = '\0';
#if defined(__NetBSD__)
	pthread_setname_np(lu->reloader, "%s", reloader_thd_name);
#elif defined(__APPLE__)
	pthread_setname_np(reloader_thd_name); // must check
#else
	pthread_setname_np(lu->reloader, reloader_thd_name);
#endif
#endif
	CHKiRet(lookupReadFile(lu->self, lu->name, lu->filename));
	DBGPRINTF("lookup table '%s' loaded from file '%s'\n", lu->name, lu->filename);

finalize_it:
#ifdef HAVE_PTHREAD_SETNAME_NP
	free(reloader_thd_name);
#endif
	cnfparamvalsDestruct(pvals, &modpblk);
	if (iRet != RS_RET_OK) {
		if (lu != NULL) {
			lookupDestruct(lu->self);
			lu->self = NULL;
		}
	}
	RETiRet;
}

void
lookupClassExit(void)
{
	objRelease(glbl, CORE_COMPONENT);
}

rsRetVal
lookupClassInit(void)
{
	DEFiRet;
	CHKiRet(objGetObjInterface(&obj));
	CHKiRet(objUse(glbl, CORE_COMPONENT));
finalize_it:
	RETiRet;
}
