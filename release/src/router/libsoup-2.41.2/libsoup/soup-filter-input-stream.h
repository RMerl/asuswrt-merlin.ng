/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright 2012 Red Hat, Inc.
 */

#ifndef SOUP_FILTER_INPUT_STREAM_H
#define SOUP_FILTER_INPUT_STREAM_H 1

#include <libsoup/soup-types.h>

G_BEGIN_DECLS

#define SOUP_TYPE_FILTER_INPUT_STREAM            (soup_filter_input_stream_get_type ())
#define SOUP_FILTER_INPUT_STREAM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SOUP_TYPE_FILTER_INPUT_STREAM, SoupFilterInputStream))
#define SOUP_FILTER_INPUT_STREAM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_TYPE_FILTER_INPUT_STREAM, SoupFilterInputStreamClass))
#define SOUP_IS_FILTER_INPUT_STREAM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SOUP_TYPE_FILTER_INPUT_STREAM))
#define SOUP_IS_FILTER_INPUT_STREAM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), SOUP_TYPE_FILTER_INPUT_STREAM))
#define SOUP_FILTER_INPUT_STREAM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_TYPE_FILTER_INPUT_STREAM, SoupFilterInputStreamClass))

typedef struct _SoupFilterInputStreamPrivate SoupFilterInputStreamPrivate;

typedef struct {
	GFilterInputStream parent;

	SoupFilterInputStreamPrivate *priv;
} SoupFilterInputStream;

typedef struct {
	GFilterInputStreamClass parent_class;

} SoupFilterInputStreamClass;

GType soup_filter_input_stream_get_type (void);

GInputStream *soup_filter_input_stream_new        (GInputStream           *base_stream);

gssize        soup_filter_input_stream_read_line  (SoupFilterInputStream  *fstream,
						   void                   *buffer,
						   gsize                   length,
						   gboolean                blocking,
						   gboolean               *got_line,
						   GCancellable           *cancellable,
						   GError                **error);
gssize        soup_filter_input_stream_read_until (SoupFilterInputStream  *fstream,
						   void                   *buffer,
						   gsize                   length,
						   const void             *boundary,
						   gsize                   boundary_len,
						   gboolean                blocking,
						   gboolean                include_boundary,
						   gboolean               *got_boundary,
						   GCancellable           *cancellable,
						   GError                **error);

G_END_DECLS

#endif /* SOUP_FILTER_INPUT_STREAM_H */
