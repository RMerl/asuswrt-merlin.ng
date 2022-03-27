/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2003-2010  Marcel Holtmann <marcel@holtmann.org>
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
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <sys/socket.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"

#include "gdbus/gdbus.h"

#include "cups.h"

struct cups_device {
	char *bdaddr;
	char *name;
	char *id;
};

static GSList *device_list = NULL;
static GMainLoop *loop = NULL;
static DBusConnection *conn = NULL;
static gboolean doing_disco = FALSE;

#define ATTRID_1284ID 0x0300

struct context_data {
	gboolean found;
	char *id;
};

static void element_start(GMarkupParseContext *context,
				const char *element_name,
				const char **attribute_names,
				const char **attribute_values,
				gpointer user_data, GError **err)
{
	struct context_data *ctx_data = user_data;

	if (!strcmp(element_name, "record"))
		return;

	if (!strcmp(element_name, "attribute")) {
		int i;
		for (i = 0; attribute_names[i]; i++) {
			if (strcmp(attribute_names[i], "id") != 0)
				continue;
			if (strtol(attribute_values[i], 0, 0) == ATTRID_1284ID)
				ctx_data->found = TRUE;
			break;
		}
		return;
	}

	if (ctx_data->found  && !strcmp(element_name, "text")) {
		int i;
		for (i = 0; attribute_names[i]; i++) {
			if (!strcmp(attribute_names[i], "value")) {
				ctx_data->id = g_strdup(attribute_values[i] + 2);
				ctx_data->found = FALSE;
			}
		}
	}
}

static GMarkupParser parser = {
	element_start, NULL, NULL, NULL, NULL
};

static char *sdp_xml_parse_record(const char *data)
{
	GMarkupParseContext *ctx;
	struct context_data ctx_data;
	int size;

	size = strlen(data);
	ctx_data.found = FALSE;
	ctx_data.id = NULL;
	ctx = g_markup_parse_context_new(&parser, 0, &ctx_data, NULL);

	if (g_markup_parse_context_parse(ctx, data, size, NULL) == FALSE) {
		g_markup_parse_context_free(ctx);
		g_free(ctx_data.id);
		return NULL;
	}

	g_markup_parse_context_free(ctx);

	return ctx_data.id;
}

static char *device_get_ieee1284_id(const char *adapter, const char *device)
{
	DBusMessage *message, *reply;
	DBusMessageIter iter, reply_iter;
	DBusMessageIter reply_iter_entry;
	const char *hcr_print = "00001126-0000-1000-8000-00805f9b34fb";
	const char *xml;
	char *id = NULL;

	/* Look for the service handle of the HCRP service */
	message = dbus_message_new_method_call("org.bluez", device,
						"org.bluez.Device1",
						"DiscoverServices");
	dbus_message_iter_init_append(message, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &hcr_print);

	reply = dbus_connection_send_with_reply_and_block(conn,
							message, -1, NULL);

	dbus_message_unref(message);

	if (!reply)
		return NULL;

	dbus_message_iter_init(reply, &reply_iter);

	if (dbus_message_iter_get_arg_type(&reply_iter) != DBUS_TYPE_ARRAY) {
		dbus_message_unref(reply);
		return NULL;
	}

	dbus_message_iter_recurse(&reply_iter, &reply_iter_entry);

	/* Hopefully we only get one handle, or take a punt */
	while (dbus_message_iter_get_arg_type(&reply_iter_entry) ==
							DBUS_TYPE_DICT_ENTRY) {
		guint32 key;
		DBusMessageIter dict_entry;

		dbus_message_iter_recurse(&reply_iter_entry, &dict_entry);

		/* Key ? */
		dbus_message_iter_get_basic(&dict_entry, &key);
		if (!key) {
			dbus_message_iter_next(&reply_iter_entry);
			continue;
		}

		/* Try to get the value */
		if (!dbus_message_iter_next(&dict_entry)) {
			dbus_message_iter_next(&reply_iter_entry);
			continue;
		}

		dbus_message_iter_get_basic(&dict_entry, &xml);

		id = sdp_xml_parse_record(xml);
		if (id != NULL)
			break;
		dbus_message_iter_next(&reply_iter_entry);
	}

	dbus_message_unref(reply);

	return id;
}

static void print_printer_details(const char *name, const char *bdaddr,
								const char *id)
{
	char *uri, *escaped;

	escaped = g_strdelimit(g_strdup(name), "\"", '\'');
	uri = g_strdup_printf("bluetooth://%c%c%c%c%c%c%c%c%c%c%c%c",
				bdaddr[0], bdaddr[1],
				bdaddr[3], bdaddr[4],
				bdaddr[6], bdaddr[7],
				bdaddr[9], bdaddr[10],
				bdaddr[12], bdaddr[13],
				bdaddr[15], bdaddr[16]);
	printf("direct %s \"%s\" \"%s (Bluetooth)\"", uri, escaped, escaped);
	if (id != NULL)
		printf(" \"%s\"\n", id);
	else
		printf("\n");
	g_free(escaped);
	g_free(uri);
}

static void add_device_to_list(const char *name, const char *bdaddr,
								const char *id)
{
	struct cups_device *device;
	GSList *l;

	/* Look for the device in the list */
	for (l = device_list; l != NULL; l = l->next) {
		device = (struct cups_device *) l->data;

		if (strcmp(device->bdaddr, bdaddr) == 0) {
			if (device->name != name) {
				g_free(device->name);
				device->name = g_strdup(name);
			}
			g_free(device->id);
			device->id = g_strdup(id);
			return;
		}
	}

	/* Or add it to the list if it's not there */
	device = g_new0(struct cups_device, 1);
	device->bdaddr = g_strdup(bdaddr);
	device->name = g_strdup(name);
	device->id = g_strdup(id);

	device_list = g_slist_prepend(device_list, device);
	print_printer_details(device->name, device->bdaddr, device->id);
}

static gboolean parse_device_properties(DBusMessageIter *reply_iter,
						char **name, char **bdaddr)
{
	guint32 class = 0;
	DBusMessageIter reply_iter_entry;

	if (dbus_message_iter_get_arg_type(reply_iter) != DBUS_TYPE_ARRAY)
		return FALSE;

	dbus_message_iter_recurse(reply_iter, &reply_iter_entry);

	while (dbus_message_iter_get_arg_type(&reply_iter_entry) ==
							DBUS_TYPE_DICT_ENTRY) {
		const char *key;
		DBusMessageIter dict_entry, iter_dict_val;

		dbus_message_iter_recurse(&reply_iter_entry, &dict_entry);

		/* Key == Class ? */
		dbus_message_iter_get_basic(&dict_entry, &key);
		if (!key) {
			dbus_message_iter_next(&reply_iter_entry);
			continue;
		}

		if (strcmp(key, "Class") != 0 &&
				strcmp(key, "Alias") != 0 &&
				strcmp(key, "Address") != 0) {
			dbus_message_iter_next(&reply_iter_entry);
			continue;
		}

		/* Try to get the value */
		if (!dbus_message_iter_next(&dict_entry)) {
			dbus_message_iter_next(&reply_iter_entry);
			continue;
		}
		dbus_message_iter_recurse(&dict_entry, &iter_dict_val);
		if (strcmp(key, "Class") == 0) {
			dbus_message_iter_get_basic(&iter_dict_val, &class);
		} else {
			const char *value;
			dbus_message_iter_get_basic(&iter_dict_val, &value);
			if (strcmp(key, "Alias") == 0) {
				*name = g_strdup(value);
			} else if (bdaddr) {
				*bdaddr = g_strdup(value);
			}
		}
		dbus_message_iter_next(&reply_iter_entry);
	}

	if (class == 0)
		return FALSE;
	if (((class & 0x1f00) >> 8) == 0x06 && (class & 0x80))
		return TRUE;

	return FALSE;
}

static gboolean device_is_printer(const char *adapter, const char *device_path, char **name, char **bdaddr)
{
	DBusMessage *message, *reply;
	DBusMessageIter reply_iter;
	gboolean retval;

	message = dbus_message_new_method_call("org.bluez", device_path,
							"org.bluez.Device1",
							"GetProperties");

	reply = dbus_connection_send_with_reply_and_block(conn,
							message, -1, NULL);

	dbus_message_unref(message);

	if (!reply)
		return FALSE;

	dbus_message_iter_init(reply, &reply_iter);

	retval = parse_device_properties(&reply_iter, name, bdaddr);

	dbus_message_unref(reply);

	return retval;
}

static void remote_device_found(const char *adapter, const char *bdaddr,
							const char *name)
{
	DBusMessage *message, *reply;
	DBusMessageIter iter;
	char *object_path = NULL;
	char *id;

	assert(adapter != NULL);

	message = dbus_message_new_method_call("org.bluez", adapter,
							"org.bluez.Adapter1",
							"FindDevice");
	dbus_message_iter_init_append(message, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &bdaddr);

	reply = dbus_connection_send_with_reply_and_block(conn,
							message, -1, NULL);

	dbus_message_unref(message);

	if (!reply) {
		message = dbus_message_new_method_call("org.bluez", adapter,
							"org.bluez.Adapter1",
							"CreateDevice");
		dbus_message_iter_init_append(message, &iter);
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &bdaddr);

		reply = dbus_connection_send_with_reply_and_block(conn,
							message, -1, NULL);

		dbus_message_unref(message);

		if (!reply)
			return;
	}

	if (dbus_message_get_args(reply, NULL,
					DBUS_TYPE_OBJECT_PATH, &object_path,
					DBUS_TYPE_INVALID) == FALSE) {
		dbus_message_unref(reply);
		return;
	}

	id = device_get_ieee1284_id(adapter, object_path);
	add_device_to_list(name, bdaddr, id);
	g_free(id);

	dbus_message_unref(reply);
}

static void discovery_completed(void)
{
	g_slist_free(device_list);
	device_list = NULL;

	g_main_loop_quit(loop);
}

static void remote_device_disappeared(const char *bdaddr)
{
	GSList *l;

	for (l = device_list; l != NULL; l = l->next) {
		struct cups_device *device = l->data;

		if (strcmp(device->bdaddr, bdaddr) == 0) {
			g_free(device->name);
			g_free(device->bdaddr);
			g_free(device);
			device_list = g_slist_delete_link(device_list, l);
			return;
		}
	}
}

static gboolean list_known_printers(const char *adapter)
{
	DBusMessageIter reply_iter, iter_array;
	DBusError error;
	DBusMessage *message, *reply;

	message = dbus_message_new_method_call("org.bluez", adapter,
						"org.bluez.Adapter1",
						"ListDevices");
	if (message == NULL)
		return FALSE;

	dbus_error_init(&error);
	reply = dbus_connection_send_with_reply_and_block(conn, message,
								-1, &error);

	dbus_message_unref(message);

	if (dbus_error_is_set(&error)) {
		dbus_error_free(&error);
		return FALSE;
	}

	dbus_message_iter_init(reply, &reply_iter);
	if (dbus_message_iter_get_arg_type(&reply_iter) != DBUS_TYPE_ARRAY) {
		dbus_message_unref(reply);
		return FALSE;
	}

	dbus_message_iter_recurse(&reply_iter, &iter_array);
	while (dbus_message_iter_get_arg_type(&iter_array) ==
						DBUS_TYPE_OBJECT_PATH) {
		const char *object_path;
		char *name = NULL;
		char *bdaddr = NULL;

		dbus_message_iter_get_basic(&iter_array, &object_path);
		if (device_is_printer(adapter, object_path, &name, &bdaddr)) {
			char *id;

			id = device_get_ieee1284_id(adapter, object_path);
			add_device_to_list(name, bdaddr, id);
			g_free(id);
		}
		g_free(name);
		g_free(bdaddr);
		dbus_message_iter_next(&iter_array);
	}

	dbus_message_unref(reply);

	return FALSE;
}

static DBusHandlerResult filter_func(DBusConnection *connection,
					DBusMessage *message, void *user_data)
{
	if (dbus_message_is_signal(message, "org.bluez.Adapter1",
						"DeviceFound")) {
		const char *adapter, *bdaddr;
		char *name;
		DBusMessageIter iter;

		dbus_message_iter_init(message, &iter);
		dbus_message_iter_get_basic(&iter, &bdaddr);
		dbus_message_iter_next(&iter);

		adapter = dbus_message_get_path(message);
		if (parse_device_properties(&iter, &name, NULL))
			remote_device_found(adapter, bdaddr, name);
		g_free (name);
	} else if (dbus_message_is_signal(message, "org.bluez.Adapter1",
						"DeviceDisappeared")) {
		const char *bdaddr;

		dbus_message_get_args(message, NULL,
					DBUS_TYPE_STRING, &bdaddr,
					DBUS_TYPE_INVALID);
		remote_device_disappeared(bdaddr);
	} else if (dbus_message_is_signal(message, "org.bluez.Adapter1",
						"PropertyChanged")) {
		DBusMessageIter iter, value_iter;
		const char *name;
		gboolean discovering;

		dbus_message_iter_init(message, &iter);
		dbus_message_iter_get_basic(&iter, &name);
		if (name == NULL || strcmp(name, "Discovering") != 0)
			return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter, &value_iter);
		dbus_message_iter_get_basic(&value_iter, &discovering);

		if (discovering == FALSE && doing_disco) {
			doing_disco = FALSE;
			discovery_completed();
		}
	}

	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static gboolean list_printers(void)
{
	/* 1. Connect to the bus
	 * 2. Get the manager
	 * 3. Get the default adapter
	 * 4. Get a list of devices
	 * 5. Get the class of each device
	 * 6. Print the details from each printer device
	 */
	DBusError error;
	dbus_bool_t hcid_exists;
	DBusMessage *reply, *message;
	DBusMessageIter reply_iter;
	char *adapter, *match;

	conn = g_dbus_setup_bus(DBUS_BUS_SYSTEM, NULL, NULL);
	if (conn == NULL)
		return TRUE;

	dbus_error_init(&error);
	hcid_exists = dbus_bus_name_has_owner(conn, "org.bluez", &error);
	if (dbus_error_is_set(&error)) {
		dbus_error_free(&error);
		return TRUE;
	}

	if (!hcid_exists)
		return TRUE;

	/* Get the default adapter */
	message = dbus_message_new_method_call("org.bluez", "/",
						"org.bluez.Manager",
						"DefaultAdapter");
	if (message == NULL) {
		dbus_connection_unref(conn);
		return FALSE;
	}

	reply = dbus_connection_send_with_reply_and_block(conn,
							message, -1, &error);

	dbus_message_unref(message);

	if (dbus_error_is_set(&error)) {
		dbus_error_free(&error);
		dbus_connection_unref(conn);
		/* No adapter */
		return TRUE;
	}

	dbus_message_iter_init(reply, &reply_iter);
	if (dbus_message_iter_get_arg_type(&reply_iter) !=
						DBUS_TYPE_OBJECT_PATH) {
		dbus_message_unref(reply);
		dbus_connection_unref(conn);
		return FALSE;
	}

	dbus_message_iter_get_basic(&reply_iter, &adapter);
	adapter = g_strdup(adapter);
	dbus_message_unref(reply);

	if (!dbus_connection_add_filter(conn, filter_func, adapter, g_free)) {
		g_free(adapter);
		dbus_connection_unref(conn);
		return FALSE;
	}

#define MATCH_FORMAT				\
	"type='signal',"			\
	"interface='org.bluez.Adapter1',"	\
	"sender='org.bluez',"			\
	"path='%s'"

	match = g_strdup_printf(MATCH_FORMAT, adapter);
	dbus_bus_add_match(conn, match, &error);
	g_free(match);

	/* Add the the recent devices */
	list_known_printers(adapter);

	doing_disco = TRUE;
	message = dbus_message_new_method_call("org.bluez", adapter,
					"org.bluez.Adapter1",
					"StartDiscovery");

	if (!dbus_connection_send_with_reply(conn, message, NULL, -1)) {
		dbus_message_unref(message);
		dbus_connection_unref(conn);
		g_free(adapter);
		return FALSE;
	}
	dbus_message_unref(message);

	loop = g_main_loop_new(NULL, TRUE);
	g_main_loop_run(loop);

	g_free(adapter);
	dbus_connection_unref(conn);

	return TRUE;
}

static gboolean print_ieee1284(const char *bdaddr)
{
	DBusMessage *message, *reply, *adapter_reply;
	DBusMessageIter iter;
	char *object_path = NULL;
	char *adapter;
	char *id;

	adapter_reply = NULL;

	conn = g_dbus_setup_bus(DBUS_BUS_SYSTEM, NULL, NULL);
	if (conn == NULL)
		return FALSE;

	message = dbus_message_new_method_call("org.bluez", "/",
			"org.bluez.Manager",
			"DefaultAdapter");

	adapter_reply = dbus_connection_send_with_reply_and_block(conn,
			message, -1, NULL);

	dbus_message_unref(message);

	if (!adapter_reply)
		return FALSE;

	if (dbus_message_get_args(adapter_reply, NULL,
			DBUS_TYPE_OBJECT_PATH, &adapter,
			DBUS_TYPE_INVALID) == FALSE) {
		dbus_message_unref(adapter_reply);
		return FALSE;
	}

	message = dbus_message_new_method_call("org.bluez", adapter,
			"org.bluez.Adapter1",
			"FindDevice");
	dbus_message_iter_init_append(message, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &bdaddr);

	if (adapter_reply != NULL)
		dbus_message_unref(adapter_reply);

	reply = dbus_connection_send_with_reply_and_block(conn,
			message, -1, NULL);

	dbus_message_unref(message);

	if (!reply) {
		message = dbus_message_new_method_call("org.bluez", adapter,
				"org.bluez.Adapter1",
				"CreateDevice");
		dbus_message_iter_init_append(message, &iter);
		dbus_message_iter_append_basic(&iter,
				DBUS_TYPE_STRING, &bdaddr);

		reply = dbus_connection_send_with_reply_and_block(conn,
				message, -1, NULL);

		dbus_message_unref(message);

		if (!reply)
			return FALSE;
	}

	if (dbus_message_get_args(reply, NULL,
					DBUS_TYPE_OBJECT_PATH, &object_path,
					DBUS_TYPE_INVALID) == FALSE) {
		dbus_message_unref(reply);
		return FALSE;
	}

	id = device_get_ieee1284_id(adapter, object_path);
	if (id == NULL) {
		dbus_message_unref(reply);
		return FALSE;
	}
	printf("%s", id);
	g_free(id);

	dbus_message_unref(reply);

	return TRUE;
}

/*
 *  Usage: printer-uri job-id user title copies options [file]
 *
 */

int main(int argc, char *argv[])
{
	sdp_session_t *sdp;
	bdaddr_t bdaddr;
	unsigned short ctrl_psm, data_psm;
	uint8_t channel, b[6];
	char *ptr, str[3], device[18], service[12];
	const char *uri, *cups_class;
	int i, err, fd, copies, proto;

	/* Make sure status messages are not buffered */
	setbuf(stderr, NULL);

	/* Make sure output is not buffered */
	setbuf(stdout, NULL);

	/* Ignore SIGPIPE signals */
#ifdef HAVE_SIGSET
	sigset(SIGPIPE, SIG_IGN);
#elif defined(HAVE_SIGACTION)
	memset(&action, 0, sizeof(action));
	action.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &action, NULL);
#else
	signal(SIGPIPE, SIG_IGN);
#endif /* HAVE_SIGSET */

	if (argc == 1) {
		if (list_printers() == TRUE)
			return CUPS_BACKEND_OK;
		else
			return CUPS_BACKEND_FAILED;
	} else if (argc == 3 && strcmp(argv[1], "--get-deviceid") == 0) {
		if (bachk(argv[2]) < 0) {
			fprintf(stderr, "Invalid Bluetooth address '%s'\n",
					argv[2]);
			return CUPS_BACKEND_FAILED;
		}
		if (print_ieee1284(argv[2]) == FALSE)
			return CUPS_BACKEND_FAILED;
		return CUPS_BACKEND_OK;
	}

	if (argc < 6 || argc > 7) {
		fprintf(stderr, "Usage: bluetooth job-id user title copies"
				" options [file]\n");
		fprintf(stderr, "       bluetooth --get-deviceid [bdaddr]\n");
		return CUPS_BACKEND_FAILED;
	}

	if (argc == 6) {
		fd = 0;
		copies = 1;
	} else {
		if ((fd = open(argv[6], O_RDONLY)) < 0) {
			perror("ERROR: Unable to open print file");
			return CUPS_BACKEND_FAILED;
		}
		copies = atoi(argv[4]);
	}

	uri = getenv("DEVICE_URI");
	if (!uri)
		uri = argv[0];

	if (strncasecmp(uri, "bluetooth://", 12)) {
		fprintf(stderr, "ERROR: No device URI found\n");
		return CUPS_BACKEND_FAILED;
	}

	ptr = argv[0] + 12;
	for (i = 0; i < 6; i++) {
		strncpy(str, ptr, 2);
		b[i] = (uint8_t) strtol(str, NULL, 16);
		ptr += 2;
	}
	sprintf(device, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
			b[0], b[1], b[2], b[3], b[4], b[5]);

	str2ba(device, &bdaddr);

	ptr = strchr(ptr, '/');
	if (ptr) {
		strncpy(service, ptr + 1, 12);

		if (!strncasecmp(ptr + 1, "spp", 3))
			proto = 1;
		else if (!strncasecmp(ptr + 1, "hcrp", 4))
			proto = 2;
		else
			proto = 0;
	} else {
		strcpy(service, "auto");
		proto = 0;
	}

	cups_class = getenv("CLASS");

	fprintf(stderr,
		"DEBUG: %s device %s service %s fd %d copies %d class %s\n",
			argv[0], device, service, fd, copies,
			cups_class ? cups_class : "(none)");

	fputs("STATE: +connecting-to-device\n", stderr);

service_search:
	sdp = sdp_connect(BDADDR_ANY, &bdaddr, SDP_RETRY_IF_BUSY);
	if (!sdp) {
		fprintf(stderr, "ERROR: Can't open Bluetooth connection\n");
		return CUPS_BACKEND_FAILED;
	}

	switch (proto) {
	case 1:
		err = sdp_search_spp(sdp, &channel);
		break;
	case 2:
		err = sdp_search_hcrp(sdp, &ctrl_psm, &data_psm);
		break;
	default:
		proto = 2;
		err = sdp_search_hcrp(sdp, &ctrl_psm, &data_psm);
		if (err) {
			proto = 1;
			err = sdp_search_spp(sdp, &channel);
		}
		break;
	}

	sdp_close(sdp);

	if (err) {
		if (cups_class) {
			fputs("INFO: Unable to contact printer, queuing on "
					"next printer in class...\n", stderr);
			sleep(5);
			return CUPS_BACKEND_FAILED;
		}
		sleep(20);
		fprintf(stderr, "ERROR: Can't get service information\n");
		goto service_search;
	}

connect:
	switch (proto) {
	case 1:
		err = spp_print(BDADDR_ANY, &bdaddr, channel,
						fd, copies, cups_class);
		break;
	case 2:
		err = hcrp_print(BDADDR_ANY, &bdaddr, ctrl_psm, data_psm,
						fd, copies, cups_class);
		break;
	default:
		err = CUPS_BACKEND_FAILED;
		fprintf(stderr, "ERROR: Unsupported protocol\n");
		break;
	}

	if (err == CUPS_BACKEND_FAILED && cups_class) {
		fputs("INFO: Unable to contact printer, queuing on "
					"next printer in class...\n", stderr);
		sleep(5);
		return CUPS_BACKEND_FAILED;
	} else if (err == CUPS_BACKEND_RETRY) {
		sleep(20);
		goto connect;
	}

	if (fd != 0)
		close(fd);

	if (!err)
		fprintf(stderr, "INFO: Ready to print\n");

	return err;
}
