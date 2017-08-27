/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-request-data.c: data: URI request object
 *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n-lib.h>

#define LIBSOUP_USE_UNSTABLE_REQUEST_API

#include "soup-request-data.h"
#include "soup.h"
#include "soup-misc-private.h"

G_DEFINE_TYPE (SoupRequestData, soup_request_data, SOUP_TYPE_REQUEST)

struct _SoupRequestDataPrivate {
	gsize content_length;
	char *content_type;
};

static void
soup_request_data_init (SoupRequestData *data)
{
	data->priv = G_TYPE_INSTANCE_GET_PRIVATE (data, SOUP_TYPE_REQUEST_DATA, SoupRequestDataPrivate);
}

static void
soup_request_data_finalize (GObject *object)
{
	SoupRequestData *data = SOUP_REQUEST_DATA (object);

	g_free (data->priv->content_type);

	G_OBJECT_CLASS (soup_request_data_parent_class)->finalize (object);
}

static gboolean
soup_request_data_check_uri (SoupRequest  *request,
			     SoupURI      *uri,
			     GError      **error)
{
	return uri->host == NULL;
}

#define BASE64_INDICATOR     ";base64"
#define BASE64_INDICATOR_LEN (sizeof (";base64") - 1)

static GInputStream *
soup_request_data_send (SoupRequest   *request,
			GCancellable  *cancellable,
			GError       **error)
{
	SoupRequestData *data = SOUP_REQUEST_DATA (request);
	SoupURI *uri = soup_request_get_uri (request);
	GInputStream *memstream;
	const char *comma, *start, *end;
	gboolean base64 = FALSE;
	char *uristr;

	uristr = soup_uri_to_string (uri, FALSE);
	start = uristr + 5;
	comma = strchr (start, ',');
	if (comma && comma != start) {
		/* Deal with MIME type / params */
		if (comma > start + BASE64_INDICATOR_LEN && !g_ascii_strncasecmp (comma - BASE64_INDICATOR_LEN, BASE64_INDICATOR, BASE64_INDICATOR_LEN)) {
			end = comma - BASE64_INDICATOR_LEN;
			base64 = TRUE;
		} else
			end = comma;

		if (end != start)
			data->priv->content_type = uri_decoded_copy (start, end - start);
	}

	memstream = g_memory_input_stream_new ();

	if (comma)
		start = comma + 1;

	if (*start) {
		guchar *buf = (guchar *) soup_uri_decode (start);

		if (base64)
			buf = g_base64_decode_inplace ((gchar*) buf, &data->priv->content_length);
		else
			data->priv->content_length = strlen ((const char *) buf);

		g_memory_input_stream_add_data (G_MEMORY_INPUT_STREAM (memstream),
						buf, data->priv->content_length,
						g_free);
	}
	g_free (uristr);

	return memstream;
}

static goffset
soup_request_data_get_content_length (SoupRequest *request)
{
	SoupRequestData *data = SOUP_REQUEST_DATA (request);

	return data->priv->content_length;
}

static const char *
soup_request_data_get_content_type (SoupRequest *request)
{
	SoupRequestData *data = SOUP_REQUEST_DATA (request);

	if (data->priv->content_type)
		return data->priv->content_type;
	else
		return "text/plain;charset=US-ASCII";
}

static const char *data_schemes[] = { "data", NULL };

static void
soup_request_data_class_init (SoupRequestDataClass *request_data_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (request_data_class);
	SoupRequestClass *request_class =
		SOUP_REQUEST_CLASS (request_data_class);

	g_type_class_add_private (request_data_class, sizeof (SoupRequestDataPrivate));

	request_class->schemes = data_schemes;

	object_class->finalize = soup_request_data_finalize;

	request_class->check_uri = soup_request_data_check_uri;
	request_class->send = soup_request_data_send;
	request_class->get_content_length = soup_request_data_get_content_length;
	request_class->get_content_type = soup_request_data_get_content_type;
}
