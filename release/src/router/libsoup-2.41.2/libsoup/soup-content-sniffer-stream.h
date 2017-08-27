/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2010 Red Hat, Inc.
 */

#ifndef SOUP_CONTENT_SNIFFER_STREAM_H
#define SOUP_CONTENT_SNIFFER_STREAM_H 1

#include <libsoup/soup-types.h>
#include <libsoup/soup-content-sniffer.h>

G_BEGIN_DECLS

#define SOUP_TYPE_CONTENT_SNIFFER_STREAM         (soup_content_sniffer_stream_get_type ())
#define SOUP_CONTENT_SNIFFER_STREAM(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), SOUP_TYPE_CONTENT_SNIFFER_STREAM, SoupContentSnifferStream))
#define SOUP_CONTENT_SNIFFER_STREAM_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), SOUP_TYPE_CONTENT_SNIFFER_STREAM, SoupContentSnifferStreamClass))
#define SOUP_IS_CONTENT_SNIFFER_STREAM(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), SOUP_TYPE_CONTENT_SNIFFER_STREAM))
#define SOUP_IS_CONTENT_SNIFFER_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), SOUP_TYPE_CONTENT_SNIFFER_STREAM))
#define SOUP_CONTENT_SNIFFER_STREAM_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), SOUP_TYPE_CONTENT_SNIFFER_STREAM, SoupContentSnifferStreamClass))

typedef struct _SoupContentSnifferStream        SoupContentSnifferStream;
typedef struct _SoupContentSnifferStreamPrivate SoupContentSnifferStreamPrivate;
typedef struct _SoupContentSnifferStreamClass   SoupContentSnifferStreamClass;

struct _SoupContentSnifferStream {
	GFilterInputStream parent_instance;

	/*< private >*/
	SoupContentSnifferStreamPrivate *priv;
};

struct _SoupContentSnifferStreamClass {
	GFilterInputStreamClass parent_class;

};

GType soup_content_sniffer_stream_get_type (void) G_GNUC_CONST;

gboolean      soup_content_sniffer_stream_is_ready (SoupContentSnifferStream  *sniffer,
						    gboolean                   blocking,
						    GCancellable              *cancellable,
						    GError                   **error);
const char   *soup_content_sniffer_stream_sniff    (SoupContentSnifferStream  *sniffer,
						    GHashTable               **params);


G_END_DECLS

#endif /* SOUP_CONTENT_SNIFFER_STREAM_H */
