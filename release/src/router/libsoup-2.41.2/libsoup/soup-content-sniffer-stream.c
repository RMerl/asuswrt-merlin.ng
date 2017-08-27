/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-content-sniffer-stream.c
 *
 * Copyright (C) 2010 Red Hat, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "soup-content-sniffer-stream.h"
#include "soup.h"

static void soup_content_sniffer_stream_pollable_init (GPollableInputStreamInterface *pollable_interface, gpointer interface_data);

G_DEFINE_TYPE_WITH_CODE (SoupContentSnifferStream, soup_content_sniffer_stream, G_TYPE_FILTER_INPUT_STREAM,
			 G_IMPLEMENT_INTERFACE (G_TYPE_POLLABLE_INPUT_STREAM,
						soup_content_sniffer_stream_pollable_init))

enum {
	PROP_0,

	PROP_SNIFFER,
	PROP_MESSAGE,
};

struct _SoupContentSnifferStreamPrivate {
	SoupContentSniffer *sniffer;
	SoupMessage *msg;

	guchar *buffer;
	gsize buffer_size, buffer_nread;
	gboolean sniffing;
	GError *error;

	char *sniffed_type;
	GHashTable *sniffed_params;
};

static void
soup_content_sniffer_stream_finalize (GObject *object)
{
	SoupContentSnifferStream *sniffer = SOUP_CONTENT_SNIFFER_STREAM (object);

	g_clear_object (&sniffer->priv->sniffer);
	g_clear_object (&sniffer->priv->msg);
	g_free (sniffer->priv->buffer);
	g_clear_error (&sniffer->priv->error);
	g_free (sniffer->priv->sniffed_type);
	g_clear_pointer (&sniffer->priv->sniffed_params, g_hash_table_unref);

	G_OBJECT_CLASS (soup_content_sniffer_stream_parent_class)->finalize (object);
}

static void
soup_content_sniffer_stream_set_property (GObject *object, guint prop_id,
					  const GValue *value, GParamSpec *pspec)
{
	SoupContentSnifferStream *sniffer = SOUP_CONTENT_SNIFFER_STREAM (object);

	switch (prop_id) {
	case PROP_SNIFFER:
		sniffer->priv->sniffer = g_value_dup_object (value);
		/* FIXME: supposed to wait until after got-headers for this */
		sniffer->priv->buffer_size = soup_content_sniffer_get_buffer_size (sniffer->priv->sniffer);
		sniffer->priv->buffer = g_malloc (sniffer->priv->buffer_size);
		break;
	case PROP_MESSAGE:
		sniffer->priv->msg = g_value_dup_object (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
soup_content_sniffer_stream_get_property (GObject *object, guint prop_id,
					  GValue *value, GParamSpec *pspec)
{
	SoupContentSnifferStream *sniffer = SOUP_CONTENT_SNIFFER_STREAM (object);

	switch (prop_id) {
	case PROP_SNIFFER:
		g_value_set_object (value, sniffer->priv->sniffer);
		break;
	case PROP_MESSAGE:
		g_value_set_object (value, sniffer->priv->msg);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static gssize
read_and_sniff (GInputStream *stream, gboolean blocking,
		GCancellable *cancellable, GError **error)
{
	SoupContentSnifferStreamPrivate *priv = SOUP_CONTENT_SNIFFER_STREAM (stream)->priv;
	gssize nread;
	GError *my_error = NULL;
	SoupBuffer *buf;

	do {
		nread = g_pollable_stream_read (G_FILTER_INPUT_STREAM (stream)->base_stream,
						priv->buffer + priv->buffer_nread,
						priv->buffer_size - priv->buffer_nread,
						blocking, cancellable, &my_error);
		if (nread <= 0)
			break;
		priv->buffer_nread += nread;
	} while (priv->buffer_nread < priv->buffer_size);

	/* If we got EAGAIN or cancellation before filling the buffer,
	 * just return that right away. Likewise if we got any other
	 * error without ever reading any data. Otherwise, save the
	 * error to return after we're done sniffing.
	 */
	if (my_error) {
		if (g_error_matches (my_error, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK) ||
		    g_error_matches (my_error, G_IO_ERROR, G_IO_ERROR_CANCELLED) ||
		    priv->buffer_nread == 0) {
			g_propagate_error (error, my_error);
			return -1;
		} else
			priv->error = my_error;
	}

	/* Sniff, then return the data */
	buf = soup_buffer_new (SOUP_MEMORY_TEMPORARY, priv->buffer, priv->buffer_nread);
	priv->sniffed_type =
		soup_content_sniffer_sniff (priv->sniffer, priv->msg, buf,
					    &priv->sniffed_params);
	soup_buffer_free (buf);
	priv->sniffing = FALSE;

	return priv->buffer_nread;
}	

static gssize
read_internal (GInputStream  *stream,
	       void          *buffer,
	       gsize          count,
	       gboolean       blocking,
	       GCancellable  *cancellable,
	       GError       **error)
{
	SoupContentSnifferStream *sniffer = SOUP_CONTENT_SNIFFER_STREAM (stream);
	gssize nread;

	if (sniffer->priv->error) {
		g_propagate_error (error, sniffer->priv->error);
		sniffer->priv->error = NULL;
		return -1;
	}

	if (sniffer->priv->sniffing) {
		nread = read_and_sniff (stream, blocking, cancellable, error);
		if (nread <= 0)
			return nread;
	}

	if (sniffer->priv->buffer) {
		nread = MIN (count, sniffer->priv->buffer_nread);
		memcpy (buffer, sniffer->priv->buffer, nread);
		if (nread == sniffer->priv->buffer_nread) {
			g_free (sniffer->priv->buffer);
			sniffer->priv->buffer = NULL;
		} else {
			/* FIXME, inefficient */
			memmove (sniffer->priv->buffer,
				 sniffer->priv->buffer + nread,
				 sniffer->priv->buffer_nread - nread);
			sniffer->priv->buffer_nread -= nread;
		}
	} else {
		nread = g_pollable_stream_read (G_FILTER_INPUT_STREAM (stream)->base_stream,
						buffer, count, blocking,
						cancellable, error);
	}
	return nread;
}

static gssize
soup_content_sniffer_stream_read (GInputStream  *stream,
				  void          *buffer,
				  gsize          count,
				  GCancellable  *cancellable,
				  GError       **error)
{
	return read_internal (stream, buffer, count, TRUE,
			      cancellable, error);
}

static gssize
soup_content_sniffer_stream_skip (GInputStream  *stream,
				  gsize          count,
				  GCancellable  *cancellable,
				  GError       **error)
{
	SoupContentSnifferStream *sniffer = SOUP_CONTENT_SNIFFER_STREAM (stream);
	gssize nskipped;

	if (sniffer->priv->sniffing) {
		/* Read into the internal buffer... */
		nskipped = soup_content_sniffer_stream_read (stream, NULL, 0, cancellable, error);
		if (nskipped == -1)
			return -1;
		/* Now fall through */
	}

	if (sniffer->priv->buffer) {
		nskipped = MIN (count, sniffer->priv->buffer_nread);
		if (nskipped == sniffer->priv->buffer_nread) {
			g_free (sniffer->priv->buffer);
			sniffer->priv->buffer = NULL;
		} else {
			/* FIXME */
			memmove (sniffer->priv->buffer,
				 sniffer->priv->buffer + nskipped,
				 sniffer->priv->buffer_nread - nskipped);
			sniffer->priv->buffer_nread -= nskipped;
		}
	} else {
		nskipped = G_INPUT_STREAM_CLASS (soup_content_sniffer_stream_parent_class)->
			skip (stream, count, cancellable, error);
	}
	return nskipped;
}

static gboolean
soup_content_sniffer_stream_can_poll (GPollableInputStream *pollable)
{
	GInputStream *base_stream = G_FILTER_INPUT_STREAM (pollable)->base_stream;

	return G_IS_POLLABLE_INPUT_STREAM (base_stream) &&
		g_pollable_input_stream_can_poll (G_POLLABLE_INPUT_STREAM (base_stream));
}


static gboolean
soup_content_sniffer_stream_is_readable (GPollableInputStream *stream)
{
	SoupContentSnifferStream *sniffer = SOUP_CONTENT_SNIFFER_STREAM (stream);

	if (sniffer->priv->error ||
	    (!sniffer->priv->sniffing && sniffer->priv->buffer))
		return TRUE;

	return g_pollable_input_stream_is_readable (G_POLLABLE_INPUT_STREAM (G_FILTER_INPUT_STREAM (stream)->base_stream));
}

static gssize
soup_content_sniffer_stream_read_nonblocking (GPollableInputStream  *stream,
					      void                  *buffer,
					      gsize                  count,
					      GError               **error)
{
	return read_internal (G_INPUT_STREAM (stream), buffer, count,
			      FALSE, NULL, error);
}

static GSource *
soup_content_sniffer_stream_create_source (GPollableInputStream *stream,
					   GCancellable         *cancellable)
{
	SoupContentSnifferStream *sniffer = SOUP_CONTENT_SNIFFER_STREAM (stream);
	GSource *base_source, *pollable_source;

	if (sniffer->priv->error ||
	    (!sniffer->priv->sniffing && sniffer->priv->buffer))
		base_source = g_timeout_source_new (0);
	else
		base_source = g_pollable_input_stream_create_source (G_POLLABLE_INPUT_STREAM (G_FILTER_INPUT_STREAM (stream)->base_stream), cancellable);

	g_source_set_dummy_callback (base_source);
	pollable_source = g_pollable_source_new (G_OBJECT (stream));
	g_source_add_child_source (pollable_source, base_source);
	g_source_unref (base_source);

	return pollable_source;
}

static void
soup_content_sniffer_stream_init (SoupContentSnifferStream *sniffer)
{
	sniffer->priv = G_TYPE_INSTANCE_GET_PRIVATE (sniffer,
						     SOUP_TYPE_CONTENT_SNIFFER_STREAM,
						     SoupContentSnifferStreamPrivate);
	sniffer->priv->sniffing = TRUE;
}

static void
soup_content_sniffer_stream_class_init (SoupContentSnifferStreamClass *sniffer_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (sniffer_class);
	GInputStreamClass *input_stream_class =
		G_INPUT_STREAM_CLASS (sniffer_class);
 
	g_type_class_add_private (sniffer_class, sizeof (SoupContentSnifferStreamPrivate));

	object_class->finalize = soup_content_sniffer_stream_finalize;
	object_class->set_property = soup_content_sniffer_stream_set_property;
	object_class->get_property = soup_content_sniffer_stream_get_property;

	input_stream_class->read_fn = soup_content_sniffer_stream_read;
	input_stream_class->skip = soup_content_sniffer_stream_skip;

	g_object_class_install_property (
		object_class, PROP_SNIFFER,
		g_param_spec_object ("sniffer",
				     "Sniffer",
				     "The stream's SoupContentSniffer",
				     SOUP_TYPE_CONTENT_SNIFFER,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (
		object_class, PROP_MESSAGE,
		g_param_spec_object ("message",
				     "Message",
				     "The stream's SoupMessage",
				     SOUP_TYPE_MESSAGE,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}

static void
soup_content_sniffer_stream_pollable_init (GPollableInputStreamInterface *pollable_interface,
					   gpointer                       interface_data)
{
	pollable_interface->can_poll = soup_content_sniffer_stream_can_poll;
	pollable_interface->is_readable = soup_content_sniffer_stream_is_readable;
	pollable_interface->read_nonblocking = soup_content_sniffer_stream_read_nonblocking;
	pollable_interface->create_source = soup_content_sniffer_stream_create_source;
}

gboolean
soup_content_sniffer_stream_is_ready (SoupContentSnifferStream  *sniffer,
				      gboolean                   blocking,
				      GCancellable              *cancellable,
				      GError                   **error)
{
	if (!sniffer->priv->sniffing)
		return TRUE;

	return read_and_sniff (G_INPUT_STREAM (sniffer), blocking,
			       cancellable, error) != -1;
}

const char *
soup_content_sniffer_stream_sniff (SoupContentSnifferStream  *sniffer,
				   GHashTable               **params)
{
	if (params)
		*params = sniffer->priv->sniffed_params;
	return sniffer->priv->sniffed_type;
}
