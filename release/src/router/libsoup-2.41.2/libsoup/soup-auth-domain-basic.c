/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-auth-domain-basic.c: HTTP Basic Authentication (server-side)
 *
 * Copyright (C) 2007 Novell, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "soup-auth-domain-basic.h"
#include "soup.h"
#include "soup-marshal.h"

/**
 * SECTION:soup-auth-domain-basic
 * @short_description: Server-side "Basic" authentication
 *
 * #SoupAuthDomainBasic handles the server side of HTTP "Basic" (ie,
 * cleartext password) authentication.
 **/

enum {
	PROP_0,

	PROP_AUTH_CALLBACK,
	PROP_AUTH_DATA,

	LAST_PROP
};

typedef struct {
	SoupAuthDomainBasicAuthCallback auth_callback;
	gpointer auth_data;
	GDestroyNotify auth_dnotify;
} SoupAuthDomainBasicPrivate;

#define SOUP_AUTH_DOMAIN_BASIC_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), SOUP_TYPE_AUTH_DOMAIN_BASIC, SoupAuthDomainBasicPrivate))

G_DEFINE_TYPE (SoupAuthDomainBasic, soup_auth_domain_basic, SOUP_TYPE_AUTH_DOMAIN)

static void
soup_auth_domain_basic_init (SoupAuthDomainBasic *basic)
{
}

static void
soup_auth_domain_basic_finalize (GObject *object)
{
	SoupAuthDomainBasicPrivate *priv =
		SOUP_AUTH_DOMAIN_BASIC_GET_PRIVATE (object);

	if (priv->auth_dnotify)
		priv->auth_dnotify (priv->auth_data);

	G_OBJECT_CLASS (soup_auth_domain_basic_parent_class)->finalize (object);
}

static void
soup_auth_domain_basic_set_property (GObject *object, guint prop_id,
				     const GValue *value, GParamSpec *pspec)
{
	SoupAuthDomainBasicPrivate *priv =
		SOUP_AUTH_DOMAIN_BASIC_GET_PRIVATE (object);

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
soup_auth_domain_basic_get_property (GObject *object, guint prop_id,
				     GValue *value, GParamSpec *pspec)
{
	SoupAuthDomainBasicPrivate *priv =
		SOUP_AUTH_DOMAIN_BASIC_GET_PRIVATE (object);

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
 * soup_auth_domain_basic_new:
 * @optname1: name of first option, or %NULL
 * @...: option name/value pairs
 *
 * Creates a #SoupAuthDomainBasic. You must set the
 * %SOUP_AUTH_DOMAIN_REALM parameter, to indicate the realm name to be
 * returned with the authentication challenge to the client. Other
 * parameters are optional.
 *
 * Return value: the new #SoupAuthDomain
 **/
SoupAuthDomain *
soup_auth_domain_basic_new (const char *optname1, ...)
{
	SoupAuthDomain *domain;
	va_list ap;

	va_start (ap, optname1);
	domain = (SoupAuthDomain *)g_object_new_valist (SOUP_TYPE_AUTH_DOMAIN_BASIC,
							optname1, ap);
	va_end (ap);

	g_return_val_if_fail (soup_auth_domain_get_realm (domain) != NULL, NULL);

	return domain;
}

/**
 * SoupAuthDomainBasicAuthCallback:
 * @domain: the domain
 * @msg: the message being authenticated
 * @username: the username provided by the client
 * @password: the password provided by the client
 * @user_data: the data passed to soup_auth_domain_basic_set_auth_callback()
 *
 * Callback used by #SoupAuthDomainBasic for authentication purposes.
 * The application should verify that @username and @password and valid
 * and return %TRUE or %FALSE.
 *
 * If you are maintaining your own password database (rather than
 * using the password to authenticate against some other system like
 * PAM or a remote server), you should make sure you know what you are
 * doing. In particular, don't store cleartext passwords, or
 * easily-computed hashes of cleartext passwords, even if you don't
 * care that much about the security of your server, because users
 * will frequently use the same password for multiple sites, and so
 * compromising any site with a cleartext (or easily-cracked) password
 * database may give attackers access to other more-interesting sites
 * as well.
 *
 * Return value: %TRUE if @username and @password are valid
 **/

/**
 * soup_auth_domain_basic_set_auth_callback:
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
 * %SOUP_AUTH_DOMAIN_BASIC_AUTH_CALLBACK and
 * %SOUP_AUTH_DOMAIN_BASIC_AUTH_DATA properties, which can also be
 * used to set the callback at construct time.
 **/
void
soup_auth_domain_basic_set_auth_callback (SoupAuthDomain *domain,
					  SoupAuthDomainBasicAuthCallback callback,
					  gpointer        user_data,
					  GDestroyNotify  dnotify)
{
	SoupAuthDomainBasicPrivate *priv =
		SOUP_AUTH_DOMAIN_BASIC_GET_PRIVATE (domain);

	if (priv->auth_dnotify)
		priv->auth_dnotify (priv->auth_data);

	priv->auth_callback = callback;
	priv->auth_data = user_data;
	priv->auth_dnotify = dnotify;

	g_object_notify (G_OBJECT (domain), SOUP_AUTH_DOMAIN_BASIC_AUTH_CALLBACK);
	g_object_notify (G_OBJECT (domain), SOUP_AUTH_DOMAIN_BASIC_AUTH_DATA);
}

static void
pw_free (char *pw)
{
	memset (pw, 0, strlen (pw));
	g_free (pw);
}

static gboolean
parse_basic (SoupMessage *msg, const char *header,
	     char **username, char **password)
{
	char *decoded, *colon;
	gsize len, plen;

	if (!header || (strncmp (header, "Basic ", 6) != 0))
		return FALSE;

	decoded = (char *)g_base64_decode (header + 6, &len);
	if (!decoded)
		return FALSE;

	colon = memchr (decoded, ':', len);
	if (!colon) {
		pw_free (decoded);
		return FALSE;
	}
	*colon = '\0';
	plen = len - (colon - decoded) - 1;

	*password = g_strndup (colon + 1, plen);
	memset (colon + 1, 0, plen);
	*username = decoded;
	return TRUE;
}

static char *
soup_auth_domain_basic_accepts (SoupAuthDomain *domain, SoupMessage *msg,
				const char *header)
{
	SoupAuthDomainBasicPrivate *priv =
		SOUP_AUTH_DOMAIN_BASIC_GET_PRIVATE (domain);
	char *username, *password;
	gboolean ok = FALSE;

	if (!parse_basic (msg, header, &username, &password))
		return NULL;

	if (priv->auth_callback) {
		ok = priv->auth_callback (domain, msg, username, password,
					  priv->auth_data);
	} else {
		ok = soup_auth_domain_try_generic_auth_callback (
			domain, msg, username);
	}

	pw_free (password);

	if (ok)
		return username;
	else {
		g_free (username);
		return NULL;
	}
}

static char *
soup_auth_domain_basic_challenge (SoupAuthDomain *domain, SoupMessage *msg)
{
	GString *challenge;

	challenge = g_string_new ("Basic ");
	soup_header_g_string_append_param (challenge, "realm", soup_auth_domain_get_realm (domain));
	return g_string_free (challenge, FALSE);
}

static gboolean
soup_auth_domain_basic_check_password (SoupAuthDomain *domain,
				       SoupMessage    *msg,
				       const char     *username,
				       const char     *password)
{
	const char *header;
	char *msg_username, *msg_password;
	gboolean ok;

	header = soup_message_headers_get_one (msg->request_headers,
					       "Authorization");
	if (!parse_basic (msg, header, &msg_username, &msg_password))
		return FALSE;

	ok = (!strcmp (username, msg_username) &&
	      !strcmp (password, msg_password));
	g_free (msg_username);
	pw_free (msg_password);

	return ok;
}

static void
soup_auth_domain_basic_class_init (SoupAuthDomainBasicClass *basic_class)
{
	SoupAuthDomainClass *auth_domain_class =
		SOUP_AUTH_DOMAIN_CLASS (basic_class);
	GObjectClass *object_class = G_OBJECT_CLASS (basic_class);

	g_type_class_add_private (basic_class, sizeof (SoupAuthDomainBasicPrivate));

	auth_domain_class->accepts        = soup_auth_domain_basic_accepts;
	auth_domain_class->challenge      = soup_auth_domain_basic_challenge;
	auth_domain_class->check_password = soup_auth_domain_basic_check_password;

	object_class->finalize     = soup_auth_domain_basic_finalize;
	object_class->set_property = soup_auth_domain_basic_set_property;
	object_class->get_property = soup_auth_domain_basic_get_property;

	/**
	 * SOUP_AUTH_DOMAIN_BASIC_AUTH_CALLBACK:
	 *
	 * Alias for the #SoupAuthDomainBasic:auth-callback property.
	 * (The #SoupAuthDomainBasicAuthCallback.)
	 **/
	g_object_class_install_property (
		object_class, PROP_AUTH_CALLBACK,
		g_param_spec_pointer (SOUP_AUTH_DOMAIN_BASIC_AUTH_CALLBACK,
				      "Authentication callback",
				      "Password-checking callback",
				      G_PARAM_READWRITE));
	/**
	 * SOUP_AUTH_DOMAIN_BASIC_AUTH_DATA:
	 *
	 * Alias for the #SoupAuthDomainBasic:auth-data property.
	 * (The data to pass to the #SoupAuthDomainBasicAuthCallback.)
	 **/
	g_object_class_install_property (
		object_class, PROP_AUTH_DATA,
		g_param_spec_pointer (SOUP_AUTH_DOMAIN_BASIC_AUTH_DATA,
				      "Authentication callback data",
				      "Data to pass to authentication callback",
				      G_PARAM_READWRITE));
}
