/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-cookie-jar-sqlite.c: ff sqlite-based cookie storage
 *
 * Using danw's soup-cookie-jar-text as template
 * Copyright (C) 2008 Diego Escalante Urrelo
 * Copyright (C) 2009 Collabora Ltd.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include <sqlite3.h>

#include "soup-cookie-jar-sqlite.h"
#include "soup.h"

/**
 * SECTION:soup-cookie-jar-sqlite
 * @short_description: SQLite-based Cookie Jar
 *
 * #SoupCookieJarSqlite is a #SoupCookieJar that reads cookies from and
 * writes them to an SQLite file in the new Mozilla format.
 **/

enum {
	PROP_0,

	PROP_FILENAME,

	LAST_PROP
};

typedef struct {
	char *filename;
	sqlite3 *db;
} SoupCookieJarSqlitePrivate;

#define SOUP_COOKIE_JAR_SQLITE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), SOUP_TYPE_COOKIE_JAR_SQLITE, SoupCookieJarSqlitePrivate))

G_DEFINE_TYPE (SoupCookieJarSqlite, soup_cookie_jar_sqlite, SOUP_TYPE_COOKIE_JAR)

static void load (SoupCookieJar *jar);

static void
soup_cookie_jar_sqlite_init (SoupCookieJarSqlite *sqlite)
{
}

static void
soup_cookie_jar_sqlite_finalize (GObject *object)
{
	SoupCookieJarSqlitePrivate *priv =
		SOUP_COOKIE_JAR_SQLITE_GET_PRIVATE (object);

	g_free (priv->filename);

	if (priv->db)
		sqlite3_close (priv->db);

	G_OBJECT_CLASS (soup_cookie_jar_sqlite_parent_class)->finalize (object);
}

static void
soup_cookie_jar_sqlite_set_property (GObject *object, guint prop_id,
				     const GValue *value, GParamSpec *pspec)
{
	SoupCookieJarSqlitePrivate *priv =
		SOUP_COOKIE_JAR_SQLITE_GET_PRIVATE (object);

	switch (prop_id) {
	case PROP_FILENAME:
		priv->filename = g_value_dup_string (value);
		load (SOUP_COOKIE_JAR (object));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
soup_cookie_jar_sqlite_get_property (GObject *object, guint prop_id,
				     GValue *value, GParamSpec *pspec)
{
	SoupCookieJarSqlitePrivate *priv =
		SOUP_COOKIE_JAR_SQLITE_GET_PRIVATE (object);

	switch (prop_id) {
	case PROP_FILENAME:
		g_value_set_string (value, priv->filename);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

/**
 * soup_cookie_jar_sqlite_new:
 * @filename: the filename to read to/write from, or %NULL
 * @read_only: %TRUE if @filename is read-only
 *
 * Creates a #SoupCookieJarSqlite.
 *
 * @filename will be read in at startup to create an initial set of
 * cookies. If @read_only is %FALSE, then the non-session cookies will
 * be written to @filename when the 'changed' signal is emitted from
 * the jar. (If @read_only is %TRUE, then the cookie jar will only be
 * used for this session, and changes made to it will be lost when the
 * jar is destroyed.)
 *
 * Return value: the new #SoupCookieJar
 *
 * Since: 2.26
 **/
SoupCookieJar *
soup_cookie_jar_sqlite_new (const char *filename, gboolean read_only)
{
	g_return_val_if_fail (filename != NULL, NULL);

	return g_object_new (SOUP_TYPE_COOKIE_JAR_SQLITE,
			     SOUP_COOKIE_JAR_SQLITE_FILENAME, filename,
			     SOUP_COOKIE_JAR_READ_ONLY, read_only,
			     NULL);
}

#define QUERY_ALL "SELECT id, name, value, host, path, expiry, lastAccessed, isSecure, isHttpOnly FROM moz_cookies;"
#define CREATE_TABLE "CREATE TABLE moz_cookies (id INTEGER PRIMARY KEY, name TEXT, value TEXT, host TEXT, path TEXT,expiry INTEGER, lastAccessed INTEGER, isSecure INTEGER, isHttpOnly INTEGER)"
#define QUERY_INSERT "INSERT INTO moz_cookies VALUES(NULL, %Q, %Q, %Q, %Q, %d, NULL, %d, %d);"
#define QUERY_DELETE "DELETE FROM moz_cookies WHERE name=%Q AND host=%Q;"

enum {
	COL_ID,
	COL_NAME,
	COL_VALUE,
	COL_HOST,
	COL_PATH,
	COL_EXPIRY,
	COL_LAST_ACCESS,
	COL_SECURE,
	COL_HTTP_ONLY,
	N_COL,
};

static int
callback (void *data, int argc, char **argv, char **colname)
{
	SoupCookie *cookie = NULL;
	SoupCookieJar *jar = SOUP_COOKIE_JAR (data);

	char *name, *value, *host, *path;
	gulong expire_time;
	time_t now;
	int max_age;
	gboolean http_only = FALSE, secure = FALSE;

	now = time (NULL);

	name = argv[COL_NAME];
	value = argv[COL_VALUE];
	host = argv[COL_HOST];
	path = argv[COL_PATH];
	expire_time = strtoul (argv[COL_EXPIRY], NULL, 10);

	if (now >= expire_time)
		return 0;
	max_age = (expire_time - now <= G_MAXINT ? expire_time - now : G_MAXINT);

	http_only = (g_strcmp0 (argv[COL_HTTP_ONLY], "1") == 0);
	secure = (g_strcmp0 (argv[COL_SECURE], "1") == 0);

	cookie = soup_cookie_new (name, value, host, path, max_age);

	if (secure)
		soup_cookie_set_secure (cookie, TRUE);
	if (http_only)
		soup_cookie_set_http_only (cookie, TRUE);

	soup_cookie_jar_add_cookie (jar, cookie);

	return 0;
}

static void
try_create_table (sqlite3 *db)
{
	char *error = NULL;

	if (sqlite3_exec (db, CREATE_TABLE, NULL, NULL, &error)) {
		g_warning ("Failed to execute query: %s", error);
		sqlite3_free (error);
	}
}

static void
exec_query_with_try_create_table (sqlite3 *db,
				  const char *sql,
				  int (*callback)(void*,int,char**,char**),
				  void *argument)
{
	char *error = NULL;
	gboolean try_create = TRUE;

try_exec:
	if (sqlite3_exec (db, sql, callback, argument, &error)) {
		if (try_create) {
			try_create = FALSE;
			try_create_table (db);
			sqlite3_free (error);
			error = NULL;
			goto try_exec;
		} else {
			g_warning ("Failed to execute query: %s", error);
			sqlite3_free (error);
		}
	}
}

/* Follows sqlite3 convention; returns TRUE on error */
static gboolean
open_db (SoupCookieJar *jar)
{
	SoupCookieJarSqlitePrivate *priv =
		SOUP_COOKIE_JAR_SQLITE_GET_PRIVATE (jar);

	char *error = NULL;

	if (sqlite3_open (priv->filename, &priv->db)) {
		sqlite3_close (priv->db);
		priv->db = NULL;
		g_warning ("Can't open %s", priv->filename);
		return TRUE;
	}

	if (sqlite3_exec (priv->db, "PRAGMA synchronous = OFF; PRAGMA secure_delete = 1;", NULL, NULL, &error)) {
		g_warning ("Failed to execute query: %s", error);
		sqlite3_free (error);
	}

	return FALSE;
}

static void
load (SoupCookieJar *jar)
{
	SoupCookieJarSqlitePrivate *priv =
		SOUP_COOKIE_JAR_SQLITE_GET_PRIVATE (jar);

	if (priv->db == NULL) {
		if (open_db (jar))
			return;
	}

	exec_query_with_try_create_table (priv->db, QUERY_ALL, callback, jar);
}

static void
soup_cookie_jar_sqlite_changed (SoupCookieJar *jar,
				SoupCookie    *old_cookie,
				SoupCookie    *new_cookie)
{
	SoupCookieJarSqlitePrivate *priv =
		SOUP_COOKIE_JAR_SQLITE_GET_PRIVATE (jar);
	char *query;

	if (priv->db == NULL) {
		if (open_db (jar))
			return;
	}

	if (old_cookie) {
		query = sqlite3_mprintf (QUERY_DELETE,
					 old_cookie->name,
					 old_cookie->domain);
		exec_query_with_try_create_table (priv->db, query, NULL, NULL);
		sqlite3_free (query);
	}

	if (new_cookie && new_cookie->expires) {
		gulong expires;
		
		expires = (gulong)soup_date_to_time_t (new_cookie->expires);
		query = sqlite3_mprintf (QUERY_INSERT, 
					 new_cookie->name,
					 new_cookie->value,
					 new_cookie->domain,
					 new_cookie->path,
					 expires,
					 new_cookie->secure,
					 new_cookie->http_only);
		exec_query_with_try_create_table (priv->db, query, NULL, NULL);
		sqlite3_free (query);
	}
}

static gboolean
soup_cookie_jar_sqlite_is_persistent (SoupCookieJar *jar)
{
	return TRUE;
}

static void
soup_cookie_jar_sqlite_class_init (SoupCookieJarSqliteClass *sqlite_class)
{
	SoupCookieJarClass *cookie_jar_class =
		SOUP_COOKIE_JAR_CLASS (sqlite_class);
	GObjectClass *object_class = G_OBJECT_CLASS (sqlite_class);

	g_type_class_add_private (sqlite_class, sizeof (SoupCookieJarSqlitePrivate));

	cookie_jar_class->is_persistent = soup_cookie_jar_sqlite_is_persistent;
	cookie_jar_class->changed       = soup_cookie_jar_sqlite_changed;

	object_class->finalize     = soup_cookie_jar_sqlite_finalize;
	object_class->set_property = soup_cookie_jar_sqlite_set_property;
	object_class->get_property = soup_cookie_jar_sqlite_get_property;

	/**
	 * SOUP_COOKIE_JAR_SQLITE_FILENAME:
	 *
	 * Alias for the #SoupCookieJarSqlite:filename property. (The
	 * cookie-storage filename.)
	 **/
	g_object_class_install_property (
		object_class, PROP_FILENAME,
		g_param_spec_string (SOUP_COOKIE_JAR_SQLITE_FILENAME,
				     "Filename",
				     "Cookie-storage filename",
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}
