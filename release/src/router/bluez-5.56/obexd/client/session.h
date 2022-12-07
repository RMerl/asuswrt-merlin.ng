/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  OBEX Client
 *
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2011-2012  BMW Car IT GmbH. All rights reserved.
 *
 *
 */

#include <stdint.h>
#include <glib.h>

struct obc_session;

typedef void (*session_callback_t) (struct obc_session *session,
					struct obc_transfer *transfer,
					GError *err, void *user_data);

struct obc_session *obc_session_create(const char *source,
						const char *destination,
						const char *service,
						uint8_t channel,
						const char *owner,
						session_callback_t function,
						void *user_data);

struct obc_session *obc_session_ref(struct obc_session *session);
void obc_session_unref(struct obc_session *session);
void obc_session_shutdown(struct obc_session *session);

int obc_session_set_owner(struct obc_session *session, const char *name,
			GDBusWatchFunction func);
const char *obc_session_get_owner(struct obc_session *session);

const char *obc_session_get_destination(struct obc_session *session);
const char *obc_session_get_path(struct obc_session *session);
const char *obc_session_get_target(struct obc_session *session);

const char *obc_session_register(struct obc_session *session,
						GDBusDestroyFunction destroy);

const void *obc_session_get_attribute(struct obc_session *session,
							int attribute_id);

const char *obc_session_get_folder(struct obc_session *session);

guint obc_session_queue(struct obc_session *session,
				struct obc_transfer *transfer,
				session_callback_t func, void *user_data,
				GError **err);
guint obc_session_setpath(struct obc_session *session, const char *path,
				session_callback_t func, void *user_data,
				GError **err);
guint obc_session_mkdir(struct obc_session *session, const char *folder,
				session_callback_t func, void *user_data,
				GError **err);
guint obc_session_copy(struct obc_session *session, const char *srcname,
				const char *destname, session_callback_t func,
				void *user_data, GError **err);
guint obc_session_move(struct obc_session *session, const char *srcname,
				const char *destname, session_callback_t func,
				void *user_data, GError **err);
guint obc_session_delete(struct obc_session *session, const char *file,
				session_callback_t func, void *user_data,
				GError **err);
void obc_session_cancel(struct obc_session *session, guint id,
							gboolean remove);
