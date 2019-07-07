/*
 * Copyright (C) 2015 Lubomir Rintel
 *
 * Copyright (C) 2013-2016 Tobias Brunner
 * Copyright (C) 2008-2011 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2004 Dan Williams
 * Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libsecret/secret.h>

#include <NetworkManager.h>
#include <nm-vpn-service-plugin.h>
#include <nma-vpn-password-dialog.h>

#define NM_DBUS_SERVICE_STRONGSWAN	"org.freedesktop.NetworkManager.strongswan"

#define KEYRING_UUID_TAG "connection-uuid"
#define KEYRING_SN_TAG "setting-name"
#define KEYRING_SK_TAG "setting-key"

static const SecretSchema network_manager_secret_schema = {
	"org.freedesktop.NetworkManager.Connection",
	SECRET_SCHEMA_DONT_MATCH_NAME,
	{
		{ KEYRING_UUID_TAG, SECRET_SCHEMA_ATTRIBUTE_STRING },
		{ KEYRING_SN_TAG, SECRET_SCHEMA_ATTRIBUTE_STRING },
		{ KEYRING_SK_TAG, SECRET_SCHEMA_ATTRIBUTE_STRING },
		{ NULL, 0 },
	}
};

#define UI_KEYFILE_GROUP "VPN Plugin UI"

static char *keyring_lookup_secret(const char *uuid, const char *secret_name)
{
	GHashTable *attrs;
	GList *list;
	char *secret = NULL;

	attrs = secret_attributes_build(&network_manager_secret_schema,
									KEYRING_UUID_TAG, uuid,
									KEYRING_SN_TAG, NM_SETTING_VPN_SETTING_NAME,
									KEYRING_SK_TAG, secret_name,
									NULL);

	list = secret_service_search_sync (NULL, &network_manager_secret_schema, attrs,
									   SECRET_SEARCH_ALL | SECRET_SEARCH_UNLOCK | SECRET_SEARCH_LOAD_SECRETS,
									   NULL, NULL);
	if (list && list->data)
	{
		SecretItem *item = list->data;
		SecretValue *value = secret_item_get_secret (item);

		if (value)
		{
			secret = g_strdup (secret_value_get (value, NULL));
			secret_value_unref (value);
		}
	}

	g_list_free_full (list, g_object_unref);
	g_hash_table_unref (attrs);
	return secret;
}

static void keyfile_add_entry_info(GKeyFile *keyfile, const gchar *key, const gchar *value,
								   const gchar *label, gboolean is_secret, gboolean should_ask)
{
	g_key_file_set_string (keyfile, key, "Value", value);
	g_key_file_set_string (keyfile, key, "Label", label);
	g_key_file_set_boolean (keyfile, key, "IsSecret", is_secret);
	g_key_file_set_boolean (keyfile, key, "ShouldAsk", should_ask);
}

static void keyfile_print_stdout (GKeyFile *keyfile)
{
	gchar *data;
	gsize length;

	data = g_key_file_to_data (keyfile, &length, NULL);

	fputs (data, stdout);

	g_free (data);
}

static gboolean get_secrets(const char *type, const char *uuid, const char *name, gboolean retry,
							gboolean allow_interaction, gboolean external_ui_mode,
							const char *in_pw, char **out_pw, NMSettingSecretFlags flags)
{
	NMAVpnPasswordDialog *dialog;
	char *prompt, *pw = NULL;
	const char *new_pw = NULL;
	guint32 minlen = 0;

	if (!(flags & NM_SETTING_SECRET_FLAG_NOT_SAVED) &&
		!(flags & NM_SETTING_SECRET_FLAG_NOT_REQUIRED))
	{
		if (in_pw)
		{
			pw = g_strdup (in_pw);
		}
		else
		{
			pw = keyring_lookup_secret (uuid, "password");
		}
	}
	if (flags & NM_SETTING_SECRET_FLAG_NOT_REQUIRED)
	{
		g_free (pw);
		return TRUE;
	}
	if (!strcmp(type, "eap"))
	{
		prompt = g_strdup_printf (_("EAP password required to establish VPN connection '%s'."),
								  name);
	}
	else if (!strcmp(type, "key"))
	{
		prompt = g_strdup_printf (_("Private key decryption password required to establish VPN connection '%s'."),
								  name);
	}
	else if (!strcmp(type, "psk"))
	{
		prompt = g_strdup_printf (_("Pre-shared key required to establish VPN connection '%s' (min. 20 characters)."),
								  name);
		minlen = 20;
	}
	else /* smartcard */
	{
		prompt = g_strdup_printf (_("Smartcard PIN required to establish VPN connection '%s'."),
								  name);
	}
	if (external_ui_mode)
	{
		GKeyFile *keyfile;

		keyfile = g_key_file_new ();

		g_key_file_set_integer (keyfile, UI_KEYFILE_GROUP, "Version", 2);
		g_key_file_set_string (keyfile, UI_KEYFILE_GROUP, "Description", prompt);
		g_key_file_set_string (keyfile, UI_KEYFILE_GROUP, "Title", _("Authenticate VPN"));

		keyfile_add_entry_info (keyfile, "password", pw ?: "", _("Password:"), TRUE, allow_interaction);

		keyfile_print_stdout (keyfile);
		g_key_file_unref (keyfile);
		goto out;
	}
	else if (!allow_interaction ||
			(!retry && pw && !(flags & NM_SETTING_SECRET_FLAG_NOT_SAVED)))
	{
		/* If we can't prompt the user, just return the existing password. Do the same
		 * if we don't nee a new password (!retry) and have an existing saved one */
		*out_pw = pw;
		g_free (prompt);
		return TRUE;
	}

	dialog = (NMAVpnPasswordDialog*)nma_vpn_password_dialog_new(_("Authenticate VPN"), prompt, NULL);
	nma_vpn_password_dialog_set_show_password_secondary(dialog, FALSE);

	if (pw && !(flags & NM_SETTING_SECRET_FLAG_NOT_SAVED))
	{
		nma_vpn_password_dialog_set_password(dialog, pw);
	}
	gtk_widget_show (GTK_WIDGET (dialog));

too_short_retry:
	if (nma_vpn_password_dialog_run_and_block (dialog))
	{
		new_pw = nma_vpn_password_dialog_get_password(dialog);
		if (new_pw && minlen && strlen(new_pw) < minlen)
		{
			goto too_short_retry;
		}
		else if (new_pw)
		{
			*out_pw = g_strdup (new_pw);
		}
	}
	gtk_widget_hide (GTK_WIDGET (dialog));
	gtk_widget_destroy (GTK_WIDGET (dialog));
out:
	g_free (prompt);
	return TRUE;
}

static void print_secret (const char *secret_name, gchar *secret)
{
	if (secret)
	{
		printf("%s\n%s\n", secret_name, secret);
		g_free(secret);
	}
	printf("\n\n");
	fflush(stdout);
}

static void wait_for_quit (void)
{
	GString *str;
	char c;
	ssize_t n;
	time_t start;

	str = g_string_sized_new (10);
	start = time (NULL);
	do {
		errno = 0;
		n = read (0, &c, 1);
		if (n == 0 || (n < 0 && errno == EAGAIN))
			g_usleep (G_USEC_PER_SEC / 10);
		else if (n == 1) {
			g_string_append_c (str, c);
			if (strstr (str->str, "QUIT") || (str->len > 10))
				break;
		} else
			break;
	} while (time (NULL) < start + 20);
	g_string_free (str, TRUE);
}

int main (int argc, char *argv[])
{
	gboolean retry = FALSE, allow_interaction = FALSE, external_ui_mode = FALSE;
	gchar *name = NULL, *uuid = NULL, *service = NULL, *pass = NULL;
	GHashTable *data = NULL, *secrets = NULL;
	NMSettingSecretFlags flags = NM_SETTING_SECRET_FLAG_NONE;
	GOptionContext *context;
	char *agent, *type;
	int status = 0;
	GOptionEntry entries[] = {
		{ "reprompt", 'r', 0, G_OPTION_ARG_NONE, &retry, "Reprompt for passwords", NULL},
		{ "uuid", 'u', 0, G_OPTION_ARG_STRING, &uuid, "UUID of VPN connection", NULL},
		{ "name", 'n', 0, G_OPTION_ARG_STRING, &name, "Name of VPN connection", NULL},
		{ "service", 's', 0, G_OPTION_ARG_STRING, &service, "VPN service type", NULL},
		{ "allow-interaction", 'i', 0, G_OPTION_ARG_NONE, &allow_interaction, "Allow user interaction", NULL},
		{ "external-ui-mode", 0, 0, G_OPTION_ARG_NONE, &external_ui_mode, "External UI mode", NULL},
		{ NULL }
	};

	bindtextdomain(GETTEXT_PACKAGE, NULL);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	gtk_init (&argc, &argv);

	context = g_option_context_new ("- strongswan auth dialog");
	g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
	g_option_context_parse (context, &argc, &argv, NULL);
	g_option_context_free (context);

	if (uuid == NULL || name == NULL || service == NULL)
	{
		fprintf (stderr, "Have to supply UUID, name, and service\n");
		return 1;
	}

	if (strcmp(service, NM_DBUS_SERVICE_STRONGSWAN) != 0)
	{
		fprintf(stderr, "This dialog only works with the '%s' service\n",
				NM_DBUS_SERVICE_STRONGSWAN);
		return 1;
	}

	if (!nm_vpn_service_plugin_read_vpn_details (0, &data, &secrets))
	{
		fprintf(stderr, "Failed to read '%s' (%s) data and secrets from stdin.\n",
				name, uuid);
		return 1;
	}

	type = g_hash_table_lookup (data, "method");
	if (!type)
	{
		fprintf(stderr, "Connection lookup failed\n");
		status = 1;
		goto out;
	}

	if (!strcmp(type, "eap") || !strcmp(type, "key") ||
		!strcmp(type, "psk") || !strcmp(type, "smartcard"))
	{
		nm_vpn_service_plugin_get_secret_flags (secrets, "password", &flags);
		if (!get_secrets(type, uuid, name, retry, allow_interaction, external_ui_mode,
						 g_hash_table_lookup (secrets, "password"), &pass, flags))
		{
			status = 1;
		}
		else if (!external_ui_mode)
		{
			print_secret("password", pass);
			wait_for_quit ();
		}
	}
	else if (!strcmp(type, "agent"))
	{
		agent = getenv("SSH_AUTH_SOCK");
		if (agent)
		{
			if (external_ui_mode)
			{
				GKeyFile *keyfile;

				keyfile = g_key_file_new ();

				g_key_file_set_integer (keyfile, UI_KEYFILE_GROUP, "Version", 2);
				g_key_file_set_string (keyfile, UI_KEYFILE_GROUP, "Description", "SSH agent");
				g_key_file_set_string (keyfile, UI_KEYFILE_GROUP, "Title", _("Authenticate VPN"));

				keyfile_add_entry_info (keyfile, "agent", agent, "SSH agent socket", TRUE, FALSE);

				keyfile_print_stdout (keyfile);
				g_key_file_unref (keyfile);
			}
			else
			{
				print_secret("agent", g_strdup (agent));
				wait_for_quit ();
			}
		}
		else if (allow_interaction)
		{
			GtkWidget *dialog;
			dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR,
						  GTK_BUTTONS_OK,
						  _("Configuration uses ssh-agent for authentication, "
						  "but ssh-agent is not running!"));
			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
		}
	}

out:
	if (data)
	{
		g_hash_table_unref (data);
	}
	if (secrets)
	{
		g_hash_table_unref(secrets);
	}
	return status;
}
