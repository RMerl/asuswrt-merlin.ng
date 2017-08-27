/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-body-input-stream.c
 *
 * Copyright 2012 Red Hat, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include <glib/gi18n-lib.h>

#include "soup-body-input-stream.h"
#include "soup.h"
#include "soup-filter-input-stream.h"
#include "soup-marshal.h"

typedef enum {
	SOUP_BODY_INPUT_STREAM_STATE_CHUNK_SIZE,
	SOUP_BODY_INPUT_STREAM_STATE_CHUNK_END,
	SOUP_BODY_INPUT_STREAM_STATE_CHUNK,
	SOUP_BODY_INPUT_STREAM_STATE_TRAILERS,
	SOUP_BODY_INPUT_STREAM_STATE_DONE
} SoupBodyInputStreamState;

struct _SoupBodyInputStreamPrivate {
	GInputStream *base_stream;

	SoupEncoding  encoding;
	goffset       read_length;
	SoupBodyInputStreamState chunked_state;
	gboolean      eof;

	goffset       pos;
};

enum {
	CLOSED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

enum {
	PROP_0,

	PROP_ENCODING,
	PROP_CONTENT_LENGTH
};

static void soup_body_input_stream_pollable_init (GPollableInputStreamInterface *pollable_interface, gpointer interface_data);
static void soup_body_input_stream_seekable_init (GSeekableIface *seekable_interface);

G_DEFINE_TYPE_WITH_CODE (SoupBodyInputStream, soup_body_input_stream, G_TYPE_FILTER_INPUT_STREAM,
			 G_IMPLEMENT_INTERFACE (G_TYPE_POLLABLE_INPUT_STREAM,
						soup_body_input_stream_pollable_init)
			 G_IMPLEMENT_INTERFACE (G_TYPE_SEEKABLE,
						soup_body_input_stream_seekable_init))

static void
soup_body_input_stream_init (SoupBodyInputStream *bistream)
{
	bistream->priv = G_TYPE_INSTANCE_GET_PRIVATE (bistream,
						      SOUP_TYPE_BODY_INPUT_STREAM,
						      SoupBodyInputStreamPrivate);
	bistream->priv->encoding = SOUP_ENCODING_NONE;
}

static void
soup_body_input_stream_constructed (GObject *object)
{
	SoupBodyInputStream *bistream = SOUP_BODY_INPUT_STREAM (object);

	bistream->priv->base_stream = g_filter_input_stream_get_base_stream (G_FILTER_INPUT_STREAM (bistream));

	if (bistream->priv->encoding == SOUP_ENCODING_NONE ||
	    (bistream->priv->encoding == SOUP_ENCODING_CONTENT_LENGTH &&
	     bistream->priv->read_length == 0))
		bistream->priv->eof = TRUE;
}

static void
soup_body_input_stream_set_property (GObject *object, guint prop_id,
				     const GValue *value, GParamSpec *pspec)
{
	SoupBodyInputStream *bistream = SOUP_BODY_INPUT_STREAM (object);

	switch (prop_id) {
	case PROP_ENCODING:
		bistream->priv->encoding = g_value_get_enum (value);
		if (bistream->priv->encoding == SOUP_ENCODING_CHUNKED)
			bistream->priv->chunked_state = SOUP_BODY_INPUT_STREAM_STATE_CHUNK_SIZE;
		break;
	case PROP_CONTENT_LENGTH:
		bistream->priv->read_length = g_value_get_int64 (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
soup_body_input_stream_get_property (GObject *object, guint prop_id,
				     GValue *value, GParamSpec *pspec)
{
	SoupBodyInputStream *bistream = SOUP_BODY_INPUT_STREAM (object);

	switch (prop_id) {
	case PROP_ENCODING:
		g_value_set_enum (value, bistream->priv->encoding);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static gssize
soup_body_input_stream_read_raw (SoupBodyInputStream  *bistream,
				 void                 *buffer,
				 gsize                 count,
				 gboolean              blocking,
				 GCancellable         *cancellable,
				 GError              **error)
{
	gssize nread;

	nread = g_pollable_stream_read (bistream->priv->base_stream,
					buffer, count,
					blocking,
					cancellable, error);
	if (nread == 0) {
		bistream->priv->eof = TRUE;
		if (bistream->priv->encoding != SOUP_ENCODING_EOF) {
			g_set_error_literal (error, G_IO_ERROR,
					     G_IO_ERROR_PARTIAL_INPUT,
					     _("Connection terminated unexpectedly"));
			return -1;
		}
	}
	return nread;
}

static gssize
soup_body_input_stream_read_chunked (SoupBodyInputStream  *bistream,
				     void                 *buffer,
				     gsize                 count,
				     gboolean              blocking,
				     GCancellable         *cancellable,
				     GError              **error)
{
	SoupFilterInputStream *fstream = SOUP_FILTER_INPUT_STREAM (bistream->priv->base_stream);
	char metabuf[128];
	gssize nread;
	gboolean got_line;

again:
	switch (bistream->priv->chunked_state) {
	case SOUP_BODY_INPUT_STREAM_STATE_CHUNK_SIZE:
		nread = soup_filter_input_stream_read_line (
			fstream, metabuf, sizeof (metabuf), blocking,
			&got_line, cancellable, error);
		if (nread <= 0)
			return nread;
		if (!got_line) {
			g_set_error_literal (error, G_IO_ERROR,
					     G_IO_ERROR_PARTIAL_INPUT,
					     _("Connection terminated unexpectedly"));
			return -1;
		}

		bistream->priv->read_length = strtoul (metabuf, NULL, 16);
		if (bistream->priv->read_length > 0)
			bistream->priv->chunked_state = SOUP_BODY_INPUT_STREAM_STATE_CHUNK;
		else
			bistream->priv->chunked_state = SOUP_BODY_INPUT_STREAM_STATE_TRAILERS;
		break;

	case SOUP_BODY_INPUT_STREAM_STATE_CHUNK:
		nread = soup_body_input_stream_read_raw (
			bistream, buffer,
			MIN (count, bistream->priv->read_length),
			blocking, cancellable, error);
		if (nread > 0) {
			bistream->priv->read_length -= nread;
			if (bistream->priv->read_length == 0)
				bistream->priv->chunked_state = SOUP_BODY_INPUT_STREAM_STATE_CHUNK_END;
		}
		return nread;

	case SOUP_BODY_INPUT_STREAM_STATE_CHUNK_END:
		nread = soup_filter_input_stream_read_line (
			SOUP_FILTER_INPUT_STREAM (bistream->priv->base_stream),
			metabuf, sizeof (metabuf), blocking,
			&got_line, cancellable, error);
		if (nread <= 0)
			return nread;
		if (!got_line) {
			g_set_error_literal (error, G_IO_ERROR,
					     G_IO_ERROR_PARTIAL_INPUT,
					     _("Connection terminated unexpectedly"));
			return -1;
		}

		bistream->priv->chunked_state = SOUP_BODY_INPUT_STREAM_STATE_CHUNK_SIZE;
		break;

	case SOUP_BODY_INPUT_STREAM_STATE_TRAILERS:
		nread = soup_filter_input_stream_read_line (
			fstream, buffer, count, blocking,
			&got_line, cancellable, error);
		if (nread <= 0)
			return nread;

		if (strncmp (buffer, "\r\n", nread) || strncmp (buffer, "\n", nread)) {
			bistream->priv->chunked_state = SOUP_BODY_INPUT_STREAM_STATE_DONE;
			bistream->priv->eof = TRUE;
		}
		break;

	case SOUP_BODY_INPUT_STREAM_STATE_DONE:
		return 0;
	}

	goto again;
}

static gssize
read_internal (GInputStream  *stream,
	       void          *buffer,
	       gsize          count,
	       gboolean       blocking,
	       GCancellable  *cancellable,
	       GError       **error)
{
	SoupBodyInputStream *bistream = SOUP_BODY_INPUT_STREAM (stream);
	gssize nread;

	if (bistream->priv->eof)
		return 0;

	switch (bistream->priv->encoding) {
	case SOUP_ENCODING_NONE:
		return 0;

	case SOUP_ENCODING_CHUNKED:
		return soup_body_input_stream_read_chunked (bistream, buffer, count,
							    blocking, cancellable, error);

	case SOUP_ENCODING_CONTENT_LENGTH:
	case SOUP_ENCODING_EOF:
		if (bistream->priv->read_length != -1) {
			count = MIN (count, bistream->priv->read_length);
			if (count == 0)
				return 0;
		}

		nread = soup_body_input_stream_read_raw (bistream, buffer, count,
							 blocking, cancellable, error);
		if (bistream->priv->read_length != -1 && nread > 0)
			bistream->priv->read_length -= nread;

		if (bistream->priv->encoding == SOUP_ENCODING_CONTENT_LENGTH)
			bistream->priv->pos += nread;
		return nread;

	default:
		g_return_val_if_reached (-1);
	}
}

static gssize
soup_body_input_stream_skip (GInputStream *stream,
			     gsize         count,
			     GCancellable *cancellable,
			     GError      **error)
{
	SoupBodyInputStreamPrivate *priv = SOUP_BODY_INPUT_STREAM(stream)->priv;
	gssize skipped;

	skipped = g_input_stream_skip (G_FILTER_INPUT_STREAM (stream)->base_stream,
				       MIN (count, priv->read_length),
				       cancellable, error);

	if (skipped != -1)
		priv->pos += skipped;

	return skipped;
}

static gssize
soup_body_input_stream_read_fn (GInputStream  *stream,
				void          *buffer,
				gsize          count,
				GCancellable  *cancellable,
				GError       **error)
{
	return read_internal (stream, buffer, count, TRUE,
			      cancellable, error);
}

static gboolean
soup_body_input_stream_close_fn (GInputStream  *stream,
				 GCancellable  *cancellable,
				 GError       **error)
{
	g_signal_emit (stream, signals[CLOSED], 0);

	return G_INPUT_STREAM_CLASS (soup_body_input_stream_parent_class)->close_fn (stream, cancellable, error);
}

static gboolean
soup_body_input_stream_is_readable (GPollableInputStream *stream)
{
	SoupBodyInputStream *bistream = SOUP_BODY_INPUT_STREAM (stream);

	return bistream->priv->eof ||
		g_pollable_input_stream_is_readable (G_POLLABLE_INPUT_STREAM (bistream->priv->base_stream));
}

static gboolean
soup_body_input_stream_can_poll (GPollableInputStream *pollable)
{
	GInputStream *base_stream = SOUP_BODY_INPUT_STREAM (pollable)->priv->base_stream;

	return G_IS_POLLABLE_INPUT_STREAM (base_stream) &&
		g_pollable_input_stream_can_poll (G_POLLABLE_INPUT_STREAM (base_stream));
}

static gssize
soup_body_input_stream_read_nonblocking (GPollableInputStream  *stream,
					 void                  *buffer,
					 gsize                  count,
					 GError               **error)
{
	return read_internal (G_INPUT_STREAM (stream), buffer, count, FALSE,
			      NULL, error);
}

static GSource *
soup_body_input_stream_create_source (GPollableInputStream *stream,
				      GCancellable *cancellable)
{
	SoupBodyInputStream *bistream = SOUP_BODY_INPUT_STREAM (stream);
	GSource *base_source, *pollable_source;

	if (bistream->priv->eof)
		base_source = g_timeout_source_new (0);
	else
		base_source = g_pollable_input_stream_create_source (G_POLLABLE_INPUT_STREAM (bistream->priv->base_stream), cancellable);
	g_source_set_dummy_callback (base_source);

	pollable_source = g_pollable_source_new (G_OBJECT (stream));
	g_source_add_child_source (pollable_source, base_source);
	g_source_unref (base_source);

	return pollable_source;
}

static void
soup_body_input_stream_class_init (SoupBodyInputStreamClass *stream_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (stream_class);
	GInputStreamClass *input_stream_class = G_INPUT_STREAM_CLASS (stream_class);

	g_type_class_add_private (stream_class, sizeof (SoupBodyInputStreamPrivate));

	object_class->constructed = soup_body_input_stream_constructed;
	object_class->set_property = soup_body_input_stream_set_property;
	object_class->get_property = soup_body_input_stream_get_property;

	input_stream_class->skip = soup_body_input_stream_skip;
	input_stream_class->read_fn = soup_body_input_stream_read_fn;
	input_stream_class->close_fn = soup_body_input_stream_close_fn;

	signals[CLOSED] =
		g_signal_new ("closed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
			      _soup_marshal_NONE__NONE,
			      G_TYPE_NONE, 0);

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
		g_param_spec_int64 ("content-length",
				    "Content-Length",
				    "Message body Content-Length",
				    -1, G_MAXINT64, -1,
				    G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}

static void
soup_body_input_stream_pollable_init (GPollableInputStreamInterface *pollable_interface,
				 gpointer interface_data)
{
	pollable_interface->can_poll = soup_body_input_stream_can_poll;
	pollable_interface->is_readable = soup_body_input_stream_is_readable;
	pollable_interface->read_nonblocking = soup_body_input_stream_read_nonblocking;
	pollable_interface->create_source = soup_body_input_stream_create_source;
}

static goffset
soup_body_input_stream_tell (GSeekable *seekable)
{
	return SOUP_BODY_INPUT_STREAM (seekable)->priv->pos;
}

static gboolean
soup_body_input_stream_can_seek (GSeekable *seekable)
{
	SoupBodyInputStreamPrivate *priv = SOUP_BODY_INPUT_STREAM (seekable)->priv;

	return priv->encoding == SOUP_ENCODING_CONTENT_LENGTH
		&& G_IS_SEEKABLE (priv->base_stream)
		&& g_seekable_can_seek (G_SEEKABLE (priv->base_stream));
}

static gboolean
soup_body_input_stream_seek (GSeekable     *seekable,
			     goffset        offset,
			     GSeekType      type,
			     GCancellable  *cancellable,
			     GError       **error)
{
	SoupBodyInputStreamPrivate *priv = SOUP_BODY_INPUT_STREAM (seekable)->priv;
	goffset position, end_position;

	end_position = priv->pos + priv->read_length;
	switch (type) {
	case G_SEEK_CUR:
		position = priv->pos + offset;
		break;
	case G_SEEK_SET:
		position = offset;
		break;
	case G_SEEK_END:
		position = end_position + offset;
		break;
	default:
		g_return_val_if_reached (FALSE);
	}

	if (position < 0 || position >= end_position) {
		g_set_error_literal (error,
				     G_IO_ERROR,
				     G_IO_ERROR_INVALID_ARGUMENT,
				     _("Invalid seek request"));
		return FALSE;
	}

	if (!g_seekable_seek (G_SEEKABLE (priv->base_stream), position - priv->pos,
			      G_SEEK_CUR, cancellable, error))
		return FALSE;

	priv->pos = position;

	return TRUE;
}

static gboolean
soup_body_input_stream_can_truncate (GSeekable *seekable)
{
	return FALSE;
}

static gboolean
soup_body_input_stream_truncate_fn (GSeekable     *seekable,
				    goffset        offset,
				    GCancellable  *cancellable,
				    GError       **error)
{
	g_set_error_literal (error,
			     G_IO_ERROR,
			     G_IO_ERROR_NOT_SUPPORTED,
			     _("Cannot truncate SoupBodyInputStream"));
	return FALSE;
}

static void
soup_body_input_stream_seekable_init (GSeekableIface *seekable_interface)
{
	seekable_interface->tell         = soup_body_input_stream_tell;
	seekable_interface->can_seek     = soup_body_input_stream_can_seek;
	seekable_interface->seek         = soup_body_input_stream_seek;
	seekable_interface->can_truncate = soup_body_input_stream_can_truncate;
	seekable_interface->truncate_fn  = soup_body_input_stream_truncate_fn;
}

GInputStream *
soup_body_input_stream_new (GInputStream *base_stream,
			    SoupEncoding  encoding,
			    goffset       content_length)
{
	if (encoding == SOUP_ENCODING_CHUNKED)
		g_return_val_if_fail (SOUP_IS_FILTER_INPUT_STREAM (base_stream), NULL);

	return g_object_new (SOUP_TYPE_BODY_INPUT_STREAM,
			     "base-stream", base_stream,
			     "close-base-stream", FALSE,
			     "encoding", encoding,
			     "content-length", content_length,
			     NULL);
}
