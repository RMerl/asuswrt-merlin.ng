/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <glib.h>

#include "log.h"
#include "obex.h"
#include "mimetype.h"

static GSList *drivers = NULL;

static GSList *watches = NULL;

struct io_watch {
	void *object;
	obex_object_io_func func;
	void *user_data;
};

void obex_object_set_io_flags(void *object, int flags, int err)
{
	GSList *l;

	for (l = watches; l;) {
		struct io_watch *watch = l->data;

		l = l->next;

		if (watch->object != object)
			continue;

		if (watch->func(object, flags, err, watch->user_data) == TRUE)
			continue;

		if (g_slist_find(watches, watch) == NULL)
			continue;

		watches = g_slist_remove(watches, watch);
		g_free(watch);
	}
}

static struct io_watch *find_io_watch(void *object)
{
	GSList *l;

	for (l = watches; l; l = l->next) {
		struct io_watch *watch = l->data;

		if (watch->object == object)
			return watch;
	}

	return NULL;
}

static void reset_io_watch(void *object)
{
	struct io_watch *watch;

	watch = find_io_watch(object);
	if (watch == NULL)
		return;

	watches = g_slist_remove(watches, watch);
	g_free(watch);
}

static int set_io_watch(void *object, obex_object_io_func func,
				void *user_data)
{
	struct io_watch *watch;

	if (func == NULL) {
		reset_io_watch(object);
		return 0;
	}

	watch = find_io_watch(object);
	if (watch)
		return -EPERM;

	watch = g_new0(struct io_watch, 1);
	watch->object = object;
	watch->func = func;
	watch->user_data = user_data;

	watches = g_slist_append(watches, watch);

	return 0;
}

static struct obex_mime_type_driver *find_driver(const uint8_t *target,
				unsigned int target_size,
				const char *mimetype, const uint8_t *who,
				unsigned int who_size)
{
	GSList *l;

	for (l = drivers; l; l = l->next) {
		struct obex_mime_type_driver *driver = l->data;

		if (memncmp0(target, target_size, driver->target, driver->target_size))
			continue;

		if (memncmp0(who, who_size, driver->who, driver->who_size))
			continue;

		if (mimetype == NULL || driver->mimetype == NULL) {
			if (mimetype == driver->mimetype)
				return driver;
			else
				continue;
		}

		if (g_ascii_strcasecmp(mimetype, driver->mimetype) == 0)
			return driver;
	}

	return NULL;
}

struct obex_mime_type_driver *obex_mime_type_driver_find(const uint8_t *target,
				unsigned int target_size,
				const char *mimetype, const uint8_t *who,
				unsigned int who_size)
{
	struct obex_mime_type_driver *driver;

	driver = find_driver(target, target_size, mimetype, who, who_size);
	if (driver == NULL) {
		if (who != NULL) {
			/* Fallback to non-who specific */
			driver = find_driver(target, target_size, mimetype, NULL, 0);
			if (driver != NULL)
				return driver;
		}

		if (mimetype != NULL)
			/* Fallback to target default */
			driver = find_driver(target, target_size, NULL, NULL, 0);

		if (driver == NULL)
			/* Fallback to general default */
			driver = find_driver(NULL, 0, NULL, NULL, 0);
	}

	return driver;
}

int obex_mime_type_driver_register(struct obex_mime_type_driver *driver)
{
	if (!driver) {
		error("Invalid driver");
		return -EINVAL;
	}

	if (find_driver(driver->target, driver->target_size, driver->mimetype,
					driver->who, driver->who_size)) {
		error("Permission denied: %s could not be registered",
				driver->mimetype);
		return -EPERM;
	}

	if (driver->set_io_watch == NULL)
		driver->set_io_watch = set_io_watch;

	DBG("driver %p mimetype %s registered", driver, driver->mimetype);

	drivers = g_slist_append(drivers, driver);

	return 0;
}

void obex_mime_type_driver_unregister(struct obex_mime_type_driver *driver)
{
	if (!g_slist_find(drivers, driver)) {
		error("Unable to unregister: No such driver %p", driver);
		return;
	}

	DBG("driver %p mimetype %s unregistered", driver, driver->mimetype);

	drivers = g_slist_remove(drivers, driver);
}
