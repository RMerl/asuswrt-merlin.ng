/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2010 Igalia S.L.
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

#ifndef SOUP_REQUESTER_H
#define SOUP_REQUESTER_H 1

#ifdef LIBSOUP_USE_UNSTABLE_REQUEST_API

#include <libsoup/soup-types.h>
#include <libsoup/soup-request.h>

G_BEGIN_DECLS

#define SOUP_TYPE_REQUESTER            (soup_requester_get_type ())
#define SOUP_REQUESTER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SOUP_TYPE_REQUESTER, SoupRequester))
#define SOUP_REQUESTER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_TYPE_REQUESTER, SoupRequesterClass))
#define SOUP_IS_REQUESTER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SOUP_TYPE_REQUESTER))
#define SOUP_IS_REQUESTER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), SOUP_TYPE_REQUESTER))
#define SOUP_REQUESTER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_TYPE_REQUESTER, SoupRequesterClass))

typedef struct _SoupRequester SoupRequester;
typedef struct _SoupRequesterPrivate SoupRequesterPrivate;

struct _SoupRequester {
	GObject parent;

	SoupRequesterPrivate *priv;
};

typedef struct {
	GObjectClass parent_class;
} SoupRequesterClass;

SOUP_AVAILABLE_IN_2_34
GType          soup_requester_get_type        (void);

SOUP_AVAILABLE_IN_2_34
SoupRequester *soup_requester_new             (void);

SOUP_AVAILABLE_IN_2_34
SoupRequest   *soup_requester_request         (SoupRequester  *requester,
					       const char     *uri_string,
					       GError        **error);

SOUP_AVAILABLE_IN_2_34
SoupRequest   *soup_requester_request_uri     (SoupRequester  *requester,
					       SoupURI        *uri,
					       GError        **error);

SOUP_AVAILABLE_IN_2_34
GQuark soup_requester_error_quark (void);
#define SOUP_REQUESTER_ERROR soup_requester_error_quark ()

typedef enum {
	SOUP_REQUESTER_ERROR_BAD_URI,
	SOUP_REQUESTER_ERROR_UNSUPPORTED_URI_SCHEME
} SoupRequesterError;

G_END_DECLS

#endif /* LIBSOUP_USE_UNSTABLE_REQUEST_API */

#endif /* SOUP_REQUESTER_H */
