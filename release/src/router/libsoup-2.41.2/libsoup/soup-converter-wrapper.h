/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright 2011 Red Hat, Inc.
 */

#ifndef SOUP_CONVERTER_WRAPPER_H
#define SOUP_CONVERTER_WRAPPER_H 1

#include <libsoup/soup-types.h>

G_BEGIN_DECLS

#define SOUP_TYPE_CONVERTER_WRAPPER            (soup_converter_wrapper_get_type ())
#define SOUP_CONVERTER_WRAPPER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SOUP_TYPE_CONVERTER_WRAPPER, SoupConverterWrapper))
#define SOUP_CONVERTER_WRAPPER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_TYPE_CONVERTER_WRAPPER, SoupConverterWrapperClass))
#define SOUP_IS_CONVERTER_WRAPPER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SOUP_TYPE_CONVERTER_WRAPPER))
#define SOUP_IS_CONVERTER_WRAPPER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), SOUP_TYPE_CONVERTER_WRAPPER))
#define SOUP_CONVERTER_WRAPPER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_TYPE_CONVERTER_WRAPPER, SoupConverterWrapperClass))

typedef struct _SoupConverterWrapperPrivate SoupConverterWrapperPrivate;

typedef struct {
	GObject parent;

	SoupConverterWrapperPrivate *priv;
} SoupConverterWrapper;

typedef struct {
	GObjectClass parent_class;

	/* Padding for future expansion */
	void (*_libsoup_reserved1) (void);
	void (*_libsoup_reserved2) (void);
	void (*_libsoup_reserved3) (void);
	void (*_libsoup_reserved4) (void);
} SoupConverterWrapperClass;

GType soup_converter_wrapper_get_type (void);

GConverter *soup_converter_wrapper_new (GConverter  *base_converter,
					SoupMessage *msg);

G_END_DECLS

#endif /* SOUP_CONVERTER_WRAPPER_H */
