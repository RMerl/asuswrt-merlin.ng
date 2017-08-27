/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2008 Diego Escalante Urrelo
 */

#ifndef SOUP_COOKIE_JAR_SQLITE_H
#define SOUP_COOKIE_JAR_SQLITE_H 1

#include <libsoup/soup-cookie-jar.h>

G_BEGIN_DECLS

#define SOUP_TYPE_COOKIE_JAR_SQLITE            (soup_cookie_jar_sqlite_get_type ())
#define SOUP_COOKIE_JAR_SQLITE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SOUP_TYPE_COOKIE_JAR_SQLITE, SoupCookieJarSqlite))
#define SOUP_COOKIE_JAR_SQLITE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_TYPE_COOKIE_JAR_SQLITE, SoupCookieJarSqliteClass))
#define SOUP_IS_COOKIE_JAR_SQLITE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SOUP_TYPE_COOKIE_JAR_SQLITE))
#define SOUP_IS_COOKIE_JAR_SQLITE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), SOUP_TYPE_COOKIE_JAR_SQLITE))
#define SOUP_COOKIE_JAR_SQLITE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_TYPE_COOKIE_JAR_SQLITE, SoupCookieJarSqliteClass))

typedef struct {
	SoupCookieJar parent;

} SoupCookieJarSqlite;

typedef struct {
	SoupCookieJarClass parent_class;

	/* Padding for future expansion */
	void (*_libsoup_reserved1) (void);
	void (*_libsoup_reserved2) (void);
	void (*_libsoup_reserved3) (void);
	void (*_libsoup_reserved4) (void);
} SoupCookieJarSqliteClass;

#define SOUP_COOKIE_JAR_SQLITE_FILENAME  "filename"

SOUP_AVAILABLE_IN_2_26
GType soup_cookie_jar_sqlite_get_type (void);

SOUP_AVAILABLE_IN_2_26
SoupCookieJar *soup_cookie_jar_sqlite_new (const char *filename,
					   gboolean    read_only);

G_END_DECLS

#endif /* SOUP_COOKIE_JAR_SQLITE_H */
