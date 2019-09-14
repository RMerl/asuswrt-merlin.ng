/* header for lookup.c
 *
 * Copyright 2013 Adiscon GmbH.
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
#ifndef INCLUDED_LOOKUP_H
#define INCLUDED_LOOKUP_H
#include <libestr.h>

#define STRING_LOOKUP_TABLE 1
#define ARRAY_LOOKUP_TABLE 2
#define SPARSE_ARRAY_LOOKUP_TABLE 3
#define STUBBED_LOOKUP_TABLE 4

#define LOOKUP_KEY_TYPE_STRING 1
#define LOOKUP_KEY_TYPE_UINT 2
#define LOOKUP_KEY_TYPE_NONE 3

struct lookup_tables_s {
	lookup_ref_t *root;	/* the root of the template list */
	lookup_ref_t *last;	/* points to the last element of the template list */
};

struct lookup_array_tab_s {
	uint32_t first_key;
	uchar **interned_val_refs;
};

struct lookup_sparseArray_tab_entry_s {
	uint32_t key;
	uchar *interned_val_ref;
};

struct lookup_sparseArray_tab_s {
	lookup_sparseArray_tab_entry_t *entries;
};

struct lookup_string_tab_entry_s {
	uchar *key;
	uchar *interned_val_ref;
};

struct lookup_string_tab_s {
	lookup_string_tab_entry_t *entries;
};

struct lookup_ref_s {
	pthread_rwlock_t rwlock;	/* protect us in case of dynamic reloads */
	uchar *name;
	uchar *filename;
	lookup_t *self;
	lookup_ref_t *next;
	/* reload specific attributes */
	pthread_mutex_t reloader_mut; /* signaling + access to reload-flow variables*/
	/* rwlock(above) may be acquired inside critical-section reloader_mut guards */
	pthread_cond_t run_reloader;
	pthread_t reloader;
	pthread_attr_t reloader_thd_attr;
	uchar *stub_value_for_reload_failure;
	uint8_t do_reload;
	uint8_t do_stop;
	uint8_t reload_on_hup;
};

typedef es_str_t* (lookup_fn_t)(lookup_t*, lookup_key_t);

/* a single lookup table */
struct lookup_s {
	uint32_t nmemb;
	uint8_t type;
	uint8_t key_type;
	union {
		lookup_string_tab_t *str;
		lookup_array_tab_t *arr;
		lookup_sparseArray_tab_t *sprsArr;
	} table;
	uint32_t interned_val_count;
	uchar **interned_vals;
	uchar *nomatch;
	lookup_fn_t *lookup;
};

union lookup_key_u {
	uchar* k_str;
	uint32_t k_uint;
};

/* prototypes */
void lookupInitCnf(lookup_tables_t *lu_tabs);
rsRetVal lookupTableDefProcessCnf(struct cnfobj *o);
lookup_ref_t *lookupFindTable(uchar *name);
es_str_t * lookupKey(lookup_ref_t *pThis, lookup_key_t key);
void lookupDestroyCnf(void);
void lookupClassExit(void);
void lookupDoHUP(void);
rsRetVal lookupReload(lookup_ref_t *pThis, const uchar *stub_value_if_reload_fails);
uint lookupPendingReloadCount(void);
rsRetVal lookupClassInit(void);

#endif /* #ifndef INCLUDED_LOOKUP_H */
