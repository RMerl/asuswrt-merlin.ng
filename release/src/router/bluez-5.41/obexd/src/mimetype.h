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

typedef gboolean (*obex_object_io_func) (void *object, int flags, int err,
							void *user_data);

struct obex_mime_type_driver {
	const uint8_t *target;
	unsigned int target_size;
	const char *mimetype;
	const uint8_t *who;
	unsigned int who_size;
	void *(*open) (const char *name, int oflag, mode_t mode,
			void *driver_data, size_t *size, int *err);
	int (*close) (void *object);
	ssize_t (*get_next_header)(void *object, void *buf, size_t mtu,
								uint8_t *hi);
	ssize_t (*read) (void *object, void *buf, size_t count);
	ssize_t (*write) (void *object, const void *buf, size_t count);
	int (*flush) (void *object);
	int (*copy) (const char *name, const char *destname);
	int (*move) (const char *name, const char *destname);
	int (*remove) (const char *name);
	int (*set_io_watch) (void *object, obex_object_io_func func,
				void *user_data);
};

int obex_mime_type_driver_register(struct obex_mime_type_driver *driver);
void obex_mime_type_driver_unregister(struct obex_mime_type_driver *driver);
struct obex_mime_type_driver *obex_mime_type_driver_find(const uint8_t *target,
				unsigned int target_size,
				const char *mimetype, const uint8_t *who,
				unsigned int who_size);

void obex_object_set_io_flags(void *object, int flags, int err);
