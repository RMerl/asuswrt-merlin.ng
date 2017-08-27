/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-io-stream.c
 *
 * Copyright 2012 Red Hat, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "soup-io-stream.h"
#include "soup.h"
#include "soup-filter-input-stream.h"

struct _SoupIOStreamPrivate {
	GIOStream *base_iostream;
	gboolean close_on_dispose;

	GInputStream *istream;
	GOutputStream *ostream;
	gboolean disposing;
};

enum {
	PROP_0,

	PROP_BASE_IOSTREAM,
	PROP_CLOSE_ON_DISPOSE
};

G_DEFINE_TYPE (SoupIOStream, soup_io_stream, G_TYPE_IO_STREAM)

static void
soup_io_stream_init (SoupIOStream *stream)
{
	stream->priv = G_TYPE_INSTANCE_GET_PRIVATE (stream,
						    SOUP_TYPE_IO_STREAM,
						    SoupIOStreamPrivate);
}

static void
soup_io_stream_set_property (GObject *object, guint prop_id,
			     const GValue *value, GParamSpec *pspec)
{
	SoupIOStream *siostream = SOUP_IO_STREAM (object);
	GIOStream *io;

	switch (prop_id) {
	case PROP_BASE_IOSTREAM:
		io = siostream->priv->base_iostream = g_value_dup_object (value);
		if (io) {
			siostream->priv->istream =
				soup_filter_input_stream_new (g_io_stream_get_input_stream (io));
			siostream->priv->ostream =
				g_object_ref (g_io_stream_get_output_stream (io));
		} else {
			g_clear_object (&siostream->priv->istream);
			g_clear_object (&siostream->priv->ostream);
		}
		break;
	case PROP_CLOSE_ON_DISPOSE:
		siostream->priv->close_on_dispose = g_value_get_boolean (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
soup_io_stream_get_property (GObject *object, guint prop_id,
			     GValue *value, GParamSpec *pspec)
{
	SoupIOStream *siostream = SOUP_IO_STREAM (object);

	switch (prop_id) {
	case PROP_BASE_IOSTREAM:
		g_value_set_object (value, siostream->priv->base_iostream);
		break;
	case PROP_CLOSE_ON_DISPOSE:
		g_value_set_boolean (value, siostream->priv->close_on_dispose);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
soup_io_stream_dispose (GObject *object)
{
	SoupIOStream *siostream = SOUP_IO_STREAM (object);

	siostream->priv->disposing = TRUE;

	G_OBJECT_CLASS (soup_io_stream_parent_class)->dispose (object);
}

static void
soup_io_stream_finalize (GObject *object)
{
	SoupIOStream *siostream = SOUP_IO_STREAM (object);

	g_clear_object (&siostream->priv->base_iostream);
	g_clear_object (&siostream->priv->istream);
	g_clear_object (&siostream->priv->ostream);

	G_OBJECT_CLASS (soup_io_stream_parent_class)->finalize (object);
}

static GInputStream *
soup_io_stream_get_input_stream (GIOStream *stream)
{
	return SOUP_IO_STREAM (stream)->priv->istream;
}

static GOutputStream *
soup_io_stream_get_output_stream (GIOStream *stream)
{
	return SOUP_IO_STREAM (stream)->priv->ostream;
}


static gboolean
soup_io_stream_close (GIOStream     *stream,
		      GCancellable  *cancellable,
		      GError       **error)
{
	SoupIOStream *siostream = SOUP_IO_STREAM (stream);

	if (siostream->priv->disposing &&
	    !siostream->priv->close_on_dispose)
		return TRUE;

	return g_io_stream_close (siostream->priv->base_iostream,
				  cancellable, error);
}

static void    
soup_io_stream_close_async (GIOStream           *stream,
			    int                  io_priority,
			    GCancellable        *cancellable,
			    GAsyncReadyCallback  callback,
			    gpointer             user_data)
{
	g_io_stream_close_async (SOUP_IO_STREAM (stream)->priv->base_iostream,
				 io_priority, cancellable, callback, user_data);
}

static gboolean
soup_io_stream_close_finish (GIOStream     *stream,
                             GAsyncResult  *result,
			     GError       **error)
{
	return g_io_stream_close_finish (SOUP_IO_STREAM (stream)->priv->base_iostream,
					 result, error);
}

static void
soup_io_stream_class_init (SoupIOStreamClass *stream_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (stream_class);
	GIOStreamClass *io_stream_class = G_IO_STREAM_CLASS (stream_class);

	g_type_class_add_private (stream_class, sizeof (SoupIOStreamPrivate));

	object_class->set_property = soup_io_stream_set_property;
	object_class->get_property = soup_io_stream_get_property;
	object_class->dispose = soup_io_stream_dispose;
	object_class->finalize = soup_io_stream_finalize;

	io_stream_class->get_input_stream = soup_io_stream_get_input_stream;
	io_stream_class->get_output_stream = soup_io_stream_get_output_stream;
	io_stream_class->close_fn = soup_io_stream_close;
	io_stream_class->close_async = soup_io_stream_close_async;
	io_stream_class->close_finish = soup_io_stream_close_finish;

	g_object_class_install_property (
		object_class, PROP_BASE_IOSTREAM,
		g_param_spec_object ("base-iostream",
				     "Base IOStream",
				     "Base GIOStream",
				     G_TYPE_IO_STREAM,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (
		object_class, PROP_CLOSE_ON_DISPOSE,
		g_param_spec_boolean ("close-on-dispose",
				      "Close base stream",
				      "Close base GIOStream when closing",
				      TRUE,
				      G_PARAM_READWRITE |
				      G_PARAM_CONSTRUCT_ONLY));
}

GIOStream *
soup_io_stream_new (GIOStream *base_iostream,
		    gboolean   close_on_dispose)
{
	return g_object_new (SOUP_TYPE_IO_STREAM,
			     "base-iostream", base_iostream,
			     "close-on-dispose", close_on_dispose,
			     NULL);
}
