/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-auth.c: HTTP Authentication framework
 *
 * Copyright (C) 2001-2003, Ximian, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define LIBSOUP_I_HAVE_READ_BUG_594377_AND_KNOW_SOUP_PASSWORD_MANAGER_MIGHT_GO_AWAY

#include <string.h>

#include "soup-auth.h"
#include "soup-headers.h"
#include "soup-marshal.h"
#include "soup-uri.h"

/**
 * SECTION:soup-auth
 * @short_description: HTTP client-side authentication support
 * @see_also: #SoupSession
 *
 * #SoupAuth objects store the authentication data associated with a
 * given bit of web space. They are created automatically by
 * #SoupSession.
 **/

/**
 * SoupAuth:
 *
 * The abstract base class for handling authentication. Specific HTTP
 * Authentication mechanisms are implemented by its subclasses, but
 * applications never need to be aware of the specific subclasses
 * being used.
 **/

typedef struct {
	gboolean proxy;
	char *host;

	GHashTable *saved_passwords;
} SoupAuthPrivate;
#define SOUP_AUTH_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), SOUP_TYPE_AUTH, SoupAuthPrivate))

G_DEFINE_ABSTRACT_TYPE (SoupAuth, soup_auth, G_TYPE_OBJECT)

enum {
	SAVE_PASSWORD,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

enum {
	PROP_0,

	PROP_SCHEME_NAME,
	PROP_REALM,
	PROP_HOST,
	PROP_IS_FOR_PROXY,
	PROP_IS_AUTHENTICATED,

	LAST_PROP
};

static void
soup_auth_init (SoupAuth *auth)
{
}

static void
soup_auth_finalize (GObject *object)
{
	SoupAuth *auth = SOUP_AUTH (object);
	SoupAuthPrivate *priv = SOUP_AUTH_GET_PRIVATE (auth);

	g_free (auth->realm);
	g_free (priv->host);
	if (priv->saved_passwords)
		g_hash_table_destroy (priv->saved_passwords);

	G_OBJECT_CLASS (soup_auth_parent_class)->finalize (object);
}

static void
soup_auth_set_property (GObject *object, guint prop_id,
			const GValue *value, GParamSpec *pspec)
{
	SoupAuth *auth = SOUP_AUTH (object);
	SoupAuthPrivate *priv = SOUP_AUTH_GET_PRIVATE (object);

	switch (prop_id) {
	case PROP_REALM:
		auth->realm = g_value_dup_string (value);
		break;
	case PROP_HOST:
		priv->host = g_value_dup_string (value);
		break;
	case PROP_IS_FOR_PROXY:
		priv->proxy = g_value_get_boolean (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
soup_auth_get_property (GObject *object, guint prop_id,
			GValue *value, GParamSpec *pspec)
{
	SoupAuth *auth = SOUP_AUTH (object);
	SoupAuthPrivate *priv = SOUP_AUTH_GET_PRIVATE (object);

	switch (prop_id) {
	case PROP_SCHEME_NAME:
		g_value_set_string (value, soup_auth_get_scheme_name (auth));
		break;
	case PROP_REALM:
		g_value_set_string (value, soup_auth_get_realm (auth));
		break;
	case PROP_HOST:
		g_value_set_string (value, soup_auth_get_host (auth));
		break;
	case PROP_IS_FOR_PROXY:
		g_value_set_boolean (value, priv->proxy);
		break;
	case PROP_IS_AUTHENTICATED:
		g_value_set_boolean (value, soup_auth_is_authenticated (auth));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
soup_auth_class_init (SoupAuthClass *auth_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (auth_class);

	g_type_class_add_private (auth_class, sizeof (SoupAuthPrivate));

	object_class->finalize     = soup_auth_finalize;
	object_class->set_property = soup_auth_set_property;
	object_class->get_property = soup_auth_get_property;

	/**
	 * SoupAuth::save-password:
	 * @auth: the auth
	 * @username: the username to save
	 * @password: the password to save
	 *
	 * Emitted to request that the @username/@password pair be
	 * saved. If the session supports password-saving, it will
	 * connect to this signal before emitting
	 * #SoupSession::authenticate, so that it record the password
	 * if requested by the caller.
	 *
	 * Since: 2.28
	 **/
	signals[SAVE_PASSWORD] =
		g_signal_new ("save-password",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      0, NULL, NULL,
			      _soup_marshal_NONE__STRING_STRING,
			      G_TYPE_NONE, 2,
			      G_TYPE_STRING,
			      G_TYPE_STRING);

	/* properties */
	/**
	 * SOUP_AUTH_SCHEME_NAME:
	 *
	 * An alias for the #SoupAuth:scheme-name property. (The
	 * authentication scheme name.)
	 **/
	g_object_class_install_property (
		object_class, PROP_SCHEME_NAME,
		g_param_spec_string (SOUP_AUTH_SCHEME_NAME,
				     "Scheme name",
				     "Authentication scheme name",
				     NULL,
				     G_PARAM_READABLE));
	/**
	 * SOUP_AUTH_REALM:
	 *
	 * An alias for the #SoupAuth:realm property. (The
	 * authentication realm.)
	 **/
	g_object_class_install_property (
		object_class, PROP_REALM,
		g_param_spec_string (SOUP_AUTH_REALM,
				     "Realm",
				     "Authentication realm",
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * SOUP_AUTH_HOST:
	 *
	 * An alias for the #SoupAuth:host property. (The
	 * host being authenticated to.)
	 **/
	g_object_class_install_property (
		object_class, PROP_HOST,
		g_param_spec_string (SOUP_AUTH_HOST,
				     "Host",
				     "Authentication host",
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * SOUP_AUTH_IS_FOR_PROXY:
	 *
	 * An alias for the #SoupAuth:is-for-proxy property. (Whether
	 * or not the auth is for a proxy server.)
	 **/
	g_object_class_install_property (
		object_class, PROP_IS_FOR_PROXY,
		g_param_spec_boolean (SOUP_AUTH_IS_FOR_PROXY,
				      "For Proxy",
				      "Whether or not the auth is for a proxy server",
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * SOUP_AUTH_IS_AUTHENTICATED:
	 *
	 * An alias for the #SoupAuth:is-authenticated property.
	 * (Whether or not the auth has been authenticated.)
	 **/
	g_object_class_install_property (
		object_class, PROP_IS_AUTHENTICATED,
		g_param_spec_boolean (SOUP_AUTH_IS_AUTHENTICATED,
				      "Authenticated",
				      "Whether or not the auth is authenticated",
				      FALSE,
				      G_PARAM_READABLE));
}

/**
 * soup_auth_new:
 * @type: the type of auth to create (a subtype of #SoupAuth)
 * @msg: the #SoupMessage the auth is being created for
 * @auth_header: the WWW-Authenticate/Proxy-Authenticate header
 *
 * Creates a new #SoupAuth of type @type with the information from
 * @msg and @auth_header.
 *
 * This is called by #SoupSession; you will normally not create auths
 * yourself.
 *
 * Return value: the new #SoupAuth, or %NULL if it could not be
 * created
 **/
SoupAuth *
soup_auth_new (GType type, SoupMessage *msg, const char *auth_header)
{
	SoupAuth *auth;
	GHashTable *params;
	const char *scheme, *realm;

	g_return_val_if_fail (g_type_is_a (type, SOUP_TYPE_AUTH), NULL);
	g_return_val_if_fail (SOUP_IS_MESSAGE (msg), NULL);
	g_return_val_if_fail (auth_header != NULL, NULL);

	auth = g_object_new (type,
			     SOUP_AUTH_IS_FOR_PROXY, (msg->status_code == SOUP_STATUS_PROXY_UNAUTHORIZED),
			     SOUP_AUTH_HOST, soup_message_get_uri (msg)->host,
			     NULL);

	scheme = soup_auth_get_scheme_name (auth);
	if (g_ascii_strncasecmp (auth_header, scheme, strlen (scheme)) != 0) {
		g_object_unref (auth);
		return NULL;
	}

	params = soup_header_parse_param_list (auth_header + strlen (scheme));
	if (!params) {
		g_object_unref (auth);
		return NULL;
	}

	realm = g_hash_table_lookup (params, "realm");
	if (!realm) {
		soup_header_free_param_list (params);
		g_object_unref (auth);
		return NULL;
	}

	auth->realm = g_strdup (realm);

	if (!SOUP_AUTH_GET_CLASS (auth)->update (auth, msg, params)) {
		g_object_unref (auth);
		auth = NULL;
	}
	soup_header_free_param_list (params);
	return auth;
}

/**
 * soup_auth_update:
 * @auth: a #SoupAuth
 * @msg: the #SoupMessage @auth is being updated for
 * @auth_header: the WWW-Authenticate/Proxy-Authenticate header
 *
 * Updates @auth with the information from @msg and @auth_header,
 * possibly un-authenticating it. As with soup_auth_new(), this is
 * normally only used by #SoupSession.
 *
 * Return value: %TRUE if @auth is still a valid (but potentially
 * unauthenticated) #SoupAuth. %FALSE if something about @auth_params
 * could not be parsed or incorporated into @auth at all.
 **/
gboolean
soup_auth_update (SoupAuth *auth, SoupMessage *msg, const char *auth_header)
{
	GHashTable *params;
	const char *scheme, *realm;
	gboolean was_authenticated, success;

	g_return_val_if_fail (SOUP_IS_AUTH (auth), FALSE);
	g_return_val_if_fail (SOUP_IS_MESSAGE (msg), FALSE);
	g_return_val_if_fail (auth_header != NULL, FALSE);

	scheme = soup_auth_get_scheme_name (auth);
	if (g_ascii_strncasecmp (auth_header, scheme, strlen (scheme)) != 0)
		return FALSE;

	params = soup_header_parse_param_list (auth_header + strlen (scheme));
	if (!params)
		return FALSE;

	realm = g_hash_table_lookup (params, "realm");
	if (!realm || strcmp (realm, auth->realm) != 0) {
		soup_header_free_param_list (params);
		return FALSE;
	}

	was_authenticated = soup_auth_is_authenticated (auth);
	success = SOUP_AUTH_GET_CLASS (auth)->update (auth, msg, params);
	if (was_authenticated != soup_auth_is_authenticated (auth))
		g_object_notify (G_OBJECT (auth), SOUP_AUTH_IS_AUTHENTICATED);
	soup_header_free_param_list (params);
	return success;
}

/**
 * soup_auth_authenticate:
 * @auth: a #SoupAuth
 * @username: the username provided by the user or client
 * @password: the password provided by the user or client
 *
 * Call this on an auth to authenticate it; normally this will cause
 * the auth's message to be requeued with the new authentication info.
 *
 * This does not cause the password to be saved to persistent storage;
 * see soup_auth_save_password() for that.
 **/
void
soup_auth_authenticate (SoupAuth *auth, const char *username, const char *password)
{
	gboolean was_authenticated;

	g_return_if_fail (SOUP_IS_AUTH (auth));
	g_return_if_fail (username != NULL);
	g_return_if_fail (password != NULL);

	was_authenticated = soup_auth_is_authenticated (auth);
	SOUP_AUTH_GET_CLASS (auth)->authenticate (auth, username, password);
	if (was_authenticated != soup_auth_is_authenticated (auth))
		g_object_notify (G_OBJECT (auth), SOUP_AUTH_IS_AUTHENTICATED);
}

/**
 * soup_auth_is_for_proxy:
 * @auth: a #SoupAuth
 *
 * Tests whether or not @auth is associated with a proxy server rather
 * than an "origin" server.
 *
 * Return value: %TRUE or %FALSE
 **/
gboolean
soup_auth_is_for_proxy (SoupAuth *auth)
{
	g_return_val_if_fail (SOUP_IS_AUTH (auth), FALSE);

	return SOUP_AUTH_GET_PRIVATE (auth)->proxy;
}

/**
 * soup_auth_get_scheme_name:
 * @auth: a #SoupAuth
 *
 * Returns @auth's scheme name. (Eg, "Basic", "Digest", or "NTLM")
 *
 * Return value: the scheme name
 **/
const char *
soup_auth_get_scheme_name (SoupAuth *auth)
{
	g_return_val_if_fail (SOUP_IS_AUTH (auth), NULL);

	return SOUP_AUTH_GET_CLASS (auth)->scheme_name;
}

/**
 * soup_auth_get_host:
 * @auth: a #SoupAuth
 *
 * Returns the host that @auth is associated with.
 *
 * Return value: the hostname
 **/
const char *
soup_auth_get_host (SoupAuth *auth)
{
	g_return_val_if_fail (SOUP_IS_AUTH (auth), NULL);

	return SOUP_AUTH_GET_PRIVATE (auth)->host;
}


/**
 * soup_auth_get_realm:
 * @auth: a #SoupAuth
 *
 * Returns @auth's realm. This is an identifier that distinguishes
 * separate authentication spaces on a given server, and may be some
 * string that is meaningful to the user. (Although it is probably not
 * localized.)
 *
 * Return value: the realm name
 **/
const char *
soup_auth_get_realm (SoupAuth *auth)
{
	g_return_val_if_fail (SOUP_IS_AUTH (auth), NULL);

	return auth->realm;
}

/**
 * soup_auth_get_info:
 * @auth: a #SoupAuth
 *
 * Gets an opaque identifier for @auth, for use as a hash key or the
 * like. #SoupAuth objects from the same server with the same
 * identifier refer to the same authentication domain (eg, the URLs
 * associated with them take the same usernames and passwords).
 *
 * Return value: the identifier
 **/
char *
soup_auth_get_info (SoupAuth *auth)
{
	g_return_val_if_fail (SOUP_IS_AUTH (auth), NULL);

	return g_strdup_printf ("%s:%s",
				SOUP_AUTH_GET_CLASS (auth)->scheme_name,
				auth->realm);
}

/**
 * soup_auth_is_authenticated:
 * @auth: a #SoupAuth
 *
 * Tests if @auth has been given a username and password
 *
 * Return value: %TRUE if @auth has been given a username and password
 **/
gboolean
soup_auth_is_authenticated (SoupAuth *auth)
{
	g_return_val_if_fail (SOUP_IS_AUTH (auth), TRUE);

	return SOUP_AUTH_GET_CLASS (auth)->is_authenticated (auth);
}

/**
 * soup_auth_get_authorization:
 * @auth: a #SoupAuth
 * @msg: the #SoupMessage to be authorized
 *
 * Generates an appropriate "Authorization" header for @msg. (The
 * session will only call this if soup_auth_is_authenticated()
 * returned %TRUE.)
 *
 * Return value: the "Authorization" header, which must be freed.
 **/
char *
soup_auth_get_authorization (SoupAuth *auth, SoupMessage *msg)
{
	g_return_val_if_fail (SOUP_IS_AUTH (auth), NULL);
	g_return_val_if_fail (msg != NULL, NULL);

	return SOUP_AUTH_GET_CLASS (auth)->get_authorization (auth, msg);
}

/**
 * soup_auth_get_protection_space:
 * @auth: a #SoupAuth
 * @source_uri: the URI of the request that @auth was generated in
 * response to.
 *
 * Returns a list of paths on the server which @auth extends over.
 * (All subdirectories of these paths are also assumed to be part
 * of @auth's protection space, unless otherwise discovered not to
 * be.)
 *
 * Return value: (element-type utf8) (transfer full): the list of
 * paths, which can be freed with soup_auth_free_protection_space().
 **/
GSList *
soup_auth_get_protection_space (SoupAuth *auth, SoupURI *source_uri)
{
	g_return_val_if_fail (SOUP_IS_AUTH (auth), NULL);
	g_return_val_if_fail (source_uri != NULL, NULL);

	return SOUP_AUTH_GET_CLASS (auth)->get_protection_space (auth, source_uri);
}

/**
 * soup_auth_free_protection_space: (skip)
 * @auth: a #SoupAuth
 * @space: the return value from soup_auth_get_protection_space()
 *
 * Frees @space.
 **/
void
soup_auth_free_protection_space (SoupAuth *auth, GSList *space)
{
	g_slist_free_full (space, g_free);
}

/**
 * soup_auth_get_saved_users:
 * @auth: a #SoupAuth
 *
 * Gets a list of usernames for which a saved password is available.
 * (If the session is not configured to save passwords, this will
 * always be %NULL.)
 *
 * Return value: (transfer container): the list of usernames. You must
 * free the list with g_slist_free(), but do not free or modify the
 * contents.
 *
 * Since: 2.28
 **/
GSList *
soup_auth_get_saved_users (SoupAuth *auth)
{
	SoupAuthPrivate *priv;
	GSList *users;

	g_return_val_if_fail (SOUP_IS_AUTH (auth), NULL);

	priv = SOUP_AUTH_GET_PRIVATE (auth);
	users = NULL;

	if (priv->saved_passwords) {
		GHashTableIter iter;
		gpointer key, value;

		g_hash_table_iter_init (&iter, priv->saved_passwords);
		while (g_hash_table_iter_next (&iter, &key, &value))
			users = g_slist_prepend (users, key);
	}
	return users;
}

/**
 * soup_auth_get_saved_password:
 * @auth: a #SoupAuth
 * @user: a username from the list returned from
 * soup_auth_get_saved_users().
 *
 * Given a username for which @auth has a saved password, this returns
 * that password. If @auth doesn't have a passwords saved for @user, it
 * returns %NULL.
 *
 * Return value: the saved password, or %NULL.
 *
 * Since: 2.28
 **/
const char *
soup_auth_get_saved_password (SoupAuth *auth, const char *user)
{
	SoupAuthPrivate *priv;

	g_return_val_if_fail (SOUP_IS_AUTH (auth), NULL);
	g_return_val_if_fail (user != NULL, NULL);

	priv = SOUP_AUTH_GET_PRIVATE (auth);
	if (!priv->saved_passwords)
		return NULL;
	return g_hash_table_lookup (priv->saved_passwords, user);
}

static void
free_password (gpointer password)
{
	memset (password, 0, strlen (password));
	g_free (password);
}

static inline void
init_saved_passwords (SoupAuthPrivate *priv)
{
	priv->saved_passwords = g_hash_table_new_full (
		g_str_hash, g_str_equal, g_free, free_password);
}

/**
 * soup_auth_has_saved_password:
 * @auth: a #SoupAuth
 * @username: a username
 * @password: a password
 *
 * Updates @auth to be aware of an already-saved username/password
 * combination. This method <emphasis>does not</emphasis> cause the
 * given @username and @password to be saved; use
 * soup_auth_save_password() for that. (soup_auth_has_saved_password()
 * is an internal method, which is used by the code that actually
 * saves and restores the passwords.)
 *
 * Since: 2.28
 **/
void
soup_auth_has_saved_password (SoupAuth *auth, const char *username,
			      const char *password)
{
	SoupAuthPrivate *priv;

	g_return_if_fail (SOUP_IS_AUTH (auth));
	g_return_if_fail (username != NULL);
	g_return_if_fail (password != NULL);

	priv = SOUP_AUTH_GET_PRIVATE (auth);

	if (!priv->saved_passwords)
		init_saved_passwords (priv);
	g_hash_table_insert (priv->saved_passwords,
			     g_strdup (username), g_strdup (password));
}

/**
 * soup_auth_save_password:
 * @auth: a #SoupAuth
 * @username: the username provided by the user or client
 * @password: the password provided by the user or client
 *
 * Requests that the username/password pair be saved to whatever form
 * of persistent password storage the session supports.
 *
 * Since: 2.28
 **/
void
soup_auth_save_password (SoupAuth *auth, const char *username,
			 const char *password)
{
	g_return_if_fail (SOUP_IS_AUTH (auth));
	g_return_if_fail (username != NULL);
	g_return_if_fail (password != NULL);

	g_signal_emit (auth, signals[SAVE_PASSWORD], 0,
		       username, password);
}
