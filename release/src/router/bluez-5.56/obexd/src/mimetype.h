/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
 *
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
