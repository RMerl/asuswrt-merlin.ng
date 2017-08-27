/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-cache.h:
 *
 * Copyright (C) 2009, 2010 Igalia, S.L.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SOUP_CACHE_H
#define SOUP_CACHE_H 1

#ifdef LIBSOUP_USE_UNSTABLE_REQUEST_API

#include <libsoup/soup-types.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define SOUP_TYPE_CACHE            (soup_cache_get_type ())
#define SOUP_CACHE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SOUP_TYPE_CACHE, SoupCache))
#define SOUP_CACHE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_TYPE_CACHE, SoupCacheClass))
#define SOUP_IS_CACHE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SOUP_TYPE_CACHE))
#define SOUP_IS_CACHE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), SOUP_TYPE_CACHE))
#define SOUP_CACHE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_TYPE_CACHE, SoupCacheClass))

typedef struct _SoupCache SoupCache;
typedef struct _SoupCachePrivate SoupCachePrivate;

typedef enum {
	SOUP_CACHE_CACHEABLE = (1 << 0),
	SOUP_CACHE_UNCACHEABLE = (1 << 1),
	SOUP_CACHE_INVALIDATES = (1 << 2),
	SOUP_CACHE_VALIDATES = (1 << 3)
} SoupCacheability;

typedef enum {
	SOUP_CACHE_RESPONSE_FRESH,
	SOUP_CACHE_RESPONSE_NEEDS_VALIDATION,
	SOUP_CACHE_RESPONSE_STALE
} SoupCacheResponse;

typedef enum {
	SOUP_CACHE_SINGLE_USER,
	SOUP_CACHE_SHARED
} SoupCacheType;

struct _SoupCache {
	GObject parent_instance;

	SoupCachePrivate *priv;
};

typedef struct {
	GObjectClass parent_class;

	/* methods */
	SoupCacheability (*get_cacheability) (SoupCache   *cache,
					      SoupMessage *msg);

	/* Padding for future expansion */
	void (*_libsoup_reserved1)(void);
	void (*_libsoup_reserved2)(void);
	void (*_libsoup_reserved3)(void);
} SoupCacheClass;

SOUP_AVAILABLE_IN_2_34
GType      soup_cache_get_type     (void);
SOUP_AVAILABLE_IN_2_34
SoupCache *soup_cache_new          (const char    *cache_dir,
				    SoupCacheType  cache_type);
SOUP_AVAILABLE_IN_2_34
void       soup_cache_flush        (SoupCache     *cache);
SOUP_AVAILABLE_IN_2_34
void       soup_cache_clear        (SoupCache     *cache);

SOUP_AVAILABLE_IN_2_34
void       soup_cache_dump         (SoupCache     *cache);
SOUP_AVAILABLE_IN_2_34
void       soup_cache_load         (SoupCache     *cache);

SOUP_AVAILABLE_IN_2_34
void       soup_cache_set_max_size (SoupCache     *cache,
				    guint          max_size);
SOUP_AVAILABLE_IN_2_34
guint      soup_cache_get_max_size (SoupCache     *cache);

G_END_DECLS

#endif /* LIBSOUP_USE_UNSTABLE_REQUEST_API */

#endif /* SOUP_CACHE_H */

