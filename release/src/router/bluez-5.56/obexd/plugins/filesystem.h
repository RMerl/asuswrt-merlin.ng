/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

ssize_t string_read(void *object, void *buf, size_t count);
gboolean is_filename(const char *name);
int verify_path(const char *path);
