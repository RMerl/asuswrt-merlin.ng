/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/signalfd.h>
#include <fcntl.h>
#include <termios.h>
#include <getopt.h>
#include <syslog.h>
#include <glib.h>

#include "gdbus/gdbus.h"

#include "../client/manager.h"

#include "log.h"
#include "obexd.h"
#include "server.h"

#define DEFAULT_CAP_FILE CONFIGDIR "/capability.xml"

static GMainLoop *main_loop = NULL;

static gboolean signal_handler(GIOChannel *channel, GIOCondition cond,
							gpointer user_data)
{
	static unsigned int __terminated = 0;
	struct signalfd_siginfo si;
	ssize_t result;
	int fd;

	if (cond & (G_IO_NVAL | G_IO_ERR | G_IO_HUP))
		return FALSE;

	fd = g_io_channel_unix_get_fd(channel);

	result = read(fd, &si, sizeof(si));
	if (result != sizeof(si))
		return FALSE;

	switch (si.ssi_signo) {
	case SIGINT:
	case SIGTERM:
		if (__terminated == 0) {
			info("Terminating");
			g_main_loop_quit(main_loop);
		}

		__terminated = 1;
		break;
	case SIGUSR2:
		__obex_log_enable_debug();
		break;
	}

	return TRUE;
}

static guint setup_signalfd(void)
{
	GIOChannel *channel;
	guint source;
	sigset_t mask;
	int fd;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGUSR2);

	if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
		perror("Failed to set signal mask");
		return 0;
	}

	fd = signalfd(-1, &mask, 0);
	if (fd < 0) {
		perror("Failed to create signal descriptor");
		return 0;
	}

	channel = g_io_channel_unix_new(fd);

	g_io_channel_set_close_on_unref(channel, TRUE);
	g_io_channel_set_encoding(channel, NULL, NULL);
	g_io_channel_set_buffered(channel, FALSE);

	source = g_io_add_watch(channel,
				G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				signal_handler, NULL);

	g_io_channel_unref(channel);

	return source;
}

static gboolean option_detach = TRUE;
static char *option_debug = NULL;

static char *option_root = NULL;
static char *option_root_setup = NULL;
static char *option_capability = NULL;
static char *option_plugin = NULL;
static char *option_noplugin = NULL;

static gboolean option_autoaccept = FALSE;
static gboolean option_symlinks = FALSE;

static gboolean parse_debug(const char *key, const char *value,
				gpointer user_data, GError **error)
{
	if (value)
		option_debug = g_strdup(value);
	else
		option_debug = g_strdup("*");

	return TRUE;
}

static GOptionEntry options[] = {
	{ "debug", 'd', G_OPTION_FLAG_OPTIONAL_ARG,
				G_OPTION_ARG_CALLBACK, parse_debug,
				"Enable debug information output", "DEBUG" },
	{ "plugin", 'p', 0, G_OPTION_ARG_STRING, &option_plugin,
				"Specify plugins to load", "NAME,..." },
	{ "noplugin", 'P', 0, G_OPTION_ARG_STRING, &option_noplugin,
				"Specify plugins not to load", "NAME,..." },
	{ "nodetach", 'n', G_OPTION_FLAG_REVERSE,
				G_OPTION_ARG_NONE, &option_detach,
				"Run with logging in foreground" },
	{ "root", 'r', 0, G_OPTION_ARG_STRING, &option_root,
				"Specify root folder location. Both absolute "
				"and relative can be used, but relative paths "
				"are assumed to be relative to user $HOME "
				"folder. Default $XDG_CACHE_HOME", "PATH" },
	{ "root-setup", 'S', 0, G_OPTION_ARG_STRING, &option_root_setup,
				"Root folder setup script", "SCRIPT" },
	{ "symlinks", 'l', 0, G_OPTION_ARG_NONE, &option_symlinks,
				"Allow symlinks leading outside of the root "
				"folder" },
	{ "capability", 'c', 0, G_OPTION_ARG_STRING, &option_capability,
				"Specify capability file, use '!' mark for "
				"scripts", "FILE" },
	{ "auto-accept", 'a', 0, G_OPTION_ARG_NONE, &option_autoaccept,
				"Automatically accept push requests" },
	{ NULL },
};

gboolean obex_option_auto_accept(void)
{
	return option_autoaccept;
}

const char *obex_option_root_folder(void)
{
	return option_root;
}

gboolean obex_option_symlinks(void)
{
	return option_symlinks;
}

const char *obex_option_capability(void)
{
	return option_capability;
}

static gboolean is_dir(const char *dir)
{
	struct stat st;

	if (stat(dir, &st) < 0) {
		error("stat(%s): %s (%d)", dir, strerror(errno), errno);
		return FALSE;
	}

	return S_ISDIR(st.st_mode);
}

static gboolean root_folder_setup(char *root, char *root_setup)
{
	int status;
	char *argv[3] = { root_setup, root, NULL };

	if (is_dir(root))
		return TRUE;

	if (root_setup == NULL)
		return FALSE;

	DBG("Setting up %s using %s", root, root_setup);

	if (!g_spawn_sync(NULL, argv, NULL, 0, NULL, NULL, NULL, NULL,
							&status, NULL)) {
		error("Unable to execute %s", root_setup);
		return FALSE;
	}

	if (WEXITSTATUS(status) != EXIT_SUCCESS) {
		error("%s exited with status %d", root_setup,
							WEXITSTATUS(status));
		return FALSE;
	}

	return is_dir(root);
}

int main(int argc, char *argv[])
{
	GOptionContext *context;
	GError *err = NULL;
	guint signal;

#ifdef NEED_THREADS
	if (g_thread_supported() == FALSE)
		g_thread_init(NULL);
#endif

	context = g_option_context_new(NULL);
	g_option_context_add_main_entries(context, options, NULL);

	if (g_option_context_parse(context, &argc, &argv, &err) == FALSE) {
		if (err != NULL) {
			g_printerr("%s\n", err->message);
			g_error_free(err);
		} else
			g_printerr("An unknown error occurred\n");
		exit(EXIT_FAILURE);
	}

	g_option_context_free(context);

	__obex_log_init(option_debug, option_detach);

	DBG("Entering main loop");

	main_loop = g_main_loop_new(NULL, FALSE);

	signal = setup_signalfd();

#ifdef NEED_THREADS
	if (dbus_threads_init_default() == FALSE) {
		fprintf(stderr, "Can't init usage of threads\n");
		exit(EXIT_FAILURE);
	}
#endif

	if (manager_init() == FALSE) {
		error("manager_init failed");
		exit(EXIT_FAILURE);
	}

	if (option_root == NULL) {
		option_root = g_build_filename(g_get_user_cache_dir(), "obexd",
									NULL);
		g_mkdir_with_parents(option_root, 0700);
	}

	if (option_root[0] != '/') {
		const char *home = getenv("HOME");
		if (home) {
			char *old_root = option_root;
			option_root = g_strdup_printf("%s/%s", home, old_root);
			g_free(old_root);
		}
	}

	if (option_capability == NULL)
		option_capability = g_strdup(DEFAULT_CAP_FILE);

	plugin_init(option_plugin, option_noplugin);

	if (obex_server_init() < 0) {
		error("obex_server_init failed");
		exit(EXIT_FAILURE);
	}

	if (!root_folder_setup(option_root, option_root_setup)) {
		error("Unable to setup root folder %s", option_root);
		exit(EXIT_FAILURE);
	}

	if (client_manager_init() < 0) {
		error("client_manager_init failed");
		exit(EXIT_FAILURE);
	}

	g_main_loop_run(main_loop);

	g_source_remove(signal);

	client_manager_exit();

	obex_server_exit();

	plugin_cleanup();

	manager_cleanup();

	g_main_loop_unref(main_loop);

	g_free(option_capability);
	g_free(option_root);

	__obex_log_cleanup();

	return 0;
}
