/*
 *
 *  OBEX Client
 *
 *  Copyright (C) 2011  Bartosz Szatkowski <bulislaw@linux.com> for Comarch
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
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdlib.h>

#include <glib.h>

#include "lib/sdp.h"

#include "gobex/gobex-apparam.h"
#include "gdbus/gdbus.h"

#include "obexd/src/log.h"
#include "obexd/src/map_ap.h"
#include "dbus.h"
#include "map-event.h"

#include "map.h"
#include "transfer.h"
#include "session.h"
#include "driver.h"

#define OBEX_MAS_UUID \
	"\xBB\x58\x2B\x40\x42\x0C\x11\xDB\xB0\xDE\x08\x00\x20\x0C\x9A\x66"
#define OBEX_MAS_UUID_LEN 16

#define MAP_INTERFACE "org.bluez.obex.MessageAccess1"
#define MAP_MSG_INTERFACE "org.bluez.obex.Message1"
#define ERROR_INTERFACE "org.bluez.obex.Error"
#define MAS_UUID "00001132-0000-1000-8000-00805f9b34fb"

#define DEFAULT_COUNT 1024
#define DEFAULT_OFFSET 0

#define CHARSET_NATIVE 0
#define CHARSET_UTF8 1

static const char * const filter_list[] = {
	"subject",
	"timestamp",
	"sender",
	"sender-address",
	"recipient",
	"recipient-address",
	"type",
	"size",
	"status",
	"text",
	"attachment",
	"priority",
	"read",
	"sent",
	"protected",
	"replyto",
	NULL
};

#define FILTER_BIT_MAX	15
#define FILTER_ALL	0x0000FFFF

#define FILTER_READ_STATUS_NONE		0x00
#define FILTER_READ_STATUS_ONLY_UNREAD	0x01
#define FILTER_READ_STATUS_ONLY_READ	0x02

#define FILTER_PRIORITY_NONE		0x00
#define FILTER_PRIORITY_ONLY_HIGH	0x01
#define FILTER_PRIORITY_ONLY_NONHIGH	0x02

#define STATUS_READ 0
#define STATUS_DELETE 1
#define FILLER_BYTE 0x30

struct map_data {
	struct obc_session *session;
	GHashTable *messages;
	int16_t mas_instance_id;
	uint8_t supported_message_types;
	uint32_t supported_features;
};

struct pending_request {
	struct map_data *map;
	DBusMessage *msg;
	char *folder;
};

#define MAP_MSG_FLAG_PRIORITY	0x01
#define MAP_MSG_FLAG_READ	0x02
#define MAP_MSG_FLAG_SENT	0x04
#define MAP_MSG_FLAG_PROTECTED	0x08
#define MAP_MSG_FLAG_TEXT	0x10

struct map_msg {
	struct map_data *data;
	char *path;
	uint64_t handle;
	char *subject;
	char *timestamp;
	char *sender;
	char *sender_address;
	char *replyto;
	char *recipient;
	char *recipient_address;
	char *type;
	uint64_t size;
	char *status;
	uint64_t attachment_size;
	uint8_t flags;
	char *folder;
	GDBusPendingPropertySet pending;
};

struct map_parser {
	struct pending_request *request;
	DBusMessageIter *iter;
};

static DBusConnection *conn = NULL;

static struct pending_request *pending_request_new(struct map_data *map,
							DBusMessage *message)
{
	struct pending_request *p;

	p = g_new0(struct pending_request, 1);
	p->map = map;
	p->msg = dbus_message_ref(message);

	return p;
}

static void pending_request_free(struct pending_request *p)
{
	dbus_message_unref(p->msg);

	g_free(p->folder);
	g_free(p);
}

static void simple_cb(struct obc_session *session,
						struct obc_transfer *transfer,
						GError *err, void *user_data)
{
	struct pending_request *request = user_data;
	DBusMessage *reply;

	if (err != NULL)
		reply = g_dbus_create_error(request->msg,
						ERROR_INTERFACE ".Failed",
						"%s", err->message);
	else
		reply = dbus_message_new_method_return(request->msg);

	g_dbus_send_message(conn, reply);
	pending_request_free(request);
}

static DBusMessage *map_setpath(DBusConnection *connection,
					DBusMessage *message, void *user_data)
{
	struct map_data *map = user_data;
	const char *folder;
	struct pending_request *request;
	GError *err = NULL;

	if (dbus_message_get_args(message, NULL, DBUS_TYPE_STRING, &folder,
						DBUS_TYPE_INVALID) == FALSE)
		return g_dbus_create_error(message,
					ERROR_INTERFACE ".InvalidArguments",
					NULL);

	request = pending_request_new(map, message);

	obc_session_setpath(map->session, folder, simple_cb, request, &err);
	if (err != NULL) {
		DBusMessage *reply;
		reply =  g_dbus_create_error(message,
						ERROR_INTERFACE ".Failed",
						"%s", err->message);
		g_error_free(err);
		pending_request_free(request);
		return reply;
	}

	return NULL;
}

static void folder_element(GMarkupParseContext *ctxt, const char *element,
				const char **names, const char **values,
				gpointer user_data, GError **gerr)
{
	DBusMessageIter dict, *iter = user_data;
	const char *key;
	int i;

	if (strcasecmp("folder", element) != 0)
		return;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
			DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
			DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_VARIANT_AS_STRING
			DBUS_DICT_ENTRY_END_CHAR_AS_STRING, &dict);

	for (i = 0, key = names[i]; key; key = names[++i]) {
		if (strcasecmp("name", key) == 0)
			obex_dbus_dict_append(&dict, "Name", DBUS_TYPE_STRING,
								&values[i]);
	}

	dbus_message_iter_close_container(iter, &dict);
}

static const GMarkupParser folder_parser = {
	folder_element,
	NULL,
	NULL,
	NULL,
	NULL
};

static void folder_listing_cb(struct obc_session *session,
						struct obc_transfer *transfer,
						GError *err, void *user_data)
{
	struct pending_request *request = user_data;
	GMarkupParseContext *ctxt;
	DBusMessage *reply;
	DBusMessageIter iter, array;
	char *contents;
	size_t size;
	int perr;

	if (err != NULL) {
		reply = g_dbus_create_error(request->msg,
						ERROR_INTERFACE ".Failed",
						"%s", err->message);
		goto done;
	}

	perr = obc_transfer_get_contents(transfer, &contents, &size);
	if (perr < 0) {
		reply = g_dbus_create_error(request->msg,
						ERROR_INTERFACE ".Failed",
						"Error reading contents: %s",
						strerror(-perr));
		goto done;
	}

	reply = dbus_message_new_method_return(request->msg);
	if (reply == NULL) {
		g_free(contents);
		goto clean;
	}

	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
			DBUS_TYPE_ARRAY_AS_STRING
			DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
			DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_VARIANT_AS_STRING
			DBUS_DICT_ENTRY_END_CHAR_AS_STRING, &array);
	ctxt = g_markup_parse_context_new(&folder_parser, 0, &array, NULL);
	g_markup_parse_context_parse(ctxt, contents, size, NULL);
	g_markup_parse_context_free(ctxt);
	dbus_message_iter_close_container(&iter, &array);
	g_free(contents);

done:
	g_dbus_send_message(conn, reply);
clean:
	pending_request_free(request);
}

static DBusMessage *get_folder_listing(struct map_data *map,
							DBusMessage *message,
							GObexApparam *apparam)
{
	struct pending_request *request;
	struct obc_transfer *transfer;
	GError *err = NULL;
	DBusMessage *reply;

	transfer = obc_transfer_get("x-obex/folder-listing", NULL, NULL, &err);
	if (transfer == NULL) {
		g_obex_apparam_free(apparam);
		goto fail;
	}

	obc_transfer_set_apparam(transfer, apparam);

	request = pending_request_new(map, message);

	if (!obc_session_queue(map->session, transfer, folder_listing_cb,
							request, &err)) {
		pending_request_free(request);
		goto fail;
	}

	return NULL;

fail:
	reply = g_dbus_create_error(message, ERROR_INTERFACE ".Failed", "%s",
								err->message);
	g_error_free(err);
	return reply;
}

static GObexApparam *parse_offset(GObexApparam *apparam, DBusMessageIter *iter)
{
	guint16 num;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_UINT16)
		return NULL;

	dbus_message_iter_get_basic(iter, &num);

	return g_obex_apparam_set_uint16(apparam, MAP_AP_STARTOFFSET, num);
}

static GObexApparam *parse_max_count(GObexApparam *apparam,
							DBusMessageIter *iter)
{
	guint16 num;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_UINT16)
		return NULL;

	dbus_message_iter_get_basic(iter, &num);

	return g_obex_apparam_set_uint16(apparam, MAP_AP_MAXLISTCOUNT, num);
}

static GObexApparam *parse_folder_filters(GObexApparam *apparam,
							DBusMessageIter *iter)
{
	DBusMessageIter array;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
		return NULL;

	dbus_message_iter_recurse(iter, &array);

	while (dbus_message_iter_get_arg_type(&array) == DBUS_TYPE_DICT_ENTRY) {
		const char *key;
		DBusMessageIter value, entry;

		dbus_message_iter_recurse(&array, &entry);
		dbus_message_iter_get_basic(&entry, &key);

		dbus_message_iter_next(&entry);
		dbus_message_iter_recurse(&entry, &value);

		if (strcasecmp(key, "Offset") == 0) {
			if (parse_offset(apparam, &value) == NULL)
				return NULL;
		} else if (strcasecmp(key, "MaxCount") == 0) {
			if (parse_max_count(apparam, &value) == NULL)
				return NULL;
		}

		dbus_message_iter_next(&array);
	}

	return apparam;
}

static DBusMessage *map_list_folders(DBusConnection *connection,
					DBusMessage *message, void *user_data)
{
	struct map_data *map = user_data;
	GObexApparam *apparam;
	DBusMessageIter args;

	dbus_message_iter_init(message, &args);

	apparam = g_obex_apparam_set_uint16(NULL, MAP_AP_MAXLISTCOUNT,
							DEFAULT_COUNT);
	apparam = g_obex_apparam_set_uint16(apparam, MAP_AP_STARTOFFSET,
							DEFAULT_OFFSET);

	if (parse_folder_filters(apparam, &args) == NULL) {
		g_obex_apparam_free(apparam);
		return g_dbus_create_error(message,
				ERROR_INTERFACE ".InvalidArguments", NULL);
	}

	return get_folder_listing(map, message, apparam);
}

static void map_msg_free(void *data)
{
	struct map_msg *msg = data;

	g_free(msg->path);
	g_free(msg->subject);
	g_free(msg->folder);
	g_free(msg->timestamp);
	g_free(msg->sender);
	g_free(msg->sender_address);
	g_free(msg->replyto);
	g_free(msg->recipient);
	g_free(msg->recipient_address);
	g_free(msg->type);
	g_free(msg->status);
	g_free(msg);
}

static DBusMessage *map_msg_get(DBusConnection *connection,
					DBusMessage *message, void *user_data)
{
	struct map_msg *msg = user_data;
	struct obc_transfer *transfer;
	const char *target_file;
	gboolean attachment;
	GError *err = NULL;
	DBusMessage *reply;
	GObexApparam *apparam;
	char handle[17];

	if (dbus_message_get_args(message, NULL,
				DBUS_TYPE_STRING, &target_file,
				DBUS_TYPE_BOOLEAN, &attachment,
				DBUS_TYPE_INVALID) == FALSE)
		return g_dbus_create_error(message,
				ERROR_INTERFACE ".InvalidArguments", NULL);

	snprintf(handle, sizeof(handle), "%" PRIx64, msg->handle);

	transfer = obc_transfer_get("x-bt/message", handle, target_file, &err);
	if (transfer == NULL)
		goto fail;

	apparam = g_obex_apparam_set_uint8(NULL, MAP_AP_ATTACHMENT,
								attachment);
	apparam = g_obex_apparam_set_uint8(apparam, MAP_AP_CHARSET,
								CHARSET_UTF8);

	obc_transfer_set_apparam(transfer, apparam);

	if (!obc_session_queue(msg->data->session, transfer, NULL, NULL, &err))
		goto fail;

	return obc_transfer_create_dbus_reply(transfer, message);

fail:
	reply = g_dbus_create_error(message, ERROR_INTERFACE ".Failed", "%s",
								err->message);
	g_error_free(err);
	return reply;
}

static void set_message_status_cb(struct obc_session *session,
						struct obc_transfer *transfer,
						GError *err, void *user_data)
{
	struct map_msg *msg = user_data;

	if (err != NULL) {
		g_dbus_pending_property_error(msg->pending,
						ERROR_INTERFACE ".Failed",
						"%s", err->message);
		goto done;
	}

	g_dbus_pending_property_success(msg->pending);

done:
	msg->pending = 0;
}

static gboolean get_folder(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct map_msg *msg = data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &msg->folder);

	return TRUE;
}

static gboolean subject_exists(const GDBusPropertyTable *property, void *data)
{
	struct map_msg *msg = data;

	return msg->subject != NULL;
}

static gboolean get_subject(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct map_msg *msg = data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &msg->subject);

	return TRUE;
}

static gboolean timestamp_exists(const GDBusPropertyTable *property, void *data)
{
	struct map_msg *msg = data;

	return msg->timestamp != NULL;
}

static gboolean get_timestamp(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct map_msg *msg = data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &msg->timestamp);

	return TRUE;
}

static gboolean sender_exists(const GDBusPropertyTable *property, void *data)
{
	struct map_msg *msg = data;

	return msg->sender != NULL;
}

static gboolean get_sender(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct map_msg *msg = data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &msg->sender);

	return TRUE;
}

static gboolean sender_address_exists(const GDBusPropertyTable *property,
								void *data)
{
	struct map_msg *msg = data;

	return msg->sender_address != NULL;
}

static gboolean get_sender_address(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct map_msg *msg = data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING,
							&msg->sender_address);

	return TRUE;
}

static gboolean replyto_exists(const GDBusPropertyTable *property, void *data)
{
	struct map_msg *msg = data;

	return msg->replyto != NULL;
}

static gboolean get_replyto(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct map_msg *msg = data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &msg->replyto);

	return TRUE;
}

static gboolean recipient_exists(const GDBusPropertyTable *property, void *data)
{
	struct map_msg *msg = data;

	return msg->recipient != NULL;
}

static gboolean get_recipient(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct map_msg *msg = data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &msg->recipient);

	return TRUE;
}

static gboolean recipient_address_exists(const GDBusPropertyTable *property,
								void *data)
{
	struct map_msg *msg = data;

	return msg->recipient_address != NULL;
}

static gboolean get_recipient_address(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct map_msg *msg = data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING,
						&msg->recipient_address);

	return TRUE;
}

static gboolean type_exists(const GDBusPropertyTable *property, void *data)
{
	struct map_msg *msg = data;

	return msg->type != NULL;
}

static gboolean get_type(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct map_msg *msg = data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &msg->type);

	return TRUE;
}

static gboolean get_size(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct map_msg *msg = data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT64, &msg->size);

	return TRUE;
}

static gboolean reception_status_exists(const GDBusPropertyTable *property,
								void *data)
{
	struct map_msg *msg = data;

	return msg->status != NULL;
}

static gboolean get_reception_status(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct map_msg *msg = data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &msg->status);

	return TRUE;
}

static gboolean get_attachment_size(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct map_msg *msg = data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT64,
							&msg->attachment_size);

	return TRUE;
}

static gboolean get_flag(const GDBusPropertyTable *property,
					DBusMessageIter *iter, uint8_t flag,
					void *data)
{
	struct map_msg *msg = data;
	dbus_bool_t value = (msg->flags & flag) != 0;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &value);

	return TRUE;
}

static gboolean get_priority(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	return get_flag(property, iter, MAP_MSG_FLAG_PRIORITY, data);
}

static gboolean get_read(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	return get_flag(property, iter, MAP_MSG_FLAG_READ, data);
}

static gboolean get_sent(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	return get_flag(property, iter, MAP_MSG_FLAG_SENT, data);
}

static gboolean get_protected(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	return get_flag(property, iter, MAP_MSG_FLAG_PROTECTED, data);
}

static gboolean get_text(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	return get_flag(property, iter, MAP_MSG_FLAG_TEXT, data);
}

static void set_status(const GDBusPropertyTable *property,
			DBusMessageIter *iter, GDBusPendingPropertySet id,
			uint8_t status, void *data)
{
	struct map_msg *msg = data;
	struct obc_transfer *transfer;
	gboolean value;
	GError *err = NULL;
	GObexApparam *apparam;
	char contents[1];
	char handle[17];

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_BOOLEAN) {
		g_dbus_pending_property_error(id,
					ERROR_INTERFACE ".InvalidArguments",
					"Invalid arguments in method call");
		return;
	}

	dbus_message_iter_get_basic(iter, &value);

	contents[0] = FILLER_BYTE;

	snprintf(handle, sizeof(handle), "%" PRIx64, msg->handle);

	transfer = obc_transfer_put("x-bt/messageStatus", handle, NULL,
					contents, sizeof(contents), &err);
	if (transfer == NULL)
		goto fail;

	apparam = g_obex_apparam_set_uint8(NULL, MAP_AP_STATUSINDICATOR,
								status);
	apparam = g_obex_apparam_set_uint8(apparam, MAP_AP_STATUSVALUE,
								value);
	obc_transfer_set_apparam(transfer, apparam);

	if (!obc_session_queue(msg->data->session, transfer,
				set_message_status_cb, msg, &err))
		goto fail;

	msg->pending = id;
	return;

fail:
	g_dbus_pending_property_error(id, ERROR_INTERFACE ".Failed", "%s",
								err->message);
	g_error_free(err);
}

static void set_read(const GDBusPropertyTable *property,
			DBusMessageIter *iter, GDBusPendingPropertySet id,
			void *data)
{
	set_status(property, iter, id, STATUS_READ, data);
}

static void set_deleted(const GDBusPropertyTable *property,
			DBusMessageIter *iter, GDBusPendingPropertySet id,
			void *data)
{
	set_status(property, iter, id, STATUS_DELETE, data);
}

static const GDBusMethodTable map_msg_methods[] = {
	{ GDBUS_METHOD("Get",
			GDBUS_ARGS({ "targetfile", "s" },
						{ "attachment", "b" }),
			GDBUS_ARGS({ "transfer", "o" },
						{ "properties", "a{sv}" }),
			map_msg_get) },
	{ }
};

static const GDBusPropertyTable map_msg_properties[] = {
	{ "Folder", "s", get_folder },
	{ "Subject", "s", get_subject, NULL, subject_exists },
	{ "Timestamp", "s", get_timestamp, NULL, timestamp_exists },
	{ "Sender", "s", get_sender, NULL, sender_exists },
	{ "SenderAddress", "s", get_sender_address, NULL,
						sender_address_exists },
	{ "ReplyTo", "s", get_replyto, NULL, replyto_exists },
	{ "Recipient", "s", get_recipient, NULL, recipient_exists },
	{ "RecipientAddress", "s", get_recipient_address, NULL,
						recipient_address_exists },
	{ "Type", "s", get_type, NULL, type_exists },
	{ "Size", "t", get_size },
	{ "Text", "b", get_text },
	{ "Status", "s", get_reception_status, NULL, reception_status_exists },
	{ "AttachmentSize", "t", get_attachment_size },
	{ "Priority", "b", get_priority },
	{ "Read", "b", get_read, set_read },
	{ "Sent", "b", get_sent },
	{ "Protected", "b", get_protected },
	{ "Deleted", "b", NULL, set_deleted },
	{ }
};

static void parse_type(struct map_msg *msg, const char *value)
{
	const char *type = NULL;

	if (strcasecmp(value, "SMS_GSM") == 0)
		type = "sms-gsm";
	else if (strcasecmp(value, "SMS_CDMA") == 0)
		type = "sms-cdma";
	else if (strcasecmp(value, "EMAIL") == 0)
		type = "email";
	else if (strcasecmp(value, "MMS") == 0)
		type = "mms";

	if (g_strcmp0(msg->type, type) == 0)
		return;

	g_free(msg->type);
	msg->type = g_strdup(type);

	g_dbus_emit_property_changed(conn, msg->path,
						MAP_MSG_INTERFACE, "Type");
}

static struct map_msg *map_msg_create(struct map_data *data, uint64_t handle,
					const char *folder, const char *type)
{
	struct map_msg *msg;

	msg = g_new0(struct map_msg, 1);
	msg->data = data;
	msg->handle = handle;
	msg->path = g_strdup_printf("%s/message%" PRIu64,
					obc_session_get_path(data->session),
					msg->handle);
	msg->folder = g_strdup(folder);

	if (!g_dbus_register_interface(conn, msg->path, MAP_MSG_INTERFACE,
						map_msg_methods, NULL,
						map_msg_properties,
						msg, map_msg_free)) {
		map_msg_free(msg);
		return NULL;
	}

	g_hash_table_insert(data->messages, &msg->handle, msg);

	if (type)
		parse_type(msg, type);

	return msg;
}

static void parse_subject(struct map_msg *msg, const char *value)
{
	if (g_strcmp0(msg->subject, value) == 0)
		return;

	g_free(msg->subject);
	msg->subject = g_strdup(value);

	g_dbus_emit_property_changed(conn, msg->path,
						MAP_MSG_INTERFACE, "Subject");
}

static void parse_datetime(struct map_msg *msg, const char *value)
{
	if (g_strcmp0(msg->timestamp, value) == 0)
		return;

	g_free(msg->timestamp);
	msg->timestamp = g_strdup(value);

	g_dbus_emit_property_changed(conn, msg->path,
						MAP_MSG_INTERFACE, "Timestamp");
}

static void parse_sender(struct map_msg *msg, const char *value)
{
	if (g_strcmp0(msg->sender, value) == 0)
		return;

	g_free(msg->sender);
	msg->sender = g_strdup(value);

	g_dbus_emit_property_changed(conn, msg->path,
						MAP_MSG_INTERFACE, "Sender");
}

static void parse_sender_address(struct map_msg *msg, const char *value)
{
	if (g_strcmp0(msg->sender_address, value) == 0)
		return;

	g_free(msg->sender_address);
	msg->sender_address = g_strdup(value);

	g_dbus_emit_property_changed(conn, msg->path,
					MAP_MSG_INTERFACE, "SenderAddress");
}

static void parse_replyto(struct map_msg *msg, const char *value)
{
	if (g_strcmp0(msg->replyto, value) == 0)
		return;

	g_free(msg->replyto);
	msg->replyto = g_strdup(value);

	g_dbus_emit_property_changed(conn, msg->path,
						MAP_MSG_INTERFACE, "ReplyTo");
}

static void parse_recipient(struct map_msg *msg, const char *value)
{
	if (g_strcmp0(msg->recipient, value) == 0)
		return;

	g_free(msg->recipient);
	msg->recipient = g_strdup(value);

	g_dbus_emit_property_changed(conn, msg->path,
						MAP_MSG_INTERFACE, "Recipient");
}

static void parse_recipient_address(struct map_msg *msg, const char *value)
{
	if (g_strcmp0(msg->recipient_address, value) == 0)
		return;

	g_free(msg->recipient_address);
	msg->recipient_address = g_strdup(value);

	g_dbus_emit_property_changed(conn, msg->path,
					MAP_MSG_INTERFACE, "RecipientAddress");
}

static void parse_size(struct map_msg *msg, const char *value)
{
	uint64_t size = g_ascii_strtoll(value, NULL, 10);

	if (msg->size == size)
		return;

	msg->size = size;

	g_dbus_emit_property_changed(conn, msg->path,
						MAP_MSG_INTERFACE, "Size");
}

static void parse_text(struct map_msg *msg, const char *value)
{
	gboolean flag = strcasecmp(value, "no") != 0;
	uint8_t oldflags = msg->flags;

	if (flag)
		msg->flags |= MAP_MSG_FLAG_TEXT;
	else
		msg->flags &= ~MAP_MSG_FLAG_TEXT;

	if (msg->flags != oldflags)
		g_dbus_emit_property_changed(conn, msg->path,
						MAP_MSG_INTERFACE, "Text");
}

static void parse_status(struct map_msg *msg, const char *value)
{
	if (g_strcmp0(msg->status, value) == 0)
		return;

	g_free(msg->status);
	msg->status = g_strdup(value);

	g_dbus_emit_property_changed(conn, msg->path,
						MAP_MSG_INTERFACE, "Status");
}

static void parse_attachment_size(struct map_msg *msg, const char *value)
{
	uint64_t attachment_size = g_ascii_strtoll(value, NULL, 10);

	if (msg->attachment_size == attachment_size)
		return;

	msg->attachment_size = attachment_size;

	g_dbus_emit_property_changed(conn, msg->path,
					MAP_MSG_INTERFACE, "AttachmentSize");
}

static void parse_priority(struct map_msg *msg, const char *value)
{
	gboolean flag = strcasecmp(value, "no") != 0;
	uint8_t oldflags = msg->flags;

	if (flag)
		msg->flags |= MAP_MSG_FLAG_PRIORITY;
	else
		msg->flags &= ~MAP_MSG_FLAG_PRIORITY;

	if (msg->flags != oldflags)
		g_dbus_emit_property_changed(conn, msg->path,
						MAP_MSG_INTERFACE, "Priority");
}

static void parse_read(struct map_msg *msg, const char *value)
{
	gboolean flag = strcasecmp(value, "no") != 0;
	uint8_t oldflags = msg->flags;

	if (flag)
		msg->flags |= MAP_MSG_FLAG_READ;
	else
		msg->flags &= ~MAP_MSG_FLAG_READ;

	if (msg->flags != oldflags)
		g_dbus_emit_property_changed(conn, msg->path,
						MAP_MSG_INTERFACE, "Read");
}

static void parse_sent(struct map_msg *msg, const char *value)
{
	gboolean flag = strcasecmp(value, "no") != 0;
	uint8_t oldflags = msg->flags;

	if (flag)
		msg->flags |= MAP_MSG_FLAG_SENT;
	else
		msg->flags &= ~MAP_MSG_FLAG_SENT;

	if (msg->flags != oldflags)
		g_dbus_emit_property_changed(conn, msg->path,
						MAP_MSG_INTERFACE, "Sent");
}

static void parse_protected(struct map_msg *msg, const char *value)
{
	gboolean flag = strcasecmp(value, "no") != 0;
	uint8_t oldflags = msg->flags;

	if (flag)
		msg->flags |= MAP_MSG_FLAG_PROTECTED;
	else
		msg->flags &= ~MAP_MSG_FLAG_PROTECTED;

	if (msg->flags != oldflags)
		g_dbus_emit_property_changed(conn, msg->path,
						MAP_MSG_INTERFACE, "Protected");
}

static struct map_msg_parser {
	const char *name;
	void (*func) (struct map_msg *msg, const char *value);
} msg_parsers[] = {
		{ "subject", parse_subject },
		{ "datetime", parse_datetime },
		{ "sender_name", parse_sender },
		{ "sender_addressing", parse_sender_address },
		{ "replyto_addressing", parse_replyto },
		{ "recipient_name", parse_recipient },
		{ "recipient_addressing", parse_recipient_address },
		{ "type", parse_type },
		{ "size", parse_size },
		{ "text", parse_text },
		{ "reception_status", parse_status },
		{ "attachment_size", parse_attachment_size },
		{ "priority", parse_priority },
		{ "read", parse_read },
		{ "sent", parse_sent },
		{ "protected", parse_protected },
		{ }
};

static void msg_element(GMarkupParseContext *ctxt, const char *element,
				const char **names, const char **values,
				gpointer user_data, GError **gerr)
{
	struct map_parser *parser = user_data;
	struct map_data *data = parser->request->map;
	DBusMessageIter entry, *iter = parser->iter;
	struct map_msg *msg;
	const char *key;
	int i;
	uint64_t handle;

	if (strcasecmp("msg", element) != 0)
		return;

	for (i = 0, key = names[i]; key; key = names[++i]) {
		if (strcasecmp(key, "handle") == 0)
			break;
	}

	handle = strtoull(values[i], NULL, 16);

	msg = g_hash_table_lookup(data->messages, &handle);
	if (msg == NULL) {
		msg = map_msg_create(data, handle, parser->request->folder,
									NULL);
		if (msg == NULL)
			return;
	}

	dbus_message_iter_open_container(iter, DBUS_TYPE_DICT_ENTRY, NULL,
								&entry);

	dbus_message_iter_append_basic(&entry, DBUS_TYPE_OBJECT_PATH,
								&msg->path);

	for (i = 0, key = names[i]; key; key = names[++i]) {
		struct map_msg_parser *parser;

		for (parser = msg_parsers; parser && parser->name; parser++) {
			if (strcasecmp(key, parser->name) == 0) {
				if (values[i])
					parser->func(msg, values[i]);
				break;
			}
		}
	}

	g_dbus_get_properties(conn, msg->path, MAP_MSG_INTERFACE, &entry);

	dbus_message_iter_close_container(iter, &entry);
}

static const GMarkupParser msg_parser = {
	msg_element,
	NULL,
	NULL,
	NULL,
	NULL
};

static void message_listing_cb(struct obc_session *session,
						struct obc_transfer *transfer,
						GError *err, void *user_data)
{
	struct pending_request *request = user_data;
	struct map_parser *parser;
	GMarkupParseContext *ctxt;
	DBusMessage *reply;
	DBusMessageIter iter, array;
	char *contents;
	size_t size;
	int perr;

	if (err != NULL) {
		reply = g_dbus_create_error(request->msg,
						ERROR_INTERFACE ".Failed",
						"%s", err->message);
		goto done;
	}

	perr = obc_transfer_get_contents(transfer, &contents, &size);
	if (perr < 0) {
		reply = g_dbus_create_error(request->msg,
						ERROR_INTERFACE ".Failed",
						"Error reading contents: %s",
						strerror(-perr));
		goto done;
	}

	reply = dbus_message_new_method_return(request->msg);
	if (reply == NULL) {
		g_free(contents);
		goto clean;
	}

	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
					DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_OBJECT_PATH_AS_STRING
					DBUS_TYPE_ARRAY_AS_STRING
					DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_STRING_AS_STRING
					DBUS_TYPE_VARIANT_AS_STRING
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
					&array);

	parser = g_new(struct map_parser, 1);
	parser->request = request;
	parser->iter = &array;

	ctxt = g_markup_parse_context_new(&msg_parser, 0, parser, NULL);
	g_markup_parse_context_parse(ctxt, contents, size, NULL);
	g_markup_parse_context_free(ctxt);
	dbus_message_iter_close_container(&iter, &array);
	g_free(contents);
	g_free(parser);

done:
	g_dbus_send_message(conn, reply);
clean:
	pending_request_free(request);
}

static char *get_absolute_folder(struct map_data *map, const char *subfolder)
{
	const char *root = obc_session_get_folder(map->session);

	if (!subfolder || strlen(subfolder) == 0)
		return g_strdup(root);
	else if (g_str_has_suffix(root, "/"))
		return g_strconcat(root, subfolder, NULL);
	else
		return g_strconcat(root, "/", subfolder, NULL);
}

static DBusMessage *get_message_listing(struct map_data *map,
							DBusMessage *message,
							const char *folder,
							GObexApparam *apparam)
{
	struct pending_request *request;
	struct obc_transfer *transfer;
	GError *err = NULL;
	DBusMessage *reply;

	transfer = obc_transfer_get("x-bt/MAP-msg-listing", folder, NULL, &err);
	if (transfer == NULL) {
		g_obex_apparam_free(apparam);
		goto fail;
	}

	obc_transfer_set_apparam(transfer, apparam);

	request = pending_request_new(map, message);
	request->folder = get_absolute_folder(map, folder);

	if (!obc_session_queue(map->session, transfer, message_listing_cb,
							request, &err)) {
		pending_request_free(request);
		goto fail;
	}

	return NULL;

fail:
	reply = g_dbus_create_error(message, ERROR_INTERFACE ".Failed", "%s",
								err->message);
	g_error_free(err);
	return reply;
}

static GObexApparam *parse_subject_length(GObexApparam *apparam,
							DBusMessageIter *iter)
{
	guint8 num;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_BYTE)
		return NULL;

	dbus_message_iter_get_basic(iter, &num);

	return g_obex_apparam_set_uint8(apparam, MAP_AP_SUBJECTLENGTH, num);
}

static uint64_t get_filter_mask(const char *filterstr)
{
	int i;

	if (!filterstr)
		return 0;

	if (!g_ascii_strcasecmp(filterstr, "ALL"))
		return FILTER_ALL;

	for (i = 0; filter_list[i] != NULL; i++)
		if (!g_ascii_strcasecmp(filterstr, filter_list[i]))
			return 1ULL << i;

	return 0;
}

static int set_field(guint32 *filter, const char *filterstr)
{
	guint64 mask;

	mask = get_filter_mask(filterstr);

	if (mask == 0)
		return -EINVAL;

	*filter |= mask;
	return 0;
}

static GObexApparam *parse_fields(GObexApparam *apparam, DBusMessageIter *iter)
{
	DBusMessageIter array;
	guint32 filter = 0;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
		return NULL;

	dbus_message_iter_recurse(iter, &array);

	while (dbus_message_iter_get_arg_type(&array) == DBUS_TYPE_STRING) {
		const char *string;

		dbus_message_iter_get_basic(&array, &string);

		if (set_field(&filter, string) < 0)
			return NULL;

		dbus_message_iter_next(&array);
	}

	return g_obex_apparam_set_uint32(apparam, MAP_AP_PARAMETERMASK,
								filter);
}

static GObexApparam *parse_filter_type(GObexApparam *apparam,
							DBusMessageIter *iter)
{
	DBusMessageIter array;
	guint8 types = 0;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
		return NULL;

	dbus_message_iter_recurse(iter, &array);

	while (dbus_message_iter_get_arg_type(&array) == DBUS_TYPE_STRING) {
		const char *string;

		dbus_message_iter_get_basic(&array, &string);

		if (!g_ascii_strcasecmp(string, "sms"))
			types |= 0x03; /* sms-gsm and sms-cdma */
		else if (!g_ascii_strcasecmp(string, "email"))
			types |= 0x04; /* email */
		else if (!g_ascii_strcasecmp(string, "mms"))
			types |= 0x08; /* mms */
		else
			return NULL;

		dbus_message_iter_next(&array);
	}

	return g_obex_apparam_set_uint8(apparam, MAP_AP_FILTERMESSAGETYPE,
									types);
}

static GObexApparam *parse_period_begin(GObexApparam *apparam,
							DBusMessageIter *iter)
{
	const char *string;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_STRING)
		return NULL;

	dbus_message_iter_get_basic(iter, &string);

	return g_obex_apparam_set_string(apparam, MAP_AP_FILTERPERIODBEGIN,
								string);
}

static GObexApparam *parse_period_end(GObexApparam *apparam,
							DBusMessageIter *iter)
{
	const char *string;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_STRING)
		return NULL;

	dbus_message_iter_get_basic(iter, &string);

	return g_obex_apparam_set_string(apparam, MAP_AP_FILTERPERIODEND,
								string);
}

static GObexApparam *parse_filter_read(GObexApparam *apparam,
							DBusMessageIter *iter)
{
	guint8 status = FILTER_READ_STATUS_NONE;
	dbus_bool_t dbus_status = FALSE;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_BOOLEAN)
		return NULL;

	dbus_message_iter_get_basic(iter, &dbus_status);

	if (dbus_status)
		status = FILTER_READ_STATUS_ONLY_READ;
	else
		status = FILTER_READ_STATUS_ONLY_UNREAD;

	return g_obex_apparam_set_uint8(apparam, MAP_AP_FILTERREADSTATUS,
								status);
}

static GObexApparam *parse_filter_recipient(GObexApparam *apparam,
							DBusMessageIter *iter)
{
	const char *string;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_STRING)
		return NULL;

	dbus_message_iter_get_basic(iter, &string);

	return g_obex_apparam_set_string(apparam, MAP_AP_FILTERRECIPIENT,
								string);
}

static GObexApparam *parse_filter_sender(GObexApparam *apparam,
							DBusMessageIter *iter)
{
	const char *string;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_STRING)
		return NULL;

	dbus_message_iter_get_basic(iter, &string);

	return g_obex_apparam_set_string(apparam, MAP_AP_FILTERORIGINATOR,
								string);
}

static GObexApparam *parse_filter_priority(GObexApparam *apparam,
							DBusMessageIter *iter)
{
	guint8 priority = FILTER_PRIORITY_NONE;
	dbus_bool_t dbus_priority = FALSE;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_BOOLEAN)
		return NULL;

	dbus_message_iter_get_basic(iter, &dbus_priority);

	if (dbus_priority)
		priority = FILTER_PRIORITY_ONLY_HIGH;
	else
		priority = FILTER_PRIORITY_ONLY_NONHIGH;

	return g_obex_apparam_set_uint8(apparam, MAP_AP_FILTERPRIORITY,
								priority);
}

static GObexApparam *parse_message_filters(GObexApparam *apparam,
							DBusMessageIter *iter)
{
	DBusMessageIter array;

	DBG("");

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
		return NULL;

	dbus_message_iter_recurse(iter, &array);

	while (dbus_message_iter_get_arg_type(&array) == DBUS_TYPE_DICT_ENTRY) {
		const char *key;
		DBusMessageIter value, entry;

		dbus_message_iter_recurse(&array, &entry);
		dbus_message_iter_get_basic(&entry, &key);

		dbus_message_iter_next(&entry);
		dbus_message_iter_recurse(&entry, &value);

		if (strcasecmp(key, "Offset") == 0) {
			if (parse_offset(apparam, &value) == NULL)
				return NULL;
		} else if (strcasecmp(key, "MaxCount") == 0) {
			if (parse_max_count(apparam, &value) == NULL)
				return NULL;
		} else if (strcasecmp(key, "SubjectLength") == 0) {
			if (parse_subject_length(apparam, &value) == NULL)
				return NULL;
		} else if (strcasecmp(key, "Fields") == 0) {
			if (parse_fields(apparam, &value) == NULL)
				return NULL;
		} else if (strcasecmp(key, "Types") == 0) {
			if (parse_filter_type(apparam, &value) == NULL)
				return NULL;
		} else if (strcasecmp(key, "PeriodBegin") == 0) {
			if (parse_period_begin(apparam, &value) == NULL)
				return NULL;
		} else if (strcasecmp(key, "PeriodEnd") == 0) {
			if (parse_period_end(apparam, &value) == NULL)
				return NULL;
		} else if (strcasecmp(key, "Read") == 0) {
			if (parse_filter_read(apparam, &value) == NULL)
				return NULL;
		} else if (strcasecmp(key, "Recipient") == 0) {
			if (parse_filter_recipient(apparam, &value) == NULL)
				return NULL;
		} else if (strcasecmp(key, "Sender") == 0) {
			if (parse_filter_sender(apparam, &value) == NULL)
				return NULL;
		} else if (strcasecmp(key, "Priority") == 0) {
			if (parse_filter_priority(apparam, &value) == NULL)
				return NULL;
		}

		dbus_message_iter_next(&array);
	}

	return apparam;
}

static DBusMessage *map_list_messages(DBusConnection *connection,
					DBusMessage *message, void *user_data)
{
	struct map_data *map = user_data;
	const char *folder;
	GObexApparam *apparam;
	DBusMessageIter args;

	dbus_message_iter_init(message, &args);

	if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_STRING)
		return g_dbus_create_error(message,
				ERROR_INTERFACE ".InvalidArguments", NULL);

	dbus_message_iter_get_basic(&args, &folder);

	apparam = g_obex_apparam_set_uint16(NULL, MAP_AP_MAXLISTCOUNT,
							DEFAULT_COUNT);
	apparam = g_obex_apparam_set_uint16(apparam, MAP_AP_STARTOFFSET,
							DEFAULT_OFFSET);

	dbus_message_iter_next(&args);

	if (parse_message_filters(apparam, &args) == NULL) {
		g_obex_apparam_free(apparam);
		return g_dbus_create_error(message,
				ERROR_INTERFACE ".InvalidArguments", NULL);
	}

	return get_message_listing(map, message, folder, apparam);
}

static char **get_filter_strs(uint64_t filter, int *size)
{
	char **list, **item;
	int i;

	list = g_malloc0(sizeof(char **) * (FILTER_BIT_MAX + 2));

	item = list;

	for (i = 0; filter_list[i] != NULL; i++)
		if (filter & (1ULL << i))
			*(item++) = g_strdup(filter_list[i]);

	*item = NULL;
	*size = item - list;
	return list;
}

static DBusMessage *map_list_filter_fields(DBusConnection *connection,
					DBusMessage *message, void *user_data)
{
	char **filters = NULL;
	int size;
	DBusMessage *reply;

	filters = get_filter_strs(FILTER_ALL, &size);
	reply = dbus_message_new_method_return(message);
	dbus_message_append_args(reply, DBUS_TYPE_ARRAY,
				DBUS_TYPE_STRING, &filters, size,
				DBUS_TYPE_INVALID);

	g_strfreev(filters);
	return reply;
}

static void update_inbox_cb(struct obc_session *session,
				struct obc_transfer *transfer,
				GError *err, void *user_data)
{
	struct pending_request *request = user_data;
	DBusMessage *reply;

	if (err != NULL) {
		reply = g_dbus_create_error(request->msg,
						ERROR_INTERFACE ".Failed",
						"%s", err->message);
		goto done;
	}

	reply = dbus_message_new_method_return(request->msg);

done:
	g_dbus_send_message(conn, reply);
	pending_request_free(request);
}

static DBusMessage *map_update_inbox(DBusConnection *connection,
					DBusMessage *message, void *user_data)
{
	struct map_data *map = user_data;
	DBusMessage *reply;
	char contents[1];
	struct obc_transfer *transfer;
	GError *err = NULL;
	struct pending_request *request;

	contents[0] = FILLER_BYTE;

	transfer = obc_transfer_put("x-bt/MAP-messageUpdate", NULL, NULL,
						contents, sizeof(contents),
						&err);
	if (transfer == NULL)
		goto fail;

	request = pending_request_new(map, message);

	if (!obc_session_queue(map->session, transfer, update_inbox_cb,
							request, &err)) {
		pending_request_free(request);
		goto fail;
	}

	return NULL;

fail:
	reply = g_dbus_create_error(message, ERROR_INTERFACE ".Failed", "%s",
								err->message);
	g_error_free(err);
	return reply;
}

static DBusMessage *push_message(struct map_data *map,
							DBusMessage *message,
							const char *filename,
							const char *folder,
							GObexApparam *apparam)
{
	struct obc_transfer *transfer;
	GError *err = NULL;
	DBusMessage *reply;

	transfer = obc_transfer_put("x-bt/message", folder, filename,
								NULL, 0, &err);
	if (transfer == NULL) {
		g_obex_apparam_free(apparam);
		goto fail;
	}

	obc_transfer_set_apparam(transfer, apparam);

	if (!obc_session_queue(map->session, transfer, NULL, NULL, &err))
		goto fail;

	return obc_transfer_create_dbus_reply(transfer, message);

fail:
	reply = g_dbus_create_error(message, ERROR_INTERFACE ".Failed", "%s",
								err->message);
	g_error_free(err);
	return reply;
}

static GObexApparam *parse_transparent(GObexApparam *apparam,
							DBusMessageIter *iter)
{
	dbus_bool_t transparent;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_BOOLEAN)
		return NULL;

	dbus_message_iter_get_basic(iter, &transparent);

	return g_obex_apparam_set_uint8(apparam, MAP_AP_TRANSPARENT,
						transparent ? TRUE : FALSE);
}

static GObexApparam *parse_retry(GObexApparam *apparam, DBusMessageIter *iter)
{
	dbus_bool_t retry;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_BOOLEAN)
		return NULL;

	dbus_message_iter_get_basic(iter, &retry);

	return g_obex_apparam_set_uint8(apparam, MAP_AP_RETRY,
							retry ? TRUE : FALSE);
}

static GObexApparam *parse_charset(GObexApparam *apparam, DBusMessageIter *iter)
{
	guint8 charset = 0;
	const char *string;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_STRING)
		return NULL;

	dbus_message_iter_get_basic(iter, &string);

	if (strcasecmp(string, "native") == 0)
		charset = CHARSET_NATIVE;
	else if (strcasecmp(string, "utf8") == 0)
		charset = CHARSET_UTF8;
	else
		return NULL;

	return g_obex_apparam_set_uint8(apparam, MAP_AP_CHARSET, charset);
}

static GObexApparam *parse_push_options(GObexApparam *apparam,
							DBusMessageIter *iter)
{
	DBusMessageIter array;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
		return NULL;

	dbus_message_iter_recurse(iter, &array);

	while (dbus_message_iter_get_arg_type(&array) == DBUS_TYPE_DICT_ENTRY) {
		const char *key;
		DBusMessageIter value, entry;

		dbus_message_iter_recurse(&array, &entry);
		dbus_message_iter_get_basic(&entry, &key);

		dbus_message_iter_next(&entry);
		dbus_message_iter_recurse(&entry, &value);

		if (strcasecmp(key, "Transparent") == 0) {
			if (parse_transparent(apparam, &value) == NULL)
				return NULL;
		} else if (strcasecmp(key, "Retry") == 0) {
			if (parse_retry(apparam, &value) == NULL)
				return NULL;
		} else if (strcasecmp(key, "Charset") == 0) {
			if (parse_charset(apparam, &value) == NULL)
				return NULL;
		}

		dbus_message_iter_next(&array);
	}

	return apparam;
}

static DBusMessage *map_push_message(DBusConnection *connection,
					DBusMessage *message, void *user_data)
{
	struct map_data *map = user_data;
	char *filename;
	char *folder;
	GObexApparam *apparam;
	DBusMessageIter args;

	dbus_message_iter_init(message, &args);

	if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_STRING)
		return g_dbus_create_error(message,
				ERROR_INTERFACE ".InvalidArguments", NULL);

	dbus_message_iter_get_basic(&args, &filename);

	dbus_message_iter_next(&args);

	if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_STRING) {
		return g_dbus_create_error(message,
				ERROR_INTERFACE ".InvalidArguments", NULL);
	}

	dbus_message_iter_get_basic(&args, &folder);

	dbus_message_iter_next(&args);

	apparam = g_obex_apparam_set_uint8(NULL, MAP_AP_CHARSET, CHARSET_UTF8);

	if (parse_push_options(apparam, &args) == NULL) {
		g_obex_apparam_free(apparam);
		return g_dbus_create_error(message,
				ERROR_INTERFACE ".InvalidArguments", NULL);
	}

	return push_message(map, message, filename, folder, apparam);
}

static const GDBusMethodTable map_methods[] = {
	{ GDBUS_ASYNC_METHOD("SetFolder",
				GDBUS_ARGS({ "name", "s" }), NULL,
				map_setpath) },
	{ GDBUS_ASYNC_METHOD("ListFolders",
			GDBUS_ARGS({ "filters", "a{sv}" }),
			GDBUS_ARGS({ "content", "aa{sv}" }),
			map_list_folders) },
	{ GDBUS_ASYNC_METHOD("ListMessages",
			GDBUS_ARGS({ "folder", "s" }, { "filter", "a{sv}" }),
			GDBUS_ARGS({ "messages", "a{oa{sv}}" }),
			map_list_messages) },
	{ GDBUS_METHOD("ListFilterFields",
			NULL,
			GDBUS_ARGS({ "fields", "as" }),
			map_list_filter_fields) },
	{ GDBUS_ASYNC_METHOD("UpdateInbox",
			NULL,
			NULL,
			map_update_inbox) },
	{ GDBUS_ASYNC_METHOD("PushMessage",
			GDBUS_ARGS({ "file", "s" }, { "folder", "s" },
						{ "args", "a{sv}" }),
			GDBUS_ARGS({ "transfer", "o" },
						{ "properties", "a{sv}" }),
			map_push_message) },
	{ }
};

static void map_msg_remove(void *data)
{
	struct map_msg *msg = data;
	char *path;

	path = msg->path;
	msg->path = NULL;
	g_dbus_unregister_interface(conn, path, MAP_MSG_INTERFACE);
	g_free(path);
}

static void map_handle_new_message(struct map_data *map,
							struct map_event *event)
{
	struct map_msg *msg;

	msg = g_hash_table_lookup(map->messages, &event->handle);
	/* New message event can be used if a new message replaces an old one */
	if (msg)
		g_hash_table_remove(map->messages, &event->handle);

	map_msg_create(map, event->handle, event->folder, event->msg_type);
}

static void map_handle_status_changed(struct map_data *map,
							struct map_event *event,
							const char *status)
{
	struct map_msg *msg;

	msg = g_hash_table_lookup(map->messages, &event->handle);
	if (msg == NULL)
		return;

	if (g_strcmp0(msg->status, status) == 0)
		return;

	g_free(msg->status);
	msg->status = g_strdup(status);

	g_dbus_emit_property_changed(conn, msg->path, MAP_MSG_INTERFACE,
								"Status");
}

static void map_handle_folder_changed(struct map_data *map,
							struct map_event *event,
							const char *folder)
{
	struct map_msg *msg;

	if (!folder)
		return;

	msg = g_hash_table_lookup(map->messages, &event->handle);
	if (!msg)
		return;

	if (g_strcmp0(msg->folder, folder) == 0)
		return;

	g_free(msg->folder);
	msg->folder = g_strdup(folder);

	g_dbus_emit_property_changed(conn, msg->path, MAP_MSG_INTERFACE,
								"Folder");
}

static void map_handle_notification(struct map_event *event, void *user_data)
{
	struct map_data *map = user_data;

	DBG("Event report for %s:%d", obc_session_get_destination(map->session),
							map->mas_instance_id);
	DBG("type=%x handle=%" PRIx64 " folder=%s old_folder=%s msg_type=%s",
		event->type, event->handle, event->folder, event->old_folder,
		event->msg_type);

	switch (event->type) {
	case MAP_ET_NEW_MESSAGE:
		map_handle_new_message(map, event);
		break;
	case MAP_ET_DELIVERY_SUCCESS:
		map_handle_status_changed(map, event, "delivery-success");
		break;
	case MAP_ET_SENDING_SUCCESS:
		map_handle_status_changed(map, event, "sending-success");
		break;
	case MAP_ET_DELIVERY_FAILURE:
		map_handle_status_changed(map, event, "delivery-failure");
		break;
	case MAP_ET_SENDING_FAILURE:
		map_handle_status_changed(map, event, "sending-failure");
		break;
	case MAP_ET_MESSAGE_DELETED:
		map_handle_folder_changed(map, event, "/telecom/msg/deleted");
		break;
	case MAP_ET_MESSAGE_SHIFT:
		map_handle_folder_changed(map, event, event->folder);
		break;
	case MAP_ET_MEMORY_FULL:
	case MAP_ET_MEMORY_AVAILABLE:
	default:
		break;
	}
}

static bool set_notification_registration(struct map_data *map, bool status)
{
	struct obc_transfer *transfer;
	GError *err = NULL;
	GObexApparam *apparam;
	char contents[1];
	const char *address;

	address = obc_session_get_destination(map->session);
	if (!address || map->mas_instance_id < 0)
		return FALSE;

	if (status) {
		map_register_event_handler(map->session, map->mas_instance_id,
						&map_handle_notification, map);
	} else {
		map_unregister_event_handler(map->session,
							map->mas_instance_id);
	}

	contents[0] = FILLER_BYTE;

	transfer = obc_transfer_put("x-bt/MAP-NotificationRegistration", NULL,
					NULL, contents, sizeof(contents), &err);

	if (transfer == NULL)
		return false;

	apparam = g_obex_apparam_set_uint8(NULL, MAP_AP_NOTIFICATIONSTATUS,
									status);

	obc_transfer_set_apparam(transfer, apparam);

	if (obc_session_queue(map->session, transfer, NULL, map, &err))
		return true;

	return false;
}

static void map_free(void *data)
{
	struct map_data *map = data;

	set_notification_registration(map, false);

	obc_session_unref(map->session);
	g_hash_table_unref(map->messages);
	g_free(map);
}

static void parse_service_record(struct map_data *map)
{
	const void *data;

	/* MAS instance id */
	map->mas_instance_id = -1;
	data = obc_session_get_attribute(map->session,
						SDP_ATTR_MAS_INSTANCE_ID);
	if (data != NULL)
		map->mas_instance_id = *(uint8_t *)data;
	else
		DBG("Failed to read MAS instance id");

	/* Supported Message Types */
	data = obc_session_get_attribute(map->session,
					SDP_ATTR_SUPPORTED_MESSAGE_TYPES);
	if (data != NULL)
		map->supported_message_types = *(uint8_t *)data;
	else
		DBG("Failed to read supported message types");

	/* Supported Feature Bits */
	data = obc_session_get_attribute(map->session,
					SDP_ATTR_MAP_SUPPORTED_FEATURES);
	if(data != NULL)
		map->supported_features = *(uint32_t *) data;
	else
		map->supported_features = 0x0000001f;
}

static int map_probe(struct obc_session *session)
{
	struct map_data *map;
	const char *path;

	path = obc_session_get_path(session);

	map = g_try_new0(struct map_data, 1);
	if (!map)
		return -ENOMEM;

	map->session = obc_session_ref(session);
	map->messages = g_hash_table_new_full(g_int64_hash, g_int64_equal, NULL,
								map_msg_remove);

	parse_service_record(map);

	DBG("%s, instance id %d", path, map->mas_instance_id);

	set_notification_registration(map, true);

	if (!g_dbus_register_interface(conn, path, MAP_INTERFACE, map_methods,
					NULL, NULL, map, map_free)) {
		map_free(map);

		return -ENOMEM;
	}

	return 0;
}

static void map_remove(struct obc_session *session)
{
	const char *path = obc_session_get_path(session);

	DBG("%s", path);

	g_dbus_unregister_interface(conn, path, MAP_INTERFACE);
}

static struct obc_driver map = {
	.service = "MAP",
	.uuid = MAS_UUID,
	.target = OBEX_MAS_UUID,
	.target_len = OBEX_MAS_UUID_LEN,
	.probe = map_probe,
	.remove = map_remove
};

int map_init(void)
{
	int err;

	DBG("");

	conn = dbus_bus_get(DBUS_BUS_SESSION, NULL);
	if (!conn)
		return -EIO;

	err = obc_driver_register(&map);
	if (err < 0) {
		dbus_connection_unref(conn);
		conn = NULL;
		return err;
	}

	return 0;
}

void map_exit(void)
{
	DBG("");

	dbus_connection_unref(conn);
	conn = NULL;

	obc_driver_unregister(&map);
}
