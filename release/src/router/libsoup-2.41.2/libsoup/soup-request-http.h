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

#ifndef SOUP_REQUEST_HTTP_H
#define SOUP_REQUEST_HTTP_H 1

#ifdef LIBSOUP_USE_UNSTABLE_REQUEST_API

#include "soup-request.h"

G_BEGIN_DECLS

#define SOUP_TYPE_REQUEST_HTTP            (soup_request_http_get_type ())
#define SOUP_REQUEST_HTTP(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), SOUP_TYPE_REQUEST_HTTP, SoupRequestHTTP))
#define SOUP_REQUEST_HTTP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_TYPE_REQUEST_HTTP, SoupRequestHTTPClass))
#define SOUP_IS_REQUEST_HTTP(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), SOUP_TYPE_REQUEST_HTTP))
#define SOUP_IS_REQUEST_HTTP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SOUP_TYPE_REQUEST_HTTP))
#define SOUP_REQUEST_HTTP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_TYPE_REQUEST_HTTP, SoupRequestHTTPClass))

typedef struct _SoupRequestHTTPPrivate SoupRequestHTTPPrivate;

typedef struct {
	SoupRequest parent;

	SoupRequestHTTPPrivate *priv;
} SoupRequestHTTP;

typedef struct {
	SoupRequestClass parent;
} SoupRequestHTTPClass;

SOUP_AVAILABLE_IN_2_34
GType soup_request_http_get_type (void);

SOUP_AVAILABLE_IN_2_34
SoupMessage *soup_request_http_get_message (SoupRequestHTTP *http);

G_END_DECLS

#endif /* LIBSOUP_USE_UNSTABLE_REQUEST_API */

#endif /* SOUP_REQUEST_HTTP_H */
