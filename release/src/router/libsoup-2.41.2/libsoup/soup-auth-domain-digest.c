/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-auth-domain-digest.c: HTTP Digest Authentication (server-side)
 *
 * Copyright (C) 2007 Novell, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>

#include "soup-auth-domain-digest.h"
#include "soup.h"
#include "soup-auth-digest.h"
#include "soup-marshal.h"

/**
 * SECTION:soup-auth-domain-digest
 * @short_description: Server-side "Digest" authentication
 *
 * #SoupAuthDomainBasic handles the server side of HTTP "Digest"
 * authentication.
 **/

enum {
	PROP_0,

	PROP_AUTH_CALLBACK,
	PROP_AUTH_DATA,

	LAST_PROP
};

typedef struct {
	SoupAuthDomainDigestAuthCallback auth_callback;
	gpointer auth_data;
	GDestroyNotify auth_dnotify;

} SoupAuthDomainDigestPrivate;

#define SOUP_AUTH_DOMAIN_DIGEST_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), SOUP_TYPE_AUTH_DOMAIN_DIGEST, SoupAuthDomainDigestPrivate))

G_DEFINE_TYPE (SoupAuthDomainDigest, soup_auth_domain_digest, SOUP_TYPE_AUTH_DOMAIN)

static void
soup_auth_domain_digest_init (SoupAuthDomainDigest *digest)
{
}

static void
soup_auth_domain_digest_finalize (GObject *object)
{
	SoupAuthDomainDigestPrivate *priv =
		SOUP_AUTH_DOMAIN_DIGEST_GET_PRIVATE (object);

	if (priv->auth_dnotify)
		priv->auth_dnotify (priv->auth_data);

	G_OBJECT_CLASS (soup_auth_domain_digest_parent_class)->finalize (object);
}

static void
soup_auth_domain_digest_set_property (GObject *object, guint prop_id,
				      const GValue *value, GParamSpec *pspec)
{
	SoupAuthDomainDigestPrivate *priv =
		SOUP_AUTH_DOMAIN_DIGEST_GET_PRIVATE (object);

	switch (prop_id) {
	case PROP_AUTH_CALLBACK:
		priv->auth_callback = g_value_get_pointer (value);
		break;
	case PROP_AUTH_DATA:
		if (priv->auth_dnotify) {
			priv->auth_dnotify (priv->auth_data);
			priv->auth_dnotify = NULL;
		}
		priv->auth_data = g_value_get_pointer (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
soup_auth_domain_digest_get_property (GObject *object, guint prop_id,
				      GValue *value, GParamSpec *pspec)
{
	SoupAuthDomainDigestPrivate *priv =
		SOUP_AUTH_DOMAIN_DIGEST_GET_PRIVATE (object);

	switch (prop_id) {
	case PROP_AUTH_CALLBACK:
		g_value_set_pointer (value, priv->auth_callback);
		break;
	case PROP_AUTH_DATA:
		g_value_set_pointer (value, priv->auth_data);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

/**
 * soup_auth_domain_digest_new:
 * @optname1: name of first option, or %NULL
 * @...: option name/value pairs
 *
 * Creates a #SoupAuthDomainDigest. You must set the
 * %SOUP_AUTH_DOMAIN_REALM parameter, to indicate the realm name to be
 * returned with the authentication challenge to the client. Other
 * parameters are optional.
 *
 * Return value: the new #SoupAuthDomain
 **/
SoupAuthDomain *
soup_auth_domain_digest_new (const char *optname1, ...)
{
	SoupAuthDomain *domain;
	va_list ap;

	va_start (ap, optname1);
	domain = (SoupAuthDomain *)g_object_new_valist (SOUP_TYPE_AUTH_DOMAIN_DIGEST,
							optname1, ap);
	va_end (ap);

	g_return_val_if_fail (soup_auth_domain_get_realm (domain) != NULL, NULL);

	return domain;
}

/**
 * SoupAuthDomainDigestAuthCallback:
 * @domain: the domain
 * @msg: the message being authenticated
 * @username: the username provided by the client
 * @user_data: the data passed to soup_auth_domain_digest_set_auth_callback()
 *
 * Callback used by #SoupAuthDomainDigest for authentication purposes.
 * The application should look up @username in its password database,
 * and return the corresponding encoded password (see
 * soup_auth_domain_digest_encode_password()).
 *
 * Return value: the encoded password, or %NULL if @username is not a
 * valid user. @domain will free the password when it is done with it.
 **/

/**
 * soup_auth_domain_digest_set_auth_callback:
 * @domain: the domain
 * @callback: the callback
 * @user_data: data to pass to @auth_callback
 * @dnotify: destroy notifier to free @user_data when @domain
 * is destroyed
 *
 * Sets the callback that @domain will use to authenticate incoming
 * requests. For each request containing authorization, @domain will
 * invoke the callback, and then either accept or reject the request
 * based on @callback's return value.
 *
 * You can also set the auth callback by setting the
 * %SOUP_AUTH_DOMAIN_DIGEST_AUTH_CALLBACK and
 * %SOUP_AUTH_DOMAIN_DIGEST_AUTH_DATA properties, which can also be
 * used to set the callback at construct time.
 **/
void
soup_auth_domain_digest_set_auth_callback (SoupAuthDomain *domain,
					   SoupAuthDomainDigestAuthCallback callback,
					   gpointer        user_data,
					   GDestroyNotify  dnotify)
{
	SoupAuthDomainDigestPrivate *priv =
		SOUP_AUTH_DOMAIN_DIGEST_GET_PRIVATE (domain);

	if (priv->auth_dnotify)
		priv->auth_dnotify (priv->auth_data);

	priv->auth_callback = callback;
	priv->auth_data = user_data;
	priv->auth_dnotify = dnotify;

	g_object_notify (G_OBJECT (domain), SOUP_AUTH_DOMAIN_DIGEST_AUTH_CALLBACK);
	g_object_notify (G_OBJECT (domain), SOUP_AUTH_DOMAIN_DIGEST_AUTH_DATA);
}

static gboolean
check_hex_urp (SoupAuthDomain *domain, SoupMessage *msg,
	       GHashTable *params, const char *username,
	       const char *hex_urp)
{
	const char *uri, *qop, *realm, *msg_username;
	const char *nonce, *nc, *cnonce, *response;
	char hex_a1[33], computed_response[33];
	int nonce_count;
	SoupURI *dig_uri, *req_uri;

	msg_username = g_hash_table_lookup (params, "username");
	if (!msg_username || strcmp (msg_username, username) != 0)
		return FALSE;

	/* Check uri */
	uri = g_hash_table_lookup (params, "uri");
	if (!uri)
		return FALSE;

	req_uri = soup_message_get_uri (msg);
	dig_uri = soup_uri_new (uri);
	if (dig_uri) {
		if (!soup_uri_equal (dig_uri, req_uri)) {
			soup_uri_free (dig_uri);
			return FALSE;
		}
		soup_uri_free (dig_uri);
	} else {	
		char *req_path;

		req_path = soup_uri_to_string (req_uri, TRUE);
		if (strcmp (uri, req_path) != 0) {
			g_free (req_path);
			return FALSE;
		}
		g_free (req_path);
	}

	/* Check qop; we only support "auth" for now */
	qop = g_hash_table_lookup (params, "qop");
	if (!qop || strcmp (qop, "auth") != 0)
		return FALSE;

	/* Check realm */
	realm = g_hash_table_lookup (params, "realm");
	if (!realm || strcmp (realm, soup_auth_domain_get_realm (domain)) != 0)
		return FALSE;

	nonce = g_hash_table_lookup (params, "nonce");
	if (!nonce)
		return FALSE;
	nc = g_hash_table_lookup (params, "nc");
	if (!nc)
		return FALSE;
	nonce_count = strtoul (nc, NULL, 16);
	if (nonce_count <= 0)
		return FALSE;
	cnonce = g_hash_table_lookup (params, "cnonce");
	if (!cnonce)
		return FALSE;
	response = g_hash_table_lookup (params, "response");
	if (!response)
		return FALSE;

	soup_auth_digest_compute_hex_a1 (hex_urp,
					 SOUP_AUTH_DIGEST_ALGORITHM_MD5,
					 nonce, cnonce, hex_a1);
	soup_auth_digest_compute_response (msg->method, uri,
					   hex_a1,
					   SOUP_AUTH_DIGEST_QOP_AUTH,
					   nonce, cnonce, nonce_count,
					   computed_response);
	return strcmp (response, computed_response) == 0;
}

static char *
soup_auth_domain_digest_accepts (SoupAuthDomain *domain, SoupMessage *msg,
				 const char *header)
{
	SoupAuthDomainDigestPrivate *priv =
		SOUP_AUTH_DOMAIN_DIGEST_GET_PRIVATE (domain);
	GHashTable *params;
	const char *username;
	gboolean accept = FALSE;
	char *ret_user;

	if (strncmp (header, "Digest ", 7) != 0)
		return NULL;

	params = soup_header_parse_param_list (header + 7);
	if (!params)
		return NULL;

	username = g_hash_table_lookup (params, "username");
	if (!username) {
		soup_header_free_param_list (params);
		return NULL;
	}

	if (priv->auth_callback) {
		char *hex_urp;

		hex_urp = priv->auth_callback (domain, msg, username,
					       priv->auth_data);
		if (hex_urp) {
			accept = check_hex_urp (domain, msg, params,
						username, hex_urp);
			g_free (hex_urp);
		} else
			accept = FALSE;
	} else {
		accept = soup_auth_domain_try_generic_auth_callback (
			domain, msg, username);
	}

	ret_user = accept ? g_strdup (username) : NULL;
	soup_header_free_param_list (params);
	return ret_user;
}

static char *
soup_auth_domain_digest_challenge (SoupAuthDomain *domain, SoupMessage *msg)
{
	GString *str;

	str = g_string_new ("Digest ");
	soup_header_g_string_append_param_quoted (str, "realm", soup_auth_domain_get_realm (domain));
	g_string_append_printf (str, ", nonce=\"%lu%lu\"", 
				(unsigned long) msg,
				(unsigned long) time (0));
	g_string_append_printf (str, ", qop=\"auth\"");
	g_string_append_printf (str, ", algorithm=MD5");

	return g_string_free (str, FALSE);
}

/**
 * soup_auth_domain_digest_encode_password:
 * @username: a username
 * @realm: an auth realm name
 * @password: the password for @username in @realm
 *
 * Encodes the username/realm/password triplet for Digest
 * authentication. (That is, it returns a stringified MD5 hash of
 * @username, @realm, and @password concatenated together). This is
 * the form that is needed as the return value of
 * #SoupAuthDomainDigest's auth handler.
 *
 * For security reasons, you should store the encoded hash, rather
 * than storing the cleartext password itself and calling this method
 * only when you need to verify it. This way, if your server is
 * compromised, the attackers will not gain access to cleartext
 * passwords which might also be usable at other sites. (Note also
 * that the encoded password returned by this method is identical to
 * the encoded password stored in an Apache .htdigest file.)
 *
 * Return value: the encoded password
 **/
char *
soup_auth_domain_digest_encode_password (const char *username,
					 const char *realm,
					 const char *password)
{
	char hex_urp[33];

	soup_auth_digest_compute_hex_urp (username, realm, password, hex_urp);
	return g_strdup (hex_urp);
}

static gboolean
soup_auth_domain_digest_check_password (SoupAuthDomain *domain,
					SoupMessage    *msg,
					const char     *username,
					const char     *password)
{
	const char *header;
	GHashTable *params;
	const char *msg_username;
	char hex_urp[33];
	gboolean accept;

	header = soup_message_headers_get_one (msg->request_headers,
					       "Authorization");
	if (!header || (strncmp (header, "Digest ", 7) != 0))
		return FALSE;

	params = soup_header_parse_param_list (header + 7);
	if (!params)
		return FALSE;

	msg_username = g_hash_table_lookup (params, "username");
	if (!msg_username || strcmp (msg_username, username) != 0) {
		soup_header_free_param_list (params);
		return FALSE;
	}

	soup_auth_digest_compute_hex_urp (username,
					  soup_auth_domain_get_realm (domain),
					  password, hex_urp);
	accept = check_hex_urp (domain, msg, params, username, hex_urp);
	soup_header_free_param_list (params);
	return accept;
}

static void
soup_auth_domain_digest_class_init (SoupAuthDomainDigestClass *digest_class)
{
	SoupAuthDomainClass *auth_domain_class =
		SOUP_AUTH_DOMAIN_CLASS (digest_class);
	GObjectClass *object_class = G_OBJECT_CLASS (digest_class);

	g_type_class_add_private (digest_class, sizeof (SoupAuthDomainDigestPrivate));

	auth_domain_class->accepts        = soup_auth_domain_digest_accepts;
	auth_domain_class->challenge      = soup_auth_domain_digest_challenge;
	auth_domain_class->check_password = soup_auth_domain_digest_check_password;

	object_class->finalize     = soup_auth_domain_digest_finalize;
	object_class->set_property = soup_auth_domain_digest_set_property;
	object_class->get_property = soup_auth_domain_digest_get_property;

	/**
	 * SOUP_AUTH_DOMAIN_DIGEST_AUTH_CALLBACK:
	 *
	 * Alias for the #SoupAuthDomainDigest:auth-callback property.
	 * (The #SoupAuthDomainDigestAuthCallback.)
	 **/
	g_object_class_install_property (
		object_class, PROP_AUTH_CALLBACK,
		g_param_spec_pointer (SOUP_AUTH_DOMAIN_DIGEST_AUTH_CALLBACK,
				      "Authentication callback",
				      "Password-finding callback",
				      G_PARAM_READWRITE));
	/**
	 * SOUP_AUTH_DOMAIN_DIGEST_AUTH_DATA:
	 *
	 * Alias for the #SoupAuthDomainDigest:auth-callback property.
	 * (The #SoupAuthDomainDigestAuthCallback.)
	 **/
	g_object_class_install_property (
		object_class, PROP_AUTH_DATA,
		g_param_spec_pointer (SOUP_AUTH_DOMAIN_DIGEST_AUTH_DATA,
				      "Authentication callback data",
				      "Data to pass to authentication callback",
				      G_PARAM_READWRITE));
}
