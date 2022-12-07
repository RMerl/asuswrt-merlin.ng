/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2007-2010  Nokia Corporation
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#include <unistd.h>

#define OBJECT_SIZE_UNKNOWN -1
#define OBJECT_SIZE_DELETE -2

#define TARGET_SIZE 16

struct obex_session;

int obex_get_stream_start(struct obex_session *os, const char *filename);
int obex_put_stream_start(struct obex_session *os, const char *filename);
const char *obex_get_name(struct obex_session *os);
const char *obex_get_destname(struct obex_session *os);
void obex_set_name(struct obex_session *os, const char *name);
ssize_t obex_get_size(struct obex_session *os);
const char *obex_get_type(struct obex_session *os);
int obex_remove(struct obex_session *os, const char *path);
int obex_copy(struct obex_session *os, const char *source,
						const char *destination);
int obex_move(struct obex_session *os, const char *source,
						const char *destination);
uint8_t obex_get_action_id(struct obex_session *os);
ssize_t obex_get_apparam(struct obex_session *os, const uint8_t **buffer);
ssize_t obex_get_non_header_data(struct obex_session *os,
							const uint8_t **data);
int obex_getpeername(struct obex_session *os, char **name);
int obex_getsockname(struct obex_session *os, char **name);

/* Just a thin wrapper around memcmp to deal with NULL values */
int memncmp0(const void *a, size_t na, const void *b, size_t nb);
