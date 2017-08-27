/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-password-manager.c: HTTP auth password manager interface
 *
 * Copyright (C) 2008 Red Hat, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define LIBSOUP_I_HAVE_READ_BUG_594377_AND_KNOW_SOUP_PASSWORD_MANAGER_MIGHT_GO_AWAY

#include "soup-password-manager.h"
#include "soup.h"

G_DEFINE_INTERFACE_WITH_CODE (SoupPasswordManager, soup_password_manager, G_TYPE_OBJECT,
			      g_type_interface_add_prerequisite (g_define_type_id, SOUP_TYPE_SESSION_FEATURE);
			      )

static void
soup_password_manager_default_init (SoupPasswordManagerInterface *iface)
{
}

/**
 * soup_password_manager_get_passwords_async:
 * @password_manager: the #SoupPasswordManager
 * @msg: the #SoupMessage being authenticated
 * @auth: the #SoupAuth being authenticated
 * @retrying: whether or not this is a re-attempt to authenticate
 * @async_context: (allow-none): the #GMainContext to invoke @callback in
 * @cancellable: a #GCancellable, or %NULL
 * @callback: callback to invoke after fetching passwords
 * @user_data: data for @callback
 *
 * Asynchronously attempts to look up saved passwords for @auth/@msg
 * and then calls @callback after updating @auth with the information.
 * Also registers @auth with @password_manager so that if the caller
 * calls soup_auth_save_password() on it, the password will be saved.
 *
 * #SoupPasswordManager does not actually use the @retrying flag itself;
 * it just passes its value on to @callback.
 * 
 * If @cancellable is cancelled, @callback will still be invoked.
 *
 * Since: 2.28
 **/
void
soup_password_manager_get_passwords_async (SoupPasswordManager  *password_manager,
					   SoupMessage          *msg,
					   SoupAuth             *auth,
					   gboolean              retrying,
					   GMainContext         *async_context,
					   GCancellable         *cancellable,
					   SoupPasswordManagerCallback callback,
					   gpointer              user_data)
{
	SOUP_PASSWORD_MANAGER_GET_CLASS (password_manager)->
		get_passwords_async (password_manager, msg, auth, retrying,
				     async_context, cancellable,
				     callback, user_data);
}

/**
 * soup_password_manager_get_passwords_sync:
 * @password_manager: the #SoupPasswordManager
 * @msg: the #SoupMessage being authenticated
 * @auth: the #SoupAuth being authenticated
 * @cancellable: a #GCancellable, or %NULL
 *
 * Synchronously attempts to look up saved passwords for @auth/@msg
 * and updates @auth with the information. Also registers @auth with
 * @password_manager so that if the caller calls
 * soup_auth_save_password() on it, the password will be saved.
 *
 * Since: 2.28
 **/
void
soup_password_manager_get_passwords_sync (SoupPasswordManager  *password_manager,
					  SoupMessage          *msg,
					  SoupAuth             *auth,
					  GCancellable         *cancellable)
{
	SOUP_PASSWORD_MANAGER_GET_CLASS (password_manager)->
		get_passwords_sync (password_manager, msg, auth, cancellable);
}
