/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2016  Intel Corporation. All rights reserved.
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

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>

#include "util.h"
#include "hashmap.h"
#include "idle.h"
#include "dbus.h"
#include "dbus-private.h"

struct _dbus_name_cache {
	struct l_dbus *bus;
	struct l_hashmap *names;
	const struct _dbus_name_ops *driver;
	unsigned int last_watch_id;
	struct l_idle *watch_remove_work;
};

struct service_watch {
	l_dbus_watch_func_t connect_func;
	l_dbus_watch_func_t disconnect_func;
	l_dbus_destroy_func_t destroy;
	void *user_data;
	unsigned int id;
	struct service_watch *next;
};

struct name_cache_entry {
	int ref_count;
	char *unique_name;
	struct service_watch *watches;
};

struct _dbus_name_cache *_dbus_name_cache_new(struct l_dbus *bus,
					const struct _dbus_name_ops *driver)
{
	struct _dbus_name_cache *cache;

	cache = l_new(struct _dbus_name_cache, 1);

	cache->bus = bus;
	cache->driver = driver;

	return cache;
}

static void service_watch_destroy(void *data)
{
	struct service_watch *watch = data;

	if (watch->destroy)
		watch->destroy(watch->user_data);

	l_free(watch);
}

static void name_cache_entry_destroy(void *data)
{
	struct name_cache_entry *entry = data;
	struct service_watch *watch;

	while (entry->watches) {
		watch = entry->watches;
		entry->watches = watch->next;

		service_watch_destroy(watch);
	}

	l_free(entry->unique_name);

	l_free(entry);
}

void _dbus_name_cache_free(struct _dbus_name_cache *cache)
{
	if (!cache)
		return;

	if (cache->watch_remove_work)
		l_idle_remove(cache->watch_remove_work);

	l_hashmap_destroy(cache->names, name_cache_entry_destroy);

	l_free(cache);
}

bool _dbus_name_cache_add(struct _dbus_name_cache *cache, const char *name)
{
	struct name_cache_entry *entry;

	if (!_dbus_valid_bus_name(name))
		return false;

	if (!cache->names)
		cache->names = l_hashmap_string_new();

	entry = l_hashmap_lookup(cache->names, name);

	if (!entry) {
		entry = l_new(struct name_cache_entry, 1);

		l_hashmap_insert(cache->names, name, entry);

		cache->driver->get_name_owner(cache->bus, name);
	}

	entry->ref_count++;

	return true;
}

bool _dbus_name_cache_remove(struct _dbus_name_cache *cache, const char *name)
{
	struct name_cache_entry *entry;

	entry = l_hashmap_lookup(cache->names, name);

	if (!entry)
		return false;

	if (--entry->ref_count)
		return true;

	l_hashmap_remove(cache->names, name);

	name_cache_entry_destroy(entry);

	return true;
}

const char *_dbus_name_cache_lookup(struct _dbus_name_cache *cache,
					const char *name)
{
	struct name_cache_entry *entry;

	entry = l_hashmap_lookup(cache->names, name);

	if (!entry)
		return NULL;

	return entry->unique_name;
}

void _dbus_name_cache_notify(struct _dbus_name_cache *cache,
				const char *name, const char *owner)
{
	struct name_cache_entry *entry;
	struct service_watch *watch;
	bool prev_connected, connected;

	if (!cache)
		return;

	entry = l_hashmap_lookup(cache->names, name);

	if (!entry)
		return;

	prev_connected = !!entry->unique_name;
	connected = owner && *owner != '\0';

	l_free(entry->unique_name);

	entry->unique_name = connected ? l_strdup(owner) : NULL;

	/*
	 * This check also means we notify all watchers who have a connected
	 * callback when we first learn that the service is in fact connected.
	 */
	if (connected == prev_connected)
		return;

	for (watch = entry->watches; watch; watch = watch->next)
		if (connected && watch->connect_func)
			watch->connect_func(cache->bus, watch->user_data);
		else if (!connected && watch->disconnect_func)
			watch->disconnect_func(cache->bus, watch->user_data);
}

unsigned int _dbus_name_cache_add_watch(struct _dbus_name_cache *cache,
					const char *name,
					l_dbus_watch_func_t connect_func,
					l_dbus_watch_func_t disconnect_func,
					void *user_data,
					l_dbus_destroy_func_t destroy)
{
	struct name_cache_entry *entry;
	struct service_watch *watch;

	if (!_dbus_name_cache_add(cache, name))
		return 0;

	watch = l_new(struct service_watch, 1);
	watch->id = ++cache->last_watch_id;
	watch->connect_func = connect_func;
	watch->disconnect_func = disconnect_func;
	watch->user_data = user_data;
	watch->destroy = destroy;

	entry = l_hashmap_lookup(cache->names, name);

	watch->next = entry->watches;
	entry->watches = watch;

	if (entry->unique_name && connect_func)
		watch->connect_func(cache->bus, watch->user_data);

	return watch->id;
}

static bool service_watch_remove(const void *key, void *value, void *user_data)
{
	struct name_cache_entry *entry = value;
	struct service_watch **watch, *tmp;

	for (watch = &entry->watches; *watch;) {
		if ((*watch)->id) {
			watch = &(*watch)->next;
			continue;
		}

		tmp = *watch;
		*watch = tmp->next;

		service_watch_destroy(tmp);

		entry->ref_count--;
	}

	if (entry->ref_count)
		return false;

	name_cache_entry_destroy(entry);

	return true;
}

static void service_watch_remove_all(struct l_idle *idle, void *user_data)
{
	struct _dbus_name_cache *cache = user_data;

	l_idle_remove(cache->watch_remove_work);
	cache->watch_remove_work = NULL;

	l_hashmap_foreach_remove(cache->names, service_watch_remove, cache);
}

static void service_watch_mark(const void *key, void *value, void *user_data)
{
	struct name_cache_entry *entry = value;
	struct service_watch *watch;
	unsigned int *id = user_data;

	if (!*id)
		return;

	for (watch = entry->watches; watch; watch = watch->next)
		if (watch->id == *id) {
			watch->id = 0;
			watch->connect_func = NULL;
			watch->disconnect_func = NULL;

			if (watch->destroy) {
				watch->destroy(watch->user_data);
				watch->destroy = NULL;
			}

			*id = 0;
			break;
		}
}

bool _dbus_name_cache_remove_watch(struct _dbus_name_cache *cache,
					unsigned int id)
{
	l_hashmap_foreach(cache->names, service_watch_mark, &id);

	if (id)
		return false;

	if (!cache->watch_remove_work)
		cache->watch_remove_work = l_idle_create(
						service_watch_remove_all,
						cache, NULL);

	return true;
}
