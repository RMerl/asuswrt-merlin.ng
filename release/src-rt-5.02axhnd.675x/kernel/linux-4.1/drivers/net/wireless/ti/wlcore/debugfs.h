/*
 * This file is part of wl1271
 *
 * Copyright (C) 2009 Nokia Corporation
 *
 * Contact: Luciano Coelho <luciano.coelho@nokia.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifndef __DEBUGFS_H__
#define __DEBUGFS_H__

#include "wlcore.h"

__printf(4, 5) int wl1271_format_buffer(char __user *userbuf, size_t count,
					loff_t *ppos, char *fmt, ...);

int wl1271_debugfs_init(struct wl1271 *wl);
void wl1271_debugfs_exit(struct wl1271 *wl);
void wl1271_debugfs_reset(struct wl1271 *wl);
void wl1271_debugfs_update_stats(struct wl1271 *wl);

#define DEBUGFS_FORMAT_BUFFER_SIZE 256

#define DEBUGFS_READONLY_FILE(name, fmt, value...)			\
static ssize_t name## _read(struct file *file, char __user *userbuf,	\
			    size_t count, loff_t *ppos)			\
{									\
	struct wl1271 *wl = file->private_data;				\
	return wl1271_format_buffer(userbuf, count, ppos,		\
				    fmt "\n", ##value);			\
}									\
									\
static const struct file_operations name## _ops = {			\
	.read = name## _read,						\
	.open = simple_open,						\
	.llseek	= generic_file_llseek,					\
};

#define DEBUGFS_ADD(name, parent)					\
	do {								\
		entry = debugfs_create_file(#name, 0400, parent,	\
					    wl, &name## _ops);		\
		if (!entry || IS_ERR(entry))				\
			goto err;					\
	} while (0)


#define DEBUGFS_ADD_PREFIX(prefix, name, parent)			\
	do {								\
		entry = debugfs_create_file(#name, 0400, parent,	\
				    wl, &prefix## _## name## _ops);	\
		if (!entry || IS_ERR(entry))				\
			goto err;					\
	} while (0)

#define DEBUGFS_FWSTATS_FILE(sub, name, fmt, struct_type)		\
static ssize_t sub## _ ##name## _read(struct file *file,		\
				      char __user *userbuf,		\
				      size_t count, loff_t *ppos)	\
{									\
	struct wl1271 *wl = file->private_data;				\
	struct struct_type *stats = wl->stats.fw_stats;			\
									\
	wl1271_debugfs_update_stats(wl);				\
									\
	return wl1271_format_buffer(userbuf, count, ppos, fmt "\n",	\
				    stats->sub.name);			\
}									\
									\
static const struct file_operations sub## _ ##name## _ops = {		\
	.read = sub## _ ##name## _read,					\
	.open = simple_open,						\
	.llseek	= generic_file_llseek,					\
};

#define DEBUGFS_FWSTATS_FILE_ARRAY(sub, name, len, struct_type)		\
static ssize_t sub## _ ##name## _read(struct file *file,		\
				      char __user *userbuf,		\
				      size_t count, loff_t *ppos)	\
{									\
	struct wl1271 *wl = file->private_data;				\
	struct struct_type *stats = wl->stats.fw_stats;			\
	char buf[DEBUGFS_FORMAT_BUFFER_SIZE] = "";			\
	int res, i;							\
									\
	wl1271_debugfs_update_stats(wl);				\
									\
	for (i = 0; i < len; i++)					\
		res = snprintf(buf, sizeof(buf), "%s[%d] = %d\n",	\
			       buf, i, stats->sub.name[i]);		\
									\
	return wl1271_format_buffer(userbuf, count, ppos, "%s", buf);	\
}									\
									\
static const struct file_operations sub## _ ##name## _ops = {		\
	.read = sub## _ ##name## _read,					\
	.open = simple_open,						\
	.llseek	= generic_file_llseek,					\
};

#define DEBUGFS_FWSTATS_ADD(sub, name)					\
	DEBUGFS_ADD(sub## _ ##name, stats)


#endif /* WL1271_DEBUGFS_H */
