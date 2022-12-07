/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

int create_file(const char *filename, const mode_t mode);
int create_name(char *buf, size_t size, const char *path,
				const char *address, const char *name);

int textfile_put(const char *pathname, const char *key, const char *value);
int textfile_del(const char *pathname, const char *key);
char *textfile_get(const char *pathname, const char *key);

typedef void (*textfile_cb) (char *key, char *value, void *data);

int textfile_foreach(const char *pathname, textfile_cb func, void *data);
