/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-proxy-resolver-static.c: Static proxy "resolution"
 *
 * Copyright (C) 2008 Red Hat, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "soup-proxy-resolver-static.h"
#include "soup.h"

typedef struct {
	SoupURI *proxy_uri;

} SoupProxyResolverStaticPrivate;
#define SOUP_PROXY_RESOLVER_STATIC_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), SOUP_TYPE_PROXY_RESOLVER_STATIC, SoupProxyResolverStaticPrivate))

static void soup_proxy_resolver_static_interface_init (SoupProxyURIResolverInterface *proxy_resolver_interface);

G_DEFINE_TYPE_EXTENDED (SoupProxyResolverStatic, soup_proxy_resolver_static, G_TYPE_OBJECT, 0,
			G_IMPLEMENT_INTERFACE (SOUP_TYPE_SESSION_FEATURE, NULL)
			G_IMPLEMENT_INTERFACE (SOUP_TYPE_PROXY_URI_RESOLVER, soup_proxy_resolver_static_interface_init))

enum {
	PROP_0,

	PROP_PROXY_URI,

	LAST_PROP
};

static void
soup_proxy_resolver_static_init (SoupProxyResolverStatic *resolver_static)
{
}

static void
soup_proxy_resolver_static_finalize (GObject *object)
{
	SoupProxyResolverStaticPrivate *priv =
		SOUP_PROXY_RESOLVER_STATIC_GET_PRIVATE (object);

	g_clear_pointer (&priv->proxy_uri, soup_uri_free);

	G_OBJECT_CLASS (soup_proxy_resolver_static_parent_class)->finalize (object);
}

static void
soup_proxy_resolver_static_set_property (GObject *object, guint prop_id,
					 const GValue *value, GParamSpec *pspec)
{
	SoupProxyResolverStaticPrivate *priv =
		SOUP_PROXY_RESOLVER_STATIC_GET_PRIVATE (object);
	SoupURI *uri;

	switch (prop_id) {
	case PROP_PROXY_URI:
		uri = g_value_get_boxed (value);
		if (priv->proxy_uri)
			soup_uri_free (priv->proxy_uri);

		priv->proxy_uri = uri ? soup_uri_copy (uri) : NULL;
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
soup_proxy_resolver_static_get_property (GObject *object, guint prop_id,
					 GValue *value, GParamSpec *pspec)
{
	SoupProxyResolverStaticPrivate *priv =
		SOUP_PROXY_RESOLVER_STATIC_GET_PRIVATE (object);

	switch (prop_id) {
	case PROP_PROXY_URI:
		g_value_set_boxed (value, priv->proxy_uri);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

typedef struct {
	SoupProxyURIResolver *proxy_resolver;
	SoupProxyURIResolverCallback callback;
	gpointer user_data;
} SoupStaticAsyncData;

static gboolean
idle_return_proxy_uri (gpointer data)
{
	SoupStaticAsyncData *ssad = data;
	SoupProxyResolverStaticPrivate *priv =
		SOUP_PROXY_RESOLVER_STATIC_GET_PRIVATE (ssad->proxy_resolver);

	ssad->callback (ssad->proxy_resolver,
			SOUP_STATUS_OK, priv->proxy_uri,
			ssad->user_data);
	g_object_unref (ssad->proxy_resolver);
	g_slice_free (SoupStaticAsyncData, ssad);

	return FALSE;
}

static void
soup_proxy_resolver_static_get_proxy_uri_async (SoupProxyURIResolver  *proxy_resolver,
						SoupURI               *uri,
						GMainContext          *async_context,
						GCancellable          *cancellable,
						SoupProxyURIResolverCallback callback,
						gpointer               user_data)
{
	SoupStaticAsyncData *ssad;

	ssad = g_slice_new0 (SoupStaticAsyncData);
	ssad->proxy_resolver = g_object_ref (proxy_resolver);
	ssad->callback = callback;
	ssad->user_data = user_data;
	soup_add_completion (async_context, idle_return_proxy_uri, ssad);
}

static guint
soup_proxy_resolver_static_get_proxy_uri_sync (SoupProxyURIResolver  *proxy_resolver,
					       SoupURI               *uri,
					       GCancellable          *cancellable,
					       SoupURI              **proxy_uri)
{
	SoupProxyResolverStaticPrivate *priv =
		SOUP_PROXY_RESOLVER_STATIC_GET_PRIVATE (proxy_resolver);

	*proxy_uri = soup_uri_copy (priv->proxy_uri);
	return SOUP_STATUS_OK;
}

static void
soup_proxy_resolver_static_class_init (SoupProxyResolverStaticClass *static_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (static_class);

	g_type_class_add_private (static_class, sizeof (SoupProxyResolverStaticPrivate));

	object_class->set_property = soup_proxy_resolver_static_set_property;
	object_class->get_property = soup_proxy_resolver_static_get_property;
	object_class->finalize = soup_proxy_resolver_static_finalize;

	g_object_class_install_property (
		object_class, PROP_PROXY_URI,
		g_param_spec_boxed (SOUP_PROXY_RESOLVER_STATIC_PROXY_URI,
				    "Proxy URI",
				    "The HTTP Proxy to use",
				    SOUP_TYPE_URI,
				    G_PARAM_READWRITE));
}

static void
soup_proxy_resolver_static_interface_init (SoupProxyURIResolverInterface *proxy_uri_resolver_interface)
{
	proxy_uri_resolver_interface->get_proxy_uri_async =
		soup_proxy_resolver_static_get_proxy_uri_async;
	proxy_uri_resolver_interface->get_proxy_uri_sync =
		soup_proxy_resolver_static_get_proxy_uri_sync;
}

SoupProxyURIResolver *
soup_proxy_resolver_static_new (SoupURI *proxy_uri)
{
	return g_object_new (SOUP_TYPE_PROXY_RESOLVER_STATIC,
			     SOUP_PROXY_RESOLVER_STATIC_PROXY_URI, proxy_uri,
			     NULL);
}
