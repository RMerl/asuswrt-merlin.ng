/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2009, 2010 Red Hat, Inc.
 * Copyright (C) 2010 Igalia, S.L.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SOUP_REQUEST_FILE_H
#define SOUP_REQUEST_FILE_H 1

#ifdef LIBSOUP_USE_UNSTABLE_REQUEST_API

#include "soup-request.h"

G_BEGIN_DECLS

#define SOUP_TYPE_REQUEST_FILE            (soup_request_file_get_type ())
#define SOUP_REQUEST_FILE(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), SOUP_TYPE_REQUEST_FILE, SoupRequestFile))
#define SOUP_REQUEST_FILE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_TYPE_REQUEST_FILE, SoupRequestFileClass))
#define SOUP_IS_REQUEST_FILE(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), SOUP_TYPE_REQUEST_FILE))
#define SOUP_IS_REQUEST_FILE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SOUP_TYPE_REQUEST_FILE))
#define SOUP_REQUEST_FILE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_TYPE_REQUEST_FILE, SoupRequestFileClass))

typedef struct _SoupRequestFilePrivate SoupRequestFilePrivate;

typedef struct {
	SoupRequest parent;

	SoupRequestFilePrivate *priv;
} SoupRequestFile;

typedef struct {
	SoupRequestClass parent;
} SoupRequestFileClass;

SOUP_AVAILABLE_IN_2_34
GType  soup_request_file_get_type (void);

SOUP_AVAILABLE_IN_2_34
GFile *soup_request_file_get_file (SoupRequestFile *file);

G_END_DECLS

#endif /* LIBSOUP_USE_UNSTABLE_REQUEST_API */

#endif /* SOUP_REQUEST_FILE_H */
