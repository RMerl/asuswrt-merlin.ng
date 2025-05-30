/*
 * libudev - interface to udev device information
 *
 * Copyright (C) 2008-2010 Kay Sievers <kay.sievers@vrfy.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <fnmatch.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/param.h>
#if defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_HND_ROUTER_BE_4916)
#include <sys/sysmacros.h>
#endif

#include "libudev.h"
#include "libudev-private.h"

/**
 * SECTION:libudev-enumerate
 * @short_description: lookup and sort sys devices
 *
 * Lookup devices in the sys filesystem, filter devices by properties,
 * and return a sorted list of devices.
 */

struct syspath {
	char *syspath;
	size_t len;
};

/**
 * udev_enumerate:
 *
 * Opaque object representing one device lookup/sort context.
 */
struct udev_enumerate {
	struct udev *udev;
	int refcount;
	struct udev_list_node sysattr_match_list;
	struct udev_list_node sysattr_nomatch_list;
	struct udev_list_node subsystem_match_list;
	struct udev_list_node subsystem_nomatch_list;
	struct udev_list_node sysname_match_list;
	struct udev_list_node properties_match_list;
	struct udev_list_node tags_match_list;
	struct udev_device *parent_match;
	struct udev_list_node devices_list;
	struct syspath *devices;
	unsigned int devices_cur;
	unsigned int devices_max;
	bool devices_uptodate:1;
	bool match_is_initialized;
};

/**
 * udev_enumerate_new:
 * @udev: udev library context
 *
 * Returns: an enumeration context
 **/
UDEV_EXPORT struct udev_enumerate *udev_enumerate_new(struct udev *udev)
{
	struct udev_enumerate *udev_enumerate;

	udev_enumerate = calloc(1, sizeof(struct udev_enumerate));
	if (udev_enumerate == NULL)
		return NULL;
	udev_enumerate->refcount = 1;
	udev_enumerate->udev = udev;
	udev_list_init(&udev_enumerate->sysattr_match_list);
	udev_list_init(&udev_enumerate->sysattr_nomatch_list);
	udev_list_init(&udev_enumerate->subsystem_match_list);
	udev_list_init(&udev_enumerate->subsystem_nomatch_list);
	udev_list_init(&udev_enumerate->sysname_match_list);
	udev_list_init(&udev_enumerate->properties_match_list);
	udev_list_init(&udev_enumerate->tags_match_list);
	udev_list_init(&udev_enumerate->devices_list);
	return udev_enumerate;
}

/**
 * udev_enumerate_ref:
 * @udev_enumerate: context
 *
 * Take a reference of a enumeration context.
 *
 * Returns: the passed enumeration context
 **/
UDEV_EXPORT struct udev_enumerate *udev_enumerate_ref(struct udev_enumerate *udev_enumerate)
{
	if (udev_enumerate == NULL)
		return NULL;
	udev_enumerate->refcount++;
	return udev_enumerate;
}

/**
 * udev_enumerate_unref:
 * @udev_enumerate: context
 *
 * Drop a reference of an enumeration context. If the refcount reaches zero,
 * all resources of the enumeration context will be released.
 **/
UDEV_EXPORT void udev_enumerate_unref(struct udev_enumerate *udev_enumerate)
{
	unsigned int i;

	if (udev_enumerate == NULL)
		return;
	udev_enumerate->refcount--;
	if (udev_enumerate->refcount > 0)
		return;
	udev_list_cleanup_entries(udev_enumerate->udev, &udev_enumerate->sysattr_match_list);
	udev_list_cleanup_entries(udev_enumerate->udev, &udev_enumerate->sysattr_nomatch_list);
	udev_list_cleanup_entries(udev_enumerate->udev, &udev_enumerate->subsystem_match_list);
	udev_list_cleanup_entries(udev_enumerate->udev, &udev_enumerate->subsystem_nomatch_list);
	udev_list_cleanup_entries(udev_enumerate->udev, &udev_enumerate->sysname_match_list);
	udev_list_cleanup_entries(udev_enumerate->udev, &udev_enumerate->properties_match_list);
	udev_list_cleanup_entries(udev_enumerate->udev, &udev_enumerate->tags_match_list);
	udev_device_unref(udev_enumerate->parent_match);
	udev_list_cleanup_entries(udev_enumerate->udev, &udev_enumerate->devices_list);
	for (i = 0; i < udev_enumerate->devices_cur; i++)
		free(udev_enumerate->devices[i].syspath);
	free(udev_enumerate->devices);
	free(udev_enumerate);
}

/**
 * udev_enumerate_get_udev:
 * @udev_enumerate: context
 *
 * Returns: the udev library context.
 */
UDEV_EXPORT struct udev *udev_enumerate_get_udev(struct udev_enumerate *udev_enumerate)
{
	if (udev_enumerate == NULL)
		return NULL;
	return udev_enumerate->udev;
}

static int syspath_add(struct udev_enumerate *udev_enumerate, const char *syspath)
{
	char *path;
	struct syspath *entry;

	/* double array size if needed */
	if (udev_enumerate->devices_cur >= udev_enumerate->devices_max) {
		struct syspath *buf;
		unsigned int add;

		add = udev_enumerate->devices_max;
		if (add < 1024)
			add = 1024;
		buf = realloc(udev_enumerate->devices, (udev_enumerate->devices_max + add) * sizeof(struct syspath));
		if (buf == NULL)
			return -ENOMEM;
		udev_enumerate->devices = buf;
		udev_enumerate->devices_max += add;
	}

	path = strdup(syspath);
	if (path == NULL)
		return -ENOMEM;
	entry = &udev_enumerate->devices[udev_enumerate->devices_cur];
	entry->syspath = path;
	entry->len = strlen(path);
	udev_enumerate->devices_cur++;
	udev_enumerate->devices_uptodate = false;
	return 0;
}

static int syspath_cmp(const void *p1, const void *p2)
{
	const struct syspath *path1 = p1;
	const struct syspath *path2 = p2;
	size_t len;
	int ret;

	len = MIN(path1->len, path2->len);
	ret = memcmp(path1->syspath, path2->syspath, len);
	if (ret == 0) {
		if (path1->len < path2->len)
			ret = -1;
		else if (path1->len > path2->len)
			ret = 1;
	}
	return ret;
}

/* For devices that should be moved to the absolute end of the list */
static bool devices_delay_end(struct udev *udev, const char *syspath)
{
	static const char *delay_device_list[] = {
		"/block/md",
		"/block/dm-",
		NULL
	};
	size_t len;
	int i;

	len = strlen(udev_get_sys_path(udev));
	for (i = 0; delay_device_list[i] != NULL; i++) {
		if (strstr(&syspath[len], delay_device_list[i]) != NULL) {
			dbg(udev, "delaying: %s\n", syspath);
			return true;
		}
	}
	return false;
}

/* For devices that should just be moved a little bit later, just
 * before the point where some common path prefix changes. Returns the
 * number of characters that make up that common prefix */
static size_t devices_delay_later(struct udev *udev, const char *syspath)
{
	const char *c;

	/* For sound cards the control device must be enumerated last
	 * to make sure it's the final device node that gets ACLs
	 * applied. Applications rely on this fact and use ACL changes
	 * on the control node as an indicator that the ACL change of
	 * the entire sound card completed. The kernel makes this
	 * guarantee when creating those devices, and hence we should
	 * too when enumerating them. */

	if ((c = strstr(syspath, "/sound/card"))) {
		c += 11;
		c += strcspn(c, "/");

		if (strncmp(c, "/controlC", 9) == 0)
			return c - syspath + 1;
	}

	return 0;
}

/**
 * udev_enumerate_get_list_entry:
 * @udev_enumerate: context
 *
 * Returns: the first entry of the sorted list of device paths.
 */
UDEV_EXPORT struct udev_list_entry *udev_enumerate_get_list_entry(struct udev_enumerate *udev_enumerate)
{
	if (udev_enumerate == NULL)
		return NULL;
	if (!udev_enumerate->devices_uptodate) {
		unsigned int i;
		unsigned int max;
		struct syspath *prev = NULL, *move_later = NULL;
		size_t move_later_prefix = 0;

		udev_list_cleanup_entries(udev_enumerate->udev, &udev_enumerate->devices_list);
		qsort(udev_enumerate->devices, udev_enumerate->devices_cur, sizeof(struct syspath), syspath_cmp);

		max = udev_enumerate->devices_cur;
		for (i = 0; i < max; i++) {
			struct syspath *entry = &udev_enumerate->devices[i];

			/* skip duplicated entries */
			if (prev != NULL &&
			    entry->len == prev->len &&
			    memcmp(entry->syspath, prev->syspath, entry->len) == 0)
				continue;
			prev = entry;

			/* skip to be delayed devices, and add them to the end of the list */
			if (devices_delay_end(udev_enumerate->udev, entry->syspath)) {
				syspath_add(udev_enumerate, entry->syspath);
				/* need to update prev here for the case realloc() gives a different address */
				prev = &udev_enumerate->devices[i];
				continue;
			}

			/* skip to be delayed devices, and move the to
			 * the point where the prefix changes. We can
			 * only move one item at a time. */
			if (!move_later) {
				move_later_prefix = devices_delay_later(udev_enumerate->udev, entry->syspath);

				if (move_later_prefix > 0) {
					move_later = entry;
					continue;
				}
			}

			if (move_later &&
			    strncmp(entry->syspath, move_later->syspath, move_later_prefix) != 0) {

				udev_list_entry_add(udev_enumerate->udev, &udev_enumerate->devices_list,
					    move_later->syspath, NULL, 0);
				move_later = NULL;
			}

			udev_list_entry_add(udev_enumerate->udev, &udev_enumerate->devices_list,
					    entry->syspath, NULL, 0);
		}

		if (move_later)
			udev_list_entry_add(udev_enumerate->udev, &udev_enumerate->devices_list,
					    move_later->syspath, NULL, 0);

		/* add and cleanup delayed devices from end of list */
		for (i = max; i < udev_enumerate->devices_cur; i++) {
			struct syspath *entry = &udev_enumerate->devices[i];

			udev_list_entry_add(udev_enumerate->udev, &udev_enumerate->devices_list,
					    entry->syspath, NULL, 0);
			free(entry->syspath);
		}
		udev_enumerate->devices_cur = max;

		udev_enumerate->devices_uptodate = true;
	}
	return udev_list_get_entry(&udev_enumerate->devices_list);
}

/**
 * udev_enumerate_add_match_subsystem:
 * @udev_enumerate: context
 * @subsystem: filter for a subsystem of the device to include in the list
 *
 * Returns: 0 on success, otherwise a negative error value.
 */
UDEV_EXPORT int udev_enumerate_add_match_subsystem(struct udev_enumerate *udev_enumerate, const char *subsystem)
{
	if (udev_enumerate == NULL)
		return -EINVAL;
	if (subsystem == NULL)
		return 0;
	if (udev_list_entry_add(udev_enumerate_get_udev(udev_enumerate),
				&udev_enumerate->subsystem_match_list, subsystem, NULL, UDEV_LIST_UNIQUE) == NULL)
		return -ENOMEM;
	return 0;
}

/**
 * udev_enumerate_add_nomatch_subsystem:
 * @udev_enumerate: context
 * @subsystem: filter for a subsystem of the device to exclude from the list
 *
 * Returns: 0 on success, otherwise a negative error value.
 */
UDEV_EXPORT int udev_enumerate_add_nomatch_subsystem(struct udev_enumerate *udev_enumerate, const char *subsystem)
{
	if (udev_enumerate == NULL)
		return -EINVAL;
	if (subsystem == NULL)
		return 0;
	if (udev_list_entry_add(udev_enumerate_get_udev(udev_enumerate),
				&udev_enumerate->subsystem_nomatch_list, subsystem, NULL, UDEV_LIST_UNIQUE) == NULL)
		return -ENOMEM;
	return 0;
}

/**
 * udev_enumerate_add_match_sysattr:
 * @udev_enumerate: context
 * @sysattr: filter for a sys attribute at the device to include in the list
 * @value: optional value of the sys attribute
 *
 * Returns: 0 on success, otherwise a negative error value.
 */
UDEV_EXPORT int udev_enumerate_add_match_sysattr(struct udev_enumerate *udev_enumerate, const char *sysattr, const char *value)
{
	if (udev_enumerate == NULL)
		return -EINVAL;
	if (sysattr == NULL)
		return 0;
	if (udev_list_entry_add(udev_enumerate_get_udev(udev_enumerate),
				&udev_enumerate->sysattr_match_list, sysattr, value, 0) == NULL)
		return -ENOMEM;
	return 0;
}

/**
 * udev_enumerate_add_nomatch_sysattr:
 * @udev_enumerate: context
 * @sysattr: filter for a sys attribute at the device to exclude from the list
 * @value: optional value of the sys attribute
 *
 * Returns: 0 on success, otherwise a negative error value.
 */
UDEV_EXPORT int udev_enumerate_add_nomatch_sysattr(struct udev_enumerate *udev_enumerate, const char *sysattr, const char *value)
{
	if (udev_enumerate == NULL)
		return -EINVAL;
	if (sysattr == NULL)
		return 0;
	if (udev_list_entry_add(udev_enumerate_get_udev(udev_enumerate),
			   &udev_enumerate->sysattr_nomatch_list, sysattr, value, 0) == NULL)
		return -ENOMEM;
	return 0;
}

static int match_sysattr_value(struct udev_device *dev, const char *sysattr, const char *match_val)
{
	const char *val = NULL;
	bool match = false;

	val = udev_device_get_sysattr_value(dev, sysattr);
	if (val == NULL)
		goto exit;
	if (match_val == NULL) {
		match = true;
		goto exit;
	}
	if (fnmatch(match_val, val, 0) == 0) {
		match = true;
		goto exit;
	}
exit:
	return match;
}

/**
 * udev_enumerate_add_match_property:
 * @udev_enumerate: context
 * @property: filter for a property of the device to include in the list
 * @value: value of the property
 *
 * Returns: 0 on success, otherwise a negative error value.
 */
UDEV_EXPORT int udev_enumerate_add_match_property(struct udev_enumerate *udev_enumerate, const char *property, const char *value)
{
	if (udev_enumerate == NULL)
		return -EINVAL;
	if (property == NULL)
		return 0;
	if (udev_list_entry_add(udev_enumerate_get_udev(udev_enumerate),
				&udev_enumerate->properties_match_list, property, value, 0) == NULL)
		return -ENOMEM;
	return 0;
}

/**
 * udev_enumerate_add_match_tag:
 * @udev_enumerate: context
 * @tag: filter for a tag of the device to include in the list
 *
 * Returns: 0 on success, otherwise a negative error value.
 */
UDEV_EXPORT int udev_enumerate_add_match_tag(struct udev_enumerate *udev_enumerate, const char *tag)
{
	if (udev_enumerate == NULL)
		return -EINVAL;
	if (tag == NULL)
		return 0;
	if (udev_list_entry_add(udev_enumerate_get_udev(udev_enumerate),
				&udev_enumerate->tags_match_list, tag, NULL, UDEV_LIST_UNIQUE) == NULL)
		return -ENOMEM;
	return 0;
}

/**
 * udev_enumerate_add_match_parent:
 * @udev_enumerate: context
 * @parent: parent device where to start searching
 *
 * Return the devices on the subtree of one given device. The parent
 * itself is included in the list.
 *
 * A reference for the device is held until the udev_enumerate context
 * is cleaned up.
 *
 * Returns: 0 on success, otherwise a negative error value.
 */
UDEV_EXPORT int udev_enumerate_add_match_parent(struct udev_enumerate *udev_enumerate, struct udev_device *parent)
{
	if (udev_enumerate == NULL)
		return -EINVAL;
	if (parent == NULL)
		return 0;
	if (udev_enumerate->parent_match != NULL)
		udev_device_unref(udev_enumerate->parent_match);
	udev_enumerate->parent_match = udev_device_ref(parent);
	return 0;
}

/**
 * udev_enumerate_add_match_is_initialized:
 * @udev_enumerate: context
 *
 * Match only devices which udev has set up already. This makes
 * sure, that the device node permissions and context are properly set
 * and that network devices are fully renamed.
 *
 * Usually, devices which are found in the kernel but not already
 * handled by udev, have still pending events. Services should subscribe
 * to monitor events and wait for these devices to become ready, instead
 * of using uninitialized devices.
 *
 * For now, this will not affect devices which do not have a device node
 * and are not network interfaces.
 *
 * Returns: 0 on success, otherwise a negative error value.
 */
UDEV_EXPORT int udev_enumerate_add_match_is_initialized(struct udev_enumerate *udev_enumerate)
{
	if (udev_enumerate == NULL)
		return -EINVAL;
	udev_enumerate->match_is_initialized = true;
	return 0;
}

/**
 * udev_enumerate_add_match_sysname:
 * @udev_enumerate: context
 * @sysname: filter for the name of the device to include in the list
 *
 * Returns: 0 on success, otherwise a negative error value.
 */
UDEV_EXPORT int udev_enumerate_add_match_sysname(struct udev_enumerate *udev_enumerate, const char *sysname)
{
	if (udev_enumerate == NULL)
		return -EINVAL;
	if (sysname == NULL)
		return 0;
	if (udev_list_entry_add(udev_enumerate_get_udev(udev_enumerate),
				&udev_enumerate->sysname_match_list, sysname, NULL, UDEV_LIST_UNIQUE) == NULL)
		return -ENOMEM;
	return 0;
}

static bool match_sysattr(struct udev_enumerate *udev_enumerate, struct udev_device *dev)
{
	struct udev_list_entry *list_entry;

	/* skip list */
	udev_list_entry_foreach(list_entry, udev_list_get_entry(&udev_enumerate->sysattr_nomatch_list)) {
		if (match_sysattr_value(dev, udev_list_entry_get_name(list_entry),
					udev_list_entry_get_value(list_entry)))
			return false;
	}
	/* include list */
	if (udev_list_get_entry(&udev_enumerate->sysattr_match_list) != NULL) {
		udev_list_entry_foreach(list_entry, udev_list_get_entry(&udev_enumerate->sysattr_match_list)) {
			/* anything that does not match, will make it FALSE */
			if (!match_sysattr_value(dev, udev_list_entry_get_name(list_entry),
						 udev_list_entry_get_value(list_entry)))
				return false;
		}
		return true;
	}
	return true;
}

static bool match_property(struct udev_enumerate *udev_enumerate, struct udev_device *dev)
{
	struct udev_list_entry *list_entry;
	bool match = false;

	/* no match always matches */
	if (udev_list_get_entry(&udev_enumerate->properties_match_list) == NULL)
		return true;

	/* loop over matches */
	udev_list_entry_foreach(list_entry, udev_list_get_entry(&udev_enumerate->properties_match_list)) {
		const char *match_key = udev_list_entry_get_name(list_entry);
		const char *match_value = udev_list_entry_get_value(list_entry);
		struct udev_list_entry *property_entry;

		/* loop over device properties */
		udev_list_entry_foreach(property_entry, udev_device_get_properties_list_entry(dev)) {
			const char *dev_key = udev_list_entry_get_name(property_entry);
			const char *dev_value = udev_list_entry_get_value(property_entry);

			if (fnmatch(match_key, dev_key, 0) != 0)
				continue;
			if (match_value == NULL && dev_value == NULL) {
				match = true;
				goto out;
			}
			if (match_value == NULL || dev_value == NULL)
				continue;
			if (fnmatch(match_value, dev_value, 0) == 0) {
				match = true;
				goto out;
			}
		}
	}
out:
	return match;
}

static bool match_tag(struct udev_enumerate *udev_enumerate, struct udev_device *dev)
{
	struct udev_list_entry *list_entry;

	/* no match always matches */
	if (udev_list_get_entry(&udev_enumerate->tags_match_list) == NULL)
		return true;

	/* loop over matches */
	udev_list_entry_foreach(list_entry, udev_list_get_entry(&udev_enumerate->tags_match_list))
		if (!udev_device_has_tag(dev, udev_list_entry_get_name(list_entry)))
			return false;

	return true;
}

static bool match_parent(struct udev_enumerate *udev_enumerate, struct udev_device *dev)
{
	const char *parent;

	if (udev_enumerate->parent_match == NULL)
		return true;

	parent = udev_device_get_devpath(udev_enumerate->parent_match);
	return strncmp(parent, udev_device_get_devpath(dev), strlen(parent)) == 0;
}

static bool match_sysname(struct udev_enumerate *udev_enumerate, const char *sysname)
{
	struct udev_list_entry *list_entry;

	if (udev_list_get_entry(&udev_enumerate->sysname_match_list) == NULL)
		return true;

	udev_list_entry_foreach(list_entry, udev_list_get_entry(&udev_enumerate->sysname_match_list)) {
		if (fnmatch(udev_list_entry_get_name(list_entry), sysname, 0) != 0)
			continue;
		return true;
	}
	return false;
}

static int scan_dir_and_add_devices(struct udev_enumerate *udev_enumerate,
				    const char *basedir, const char *subdir1, const char *subdir2)
{
	struct udev *udev = udev_enumerate_get_udev(udev_enumerate);
	char path[UTIL_PATH_SIZE];
	size_t l;
	char *s;
	DIR *dir;
	struct dirent *dent;

	s = path;
	l = util_strpcpyl(&s, sizeof(path), udev_get_sys_path(udev), "/", basedir, NULL);
	if (subdir1 != NULL)
		l = util_strpcpyl(&s, l, "/", subdir1, NULL);
	if (subdir2 != NULL)
		util_strpcpyl(&s, l, "/", subdir2, NULL);
	dir = opendir(path);
	if (dir == NULL)
		return -ENOENT;
	for (dent = readdir(dir); dent != NULL; dent = readdir(dir)) {
		char syspath[UTIL_PATH_SIZE];
		struct udev_device *dev;

		if (dent->d_name[0] == '.')
			continue;

		if (!match_sysname(udev_enumerate, dent->d_name))
			continue;

		util_strscpyl(syspath, sizeof(syspath), path, "/", dent->d_name, NULL);
		dev = udev_device_new_from_syspath(udev_enumerate->udev, syspath);
		if (dev == NULL)
			continue;

		if (udev_enumerate->match_is_initialized) {
			/*
			 * All devices with a device node or network interfaces
			 * possibly need udev to adjust the device node permission
			 * or context, or rename the interface before it can be
			 * reliably used from other processes.
			 *
			 * For now, we can only check these types of devices, we
			 * might not store a database, and have no way to find out
			 * for all other types of devices.
			 */
			if (!udev_device_get_is_initialized(dev) &&
			    (major(udev_device_get_devnum(dev)) > 0 || udev_device_get_ifindex(dev) > 0))
				goto nomatch;
		}
		if (!match_parent(udev_enumerate, dev))
			goto nomatch;
		if (!match_tag(udev_enumerate, dev))
			goto nomatch;
		if (!match_property(udev_enumerate, dev))
			goto nomatch;
		if (!match_sysattr(udev_enumerate, dev))
			goto nomatch;

		syspath_add(udev_enumerate, udev_device_get_syspath(dev));
nomatch:
		udev_device_unref(dev);
	}
	closedir(dir);
	return 0;
}

static bool match_subsystem(struct udev_enumerate *udev_enumerate, const char *subsystem)
{
	struct udev_list_entry *list_entry;

	udev_list_entry_foreach(list_entry, udev_list_get_entry(&udev_enumerate->subsystem_nomatch_list)) {
		if (fnmatch(udev_list_entry_get_name(list_entry), subsystem, 0) == 0)
			return false;
	}
	if (udev_list_get_entry(&udev_enumerate->subsystem_match_list) != NULL) {
		udev_list_entry_foreach(list_entry, udev_list_get_entry(&udev_enumerate->subsystem_match_list)) {
			if (fnmatch(udev_list_entry_get_name(list_entry), subsystem, 0) == 0)
				return true;
		}
		return false;
	}
	return true;
}

static int scan_dir(struct udev_enumerate *udev_enumerate, const char *basedir, const char *subdir, const char *subsystem)
{
	struct udev *udev = udev_enumerate_get_udev(udev_enumerate);

	char path[UTIL_PATH_SIZE];
	DIR *dir;
	struct dirent *dent;

	util_strscpyl(path, sizeof(path), udev_get_sys_path(udev), "/", basedir, NULL);
	dir = opendir(path);
	if (dir == NULL)
		return -1;
	for (dent = readdir(dir); dent != NULL; dent = readdir(dir)) {
		if (dent->d_name[0] == '.')
			continue;
		if (!match_subsystem(udev_enumerate, subsystem != NULL ? subsystem : dent->d_name))
			continue;
		scan_dir_and_add_devices(udev_enumerate, basedir, dent->d_name, subdir);
	}
	closedir(dir);
	return 0;
}

/**
 * udev_enumerate_add_syspath:
 * @udev_enumerate: context
 * @syspath: path of a device
 *
 * Add a device to the list of devices, to retrieve it back sorted in dependency order.
 *
 * Returns: 0 on success, otherwise a negative error value.
 */
UDEV_EXPORT int udev_enumerate_add_syspath(struct udev_enumerate *udev_enumerate, const char *syspath)
{
	struct udev_device *udev_device;

	if (udev_enumerate == NULL)
		return -EINVAL;
	if (syspath == NULL)
		return 0;
	/* resolve to real syspath */
	udev_device = udev_device_new_from_syspath(udev_enumerate->udev, syspath);
	if (udev_device == NULL)
		return -EINVAL;
	syspath_add(udev_enumerate, udev_device_get_syspath(udev_device));
	udev_device_unref(udev_device);
	return 0;
}

static int scan_devices_tags(struct udev_enumerate *udev_enumerate)
{
	struct udev *udev = udev_enumerate_get_udev(udev_enumerate);
	struct udev_list_entry *list_entry;

	/* scan only tagged devices, use tags reverse-index, instead of searching all devices in /sys */
	udev_list_entry_foreach(list_entry, udev_list_get_entry(&udev_enumerate->tags_match_list)) {
		DIR *dir;
		struct dirent *dent;
		char path[UTIL_PATH_SIZE];

		util_strscpyl(path, sizeof(path), udev_get_run_path(udev), "/tags/",
			      udev_list_entry_get_name(list_entry), NULL);
		dir = opendir(path);
		if (dir == NULL)
			continue;
		for (dent = readdir(dir); dent != NULL; dent = readdir(dir)) {
			struct udev_device *dev;

			if (dent->d_name[0] == '.')
				continue;

			dev = udev_device_new_from_id_filename(udev_enumerate->udev, dent->d_name);
			if (dev == NULL)
				continue;

			if (!match_subsystem(udev_enumerate, udev_device_get_subsystem(dev)))
				goto nomatch;
			if (!match_sysname(udev_enumerate, udev_device_get_sysname(dev)))
				goto nomatch;
			if (!match_parent(udev_enumerate, dev))
				goto nomatch;
			if (!match_property(udev_enumerate, dev))
				goto nomatch;
			if (!match_sysattr(udev_enumerate, dev))
				goto nomatch;

			syspath_add(udev_enumerate, udev_device_get_syspath(dev));
nomatch:
			udev_device_unref(dev);
		}
		closedir(dir);
	}
	return 0;
}

static int parent_add_child(struct udev_enumerate *enumerate, const char *path)
{
	struct udev_device *dev;

	dev = udev_device_new_from_syspath(enumerate->udev, path);
	if (dev == NULL)
		return -ENODEV;

	if (!match_subsystem(enumerate, udev_device_get_subsystem(dev)))
		return 0;
	if (!match_sysname(enumerate, udev_device_get_sysname(dev)))
		return 0;
	if (!match_property(enumerate, dev))
		return 0;
	if (!match_sysattr(enumerate, dev))
		return 0;

	syspath_add(enumerate, udev_device_get_syspath(dev));
	udev_device_unref(dev);
	return 1;
}

static int parent_crawl_children(struct udev_enumerate *enumerate, const char *path, int maxdepth)
{
	DIR *d;
	struct dirent *dent;

	d = opendir(path);
	if (d == NULL)
		return -errno;

	for (dent = readdir(d); dent != NULL; dent = readdir(d)) {
		char *child;

		if (dent->d_name[0] == '.')
			continue;
		if (dent->d_type != DT_DIR)
			continue;
		if (asprintf(&child, "%s/%s", path, dent->d_name) < 0)
			continue;
		parent_add_child(enumerate, child);
		if (maxdepth > 0)
			parent_crawl_children(enumerate, child, maxdepth-1);
		free(child);
	}

	closedir(d);
	return 0;
}

static int scan_devices_children(struct udev_enumerate *enumerate)
{
	const char *path;

	path = udev_device_get_syspath(enumerate->parent_match);
	parent_add_child(enumerate, path);
	return parent_crawl_children(enumerate, path, 256);
}

static int scan_devices_all(struct udev_enumerate *udev_enumerate)
{
	struct udev *udev = udev_enumerate_get_udev(udev_enumerate);
	char base[UTIL_PATH_SIZE];
	struct stat statbuf;

	util_strscpyl(base, sizeof(base), udev_get_sys_path(udev), "/subsystem", NULL);
	if (stat(base, &statbuf) == 0) {
		/* we have /subsystem/, forget all the old stuff */
		dbg(udev, "searching '/subsystem/*/devices/*' dir\n");
		scan_dir(udev_enumerate, "subsystem", "devices", NULL);
	} else {
		dbg(udev, "searching '/bus/*/devices/*' dir\n");
		scan_dir(udev_enumerate, "bus", "devices", NULL);
		dbg(udev, "searching '/class/*' dir\n");
		scan_dir(udev_enumerate, "class", NULL, NULL);
	}
	return 0;
}

/**
 * udev_enumerate_scan_devices:
 * @udev_enumerate: udev enumeration context
 *
 * Returns: 0 on success, otherwise a negative error value.
 **/
UDEV_EXPORT int udev_enumerate_scan_devices(struct udev_enumerate *udev_enumerate)
{
	if (udev_enumerate == NULL)
		return -EINVAL;

	/* efficiently lookup tags only, we maintain a reverse-index */
	if (udev_list_get_entry(&udev_enumerate->tags_match_list) != NULL)
		return scan_devices_tags(udev_enumerate);

	/* walk the subtree of one parent device only */
	if (udev_enumerate->parent_match != NULL)
		return scan_devices_children(udev_enumerate);

	/* scan devices of all subsystems */
	return scan_devices_all(udev_enumerate);
}

/**
 * udev_enumerate_scan_subsystems:
 * @udev_enumerate: udev enumeration context
 *
 * Returns: 0 on success, otherwise a negative error value.
 **/
UDEV_EXPORT int udev_enumerate_scan_subsystems(struct udev_enumerate *udev_enumerate)
{
	struct udev *udev = udev_enumerate_get_udev(udev_enumerate);
	char base[UTIL_PATH_SIZE];
	struct stat statbuf;
	const char *subsysdir;

	if (udev_enumerate == NULL)
		return -EINVAL;

	/* all kernel modules */
	if (match_subsystem(udev_enumerate, "module")) {
		dbg(udev, "searching '%s/modules/*' dir\n", subsysdir);
		scan_dir_and_add_devices(udev_enumerate, "module", NULL, NULL);
	}

	util_strscpyl(base, sizeof(base), udev_get_sys_path(udev), "/subsystem", NULL);
	if (stat(base, &statbuf) == 0)
		subsysdir = "subsystem";
	else
		subsysdir = "bus";

	/* all subsystems (only buses support coldplug) */
	if (match_subsystem(udev_enumerate, "subsystem")) {
		dbg(udev, "searching '%s/*' dir\n", subsysdir);
		scan_dir_and_add_devices(udev_enumerate, subsysdir, NULL, NULL);
	}

	/* all subsystem drivers */
	if (match_subsystem(udev_enumerate, "drivers")) {
		dbg(udev, "searching '%s/*/drivers/*' dir\n", subsysdir);
		scan_dir(udev_enumerate, subsysdir, "drivers", "drivers");
	}
	return 0;
}
