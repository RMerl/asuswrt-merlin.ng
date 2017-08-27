/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-multipart-input-stream.c
 *
 * Copyright (C) 2012 Collabora Ltd.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "soup-body-input-stream.h"
#include "soup-filter-input-stream.h"
#include "soup-enum-types.h"
#include "soup-message.h"
#include "soup-message-private.h"
#include "soup-multipart-input-stream.h"

#define RESPONSE_BLOCK_SIZE 8192

/**
 * SECTION:soup-multipart-input-stream
 * @short_description: Multipart input handling stream
 *
 * This adds support for the multipart responses. For handling the
 * multiple parts the user needs to wrap the #GInputStream obtained by
 * sending the request with a #SoupMultipartInputStream and use
 * soup_multipart_input_stream_next_part() before reading. Responses
 * which are not wrapped will be treated like non-multipart responses.
 *
 * Note that although #SoupMultipartInputStream is a #GInputStream,
 * you should not read directly from it, and the results are undefined
 * if you do.
 *
 * Since: 2.40
 **/

static void soup_multipart_input_stream_pollable_init (GPollableInputStreamInterface *pollable_interface, gpointer interface_data);

G_DEFINE_TYPE_WITH_CODE (SoupMultipartInputStream, soup_multipart_input_stream, G_TYPE_FILTER_INPUT_STREAM,
			 G_IMPLEMENT_INTERFACE (G_TYPE_POLLABLE_INPUT_STREAM,
						soup_multipart_input_stream_pollable_init))

enum {
	PROP_0,

	PROP_MESSAGE,
};

struct _SoupMultipartInputStreamPrivate {
	SoupMessage	        *msg;

	gboolean	         done_with_part;

	GByteArray	        *meta_buf;
	SoupMessageHeaders      *current_headers;

	SoupFilterInputStream   *base_stream;

	char		        *boundary;
	gsize		         boundary_size;

	goffset		        remaining_bytes;
};

static void
soup_multipart_input_stream_dispose (GObject *object)
{
	SoupMultipartInputStream *multipart = SOUP_MULTIPART_INPUT_STREAM (object);

	g_clear_object (&multipart->priv->msg);
	g_clear_object (&multipart->priv->base_stream);

	G_OBJECT_CLASS (soup_multipart_input_stream_parent_class)->dispose (object);
}

static void
soup_multipart_input_stream_finalize (GObject *object)
{
	SoupMultipartInputStream *multipart = SOUP_MULTIPART_INPUT_STREAM (object);

	g_free (multipart->priv->boundary);

	if (multipart->priv->meta_buf)
		g_clear_pointer (&multipart->priv->meta_buf, g_byte_array_unref);

	G_OBJECT_CLASS (soup_multipart_input_stream_parent_class)->finalize (object);
}

static void
soup_multipart_input_stream_set_property (GObject *object, guint prop_id,
					  const GValue *value, GParamSpec *pspec)
{
	SoupMultipartInputStream *multipart = SOUP_MULTIPART_INPUT_STREAM (object);

	switch (prop_id) {
	case PROP_MESSAGE:
		multipart->priv->msg = g_value_dup_object (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
soup_multipart_input_stream_get_property (GObject *object, guint prop_id,
					  GValue *value, GParamSpec *pspec)
{
	SoupMultipartInputStream *multipart = SOUP_MULTIPART_INPUT_STREAM (object);

	switch (prop_id) {
	case PROP_MESSAGE:
		g_value_set_object (value, multipart->priv->msg);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static gssize
soup_multipart_input_stream_read_real (GInputStream	*stream,
				       void		*buffer,
				       gsize		 count,
				       gboolean          blocking,
				       GCancellable	*cancellable,
				       GError          **error)
{
	SoupMultipartInputStream *multipart = SOUP_MULTIPART_INPUT_STREAM (stream);
	SoupMultipartInputStreamPrivate *priv = multipart->priv;
	gboolean got_boundary = FALSE;
	gssize nread = 0;
	guint8 *buf;

	g_return_val_if_fail (priv->boundary != NULL, -1);

	/* If we have received a Content-Length, and are not yet close to the end of
	 * the part, let's not look for the boundary for now. This optimization is
	 * necessary for keeping CPU usage civil.
	 */
	if (priv->remaining_bytes > priv->boundary_size) {
		goffset bytes_to_read = MIN (count, priv->remaining_bytes - priv->boundary_size);

		nread = g_pollable_stream_read (G_INPUT_STREAM (priv->base_stream),
						buffer, bytes_to_read, blocking,
						cancellable, error);

		if (nread > 0)
			priv->remaining_bytes -= nread;

		return nread;
	}

	if (priv->done_with_part)
		return 0;

	nread = soup_filter_input_stream_read_until (priv->base_stream, buffer, count,
						     priv->boundary, priv->boundary_size,
						     blocking, FALSE, &got_boundary,
						     cancellable, error);

	if (nread <= 0)
		return nread;

	if (!got_boundary)
		return nread;

	priv->done_with_part = TRUE;

	/* Ignore the newline that preceded the boundary. */
	if (nread == 1) {
		buf = ((guint8*)buffer);
		if (!memcmp (buf, "\n", 1))
			nread -= 1;
	} else {
		buf = ((guint8*)buffer) + nread - 2;
		if (!memcmp (buf, "\r\n", 2))
			nread -= 2;
		else if (!memcmp (buf, "\n", 1))
			nread -= 1;
	}

	return nread;
}

static gssize
soup_multipart_input_stream_read (GInputStream	*stream,
				  void		*buffer,
				  gsize		 count,
				  GCancellable	*cancellable,
				  GError       **error)
{
	return soup_multipart_input_stream_read_real (stream, buffer, count,
						      TRUE, cancellable, error);
}

static void
soup_multipart_input_stream_init (SoupMultipartInputStream *multipart)
{
	SoupMultipartInputStreamPrivate *priv;
	priv = multipart->priv = G_TYPE_INSTANCE_GET_PRIVATE (multipart,
							      SOUP_TYPE_MULTIPART_INPUT_STREAM,
							      SoupMultipartInputStreamPrivate);

	priv->meta_buf = g_byte_array_sized_new (RESPONSE_BLOCK_SIZE);
	priv->done_with_part = FALSE;
}

static void
soup_multipart_input_stream_constructed (GObject *object)
{
	SoupMultipartInputStream *multipart;
	SoupMultipartInputStreamPrivate *priv;
	GInputStream *base_stream;
	const char* boundary;
	GHashTable *params = NULL;

	multipart = SOUP_MULTIPART_INPUT_STREAM (object);
	priv = multipart->priv;

	base_stream = G_FILTER_INPUT_STREAM (multipart)->base_stream;
	priv->base_stream = SOUP_FILTER_INPUT_STREAM (soup_filter_input_stream_new (base_stream));

	soup_message_headers_get_content_type (priv->msg->response_headers,
					       &params);

	boundary = g_hash_table_lookup (params, "boundary");
	if (boundary) {
		if (g_str_has_prefix (boundary, "--"))
			priv->boundary = g_strdup (boundary);
		else
			priv->boundary = g_strdup_printf ("--%s", boundary);

		priv->boundary_size = strlen (priv->boundary);
	} else {
		g_warning ("No boundary found in message tagged as multipart.");
	}

	g_hash_table_destroy (params);

	if (G_OBJECT_CLASS (soup_multipart_input_stream_parent_class)->constructed)
		G_OBJECT_CLASS (soup_multipart_input_stream_parent_class)->constructed (object);
}

static gboolean
soup_multipart_input_stream_is_readable (GPollableInputStream *stream)
{
	SoupMultipartInputStream *multipart = SOUP_MULTIPART_INPUT_STREAM (stream);
	SoupMultipartInputStreamPrivate *priv = multipart->priv;

	return g_pollable_input_stream_is_readable (G_POLLABLE_INPUT_STREAM (priv->base_stream));
}

static gssize
soup_multipart_input_stream_read_nonblocking (GPollableInputStream  *stream,
					      void                  *buffer,
					      gsize                  count,
					      GError               **error)
{
	SoupMultipartInputStream *multipart = SOUP_MULTIPART_INPUT_STREAM (stream);

	return soup_multipart_input_stream_read_real (G_INPUT_STREAM (multipart),
						      buffer, count,
						      FALSE, NULL, error);
}

static GSource *
soup_multipart_input_stream_create_source (GPollableInputStream *stream,
					   GCancellable         *cancellable)
{
	SoupMultipartInputStream *multipart = SOUP_MULTIPART_INPUT_STREAM (stream);
	SoupMultipartInputStreamPrivate *priv = multipart->priv;
	GSource *base_source, *pollable_source;

	base_source = g_pollable_input_stream_create_source (G_POLLABLE_INPUT_STREAM (priv->base_stream), cancellable);

	pollable_source = g_pollable_source_new_full (stream, base_source, cancellable);
	g_source_unref (base_source);

	return pollable_source;
}

static void
soup_multipart_input_stream_class_init (SoupMultipartInputStreamClass *multipart_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (multipart_class);
	GInputStreamClass *input_stream_class =
		G_INPUT_STREAM_CLASS (multipart_class);

	g_type_class_add_private (multipart_class, sizeof (SoupMultipartInputStreamPrivate));

	object_class->dispose = soup_multipart_input_stream_dispose;
	object_class->finalize = soup_multipart_input_stream_finalize;
	object_class->constructed = soup_multipart_input_stream_constructed;
	object_class->set_property = soup_multipart_input_stream_set_property;
	object_class->get_property = soup_multipart_input_stream_get_property;

	input_stream_class->read_fn = soup_multipart_input_stream_read;

	g_object_class_install_property (
		object_class, PROP_MESSAGE,
		g_param_spec_object ("message",
				     "Message",
				     "The SoupMessage",
				     SOUP_TYPE_MESSAGE,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

}

static void
soup_multipart_input_stream_pollable_init (GPollableInputStreamInterface *pollable_interface,
					   gpointer                       interface_data)
{
	pollable_interface->is_readable = soup_multipart_input_stream_is_readable;
	pollable_interface->read_nonblocking = soup_multipart_input_stream_read_nonblocking;
	pollable_interface->create_source = soup_multipart_input_stream_create_source;
}

static void
soup_multipart_input_stream_parse_headers (SoupMultipartInputStream *multipart)
{
	SoupMultipartInputStreamPrivate *priv = multipart->priv;
	gboolean success;

	priv->current_headers = soup_message_headers_new (SOUP_MESSAGE_HEADERS_MULTIPART);

	/* The part lacks headers, but is there. */
	if (!priv->meta_buf->len)
		return;

	success = soup_headers_parse ((const char*) priv->meta_buf->data,
				      (int) priv->meta_buf->len,
				      priv->current_headers);

	if (success)
		priv->remaining_bytes = soup_message_headers_get_content_length (priv->current_headers);
	else
		g_clear_pointer (&priv->current_headers, soup_message_headers_free);

	g_byte_array_remove_range (priv->meta_buf, 0, priv->meta_buf->len);
}

static gboolean
soup_multipart_input_stream_read_headers (SoupMultipartInputStream  *multipart,
					  GCancellable		    *cancellable,
					  GError		   **error)
{
	SoupMultipartInputStreamPrivate *priv = multipart->priv;
	guchar read_buf[RESPONSE_BLOCK_SIZE];
	guchar *buf;
	gboolean got_boundary = FALSE;
	gboolean got_lf = FALSE;
	gssize nread = 0;

	g_return_val_if_fail (priv->boundary != NULL, TRUE);

	g_clear_pointer (&priv->current_headers, soup_message_headers_free);

	while (1) {
		nread = soup_filter_input_stream_read_line (priv->base_stream, read_buf, sizeof (read_buf),
							    /* blocking */ TRUE, &got_lf, cancellable, error);

		if (nread <= 0)
			break;

		g_byte_array_append (priv->meta_buf, read_buf, nread);

		/* Need to do this boundary check before checking for the line feed, since we
		 * may get the multipart end indicator without getting a new line.
		 */
		if (!got_boundary &&
		    !strncmp ((char *)priv->meta_buf->data,
			      priv->boundary,
			      priv->boundary_size)) {
			got_boundary = TRUE;

			/* Now check for possible multipart termination. */
			buf = &read_buf[nread - 4];
			if ((nread >= 4 && !memcmp (buf, "--\r\n", 4)) ||
			    (nread >= 3 && !memcmp (buf + 1, "--\n", 3)) ||
			    (nread >= 3 && !memcmp (buf + 2, "--", 2))) {
				g_byte_array_set_size (priv->meta_buf, 0);
				return FALSE;
			}
		}

		g_return_val_if_fail (got_lf, FALSE);

		/* Discard pre-boundary lines. */
		if (!got_boundary) {
			g_byte_array_set_size (priv->meta_buf, 0);
			continue;
		}

		if (nread == 1 &&
		    priv->meta_buf->len >= 2 &&
		    !strncmp ((char *)priv->meta_buf->data +
			      priv->meta_buf->len - 2,
			      "\n\n", 2))
			break;
		else if (nread == 2 &&
			 priv->meta_buf->len >= 3 &&
			 !strncmp ((char *)priv->meta_buf->data +
				   priv->meta_buf->len - 3,
				   "\n\r\n", 3))
			break;
	}

	return TRUE;
}

/* Public APIs */

/**
 * soup_multipart_input_stream_new:
 * @msg: the #SoupMessage the response is related to.
 * @base_stream: the #GInputStream returned by sending the request.
 *
 * Creates a new #SoupMultipartInputStream that wraps the
 * #GInputStream obtained by sending the #SoupRequest. Reads should
 * not be done directly through this object, use the input streams
 * returned by soup_multipart_input_stream_next_part() or its async
 * counterpart instead.
 *
 * Returns: a new #SoupMultipartInputStream
 *
 * Since: 2.40
 **/
SoupMultipartInputStream *
soup_multipart_input_stream_new (SoupMessage  *msg,
				 GInputStream *base_stream)
{
	return g_object_new (SOUP_TYPE_MULTIPART_INPUT_STREAM,
			     "message", msg,
			     "base-stream", base_stream,
			     NULL);
}

/**
 * soup_multipart_input_stream_next_part:
 * @multipart: the #SoupMultipartInputStream
 * @cancellable: a #GCancellable
 * @error: a #GError
 *
 * Obtains an input stream for the next part. When dealing with a
 * multipart response the input stream needs to be wrapped in a
 * #SoupMultipartInputStream and this function or its async
 * counterpart need to be called to obtain the first part for
 * reading.
 *
 * After calling this function,
 * soup_multipart_input_stream_get_headers() can be used to obtain the
 * headers for the first part. A read of 0 bytes indicates the end of
 * the part; a new call to this function should be done at that point,
 * to obtain the next part.
 *
 * Return value: (transfer full): a new #GInputStream, or %NULL if
 * there are no more parts
 *
 * Since: 2.40
 */
GInputStream *
soup_multipart_input_stream_next_part (SoupMultipartInputStream  *multipart,
				       GCancellable	         *cancellable,
				       GError                   **error)
{
	if (!soup_multipart_input_stream_read_headers (multipart, cancellable, error))
		return NULL;

	soup_multipart_input_stream_parse_headers (multipart);

	multipart->priv->done_with_part = FALSE;

	return G_INPUT_STREAM (g_object_new (SOUP_TYPE_BODY_INPUT_STREAM,
					     "base-stream", G_INPUT_STREAM (multipart),
					     "close-base-stream", FALSE,
					     "encoding", SOUP_ENCODING_EOF,
					     NULL));

}

static void
soup_multipart_input_stream_next_part_thread (GTask        *task,
					      gpointer      object,
					      gpointer      task_data,
					      GCancellable *cancellable)
{
	SoupMultipartInputStream *multipart = SOUP_MULTIPART_INPUT_STREAM (object);
	GError *error = NULL;
	GInputStream *new_stream;

	new_stream = soup_multipart_input_stream_next_part (multipart, cancellable, &error);

	g_input_stream_clear_pending (G_INPUT_STREAM (multipart));

	if (error)
		g_task_return_error (task, error);
	else
		g_task_return_pointer (task, new_stream, g_object_unref);
}

/**
 * soup_multipart_input_stream_next_part_async:
 * @multipart: the #SoupMultipartInputStream.
 * @io_priority: the I/O priority for the request.
 * @cancellable: a #GCancellable.
 * @callback: callback to call when request is satisfied.
 *
 * Obtains a #GInputStream for the next request. See
 * soup_multipart_input_stream_next_part() for details on the
 * workflow.
 *
 * Return value: a new #GInputStream, or %NULL if there are no more
 * parts
 *
 * Since: 2.40
 */
void
soup_multipart_input_stream_next_part_async (SoupMultipartInputStream *multipart,
					     int                       io_priority,
					     GCancellable	      *cancellable,
					     GAsyncReadyCallback       callback,
					     gpointer		       data)
{
	GInputStream *stream = G_INPUT_STREAM (multipart);
	GTask *task;
	GError *error = NULL;

	g_return_if_fail (SOUP_IS_MULTIPART_INPUT_STREAM (multipart));

	task = g_task_new (multipart, cancellable, callback, data);
	g_task_set_priority (task, io_priority);

	if (!g_input_stream_set_pending (stream, &error)) {
		g_task_return_error (task, error);
		g_object_unref (task);
		return;
	}

	g_task_run_in_thread (task, soup_multipart_input_stream_next_part_thread);
	g_object_unref (task);
}

/**
 * soup_multipart_input_stream_next_part_finish:
 * @multipart: a #SoupMultipartInputStream.
 * @result: a #GAsyncResult.
 * @error: a #GError location to store any error, or NULL to ignore.
 *
 * Finishes an asynchronous request for the next part.
 *
 * Return value: (transfer full): a newly created #GInputStream for
 * reading the next part or %NULL if there are no more parts.
 *
 * Since: 2.40
 */
GInputStream *
soup_multipart_input_stream_next_part_finish (SoupMultipartInputStream	*multipart,
					      GAsyncResult		*result,
					      GError		       **error)
{
	g_return_val_if_fail (g_task_is_valid (result, multipart), FALSE);

	return g_task_propagate_pointer (G_TASK (result), error);
}

/**
 * soup_multipart_input_stream_get_headers:
 * @multipart: a #SoupMultipartInputStream.
 *
 * Obtains the headers for the part currently being processed. Note
 * that the #SoupMessageHeaders that are returned are owned by the
 * #SoupMultipartInputStream and will be replaced when a call is made
 * to soup_multipart_input_stream_next_part() or its async
 * counterpart, so if keeping the headers is required, a copy must be
 * made.
 *
 * Note that if a part had no headers at all an empty #SoupMessageHeaders
 * will be returned.
 *
 * Return value: (transfer none): a #SoupMessageHeaders containing the headers
 * for the part currently being processed or %NULL if the headers failed to
 * parse.
 *
 * Since: 2.40
 */
SoupMessageHeaders *
soup_multipart_input_stream_get_headers (SoupMultipartInputStream *multipart)
{
	return multipart->priv->current_headers;
}
