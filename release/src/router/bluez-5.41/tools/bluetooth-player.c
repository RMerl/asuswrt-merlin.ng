/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
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
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/signalfd.h>

#include <readline/readline.h>
#include <readline/history.h>
#include <glib.h>

#include "gdbus/gdbus.h"
#include "client/display.h"

/* String display constants */
#define COLORED_NEW	COLOR_GREEN "NEW" COLOR_OFF
#define COLORED_CHG	COLOR_YELLOW "CHG" COLOR_OFF
#define COLORED_DEL	COLOR_RED "DEL" COLOR_OFF

#define PROMPT_ON	COLOR_BLUE "[bluetooth]" COLOR_OFF "# "
#define PROMPT_OFF	"[bluetooth]# "

#define BLUEZ_MEDIA_PLAYER_INTERFACE "org.bluez.MediaPlayer1"
#define BLUEZ_MEDIA_FOLDER_INTERFACE "org.bluez.MediaFolder1"
#define BLUEZ_MEDIA_ITEM_INTERFACE "org.bluez.MediaItem1"

static GMainLoop *main_loop;
static DBusConnection *dbus_conn;
static GDBusProxy *default_player;
static GSList *players = NULL;
static GSList *folders = NULL;
static GSList *items = NULL;

static void connect_handler(DBusConnection *connection, void *user_data)
{
	rl_set_prompt(PROMPT_ON);
	printf("\r");
	rl_on_new_line();
	rl_redisplay();
}

static void disconnect_handler(DBusConnection *connection, void *user_data)
{
	rl_set_prompt(PROMPT_OFF);
	printf("\r");
	rl_on_new_line();
	rl_redisplay();
}

static void cmd_quit(int argc, char *argv[])
{
	g_main_loop_quit(main_loop);
}

static bool check_default_player(void)
{
	if (!default_player) {
		rl_printf("No default player available\n");
		return FALSE;
	}

	return TRUE;
}

static void play_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		rl_printf("Failed to play: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}

	rl_printf("Play successful\n");
}

static GDBusProxy *find_item(const char *path)
{
	GSList *l;

	for (l = items; l; l = g_slist_next(l)) {
		GDBusProxy *proxy = l->data;

		if (strcmp(path, g_dbus_proxy_get_path(proxy)) == 0)
			return proxy;
	}

	return NULL;
}

static void cmd_play_item(int argc, char *argv[])
{
	GDBusProxy *proxy;

	proxy = find_item(argv[1]);
	if (proxy == NULL) {
		rl_printf("Item %s not available\n", argv[1]);
		return;
	}

	if (g_dbus_proxy_method_call(proxy, "Play", NULL, play_reply,
							NULL, NULL) == FALSE) {
		rl_printf("Failed to play\n");
		return;
	}

	rl_printf("Attempting to play %s\n", argv[1]);
}

static void cmd_play(int argc, char *argv[])
{
	if (argc > 1)
		return cmd_play_item(argc, argv);

	if (!check_default_player())
		return;

	if (g_dbus_proxy_method_call(default_player, "Play", NULL, play_reply,
							NULL, NULL) == FALSE) {
		rl_printf("Failed to play\n");
		return;
	}

	rl_printf("Attempting to play\n");
}

static void pause_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		rl_printf("Failed to pause: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}

	rl_printf("Pause successful\n");
}

static void cmd_pause(int argc, char *argv[])
{
	if (!check_default_player())
		return;

	if (g_dbus_proxy_method_call(default_player, "Pause", NULL,
					pause_reply, NULL, NULL) == FALSE) {
		rl_printf("Failed to play\n");
		return;
	}

	rl_printf("Attempting to pause\n");
}

static void stop_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		rl_printf("Failed to stop: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}

	rl_printf("Stop successful\n");
}

static void cmd_stop(int argc, char *argv[])
{
	if (!check_default_player())
		return;

	if (g_dbus_proxy_method_call(default_player, "Stop", NULL, stop_reply,
							NULL, NULL) == FALSE) {
		rl_printf("Failed to stop\n");
		return;
	}

	rl_printf("Attempting to stop\n");
}

static void next_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		rl_printf("Failed to jump to next: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}

	rl_printf("Next successful\n");
}

static void cmd_next(int argc, char *argv[])
{
	if (!check_default_player())
		return;

	if (g_dbus_proxy_method_call(default_player, "Next", NULL, next_reply,
							NULL, NULL) == FALSE) {
		rl_printf("Failed to jump to next\n");
		return;
	}

	rl_printf("Attempting to jump to next\n");
}

static void previous_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		rl_printf("Failed to jump to previous: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}

	rl_printf("Previous successful\n");
}

static void cmd_previous(int argc, char *argv[])
{
	if (!check_default_player())
		return;

	if (g_dbus_proxy_method_call(default_player, "Previous", NULL,
					previous_reply, NULL, NULL) == FALSE) {
		rl_printf("Failed to jump to previous\n");
		return;
	}

	rl_printf("Attempting to jump to previous\n");
}

static void fast_forward_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		rl_printf("Failed to fast forward: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}

	rl_printf("FastForward successful\n");
}

static void cmd_fast_forward(int argc, char *argv[])
{
	if (!check_default_player())
		return;

	if (g_dbus_proxy_method_call(default_player, "FastForward", NULL,
				fast_forward_reply, NULL, NULL) == FALSE) {
		rl_printf("Failed to jump to previous\n");
		return;
	}

	rl_printf("Fast forward playback\n");
}

static void rewind_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		rl_printf("Failed to rewind: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}

	rl_printf("Rewind successful\n");
}

static void cmd_rewind(int argc, char *argv[])
{
	if (!check_default_player())
		return;

	if (g_dbus_proxy_method_call(default_player, "Rewind", NULL,
					rewind_reply, NULL, NULL) == FALSE) {
		rl_printf("Failed to rewind\n");
		return;
	}

	rl_printf("Rewind playback\n");
}

static void generic_callback(const DBusError *error, void *user_data)
{
	char *str = user_data;

	if (dbus_error_is_set(error))
		rl_printf("Failed to set %s: %s\n", str, error->name);
	else
		rl_printf("Changing %s succeeded\n", str);
}

static void cmd_equalizer(int argc, char *argv[])
{
	char *value;
	DBusMessageIter iter;

	if (!check_default_player())
		return;

	if (argc < 2) {
		rl_printf("Missing on/off argument\n");
		return;
	}

	if (!g_dbus_proxy_get_property(default_player, "Equalizer", &iter)) {
		rl_printf("Operation not supported\n");
		return;
	}

	value = g_strdup(argv[1]);

	if (g_dbus_proxy_set_property_basic(default_player, "Equalizer",
						DBUS_TYPE_STRING, &value,
						generic_callback, value,
						g_free) == FALSE) {
		rl_printf("Failed to setting equalizer\n");
		g_free(value);
		return;
	}

	rl_printf("Attempting to set equalizer\n");
}

static void cmd_repeat(int argc, char *argv[])
{
	char *value;
	DBusMessageIter iter;

	if (!check_default_player())
		return;

	if (argc < 2) {
		rl_printf("Missing mode argument\n");
		return;
	}

	if (!g_dbus_proxy_get_property(default_player, "Repeat", &iter)) {
		rl_printf("Operation not supported\n");
		return;
	}

	value = g_strdup(argv[1]);

	if (g_dbus_proxy_set_property_basic(default_player, "Repeat",
						DBUS_TYPE_STRING, &value,
						generic_callback, value,
						g_free) == FALSE) {
		rl_printf("Failed to set repeat\n");
		g_free(value);
		return;
	}

	rl_printf("Attempting to set repeat\n");
}

static void cmd_shuffle(int argc, char *argv[])
{
	char *value;
	DBusMessageIter iter;

	if (!check_default_player())
		return;

	if (argc < 2) {
		rl_printf("Missing mode argument\n");
		return;
	}

	if (!g_dbus_proxy_get_property(default_player, "Shuffle", &iter)) {
		rl_printf("Operation not supported\n");
		return;
	}

	value = g_strdup(argv[1]);

	if (g_dbus_proxy_set_property_basic(default_player, "Shuffle",
						DBUS_TYPE_STRING, &value,
						generic_callback, value,
						g_free) == FALSE) {
		rl_printf("Failed to set shuffle\n");
		g_free(value);
		return;
	}

	rl_printf("Attempting to set shuffle\n");
}

static void cmd_scan(int argc, char *argv[])
{
	char *value;
	DBusMessageIter iter;

	if (!check_default_player())
		return;

	if (argc < 2) {
		rl_printf("Missing mode argument\n");
		return;
	}

	if (!g_dbus_proxy_get_property(default_player, "Shuffle", &iter)) {
		rl_printf("Operation not supported\n");
		return;
	}

	value = g_strdup(argv[1]);

	if (g_dbus_proxy_set_property_basic(default_player, "Shuffle",
						DBUS_TYPE_STRING, &value,
						generic_callback, value,
						g_free) == FALSE) {
		rl_printf("Failed to set scan\n");
		g_free(value);
		return;
	}

	rl_printf("Attempting to set scan\n");
}

static char *proxy_description(GDBusProxy *proxy, const char *title,
						const char *description)
{
	const char *path;

	path = g_dbus_proxy_get_path(proxy);

	return g_strdup_printf("%s%s%s%s %s ",
					description ? "[" : "",
					description ? : "",
					description ? "] " : "",
					title, path);
}

static void print_player(GDBusProxy *proxy, const char *description)
{
	char *str;

	str = proxy_description(proxy, "Player", description);

	rl_printf("%s%s\n", str, default_player == proxy ? "[default]" : "");

	g_free(str);
}

static void cmd_list(int argc, char *arg[])
{
	GSList *l;

	for (l = players; l; l = g_slist_next(l)) {
		GDBusProxy *proxy = l->data;
		print_player(proxy, NULL);
	}
}

static GDBusProxy *find_player(const char *path)
{
	GSList *l;

	for (l = players; l; l = g_slist_next(l)) {
		GDBusProxy *proxy = l->data;

		if (strcmp(path, g_dbus_proxy_get_path(proxy)) == 0)
			return proxy;
	}

	return NULL;
}

static void print_iter(const char *label, const char *name,
						DBusMessageIter *iter)
{
	dbus_bool_t valbool;
	dbus_uint32_t valu32;
	dbus_uint16_t valu16;
	dbus_int16_t vals16;
	const char *valstr;
	DBusMessageIter subiter;

	if (iter == NULL) {
		rl_printf("%s%s is nil\n", label, name);
		return;
	}

	switch (dbus_message_iter_get_arg_type(iter)) {
	case DBUS_TYPE_INVALID:
		rl_printf("%s%s is invalid\n", label, name);
		break;
	case DBUS_TYPE_STRING:
	case DBUS_TYPE_OBJECT_PATH:
		dbus_message_iter_get_basic(iter, &valstr);
		rl_printf("%s%s: %s\n", label, name, valstr);
		break;
	case DBUS_TYPE_BOOLEAN:
		dbus_message_iter_get_basic(iter, &valbool);
		rl_printf("%s%s: %s\n", label, name,
					valbool == TRUE ? "yes" : "no");
		break;
	case DBUS_TYPE_UINT32:
		dbus_message_iter_get_basic(iter, &valu32);
		rl_printf("%s%s: 0x%06x\n", label, name, valu32);
		break;
	case DBUS_TYPE_UINT16:
		dbus_message_iter_get_basic(iter, &valu16);
		rl_printf("%s%s: 0x%04x\n", label, name, valu16);
		break;
	case DBUS_TYPE_INT16:
		dbus_message_iter_get_basic(iter, &vals16);
		rl_printf("%s%s: %d\n", label, name, vals16);
		break;
	case DBUS_TYPE_VARIANT:
		dbus_message_iter_recurse(iter, &subiter);
		print_iter(label, name, &subiter);
		break;
	case DBUS_TYPE_ARRAY:
		dbus_message_iter_recurse(iter, &subiter);
		while (dbus_message_iter_get_arg_type(&subiter) !=
							DBUS_TYPE_INVALID) {
			print_iter(label, name, &subiter);
			dbus_message_iter_next(&subiter);
		}
		break;
	case DBUS_TYPE_DICT_ENTRY:
		dbus_message_iter_recurse(iter, &subiter);
		dbus_message_iter_get_basic(&subiter, &valstr);
		dbus_message_iter_next(&subiter);
		print_iter(label, valstr, &subiter);
		break;
	default:
		rl_printf("%s%s has unsupported type\n", label, name);
		break;
	}
}

static void print_property(GDBusProxy *proxy, const char *name)
{
	DBusMessageIter iter;

	if (g_dbus_proxy_get_property(proxy, name, &iter) == FALSE)
		return;

	print_iter("\t", name, &iter);
}

static GDBusProxy *find_folder(const char *path)
{
	GSList *l;

	for (l = folders; l; l = g_slist_next(l)) {
		GDBusProxy *proxy = l->data;

		if (strcmp(path, g_dbus_proxy_get_path(proxy)) == 0)
			return proxy;
	}

	return NULL;
}

static void cmd_show_item(int argc, char *argv[])
{
	GDBusProxy *proxy;

	if (argc < 2) {
		rl_printf("Missing item address argument\n");
		return;
	}

	proxy = find_item(argv[1]);
	if (!proxy) {
		rl_printf("Item %s not available\n", argv[1]);
		return;
	}

	rl_printf("Item %s\n", g_dbus_proxy_get_path(proxy));

	print_property(proxy, "Player");
	print_property(proxy, "Name");
	print_property(proxy, "Type");
	print_property(proxy, "FolderType");
	print_property(proxy, "Playable");
	print_property(proxy, "Metadata");
}

static void cmd_show(int argc, char *argv[])
{
	GDBusProxy *proxy;
	GDBusProxy *folder;
	GDBusProxy *item;
	DBusMessageIter iter;
	const char *path;

	if (argc < 2) {
		if (check_default_player() == FALSE)
			return;

		proxy = default_player;
	} else {
		proxy = find_player(argv[1]);
		if (!proxy) {
			rl_printf("Player %s not available\n", argv[1]);
			return;
		}
	}

	rl_printf("Player %s\n", g_dbus_proxy_get_path(proxy));

	print_property(proxy, "Name");
	print_property(proxy, "Repeat");
	print_property(proxy, "Equalizer");
	print_property(proxy, "Shuffle");
	print_property(proxy, "Scan");
	print_property(proxy, "Status");
	print_property(proxy, "Position");
	print_property(proxy, "Track");

	folder = find_folder(g_dbus_proxy_get_path(proxy));
	if (folder == NULL)
		return;

	rl_printf("Folder %s\n", g_dbus_proxy_get_path(proxy));

	print_property(folder, "Name");
	print_property(folder, "NumberOfItems");

	if (!g_dbus_proxy_get_property(proxy, "Playlist", &iter))
		return;

	dbus_message_iter_get_basic(&iter, &path);

	item = find_item(path);
	if (item == NULL)
		return;

	rl_printf("Playlist %s\n", path);

	print_property(item, "Name");
}

static void cmd_select(int argc, char *argv[])
{
	GDBusProxy *proxy;

	if (argc < 2) {
		rl_printf("Missing player address argument\n");
		return;
	}

	proxy = find_player(argv[1]);
	if (proxy == NULL) {
		rl_printf("Player %s not available\n", argv[1]);
		return;
	}

	if (default_player == proxy)
		return;

	default_player = proxy,
	print_player(proxy, NULL);
}

static void change_folder_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		rl_printf("Failed to change folder: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}

	rl_printf("ChangeFolder successful\n");
}

static void change_folder_setup(DBusMessageIter *iter, void *user_data)
{
	const char *path = user_data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &path);
}

static void cmd_change_folder(int argc, char *argv[])
{
	GDBusProxy *proxy;

	if (argc < 2) {
		rl_printf("Missing item argument\n");
		return;
	}

	if (dbus_validate_path(argv[1], NULL) == FALSE) {
		rl_printf("Not a valid path\n");
		return;
	}

	if (check_default_player() == FALSE)
		return;

	proxy = find_folder(g_dbus_proxy_get_path(default_player));
	if (proxy == NULL) {
		rl_printf("Operation not supported\n");
		return;
	}

	if (g_dbus_proxy_method_call(proxy, "ChangeFolder", change_folder_setup,
				change_folder_reply, argv[1], NULL) == FALSE) {
		rl_printf("Failed to change current folder\n");
		return;
	}

	rl_printf("Attempting to change folder\n");
}

static void append_variant(DBusMessageIter *iter, int type, void *val)
{
	DBusMessageIter value;
	char sig[2] = { type, '\0' };

	dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, sig, &value);

	dbus_message_iter_append_basic(&value, type, val);

	dbus_message_iter_close_container(iter, &value);
}

static void dict_append_entry(DBusMessageIter *dict,
			const char *key, int type, void *val)
{
	DBusMessageIter entry;

	if (type == DBUS_TYPE_STRING) {
		const char *str = *((const char **) val);
		if (str == NULL)
			return;
	}

	dbus_message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY,
							NULL, &entry);

	dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);

	append_variant(&entry, type, val);

	dbus_message_iter_close_container(dict, &entry);
}

struct list_items_args {
	int start;
	int end;
};

static void list_items_setup(DBusMessageIter *iter, void *user_data)
{
	struct list_items_args *args = user_data;
	DBusMessageIter dict;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_STRING_AS_STRING
					DBUS_TYPE_VARIANT_AS_STRING
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
					&dict);

	if (args->start < 0)
		goto done;

	dict_append_entry(&dict, "Start", DBUS_TYPE_UINT32, &args->start);

	if (args->end < 0)
		goto done;

	dict_append_entry(&dict, "End", DBUS_TYPE_UINT32, &args->end);

done:
	dbus_message_iter_close_container(iter, &dict);
}

static void list_items_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		rl_printf("Failed to list items: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}

	rl_printf("ListItems successful\n");
}

static void cmd_list_items(int argc, char *argv[])
{
	GDBusProxy *proxy;
	struct list_items_args *args;

	if (check_default_player() == FALSE)
		return;

	proxy = find_folder(g_dbus_proxy_get_path(default_player));
	if (proxy == NULL) {
		rl_printf("Operation not supported\n");
		return;
	}

	args = g_new0(struct list_items_args, 1);
	args->start = -1;
	args->end = -1;

	if (argc < 2)
		goto done;

	errno = 0;
	args->start = strtol(argv[1], NULL, 10);
	if (errno != 0) {
		rl_printf("%s(%d)\n", strerror(errno), errno);
		g_free(args);
		return;
	}

	if (argc < 3)
		goto done;

	errno = 0;
	args->end = strtol(argv[2], NULL, 10);
	if (errno != 0) {
		rl_printf("%s(%d)\n", strerror(errno), errno);
		g_free(args);
		return;
	}

done:
	if (g_dbus_proxy_method_call(proxy, "ListItems", list_items_setup,
				list_items_reply, args, g_free) == FALSE) {
		rl_printf("Failed to change current folder\n");
		g_free(args);
		return;
	}

	rl_printf("Attempting to list items\n");
}

static void search_setup(DBusMessageIter *iter, void *user_data)
{
	char *string = user_data;
	DBusMessageIter dict;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &string);

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_STRING_AS_STRING
					DBUS_TYPE_VARIANT_AS_STRING
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
					&dict);

	dbus_message_iter_close_container(iter, &dict);
}

static void search_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		rl_printf("Failed to search: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}

	rl_printf("Search successful\n");
}

static void cmd_search(int argc, char *argv[])
{
	GDBusProxy *proxy;
	char *string;

	if (argc < 2) {
		rl_printf("Missing string argument\n");
		return;
	}

	if (check_default_player() == FALSE)
		return;

	proxy = find_folder(g_dbus_proxy_get_path(default_player));
	if (proxy == NULL) {
		rl_printf("Operation not supported\n");
		return;
	}

	string = g_strdup(argv[1]);

	if (g_dbus_proxy_method_call(proxy, "Search", search_setup,
				search_reply, string, g_free) == FALSE) {
		rl_printf("Failed to search\n");
		g_free(string);
		return;
	}

	rl_printf("Attempting to search\n");
}

static void add_to_nowplaying_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		rl_printf("Failed to queue: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}

	rl_printf("AddToNowPlaying successful\n");
}

static void cmd_queue(int argc, char *argv[])
{
	GDBusProxy *proxy;

	if (argc < 2) {
		rl_printf("Missing item address argument\n");
		return;
	}

	proxy = find_item(argv[1]);
	if (proxy == NULL) {
		rl_printf("Item %s not available\n", argv[1]);
		return;
	}

	if (g_dbus_proxy_method_call(proxy, "AddtoNowPlaying", NULL,
					add_to_nowplaying_reply, NULL,
					NULL) == FALSE) {
		rl_printf("Failed to play\n");
		return;
	}

	rl_printf("Attempting to queue %s\n", argv[1]);
}

static const struct {
	const char *cmd;
	const char *arg;
	void (*func) (int argc, char *argv[]);
	const char *desc;
} cmd_table[] = {
	{ "list",         NULL,       cmd_list, "List available players" },
	{ "show",         "[player]", cmd_show, "Player information" },
	{ "select",       "<player>", cmd_select, "Select default player" },
	{ "play",         "[item]",   cmd_play, "Start playback" },
	{ "pause",        NULL,       cmd_pause, "Pause playback" },
	{ "stop",         NULL,       cmd_stop, "Stop playback" },
	{ "next",         NULL,       cmd_next, "Jump to next item" },
	{ "previous",     NULL,       cmd_previous, "Jump to previous item" },
	{ "fast-forward", NULL,       cmd_fast_forward,
						"Fast forward playback" },
	{ "rewind",       NULL,       cmd_rewind, "Rewind playback" },
	{ "equalizer",    "<on/off>", cmd_equalizer,
						"Enable/Disable equalizer"},
	{ "repeat",       "<singletrack/alltrack/group/off>", cmd_repeat,
						"Set repeat mode"},
	{ "shuffle",      "<alltracks/group/off>", cmd_shuffle,
						"Set shuffle mode"},
	{ "scan",         "<alltracks/group/off>", cmd_scan,
						"Set scan mode"},
	{ "change-folder", "<item>",  cmd_change_folder,
						"Change current folder" },
	{ "list-items", "[start] [end]",  cmd_list_items,
					"List items of current folder" },
	{ "search",     "string",     cmd_search,
					"Search items containing string" },
	{ "queue",       "<item>",    cmd_queue, "Add item to playlist queue" },
	{ "show-item",   "<item>",    cmd_show_item, "Show item information" },
	{ "quit",         NULL,       cmd_quit, "Quit program" },
	{ "exit",         NULL,       cmd_quit },
	{ "help" },
	{}
};

static char *cmd_generator(const char *text, int state)
{
	static int index, len;
	const char *cmd;

	if (!state) {
		index = 0;
		len = strlen(text);
	}

	while ((cmd = cmd_table[index].cmd)) {
		index++;

		if (!strncmp(cmd, text, len))
			return strdup(cmd);
	}

	return NULL;
}

static char **cmd_completion(const char *text, int start, int end)
{
	char **matches = NULL;

	if (start == 0) {
		rl_completion_display_matches_hook = NULL;
		matches = rl_completion_matches(text, cmd_generator);
	}

	if (!matches)
		rl_attempted_completion_over = 1;

	return matches;
}

static void rl_handler(char *input)
{
	int argc;
	char **argv = NULL;
	int i;

	if (!input) {
		rl_insert_text("quit");
		rl_redisplay();
		rl_crlf();
		g_main_loop_quit(main_loop);
		return;
	}

	if (!strlen(input))
		goto done;

	g_strstrip(input);
	add_history(input);

	argv = g_strsplit(input, " ", -1);
	if (argv == NULL)
		goto done;

	for (argc = 0; argv[argc];)
		argc++;

	if (argc == 0)
		goto done;

	for (i = 0; cmd_table[i].cmd; i++) {
		if (strcmp(argv[0], cmd_table[i].cmd))
			continue;

		if (cmd_table[i].func) {
			cmd_table[i].func(argc, argv);
			goto done;
		}
	}

	if (strcmp(argv[0], "help")) {
		printf("Invalid command\n");
		goto done;
	}

	printf("Available commands:\n");

	for (i = 0; cmd_table[i].cmd; i++) {
		if (cmd_table[i].desc)
			printf("\t%s %s\t%s\n", cmd_table[i].cmd,
						cmd_table[i].arg ? : "    ",
						cmd_table[i].desc);
	}

done:
	g_strfreev(argv);
	free(input);
}

static gboolean option_version = FALSE;

static GOptionEntry options[] = {
	{ "version", 'v', 0, G_OPTION_ARG_NONE, &option_version,
				"Show version information and exit" },
	{ NULL },
};

static gboolean signal_handler(GIOChannel *channel, GIOCondition condition,
							gpointer user_data)
{
	static unsigned int __terminated = 0;
	struct signalfd_siginfo si;
	ssize_t result;
	int fd;

	if (condition & (G_IO_NVAL | G_IO_ERR | G_IO_HUP)) {
		g_main_loop_quit(main_loop);
		return FALSE;
	}

	fd = g_io_channel_unix_get_fd(channel);

	result = read(fd, &si, sizeof(si));
	if (result != sizeof(si))
		return FALSE;

	switch (si.ssi_signo) {
	case SIGINT:
		rl_replace_line("", 0);
		rl_crlf();
		rl_on_new_line();
		rl_redisplay();
		break;
	case SIGTERM:
		if (__terminated == 0) {
			rl_replace_line("", 0);
			rl_crlf();
			g_main_loop_quit(main_loop);
		}

		__terminated = 1;
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

static gboolean input_handler(GIOChannel *channel, GIOCondition condition,
							gpointer user_data)
{
	if (condition & (G_IO_HUP | G_IO_ERR | G_IO_NVAL)) {
		g_main_loop_quit(main_loop);
		return FALSE;
	}

	rl_callback_read_char();
	return TRUE;
}

static guint setup_standard_input(void)
{
	GIOChannel *channel;
	guint source;

	channel = g_io_channel_unix_new(fileno(stdin));

	source = g_io_add_watch(channel,
				G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				input_handler, NULL);

	g_io_channel_unref(channel);

	return source;
}

static void player_added(GDBusProxy *proxy)
{
	players = g_slist_append(players, proxy);

	if (default_player == NULL)
		default_player = proxy;

	print_player(proxy, COLORED_NEW);
}

static void print_folder(GDBusProxy *proxy, const char *description)
{
	const char *path;

	path = g_dbus_proxy_get_path(proxy);

	rl_printf("%s%s%sFolder %s\n", description ? "[" : "",
					description ? : "",
					description ? "] " : "",
					path);
}

static void folder_added(GDBusProxy *proxy)
{
	folders = g_slist_append(folders, proxy);

	print_folder(proxy, COLORED_NEW);
}

static void print_item(GDBusProxy *proxy, const char *description)
{
	const char *path, *name;
	DBusMessageIter iter;

	path = g_dbus_proxy_get_path(proxy);

	if (g_dbus_proxy_get_property(proxy, "Name", &iter))
		dbus_message_iter_get_basic(&iter, &name);
	else
		name = "<unknown>";

	rl_printf("%s%s%sItem %s %s\n", description ? "[" : "",
					description ? : "",
					description ? "] " : "",
					path, name);
}

static void item_added(GDBusProxy *proxy)
{
	items = g_slist_append(items, proxy);

	print_item(proxy, COLORED_NEW);
}

static void proxy_added(GDBusProxy *proxy, void *user_data)
{
	const char *interface;

	interface = g_dbus_proxy_get_interface(proxy);

	if (!strcmp(interface, BLUEZ_MEDIA_PLAYER_INTERFACE))
		player_added(proxy);
	else if (!strcmp(interface, BLUEZ_MEDIA_FOLDER_INTERFACE))
		folder_added(proxy);
	else if (!strcmp(interface, BLUEZ_MEDIA_ITEM_INTERFACE))
		item_added(proxy);
}

static void player_removed(GDBusProxy *proxy)
{
	print_player(proxy, COLORED_DEL);

	if (default_player == proxy)
		default_player = NULL;

	players = g_slist_remove(players, proxy);
}

static void folder_removed(GDBusProxy *proxy)
{
	folders = g_slist_remove(folders, proxy);

	print_folder(proxy, COLORED_DEL);
}

static void item_removed(GDBusProxy *proxy)
{
	items = g_slist_remove(items, proxy);

	print_item(proxy, COLORED_DEL);
}

static void proxy_removed(GDBusProxy *proxy, void *user_data)
{
	const char *interface;

	interface = g_dbus_proxy_get_interface(proxy);

	if (!strcmp(interface, BLUEZ_MEDIA_PLAYER_INTERFACE))
		player_removed(proxy);
	if (!strcmp(interface, BLUEZ_MEDIA_FOLDER_INTERFACE))
		folder_removed(proxy);
	if (!strcmp(interface, BLUEZ_MEDIA_ITEM_INTERFACE))
		item_removed(proxy);
}

static void player_property_changed(GDBusProxy *proxy, const char *name,
						DBusMessageIter *iter)
{
	char *str;

	str = proxy_description(proxy, "Player", COLORED_CHG);
	print_iter(str, name, iter);
	g_free(str);
}

static void folder_property_changed(GDBusProxy *proxy, const char *name,
						DBusMessageIter *iter)
{
	char *str;

	str = proxy_description(proxy, "Folder", COLORED_CHG);
	print_iter(str, name, iter);
	g_free(str);
}

static void item_property_changed(GDBusProxy *proxy, const char *name,
						DBusMessageIter *iter)
{
	char *str;

	str = proxy_description(proxy, "Item", COLORED_CHG);
	print_iter(str, name, iter);
	g_free(str);
}

static void property_changed(GDBusProxy *proxy, const char *name,
					DBusMessageIter *iter, void *user_data)
{
	const char *interface;

	interface = g_dbus_proxy_get_interface(proxy);

	if (!strcmp(interface, BLUEZ_MEDIA_PLAYER_INTERFACE))
		player_property_changed(proxy, name, iter);
	else if (!strcmp(interface, BLUEZ_MEDIA_FOLDER_INTERFACE))
		folder_property_changed(proxy, name, iter);
	else if (!strcmp(interface, BLUEZ_MEDIA_ITEM_INTERFACE))
		item_property_changed(proxy, name, iter);
}

int main(int argc, char *argv[])
{
	GOptionContext *context;
	GError *error = NULL;
	GDBusClient *client;
	guint signal, input;

	context = g_option_context_new(NULL);
	g_option_context_add_main_entries(context, options, NULL);

	if (g_option_context_parse(context, &argc, &argv, &error) == FALSE) {
		if (error != NULL) {
			g_printerr("%s\n", error->message);
			g_error_free(error);
		} else
			g_printerr("An unknown error occurred\n");
		exit(1);
	}

	g_option_context_free(context);

	if (option_version == TRUE) {
		printf("%s\n", VERSION);
		exit(0);
	}

	main_loop = g_main_loop_new(NULL, FALSE);
	dbus_conn = g_dbus_setup_bus(DBUS_BUS_SYSTEM, NULL, NULL);

	rl_attempted_completion_function = cmd_completion;

	rl_erase_empty_line = 1;
	rl_callback_handler_install(NULL, rl_handler);

	rl_set_prompt(PROMPT_OFF);
	rl_redisplay();

	input = setup_standard_input();
	signal = setup_signalfd();
	client = g_dbus_client_new(dbus_conn, "org.bluez", "/org/bluez");

	g_dbus_client_set_connect_watch(client, connect_handler, NULL);
	g_dbus_client_set_disconnect_watch(client, disconnect_handler, NULL);

	g_dbus_client_set_proxy_handlers(client, proxy_added, proxy_removed,
							property_changed, NULL);

	g_main_loop_run(main_loop);

	g_dbus_client_unref(client);
	g_source_remove(signal);
	g_source_remove(input);

	rl_message("");
	rl_callback_handler_remove();

	dbus_connection_unref(dbus_conn);
	g_main_loop_unref(main_loop);

	return 0;
}
