/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2008 Red Hat, Inc.
 */

#ifndef SOUP_GNOME_FEATURES_H
#define SOUP_GNOME_FEATURES_H 1

#include <libsoup/soup-types.h>

G_BEGIN_DECLS

SOUP_AVAILABLE_IN_2_26
GType soup_proxy_resolver_gnome_get_type (void);
#define SOUP_TYPE_PROXY_RESOLVER_GNOME (soup_proxy_resolver_gnome_get_type ())

SOUP_AVAILABLE_IN_2_26
GType soup_gnome_features_2_26_get_type (void);
#define SOUP_TYPE_GNOME_FEATURES_2_26 (soup_gnome_features_2_26_get_type ())

#ifndef G_OS_WIN32
#ifdef LIBSOUP_I_HAVE_READ_BUG_594377_AND_KNOW_SOUP_PASSWORD_MANAGER_MIGHT_GO_AWAY
SOUP_AVAILABLE_IN_2_28
GType soup_password_manager_gnome_get_type (void);
#define SOUP_TYPE_PASSWORD_MANAGER_GNOME (soup_password_manager_gnome_get_type ())
#endif /* LIBSOUP_I_HAVE_READ_BUG_594377_AND_KNOW_SOUP_PASSWORD_MANAGER_MIGHT_GO_AWAY */
#endif

G_END_DECLS

#endif /* SOUP_GNOME_FEATURES_H */
