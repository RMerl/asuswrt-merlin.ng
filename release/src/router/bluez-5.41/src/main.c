/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2000-2001  Qualcomm Incorporated
 *  Copyright (C) 2002-2003  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
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

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/signalfd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <glib.h>

#include <dbus/dbus.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"

#include "gdbus/gdbus.h"

#include "log.h"
#include "backtrace.h"

#include "lib/uuid.h"
#include "hcid.h"
#include "sdpd.h"
#include "adapter.h"
#include "device.h"
#include "dbus-common.h"
#include "agent.h"
#include "profile.h"
#include "systemd.h"
#define TYPEDEF_BOOL
#include <bcmnvram.h>
#include <bcmparams.h>
#include <shared.h>
#include <shutils.h>

#define BLUEZ_NAME "org.bluez"

#define DEFAULT_PAIRABLE_TIMEOUT       0 /* disabled */
#define DEFAULT_DISCOVERABLE_TIMEOUT 180 /* 3 minutes */

#define SHUTDOWN_GRACE_SECONDS 10

struct main_opts main_opts;
static GKeyFile *main_conf;

static enum {
	MPS_OFF,
	MPS_SINGLE,
	MPS_MULTIPLE,
} mps = MPS_OFF;

static const char * const supported_options[] = {
	"Name",
	"Class",
	"DiscoverableTimeout",
	"AlwaysPairable",
	"PairableTimeout",
	"AutoConnectTimeout",
	"DeviceID",
	"ReverseServiceDiscovery",
	"NameResolving",
	"DebugKeys",
	"ControllerMode",
	"MultiProfile",
};

GKeyFile *btd_get_main_conf(void)
{
	return main_conf;
}

static GKeyFile *load_config(const char *file)
{
	GError *err = NULL;
	GKeyFile *keyfile;

	keyfile = g_key_file_new();

	g_key_file_set_list_separator(keyfile, ',');

	if (!g_key_file_load_from_file(keyfile, file, 0, &err)) {
		if (!g_error_matches(err, G_FILE_ERROR, G_FILE_ERROR_NOENT))
			error("Parsing %s failed: %s", file, err->message);
		g_error_free(err);
		g_key_file_free(keyfile);
		return NULL;
	}

	return keyfile;
}

static void parse_did(const char *did)
{
	int result;
	uint16_t vendor, product, version , source;

	/* version and source are optional */
	version = 0x0000;
	source = 0x0002;

	result = sscanf(did, "bluetooth:%4hx:%4hx:%4hx",
					&vendor, &product, &version);
	if (result != EOF && result >= 2) {
		source = 0x0001;
		goto done;
	}

	result = sscanf(did, "usb:%4hx:%4hx:%4hx",
					&vendor, &product, &version);
	if (result != EOF && result >= 2)
		goto done;

	result = sscanf(did, "%4hx:%4hx:%4hx", &vendor, &product, &version);
	if (result == EOF || result < 2)
		return;

done:
	main_opts.did_source = source;
	main_opts.did_vendor = vendor;
	main_opts.did_product = product;
	main_opts.did_version = version;
}

static void check_config(GKeyFile *config)
{
	const char *valid_groups[] = { "General", "Policy", NULL };
	char **keys;
	int i;

	if (!config)
		return;

	keys = g_key_file_get_groups(config, NULL);

	for (i = 0; keys != NULL && keys[i] != NULL; i++) {
		const char **group;
		bool match = false;

		for (group = valid_groups; *group; group++) {
			if (g_str_equal(keys[i], *group)) {
				match = true;
				break;
			}
		}

		if (!match)
			warn("Unknown group %s in main.conf", keys[i]);
	}

	g_strfreev(keys);

	keys = g_key_file_get_keys(config, "General", NULL, NULL);

	for (i = 0; keys != NULL && keys[i] != NULL; i++) {
		bool found;
		unsigned int j;

		found = false;
		for (j = 0; j < G_N_ELEMENTS(supported_options); j++) {
			if (g_str_equal(keys[i], supported_options[j])) {
				found = true;
				break;
			}
		}

		if (!found)
			warn("Unknown key %s in main.conf", keys[i]);
	}

	g_strfreev(keys);
}

static int get_mode(const char *str)
{
	if (strcmp(str, "dual") == 0)
		return BT_MODE_DUAL;
	else if (strcmp(str, "bredr") == 0)
		return BT_MODE_BREDR;
	else if (strcmp(str, "le") == 0)
		return BT_MODE_LE;

	error("Unknown controller mode \"%s\"", str);

	return BT_MODE_DUAL;
}

static void parse_config(GKeyFile *config)
{
	GError *err = NULL;
	char *str;
	int val;
	gboolean boolean;

	if (!config)
		return;

	check_config(config);

	DBG("parsing main.conf");

	val = g_key_file_get_integer(config, "General",
						"DiscoverableTimeout", &err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
	} else {
		DBG("discovto=%d", val);
		main_opts.discovto = val;
	}

	val = g_key_file_get_integer(config, "General",
						"PairableTimeout", &err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
	} else {
		DBG("pairto=%d", val);
		main_opts.pairto = val;
	}

	val = g_key_file_get_integer(config, "General", "AutoConnectTimeout",
									&err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
	} else {
		DBG("auto_to=%d", val);
		main_opts.autoto = val;
	}

	str = g_key_file_get_string(config, "General", "Name", &err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
	} else {
		DBG("name=%s", str);
		g_free(main_opts.name);
		main_opts.name = str;
	}

	str = g_key_file_get_string(config, "General", "Class", &err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
	} else {
		DBG("class=%s", str);
		main_opts.class = strtol(str, NULL, 16);
		g_free(str);
	}

	str = g_key_file_get_string(config, "General", "DeviceID", &err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
	} else {
		DBG("deviceid=%s", str);
		parse_did(str);
		g_free(str);
	}

	boolean = g_key_file_get_boolean(config, "General",
						"ReverseServiceDiscovery", &err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
	} else
		main_opts.reverse_sdp = boolean;

	boolean = g_key_file_get_boolean(config, "General",
						"NameResolving", &err);
	if (err)
		g_clear_error(&err);
	else
		main_opts.name_resolv = boolean;

	boolean = g_key_file_get_boolean(config, "General",
						"DebugKeys", &err);
	if (err)
		g_clear_error(&err);
	else
		main_opts.debug_keys = boolean;

	str = g_key_file_get_string(config, "General", "ControllerMode", &err);
	if (err) {
		g_clear_error(&err);
	} else {
		DBG("ControllerMode=%s", str);
		main_opts.mode = get_mode(str);
		g_free(str);
	}

	str = g_key_file_get_string(config, "General", "MultiProfile", &err);
	if (err) {
		g_clear_error(&err);
	} else {
		DBG("MultiProfile=%s", str);

		if (!strcmp(str, "single"))
			mps = MPS_SINGLE;
		else if (!strcmp(str, "multiple"))
			mps = MPS_MULTIPLE;

		g_free(str);
	}

	boolean = g_key_file_get_boolean(config, "General",
						"FastConnectable", &err);
	if (err)
		g_clear_error(&err);
	else
		main_opts.fast_conn = boolean;
}

static void init_defaults(void)
{
	uint8_t major, minor;
	char *macp = NULL;
	unsigned char mac_binary[6];
	macp = get_2g_hwaddr();
	ether_atoe(macp, mac_binary);

	/* Default HCId settings */
	memset(&main_opts, 0, sizeof(main_opts));
#if defined(RTCONFIG_REALTEK)
	main_opts.name = g_strdup_printf("ASUS_%02X_%s", mac_binary[5], nvram_safe_get("productid"));
#elif defined(RTCONFIG_LANTIQ) || defined(RTCONFIG_ALPINE)
	main_opts.name = g_strdup_printf("ASUS_%02X", mac_binary[5]);
#elif defined(VZWAC1300)
	if (nvram_match("odmpid", "ASUSMESH-AC1300"))
		main_opts.name = g_strdup_printf("ASUS_%02X_MESH", mac_binary[5]);
	else
		main_opts.name = g_strdup_printf("VZW_%02X", mac_binary[5]);
#elif defined(RTCONFIG_SSID_AMAPS)
	main_opts.name = g_strdup_printf("ASUS_%02X_AMAPS", mac_binary[5]);
#else
	main_opts.name = g_strdup_printf("ASUS_%02X", mac_binary[5]);
#endif
	main_opts.class = 0x000000;
	main_opts.pairto = DEFAULT_PAIRABLE_TIMEOUT;
	main_opts.discovto = DEFAULT_DISCOVERABLE_TIMEOUT;
	main_opts.reverse_sdp = TRUE;
	main_opts.name_resolv = TRUE;
	main_opts.debug_keys = FALSE;
#if !defined(RTAX95Q) && !defined(XT8PRO) && !defined(BM68) && !defined(XT8_V2) && !defined(RTAXE95Q) && !defined(ET8PRO) && !defined(ET8_V2) && !defined(RTAX56_XD4) && !defined(XD4PRO) && !defined(ET12) && !defined(XT12) && !defined(BC109) && !defined(EBG19) && !defined(BC105) && !defined(EBG15)
	main_opts.mode = BT_MODE_LE;
#endif

	if (sscanf(VERSION, "%hhu.%hhu", &major, &minor) != 2)
		return;

	main_opts.did_source = 0x0002;		/* USB */
	main_opts.did_vendor = 0x1d6b;		/* Linux Foundation */
	main_opts.did_product = 0x0246;		/* BlueZ */
	main_opts.did_version = (major << 8 | minor);
}

static void log_handler(const gchar *log_domain, GLogLevelFlags log_level,
				const gchar *message, gpointer user_data)
{
	int priority;

	if (log_level & (G_LOG_LEVEL_ERROR |
				G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING))
		priority = 0x03;
	else
		priority = 0x06;

	btd_log(0xffff, priority, "GLib: %s", message);
	btd_backtrace(0xffff);
}

static GMainLoop *event_loop;

void btd_exit(void)
{
	g_main_loop_quit(event_loop);
}

static gboolean quit_eventloop(gpointer user_data)
{
	btd_exit();
	return FALSE;
}

static gboolean signal_handler(GIOChannel *channel, GIOCondition cond,
							gpointer user_data)
{
	static bool terminated = false;
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
		if (!terminated) {
			info("Terminating");
			g_timeout_add_seconds(SHUTDOWN_GRACE_SECONDS,
							quit_eventloop, NULL);

			sd_notify(0, "STATUS=Powering down");
			adapter_shutdown();
		}

		terminated = true;
		break;
	case SIGUSR2:
		__btd_toggle_debug();
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

static char *option_debug = NULL;
static char *option_plugin = NULL;
static char *option_noplugin = NULL;
static gboolean option_compat = FALSE;
static gboolean option_detach = TRUE;
static gboolean option_version = FALSE;
static gboolean option_experimental = FALSE;

static void free_options(void)
{
	g_free(option_debug);
	option_debug = NULL;

	g_free(option_plugin);
	option_plugin = NULL;

	g_free(option_noplugin);
	option_noplugin = NULL;
}

static void disconnect_dbus(void)
{
	DBusConnection *conn = btd_get_dbus_connection();

	if (!conn || !dbus_connection_get_is_connected(conn))
		return;

	g_dbus_detach_object_manager(conn);
	set_dbus_connection(NULL);

	dbus_connection_unref(conn);
}

static void disconnected_dbus(DBusConnection *conn, void *data)
{
	info("Disconnected from D-Bus. Exiting.");
	g_main_loop_quit(event_loop);
}

static int connect_dbus(void)
{
	DBusConnection *conn;
	DBusError err;

	dbus_error_init(&err);

	conn = g_dbus_setup_bus(DBUS_BUS_SYSTEM, BLUEZ_NAME, &err);
	if (!conn) {
		if (dbus_error_is_set(&err)) {
			g_printerr("D-Bus setup failed: %s\n", err.message);
			dbus_error_free(&err);
			return -EIO;
		}
		return -EALREADY;
	}

	set_dbus_connection(conn);

	g_dbus_set_disconnect_function(conn, disconnected_dbus, NULL, NULL);
	g_dbus_attach_object_manager(conn);

	return 0;
}

static gboolean watchdog_callback(gpointer user_data)
{
	sd_notify(0, "WATCHDOG=1");

	return TRUE;
}

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
				"Specify debug options to enable", "DEBUG" },
	{ "plugin", 'p', 0, G_OPTION_ARG_STRING, &option_plugin,
				"Specify plugins to load", "NAME,..," },
	{ "noplugin", 'P', 0, G_OPTION_ARG_STRING, &option_noplugin,
				"Specify plugins not to load", "NAME,..." },
	{ "compat", 'C', 0, G_OPTION_ARG_NONE, &option_compat,
				"Provide deprecated command line interfaces" },
	{ "experimental", 'E', 0, G_OPTION_ARG_NONE, &option_experimental,
				"Enable experimental interfaces" },
	{ "nodetach", 'n', G_OPTION_FLAG_REVERSE,
				G_OPTION_ARG_NONE, &option_detach,
				"Run with logging in foreground" },
	{ "version", 'v', 0, G_OPTION_ARG_NONE, &option_version,
				"Show version information and exit" },
	{ NULL },
};

int main(int argc, char *argv[])
{
	GOptionContext *context;
	GError *err = NULL;
	uint16_t sdp_mtu = 0;
	uint32_t sdp_flags = 0;
	int gdbus_flags = 0;
	guint signal, watchdog;
	const char *watchdog_usec;

	init_defaults();

	context = g_option_context_new(NULL);
	g_option_context_add_main_entries(context, options, NULL);

	if (g_option_context_parse(context, &argc, &argv, &err) == FALSE) {
		if (err != NULL) {
			g_printerr("%s\n", err->message);
			g_error_free(err);
		} else
			g_printerr("An unknown error occurred\n");
		exit(1);
	}

	g_option_context_free(context);

	if (option_version == TRUE) {
		printf("%s\n", VERSION);
		exit(0);
	}

	umask(0077);

	btd_backtrace_init();

	event_loop = g_main_loop_new(NULL, FALSE);

	signal = setup_signalfd();

	__btd_log_init(option_debug, option_detach);

	g_log_set_handler("GLib", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL |
							G_LOG_FLAG_RECURSION,
							log_handler, NULL);

	sd_notify(0, "STATUS=Starting up");

	main_conf = load_config(CONFIGDIR "/main.conf");

	parse_config(main_conf);

	if (connect_dbus() < 0) {
		error("Unable to get on D-Bus");
		exit(1);
	}

	if (option_experimental)
		gdbus_flags = G_DBUS_FLAG_ENABLE_EXPERIMENTAL;

	g_dbus_set_flags(gdbus_flags);

	if (adapter_init() < 0) {
		error("Adapter handling initialization failed");
		exit(1);
	}

	btd_device_init();
	btd_agent_init();
	btd_profile_init();

	if (main_opts.mode != BT_MODE_LE) {
		if (option_compat == TRUE)
			sdp_flags |= SDP_SERVER_COMPAT;

		start_sdp_server(sdp_mtu, sdp_flags);

		if (main_opts.did_source > 0)
			register_device_id(main_opts.did_source,
						main_opts.did_vendor,
						main_opts.did_product,
						main_opts.did_version);
	}

	if (mps != MPS_OFF)
		register_mps(mps == MPS_MULTIPLE);

	/* Loading plugins has to be done after D-Bus has been setup since
	 * the plugins might wanna expose some paths on the bus. However the
	 * best order of how to init various subsystems of the Bluetooth
	 * daemon needs to be re-worked. */
	plugin_init(option_plugin, option_noplugin);

	/* no need to keep parsed option in memory */
	free_options();

	rfkill_init();

	DBG("Entering main loop");

	sd_notify(0, "STATUS=Running");
	sd_notify(0, "READY=1");

	watchdog_usec = getenv("WATCHDOG_USEC");
	if (watchdog_usec) {
		unsigned int seconds;

		seconds = atoi(watchdog_usec) / (1000 * 1000);
		info("Watchdog timeout is %d seconds", seconds);

		watchdog = g_timeout_add_seconds_full(G_PRIORITY_HIGH,
							seconds / 2,
							watchdog_callback,
							NULL, NULL);
	} else
		watchdog = 0;

	g_main_loop_run(event_loop);

	sd_notify(0, "STATUS=Quitting");

	g_source_remove(signal);

	plugin_cleanup();

	btd_profile_cleanup();
	btd_agent_cleanup();
	btd_device_cleanup();

	adapter_cleanup();

	rfkill_exit();

	if (main_opts.mode != BT_MODE_LE)
		stop_sdp_server();

	g_main_loop_unref(event_loop);

	if (main_conf)
		g_key_file_free(main_conf);

	disconnect_dbus();

	info("Exit");

	if (watchdog > 0)
		g_source_remove(watchdog);

	__btd_log_cleanup();

	return 0;
}
