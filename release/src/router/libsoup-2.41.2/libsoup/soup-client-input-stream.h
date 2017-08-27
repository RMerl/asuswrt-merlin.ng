/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright 2010-2012 Red Hat, Inc.
 */

#ifndef SOUP_CLIENT_INPUT_STREAM_H
#define SOUP_CLIENT_INPUT_STREAM_H 1

#include "soup-types.h"
#include "soup-filter-input-stream.h"

G_BEGIN_DECLS

#define SOUP_TYPE_CLIENT_INPUT_STREAM            (soup_client_input_stream_get_type ())
#define SOUP_CLIENT_INPUT_STREAM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SOUP_TYPE_CLIENT_INPUT_STREAM, SoupClientInputStream))
#define SOUP_CLIENT_INPUT_STREAM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_TYPE_CLIENT_INPUT_STREAM, SoupClientInputStreamClass))
#define SOUP_IS_CLIENT_INPUT_STREAM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SOUP_TYPE_CLIENT_INPUT_STREAM))
#define SOUP_IS_CLIENT_INPUT_STREAM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), SOUP_TYPE_CLIENT_INPUT_STREAM))
#define SOUP_CLIENT_INPUT_STREAM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_TYPE_CLIENT_INPUT_STREAM, SoupClientInputStreamClass))

typedef struct _SoupClientInputStreamPrivate SoupClientInputStreamPrivate;

typedef struct {
	SoupFilterInputStream parent;

	SoupClientInputStreamPrivate *priv;
} SoupClientInputStream;

typedef struct {
	SoupFilterInputStreamClass parent_class;

	/* Padding for future expansion */
	void (*_libsoup_reserved1) (void);
	void (*_libsoup_reserved2) (void);
	void (*_libsoup_reserved3) (void);
	void (*_libsoup_reserved4) (void);
} SoupClientInputStreamClass;

GType soup_client_input_stream_get_type (void);

GInputStream *soup_client_input_stream_new (GInputStream *base_stream,
					    SoupMessage  *msg);

G_END_DECLS

#endif /* SOUP_CLIENT_INPUT_STREAM_H */
