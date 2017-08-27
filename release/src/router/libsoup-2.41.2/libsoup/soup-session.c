/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-session.c
 *
 * Copyright (C) 2000-2003, Ximian, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n-lib.h>

#include "soup-session.h"
#include "soup.h"
#include "soup-auth-manager-ntlm.h"
#include "soup-connection.h"
#include "soup-marshal.h"
#include "soup-message-private.h"
#include "soup-misc-private.h"
#include "soup-message-queue.h"
#include "soup-proxy-resolver-static.h"
#include "soup-session-private.h"

#define HOST_KEEP_ALIVE 5 * 60 * 1000 /* 5 min in msecs */

/**
 * SECTION:soup-session
 * @short_description: Soup session state object
 *
 * #SoupSession is the object that controls client-side HTTP. A
 * #SoupSession encapsulates all of the state that libsoup is keeping
 * on behalf of your program; cached HTTP connections, authentication
 * information, etc.
 *
 * Most applications will only need a single #SoupSession; the primary
 * reason you might need multiple sessions is if you need to have
 * multiple independent authentication contexts. (Eg, you are
 * connecting to a server and authenticating as two different users at
 * different times; the easiest way to ensure that each #SoupMessage
 * is sent with the authentication information you intended is to use
 * one session for the first user, and a second session for the other
 * user.)
 *
 * #SoupSession itself is an abstract class, with two subclasses. If
 * you are using the glib main loop, you will generally want to use
 * #SoupSessionAsync, which uses non-blocking I/O and callbacks. On
 * the other hand, if your application is threaded and you want to do
 * synchronous I/O in a separate thread from the UI, use
 * #SoupSessionSync.
 **/

static void
soup_init (void)
{
	//bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif
}

typedef struct {
	SoupURI     *uri;
	SoupAddress *addr;

	GSList      *connections;      /* CONTAINS: SoupConnection */
	guint        num_conns;

	guint        num_messages;

	gboolean     ssl_fallback;

	GSource     *keep_alive_src;
	SoupSession *session;
} SoupSessionHost;
static guint soup_host_uri_hash (gconstpointer key);
static gboolean soup_host_uri_equal (gconstpointer v1, gconstpointer v2);

typedef struct {
	GTlsDatabase *tlsdb;
	char *ssl_ca_file;
	gboolean ssl_strict;

	SoupMessageQueue *queue;

	char *user_agent;
	char *accept_language;
	gboolean accept_language_auto;

	GSList *features;
	GHashTable *features_cache;

	GHashTable *http_hosts, *https_hosts; /* char* -> SoupSessionHost */
	GHashTable *conns; /* SoupConnection -> SoupSessionHost */
	guint num_conns;
	guint max_conns, max_conns_per_host;
	guint io_timeout, idle_timeout;

	/* Must hold the conn_lock before potentially creating a new
	 * SoupSessionHost, adding/removing a connection,
	 * disconnecting a connection, or moving a connection from
	 * IDLE to IN_USE. Must not emit signals or destroy objects
	 * while holding it.
	 */
	GMutex conn_lock;

	GMainContext *async_context;
	gboolean use_thread_context;

	GResolver *resolver;

	char **http_aliases, **https_aliases;

	gboolean disposed;
} SoupSessionPrivate;
#define SOUP_SESSION_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), SOUP_TYPE_SESSION, SoupSessionPrivate))

static void free_host (SoupSessionHost *host);
static void connection_state_changed (GObject *object, GParamSpec *param,
				      gpointer user_data);
static void connection_disconnected (SoupConnection *conn, gpointer user_data);
static void drop_connection (SoupSession *session, SoupSessionHost *host,
			     SoupConnection *conn);

static void auth_manager_authenticate (SoupAuthManager *manager,
				       SoupMessage *msg, SoupAuth *auth,
				       gboolean retrying, gpointer user_data);

#define SOUP_SESSION_MAX_CONNS_DEFAULT 10
#define SOUP_SESSION_MAX_CONNS_PER_HOST_DEFAULT 2

#define SOUP_SESSION_MAX_REDIRECTION_COUNT 20

#define SOUP_SESSION_USER_AGENT_BASE "libsoup/" PACKAGE_VERSION

G_DEFINE_ABSTRACT_TYPE_WITH_CODE (SoupSession, soup_session, G_TYPE_OBJECT,
				  soup_init ();
				  )

enum {
	REQUEST_QUEUED,
	REQUEST_STARTED,
	REQUEST_UNQUEUED,
	AUTHENTICATE,
	CONNECTION_CREATED,
	TUNNELING,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

enum {
	PROP_0,

	PROP_PROXY_URI,
	PROP_MAX_CONNS,
	PROP_MAX_CONNS_PER_HOST,
	PROP_USE_NTLM,
	PROP_SSL_CA_FILE,
	PROP_SSL_USE_SYSTEM_CA_FILE,
	PROP_TLS_DATABASE,
	PROP_SSL_STRICT,
	PROP_ASYNC_CONTEXT,
	PROP_USE_THREAD_CONTEXT,
	PROP_TIMEOUT,
	PROP_USER_AGENT,
	PROP_ACCEPT_LANGUAGE,
	PROP_ACCEPT_LANGUAGE_AUTO,
	PROP_IDLE_TIMEOUT,
	PROP_ADD_FEATURE,
	PROP_ADD_FEATURE_BY_TYPE,
	PROP_REMOVE_FEATURE_BY_TYPE,
	PROP_HTTP_ALIASES,
	PROP_HTTPS_ALIASES,

	LAST_PROP
};

static void
soup_session_init (SoupSession *session)
{
	SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);
	SoupAuthManager *auth_manager;

	priv->queue = soup_message_queue_new (session);

	g_mutex_init (&priv->conn_lock);
	priv->http_hosts = g_hash_table_new_full (soup_host_uri_hash,
						  soup_host_uri_equal,
						  NULL, (GDestroyNotify)free_host);
	priv->https_hosts = g_hash_table_new_full (soup_host_uri_hash,
						   soup_host_uri_equal,
						   NULL, (GDestroyNotify)free_host);
	priv->conns = g_hash_table_new (NULL, NULL);

	priv->max_conns = SOUP_SESSION_MAX_CONNS_DEFAULT;
	priv->max_conns_per_host = SOUP_SESSION_MAX_CONNS_PER_HOST_DEFAULT;

	priv->features_cache = g_hash_table_new (NULL, NULL);

	auth_manager = g_object_new (SOUP_TYPE_AUTH_MANAGER_NTLM, NULL);
	g_signal_connect (auth_manager, "authenticate",
			  G_CALLBACK (auth_manager_authenticate), session);
	soup_session_feature_add_feature (SOUP_SESSION_FEATURE (auth_manager),
					  SOUP_TYPE_AUTH_BASIC);
	soup_session_feature_add_feature (SOUP_SESSION_FEATURE (auth_manager),
					  SOUP_TYPE_AUTH_DIGEST);
	soup_session_add_feature (session, SOUP_SESSION_FEATURE (auth_manager));
	g_object_unref (auth_manager);

	/* We'll be doing DNS continuously-ish while the session is active,
	 * so hold a ref on the default GResolver.
	 */
	priv->resolver = g_resolver_get_default ();

	priv->ssl_strict = TRUE;

	priv->http_aliases = g_new (char *, 2);
	priv->http_aliases[0] = (char *)g_intern_string ("*");
	priv->http_aliases[1] = NULL;
}

static void
soup_session_dispose (GObject *object)
{
	SoupSession *session = SOUP_SESSION (object);
	SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);

	priv->disposed = TRUE;
	soup_session_abort (session);
	g_warn_if_fail (g_hash_table_size (priv->conns) == 0);

	while (priv->features)
		soup_session_remove_feature (session, priv->features->data);

	G_OBJECT_CLASS (soup_session_parent_class)->dispose (object);
}

static void
soup_session_finalize (GObject *object)
{
	SoupSession *session = SOUP_SESSION (object);
	SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);

	soup_message_queue_destroy (priv->queue);

	g_mutex_clear (&priv->conn_lock);
	g_hash_table_destroy (priv->http_hosts);
	g_hash_table_destroy (priv->https_hosts);
	g_hash_table_destroy (priv->conns);

	g_free (priv->user_agent);
	g_free (priv->accept_language);

	g_clear_object (&priv->tlsdb);
	g_free (priv->ssl_ca_file);

	g_clear_pointer (&priv->async_context, g_main_context_unref);

	g_hash_table_destroy (priv->features_cache);

	g_object_unref (priv->resolver);

	g_free (priv->http_aliases);
	g_free (priv->https_aliases);

	G_OBJECT_CLASS (soup_session_parent_class)->finalize (object);
}

/* Converts a language in POSIX format and to be RFC2616 compliant    */
/* Based on code from epiphany-webkit (ephy_langs_append_languages()) */
static gchar *
posix_lang_to_rfc2616 (const gchar *language)
{
	/* Don't include charset variants, etc */
	if (strchr (language, '.') || strchr (language, '@'))
		return NULL;

	/* Ignore "C" locale, which g_get_language_names() always
	 * includes as a fallback.
	 */
	if (!strcmp (language, "C"))
		return NULL;

	return g_strdelimit (g_ascii_strdown (language, -1), "_", '-');
}

/* Converts @quality from 0-100 to 0.0-1.0 and appends to @str */
static gchar *
add_quality_value (const gchar *str, int quality)
{
	g_return_val_if_fail (str != NULL, NULL);

	if (quality >= 0 && quality < 100) {
		/* We don't use %.02g because of "." vs "," locale issues */
		if (quality % 10)
			return g_strdup_printf ("%s;q=0.%02d", str, quality);
		else
			return g_strdup_printf ("%s;q=0.%d", str, quality / 10);
	} else
		return g_strdup (str);
}

/* Returns a RFC2616 compliant languages list from system locales */
static gchar *
accept_languages_from_system (void)
{
	const char * const * lang_names;
	GPtrArray *langs = NULL;
	char *lang, *langs_str;
	int delta;
	int i;

	lang_names = g_get_language_names ();
	g_return_val_if_fail (lang_names != NULL, NULL);

	/* Build the array of languages */
	langs = g_ptr_array_new_with_free_func (g_free);
	for (i = 0; lang_names[i] != NULL; i++) {
		lang = posix_lang_to_rfc2616 (lang_names[i]);
		if (lang)
			g_ptr_array_add (langs, lang);
	}

	/* Add quality values */
	if (langs->len < 10)
		delta = 10;
	else if (langs->len < 20)
		delta = 5;
	else
		delta = 1;

	for (i = 0; i < langs->len; i++) {
		lang = langs->pdata[i];
		langs->pdata[i] = add_quality_value (lang, 100 - i * delta);
		g_free (lang);
	}

	/* Fallback: add "en" if list is empty */
	if (langs->len == 0)
		g_ptr_array_add (langs, g_strdup ("en"));

	g_ptr_array_add (langs, NULL);
	langs_str = g_strjoinv (", ", (char **)langs->pdata);
	g_ptr_array_free (langs, TRUE);

	return langs_str;
}

static void
set_tlsdb (SoupSession *session, GTlsDatabase *tlsdb)
{
	SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);
	GTlsDatabase *system_default;

	if (tlsdb == priv->tlsdb)
		return;

	g_object_freeze_notify (G_OBJECT (session));

	system_default = g_tls_backend_get_default_database (g_tls_backend_get_default ());
	if (priv->tlsdb == system_default || tlsdb == system_default) {
		g_object_notify (G_OBJECT (session), "ssl-use-system-ca-file");
	}
	g_object_unref (system_default);

	if (priv->ssl_ca_file) {
		g_free (priv->ssl_ca_file);
		priv->ssl_ca_file = NULL;
		g_object_notify (G_OBJECT (session), "ssl-ca-file");
	}

	if (priv->tlsdb)
		g_object_unref (priv->tlsdb);
	priv->tlsdb = tlsdb;
	if (priv->tlsdb)
		g_object_ref (priv->tlsdb);

	g_object_notify (G_OBJECT (session), "tls-database");
	g_object_thaw_notify (G_OBJECT (session));
}

static void
set_use_system_ca_file (SoupSession *session, gboolean use_system_ca_file)
{
	SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);
	GTlsDatabase *system_default;

	system_default = g_tls_backend_get_default_database (g_tls_backend_get_default ());

	if (use_system_ca_file)
		set_tlsdb (session, system_default);
	else if (priv->tlsdb == system_default)
		set_tlsdb (session, NULL);

	g_object_unref (system_default);
}

static void
set_ssl_ca_file (SoupSession *session, const char *ssl_ca_file)
{
	SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);
	GTlsDatabase *tlsdb;
	GError *error = NULL;

	if (!g_strcmp0 (priv->ssl_ca_file, ssl_ca_file))
		return;

	g_object_freeze_notify (G_OBJECT (session));

	if (g_path_is_absolute (ssl_ca_file))
		tlsdb = g_tls_file_database_new (ssl_ca_file, &error);
	else {
		char *path, *cwd;

		cwd = g_get_current_dir ();
		path = g_build_filename (cwd, ssl_ca_file, NULL);
		tlsdb = g_tls_file_database_new (path, &error);
		g_free (path);
	}

	if (error) {
		if (!g_error_matches (error, G_TLS_ERROR, G_TLS_ERROR_UNAVAILABLE)) {
			g_warning ("Could not set SSL credentials from '%s': %s",
				   ssl_ca_file, error->message);

			tlsdb = g_tls_file_database_new ("/dev/null", NULL);
		}
		g_error_free (error);
	}

	set_tlsdb (session, tlsdb);
	g_object_unref (tlsdb);

	priv->ssl_ca_file = g_strdup (ssl_ca_file);
	g_object_notify (G_OBJECT (session), "ssl-ca-file");

	g_object_thaw_notify (G_OBJECT (session));
}

/* priv->http_aliases and priv->https_aliases are stored as arrays of
 * *interned* strings, so we can't just use g_strdupv() to set them.
 */
static void
set_aliases (char ***variable, char **value)
{
	int len, i;

	if (*variable)
		g_free (*variable);

	if (!value) {
		*variable = NULL;
		return;
	}

	len = g_strv_length (value);
	*variable = g_new (char *, len + 1);
	for (i = 0; i < len; i++)
		(*variable)[i] = (char *)g_intern_string (value[i]);
	(*variable)[i] = NULL;
}

static void
soup_session_set_property (GObject *object, guint prop_id,
			   const GValue *value, GParamSpec *pspec)
{
	SoupSession *session = SOUP_SESSION (object);
	SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);
	SoupURI *uri;
	const char *user_agent;
	SoupSessionFeature *feature;

	switch (prop_id) {
	case PROP_PROXY_URI:
		uri = g_value_get_boxed (value);

		if (uri) {
#ifdef G_GNUC_BEGIN_IGNORE_DEPRECATIONS
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
#endif
			soup_session_remove_feature_by_type (session, SOUP_TYPE_PROXY_RESOLVER);
#ifdef G_GNUC_END_IGNORE_DEPRECATIONS
G_GNUC_END_IGNORE_DEPRECATIONS
#endif
			feature = SOUP_SESSION_FEATURE (soup_proxy_resolver_static_new (uri));
			soup_session_add_feature (session, feature);
			g_object_unref (feature);
		} else
			soup_session_remove_feature_by_type (session, SOUP_TYPE_PROXY_RESOLVER_STATIC);

		soup_session_abort (session);
		break;
	case PROP_MAX_CONNS:
		priv->max_conns = g_value_get_int (value);
		break;
	case PROP_MAX_CONNS_PER_HOST:
		priv->max_conns_per_host = g_value_get_int (value);
		break;
	case PROP_USE_NTLM:
		feature = soup_session_get_feature (session, SOUP_TYPE_AUTH_MANAGER_NTLM);
		if (feature) {
			if (g_value_get_boolean (value))
				soup_session_feature_add_feature (feature, SOUP_TYPE_AUTH_NTLM);
			else
				soup_session_feature_remove_feature (feature, SOUP_TYPE_AUTH_NTLM);
		} else
			g_warning ("Trying to set use-ntlm on session with no auth-manager");
		break;
	case PROP_SSL_CA_FILE:
		set_ssl_ca_file (session, g_value_get_string (value));
		break;
	case PROP_SSL_USE_SYSTEM_CA_FILE:
		set_use_system_ca_file (session, g_value_get_boolean (value));
		break;
	case PROP_TLS_DATABASE:
		set_tlsdb (session, g_value_get_object (value));
		break;
	case PROP_SSL_STRICT:
		priv->ssl_strict = g_value_get_boolean (value);
		break;
	case PROP_ASYNC_CONTEXT:
		priv->async_context = g_value_get_pointer (value);
		if (priv->async_context)
			g_main_context_ref (priv->async_context);
		break;
	case PROP_USE_THREAD_CONTEXT:
		priv->use_thread_context = g_value_get_boolean (value);
		if (priv->use_thread_context) {
			if (priv->async_context)
				g_main_context_unref (priv->async_context);
			priv->async_context = g_main_context_get_thread_default ();
			if (priv->async_context)
				g_main_context_ref (priv->async_context);
		}
		break;
	case PROP_TIMEOUT:
		priv->io_timeout = g_value_get_uint (value);
		break;
	case PROP_USER_AGENT:
		g_free (priv->user_agent);
		user_agent = g_value_get_string (value);
		if (!user_agent)
			priv->user_agent = NULL;
		else if (!*user_agent) {
			priv->user_agent =
				g_strdup (SOUP_SESSION_USER_AGENT_BASE);
		} else if (g_str_has_suffix (user_agent, " ")) {
			priv->user_agent =
				g_strdup_printf ("%s%s", user_agent,
						 SOUP_SESSION_USER_AGENT_BASE);
		} else
			priv->user_agent = g_strdup (user_agent);
		break;
	case PROP_ACCEPT_LANGUAGE:
		g_free (priv->accept_language);
		priv->accept_language = g_strdup (g_value_get_string (value));
		priv->accept_language_auto = FALSE;
		break;
	case PROP_ACCEPT_LANGUAGE_AUTO:
		priv->accept_language_auto = g_value_get_boolean (value);
		if (priv->accept_language) {
			g_free (priv->accept_language);
			priv->accept_language = NULL;
		}

		/* Get languages from system if needed */
		if (priv->accept_language_auto)
			priv->accept_language = accept_languages_from_system ();
		break;
	case PROP_IDLE_TIMEOUT:
		priv->idle_timeout = g_value_get_uint (value);
		break;
	case PROP_ADD_FEATURE:
		soup_session_add_feature (session, g_value_get_object (value));
		break;
	case PROP_ADD_FEATURE_BY_TYPE:
		soup_session_add_feature_by_type (session, g_value_get_gtype (value));
		break;
	case PROP_REMOVE_FEATURE_BY_TYPE:
		soup_session_remove_feature_by_type (session, g_value_get_gtype (value));
		break;
	case PROP_HTTP_ALIASES:
		set_aliases (&priv->http_aliases, g_value_get_boxed (value));
		break;
	case PROP_HTTPS_ALIASES:
		set_aliases (&priv->https_aliases, g_value_get_boxed (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
soup_session_get_property (GObject *object, guint prop_id,
			   GValue *value, GParamSpec *pspec)
{
	SoupSession *session = SOUP_SESSION (object);
	SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);
	SoupSessionFeature *feature;
	GTlsDatabase *tlsdb;

	switch (prop_id) {
	case PROP_PROXY_URI:
		feature = soup_session_get_feature (session, SOUP_TYPE_PROXY_RESOLVER_STATIC);
		if (feature) {
			g_object_get_property (G_OBJECT (feature),
					       SOUP_PROXY_RESOLVER_STATIC_PROXY_URI,
					       value);
		} else
			g_value_set_boxed (value, NULL);
		break;
	case PROP_MAX_CONNS:
		g_value_set_int (value, priv->max_conns);
		break;
	case PROP_MAX_CONNS_PER_HOST:
		g_value_set_int (value, priv->max_conns_per_host);
		break;
	case PROP_USE_NTLM:
		feature = soup_session_get_feature (session, SOUP_TYPE_AUTH_MANAGER_NTLM);
		if (feature)
			g_value_set_boolean (value, soup_session_feature_has_feature (feature, SOUP_TYPE_AUTH_NTLM));
		else
			g_value_set_boolean (value, FALSE);
		break;
	case PROP_SSL_CA_FILE:
		g_value_set_string (value, priv->ssl_ca_file);
		break;
	case PROP_SSL_USE_SYSTEM_CA_FILE:
		tlsdb = g_tls_backend_get_default_database (g_tls_backend_get_default ());
		g_value_set_boolean (value, priv->tlsdb == tlsdb);
		g_object_unref (tlsdb);
		break;
	case PROP_TLS_DATABASE:
		g_value_set_object (value, priv->tlsdb);
		break;
	case PROP_SSL_STRICT:
		g_value_set_boolean (value, priv->ssl_strict);
		break;
	case PROP_ASYNC_CONTEXT:
		g_value_set_pointer (value, priv->async_context ? g_main_context_ref (priv->async_context) : NULL);
		break;
	case PROP_USE_THREAD_CONTEXT:
		g_value_set_boolean (value, priv->use_thread_context);
		break;
	case PROP_TIMEOUT:
		g_value_set_uint (value, priv->io_timeout);
		break;
	case PROP_USER_AGENT:
		g_value_set_string (value, priv->user_agent);
		break;
	case PROP_ACCEPT_LANGUAGE:
		g_value_set_string (value, priv->accept_language);
		break;
	case PROP_ACCEPT_LANGUAGE_AUTO:
		g_value_set_boolean (value, priv->accept_language_auto);
		break;
	case PROP_IDLE_TIMEOUT:
		g_value_set_uint (value, priv->idle_timeout);
		break;
	case PROP_HTTP_ALIASES:
		g_value_set_boxed (value, priv->http_aliases);
		break;
	case PROP_HTTPS_ALIASES:
		g_value_set_boxed (value, priv->https_aliases);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static gboolean
uri_is_http (SoupSessionPrivate *priv, SoupURI *uri)
{
	int i;

	if (uri->scheme == SOUP_URI_SCHEME_HTTP)
		return TRUE;
	else if (uri->scheme == SOUP_URI_SCHEME_HTTPS)
		return FALSE;
	else if (!priv->http_aliases)
		return FALSE;

	for (i = 0; priv->http_aliases[i]; i++) {
		if (uri->scheme == priv->http_aliases[i])
			return TRUE;
	}

	if (!priv->http_aliases[1] && !strcmp (priv->http_aliases[0], "*"))
		return TRUE;
	else
		return FALSE;
}

static gboolean
uri_is_https (SoupSessionPrivate *priv, SoupURI *uri)
{
	int i;

	if (uri->scheme == SOUP_URI_SCHEME_HTTPS)
		return TRUE;
	else if (uri->scheme == SOUP_URI_SCHEME_HTTP)
		return FALSE;
	else if (!priv->https_aliases)
		return FALSE;

	for (i = 0; priv->https_aliases[i]; i++) {
		if (uri->scheme == priv->https_aliases[i])
			return TRUE;
	}

	return FALSE;
}

/**
 * soup_session_get_async_context:
 * @session: a #SoupSession
 *
 * Gets @session's async_context. This does not add a ref to the
 * context, so you will need to ref it yourself if you want it to
 * outlive its session.
 *
 * If #SoupSession:use-thread-context is true, this will return the
 * current thread-default main context.
 *
 * Return value: (transfer none): @session's #GMainContext, which may
 * be %NULL
 **/
GMainContext *
soup_session_get_async_context (SoupSession *session)
{
	SoupSessionPrivate *priv;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	priv = SOUP_SESSION_GET_PRIVATE (session);

	if (priv->use_thread_context)
		return g_main_context_get_thread_default ();
	else
		return priv->async_context;
}

/* Hosts */

/* Note that we can't use soup_uri_host_hash() and soup_uri_host_equal()
 * because we want to ignore the protocol; http://example.com and
 * webcal://example.com are the same host.
 */
static guint
soup_host_uri_hash (gconstpointer key)
{
	const SoupURI *uri = key;

	g_return_val_if_fail (uri != NULL && uri->host != NULL, 0);

	return uri->port + soup_str_case_hash (uri->host);
}

static gboolean
soup_host_uri_equal (gconstpointer v1, gconstpointer v2)
{
	const SoupURI *one = v1;
	const SoupURI *two = v2;

	g_return_val_if_fail (one != NULL && two != NULL, one == two);
	g_return_val_if_fail (one->host != NULL && two->host != NULL, one->host == two->host);

	if (one->port != two->port)
		return FALSE;

	return g_ascii_strcasecmp (one->host, two->host) == 0;
}


static SoupSessionHost *
soup_session_host_new (SoupSession *session, SoupURI *uri)
{
	SoupSessionHost *host;

	host = g_slice_new0 (SoupSessionHost);
	host->uri = soup_uri_copy_host (uri);
	if (host->uri->scheme != SOUP_URI_SCHEME_HTTP &&
	    host->uri->scheme != SOUP_URI_SCHEME_HTTPS) {
		SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);

		if (uri_is_https (priv, host->uri))
			host->uri->scheme = SOUP_URI_SCHEME_HTTPS;
		else
			host->uri->scheme = SOUP_URI_SCHEME_HTTP;
	}

	host->addr = g_object_new (SOUP_TYPE_ADDRESS,
				   SOUP_ADDRESS_NAME, host->uri->host,
				   SOUP_ADDRESS_PORT, host->uri->port,
				   SOUP_ADDRESS_PROTOCOL, host->uri->scheme,
				   NULL);
	host->keep_alive_src = NULL;
	host->session = session;

	return host;
}

/* Requires conn_lock to be locked */
static SoupSessionHost *
get_host_for_uri (SoupSession *session, SoupURI *uri)
{
	SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);
	SoupSessionHost *host;

	if (uri_is_https (priv, uri))
		host = g_hash_table_lookup (priv->https_hosts, uri);
	else
		host = g_hash_table_lookup (priv->http_hosts, uri);
	if (host)
		return host;

	host = soup_session_host_new (session, uri);

	if (uri_is_https (priv, uri))
		g_hash_table_insert (priv->https_hosts, host->uri, host);
	else
		g_hash_table_insert (priv->http_hosts, host->uri, host);

	return host;
}

/* Note: get_host_for_message doesn't lock the conn_lock. The caller
 * must do it itself if there's a chance the host doesn't already
 * exist.
 */
static SoupSessionHost *
get_host_for_message (SoupSession *session, SoupMessage *msg)
{
	return get_host_for_uri (session, soup_message_get_uri (msg));
}

static void
free_host (SoupSessionHost *host)
{
	g_warn_if_fail (host->connections == NULL);

	if (host->keep_alive_src) {
		g_source_destroy (host->keep_alive_src);
		g_source_unref (host->keep_alive_src);
	}

	soup_uri_free (host->uri);
	g_object_unref (host->addr);
	g_slice_free (SoupSessionHost, host);
}

static void
soup_session_real_auth_required (SoupSession *session, SoupMessage *msg,
	       SoupAuth *auth, gboolean retrying)
{
	g_signal_emit (session, signals[AUTHENTICATE], 0, msg, auth, retrying);
}

static void
auth_manager_authenticate (SoupAuthManager *manager, SoupMessage *msg,
			   SoupAuth *auth, gboolean retrying,
			   gpointer session)
{
	SOUP_SESSION_GET_CLASS (session)->auth_required (
		session, msg, auth, retrying);
}

#define SOUP_SESSION_WOULD_REDIRECT_AS_GET(session, msg) \
	((msg)->status_code == SOUP_STATUS_SEE_OTHER || \
	 ((msg)->status_code == SOUP_STATUS_FOUND && \
	  !SOUP_METHOD_IS_SAFE ((msg)->method)) || \
	 ((msg)->status_code == SOUP_STATUS_MOVED_PERMANENTLY && \
	  (msg)->method == SOUP_METHOD_POST))

#define SOUP_SESSION_WOULD_REDIRECT_AS_SAFE(session, msg) \
	(((msg)->status_code == SOUP_STATUS_MOVED_PERMANENTLY || \
	  (msg)->status_code == SOUP_STATUS_TEMPORARY_REDIRECT || \
	  (msg)->status_code == SOUP_STATUS_FOUND) && \
	 SOUP_METHOD_IS_SAFE ((msg)->method))

static inline SoupURI *
redirection_uri (SoupMessage *msg)
{
	const char *new_loc;
	SoupURI *new_uri;

	new_loc = soup_message_headers_get_one (msg->response_headers,
						"Location");
	if (!new_loc)
		return NULL;
	new_uri = soup_uri_new_with_base (soup_message_get_uri (msg), new_loc);
	if (!new_uri || !new_uri->host) {
		if (new_uri)
			soup_uri_free (new_uri);
		return NULL;
	}

	return new_uri;
}

/**
 * soup_session_would_redirect:
 * @session: a #SoupSession
 * @msg: a #SoupMessage that has response headers
 *
 * Checks if @msg contains a response that would cause @session to
 * redirect it to a new URL (ignoring @msg's %SOUP_MESSAGE_NO_REDIRECT
 * flag, and the number of times it has already been redirected).
 *
 * Return value: whether @msg would be redirected
 *
 * Since: 2.38
 */
gboolean
soup_session_would_redirect (SoupSession *session, SoupMessage *msg)
{
	SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);
	SoupURI *new_uri;

	/* It must have an appropriate status code and method */
	if (!SOUP_SESSION_WOULD_REDIRECT_AS_GET (session, msg) &&
	    !SOUP_SESSION_WOULD_REDIRECT_AS_SAFE (session, msg))
		return FALSE;

	/* and a Location header that parses to an http URI */
	if (!soup_message_headers_get_one (msg->response_headers, "Location"))
		return FALSE;
	new_uri = redirection_uri (msg);
	if (!new_uri)
		return FALSE;
	if (!new_uri->host || !*new_uri->host ||
	    (!uri_is_http (priv, new_uri) && !uri_is_https (priv, new_uri))) {
		soup_uri_free (new_uri);
		return FALSE;
	}

	soup_uri_free (new_uri);
	return TRUE;
}

/**
 * soup_session_redirect_message:
 * @session: the session
 * @msg: a #SoupMessage that has received a 3xx response
 *
 * Updates @msg's URI according to its status code and "Location"
 * header, and requeues it on @session. Use this when you have set
 * %SOUP_MESSAGE_NO_REDIRECT on a message, but have decided to allow a
 * particular redirection to occur, or if you want to allow a
 * redirection that #SoupSession will not perform automatically (eg,
 * redirecting a non-safe method such as DELETE).
 *
 * If @msg's status code indicates that it should be retried as a GET
 * request, then @msg will be modified accordingly.
 *
 * If @msg has already been redirected too many times, this will
 * cause it to fail with %SOUP_STATUS_TOO_MANY_REDIRECTS.
 *
 * Return value: %TRUE if a redirection was applied, %FALSE if not
 * (eg, because there was no Location header, or it could not be
 * parsed).
 *
 * Since: 2.38
 */
gboolean
soup_session_redirect_message (SoupSession *session, SoupMessage *msg)
{
	SoupMessageQueueItem *item;
	SoupURI *new_uri;

	new_uri = redirection_uri (msg);
	if (!new_uri)
		return FALSE;

	item = soup_message_queue_lookup (soup_session_get_queue (session), msg);
	if (!item) {
		soup_uri_free (new_uri);
		return FALSE;
	}
	if (item->redirection_count >= SOUP_SESSION_MAX_REDIRECTION_COUNT) {
		soup_uri_free (new_uri);
		soup_session_cancel_message (session, msg, SOUP_STATUS_TOO_MANY_REDIRECTS);
		soup_message_queue_item_unref (item);
		return FALSE;
	}
	item->redirection_count++;
	soup_message_queue_item_unref (item);

	if (SOUP_SESSION_WOULD_REDIRECT_AS_GET (session, msg)) {
		if (msg->method != SOUP_METHOD_HEAD) {
			g_object_set (msg,
				      SOUP_MESSAGE_METHOD, SOUP_METHOD_GET,
				      NULL);
		}
		soup_message_set_request (msg, NULL,
					  SOUP_MEMORY_STATIC, NULL, 0);
		soup_message_headers_set_encoding (msg->request_headers,
						   SOUP_ENCODING_NONE);
	}

	soup_message_set_uri (msg, new_uri);
	soup_uri_free (new_uri);

	soup_session_requeue_message (session, msg);
	return TRUE;
}

static void
redirect_handler (SoupMessage *msg, gpointer user_data)
{
	SoupMessageQueueItem *item = user_data;
	SoupSession *session = item->session;

	if (!soup_session_would_redirect (session, msg)) {
		SoupURI *new_uri = redirection_uri (msg);
		gboolean invalid = !new_uri || !new_uri->host;

		if (new_uri)
			soup_uri_free (new_uri);
		if (invalid && !item->new_api) {
			soup_message_set_status_full (msg,
						      SOUP_STATUS_MALFORMED,
						      "Invalid Redirect URL");
		}
		return;
	}

	soup_session_redirect_message (session, msg);
}

static void
message_restarted (SoupMessage *msg, gpointer user_data)
{
	SoupMessageQueueItem *item = user_data;

	if (item->conn &&
	    (!soup_message_is_keepalive (msg) ||
	     SOUP_STATUS_IS_REDIRECTION (msg->status_code))) {
		if (soup_connection_get_state (item->conn) == SOUP_CONNECTION_IN_USE)
			soup_connection_set_state (item->conn, SOUP_CONNECTION_IDLE);
		soup_session_set_item_connection (item->session, item, NULL);
	}

	soup_message_cleanup_response (msg);
}

SoupMessageQueueItem *
soup_session_append_queue_item (SoupSession *session, SoupMessage *msg,
				SoupSessionCallback callback, gpointer user_data)
{
	SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);
	SoupMessageQueueItem *item;
	SoupSessionHost *host;

	soup_message_cleanup_response (msg);

	item = soup_message_queue_append (priv->queue, msg, callback, user_data);

	g_mutex_lock (&priv->conn_lock);
	host = get_host_for_message (session, item->msg);
	host->num_messages++;
	g_mutex_unlock (&priv->conn_lock);

	if (!(soup_message_get_flags (msg) & SOUP_MESSAGE_NO_REDIRECT)) {
		soup_message_add_header_handler (
			msg, "got_body", "Location",
			G_CALLBACK (redirect_handler), item);
	}
	g_signal_connect (msg, "restarted",
			  G_CALLBACK (message_restarted), item);

	g_signal_emit (session, signals[REQUEST_QUEUED], 0, msg);

	soup_message_queue_item_ref (item);
	return item;
}

void
soup_session_send_queue_item (SoupSession *session,
			      SoupMessageQueueItem *item,
			      SoupMessageCompletionFn completion_cb)
{
	SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);
	const char *conn_header;

	if (priv->user_agent) {
		soup_message_headers_replace (item->msg->request_headers,
					      "User-Agent", priv->user_agent);
	}

	if (priv->accept_language &&
	    !soup_message_headers_get_list (item->msg->request_headers,
					    "Accept-Language")) {
		soup_message_headers_append (item->msg->request_headers,
					     "Accept-Language",
					     priv->accept_language);
	}

	/* Force keep alive connections for HTTP 1.0. Performance will
	 * improve when issuing multiple requests to the same host in
	 * a short period of time, as we wouldn't need to establish
	 * new connections. Keep alive is implicit for HTTP 1.1.
	 */
	conn_header = soup_message_headers_get_list (item->msg->request_headers, "Connection");
	if (!conn_header ||
	    (!soup_header_contains (conn_header, "Keep-Alive") &&
	     !soup_header_contains (conn_header, "close")))
		soup_message_headers_append (item->msg->request_headers,
					     "Connection", "Keep-Alive");

	g_signal_emit (session, signals[REQUEST_STARTED], 0,
		       item->msg, soup_connection_get_socket (item->conn));
	if (item->state == SOUP_MESSAGE_RUNNING)
		soup_connection_send_request (item->conn, item, completion_cb, item);
}

gboolean
soup_session_cleanup_connections (SoupSession *session,
				  gboolean     prune_idle)
{
	SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);
	GSList *conns = NULL, *c;
	GHashTableIter iter;
	gpointer conn, host;
	SoupConnectionState state;

	g_mutex_lock (&priv->conn_lock);
	g_hash_table_iter_init (&iter, priv->conns);
	while (g_hash_table_iter_next (&iter, &conn, &host)) {
		state = soup_connection_get_state (conn);
		if (state == SOUP_CONNECTION_REMOTE_DISCONNECTED ||
		    (prune_idle && state == SOUP_CONNECTION_IDLE)) {
			conns = g_slist_prepend (conns, g_object_ref (conn));
			g_hash_table_iter_remove (&iter);
			drop_connection (session, host, conn);
		}
	}
	g_mutex_unlock (&priv->conn_lock);

	if (!conns)
		return FALSE;

	for (c = conns; c; c = c->next) {
		conn = c->data;
		soup_connection_disconnect (conn);
		g_object_unref (conn);
	}
	g_slist_free (conns);

	return TRUE;
}

static gboolean
free_unused_host (gpointer user_data)
{
	SoupSessionHost *host = (SoupSessionHost *) user_data;
	SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (host->session);

	g_mutex_lock (&priv->conn_lock);

	/* In a multithreaded session, a connection might have been
	 * added while we were waiting for conn_lock.
	 */
	if (host->connections) {
		g_mutex_unlock (&priv->conn_lock);
		return FALSE;
	}

	/* This will free the host in addition to removing it from the
	 * hash table
	 */
	if (host->uri->scheme == SOUP_URI_SCHEME_HTTPS)
		g_hash_table_remove (priv->https_hosts, host->uri);
	else
		g_hash_table_remove (priv->http_hosts, host->uri);
	g_mutex_unlock (&priv->conn_lock);

	return FALSE;
}

static void
drop_connection (SoupSession *session, SoupSessionHost *host, SoupConnection *conn)
{
	SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);

	/* Note: caller must hold conn_lock, and must remove @conn
	 * from priv->conns itself.
	 */

	if (host) {
		host->connections = g_slist_remove (host->connections, conn);
		host->num_conns--;

		/* Free the SoupHost (and its SoupAddress) if there
		 * has not been any new connection to the host during
		 * the last HOST_KEEP_ALIVE msecs.
		 */
		if (host->num_conns == 0) {
			g_assert (host->keep_alive_src == NULL);
			host->keep_alive_src = soup_add_timeout (priv->async_context,
								 HOST_KEEP_ALIVE,
								 free_unused_host,
								 host);
			host->keep_alive_src = g_source_ref (host->keep_alive_src);
		}

		if (soup_connection_get_ssl_fallback (conn))
			host->ssl_fallback = TRUE;
	}

	g_signal_handlers_disconnect_by_func (conn, connection_disconnected, session);
	g_signal_handlers_disconnect_by_func (conn, connection_state_changed, session);
	priv->num_conns--;

	g_object_unref (conn);
}

static void
connection_disconnected (SoupConnection *conn, gpointer user_data)
{
	SoupSession *session = user_data;
	SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);
	SoupSessionHost *host;

	g_mutex_lock (&priv->conn_lock);

	host = g_hash_table_lookup (priv->conns, conn);
	if (host)
		g_hash_table_remove (priv->conns, conn);
	drop_connection (session, host, conn);

	g_mutex_unlock (&priv->conn_lock);

	SOUP_SESSION_GET_CLASS (session)->kick (session);
}

static void
connection_state_changed (GObject *object, GParamSpec *param, gpointer user_data)
{
	SoupSession *session = user_data;
	SoupConnection *conn = SOUP_CONNECTION (object);

	if (soup_connection_get_state (conn) == SOUP_CONNECTION_IDLE)
		SOUP_SESSION_GET_CLASS (session)->kick (session);
}

SoupMessageQueueItem *
soup_session_make_connect_message (SoupSession    *session,
				   SoupConnection *conn)
{
	SoupURI *uri;
	SoupMessage *msg;
	SoupMessageQueueItem *item;

	uri = soup_connection_get_remote_uri (conn);
	msg = soup_message_new_from_uri (SOUP_METHOD_CONNECT, uri);
	soup_message_set_flags (msg, SOUP_MESSAGE_NO_REDIRECT);

	item = soup_session_append_queue_item (session, msg, NULL, NULL);
	soup_session_set_item_connection (session, item, conn);
	g_object_unref (msg);
	item->state = SOUP_MESSAGE_RUNNING;

	g_signal_emit (session, signals[TUNNELING], 0, conn);
	return item;
}

gboolean
soup_session_get_connection (SoupSession *session,
			     SoupMessageQueueItem *item,
			     gboolean *try_pruning)
{
	SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);
	SoupConnection *conn;
	SoupSessionHost *host;
	GSList *conns;
	int num_pending = 0;
	gboolean need_new_connection;

	if (priv->disposed)
		return FALSE;

	if (item->conn) {
		g_return_val_if_fail (soup_connection_get_state (item->conn) != SOUP_CONNECTION_DISCONNECTED, FALSE);
		return TRUE;
	}

	need_new_connection =
		(soup_message_get_flags (item->msg) & SOUP_MESSAGE_NEW_CONNECTION) ||
		(!(soup_message_get_flags (item->msg) & SOUP_MESSAGE_IDEMPOTENT) &&
		 !SOUP_METHOD_IS_IDEMPOTENT (item->msg->method));

	g_mutex_lock (&priv->conn_lock);

	host = get_host_for_message (session, item->msg);
	for (conns = host->connections; conns; conns = conns->next) {
		if (!need_new_connection && soup_connection_get_state (conns->data) == SOUP_CONNECTION_IDLE) {
			soup_connection_set_state (conns->data, SOUP_CONNECTION_IN_USE);
			g_mutex_unlock (&priv->conn_lock);
			soup_session_set_item_connection (session, item, conns->data);
			soup_message_set_https_status (item->msg, item->conn);
			return TRUE;
		} else if (soup_connection_get_state (conns->data) == SOUP_CONNECTION_CONNECTING)
			num_pending++;
	}

	/* Limit the number of pending connections; num_messages / 2
	 * is somewhat arbitrary...
	 */
	if (num_pending > host->num_messages / 2) {
		g_mutex_unlock (&priv->conn_lock);
		return FALSE;
	}

	if (host->num_conns >= priv->max_conns_per_host) {
		if (need_new_connection)
			*try_pruning = TRUE;
		g_mutex_unlock (&priv->conn_lock);
		return FALSE;
	}

	if (priv->num_conns >= priv->max_conns) {
		*try_pruning = TRUE;
		g_mutex_unlock (&priv->conn_lock);
		return FALSE;
	}

	conn = g_object_new (
		SOUP_TYPE_CONNECTION,
		SOUP_CONNECTION_REMOTE_URI, host->uri,
		SOUP_CONNECTION_PROXY_RESOLVER, soup_session_get_feature (session, SOUP_TYPE_PROXY_URI_RESOLVER),
		SOUP_CONNECTION_SSL, uri_is_https (priv, soup_message_get_uri (item->msg)),
		SOUP_CONNECTION_SSL_CREDENTIALS, priv->tlsdb,
		SOUP_CONNECTION_SSL_STRICT, (priv->tlsdb != NULL) && priv->ssl_strict,
		SOUP_CONNECTION_ASYNC_CONTEXT, priv->async_context,
		SOUP_CONNECTION_USE_THREAD_CONTEXT, priv->use_thread_context,
		SOUP_CONNECTION_TIMEOUT, priv->io_timeout,
		SOUP_CONNECTION_IDLE_TIMEOUT, priv->idle_timeout,
		SOUP_CONNECTION_SSL_FALLBACK, host->ssl_fallback,
		NULL);
	g_signal_connect (conn, "disconnected",
			  G_CALLBACK (connection_disconnected),
			  session);
	g_signal_connect (conn, "notify::state",
			  G_CALLBACK (connection_state_changed),
			  session);

	g_signal_emit (session, signals[CONNECTION_CREATED], 0, conn);

	g_hash_table_insert (priv->conns, conn, host);

	priv->num_conns++;
	host->num_conns++;
	host->connections = g_slist_prepend (host->connections, conn);

	if (host->keep_alive_src) {
		g_source_destroy (host->keep_alive_src);
		g_source_unref (host->keep_alive_src);
		host->keep_alive_src = NULL;
	}

	g_mutex_unlock (&priv->conn_lock);
	soup_session_set_item_connection (session, item, conn);
	return TRUE;
}

SoupMessageQueue *
soup_session_get_queue (SoupSession *session)
{
	SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);

	return priv->queue;
}

void
soup_session_unqueue_item (SoupSession          *session,
			   SoupMessageQueueItem *item)
{
	SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);
	SoupSessionHost *host;

	if (item->conn) {
		if (item->msg->method != SOUP_METHOD_CONNECT ||
		    !SOUP_STATUS_IS_SUCCESSFUL (item->msg->status_code))
			soup_connection_set_state (item->conn, SOUP_CONNECTION_IDLE);
		soup_session_set_item_connection (session, item, NULL);
	}

	if (item->state != SOUP_MESSAGE_FINISHED) {
		g_warning ("finished an item with state %d", item->state);
		return;
	}

	soup_message_queue_remove (priv->queue, item);

	g_mutex_lock (&priv->conn_lock);
	host = get_host_for_message (session, item->msg);
	host->num_messages--;
	g_mutex_unlock (&priv->conn_lock);

	/* g_signal_handlers_disconnect_by_func doesn't work if you
	 * have a metamarshal, meaning it doesn't work with
	 * soup_message_add_header_handler()
	 */
	g_signal_handlers_disconnect_matched (item->msg, G_SIGNAL_MATCH_DATA,
					      0, 0, NULL, NULL, item);
	g_signal_emit (session, signals[REQUEST_UNQUEUED], 0, item->msg);
	soup_message_queue_item_unref (item);
}

static void
proxy_connection_event (SoupConnection      *conn,
			GSocketClientEvent   event,
			GIOStream           *connection,
			gpointer             user_data)
{
	SoupMessageQueueItem *item = user_data;

	soup_message_network_event (item->msg, event, connection);
}

void
soup_session_set_item_connection (SoupSession          *session,
				  SoupMessageQueueItem *item,
				  SoupConnection       *conn)
{
	if (item->conn) {
		g_signal_handlers_disconnect_by_func (item->conn, proxy_connection_event, item);
		g_object_unref (item->conn);
	}

	item->conn = conn;

	if (item->conn) {
		g_object_ref (item->conn);
		g_signal_connect (item->conn, "event",
				  G_CALLBACK (proxy_connection_event), item);
	}
}

void
soup_session_set_item_status (SoupSession          *session,
			      SoupMessageQueueItem *item,
			      guint                 status_code)
{
	SoupURI *uri;
	char *msg;

	switch (status_code) {
	case SOUP_STATUS_CANT_RESOLVE:
	case SOUP_STATUS_CANT_CONNECT:
		uri = soup_message_get_uri (item->msg);
		msg = g_strdup_printf ("%s (%s)",
				       soup_status_get_phrase (status_code),
				       uri->host);
		soup_message_set_status_full (item->msg, status_code, msg);
		g_free (msg);
		break;

	case SOUP_STATUS_CANT_RESOLVE_PROXY:
	case SOUP_STATUS_CANT_CONNECT_PROXY:
		if (item->proxy_uri && item->proxy_uri->host) {
			msg = g_strdup_printf ("%s (%s)",
					       soup_status_get_phrase (status_code),
					       item->proxy_uri->host);
			soup_message_set_status_full (item->msg, status_code, msg);
			g_free (msg);
			break;
		}
		soup_message_set_status (item->msg, status_code);
		break;

	case SOUP_STATUS_SSL_FAILED:
		if (!g_tls_backend_supports_tls (g_tls_backend_get_default ())) {
			soup_message_set_status_full (item->msg, status_code,
						      "TLS/SSL support not available; install glib-networking");
		} else
			soup_message_set_status (item->msg, status_code);
		break;

	default:
		soup_message_set_status (item->msg, status_code);
		break;
	}
}

/**
 * SoupSessionCallback:
 * @session: the session
 * @msg: the message that has finished
 * @user_data: the data passed to soup_session_queue_message
 *
 * Prototype for the callback passed to soup_session_queue_message(),
 * qv.
 **/

/**
 * soup_session_queue_message:
 * @session: a #SoupSession
 * @msg: (transfer full): the message to queue
 * @callback: (allow-none) (scope async): a #SoupSessionCallback which will
 * be called after the message completes or when an unrecoverable error occurs.
 * @user_data: (allow-none): a pointer passed to @callback.
 * 
 * Queues the message @msg for sending. All messages are processed
 * while the glib main loop runs. If @msg has been processed before,
 * any resources related to the time it was last sent are freed.
 *
 * Upon message completion, the callback specified in @callback will
 * be invoked (in the thread associated with @session's async
 * context). If after returning from this callback the message has not
 * been requeued, @msg will be unreffed.
 */
void
soup_session_queue_message (SoupSession *session, SoupMessage *msg,
			    SoupSessionCallback callback, gpointer user_data)
{
	g_return_if_fail (SOUP_IS_SESSION (session));
	g_return_if_fail (SOUP_IS_MESSAGE (msg));

	SOUP_SESSION_GET_CLASS (session)->queue_message (session, msg,
							 callback, user_data);
	/* The SoupMessageQueueItem will hold a ref on @msg until it is
	 * finished, so we can drop the ref adopted from the caller now.
	 */
	g_object_unref (msg);
}

static void
soup_session_real_requeue_message (SoupSession *session, SoupMessage *msg)
{
	SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);
	SoupMessageQueueItem *item;

	item = soup_message_queue_lookup (priv->queue, msg);
	g_return_if_fail (item != NULL);
	item->state = SOUP_MESSAGE_RESTARTING;
	soup_message_queue_item_unref (item);
}

/**
 * soup_session_requeue_message:
 * @session: a #SoupSession
 * @msg: the message to requeue
 *
 * This causes @msg to be placed back on the queue to be attempted
 * again.
 **/
void
soup_session_requeue_message (SoupSession *session, SoupMessage *msg)
{
	g_return_if_fail (SOUP_IS_SESSION (session));
	g_return_if_fail (SOUP_IS_MESSAGE (msg));

	SOUP_SESSION_GET_CLASS (session)->requeue_message (session, msg);
}


/**
 * soup_session_send_message:
 * @session: a #SoupSession
 * @msg: the message to send
 * 
 * Synchronously send @msg. This call will not return until the
 * transfer is finished successfully or there is an unrecoverable
 * error.
 *
 * @msg is not freed upon return.
 *
 * Return value: the HTTP status code of the response
 */
guint
soup_session_send_message (SoupSession *session, SoupMessage *msg)
{
	g_return_val_if_fail (SOUP_IS_SESSION (session), SOUP_STATUS_MALFORMED);
	g_return_val_if_fail (SOUP_IS_MESSAGE (msg), SOUP_STATUS_MALFORMED);

	return SOUP_SESSION_GET_CLASS (session)->send_message (session, msg);
}


/**
 * soup_session_pause_message:
 * @session: a #SoupSession
 * @msg: a #SoupMessage currently running on @session
 *
 * Pauses HTTP I/O on @msg. Call soup_session_unpause_message() to
 * resume I/O.
 **/
void
soup_session_pause_message (SoupSession *session,
			    SoupMessage *msg)
{
	SoupSessionPrivate *priv;
	SoupMessageQueueItem *item;

	g_return_if_fail (SOUP_IS_SESSION (session));
	g_return_if_fail (SOUP_IS_MESSAGE (msg));

	priv = SOUP_SESSION_GET_PRIVATE (session);
	item = soup_message_queue_lookup (priv->queue, msg);
	g_return_if_fail (item != NULL);

	item->paused = TRUE;
	if (item->state == SOUP_MESSAGE_RUNNING)
		soup_message_io_pause (msg);
	soup_message_queue_item_unref (item);
}

/**
 * soup_session_unpause_message:
 * @session: a #SoupSession
 * @msg: a #SoupMessage currently running on @session
 *
 * Resumes HTTP I/O on @msg. Use this to resume after calling
 * soup_session_pause_message().
 *
 * If @msg is being sent via blocking I/O, this will resume reading or
 * writing immediately. If @msg is using non-blocking I/O, then
 * reading or writing won't resume until you return to the main loop.
 **/
void
soup_session_unpause_message (SoupSession *session,
			      SoupMessage *msg)
{
	SoupSessionPrivate *priv;
	SoupMessageQueueItem *item;

	g_return_if_fail (SOUP_IS_SESSION (session));
	g_return_if_fail (SOUP_IS_MESSAGE (msg));

	priv = SOUP_SESSION_GET_PRIVATE (session);
	item = soup_message_queue_lookup (priv->queue, msg);
	g_return_if_fail (item != NULL);

	item->paused = FALSE;
	if (item->state == SOUP_MESSAGE_RUNNING)
		soup_message_io_unpause (msg);
	soup_message_queue_item_unref (item);

	SOUP_SESSION_GET_CLASS (session)->kick (session);
}


static void
soup_session_real_cancel_message (SoupSession *session, SoupMessage *msg, guint status_code)
{
	SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);
	SoupMessageQueueItem *item;

	item = soup_message_queue_lookup (priv->queue, msg);
	g_return_if_fail (item != NULL);

	item->paused = FALSE;
	soup_message_set_status (msg, status_code);
	g_cancellable_cancel (item->cancellable);

	soup_message_queue_item_unref (item);
}

/**
 * soup_session_cancel_message:
 * @session: a #SoupSession
 * @msg: the message to cancel
 * @status_code: status code to set on @msg (generally
 * %SOUP_STATUS_CANCELLED)
 *
 * Causes @session to immediately finish processing @msg (regardless
 * of its current state) with a final status_code of @status_code. You
 * may call this at any time after handing @msg off to @session; if
 * @session has started sending the request but has not yet received
 * the complete response, then it will close the request's connection.
 * Note that with non-idempotent requests (eg,
 * <literal>POST</literal>, <literal>PUT</literal>,
 * <literal>DELETE</literal>) it is possible that you might cancel the
 * request after the server acts on it, but before it returns a
 * response, leaving the remote resource in an unknown state.
 *
 * If the message is cancelled while its response body is being read,
 * then the response body in @msg will be left partially-filled-in.
 * The response headers, on the other hand, will always be either
 * empty or complete.
 *
 * For messages queued with soup_session_queue_message() (and
 * cancelled from the same thread), the callback will be invoked
 * before soup_session_cancel_message() returns.
 **/
void
soup_session_cancel_message (SoupSession *session, SoupMessage *msg,
			     guint status_code)
{
	SoupSessionPrivate *priv;
	SoupMessageQueueItem *item;

	g_return_if_fail (SOUP_IS_SESSION (session));
	g_return_if_fail (SOUP_IS_MESSAGE (msg));

	priv = SOUP_SESSION_GET_PRIVATE (session);
	item = soup_message_queue_lookup (priv->queue, msg);
	/* If the message is already ending, don't do anything */
	if (!item)
		return;
	if (item->state == SOUP_MESSAGE_FINISHED) {
		soup_message_queue_item_unref (item);
		return;
	}

	SOUP_SESSION_GET_CLASS (session)->cancel_message (session, msg, status_code);
	soup_message_queue_item_unref (item);
}

static void
soup_session_real_flush_queue (SoupSession *session)
{
	SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);
	SoupMessageQueueItem *item;

	for (item = soup_message_queue_first (priv->queue);
	     item;
	     item = soup_message_queue_next (priv->queue, item)) {
		soup_session_cancel_message (session, item->msg,
					     SOUP_STATUS_CANCELLED);
	}
}

/**
 * soup_session_abort:
 * @session: the session
 *
 * Cancels all pending requests in @session and closes all idle
 * persistent connections.
 **/
void
soup_session_abort (SoupSession *session)
{
	SoupSessionPrivate *priv;
	GSList *conns, *c;
	GHashTableIter iter;
	gpointer conn, host;

	g_return_if_fail (SOUP_IS_SESSION (session));
	priv = SOUP_SESSION_GET_PRIVATE (session);

	SOUP_SESSION_GET_CLASS (session)->flush_queue (session);

	/* Close all connections */
	g_mutex_lock (&priv->conn_lock);
	conns = NULL;
	g_hash_table_iter_init (&iter, priv->conns);
	while (g_hash_table_iter_next (&iter, &conn, &host)) {
		conns = g_slist_prepend (conns, g_object_ref (conn));
		g_hash_table_iter_remove (&iter);
		drop_connection (session, host, conn);
	}
	g_mutex_unlock (&priv->conn_lock);

	for (c = conns; c; c = c->next) {
		soup_connection_disconnect (c->data);
		g_object_unref (c->data);
	}

	g_slist_free (conns);
}

static void
prefetch_uri (SoupSession *session, SoupURI *uri,
	      GCancellable *cancellable,
	      SoupAddressCallback callback, gpointer user_data)
{
	SoupSessionPrivate *priv;
	SoupSessionHost *host;
	SoupAddress *addr;

	priv = SOUP_SESSION_GET_PRIVATE (session);

	g_mutex_lock (&priv->conn_lock);
	host = get_host_for_uri (session, uri);
	addr = g_object_ref (host->addr);
	g_mutex_unlock (&priv->conn_lock);

	soup_address_resolve_async (addr,
				    soup_session_get_async_context (session),
				    cancellable, callback, user_data);
	g_object_unref (addr);
}

/**
 * soup_session_prepare_for_uri:
 * @session: a #SoupSession
 * @uri: a #SoupURI which may be required
 *
 * Tells @session that @uri may be requested shortly, and so the
 * session can try to prepare (resolving the domain name, obtaining
 * proxy address, etc.) in order to work more quickly once the URI is
 * actually requested.
 *
 * This method acts asynchronously, in @session's
 * #SoupSession:async_context. If you are using #SoupSessionSync and do
 * not have a main loop running, then you can't use this method.
 *
 * Since: 2.30
 *
 * Deprecated: 2.38: use soup_session_prefetch_dns() instead
 **/
void
soup_session_prepare_for_uri (SoupSession *session, SoupURI *uri)
{
	g_return_if_fail (SOUP_IS_SESSION (session));
	g_return_if_fail (uri != NULL);

	if (!uri->host)
		return;

	prefetch_uri (session, uri, NULL, NULL, NULL);
}

/**
* soup_session_prefetch_dns:
* @session: a #SoupSession
* @hostname: a hostname to be resolved
* @cancellable: (allow-none): a #GCancellable object, or %NULL
* @callback: (scope async) (allow-none): callback to call with the
*     result, or %NULL
* @user_data: data for @callback
*
* Tells @session that an URI from the given @hostname may be requested
* shortly, and so the session can try to prepare by resolving the
* domain name in advance, in order to work more quickly once the URI
* is actually requested.
*
* If @cancellable is non-%NULL, it can be used to cancel the
* resolution. @callback will still be invoked in this case, with a
* status of %SOUP_STATUS_CANCELLED.
*
* This method acts asynchronously, in @session's
* #SoupSession:async_context. If you are using #SoupSessionSync and do
* not have a main loop running, then you can't use this method.
*
* Since: 2.38
**/
void
soup_session_prefetch_dns (SoupSession *session, const char *hostname,
			   GCancellable *cancellable,
			   SoupAddressCallback callback, gpointer user_data)
{
	SoupURI *uri;

	g_return_if_fail (SOUP_IS_SESSION (session));
	g_return_if_fail (hostname != NULL);

	/* FIXME: Prefetching should work for both HTTP and HTTPS */
	uri = soup_uri_new (NULL);
	soup_uri_set_scheme (uri, SOUP_URI_SCHEME_HTTP);
	soup_uri_set_host (uri, hostname);
	soup_uri_set_path (uri, "");

	prefetch_uri (session, uri, cancellable, callback, user_data);
	soup_uri_free (uri);
}

/**
 * soup_session_add_feature:
 * @session: a #SoupSession
 * @feature: an object that implements #SoupSessionFeature
 *
 * Adds @feature's functionality to @session. You can also add a
 * feature to the session at construct time by using the
 * %SOUP_SESSION_ADD_FEATURE property.
 *
 * Since: 2.24
 **/
void
soup_session_add_feature (SoupSession *session, SoupSessionFeature *feature)
{
	SoupSessionPrivate *priv;

	g_return_if_fail (SOUP_IS_SESSION (session));
	g_return_if_fail (SOUP_IS_SESSION_FEATURE (feature));

	priv = SOUP_SESSION_GET_PRIVATE (session);
	priv->features = g_slist_prepend (priv->features, g_object_ref (feature));
	g_hash_table_remove_all (priv->features_cache);
	soup_session_feature_attach (feature, session);
}

/**
 * soup_session_add_feature_by_type:
 * @session: a #SoupSession
 * @feature_type: a #GType
 *
 * If @feature_type is the type of a class that implements
 * #SoupSessionFeature, this creates a new feature of that type and
 * adds it to @session as with soup_session_add_feature(). You can use
 * this when you don't need to customize the new feature in any way.
 *
 * If @feature_type is not a #SoupSessionFeature type, this gives
 * each existing feature on @session the chance to accept @feature_type
 * as a "subfeature". This can be used to add new #SoupAuth types,
 * for instance.
 *
 * You can also add a feature to the session at construct time by
 * using the %SOUP_SESSION_ADD_FEATURE_BY_TYPE property.
 *
 * Since: 2.24
 **/
void
soup_session_add_feature_by_type (SoupSession *session, GType feature_type)
{
	g_return_if_fail (SOUP_IS_SESSION (session));

	if (g_type_is_a (feature_type, SOUP_TYPE_SESSION_FEATURE)) {
		SoupSessionFeature *feature;

		feature = g_object_new (feature_type, NULL);
		soup_session_add_feature (session, feature);
		g_object_unref (feature);
	} else {
		SoupSessionPrivate *priv = SOUP_SESSION_GET_PRIVATE (session);
		GSList *f;

		for (f = priv->features; f; f = f->next) {
			if (soup_session_feature_add_feature (f->data, feature_type))
				return;
		}
		g_warning ("No feature manager for feature of type '%s'", g_type_name (feature_type));
	}
}

/**
 * soup_session_remove_feature:
 * @session: a #SoupSession
 * @feature: a feature that has previously been added to @session
 *
 * Removes @feature's functionality from @session.
 *
 * Since: 2.24
 **/
void
soup_session_remove_feature (SoupSession *session, SoupSessionFeature *feature)
{
	SoupSessionPrivate *priv;

	g_return_if_fail (SOUP_IS_SESSION (session));

	priv = SOUP_SESSION_GET_PRIVATE (session);
	if (g_slist_find (priv->features, feature)) {
		priv->features = g_slist_remove (priv->features, feature);
		g_hash_table_remove_all (priv->features_cache);
		soup_session_feature_detach (feature, session);
		g_object_unref (feature);
	}
}

/**
 * soup_session_remove_feature_by_type:
 * @session: a #SoupSession
 * @feature_type: a #GType
 *
 * Removes all features of type @feature_type (or any subclass of
 * @feature_type) from @session. You can also remove standard features
 * from the session at construct time by using the
 * %SOUP_SESSION_REMOVE_FEATURE_BY_TYPE property.
 *
 * Since: 2.24
 **/
void
soup_session_remove_feature_by_type (SoupSession *session, GType feature_type)
{
	SoupSessionPrivate *priv;
	GSList *f;

	g_return_if_fail (SOUP_IS_SESSION (session));

	priv = SOUP_SESSION_GET_PRIVATE (session);

	if (g_type_is_a (feature_type, SOUP_TYPE_SESSION_FEATURE)) {
	restart:
		for (f = priv->features; f; f = f->next) {
			if (G_TYPE_CHECK_INSTANCE_TYPE (f->data, feature_type)) {
				soup_session_remove_feature (session, f->data);
				goto restart;
			}
		}
	} else {
		for (f = priv->features; f; f = f->next) {
			if (soup_session_feature_remove_feature (f->data, feature_type))
				return;
		}
		g_warning ("No feature manager for feature of type '%s'", g_type_name (feature_type));
	}
}

/**
 * soup_session_get_features:
 * @session: a #SoupSession
 * @feature_type: the #GType of the class of features to get
 *
 * Generates a list of @session's features of type @feature_type. (If
 * you want to see all features, you can pass %SOUP_TYPE_SESSION_FEATURE
 * for @feature_type.)
 *
 * Return value: (transfer container) (element-type Soup.SessionFeature):
 * a list of features. You must free the list, but not its contents
 *
 * Since: 2.26
 **/
GSList *
soup_session_get_features (SoupSession *session, GType feature_type)
{
	SoupSessionPrivate *priv;
	GSList *f, *ret;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);

	priv = SOUP_SESSION_GET_PRIVATE (session);
	for (f = priv->features, ret = NULL; f; f = f->next) {
		if (G_TYPE_CHECK_INSTANCE_TYPE (f->data, feature_type))
			ret = g_slist_prepend (ret, f->data);
	}
	return g_slist_reverse (ret);
}

/**
 * soup_session_get_feature:
 * @session: a #SoupSession
 * @feature_type: the #GType of the feature to get
 *
 * Gets the first feature in @session of type @feature_type. For
 * features where there may be more than one feature of a given type,
 * use soup_session_get_features().
 *
 * Return value: (transfer none): a #SoupSessionFeature, or %NULL. The
 * feature is owned by @session.
 *
 * Since: 2.26
 **/
SoupSessionFeature *
soup_session_get_feature (SoupSession *session, GType feature_type)
{
	SoupSessionPrivate *priv;
	SoupSessionFeature *feature;
	GSList *f;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);

	priv = SOUP_SESSION_GET_PRIVATE (session);

	feature = g_hash_table_lookup (priv->features_cache,
				       GSIZE_TO_POINTER (feature_type));
	if (feature)
		return feature;

	for (f = priv->features; f; f = f->next) {
		feature = f->data;
		if (G_TYPE_CHECK_INSTANCE_TYPE (feature, feature_type)) {
			g_hash_table_insert (priv->features_cache,
					     GSIZE_TO_POINTER (feature_type),
					     feature);
			return feature;
		}
	}
	return NULL;
}

/**
 * soup_session_get_feature_for_message:
 * @session: a #SoupSession
 * @feature_type: the #GType of the feature to get
 * @msg: a #SoupMessage
 *
 * Gets the first feature in @session of type @feature_type, provided
 * that it is not disabled for @msg. As with
 * soup_session_get_feature(), this should only be used for features
 * where @feature_type is only expected to match a single feature. In
 * particular, if there are two matching features, and the first is
 * disabled on @msg, and the second is not, then this will return
 * %NULL, not the second feature.
 *
 * Return value: (transfer none): a #SoupSessionFeature, or %NULL. The
 * feature is owned by @session.
 *
 * Since: 2.28
 **/
SoupSessionFeature *
soup_session_get_feature_for_message (SoupSession *session, GType feature_type,
				      SoupMessage *msg)
{
	SoupSessionFeature *feature;

	feature = soup_session_get_feature (session, feature_type);
	if (feature && soup_message_disables_feature (msg, feature))
		return NULL;
	return feature;
}

static void
soup_session_class_init (SoupSessionClass *session_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (session_class);

	g_type_class_add_private (session_class, sizeof (SoupSessionPrivate));

	/* virtual method definition */
	session_class->requeue_message = soup_session_real_requeue_message;
	session_class->cancel_message = soup_session_real_cancel_message;
	session_class->auth_required = soup_session_real_auth_required;
	session_class->flush_queue = soup_session_real_flush_queue;

	/* virtual method override */
	object_class->dispose = soup_session_dispose;
	object_class->finalize = soup_session_finalize;
	object_class->set_property = soup_session_set_property;
	object_class->get_property = soup_session_get_property;

	/* signals */

	/**
	 * SoupSession::request-queued:
	 * @session: the session
	 * @msg: the request that was queued
	 *
	 * Emitted when a request is queued on @session. (Note that
	 * "queued" doesn't just mean soup_session_queue_message();
	 * soup_session_send_message() implicitly queues the message
	 * as well.)
	 *
	 * When sending a request, first #SoupSession::request_queued
	 * is emitted, indicating that the session has become aware of
	 * the request.
	 *
	 * Once a connection is available to send the request on, the
	 * session emits #SoupSession::request_started. Then, various
	 * #SoupMessage signals are emitted as the message is
	 * processed. If the message is requeued, it will emit
	 * #SoupMessage::restarted, which will then be followed by
	 * another #SoupSession::request_started and another set of
	 * #SoupMessage signals when the message is re-sent.
	 *
	 * Eventually, the message will emit #SoupMessage::finished.
	 * Normally, this signals the completion of message
	 * processing. However, it is possible that the application
	 * will requeue the message from the "finished" handler (or
	 * equivalently, from the soup_session_queue_message()
	 * callback). In that case, the process will loop back to
	 * #SoupSession::request_started.
	 *
	 * Eventually, a message will reach "finished" and not be
	 * requeued. At that point, the session will emit
	 * #SoupSession::request_unqueued to indicate that it is done
	 * with the message.
	 *
	 * To sum up: #SoupSession::request_queued and
	 * #SoupSession::request_unqueued are guaranteed to be emitted
	 * exactly once, but #SoupSession::request_started and
	 * #SoupMessage::finished (and all of the other #SoupMessage
	 * signals) may be invoked multiple times for a given message.
	 *
	 * Since: 2.24
	 **/
	signals[REQUEST_QUEUED] =
		g_signal_new ("request-queued",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      0, /* FIXME? */
			      NULL, NULL,
			      _soup_marshal_NONE__OBJECT,
			      G_TYPE_NONE, 1,
			      SOUP_TYPE_MESSAGE);

	/**
	 * SoupSession::request-started:
	 * @session: the session
	 * @msg: the request being sent
	 * @socket: the socket the request is being sent on
	 *
	 * Emitted just before a request is sent. See
	 * #SoupSession::request_queued for a detailed description of
	 * the message lifecycle within a session.
	 **/
	signals[REQUEST_STARTED] =
		g_signal_new ("request-started",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (SoupSessionClass, request_started),
			      NULL, NULL,
			      _soup_marshal_NONE__OBJECT_OBJECT,
			      G_TYPE_NONE, 2,
			      SOUP_TYPE_MESSAGE,
			      SOUP_TYPE_SOCKET);

	/**
	 * SoupSession::request-unqueued:
	 * @session: the session
	 * @msg: the request that was unqueued
	 *
	 * Emitted when a request is removed from @session's queue,
	 * indicating that @session is done with it. See
	 * #SoupSession::request_queued for a detailed description of the
	 * message lifecycle within a session.
	 *
	 * Since: 2.24
	 **/
	signals[REQUEST_UNQUEUED] =
		g_signal_new ("request-unqueued",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      0, /* FIXME? */
			      NULL, NULL,
			      _soup_marshal_NONE__OBJECT,
			      G_TYPE_NONE, 1,
			      SOUP_TYPE_MESSAGE);

	/**
	 * SoupSession::authenticate:
	 * @session: the session
	 * @msg: the #SoupMessage being sent
	 * @auth: the #SoupAuth to authenticate
	 * @retrying: %TRUE if this is the second (or later) attempt
	 *
	 * Emitted when the session requires authentication. If
	 * credentials are available call soup_auth_authenticate() on
	 * @auth. If these credentials fail, the signal will be
	 * emitted again, with @retrying set to %TRUE, which will
	 * continue until you return without calling
	 * soup_auth_authenticate() on @auth.
	 *
	 * Note that this may be emitted before @msg's body has been
	 * fully read.
	 *
	 * If you call soup_session_pause_message() on @msg before
	 * returning, then you can authenticate @auth asynchronously
	 * (as long as you g_object_ref() it to make sure it doesn't
	 * get destroyed), and then unpause @msg when you are ready
	 * for it to continue.
	 **/
	signals[AUTHENTICATE] =
		g_signal_new ("authenticate",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (SoupSessionClass, authenticate),
			      NULL, NULL,
			      _soup_marshal_NONE__OBJECT_OBJECT_BOOLEAN,
			      G_TYPE_NONE, 3,
			      SOUP_TYPE_MESSAGE,
			      SOUP_TYPE_AUTH,
			      G_TYPE_BOOLEAN);

	/**
	 * SoupSession::connection-created:
	 * @session: the #SoupSession
	 * @connection: the connection
	 *
	 * Emitted when a new connection is created. This is an
	 * internal signal intended only to be used for debugging
	 * purposes, and may go away in the future.
	 *
	 * Since: 2.30
	 */
	signals[CONNECTION_CREATED] =
		g_signal_new ("connection-created",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      0,
			      NULL, NULL,
			      _soup_marshal_NONE__OBJECT,
			      G_TYPE_NONE, 1,
			      /* SoupConnection is private, so we can't use
			       * SOUP_TYPE_CONNECTION here.
			       */
			      G_TYPE_OBJECT);

	/**
	 * SoupSession::tunneling:
	 * @session: the #SoupSession
	 * @connection: the connection
	 *
	 * Emitted when an SSL tunnel is being created on a proxy
	 * connection. This is an internal signal intended only to be
	 * used for debugging purposes, and may go away in the future.
	 *
	 * Since: 2.30
	 */
	signals[TUNNELING] =
		g_signal_new ("tunneling",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      0,
			      NULL, NULL,
			      _soup_marshal_NONE__OBJECT,
			      G_TYPE_NONE, 1,
			      /* SoupConnection is private, so we can't use
			       * SOUP_TYPE_CONNECTION here.
			       */
			      G_TYPE_OBJECT);


	/* properties */
	/**
	 * SOUP_SESSION_PROXY_URI:
	 *
	 * Alias for the #SoupSession:proxy-uri property. (The HTTP
	 * proxy to use for this session.)
	 **/
	g_object_class_install_property (
		object_class, PROP_PROXY_URI,
		g_param_spec_boxed (SOUP_SESSION_PROXY_URI,
				    "Proxy URI",
				    "The HTTP Proxy to use for this session",
				    SOUP_TYPE_URI,
				    G_PARAM_READWRITE));
	/**
	 * SOUP_SESSION_MAX_CONNS:
	 *
	 * Alias for the #SoupSession:max-conns property. (The maximum
	 * number of connections that the session can open at once.)
	 **/
	g_object_class_install_property (
		object_class, PROP_MAX_CONNS,
		g_param_spec_int (SOUP_SESSION_MAX_CONNS,
				  "Max Connection Count",
				  "The maximum number of connections that the session can open at once",
				  1,
				  G_MAXINT,
				  SOUP_SESSION_MAX_CONNS_DEFAULT,
				  G_PARAM_READWRITE));
	/**
	 * SOUP_SESSION_MAX_CONNS_PER_HOST:
	 *
	 * Alias for the #SoupSession:max-conns-per-host property.
	 * (The maximum number of connections that the session can
	 * open at once to a given host.)
	 **/
	g_object_class_install_property (
		object_class, PROP_MAX_CONNS_PER_HOST,
		g_param_spec_int (SOUP_SESSION_MAX_CONNS_PER_HOST,
				  "Max Per-Host Connection Count",
				  "The maximum number of connections that the session can open at once to a given host",
				  1,
				  G_MAXINT,
				  SOUP_SESSION_MAX_CONNS_PER_HOST_DEFAULT,
				  G_PARAM_READWRITE));
	/**
	 * SoupSession:idle-timeout:
	 *
	 * Connection lifetime (in seconds) when idle. Any connection
	 * left idle longer than this will be closed.
	 *
	 * Although you can change this property at any time, it will
	 * only affect newly-created connections, not currently-open
	 * ones. You can call soup_session_abort() after setting this
	 * if you want to ensure that all future connections will have
	 * this timeout value.
	 *
	 * Since: 2.24
	 **/
	/**
	 * SOUP_SESSION_IDLE_TIMEOUT:
	 *
	 * Alias for the #SoupSession:idle-timeout property. (The idle
	 * connection lifetime.)
	 *
	 * Since: 2.24
	 **/
	g_object_class_install_property (
		object_class, PROP_IDLE_TIMEOUT,
		g_param_spec_uint (SOUP_SESSION_IDLE_TIMEOUT,
				   "Idle Timeout",
				   "Connection lifetime when idle",
				   0, G_MAXUINT, 0,
				   G_PARAM_READWRITE));
	/**
	 * SoupSession:use-ntlm:
	 *
	 * Whether or not to use NTLM authentication.
	 *
	 * Deprecated: use soup_session_add_feature_by_type() with
	 * #SOUP_TYPE_AUTH_NTLM.
	 **/
	/**
	 * SOUP_SESSION_USE_NTLM:
	 *
	 * Alias for the #SoupSession:use-ntlm property. (Whether or
	 * not to use NTLM authentication.)
	 **/
	g_object_class_install_property (
		object_class, PROP_USE_NTLM,
		g_param_spec_boolean (SOUP_SESSION_USE_NTLM,
				      "Use NTLM",
				      "Whether or not to use NTLM authentication",
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_DEPRECATED));
	/**
	 * SoupSession:ssl-ca-file:
	 *
	 * File containing SSL CA certificates.
	 *
	 * Deprecated: use #SoupSession:ssl-use-system-ca-file or
	 * #SoupSession:tls-database instead
	 **/
	/**
	 * SOUP_SESSION_SSL_CA_FILE:
	 *
	 * Alias for the #SoupSession:ssl-ca-file property. (File
	 * containing SSL CA certificates.).
	 *
	 * Deprecated: use #SoupSession:ssl-use-system-ca-file or
	 * #SoupSession:tls-database instead
	 **/
	g_object_class_install_property (
		object_class, PROP_SSL_CA_FILE,
		g_param_spec_string (SOUP_SESSION_SSL_CA_FILE,
				     "SSL CA file",
				     "File containing SSL CA certificates",
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_DEPRECATED));
	/**
	 * SOUP_SESSION_USE_SYSTEM_CA_FILE:
	 *
	 * Alias for the #SoupSession:ssl-use-system-ca-file property,
	 * qv.
	 *
	 * Since: 2.38
	 **/
	/**
	 * SoupSession:ssl-use-system-ca-file:
	 *
	 * Setting this to %TRUE is equivalent to setting
	 * #SoupSession:tls-database to the default system CA database.
	 * (and likewise, setting #SoupSession:tls-database to the
	 * default database by hand will cause this property to
	 * become %TRUE).
	 *
	 * Setting this to %FALSE (when it was previously %TRUE) will
	 * clear the #SoupSession:tls-database field.
	 *
	 * See #SoupSession:ssl-strict for more information on how
	 * https certificate validation is handled.
	 *
	 * Since: 2.38
	 **/
	g_object_class_install_property (
		object_class, PROP_SSL_USE_SYSTEM_CA_FILE,
		g_param_spec_boolean (SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE,
				      "Use system CA file",
				      "Use the system certificate database",
				      FALSE,
				      G_PARAM_READWRITE));
	/**
	 * SOUP_SESSION_TLS_DATABASE:
	 *
	 * Alias for the #SoupSession:tls-database property, qv.
	 *
	 * Since: 2.38
	 **/
	/**
	 * SoupSession:tls-database:
	 *
	 * Sets the #GTlsDatabase to use for validating SSL/TLS
	 * certificates.
	 *
	 * Note that setting the #SoupSession:ssl-ca-file or
	 * #SoupSession:ssl-use-system-ca-file property will cause
	 * this property to be set to a #GTlsDatabase corresponding to
	 * the indicated file or system default.
	 *
	 * See #SoupSession:ssl-strict for more information on how
	 * https certificate validation is handled.
	 *
	 * Since: 2.38
	 **/
	g_object_class_install_property (
		object_class, PROP_TLS_DATABASE,
		g_param_spec_object (SOUP_SESSION_TLS_DATABASE,
				     "TLS Database",
				     "TLS database to use",
				     G_TYPE_TLS_DATABASE,
				     G_PARAM_READWRITE));
	/**
	 * SOUP_SESSION_SSL_STRICT:
	 *
	 * Alias for the #SoupSession:ssl-strict property, qv.
	 *
	 * Since: 2.30
	 **/
	/**
	 * SoupSession:ssl-strict:
	 *
	 * Normally, if #SoupSession:tlsdb is set (including if it was
	 * set via #SoupSession:ssl-use-system-ca-file or
	 * #SoupSession:ssl-ca-file), then libsoup will reject any
	 * certificate that is invalid (ie, expired) or that is not
	 * signed by one of the given CA certificates, and the
	 * #SoupMessage will fail with the status
	 * %SOUP_STATUS_SSL_FAILED.
	 *
	 * If you set #SoupSession:ssl-strict to %FALSE, then all
	 * certificates will be accepted, and you will need to call
	 * soup_message_get_https_status() to distinguish valid from
	 * invalid certificates. (This can be used, eg, if you want to
	 * accept invalid certificates after giving some sort of
	 * warning.)
	 *
	 * If the session has no CA file or TLS database, then all
	 * certificates are always accepted, and this property has no
	 * effect.
	 *
	 * Since: 2.30
	 */
	g_object_class_install_property (
		object_class, PROP_SSL_STRICT,
		g_param_spec_boolean (SOUP_SESSION_SSL_STRICT,
				      "Strictly validate SSL certificates",
				      "Whether certificate errors should be considered a connection error",
				      TRUE,
				      G_PARAM_READWRITE));
	/**
	 * SOUP_SESSION_ASYNC_CONTEXT:
	 *
	 * Alias for the #SoupSession:async-context property. (The
	 * session's #GMainContext.)
	 */
	g_object_class_install_property (
		object_class, PROP_ASYNC_CONTEXT,
		g_param_spec_pointer (SOUP_SESSION_ASYNC_CONTEXT,
				      "Async GMainContext",
				      "The GMainContext to dispatch async I/O in",
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * SOUP_SESSION_USE_THREAD_CONTEXT:
	 *
	 * Alias for the #SoupSession:use-thread-context property, qv.
	 *
	 * Since: 2.38
	 */
	/**
	 * SoupSession:use-thread-context:
	 *
	 * If set, asynchronous operations in this session will run in
	 * whatever the thread-default #GMainContext is at the time
	 * they are started, rather than always occurring in a context
	 * fixed at the session's construction time. "Bookkeeping"
	 * tasks (like expiring idle connections) will happen in the
	 * context that was thread-default at the time the session was
	 * created.
	 *
	 * Since: 2.38
	 */
	g_object_class_install_property (
		object_class, PROP_USE_THREAD_CONTEXT,
		g_param_spec_boolean (SOUP_SESSION_USE_THREAD_CONTEXT,
				      "Use thread-default GMainContext",
				      "Whether to use thread-default main contexts",
				      FALSE,
				      G_PARAM_READWRITE));
	/**
	 * SoupSession:timeout:
	 *
	 * The timeout (in seconds) for socket I/O operations
	 * (including connecting to a server, and waiting for a reply
	 * to an HTTP request).
	 *
	 * Although you can change this property at any time, it will
	 * only affect newly-created connections, not currently-open
	 * ones. You can call soup_session_abort() after setting this
	 * if you want to ensure that all future connections will have
	 * this timeout value.
	 */
	/**
	 * SOUP_SESSION_TIMEOUT:
	 *
	 * Alias for the #SoupSession:timeout property. (The timeout
	 * in seconds for socket I/O operations.)
	 **/
	g_object_class_install_property (
		object_class, PROP_TIMEOUT,
		g_param_spec_uint (SOUP_SESSION_TIMEOUT,
				   "Timeout value",
				   "Value in seconds to timeout a blocking I/O",
				   0, G_MAXUINT, 0,
				   G_PARAM_READWRITE));

	/**
	 * SoupSession:user-agent:
	 *
	 * If non-%NULL, the value to use for the "User-Agent" header
	 * on #SoupMessage<!-- -->s sent from this session.
	 *
	 * RFC 2616 says: "The User-Agent request-header field
	 * contains information about the user agent originating the
	 * request. This is for statistical purposes, the tracing of
	 * protocol violations, and automated recognition of user
	 * agents for the sake of tailoring responses to avoid
	 * particular user agent limitations. User agents SHOULD
	 * include this field with requests."
	 *
	 * The User-Agent header contains a list of one or more
	 * product tokens, separated by whitespace, with the most
	 * significant product token coming first. The tokens must be
	 * brief, ASCII, and mostly alphanumeric (although "-", "_",
	 * and "." are also allowed), and may optionally include a "/"
	 * followed by a version string. You may also put comments,
	 * enclosed in parentheses, between or after the tokens.
	 *
	 * If you set a #SoupSession:user_agent property that has trailing
	 * whitespace, #SoupSession will append its own product token
	 * (eg, "<literal>libsoup/2.3.2</literal>") to the end of the
	 * header for you.
	 **/
	/**
	 * SOUP_SESSION_USER_AGENT:
	 *
	 * Alias for the #SoupSession:user-agent property, qv.
	 **/
	g_object_class_install_property (
		object_class, PROP_USER_AGENT,
		g_param_spec_string (SOUP_SESSION_USER_AGENT,
				     "User-Agent string",
				     "User-Agent string",
				     NULL,
				     G_PARAM_READWRITE));

	/**
	 * SoupSession:accept-language:
	 *
	 * If non-%NULL, the value to use for the "Accept-Language" header
	 * on #SoupMessage<!-- -->s sent from this session.
	 *
	 * Setting this will disable
	 * #SoupSession:accept-language-auto.
	 *
	 * Since: 2.30
	 **/
	/**
	 * SOUP_SESSION_ACCEPT_LANGUAGE:
	 *
	 * Alias for the #SoupSession:accept-language property, qv.
	 *
	 * Since: 2.30
	 **/
	g_object_class_install_property (
		object_class, PROP_ACCEPT_LANGUAGE,
		g_param_spec_string (SOUP_SESSION_ACCEPT_LANGUAGE,
				     "Accept-Language string",
				     "Accept-Language string",
				     NULL,
				     G_PARAM_READWRITE));

	/**
	 * SoupSession:accept-language-auto:
	 *
	 * If %TRUE, #SoupSession will automatically set the string
	 * for the "Accept-Language" header on every #SoupMessage
	 * sent, based on the return value of g_get_language_names().
	 *
	 * Setting this will override any previous value of
	 * #SoupSession:accept-language.
	 *
	 * Since: 2.30
	 **/
	/**
	 * SOUP_SESSION_ACCEPT_LANGUAGE_AUTO:
	 *
	 * Alias for the #SoupSession:accept-language-auto property, qv.
	 *
	 * Since: 2.30
	 **/
	g_object_class_install_property (
		object_class, PROP_ACCEPT_LANGUAGE_AUTO,
		g_param_spec_boolean (SOUP_SESSION_ACCEPT_LANGUAGE_AUTO,
				      "Accept-Language automatic mode",
				      "Accept-Language automatic mode",
				      FALSE,
				      G_PARAM_READWRITE));

	/**
	 * SoupSession:add-feature: (skip)
	 *
	 * Add a feature object to the session. (Shortcut for calling
	 * soup_session_add_feature().)
	 *
	 * Since: 2.24
	 **/
	/**
	 * SOUP_SESSION_ADD_FEATURE: (skip)
	 *
	 * Alias for the #SoupSession:add-feature property. (Shortcut
	 * for calling soup_session_add_feature().
	 *
	 * Since: 2.24
	 **/
	g_object_class_install_property (
		object_class, PROP_ADD_FEATURE,
		g_param_spec_object (SOUP_SESSION_ADD_FEATURE,
				     "Add Feature",
				     "Add a feature object to the session",
				     SOUP_TYPE_SESSION_FEATURE,
				     G_PARAM_READWRITE));
	/**
	 * SoupSession:add-feature-by-type: (skip)
	 *
	 * Add a feature object of the given type to the session.
	 * (Shortcut for calling soup_session_add_feature_by_type().)
	 *
	 * Since: 2.24
	 **/
	/**
	 * SOUP_SESSION_ADD_FEATURE_BY_TYPE: (skip)
	 *
	 * Alias for the #SoupSession:add-feature-by-type property.
	 * (Shortcut for calling soup_session_add_feature_by_type().
	 *
	 * Since: 2.24
	 **/
	g_object_class_install_property (
		object_class, PROP_ADD_FEATURE_BY_TYPE,
		g_param_spec_gtype (SOUP_SESSION_ADD_FEATURE_BY_TYPE,
				    "Add Feature By Type",
				    "Add a feature object of the given type to the session",
				    SOUP_TYPE_SESSION_FEATURE,
				    G_PARAM_READWRITE));
	/**
	 * SoupSession:remove-feature-by-type: (skip)
	 *
	 * Remove feature objects from the session. (Shortcut for
	 * calling soup_session_remove_feature_by_type().)
	 *
	 * Since: 2.24
	 **/
	/**
	 * SOUP_SESSION_REMOVE_FEATURE_BY_TYPE: (skip)
	 *
	 * Alias for the #SoupSession:remove-feature-by-type
	 * property. (Shortcut for calling
	 * soup_session_remove_feature_by_type().
	 *
	 * Since: 2.24
	 **/
	g_object_class_install_property (
		object_class, PROP_REMOVE_FEATURE_BY_TYPE,
		g_param_spec_gtype (SOUP_SESSION_REMOVE_FEATURE_BY_TYPE,
				    "Remove Feature By Type",
				    "Remove features of the given type from the session",
				    SOUP_TYPE_SESSION_FEATURE,
				    G_PARAM_READWRITE));
	/**
	 * SoupSession:http-aliases:
	 *
	 * A %NULL-terminated array of URI schemes that should be
	 * considered to be aliases for "http". Eg, if this included
	 * <literal>"dav"</literal>, than a URI of
	 * <literal>dav://example.com/path</literal> would be treated
	 * identically to <literal>http://example.com/path</literal>.
	 * If the value is %NULL, then only "http" is recognized as
	 * meaning "http".
	 *
	 * For backward-compatibility reasons, the default value for
	 * this property is an array containing the single element
	 * <literal>"*"</literal>, a special value which means that
	 * any scheme except "https" is considered to be an alias for
	 * "http".
	 *
	 * See also #SoupSession:https-aliases.
	 *
	 * Since: 2.38
	 */
	/**
	 * SOUP_SESSION_HTTP_ALIASES:
	 *
	 * Alias for the #SoupSession:http-aliases property. (URI
	 * schemes that will be considered aliases for "http".)
	 *
	 * Since: 2.38
	 */
	g_object_class_install_property (
		object_class, PROP_HTTP_ALIASES,
		g_param_spec_boxed (SOUP_SESSION_HTTP_ALIASES,
				    "http aliases",
				    "URI schemes that are considered aliases for 'http'",
				    G_TYPE_STRV,
				    G_PARAM_READWRITE));
	/**
	 * SoupSession:https-aliases:
	 *
	 * A comma-delimited list of URI schemes that should be
	 * considered to be aliases for "https". See
	 * #SoupSession:http-aliases for more information.
	 *
	 * The default value is %NULL, meaning that no URI schemes
	 * are considered aliases for "https".
	 *
	 * Since: 2.38
	 */
	/**
	 * SOUP_SESSION_HTTPS_ALIASES:
	 *
	 * Alias for the #SoupSession:https-aliases property. (URI
	 * schemes that will be considered aliases for "https".)
	 *
	 * Since: 2.38
	 **/
	g_object_class_install_property (
		object_class, PROP_HTTPS_ALIASES,
		g_param_spec_boxed (SOUP_SESSION_HTTPS_ALIASES,
				    "https aliases",
				    "URI schemes that are considered aliases for 'https'",
				    G_TYPE_STRV,
				    G_PARAM_READWRITE));
}
