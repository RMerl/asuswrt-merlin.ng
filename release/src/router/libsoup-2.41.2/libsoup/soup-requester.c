/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-requester.c:
 *
 * Copyright (C) 2010, Igalia S.L.
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

#include "config.h"

#include <glib/gi18n-lib.h>

#define LIBSOUP_USE_UNSTABLE_REQUEST_API

#include "soup-requester.h"
#include "soup.h"
#include "soup-request-data.h"
#include "soup-request-file.h"
#include "soup-request-http.h"

static SoupSessionFeatureInterface *soup_requester_default_feature_interface;
static void soup_requester_session_feature_init (SoupSessionFeatureInterface *feature_interface, gpointer interface_data);

struct _SoupRequesterPrivate {
	SoupSession *session;
	GHashTable *request_types;
};

G_DEFINE_TYPE_WITH_CODE (SoupRequester, soup_requester, G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (SOUP_TYPE_SESSION_FEATURE,
						soup_requester_session_feature_init))

static void
soup_requester_init (SoupRequester *requester)
{
	SoupSessionFeature *feature;

	requester->priv = G_TYPE_INSTANCE_GET_PRIVATE (requester,
						       SOUP_TYPE_REQUESTER,
						       SoupRequesterPrivate);

	requester->priv->request_types = g_hash_table_new (soup_str_case_hash,
							   soup_str_case_equal);

	feature = SOUP_SESSION_FEATURE (requester);
	soup_session_feature_add_feature (feature, SOUP_TYPE_REQUEST_HTTP);
	soup_session_feature_add_feature (feature, SOUP_TYPE_REQUEST_FILE);
	soup_session_feature_add_feature (feature, SOUP_TYPE_REQUEST_DATA);
}

static void
soup_requester_finalize (GObject *object)
{
	SoupRequester *requester = SOUP_REQUESTER (object);

	g_hash_table_destroy (requester->priv->request_types);

	G_OBJECT_CLASS (soup_requester_parent_class)->finalize (object);
}

static void
soup_requester_class_init (SoupRequesterClass *requester_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (requester_class);

	g_type_class_add_private (requester_class, sizeof (SoupRequesterPrivate));

	/* virtual method override */
	object_class->finalize = soup_requester_finalize;
}

static void
attach (SoupSessionFeature *feature, SoupSession *session)
{
	SoupRequester *requester = SOUP_REQUESTER (feature);

	requester->priv->session = session;

	soup_requester_default_feature_interface->attach (feature, session);
}

static void
detach (SoupSessionFeature *feature, SoupSession *session)
{
	SoupRequester *requester = SOUP_REQUESTER (feature);

	requester->priv->session = NULL;

	soup_requester_default_feature_interface->detach (feature, session);
}

static gboolean
add_feature (SoupSessionFeature *feature, GType type)
{
	SoupRequester *requester = SOUP_REQUESTER (feature);
	SoupRequestClass *request_class;
	int i;

	if (!g_type_is_a (type, SOUP_TYPE_REQUEST))
		return FALSE;

	request_class = g_type_class_ref (type);
	for (i = 0; request_class->schemes[i]; i++) {
		g_hash_table_insert (requester->priv->request_types,
				     (char *)request_class->schemes[i],
				     GSIZE_TO_POINTER (type));
	}
	return TRUE;
}

static gboolean
remove_feature (SoupSessionFeature *feature, GType type)
{
	SoupRequester *requester = SOUP_REQUESTER (feature);
	SoupRequestClass *request_class;
	int i, orig_size;

	if (!g_type_is_a (type, SOUP_TYPE_REQUEST))
		return FALSE;

	request_class = g_type_class_peek (type);
	if (!request_class)
		return FALSE;

	orig_size = g_hash_table_size (requester->priv->request_types);
	for (i = 0; request_class->schemes[i]; i++) {
		g_hash_table_remove (requester->priv->request_types,
				     request_class->schemes[i]);
	}

	return g_hash_table_size (requester->priv->request_types) != orig_size;
}

static gboolean
has_feature (SoupSessionFeature *feature, GType type)
{
	SoupRequester *requester = SOUP_REQUESTER (feature);
	GHashTableIter iter;
	gpointer key, value;

	if (!g_type_is_a (type, SOUP_TYPE_REQUEST))
		return FALSE;

	g_hash_table_iter_init (&iter, requester->priv->request_types);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		if (value == GSIZE_TO_POINTER (type))
			return TRUE;
	}
	return FALSE;
}

static void
soup_requester_session_feature_init (SoupSessionFeatureInterface *feature_interface,
				     gpointer interface_data)
{
	soup_requester_default_feature_interface =
		g_type_default_interface_peek (SOUP_TYPE_SESSION_FEATURE);

	feature_interface->attach = attach;
	feature_interface->detach = detach;
	feature_interface->add_feature = add_feature;
	feature_interface->remove_feature = remove_feature;
	feature_interface->has_feature = has_feature;
}

/**
 * soup_requester_new:
 *
 * Creates a new #SoupRequester object, which can be added to
 * a #SoupSession with soup_session_add_feature().
 *
 * Return value: the new #SoupRequester
 *
 * Since: 2.34
 */
SoupRequester *
soup_requester_new (void)
{
	return g_object_new (SOUP_TYPE_REQUESTER, NULL);
}

/**
 * soup_requester_request:
 * @requester: a #SoupRequester
 * @uri_string: a URI, in string form
 * @error: return location for a #GError, or %NULL
 *
 * Creates a #SoupRequest for retrieving @uri_string.
 *
 * Return value: (transfer full): a new #SoupRequest, or
 *   %NULL on error.
 *
 * Since: 2.34
 */
SoupRequest *
soup_requester_request (SoupRequester *requester, const char *uri_string,
			GError **error)
{
	SoupURI *uri;
	SoupRequest *req;

	uri = soup_uri_new (uri_string);
	if (!uri) {
		g_set_error (error, SOUP_REQUESTER_ERROR, SOUP_REQUESTER_ERROR_BAD_URI,
			     _("Could not parse URI '%s'"), uri_string);
		return NULL;
	}

	req = soup_requester_request_uri (requester, uri, error);
	soup_uri_free (uri);
	return req;
}

/**
 * soup_requester_request_uri:
 * @requester: a #SoupRequester
 * @uri: a #SoupURI representing the URI to retrieve
 * @error: return location for a #GError, or %NULL
 *
 * Creates a #SoupRequest for retrieving @uri.
 *
 * Return value: (transfer full): a new #SoupRequest, or
 *   %NULL on error.
 *
 * Since: 2.34
 */
SoupRequest *
soup_requester_request_uri (SoupRequester *requester, SoupURI *uri,
			    GError **error)
{
	GType request_type;

	g_return_val_if_fail (SOUP_IS_REQUESTER (requester), NULL);

	request_type = (GType)GPOINTER_TO_SIZE (g_hash_table_lookup (requester->priv->request_types, uri->scheme));
	if (!request_type) {
		g_set_error (error, SOUP_REQUESTER_ERROR,
			     SOUP_REQUESTER_ERROR_UNSUPPORTED_URI_SCHEME,
			     _("Unsupported URI scheme '%s'"), uri->scheme);
		return NULL;
	}

	return g_initable_new (request_type, NULL, error,
			       "uri", uri,
			       "session", requester->priv->session,
			       NULL);
}

/**
 * SOUP_REQUESTER_ERROR:
 *
 * A #GError domain for #SoupRequester errors. Used with
 * #SoupRequesterError.
 *
 * Since: 2.34
 */
/**
 * SoupRequesterError:
 * @SOUP_REQUESTER_ERROR_BAD_URI: the URI could not be parsed
 * @SOUP_REQUESTER_ERROR_UNSUPPORTED_URI_SCHEME: the URI scheme is not
 *   supported by this #SoupRequester
 *
 * A #SoupRequester error.
 *
 * Since: 2.34
 */

GQuark
soup_requester_error_quark (void)
{
	static GQuark error;
	if (!error)
		error = g_quark_from_static_string ("soup_requester_error_quark");
	return error;
}
