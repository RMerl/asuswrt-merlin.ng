/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-filter-input-stream.c
 *
 * Copyright 2012 Red Hat, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "soup-filter-input-stream.h"
#include "soup.h"

/* This is essentially a subset of GDataInputStream, except that we
 * can do the equivalent of "fill_nonblocking()" on it. (We could use
 * an actual GDataInputStream, and implement the nonblocking semantics
 * via fill_async(), but that would be more work...)
 */

struct _SoupFilterInputStreamPrivate {
	GByteArray *buf;
};

static void soup_filter_input_stream_pollable_init (GPollableInputStreamInterface *pollable_interface, gpointer interface_data);

G_DEFINE_TYPE_WITH_CODE (SoupFilterInputStream, soup_filter_input_stream, G_TYPE_FILTER_INPUT_STREAM,
			 G_IMPLEMENT_INTERFACE (G_TYPE_POLLABLE_INPUT_STREAM,
						soup_filter_input_stream_pollable_init))

static void
soup_filter_input_stream_init (SoupFilterInputStream *stream)
{
	stream->priv = G_TYPE_INSTANCE_GET_PRIVATE (stream,
						    SOUP_TYPE_FILTER_INPUT_STREAM,
						    SoupFilterInputStreamPrivate);
}

static void
soup_filter_input_stream_finalize (GObject *object)
{
	SoupFilterInputStream *fstream = SOUP_FILTER_INPUT_STREAM (object);

	g_clear_pointer (&fstream->priv->buf, g_byte_array_unref);

	G_OBJECT_CLASS (soup_filter_input_stream_parent_class)->finalize (object);
}

static gssize
read_from_buf (SoupFilterInputStream *fstream, gpointer buffer, gsize count)
{
	GByteArray *buf = fstream->priv->buf;

	if (buf->len < count)
		count = buf->len;
	memcpy (buffer, buf->data, count);

	if (count == buf->len) {
		g_byte_array_free (buf, TRUE);
		fstream->priv->buf = NULL;
	} else {
		memmove (buf->data, buf->data + count,
			 buf->len - count);
		g_byte_array_set_size (buf, buf->len - count);
	}

	return count;
}

static gssize
soup_filter_input_stream_read_fn (GInputStream  *stream,
				  void          *buffer,
				  gsize          count,
				  GCancellable  *cancellable,
				  GError       **error)
{
	SoupFilterInputStream *fstream = SOUP_FILTER_INPUT_STREAM (stream);

	if (fstream->priv->buf) {
		return read_from_buf (fstream, buffer, count);
	} else {
		return g_pollable_stream_read (G_FILTER_INPUT_STREAM (fstream)->base_stream,
					       buffer, count,
					       TRUE, cancellable, error);
	}
}

static gboolean
soup_filter_input_stream_is_readable (GPollableInputStream *stream)
{
	SoupFilterInputStream *fstream = SOUP_FILTER_INPUT_STREAM (stream);

	if (fstream->priv->buf)
		return TRUE;
	else
		return g_pollable_input_stream_is_readable (G_POLLABLE_INPUT_STREAM (G_FILTER_INPUT_STREAM (fstream)->base_stream));
}

static gssize
soup_filter_input_stream_read_nonblocking (GPollableInputStream  *stream,
					   void                  *buffer,
					   gsize                  count,
					   GError               **error)
{
	SoupFilterInputStream *fstream = SOUP_FILTER_INPUT_STREAM (stream);

	if (fstream->priv->buf) {
		return read_from_buf (fstream, buffer, count);
	} else {
		return g_pollable_stream_read (G_FILTER_INPUT_STREAM (fstream)->base_stream,
					       buffer, count,
					       FALSE, NULL, error);
	}
}

static GSource *
soup_filter_input_stream_create_source (GPollableInputStream *stream,
					GCancellable         *cancellable)
{
	SoupFilterInputStream *fstream = SOUP_FILTER_INPUT_STREAM (stream);
	GSource *base_source, *pollable_source;

	if (fstream->priv->buf)
		base_source = g_timeout_source_new (0);
	else
		base_source = g_pollable_input_stream_create_source (G_POLLABLE_INPUT_STREAM (G_FILTER_INPUT_STREAM (fstream)->base_stream), cancellable);

	g_source_set_dummy_callback (base_source);
	pollable_source = g_pollable_source_new (G_OBJECT (stream));
	g_source_add_child_source (pollable_source, base_source);
	g_source_unref (base_source);

	return pollable_source;
}

static void
soup_filter_input_stream_class_init (SoupFilterInputStreamClass *stream_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (stream_class);
	GInputStreamClass *input_stream_class = G_INPUT_STREAM_CLASS (stream_class);

	g_type_class_add_private (stream_class, sizeof (SoupFilterInputStreamPrivate));

	object_class->finalize = soup_filter_input_stream_finalize;

	input_stream_class->read_fn = soup_filter_input_stream_read_fn;
}

static void
soup_filter_input_stream_pollable_init (GPollableInputStreamInterface *pollable_interface,
					gpointer                       interface_data)
{
	pollable_interface->is_readable = soup_filter_input_stream_is_readable;
	pollable_interface->read_nonblocking = soup_filter_input_stream_read_nonblocking;
	pollable_interface->create_source = soup_filter_input_stream_create_source;
}

GInputStream *
soup_filter_input_stream_new (GInputStream *base_stream)
{
	return g_object_new (SOUP_TYPE_FILTER_INPUT_STREAM,
			     "base-stream", base_stream,
			     "close-base-stream", FALSE,
			     NULL);
}

gssize
soup_filter_input_stream_read_line (SoupFilterInputStream  *fstream,
				    void                   *buffer,
				    gsize                   length,
				    gboolean                blocking,
				    gboolean               *got_line,
				    GCancellable           *cancellable,
				    GError                **error)
{
	return soup_filter_input_stream_read_until (fstream, buffer, length,
						    "\n", 1, blocking,
						    TRUE, got_line,
						    cancellable, error);
}

gssize
soup_filter_input_stream_read_until (SoupFilterInputStream  *fstream,
				     void                   *buffer,
				     gsize                   length,
				     const void             *boundary,
				     gsize                   boundary_length,
				     gboolean                blocking,
				     gboolean                include_boundary,
				     gboolean               *got_boundary,
				     GCancellable           *cancellable,
				     GError                **error)
{
	gssize nread;
	guint8 *p, *buf, *end;
	gboolean eof = FALSE;

	g_return_val_if_fail (SOUP_IS_FILTER_INPUT_STREAM (fstream), -1);
	g_return_val_if_fail (!include_boundary || (boundary_length < length), -1);

	*got_boundary = FALSE;

	if (!fstream->priv->buf || fstream->priv->buf->len < boundary_length) {
		guint prev_len;

	fill_buffer:
		if (!fstream->priv->buf)
			fstream->priv->buf = g_byte_array_new ();
		prev_len = fstream->priv->buf->len;
		g_byte_array_set_size (fstream->priv->buf, length);
		buf = fstream->priv->buf->data;

		nread = g_pollable_stream_read (G_FILTER_INPUT_STREAM (fstream)->base_stream,
						buf + prev_len, length - prev_len,
						blocking,
						cancellable, error);
		if (nread <= 0) {
			if (prev_len)
				fstream->priv->buf->len = prev_len;
			else {
				g_byte_array_free (fstream->priv->buf, TRUE);
				fstream->priv->buf = NULL;
			}

			if (nread == 0 && prev_len)
				eof = TRUE;
			else
				return nread;
		} else
			fstream->priv->buf->len = prev_len + nread;
	} else
		buf = fstream->priv->buf->data;

	/* Scan for the boundary */
	end = buf + fstream->priv->buf->len;
	if (!eof)
		end -= boundary_length;
	for (p = buf; p <= end; p++) {
		if (*p == *(guint8*)boundary &&
		    !memcmp (p, boundary, boundary_length)) {
			if (include_boundary)
				p += boundary_length;
			*got_boundary = TRUE;
			break;
		}
	}

	if (!*got_boundary && fstream->priv->buf->len < length && !eof)
		goto fill_buffer;

	/* Return everything up to 'p' (which is either just after the boundary if
	 * include_boundary is TRUE, just before the boundary if include_boundary is
	 * FALSE, @boundary_len - 1 bytes before the end of the buffer, or end-of-
	 * file).
	 */
	return read_from_buf (fstream, buffer, p - buf);
}
