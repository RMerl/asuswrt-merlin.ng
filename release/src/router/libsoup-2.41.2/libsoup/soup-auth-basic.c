/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-auth-basic.c: HTTP Basic Authentication
 *
 * Copyright (C) 2001-2003, Ximian, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "soup-auth-basic.h"
#include "soup.h"

typedef struct {
	char *token;
} SoupAuthBasicPrivate;
#define SOUP_AUTH_BASIC_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), SOUP_TYPE_AUTH_BASIC, SoupAuthBasicPrivate))

G_DEFINE_TYPE (SoupAuthBasic, soup_auth_basic, SOUP_TYPE_AUTH)

static void
soup_auth_basic_init (SoupAuthBasic *basic)
{
}

static void
soup_auth_basic_finalize (GObject *object)
{
	SoupAuthBasicPrivate *priv = SOUP_AUTH_BASIC_GET_PRIVATE (object);

	g_free (priv->token);

	G_OBJECT_CLASS (soup_auth_basic_parent_class)->finalize (object);
}

static gboolean
soup_auth_basic_update (SoupAuth *auth, SoupMessage *msg,
			GHashTable *auth_params)
{
	SoupAuthBasicPrivate *priv = SOUP_AUTH_BASIC_GET_PRIVATE (auth);

	/* If we're updating a pre-existing auth, the
	 * username/password must be bad now, so forget it.
	 * Other than that, there's nothing to do here.
	 */
	if (priv->token) {
		memset (priv->token, 0, strlen (priv->token));
		g_free (priv->token);
		priv->token = NULL;
	}

	return TRUE;
}

static GSList *
soup_auth_basic_get_protection_space (SoupAuth *auth, SoupURI *source_uri)
{
	char *space, *p;

	space = g_strdup (source_uri->path);

	/* Strip query and filename component */
	p = strrchr (space, '/');
	if (p && p != space && p[1])
		*p = '\0';

	return g_slist_prepend (NULL, space);
}

static void
soup_auth_basic_authenticate (SoupAuth *auth, const char *username,
			      const char *password)
{
	SoupAuthBasicPrivate *priv = SOUP_AUTH_BASIC_GET_PRIVATE (auth);
	char *user_pass, *user_pass_latin1;
	int len;

	user_pass = g_strdup_printf ("%s:%s", username, password);
	user_pass_latin1 = g_convert (user_pass, -1, "ISO-8859-1", "UTF-8",
				      NULL, NULL, NULL);
	if (user_pass_latin1) {
		memset (user_pass, 0, strlen (user_pass));
		g_free (user_pass);
		user_pass = user_pass_latin1;
	}
	len = strlen (user_pass);

	if (priv->token) {
		memset (priv->token, 0, strlen (priv->token));
		g_free (priv->token);
	}
	priv->token = g_base64_encode ((guchar *)user_pass, len);

	memset (user_pass, 0, len);
	g_free (user_pass);
}

static gboolean
soup_auth_basic_is_authenticated (SoupAuth *auth)
{
	return SOUP_AUTH_BASIC_GET_PRIVATE (auth)->token != NULL;
}

static char *
soup_auth_basic_get_authorization (SoupAuth *auth, SoupMessage *msg)
{
	SoupAuthBasicPrivate *priv = SOUP_AUTH_BASIC_GET_PRIVATE (auth);

	return g_strdup_printf ("Basic %s", priv->token);
}

static void
soup_auth_basic_class_init (SoupAuthBasicClass *auth_basic_class)
{
	SoupAuthClass *auth_class = SOUP_AUTH_CLASS (auth_basic_class);
	GObjectClass *object_class = G_OBJECT_CLASS (auth_basic_class);

	g_type_class_add_private (auth_basic_class, sizeof (SoupAuthBasicPrivate));

	auth_class->scheme_name = "Basic";
	auth_class->strength = 1;

	auth_class->update = soup_auth_basic_update;
	auth_class->get_protection_space = soup_auth_basic_get_protection_space;
	auth_class->authenticate = soup_auth_basic_authenticate;
	auth_class->is_authenticated = soup_auth_basic_is_authenticated;
	auth_class->get_authorization = soup_auth_basic_get_authorization;

	object_class->finalize = soup_auth_basic_finalize;
}
