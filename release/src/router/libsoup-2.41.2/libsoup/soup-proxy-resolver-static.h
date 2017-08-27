/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2008 Red Hat, Inc.
 */

#ifndef SOUP_PROXY_RESOLVER_STATIC_H
#define SOUP_PROXY_RESOLVER_STATIC_H 1

#include "soup-proxy-uri-resolver.h"
#include "soup-uri.h"

#define SOUP_TYPE_PROXY_RESOLVER_STATIC            (soup_proxy_resolver_static_get_type ())
#define SOUP_PROXY_RESOLVER_STATIC(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), SOUP_TYPE_PROXY_RESOLVER_STATIC, SoupProxyResolverStatic))
#define SOUP_PROXY_RESOLVER_STATIC_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_TYPE_PROXY_RESOLVER_STATIC, SoupProxyResolverStaticClass))
#define SOUP_IS_PROXY_RESOLVER_STATIC(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), SOUP_TYPE_PROXY_RESOLVER_STATIC))
#define SOUP_IS_PROXY_RESOLVER_STATIC_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SOUP_TYPE_PROXY_RESOLVER_STATIC))
#define SOUP_PROXY_RESOLVER_STATIC_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_TYPE_PROXY_RESOLVER_STATIC, SoupProxyResolverStaticClass))

typedef struct {
	GObject parent;

} SoupProxyResolverStatic;

typedef struct {
	GObjectClass parent_class;

} SoupProxyResolverStaticClass;

GType soup_proxy_resolver_static_get_type (void);

#define SOUP_PROXY_RESOLVER_STATIC_PROXY_URI "proxy-uri"

SoupProxyURIResolver *soup_proxy_resolver_static_new (SoupURI *proxy_uri);

#endif /* SOUP_PROXY_RESOLVER_STATIC_H */
