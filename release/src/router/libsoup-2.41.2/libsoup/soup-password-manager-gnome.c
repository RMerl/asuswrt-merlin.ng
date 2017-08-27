/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-password-manager-gnome.c: GNOME-keyring-based password manager
 *
 * Copyright (C) 2008 Red Hat, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome-keyring.h>

#define LIBSOUP_I_HAVE_READ_BUG_594377_AND_KNOW_SOUP_PASSWORD_MANAGER_MIGHT_GO_AWAY

#include "soup-password-manager-gnome.h"
#include "soup.h"

static void soup_password_manager_gnome_interface_init (SoupPasswordManagerInterface *password_manager_interface);

G_DEFINE_TYPE_EXTENDED (SoupPasswordManagerGNOME, soup_password_manager_gnome, G_TYPE_OBJECT, 0,
			G_IMPLEMENT_INTERFACE (SOUP_TYPE_SESSION_FEATURE, NULL)
			G_IMPLEMENT_INTERFACE (SOUP_TYPE_PASSWORD_MANAGER, soup_password_manager_gnome_interface_init))

static void
soup_password_manager_gnome_init (SoupPasswordManagerGNOME *manager_gnome)
{
}


static void
save_password_callback (GnomeKeyringResult result, guint32 val, gpointer data)
{
}

static void
async_save_password (SoupAuth *auth, const char *username,
		     const char *password, gpointer user_data)
{
	SoupURI *uri = user_data;

	gnome_keyring_set_network_password (
		NULL, /* use default keyring */
		username,
		soup_auth_get_realm (auth),
		uri->host,
		NULL,
		uri->scheme,
		soup_auth_get_scheme_name (auth),
		uri->port,
		password,
		save_password_callback, NULL, NULL);
}

static void
sync_save_password (SoupAuth *auth, const char *username,
		    const char *password, gpointer user_data)
{
	SoupURI *uri = user_data;
	guint32 item_id;

	gnome_keyring_set_network_password_sync (
		NULL, /* use default keyring */
		username,
		soup_auth_get_realm (auth),
		uri->host,
		NULL,
		uri->scheme,
		soup_auth_get_scheme_name (auth),
		uri->port,
		password,
		&item_id);
}

static void
update_auth_for_passwords (SoupAuth *auth, SoupMessage *msg,
			   GList *passwords, gboolean async)
{
	GnomeKeyringNetworkPasswordData *pdata;
	SoupURI *uri;

	while (passwords) {
		pdata = passwords->data;
		soup_auth_has_saved_password (auth, pdata->user,
					      pdata->password);
		passwords = passwords->next;
	}

	uri = g_object_get_data (G_OBJECT (auth),
				 "SoupPasswordManagerGNOME-save_password-uri");
	if (uri) {
		g_signal_handlers_disconnect_by_func (auth, async_save_password, uri);
		g_signal_handlers_disconnect_by_func (auth, sync_save_password, uri);
	}

	uri = soup_uri_copy (soup_message_get_uri (msg));
	g_signal_connect (auth, "save_password",
			  G_CALLBACK (async ? async_save_password : sync_save_password),
			  uri);
	g_object_set_data_full (G_OBJECT (auth),
				"SoupPasswordManagerGNOME-save_password-uri",
				uri, (GDestroyNotify)soup_uri_free);
}

typedef struct {
	SoupPasswordManager *password_manager;
	SoupMessage *msg;
	SoupAuth *auth;
	gboolean retrying;

	SoupPasswordManagerCallback callback;
	gpointer user_data;

	gpointer request;
} SoupPasswordManagerGNOMEAuthData;

static void
find_password_callback (GnomeKeyringResult result, GList *list,
			gpointer user_data)
{
	SoupPasswordManagerGNOMEAuthData *auth_data = user_data;

	/* FIXME: check result? */

	update_auth_for_passwords (auth_data->auth, auth_data->msg, list, TRUE);
	auth_data->callback (auth_data->password_manager,
			     auth_data->msg, auth_data->auth,
			     auth_data->retrying, auth_data->user_data);

	/* gnome-keyring will call free_auth_data to clean up for us. */
}

static void
free_auth_data (gpointer data)
{
	SoupPasswordManagerGNOMEAuthData *auth_data = data;

	g_object_unref (auth_data->auth);
	g_object_unref (auth_data->msg);
	g_slice_free (SoupPasswordManagerGNOMEAuthData, auth_data);
}

static void
soup_password_manager_gnome_get_passwords_async (SoupPasswordManager  *password_manager,
						 SoupMessage          *msg,
						 SoupAuth             *auth,
						 gboolean              retrying,
						 GMainContext         *async_context,
						 GCancellable         *cancellable,
						 SoupPasswordManagerCallback callback,
						 gpointer              user_data)
{
	SoupPasswordManagerGNOMEAuthData *auth_data;
	SoupURI *uri = soup_message_get_uri (msg);

	auth_data = g_slice_new (SoupPasswordManagerGNOMEAuthData);
	auth_data->password_manager = password_manager;
	auth_data->msg = g_object_ref (msg);
	auth_data->auth = g_object_ref (auth);
	auth_data->retrying = retrying;

	/* FIXME: async_context, cancellable */

	auth_data->callback = callback;
	auth_data->user_data = user_data;

	/* FIXME: should we be specifying protocol and port here, or
	 * leaving them NULL/0 and filtering results in the callback?
	 * We don't want to send https passwords to http, but the
	 * reverse might be OK (if that's how other clients tend to
	 * behave).
	 */
	auth_data->request = gnome_keyring_find_network_password (
		NULL,                             /* user -- accept any */
		soup_auth_get_realm (auth),       /* domain */
		uri->host,                        /* server */
		NULL,                             /* object -- unused */
		uri->scheme,                      /* protocol */
		soup_auth_get_scheme_name (auth), /* authtype */
		uri->port,                        /* port */
		find_password_callback, auth_data, free_auth_data);
}

static void
soup_password_manager_gnome_get_passwords_sync (SoupPasswordManager  *password_manager,
						SoupMessage          *msg,
						SoupAuth             *auth,
						GCancellable         *cancellable)
{
	SoupURI *uri = soup_message_get_uri (msg);
	GList *results = NULL;

	/* FIXME: cancellable */

	gnome_keyring_find_network_password_sync (
		NULL,                             /* user -- accept any */
		soup_auth_get_realm (auth),       /* domain */
		uri->host,                        /* server */
		NULL,                             /* object -- unused */
		uri->scheme,                      /* protocol */
		soup_auth_get_scheme_name (auth), /* authtype */
		uri->port,                        /* port */
		&results);

	update_auth_for_passwords (auth, msg, results, FALSE);
}

static void
soup_password_manager_gnome_class_init (SoupPasswordManagerGNOMEClass *gnome_class)
{
}

static void
soup_password_manager_gnome_interface_init (SoupPasswordManagerInterface *password_manager_interface)
{
	password_manager_interface->get_passwords_async =
		soup_password_manager_gnome_get_passwords_async;
	password_manager_interface->get_passwords_sync =
		soup_password_manager_gnome_get_passwords_sync;
}
