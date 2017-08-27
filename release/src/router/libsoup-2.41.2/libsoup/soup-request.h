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

#ifndef SOUP_REQUEST_H
#define SOUP_REQUEST_H 1

#ifdef LIBSOUP_USE_UNSTABLE_REQUEST_API

#include <gio/gio.h>

#include <libsoup/soup-types.h>

G_BEGIN_DECLS

#define SOUP_TYPE_REQUEST            (soup_request_get_type ())
#define SOUP_REQUEST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SOUP_TYPE_REQUEST, SoupRequest))
#define SOUP_REQUEST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_TYPE_REQUEST, SoupRequestClass))
#define SOUP_IS_REQUEST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SOUP_TYPE_REQUEST))
#define SOUP_IS_REQUEST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SOUP_TYPE_REQUEST))
#define SOUP_REQUEST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_TYPE_REQUEST, SoupRequestClass))

typedef struct _SoupRequest SoupRequest;
typedef struct _SoupRequestPrivate SoupRequestPrivate;
typedef struct _SoupRequestClass SoupRequestClass;

struct _SoupRequest {
	GObject parent;

	SoupRequestPrivate *priv;
};

struct _SoupRequestClass {
	GObjectClass parent;

	const char **schemes;

	gboolean       (*check_uri)          (SoupRequest          *req_base,
					      SoupURI              *uri,
					      GError              **error);

	GInputStream * (*send)               (SoupRequest          *request,
					      GCancellable         *cancellable,
					      GError              **error);
	void           (*send_async)         (SoupRequest          *request,
					      GCancellable         *cancellable,
					      GAsyncReadyCallback   callback,
					      gpointer              user_data);
	GInputStream * (*send_finish)        (SoupRequest          *request,
					      GAsyncResult         *result,
					      GError              **error);

	goffset        (*get_content_length) (SoupRequest          *request);
	const char *   (*get_content_type)   (SoupRequest          *request);
};

SOUP_AVAILABLE_IN_2_34
GType soup_request_get_type (void);

#define SOUP_REQUEST_URI     "uri"
#define SOUP_REQUEST_SESSION "session"

SOUP_AVAILABLE_IN_2_34
GInputStream *soup_request_send               (SoupRequest          *request,
					       GCancellable         *cancellable,
					       GError              **error);
SOUP_AVAILABLE_IN_2_34
void          soup_request_send_async         (SoupRequest          *request,
					       GCancellable         *cancellable,
					       GAsyncReadyCallback   callback,
					       gpointer              user_data);
SOUP_AVAILABLE_IN_2_34
GInputStream *soup_request_send_finish        (SoupRequest          *request,
					       GAsyncResult         *result,
					       GError              **error);

SOUP_AVAILABLE_IN_2_34
SoupURI      *soup_request_get_uri            (SoupRequest          *request);
SOUP_AVAILABLE_IN_2_34
SoupSession  *soup_request_get_session        (SoupRequest          *request);

SOUP_AVAILABLE_IN_2_34
goffset       soup_request_get_content_length (SoupRequest          *request);
SOUP_AVAILABLE_IN_2_34
const char   *soup_request_get_content_type   (SoupRequest          *request);

G_END_DECLS

#endif /* LIBSOUP_USE_UNSTABLE_REQUEST_API */

#endif /* SOUP_REQUEST_H */
