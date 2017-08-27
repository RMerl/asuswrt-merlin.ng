/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-body-output-stream.c
 *
 * Copyright 2012 Red Hat, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "soup-body-output-stream.h"
#include "soup.h"

typedef enum {
	SOUP_BODY_OUTPUT_STREAM_STATE_CHUNK_SIZE,
	SOUP_BODY_OUTPUT_STREAM_STATE_CHUNK_END,
	SOUP_BODY_OUTPUT_STREAM_STATE_CHUNK,
	SOUP_BODY_OUTPUT_STREAM_STATE_TRAILERS,
	SOUP_BODY_OUTPUT_STREAM_STATE_DONE
} SoupBodyOutputStreamState;

struct _SoupBodyOutputStreamPrivate {
	GOutputStream *base_stream;
	char           buf[20];

	SoupEncoding   encoding;
	goffset        write_length;
	goffset        written;
	SoupBodyOutputStreamState chunked_state;
	gboolean       eof;
};

enum {
	PROP_0,

	PROP_ENCODING,
	PROP_CONTENT_LENGTH
};

static void soup_body_output_stream_pollable_init (GPollableOutputStreamInterface *pollable_interface, gpointer interface_data);

G_DEFINE_TYPE_WITH_CODE (SoupBodyOutputStream, soup_body_output_stream, G_TYPE_FILTER_OUTPUT_STREAM,
			 G_IMPLEMENT_INTERFACE (G_TYPE_POLLABLE_OUTPUT_STREAM,
						soup_body_output_stream_pollable_init))


static void
soup_body_output_stream_init (SoupBodyOutputStream *stream)
{
	stream->priv = G_TYPE_INSTANCE_GET_PRIVATE (stream,
						    SOUP_TYPE_BODY_OUTPUT_STREAM,
						    SoupBodyOutputStreamPrivate);
}

static void
soup_body_output_stream_constructed (GObject *object)
{
	SoupBodyOutputStream *bostream = SOUP_BODY_OUTPUT_STREAM (object);

	bostream->priv->base_stream = g_filter_output_stream_get_base_stream (G_FILTER_OUTPUT_STREAM (bostream));
}

static void
soup_body_output_stream_set_property (GObject *object, guint prop_id,
				      const GValue *value, GParamSpec *pspec)
{
	SoupBodyOutputStream *bostream = SOUP_BODY_OUTPUT_STREAM (object);

	switch (prop_id) {
	case PROP_ENCODING:
		bostream->priv->encoding = g_value_get_enum (value);
		if (bostream->priv->encoding == SOUP_ENCODING_CHUNKED)
			bostream->priv->chunked_state = SOUP_BODY_OUTPUT_STREAM_STATE_CHUNK_SIZE;
		break;
	case PROP_CONTENT_LENGTH:
		bostream->priv->write_length = g_value_get_uint64 (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
soup_body_output_stream_get_property (GObject *object, guint prop_id,
				      GValue *value, GParamSpec *pspec)
{
	SoupBodyOutputStream *bostream = SOUP_BODY_OUTPUT_STREAM (object);

	switch (prop_id) {
	case PROP_ENCODING:
		g_value_set_enum (value, bostream->priv->encoding);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static gssize
soup_body_output_stream_write_raw (SoupBodyOutputStream  *bostream,
				   const void            *buffer,
				   gsize                  count,
				   GCancellable          *cancellable,
				   GError               **error)
{
	gssize nwrote, my_count;

	/* If the caller tries to write too much to a Content-Length
	 * encoded stream, we truncate at the right point, but keep
	 * accepting additional data until they stop.
	 */
	if (bostream->priv->write_length) {
		my_count = MIN (count, bostream->priv->write_length - bostream->priv->written);
		if (my_count == 0) {
			bostream->priv->eof = TRUE;
			return count;
		}
	} else
		my_count = count;

	nwrote = g_output_stream_write (bostream->priv->base_stream,
					buffer, my_count,
					cancellable, error);

	if (nwrote > 0 && bostream->priv->write_length)
		bostream->priv->written += nwrote;

	if (nwrote == my_count && my_count != count)
		nwrote = count;

	return nwrote;
}

static gssize
soup_body_output_stream_write_chunked (SoupBodyOutputStream  *bostream,
				       const void            *buffer,
				       gsize                  count,
				       GCancellable          *cancellable,
				       GError               **error)
{
	char *buf = bostream->priv->buf;
	gssize nwrote, len;

again:
	len = strlen (buf);
	if (len) {
		nwrote = g_output_stream_write (bostream->priv->base_stream,
						buf, len, cancellable, error);
		if (nwrote < 0)
			return nwrote;
		memmove (buf, buf + nwrote, len + 1 - nwrote);
		goto again;
	}

	switch (bostream->priv->chunked_state) {
	case SOUP_BODY_OUTPUT_STREAM_STATE_CHUNK_SIZE:
		g_snprintf (buf, sizeof (bostream->priv->buf),
			    "%lx\r\n", (gulong)count);
		len = strlen (buf);

		if (count > 0)
			bostream->priv->chunked_state = SOUP_BODY_OUTPUT_STREAM_STATE_CHUNK;
		else
			bostream->priv->chunked_state = SOUP_BODY_OUTPUT_STREAM_STATE_TRAILERS;
		break;

	case SOUP_BODY_OUTPUT_STREAM_STATE_CHUNK:
		nwrote = g_output_stream_write (bostream->priv->base_stream,
						buffer, count, cancellable, error);
		if (nwrote < (gssize)count)
			return nwrote;

		bostream->priv->chunked_state = SOUP_BODY_OUTPUT_STREAM_STATE_CHUNK_END;
		break;

	case SOUP_BODY_OUTPUT_STREAM_STATE_CHUNK_END:
		strncpy (buf, "\r\n", sizeof (bostream->priv->buf));
		len = 2;
		bostream->priv->chunked_state = SOUP_BODY_OUTPUT_STREAM_STATE_DONE;
		break;

	case SOUP_BODY_OUTPUT_STREAM_STATE_TRAILERS:
		strncpy (buf, "\r\n", sizeof (bostream->priv->buf));
		len = 2;
		bostream->priv->chunked_state = SOUP_BODY_OUTPUT_STREAM_STATE_DONE;
		break;

	case SOUP_BODY_OUTPUT_STREAM_STATE_DONE:
		bostream->priv->chunked_state = SOUP_BODY_OUTPUT_STREAM_STATE_CHUNK_SIZE;
		return count;
	}

	goto again;
}

static gssize
soup_body_output_stream_write_fn (GOutputStream  *stream,
				  const void     *buffer,
				  gsize           count,
				  GCancellable   *cancellable,
				  GError        **error)
{
	SoupBodyOutputStream *bostream = SOUP_BODY_OUTPUT_STREAM (stream);

	if (bostream->priv->eof)
		return count;

	switch (bostream->priv->encoding) {
	case SOUP_ENCODING_CHUNKED:
		return soup_body_output_stream_write_chunked (bostream, buffer, count,
							      cancellable, error);

	default:
		return soup_body_output_stream_write_raw (bostream, buffer, count,
							  cancellable, error);
	}
}

static gboolean
soup_body_output_stream_close_fn (GOutputStream  *stream,
				  GCancellable   *cancellable,
				  GError        **error)
{
	SoupBodyOutputStream *bostream = SOUP_BODY_OUTPUT_STREAM (stream);

	if (bostream->priv->encoding == SOUP_ENCODING_CHUNKED) {
		if (soup_body_output_stream_write_chunked (bostream, NULL, 0, cancellable, error) == -1)
			return FALSE;
	}

	return G_OUTPUT_STREAM_CLASS (soup_body_output_stream_parent_class)->close_fn (stream, cancellable, error);
}

static gboolean
soup_body_output_stream_is_writable (GPollableOutputStream *stream)
{
	SoupBodyOutputStream *bostream = SOUP_BODY_OUTPUT_STREAM (stream);

	return bostream->priv->eof ||
		g_pollable_output_stream_is_writable (G_POLLABLE_OUTPUT_STREAM (bostream->priv->base_stream));
}

static GSource *
soup_body_output_stream_create_source (GPollableOutputStream *stream,
				       GCancellable *cancellable)
{
	SoupBodyOutputStream *bostream = SOUP_BODY_OUTPUT_STREAM (stream);
	GSource *base_source, *pollable_source;

	if (bostream->priv->eof)
		base_source = g_timeout_source_new (0);
	else
		base_source = g_pollable_output_stream_create_source (G_POLLABLE_OUTPUT_STREAM (bostream->priv->base_stream), cancellable);
	g_source_set_dummy_callback (base_source);

	pollable_source = g_pollable_source_new (G_OBJECT (stream));
	g_source_add_child_source (pollable_source, base_source);
	g_source_unref (base_source);

	return pollable_source;
}

static void
soup_body_output_stream_class_init (SoupBodyOutputStreamClass *stream_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (stream_class);
	GOutputStreamClass *output_stream_class = G_OUTPUT_STREAM_CLASS (stream_class);

	g_type_class_add_private (stream_class, sizeof (SoupBodyOutputStreamPrivate));

	object_class->constructed = soup_body_output_stream_constructed;
	object_class->set_property = soup_body_output_stream_set_property;
	object_class->get_property = soup_body_output_stream_get_property;

	output_stream_class->write_fn = soup_body_output_stream_write_fn;
	output_stream_class->close_fn = soup_body_output_stream_close_fn;

	g_object_class_install_property (
		object_class, PROP_ENCODING,
		g_param_spec_enum ("encoding",
				   "Encoding",
				   "Message body encoding",
				   SOUP_TYPE_ENCODING,
				   SOUP_ENCODING_NONE,
				   G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (
		object_class, PROP_CONTENT_LENGTH,
		g_param_spec_uint64 ("content-length",
				     "Content-Length",
				     "Message body Content-Length",
				     0, G_MAXUINT64, 0,
				     G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}

static void
soup_body_output_stream_pollable_init (GPollableOutputStreamInterface *pollable_interface,
				       gpointer interface_data)
{
	pollable_interface->is_writable = soup_body_output_stream_is_writable;
	pollable_interface->create_source = soup_body_output_stream_create_source;
}

GOutputStream *
soup_body_output_stream_new (GOutputStream *base_stream,
			     SoupEncoding   encoding,
			     goffset        content_length)
{
	return g_object_new (SOUP_TYPE_BODY_OUTPUT_STREAM,
			     "base-stream", base_stream,
			     "close-base-stream", FALSE,
			     "encoding", encoding,
			     "content-length", content_length,
			     NULL);
}
