/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-proxy-resolver-gnome.c: GNOME proxy resolution
 *
 * Copyright (C) 2008 Red Hat, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "soup-proxy-resolver-gnome.h"
#include "soup.h"

G_DEFINE_TYPE (SoupProxyResolverGNOME, soup_proxy_resolver_gnome, SOUP_TYPE_PROXY_RESOLVER_DEFAULT)

static void
soup_proxy_resolver_gnome_init (SoupProxyResolverGNOME *resolver_gnome)
{
	GProxyResolver *gproxyresolver;
	GIOExtensionPoint *ep;
	GIOExtension *ext;
	GType type;

	/* FIXME: there is no way to force _g_io_modules_ensure_loaded()
	 * to be run other than by requesting some extension that we
	 * don't necessarily want.
	 */
	gproxyresolver = g_proxy_resolver_get_default ();
	if (strcmp (G_OBJECT_TYPE_NAME (gproxyresolver),
		    "GProxyResolverGnome") == 0)
		return;

	ep = g_io_extension_point_lookup (G_PROXY_RESOLVER_EXTENSION_POINT_NAME);
	if (!ep)
		return;

	ext = g_io_extension_point_get_extension_by_name (ep, "gnome");
	if (!ext)
		return;

	type = g_io_extension_get_type (ext);
	gproxyresolver = g_object_new (type, NULL);
	g_object_set (G_OBJECT (resolver_gnome),
		      "gproxy-resolver", gproxyresolver,
		      NULL);
	g_object_unref (gproxyresolver);
}

static void
soup_proxy_resolver_gnome_class_init (SoupProxyResolverGNOMEClass *gnome_class)
{
}
