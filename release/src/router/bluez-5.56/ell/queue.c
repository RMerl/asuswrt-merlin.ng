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
#include "queue.h"
#include "private.h"

/**
 * SECTION:queue
 * @short_description: Queue support
 *
 * Queue support
 */

/**
 * l_queue:
 *
 * Opague object representing the queue.
 */
struct l_queue {
	struct l_queue_entry *head;
	struct l_queue_entry *tail;
	unsigned int entries;
};

/**
 * l_queue_new:
 *
 * Create a new queue.
 *
 * No error handling is needed since. In case of real memory allocation
 * problems abort() will be called.
 *
 * Returns: a newly allocated #l_queue object
 **/
LIB_EXPORT struct l_queue *l_queue_new(void)
{
	struct l_queue *queue;

	queue = l_new(struct l_queue, 1);

	queue->head = NULL;
	queue->tail = NULL;
	queue->entries = 0;

	return queue;
}

/**
 * l_queue_destroy:
 * @queue: queue object
 * @destroy: destroy function
 *
 * Free queue and call @destory on all remaining entries.
 **/
LIB_EXPORT void l_queue_destroy(struct l_queue *queue,
				l_queue_destroy_func_t destroy)
{
	l_queue_clear(queue, destroy);
	l_free(queue);
}

/**
 * l_queue_clear:
 * @queue: queue object
 * @destroy: destroy function
 *
 * Clear queue and call @destory on all remaining entries.
 **/
LIB_EXPORT void l_queue_clear(struct l_queue *queue,
				l_queue_destroy_func_t destroy)
{
	struct l_queue_entry *entry;

	if (unlikely(!queue))
		return;

	entry = queue->head;

	while (entry) {
		struct l_queue_entry *tmp = entry;

		if (destroy)
			destroy(entry->data);

		entry = entry->next;

		l_free(tmp);
	}

	queue->head = NULL;
	queue->tail = NULL;
	queue->entries = 0;
}

/**
 * l_queue_push_tail:
 * @queue: queue object
 * @data: pointer to data
 *
 * Adds @data pointer at the end of the queue.
 *
 * Returns: #true when data has been added and #false in case an invalid
 *          @queue object has been provided
 **/
LIB_EXPORT bool l_queue_push_tail(struct l_queue *queue, void *data)
{
	struct l_queue_entry *entry;

	if (unlikely(!queue))
		return false;

	entry = l_new(struct l_queue_entry, 1);

	entry->data = data;
	entry->next = NULL;

	if (queue->tail)
		queue->tail->next = entry;

	queue->tail = entry;

	if (!queue->head)
		queue->head = entry;

	queue->entries++;

	return true;
}

/**
 * l_queue_push_head:
 * @queue: queue object
 * @data: pointer to data
 *
 * Adds @data pointer at the start of the queue.
 *
 * Returns: #true when data has been added and #false in case an invalid
 *          @queue object has been provided
 **/
LIB_EXPORT bool l_queue_push_head(struct l_queue *queue, void *data)
{
	struct l_queue_entry *entry;

	if (unlikely(!queue))
		return false;

	entry = l_new(struct l_queue_entry, 1);

	entry->data = data;
	entry->next = queue->head;

	queue->head = entry;

	if (!queue->tail)
		queue->tail = entry;

	queue->entries++;

	return true;
}

/**
 * l_queue_pop_head:
 * @queue: queue object
 *
 * Removes the first element of the queue an returns it.
 *
 * Returns: data pointer to first element or #NULL in case an empty queue
 **/
LIB_EXPORT void *l_queue_pop_head(struct l_queue *queue)
{
	struct l_queue_entry *entry;
	void *data;

	if (unlikely(!queue))
		return NULL;

	if (!queue->head)
		return NULL;

	entry = queue->head;

	if (!queue->head->next) {
		queue->head = NULL;
		queue->tail = NULL;
	} else
		queue->head = queue->head->next;

	data = entry->data;

	l_free(entry);

	queue->entries--;

	return data;
}

/**
 * l_queue_peek_head:
 * @queue: queue object
 *
 * Peeks at the first element of the queue an returns it.
 *
 * Returns: data pointer to first element or #NULL in case an empty queue
 **/
LIB_EXPORT void *l_queue_peek_head(struct l_queue *queue)
{
	struct l_queue_entry *entry;

	if (unlikely(!queue))
		return NULL;

	if (!queue->head)
		return NULL;

	entry = queue->head;
	return entry->data;
}

/**
 * l_queue_peek_tail:
 * @queue: queue object
 *
 * Peeks at the last element of the queue an returns it.
 *
 * Returns: data pointer to first element or #NULL in case an empty queue
 **/
LIB_EXPORT void *l_queue_peek_tail(struct l_queue *queue)
{
	struct l_queue_entry *entry;

	if (unlikely(!queue))
		return NULL;

	if (!queue->tail)
		return NULL;

	entry = queue->tail;
	return entry->data;
}

/**
 * l_queue_insert:
 * @queue: queue object
 * @data: pointer to data
 * @function: compare function
 * @user_data: user data given to compare function
 *
 * Inserts @data pointer at a position in the queue determined by the
 * compare @function.  @function should return > 0 if the @data (first
 * parameter) should be inserted after the current entry (second parameter).
 *
 * Returns: #true when data has been added and #false in case of failure
 **/
LIB_EXPORT bool l_queue_insert(struct l_queue *queue, void *data,
                        l_queue_compare_func_t function, void *user_data)
{
	struct l_queue_entry *entry, *prev, *cur;
	int cmp;

	if (unlikely(!queue || !function))
		return false;

	entry = l_new(struct l_queue_entry, 1);

	entry->data = data;
	entry->next = NULL;

	if (!queue->head) {
		queue->head = entry;
		queue->tail = entry;
		goto done;
	}

	for (prev = NULL, cur = queue->head; cur; prev = cur, cur = cur->next) {
		cmp = function(entry->data, cur->data, user_data);

		if (cmp >= 0)
			continue;

		if (prev == NULL) {
			entry->next = queue->head;
			queue->head = entry;
			goto done;
		}

		entry->next = cur;
		prev->next = entry;

		goto done;
	}

	queue->tail->next = entry;
	queue->tail = entry;

done:
	queue->entries++;

	return true;
}

/**
 * l_queue_find:
 * @queue: queue object
 * @function: match function
 * @user_data: user data given to compare function
 *
 * Finds an entry in the queue by running the match @function
 *
 * Returns: Matching entry or NULL if no entry can be found
 **/
LIB_EXPORT void *l_queue_find(struct l_queue *queue,
				l_queue_match_func_t function,
				const void *user_data)
{
	struct l_queue_entry *entry;

	if (unlikely(!queue || !function))
		return NULL;

	for (entry = queue->head; entry; entry = entry->next)
		if (function(entry->data, user_data))
			return entry->data;

	return NULL;
}

/**
 * l_queue_remove:
 * @queue: queue object
 * @data: pointer to data
 *
 * Remove given @data from the queue.
 *
 * Returns: #true when data has been removed and #false when data could not
 *          be found or an invalid @queue object has been provided
 **/
LIB_EXPORT bool l_queue_remove(struct l_queue *queue, void *data)
{
	struct l_queue_entry *entry, *prev;

	if (unlikely(!queue))
		return false;

	for (entry = queue->head, prev = NULL; entry;
					prev = entry, entry = entry->next) {
		if (entry->data != data)
			continue;

		if (prev)
			prev->next = entry->next;
		else
			queue->head = entry->next;

		if (!entry->next)
			queue->tail = prev;

		l_free(entry);

		queue->entries--;

		return true;
	}

	return false;
}

/**
 * l_queue_reverse:
 * @queue: queue object
 *
 * Reverse entries in the queue.
 *
 * Returns: #true on success and #false on failure
 **/
LIB_EXPORT bool l_queue_reverse(struct l_queue *queue)
{
	struct l_queue_entry *entry, *prev = NULL;

	if (unlikely(!queue))
		return false;

	entry = queue->head;

	while (entry) {
		struct l_queue_entry *next = entry->next;

		entry->next = prev;

		prev = entry;
		entry = next;
	}

	queue->tail = queue->head;
	queue->head = prev;

	return true;
}

/**
 * l_queue_foreach:
 * @queue: queue object
 * @function: callback function
 * @user_data: user data given to callback function
 *
 * Call @function for every given data in @queue.
 **/
LIB_EXPORT void l_queue_foreach(struct l_queue *queue,
			l_queue_foreach_func_t function, void *user_data)
{
	struct l_queue_entry *entry;

	if (unlikely(!queue || !function))
		return;

	for (entry = queue->head; entry; entry = entry->next)
		function(entry->data, user_data);
}

/**
 * l_queue_foreach_remove:
 * @queue: queue object
 * @function: callback function
 * @user_data: user data given to callback function
 *
 * Remove all entries in the @queue where @function returns #true.
 *
 * Returns: number of removed entries
 **/
LIB_EXPORT unsigned int l_queue_foreach_remove(struct l_queue *queue,
                        l_queue_remove_func_t function, void *user_data)
{
	struct l_queue_entry *entry, *prev = NULL;
	unsigned int count = 0;

	if (unlikely(!queue || !function))
		return 0;

	entry = queue->head;

	while (entry) {
		if (function(entry->data, user_data)) {
			struct l_queue_entry *tmp = entry;

			if (prev)
				prev->next = entry->next;
			else
				queue->head = entry->next;

			if (!entry->next)
				queue->tail = prev;

			entry = entry->next;

			l_free(tmp);

			count++;
		} else {
			prev = entry;
			entry = entry->next;
		}
	}

	queue->entries -= count;

	return count;
}

/**
 * l_queue_remove_if
 * @queue: queue object
 * @function: callback function
 * @user_data: user data given to callback function
 *
 * Remove the first entry in the @queue where the function returns #true.
 *
 * Returns: NULL if no entry was found, or the entry data if removal was
 * successful.
 **/
LIB_EXPORT void *l_queue_remove_if(struct l_queue *queue,
				l_queue_match_func_t function,
				const void *user_data)
{
	struct l_queue_entry *entry, *prev = NULL;

	if (unlikely(!queue || !function))
		return NULL;

	entry = queue->head;

	while (entry) {
		if (function(entry->data, user_data)) {
			struct l_queue_entry *tmp = entry;
			void *data;

			if (prev)
				prev->next = entry->next;
			else
				queue->head = entry->next;

			if (!entry->next)
				queue->tail = prev;

			entry = entry->next;

			data = tmp->data;

			l_free(tmp);
			queue->entries--;

			return data;
		}

		prev = entry;
		entry = entry->next;
	}

	return NULL;
}

/**
 * l_queue_length:
 * @queue: queue object
 *
 * Returns: entries of the queue
 **/
LIB_EXPORT unsigned int l_queue_length(struct l_queue *queue)
{
	if (unlikely(!queue))
		return 0;

	return queue->entries;
}

/**
 * l_queue_isempty:
 * @queue: queue object
 *
 * Returns: #true if @queue is empty and #false is not
 **/
LIB_EXPORT bool l_queue_isempty(struct l_queue *queue)
{
	if (unlikely(!queue))
		return true;

	return queue->entries == 0;
}

/**
 * l_queue_get_entries:
 * @queue: queue object
 *
 * This function gives direct, read-only access to the internal list structure
 * of the queue.  This can be used to efficiently traverse the elements.
 *
 * Returns: A pointer to the head of the queue.
 **/
LIB_EXPORT const struct l_queue_entry *l_queue_get_entries(
							struct l_queue *queue)
{
	if (unlikely(!queue))
		return NULL;

	return queue->head;
}
