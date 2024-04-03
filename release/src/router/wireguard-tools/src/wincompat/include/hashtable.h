/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2018-2021 WireGuard LLC. All Rights Reserved.
 */

#ifndef _HASHTABLE_H
#define _HASHTABLE_H

#include <string.h>

enum { HASHTABLE_ENTRY_BUCKETS_POW2 = 1 << 10 };

struct hashtable_entry {
	char *key;
	void *value;
	struct hashtable_entry *next;
};

struct hashtable {
	struct hashtable_entry *entry_buckets[HASHTABLE_ENTRY_BUCKETS_POW2];
};

static unsigned int hashtable_bucket(const char *str)
{
	unsigned long hash = 5381;
	char c;
	while ((c = *str++))
		hash = ((hash << 5) + hash) ^ c;
	return hash & (HASHTABLE_ENTRY_BUCKETS_POW2 - 1);
}

static struct hashtable_entry *hashtable_find_entry(struct hashtable *hashtable, const char *key)
{
	struct hashtable_entry *entry;
	for (entry = hashtable->entry_buckets[hashtable_bucket(key)]; entry; entry = entry->next) {
		if (!strcmp(entry->key, key))
			return entry;
	}
	return NULL;
}

static struct hashtable_entry *hashtable_find_or_insert_entry(struct hashtable *hashtable, const char *key)
{
	struct hashtable_entry **entry;
	for (entry = &hashtable->entry_buckets[hashtable_bucket(key)]; *entry; entry = &(*entry)->next) {
		if (!strcmp((*entry)->key, key))
			return *entry;
	}
	*entry = calloc(1, sizeof(**entry));
	if (!*entry)
		return NULL;
	(*entry)->key = strdup(key);
	if (!(*entry)->key) {
		free(*entry);
		*entry = NULL;
		return NULL;
	}
	return *entry;
}

#endif
