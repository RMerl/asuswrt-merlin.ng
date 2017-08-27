/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-proxy-uri-resolver.c: HTTP proxy resolver interface, take 2
 *
 * Copyright (C) 2009 Red Hat, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "soup-proxy-uri-resolver.h"
#include "soup.h"

G_DEFINE_INTERFACE (SoupProxyURIResolver, soup_proxy_uri_resolver, G_TYPE_OBJECT)

static void
soup_proxy_uri_resolver_default_init (SoupProxyURIResolverInterface *iface)
{
}

/**
 * SoupProxyURIResolverCallback:
 * @resolver: the #SoupProxyURIResolver
 * @status: a #SoupKnownStatusCode
 * @proxy_uri: the resolved proxy URI, or %NULL
 * @user_data: data passed to soup_proxy_uri_resolver_get_proxy_uri_async()
 *
 * Callback for soup_proxy_uri_resolver_get_proxy_uri_async()
 **/

/**
 * soup_proxy_uri_resolver_get_proxy_uri_async:
 * @proxy_uri_resolver: the #SoupProxyURIResolver
 * @uri: the #SoupURI you want a proxy for
 * @async_context: (allow-none): the #GMainContext to invoke @callback in
 * @cancellable: a #GCancellable, or %NULL
 * @callback: (scope async): callback to invoke with the proxy address
 * @user_data: data for @callback
 *
 * Asynchronously determines a proxy URI to use for @msg and calls
 * @callback.
 *
 * Since: 2.26.3
 **/
void
soup_proxy_uri_resolver_get_proxy_uri_async (SoupProxyURIResolver  *proxy_uri_resolver,
					     SoupURI               *uri,
					     GMainContext          *async_context,
					     GCancellable          *cancellable,
					     SoupProxyURIResolverCallback callback,
					     gpointer               user_data)
{
	SOUP_PROXY_URI_RESOLVER_GET_CLASS (proxy_uri_resolver)->
		get_proxy_uri_async (proxy_uri_resolver, uri,
				     async_context, cancellable,
				     callback, user_data);
}

/**
 * soup_proxy_uri_resolver_get_proxy_uri_sync:
 * @proxy_uri_resolver: the #SoupProxyURIResolver
 * @uri: the #SoupURI you want a proxy for
 * @cancellable: a #GCancellable, or %NULL
 * @proxy_uri: (out): on return, will contain the proxy URI
 *
 * Synchronously determines a proxy URI to use for @uri. If @uri
 * should be sent via proxy, *@proxy_uri will be set to the URI of the
 * proxy, else it will be set to %NULL.
 *
 * Return value: %SOUP_STATUS_OK if successful, or a transport-level
 * error.
 *
 * Since: 2.26.3
 **/
guint
soup_proxy_uri_resolver_get_proxy_uri_sync (SoupProxyURIResolver  *proxy_uri_resolver,
					    SoupURI               *uri,
					    GCancellable          *cancellable,
					    SoupURI              **proxy_uri)
{
	return SOUP_PROXY_URI_RESOLVER_GET_CLASS (proxy_uri_resolver)->
		get_proxy_uri_sync (proxy_uri_resolver, uri, cancellable, proxy_uri);
}
