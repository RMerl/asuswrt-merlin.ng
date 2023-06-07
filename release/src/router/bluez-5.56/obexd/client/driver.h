/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

struct obc_driver {
	const char *service;
	const char *uuid;
	void *target;
	gsize target_len;
	void *(*supported_features) (struct obc_session *session);
	int (*probe) (struct obc_session *session);
	void (*remove) (struct obc_session *session);
};

int obc_driver_register(struct obc_driver *driver);
void obc_driver_unregister(struct obc_driver *driver);
struct obc_driver *obc_driver_find(const char *pattern);
