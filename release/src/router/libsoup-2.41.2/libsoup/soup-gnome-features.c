/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-gnome-features.c: GNOME-specific features
 *
 * Copyright (C) 2008 Red Hat, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "soup-gnome-features.h"

/**
 * SOUP_TYPE_PROXY_RESOLVER_GNOME:
 *
 * This returns the #GType of a #SoupProxyURIResolver that can be used to
 * resolve HTTP proxies for GNOME applications. You can add this to
 * a session using soup_session_add_feature_by_type() or by using the
 * %SOUP_SESSION_ADD_FEATURE_BY_TYPE construct-time property.
 *
 * This feature is included in %SOUP_TYPE_GNOME_FEATURES_2_26, so if
 * you are using that feature, you do not need to include this feature
 * separately.
 *
 * Since: 2.26
 **/
/* This is actually declared in soup-proxy-resolver-gnome now */

/**
 * SOUP_TYPE_GNOME_FEATURES_2_26:
 *
 * This returns the #GType of a #SoupSessionFeature that automatically
 * adds all of the GNOME features defined for libsoup 2.26 (which is
 * just %SOUP_TYPE_PROXY_RESOLVER_GNOME).
 *
 * You can add this to a session using
 * soup_session_add_feature_by_type() or by using the
 * %SOUP_SESSION_ADD_FEATURE_BY_TYPE construct-time property.
 *
 * Since: 2.26
 **/
GType
soup_gnome_features_2_26_get_type (void)
{
	/* Eventually this needs to be a special SoupSessionFeature
	 * class that registers other features. But for now we can
	 * just do this:
	 */
	return SOUP_TYPE_PROXY_RESOLVER_GNOME;
}

