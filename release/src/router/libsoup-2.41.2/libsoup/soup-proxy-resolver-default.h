/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2011 Collabora Ltd.
 */

#ifndef SOUP_PROXY_RESOLVER_DEFAULT_H
#define SOUP_PROXY_RESOLVER_DEFAULT_H 1

#include <glib-object.h>

#define SOUP_PROXY_RESOLVER_DEFAULT(object)	    (G_TYPE_CHECK_INSTANCE_CAST ((object), SOUP_TYPE_PROXY_RESOLVER_DEFAULT, SoupProxyResolverDefault))
#define SOUP_PROXY_RESOLVER_DEFAULT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_TYPE_PROXY_RESOLVER_DEFAULT, SoupProxyResolverDefaultClass))
#define SOUP_IS_PROXY_RESOLVER_DEFAULT(object)	    (G_TYPE_CHECK_INSTANCE_TYPE ((object), SOUP_TYPE_PROXY_RESOLVER_DEFAULT))
#define SOUP_IS_PROXY_RESOLVER_DEFAULT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SOUP_TYPE_PROXY_RESOLVER_DEFAULT))
#define SOUP_PROXY_RESOLVER_DEFAULT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_TYPE_PROXY_RESOLVER_DEFAULT, SoupProxyResolverDefaultClass))

typedef struct {
	GObject parent;

} SoupProxyResolverDefault;

typedef struct {
	GObjectClass parent_class;

} SoupProxyResolverDefaultClass;

GType soup_proxy_resolver_default_get_type (void);
#define SOUP_TYPE_PROXY_RESOLVER_DEFAULT (soup_proxy_resolver_default_get_type ())

#endif /*SOUP_PROXY_RESOLVER_DEFAULT_H*/
