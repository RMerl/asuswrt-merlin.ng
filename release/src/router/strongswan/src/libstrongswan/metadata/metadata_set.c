/*
 * Copyright (C) 2021 Tobias Brunner, codelabs GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "metadata_set.h"

#include <collections/array.h>

/**
 * Set of metadata objects, indexed via string.
 */
struct metadata_set_t {
	/** Stored metadata objects (entry_t) */
	array_t *entries;
};

/**
 * Stored data for each metadata object.
 */
typedef struct {
	/** Key of the entry */
	const char *key;
	/** Stored metadata object */
	metadata_t *data;
} entry_t;

/**
 * Destroy the given entry.
 */
static void destroy_entry(entry_t *entry)
{
	entry->data->destroy(entry->data);
	free((char*)entry->key);
	free(entry);
}

/**
 * Sort entries by key
 */
static int entry_sort(const void *a, const void *b, void *user)
{
	const entry_t *ea = a, *eb = b;
	return strcmp(ea->key, eb->key);
}

/**
 * Find an entry by key
 */
static int entry_find(const void *a, const void *b)
{
	return entry_sort(a, b, NULL);
}

/*
 * Described in header
 */
metadata_set_t *metadata_set_create()
{
	metadata_set_t *set;

	INIT(set);

	return set;
}

/*
 * Described in header
 */
void metadata_set_put(metadata_set_t *set, const char *key, metadata_t *data)
{
	entry_t *entry = NULL, lookup = {
		.key = key,
	};
	int idx;

	if (!set)
	{
		DESTROY_IF(data);
		return;
	}

	idx = array_bsearch(set->entries, &lookup, entry_find, &entry);
	if (idx != -1)
	{
		if (data)
		{
			entry->data->destroy(entry->data);
			entry->data = data;
		}
		else
		{
			array_remove(set->entries, idx, NULL);
			destroy_entry(entry);
		}
	}
	else if (data)
	{
		INIT(entry,
			.key = strdup(key),
			.data = data,
		);
		array_insert_create(&set->entries, ARRAY_TAIL, entry);
		array_sort(set->entries, entry_sort, NULL);
	}
}

/*
 * Described in header
 */
metadata_t *metadata_set_get(metadata_set_t *set, const char *key)
{
	entry_t *entry = NULL, lookup = {
		.key = key,
	};

	if (set && array_bsearch(set->entries, &lookup, entry_find, &entry) != -1)
	{
		return entry->data;
	}
	return NULL;
}

/*
 * Described in header
 */
metadata_set_t *metadata_set_clone(metadata_set_t *set)
{
	metadata_set_t *clone;
	entry_t *entry, *entry_clone;
	int i;

	if (!set)
	{
		return NULL;
	}

	INIT(clone,
		.entries = array_create(0, array_count(set->entries)),
	);
	for (i = 0; i < array_count(set->entries); i++)
	{
		array_get(set->entries, i, &entry);
		INIT(entry_clone,
			.key = strdup(entry->key),
			.data = entry->data->clone(entry->data),
		);
		array_insert(clone->entries, i, entry_clone);
	}
	return clone;
}

/*
 * Described in header
 */
void metadata_set_destroy(metadata_set_t *set)
{
	if (set)
	{
		array_destroy_function(set->entries, (void*)destroy_entry, NULL);
		free(set);
	}
}
