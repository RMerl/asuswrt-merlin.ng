/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2008 Red Hat, Inc.
 */

#ifndef SOUP_PROXY_RESOLVER_GNOME_H
#define SOUP_PROXY_RESOLVER_GNOME_H 1

#include "soup-gnome-features.h"
#include "soup-proxy-resolver-default.h"

/* SOUP_TYPE_PROXY_RESOLVER_GNOME and soup_proxy_resolver_gnome_get_type()
 * are declared in soup-gnome-features.h.
 */

#define SOUP_PROXY_RESOLVER_GNOME(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), SOUP_TYPE_PROXY_RESOLVER_GNOME, SoupProxyResolverGNOME))
#define SOUP_PROXY_RESOLVER_GNOME_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_TYPE_PROXY_RESOLVER_GNOME, SoupProxyResolverGNOMEClass))
#define SOUP_IS_PROXY_RESOLVER_GNOME(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), SOUP_TYPE_PROXY_RESOLVER_GNOME))
#define SOUP_IS_PROXY_RESOLVER_GNOME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SOUP_TYPE_PROXY_RESOLVER_GNOME))
#define SOUP_PROXY_RESOLVER_GNOME_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_TYPE_PROXY_RESOLVER_GNOME, SoupProxyResolverGNOMEClass))

typedef SoupProxyResolverDefault SoupProxyResolverGNOME;
typedef SoupProxyResolverDefaultClass SoupProxyResolverGNOMEClass;

#endif /*SOUP_PROXY_RESOLVER_GNOME_H*/
