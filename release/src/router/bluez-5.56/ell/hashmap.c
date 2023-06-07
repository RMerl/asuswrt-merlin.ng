/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2011-2014  Intel Corporation. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "util.h"
#include "hashmap.h"
#include "private.h"

/**
 * SECTION:hashmap
 * @short_description: Hash table support
 *
 * Hash table support
 */

#define NBUCKETS 127

struct entry {
	void *key;
	void *value;
	struct entry *next;
	unsigned int hash;
};

/**
 * l_hashmap:
 *
 * Opague object representing the hash table.
 */
struct l_hashmap {
	l_hashmap_hash_func_t hash_func;
	l_hashmap_compare_func_t compare_func;
	l_hashmap_key_new_func_t key_new_func;
	l_hashmap_key_free_func_t key_free_func;
	unsigned int entries;
	struct entry buckets[NBUCKETS];
};

static inline void *get_key_new(const struct l_hashmap *hashmap,
				const void *key)
{
	if (hashmap->key_new_func)
		return hashmap->key_new_func(key);

	return (void *)key;
}

static inline void free_key(const struct l_hashmap *hashmap, void *key)
{
	if (hashmap->key_free_func)
		hashmap->key_free_func(key);
}

static inline unsigned int hash_superfast(const uint8_t *key, unsigned int len)
{
	/*
	 * Paul Hsieh (http://www.azillionmonkeys.com/qed/hash.html)
	 * used by WebCore (http://webkit.org/blog/8/hashtables-part-2/),
	 * EFL's eina, kmod and possible others.
	 */
	unsigned int tmp, hash = len, rem = len & 3;

	len /= 4;

	/* Main loop */
	for (; len > 0; len--) {
		hash += l_get_u16(key);
		tmp = (l_get_u16(key + 2) << 11) ^ hash;
		hash = (hash << 16) ^ tmp;
		key += 4;
		hash += hash >> 11;
	}

	/* Handle end cases */
	switch (rem) {
	case 3:
		hash += l_get_u16(key);
		hash ^= hash << 16;
		hash ^= key[2] << 18;
		hash += hash >> 11;
		break;

	case 2:
		hash += l_get_u16(key);
		hash ^= hash << 11;
		hash += hash >> 17;
		break;

	case 1:
		hash += *key;
		hash ^= hash << 10;
		hash += hash >> 1;
		break;
	}

	/* Force "avalanching" of final 127 bits */
	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 4;
	hash += hash >> 17;
	hash ^= hash << 25;
	hash += hash >> 6;

	return hash;
}

static unsigned int direct_hash_func(const void *p)
{
	return L_PTR_TO_UINT(p);
}

static int direct_compare_func(const void *a, const void *b)
{
        return a < b ? -1 : (a > b ? 1 : 0);
}

/**
 * l_hashmap_new:
 *
 * Create a new hash table. The keys are managed as pointers, that is,
 * the pointer value is hashed and looked up.
 *
 * No error handling is needed since. In case of real memory allocation
 * problems abort() will be called.
 *
 * See also l_hashmap_string_new().
 *
 * Returns: a newly allocated #l_hashmap object
 **/
LIB_EXPORT struct l_hashmap *l_hashmap_new(void)
{
	struct l_hashmap *hashmap;

	hashmap = l_new(struct l_hashmap, 1);

	hashmap->hash_func = direct_hash_func;
	hashmap->compare_func = direct_compare_func;
	hashmap->entries = 0;

	return hashmap;
}

LIB_EXPORT unsigned int l_str_hash(const void *p)
{
	const char *s = p;
	size_t len = strlen(s);

	return hash_superfast((const uint8_t *)s, len);
}

/**
 * l_hashmap_string_new:
 *
 * Create a new hash table. The keys are considered strings and are
 * copied.
 *
 * No error handling is needed since. In case of real memory allocation
 * problems abort() will be called.
 *
 * See also l_hashmap_new().
 *
 * Returns: a newly allocated #l_hashmap object
 **/
LIB_EXPORT struct l_hashmap *l_hashmap_string_new(void)
{
	struct l_hashmap *hashmap;

	hashmap = l_new(struct l_hashmap, 1);

	hashmap->hash_func = l_str_hash;
	hashmap->compare_func = (l_hashmap_compare_func_t) strcmp;
	hashmap->key_new_func = (l_hashmap_key_new_func_t) l_strdup;
	hashmap->key_free_func = l_free;
	hashmap->entries = 0;

	return hashmap;
}

/**
 * l_hashmap_set_hash_function:
 * @hashmap: hash table object
 * @func: Key hashing function
 *
 * Sets the hashing function to be used by this object.
 *
 * This function can only be called when the @hashmap is empty.
 *
 * Returns: #true when the hashing function could be updated successfully,
 * and #false otherwise.
 **/
LIB_EXPORT bool l_hashmap_set_hash_function(struct l_hashmap *hashmap,
						l_hashmap_hash_func_t func)
{
	if (unlikely(!hashmap))
		return false;

	if (hashmap->entries != 0)
		return false;

	hashmap->hash_func = func;

	return true;
}

/**
 * l_hashmap_set_compare_function:
 * @hashmap: hash table object
 * @func: Key compare function
 *
 * Sets the key comparison function to be used by this object.
 *
 * This function can only be called when the @hashmap is empty.
 *
 * Returns: #true when the comparison function could be updated successfully,
 * and #false otherwise.
 **/
LIB_EXPORT bool l_hashmap_set_compare_function(struct l_hashmap *hashmap,
						l_hashmap_compare_func_t func)
{
	if (unlikely(!hashmap))
		return false;

	if (hashmap->entries != 0)
		return false;

	hashmap->compare_func = func;

	return true;
}

/**
 * l_hashmap_set_key_copy_function:
 * @hashmap: hash table object
 * @func: Key duplication function
 *
 * Sets the key duplication function to be used by this object.  If the
 * function is NULL, then the keys are assigned directly.
 *
 * This function can only be called when the @hashmap is empty.
 *
 * Returns: #true when the key copy function could be updated successfully,
 * and #false otherwise.
 **/
LIB_EXPORT bool l_hashmap_set_key_copy_function(struct l_hashmap *hashmap,
						l_hashmap_key_new_func_t func)
{
	if (unlikely(!hashmap))
		return false;

	if (hashmap->entries != 0)
		return false;

	hashmap->key_new_func = func;

	return true;
}

/**
 * l_hashmap_set_key_free_function:
 * @hashmap: hash table object
 * @func: Key destructor function
 *
 * Sets the key destructor function to be used by this object.  This function
 * should undo the result of the function specified in
 * l_hashmap_set_key_copy_function(). This function can be NULL, in which
 * case no destructor is called.
 *
 * This function can only be called when the @hashmap is empty.
 *
 * Returns: #true when the key free function could be updated successfully,
 * and #false otherwise.
 **/
LIB_EXPORT bool l_hashmap_set_key_free_function(struct l_hashmap *hashmap,
						l_hashmap_key_free_func_t func)
{
	if (unlikely(!hashmap))
		return false;

	if (hashmap->entries != 0)
		return false;

	hashmap->key_free_func = func;

	return true;
}

/**
 * l_hashmap_destroy:
 * @hashmap: hash table object
 * @destroy: destroy function
 *
 * Free hash table and call @destory on all remaining entries.
 *
 * NOTE: While the destroy is in progress, the hashmap is assumed to be
 * invariant.  The behavior of adding or removing entries while a destroy
 * operation is in progress is undefined.
 **/
LIB_EXPORT void l_hashmap_destroy(struct l_hashmap *hashmap,
				l_hashmap_destroy_func_t destroy)
{
	unsigned int i;

	if (unlikely(!hashmap))
		return;

	for (i = 0; i < NBUCKETS; i++) {
		struct entry *entry, *next, *head = &hashmap->buckets[i];

		if (!head->next)
			continue;

		for (entry = head;; entry = next) {
			if (destroy)
				destroy(entry->value);

			free_key(hashmap, entry->key);

			next = entry->next;

			if (entry != head)
				l_free(entry);

			if (next == head)
				break;
		}
	}

	l_free(hashmap);
}

/**
 * l_hashmap_insert:
 * @hashmap: hash table object
 * @key: key pointer
 * @value: value pointer
 *
 * Insert new @value entry with @key.  Note that entries with a duplicate key
 * are allowed.  If a duplicate entry in inserted, it will be added in order
 * of insertion.  @l_hashmap_lookup and @l_hashmap_remove will remove the
 * first matching entry.
 *
 * Returns: #true when value has been added and #false in case of failure
 **/
LIB_EXPORT bool l_hashmap_insert(struct l_hashmap *hashmap,
				const void *key, void *value)
{
	struct entry *entry, *head;
	unsigned int hash;
	void *key_new;

	if (unlikely(!hashmap))
		return false;

	key_new = get_key_new(hashmap, key);
	hash = hashmap->hash_func(key_new);
	head = &hashmap->buckets[hash % NBUCKETS];

	if (!head->next) {
		head->key = key_new;
		head->value = value;
		head->hash = hash;
		head->next = head;
		goto done;
	}

	entry = l_new(struct entry, 1);
	entry->key = key_new;
	entry->value = value;
	entry->hash = hash;
	entry->next = head;

	while (head->next != entry->next)
		head = head->next;

	head->next = entry;

done:
	hashmap->entries++;

	return true;
}

/**
 * l_hashmap_replace:
 * @hashmap: hash table object
 * @key: key pointer
 * @value: value pointer
 * @old_value: old value that has been replaced.
 *
 * Replace the first entry with @key by @value or insert a new @value entry
 * with @key.  If the entry was replaced, then the old value is returned
 * in @old_value, otherwise @old_value is assigned a #NULL.
 *
 * Returns: #true when value has been added and #false in case of failure
 **/
LIB_EXPORT bool l_hashmap_replace(struct l_hashmap *hashmap,
					const void *key, void *value,
					void **old_value)
{
	struct entry *entry;
	struct entry *head;
	unsigned int hash;
	void *key_new;

	if (unlikely(!hashmap))
		return false;

	key_new = get_key_new(hashmap, key);
	hash = hashmap->hash_func(key_new);
	head = &hashmap->buckets[hash % NBUCKETS];

	if (!head->next) {
		head->key = key_new;
		head->value = value;
		head->hash = hash;
		head->next = head;
		goto done;
	}

	for (entry = head;; entry = entry->next) {
		if (entry->hash != hash)
			goto next;

		if (hashmap->compare_func(key, entry->key))
			goto next;

		if (old_value)
			*old_value = entry->value;

		entry->value = value;
		free_key(hashmap, key_new);

		return true;

next:
		if (entry->next == head)
			break;
	}

	entry = l_new(struct entry, 1);
	entry->key = key_new;
	entry->value = value;
	entry->hash = hash;
	entry->next = head;

	while (head->next != entry->next)
		head = head->next;

	head->next = entry;

done:
	if (old_value)
		*old_value = NULL;

	hashmap->entries++;

	return true;
}

/**
 * l_hashmap_remove:
 * @hashmap: hash table object
 * @key: key pointer
 *
 * Remove entry for @key.
 *
 * Returns: value pointer of the removed entry or #NULL in case of failure
 **/
LIB_EXPORT void *l_hashmap_remove(struct l_hashmap *hashmap, const void *key)
{
	struct entry *entry, *head, *prev;
	unsigned int hash;

	if (unlikely(!hashmap))
		return NULL;

	hash = hashmap->hash_func(key);
	head = &hashmap->buckets[hash % NBUCKETS];

	if (!head->next)
		return NULL;

	for (entry = head, prev = NULL;; prev = entry, entry = entry->next) {
		void *value;

		if (entry->hash != hash)
			goto next;

		if (hashmap->compare_func(key, entry->key))
			goto next;

		value = entry->value;

		if (entry == head) {
			if (entry->next == head) {
				free_key(hashmap, entry->key);
				head->key = NULL;
				head->value = NULL;
				head->hash = 0;
				head->next = NULL;
			} else {
				entry = entry->next;
				free_key(hashmap, head->key);
				head->key = entry->key;
				head->value = entry->value;
				head->hash = entry->hash;
				head->next = entry->next;
				l_free(entry);
			}
		} else {
			prev->next = entry->next;
			free_key(hashmap, entry->key);
			l_free(entry);
		}

		hashmap->entries--;

		return value;

next:
		if (entry->next == head)
			break;
	}

	return NULL;
}

/**
 * l_hashmap_lookup:
 * @hashmap: hash table object
 * @key: key pointer
 *
 * Lookup entry for @key.
 *
 * Returns: value pointer for @key or #NULL in case of failure
 **/
LIB_EXPORT void *l_hashmap_lookup(struct l_hashmap *hashmap, const void *key)
{
	struct entry *entry, *head;
	unsigned int hash;

	if (unlikely(!hashmap))
		return NULL;

	hash = hashmap->hash_func(key);
	head = &hashmap->buckets[hash % NBUCKETS];

	if (!head->next)
		return NULL;

	for (entry = head;; entry = entry->next) {
		if (entry->hash == hash &&
				!hashmap->compare_func(key, entry->key))
			return entry->value;

		if (entry->next == head)
			break;
	}

	return NULL;
}

/**
 * l_hashmap_foreach:
 * @hashmap: hash table object
 * @function: callback function
 * @user_data: user data given to callback function
 *
 * Call @function for every entry in @hashmap.
 *
 * NOTE: While the foreach is in progress, the hashmap is assumed to be
 * invariant.  The behavior of adding or removing entries while a foreach
 * operation is in progress is undefined.
 **/
LIB_EXPORT void l_hashmap_foreach(struct l_hashmap *hashmap,
			l_hashmap_foreach_func_t function, void *user_data)
{
	unsigned int i;

	if (unlikely(!hashmap || !function))
		return;

	for (i = 0; i < NBUCKETS; i++) {
		struct entry *entry, *head = &hashmap->buckets[i];

		if (!head->next)
			continue;

		for (entry = head;; entry = entry->next) {
			function(entry->key, entry->value, user_data);

			if (entry->next == head)
				break;
		}
	}
}

/**
 * l_hashmap_foreach_remove:
 * @hashmap: hash table object
 * @function: callback function
 * @user_data: user data given to callback function
 *
 * Call @function for every entry in @hashmap.  If the @function returns
 * true, then the object will be removed from the hashmap.
 *
 * NOTE: While the foreach is in progress, the hashmap is assumed to be
 * invariant.  The behavior of adding or removing entries while a foreach
 * operation is in progress is undefined.
 *
 * Returns: Number of entries removed.
 **/
LIB_EXPORT unsigned int l_hashmap_foreach_remove(struct l_hashmap *hashmap,
					l_hashmap_remove_func_t function,
					void *user_data)
{
	unsigned int i;
	unsigned int nremoved = 0;

	if (unlikely(!hashmap || !function))
		return 0;

	for (i = 0; i < NBUCKETS; i++) {
		struct entry *head = &hashmap->buckets[i];
		struct entry *entry;
		struct entry *prev;
		bool remove;

		if (head->next == NULL)
			continue;

		entry = head;
		prev = NULL;

		while (true) {
			remove = function(entry->key, entry->value, user_data);

			if (!remove)
				goto next;

			nremoved += 1;
			hashmap->entries -= 1;

			if (entry == head) {
				if (entry->next == head) {
					free_key(hashmap, entry->key);
					head->key = NULL;
					head->value = NULL;
					head->hash = 0;
					head->next = NULL;
					break;
				} else {
					entry = entry->next;
					free_key(hashmap, head->key);
					head->key = entry->key;
					head->value = entry->value;
					head->hash = entry->hash;
					head->next = entry->next;
					l_free(entry);
					entry = head;
					continue;
				}
			} else {
				prev->next = entry->next;
				free_key(hashmap, entry->key);
				l_free(entry);
				entry = prev->next;
				if (entry == head)
					break;
				continue;
			}

next:
			if (entry->next == head)
				break;

			prev = entry;
			entry = entry->next;
		}
	}

	return nremoved;
}

/**
 * l_hashmap_size:
 * @hashmap: hash table object
 *
 * Returns: entries in the hash table
 **/
LIB_EXPORT unsigned int l_hashmap_size(struct l_hashmap *hashmap)
{
	if (unlikely(!hashmap))
		return 0;

	return hashmap->entries;
}

/**
 * l_hashmap_isempty:
 * @hashmap: hash table object
 *
 * Returns: #true if hash table is empty and #false if not
 **/
LIB_EXPORT bool l_hashmap_isempty(struct l_hashmap *hashmap)
{
	if (unlikely(!hashmap))
		return true;

	return hashmap->entries == 0;
}
