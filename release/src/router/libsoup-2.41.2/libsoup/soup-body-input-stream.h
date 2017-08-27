/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright 2012 Red Hat, Inc.
 */

#ifndef SOUP_BODY_INPUT_STREAM_H
#define SOUP_BODY_INPUT_STREAM_H 1

#include "soup-types.h"
#include "soup-filter-input-stream.h"
#include "soup-message-headers.h"

G_BEGIN_DECLS

#define SOUP_TYPE_BODY_INPUT_STREAM            (soup_body_input_stream_get_type ())
#define SOUP_BODY_INPUT_STREAM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SOUP_TYPE_BODY_INPUT_STREAM, SoupBodyInputStream))
#define SOUP_BODY_INPUT_STREAM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_TYPE_BODY_INPUT_STREAM, SoupBodyInputStreamClass))
#define SOUP_IS_BODY_INPUT_STREAM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SOUP_TYPE_BODY_INPUT_STREAM))
#define SOUP_IS_BODY_INPUT_STREAM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), SOUP_TYPE_BODY_INPUT_STREAM))
#define SOUP_BODY_INPUT_STREAM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_TYPE_BODY_INPUT_STREAM, SoupBodyInputStreamClass))

typedef struct _SoupBodyInputStreamPrivate SoupBodyInputStreamPrivate;

typedef struct {
	GFilterInputStream parent;

	SoupBodyInputStreamPrivate *priv;
} SoupBodyInputStream;

typedef struct {
	GFilterInputStreamClass parent_class;

	/* Padding for future expansion */
	void (*_libsoup_reserved1) (void);
	void (*_libsoup_reserved2) (void);
	void (*_libsoup_reserved3) (void);
	void (*_libsoup_reserved4) (void);
} SoupBodyInputStreamClass;

GType soup_body_input_stream_get_type (void);

GInputStream *soup_body_input_stream_new (GInputStream *base_stream,
					  SoupEncoding  encoding,
					  goffset       content_length);

G_END_DECLS

#endif /* SOUP_BODY_INPUT_STREAM_H */
