/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright 2012 Red Hat, Inc.
 */

#ifndef __SOUP_IO_STREAM_H__
#define __SOUP_IO_STREAM_H__ 1

#include <libsoup/soup-types.h>

G_BEGIN_DECLS

#define SOUP_TYPE_IO_STREAM            (soup_io_stream_get_type ())
#define SOUP_IO_STREAM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SOUP_TYPE_IO_STREAM, SoupIOStream))
#define SOUP_IO_STREAM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_TYPE_IO_STREAM, SoupIOStreamClass))
#define SOUP_IS_IO_STREAM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SOUP_TYPE_IO_STREAM))
#define SOUP_IS_IO_STREAM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), SOUP_TYPE_IO_STREAM))
#define SOUP_IO_STREAM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_TYPE_IO_STREAM, SoupIOStreamClass))

typedef struct _SoupIOStreamPrivate SoupIOStreamPrivate;

typedef struct {
	GIOStream parent;

	SoupIOStreamPrivate *priv;
} SoupIOStream;

typedef struct {
	GIOStreamClass parent_class;

} SoupIOStreamClass;

GType soup_io_stream_get_type (void);

GIOStream *soup_io_stream_new (GIOStream *base_iostream,
			       gboolean   close_on_dispose);

G_END_DECLS

#endif /* __SOUP_IO_STREAM_H__ */
