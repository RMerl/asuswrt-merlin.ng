// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2010  Nokia Corporation
 *  Copyright (C) 2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"
#include "lib/sdp.h"
#include "lib/uuid.h"

#include "src/shared/util.h"
#include "att.h"
#include "btio/btio.h"
#include "gattrib.h"
#include "gatt.h"
#include "gatttool.h"

static char *opt_src = NULL;
static char *opt_dst = NULL;
static char *opt_dst_type = NULL;
static char *opt_value = NULL;
static char *opt_sec_level = NULL;
static bt_uuid_t *opt_uuid = NULL;
static int opt_start = 0x0001;
static int opt_end = 0xffff;
static int opt_handle = -1;
static int opt_mtu = 0;
static int opt_psm = 0;
static gboolean opt_primary = FALSE;
static gboolean opt_characteristics = FALSE;
static gboolean opt_char_read = FALSE;
static gboolean opt_listen = FALSE;
static gboolean opt_char_desc = FALSE;
static gboolean opt_char_write = FALSE;
static gboolean opt_char_write_req = FALSE;
static gboolean opt_interactive = FALSE;
static GMainLoop *event_loop;
static gboolean got_error = FALSE;
static GSourceFunc operation;

struct characteristic_data {
	GAttrib *attrib;
	uint16_t start;
	uint16_t end;
};

static void events_handler(const uint8_t *pdu, uint16_t len, gpointer user_data)
{
	GAttrib *attrib = user_data;
	uint8_t *opdu;
	uint16_t handle, i, olen = 0;
	size_t plen;

	handle = get_le16(&pdu[1]);

	switch (pdu[0]) {
	case ATT_OP_HANDLE_NOTIFY:
		g_print("Notification handle = 0x%04x value: ", handle);
		break;
	case ATT_OP_HANDLE_IND:
		g_print("Indication   handle = 0x%04x value: ", handle);
		break;
	default:
		g_print("Invalid opcode\n");
		return;
	}

	for (i = 3; i < len; i++)
		g_print("%02x ", pdu[i]);

	g_print("\n");

	if (pdu[0] == ATT_OP_HANDLE_NOTIFY)
		return;

	opdu = g_attrib_get_buffer(attrib, &plen);
	olen = enc_confirmation(opdu, plen);

	if (olen > 0)
		g_attrib_send(attrib, 0, opdu, olen, NULL, NULL, NULL);
}

static gboolean listen_start(gpointer user_data)
{
	GAttrib *attrib = user_data;

	g_attrib_register(attrib, ATT_OP_HANDLE_NOTIFY, GATTRIB_ALL_HANDLES,
						events_handler, attrib, NULL);
	g_attrib_register(attrib, ATT_OP_HANDLE_IND, GATTRIB_ALL_HANDLES,
						events_handler, attrib, NULL);

	return FALSE;
}

static void connect_cb(GIOChannel *io, GError *err, gpointer user_data)
{
	GAttrib *attrib;
	uint16_t mtu;
	uint16_t cid;
	GError *gerr = NULL;

	if (err) {
		g_printerr("%s\n", err->message);
		got_error = TRUE;
		g_main_loop_quit(event_loop);
	}

	bt_io_get(io, &gerr, BT_IO_OPT_IMTU, &mtu,
				BT_IO_OPT_CID, &cid, BT_IO_OPT_INVALID);

	if (gerr) {
		g_printerr("Can't detect MTU, using default: %s",
								gerr->message);
		g_error_free(gerr);
		mtu = ATT_DEFAULT_LE_MTU;
	}

	if (cid == ATT_CID)
		mtu = ATT_DEFAULT_LE_MTU;

	attrib = g_attrib_new(io, mtu, false);

	if (opt_listen)
		g_idle_add(listen_start, attrib);

	operation(attrib);
}

static void primary_all_cb(uint8_t status, GSList *services, void *user_data)
{
	GSList *l;

	if (status) {
		g_printerr("Discover all primary services failed: %s\n",
							att_ecode2str(status));
		goto done;
	}

	for (l = services; l; l = l->next) {
		struct gatt_primary *prim = l->data;
		g_print("attr handle = 0x%04x, end grp handle = 0x%04x "
			"uuid: %s\n", prim->range.start, prim->range.end, prim->uuid);
	}

done:
	g_main_loop_quit(event_loop);
}

static void primary_by_uuid_cb(uint8_t status, GSList *ranges, void *user_data)
{
	GSList *l;

	if (status != 0) {
		g_printerr("Discover primary services by UUID failed: %s\n",
							att_ecode2str(status));
		goto done;
	}

	for (l = ranges; l; l = l->next) {
		struct att_range *range = l->data;
		g_print("Starting handle: %04x Ending handle: %04x\n",
						range->start, range->end);
	}

done:
	g_main_loop_quit(event_loop);
}

static gboolean primary(gpointer user_data)
{
	GAttrib *attrib = user_data;

	if (opt_uuid)
		gatt_discover_primary(attrib, opt_uuid, primary_by_uuid_cb,
									NULL);
	else
		gatt_discover_primary(attrib, NULL, primary_all_cb, NULL);

	return FALSE;
}

static void char_discovered_cb(uint8_t status, GSList *characteristics,
								void *user_data)
{
	GSList *l;

	if (status) {
		g_printerr("Discover all characteristics failed: %s\n",
							att_ecode2str(status));
		goto done;
	}

	for (l = characteristics; l; l = l->next) {
		struct gatt_char *chars = l->data;

		g_print("handle = 0x%04x, char properties = 0x%02x, char value "
			"handle = 0x%04x, uuid = %s\n", chars->handle,
			chars->properties, chars->value_handle, chars->uuid);
	}

done:
	g_main_loop_quit(event_loop);
}

static gboolean characteristics(gpointer user_data)
{
	GAttrib *attrib = user_data;

	gatt_discover_char(attrib, opt_start, opt_end, opt_uuid,
						char_discovered_cb, NULL);

	return FALSE;
}

static void char_read_cb(guint8 status, const guint8 *pdu, guint16 plen,
							gpointer user_data)
{
	uint8_t value[plen];
	ssize_t vlen;
	int i;

	if (status != 0) {
		g_printerr("Characteristic value/descriptor read failed: %s\n",
							att_ecode2str(status));
		goto done;
	}

	vlen = dec_read_resp(pdu, plen, value, sizeof(value));
	if (vlen < 0) {
		g_printerr("Protocol error\n");
		goto done;
	}
	g_print("Characteristic value/descriptor: ");
	for (i = 0; i < vlen; i++)
		g_print("%02x ", value[i]);
	g_print("\n");

done:
	if (!opt_listen)
		g_main_loop_quit(event_loop);
}

static void char_read_by_uuid_cb(guint8 status, const guint8 *pdu,
					guint16 plen, gpointer user_data)
{
	struct att_data_list *list;
	int i;

	if (status != 0) {
		g_printerr("Read characteristics by UUID failed: %s\n",
							att_ecode2str(status));
		goto done;
	}

	list = dec_read_by_type_resp(pdu, plen);
	if (list == NULL)
		goto done;

	for (i = 0; i < list->num; i++) {
		uint8_t *value = list->data[i];
		int j;

		g_print("handle: 0x%04x \t value: ", get_le16(value));
		value += 2;
		for (j = 0; j < list->len - 2; j++, value++)
			g_print("%02x ", *value);
		g_print("\n");
	}

	att_data_list_free(list);

done:
	g_main_loop_quit(event_loop);
}

static gboolean characteristics_read(gpointer user_data)
{
	GAttrib *attrib = user_data;

	if (opt_uuid != NULL) {

		gatt_read_char_by_uuid(attrib, opt_start, opt_end, opt_uuid,
						char_read_by_uuid_cb, NULL);

		return FALSE;
	}

	if (opt_handle <= 0) {
		g_printerr("A valid handle is required\n");
		g_main_loop_quit(event_loop);
		return FALSE;
	}

	gatt_read_char(attrib, opt_handle, char_read_cb, attrib);

	return FALSE;
}

static void mainloop_quit(gpointer user_data)
{
	uint8_t *value = user_data;

	g_free(value);
	g_main_loop_quit(event_loop);
}

static gboolean characteristics_write(gpointer user_data)
{
	GAttrib *attrib = user_data;
	uint8_t *value;
	size_t len;

	if (opt_handle <= 0) {
		g_printerr("A valid handle is required\n");
		goto error;
	}

	if (opt_value == NULL || opt_value[0] == '\0') {
		g_printerr("A value is required\n");
		goto error;
	}

	len = gatt_attr_data_from_string(opt_value, &value);
	if (len == 0) {
		g_printerr("Invalid value\n");
		goto error;
	}

	gatt_write_cmd(attrib, opt_handle, value, len, mainloop_quit, value);

	g_free(value);
	return FALSE;

error:
	g_main_loop_quit(event_loop);
	return FALSE;
}

static void char_write_req_cb(guint8 status, const guint8 *pdu, guint16 plen,
							gpointer user_data)
{
	if (status != 0) {
		g_printerr("Characteristic Write Request failed: "
						"%s\n", att_ecode2str(status));
		goto done;
	}

	if (!dec_write_resp(pdu, plen) && !dec_exec_write_resp(pdu, plen)) {
		g_printerr("Protocol error\n");
		goto done;
	}

	g_print("Characteristic value was written successfully\n");

done:
	if (!opt_listen)
		g_main_loop_quit(event_loop);
}

static gboolean characteristics_write_req(gpointer user_data)
{
	GAttrib *attrib = user_data;
	uint8_t *value;
	size_t len;

	if (opt_handle <= 0) {
		g_printerr("A valid handle is required\n");
		goto error;
	}

	if (opt_value == NULL || opt_value[0] == '\0') {
		g_printerr("A value is required\n");
		goto error;
	}

	len = gatt_attr_data_from_string(opt_value, &value);
	if (len == 0) {
		g_printerr("Invalid value\n");
		goto error;
	}

	gatt_write_char(attrib, opt_handle, value, len, char_write_req_cb,
									NULL);

	g_free(value);
	return FALSE;

error:
	g_main_loop_quit(event_loop);
	return FALSE;
}

static void char_desc_cb(uint8_t status, GSList *descriptors, void *user_data)
{
	GSList *l;

	if (status) {
		g_printerr("Discover descriptors failed: %s\n",
							att_ecode2str(status));
		return;
	}

	for (l = descriptors; l; l = l->next) {
		struct gatt_desc *desc = l->data;

		g_print("handle = 0x%04x, uuid = %s\n", desc->handle,
								desc->uuid);
	}

	if (!opt_listen)
		g_main_loop_quit(event_loop);
}

static gboolean characteristics_desc(gpointer user_data)
{
	GAttrib *attrib = user_data;

	gatt_discover_desc(attrib, opt_start, opt_end, NULL, char_desc_cb,
									NULL);

	return FALSE;
}

static gboolean parse_uuid(const char *key, const char *value,
				gpointer user_data, GError **error)
{
	if (!value)
		return FALSE;

	opt_uuid = g_try_malloc(sizeof(bt_uuid_t));
	if (opt_uuid == NULL)
		return FALSE;

	if (bt_string_to_uuid(opt_uuid, value) < 0)
		return FALSE;

	return TRUE;
}

static GOptionEntry primary_char_options[] = {
	{ "start", 's' , 0, G_OPTION_ARG_INT, &opt_start,
		"Starting handle(optional)", "0x0001" },
	{ "end", 'e' , 0, G_OPTION_ARG_INT, &opt_end,
		"Ending handle(optional)", "0xffff" },
	{ "uuid", 'u', G_OPTION_FLAG_OPTIONAL_ARG, G_OPTION_ARG_CALLBACK,
		parse_uuid, "UUID16 or UUID128(optional)", "0x1801"},
	{ NULL },
};

static GOptionEntry char_rw_options[] = {
	{ "handle", 'a' , 0, G_OPTION_ARG_INT, &opt_handle,
		"Read/Write characteristic by handle(required)", "0x0001" },
	{ "value", 'n' , 0, G_OPTION_ARG_STRING, &opt_value,
		"Write characteristic value (required for write operation)",
		"0x0001" },
	{NULL},
};

static GOptionEntry gatt_options[] = {
	{ "primary", 0, 0, G_OPTION_ARG_NONE, &opt_primary,
		"Primary Service Discovery", NULL },
	{ "characteristics", 0, 0, G_OPTION_ARG_NONE, &opt_characteristics,
		"Characteristics Discovery", NULL },
	{ "char-read", 0, 0, G_OPTION_ARG_NONE, &opt_char_read,
		"Characteristics Value/Descriptor Read", NULL },
	{ "char-write", 0, 0, G_OPTION_ARG_NONE, &opt_char_write,
		"Characteristics Value Write Without Response (Write Command)",
		NULL },
	{ "char-write-req", 0, 0, G_OPTION_ARG_NONE, &opt_char_write_req,
		"Characteristics Value Write (Write Request)", NULL },
	{ "char-desc", 0, 0, G_OPTION_ARG_NONE, &opt_char_desc,
		"Characteristics Descriptor Discovery", NULL },
	{ "listen", 0, 0, G_OPTION_ARG_NONE, &opt_listen,
		"Listen for notifications and indications", NULL },
	{ "interactive", 'I', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
		&opt_interactive, "Use interactive mode", NULL },
	{ NULL },
};

static GOptionEntry options[] = {
	{ "adapter", 'i', 0, G_OPTION_ARG_STRING, &opt_src,
		"Specify local adapter interface", "hciX" },
	{ "device", 'b', 0, G_OPTION_ARG_STRING, &opt_dst,
		"Specify remote Bluetooth address", "MAC" },
	{ "addr-type", 't', 0, G_OPTION_ARG_STRING, &opt_dst_type,
		"Set LE address type. Default: public", "[public | random]"},
	{ "mtu", 'm', 0, G_OPTION_ARG_INT, &opt_mtu,
		"Specify the MTU size", "MTU" },
	{ "psm", 'p', 0, G_OPTION_ARG_INT, &opt_psm,
		"Specify the PSM for GATT/ATT over BR/EDR", "PSM" },
	{ "sec-level", 'l', 0, G_OPTION_ARG_STRING, &opt_sec_level,
		"Set security level. Default: low", "[low | medium | high]"},
	{ NULL },
};

int main(int argc, char *argv[])
{
	GOptionContext *context;
	GOptionGroup *gatt_group, *params_group, *char_rw_group;
	GError *gerr = NULL;
	GIOChannel *chan;

	opt_dst_type = g_strdup("public");
	opt_sec_level = g_strdup("low");

	context = g_option_context_new(NULL);
	g_option_context_add_main_entries(context, options, NULL);

	/* GATT commands */
	gatt_group = g_option_group_new("gatt", "GATT commands",
					"Show all GATT commands", NULL, NULL);
	g_option_context_add_group(context, gatt_group);
	g_option_group_add_entries(gatt_group, gatt_options);

	/* Primary Services and Characteristics arguments */
	params_group = g_option_group_new("params",
			"Primary Services/Characteristics arguments",
			"Show all Primary Services/Characteristics arguments",
			NULL, NULL);
	g_option_context_add_group(context, params_group);
	g_option_group_add_entries(params_group, primary_char_options);

	/* Characteristics value/descriptor read/write arguments */
	char_rw_group = g_option_group_new("char-read-write",
		"Characteristics Value/Descriptor Read/Write arguments",
		"Show all Characteristics Value/Descriptor Read/Write "
		"arguments",
		NULL, NULL);
	g_option_context_add_group(context, char_rw_group);
	g_option_group_add_entries(char_rw_group, char_rw_options);

	if (!g_option_context_parse(context, &argc, &argv, &gerr)) {
		g_printerr("%s\n", gerr->message);
		g_clear_error(&gerr);
	}

	if (opt_interactive) {
		interactive(opt_src, opt_dst, opt_dst_type, opt_psm);
		goto done;
	}

	if (opt_primary)
		operation = primary;
	else if (opt_characteristics)
		operation = characteristics;
	else if (opt_char_read)
		operation = characteristics_read;
	else if (opt_char_write)
		operation = characteristics_write;
	else if (opt_char_write_req)
		operation = characteristics_write_req;
	else if (opt_char_desc)
		operation = characteristics_desc;
	else {
		char *help = g_option_context_get_help(context, TRUE, NULL);
		g_print("%s\n", help);
		g_free(help);
		got_error = TRUE;
		goto done;
	}

	if (opt_dst == NULL) {
		g_print("Remote Bluetooth address required\n");
		got_error = TRUE;
		goto done;
	}

	chan = gatt_connect(opt_src, opt_dst, opt_dst_type, opt_sec_level,
					opt_psm, opt_mtu, connect_cb, &gerr);
	if (chan == NULL) {
		g_printerr("%s\n", gerr->message);
		g_clear_error(&gerr);
		got_error = TRUE;
		goto done;
	}

	event_loop = g_main_loop_new(NULL, FALSE);

	g_main_loop_run(event_loop);

	g_main_loop_unref(event_loop);

done:
	g_option_context_free(context);
	g_free(opt_src);
	g_free(opt_dst);
	g_free(opt_uuid);
	g_free(opt_sec_level);

	if (got_error)
		exit(EXIT_FAILURE);
	else
		exit(EXIT_SUCCESS);
}
