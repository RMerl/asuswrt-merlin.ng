// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>

#include <glib.h>

#include "src/shared/util.h"
#include "src/shared/tester.h"
#include "src/log.h"
#include "lib/bluetooth.h"

#include "android/avctp.h"
#include "android/avrcp-lib.h"

struct test_pdu {
	bool valid;
	bool fragmented;
	bool continuing;
	bool browse;
	uint8_t *data;
	size_t size;
};

struct test_data {
	char *test_name;
	struct test_pdu *pdu_list;
};

struct context {
	struct avrcp *session;
	guint source;
	guint browse_source;
	guint process;
	int fd;
	int browse_fd;
	unsigned int pdu_offset;
	const struct test_data *data;
};

#define data(args...) ((const unsigned char[]) { args })

#define raw_pdu(args...)					\
	{							\
		.valid = true,					\
		.data = g_memdup(data(args), sizeof(data(args))), \
		.size = sizeof(data(args)),			\
	}

#define brs_pdu(args...)					\
	{							\
		.valid = true,					\
		.browse = true,					\
		.data = g_memdup(data(args), sizeof(data(args))), \
		.size = sizeof(data(args)),			\
	}

#define frg_pdu(args...)					\
	{							\
		.valid = true,					\
		.fragmented = true,				\
		.data = g_memdup(data(args), sizeof(data(args))), \
		.size = sizeof(data(args)),			\
	}

#define cont_pdu(args...)					\
	{							\
		.valid = true,					\
		.continuing = true,				\
		.data = g_memdup(data(args), sizeof(data(args))), \
		.size = sizeof(data(args)),			\
	}

#define define_test(name, function, args...)				\
	do {								\
		const struct test_pdu pdus[] = {			\
			args, { }					\
		};							\
		static struct test_data data;				\
		data.test_name = g_strdup(name);			\
		data.pdu_list = g_memdup(pdus, sizeof(pdus));		\
		tester_add(name, &data, NULL, function, NULL);		\
	} while (0)

static void test_free(gconstpointer user_data)
{
	const struct test_data *data = user_data;
	struct test_pdu *pdu;
	int i;

	for (i = 0; (pdu = &data->pdu_list[i]) && pdu->valid; i++)
		g_free(pdu->data);

	g_free(data->test_name);
	g_free(data->pdu_list);
}

static void destroy_context(struct context *context)
{
	if (context->source > 0)
		g_source_remove(context->source);

	if (context->browse_source > 0)
		g_source_remove(context->browse_source);

	avrcp_shutdown(context->session);

	test_free(context->data);
	g_free(context);
}

static gboolean context_quit(gpointer user_data)
{
	struct context *context = user_data;

	if (context->process > 0)
		g_source_remove(context->process);

	destroy_context(context);

	tester_test_passed();

	return FALSE;
}

static gboolean send_pdu(gpointer user_data)
{
	struct context *context = user_data;
	const struct test_pdu *pdu;
	ssize_t len;

	pdu = &context->data->pdu_list[context->pdu_offset++];

	if (pdu->browse) {
		len = write(context->browse_fd, pdu->data, pdu->size);
		tester_monitor('<', 0x0000, 0x001b, pdu->data, len);
	} else {
		len = write(context->fd, pdu->data, pdu->size);
		tester_monitor('<', 0x0000, 0x0017, pdu->data, len);
	}

	g_assert_cmpint(len, ==, pdu->size);

	if (pdu->fragmented)
		return send_pdu(user_data);

	context->process = 0;
	return FALSE;
}

static void context_process(struct context *context)
{
	if (!context->data->pdu_list[context->pdu_offset].valid) {
		context_quit(context);
		return;
	}

	context->process = g_idle_add(send_pdu, context);
}

static gboolean test_handler(GIOChannel *channel, GIOCondition cond,
							gpointer user_data)
{
	struct context *context = user_data;
	const struct test_pdu *pdu;
	unsigned char buf[512];
	ssize_t len;
	int fd;

	pdu = &context->data->pdu_list[context->pdu_offset++];

	g_assert(!pdu->browse);

	if (cond & (G_IO_NVAL | G_IO_ERR | G_IO_HUP)) {
		context->source = 0;
		tester_debug("%s: cond %x\n", __func__, cond);
		return FALSE;
	}

	fd = g_io_channel_unix_get_fd(channel);

	len = read(fd, buf, sizeof(buf));

	g_assert(len > 0);

	tester_monitor('>', 0x0000, 0x0017, buf, len);

	if (!pdu->continuing)
		g_assert_cmpint(len, ==, pdu->size);

	g_assert(memcmp(buf, pdu->data, pdu->size) == 0);

	if (!pdu->fragmented)
		context_process(context);

	return TRUE;
}

static gboolean browse_test_handler(GIOChannel *channel, GIOCondition cond,
							gpointer user_data)
{
	struct context *context = user_data;
	const struct test_pdu *pdu;
	unsigned char buf[512];
	ssize_t len;
	int fd;

	pdu = &context->data->pdu_list[context->pdu_offset++];

	g_assert(pdu->browse);

	if (cond & (G_IO_NVAL | G_IO_ERR | G_IO_HUP)) {
		context->browse_source = 0;
		tester_debug("%s: cond %x\n", __func__, cond);
		return FALSE;
	}

	fd = g_io_channel_unix_get_fd(channel);

	len = read(fd, buf, sizeof(buf));

	g_assert(len > 0);

	tester_monitor('>', 0x0000, 0x001b, buf, len);

	g_assert_cmpint(len, ==, pdu->size);

	g_assert(memcmp(buf, pdu->data, pdu->size) == 0);

	if (!pdu->fragmented)
		context_process(context);

	return TRUE;
}

static struct context *create_context(uint16_t version, gconstpointer data)
{
	struct context *context = g_new0(struct context, 1);
	GIOChannel *channel;
	int err, sv[2];

	/* Control channel setup */

	err = socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0, sv);
	g_assert(!err);

	context->session = avrcp_new(sv[0], 672, 672, version);
	g_assert(context->session != NULL);

	channel = g_io_channel_unix_new(sv[1]);

	g_io_channel_set_close_on_unref(channel, TRUE);
	g_io_channel_set_encoding(channel, NULL, NULL);
	g_io_channel_set_buffered(channel, FALSE);

	context->source = g_io_add_watch(channel,
				G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				test_handler, context);
	g_assert(context->source > 0);

	g_io_channel_unref(channel);

	context->fd = sv[1];

	/* Browsing channel setup */

	err = socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0, sv);
	g_assert(!err);

	err = avrcp_connect_browsing(context->session, sv[0], 672, 672);
	g_assert(!err);

	channel = g_io_channel_unix_new(sv[1]);

	g_io_channel_set_close_on_unref(channel, TRUE);
	g_io_channel_set_encoding(channel, NULL, NULL);
	g_io_channel_set_buffered(channel, FALSE);

	context->browse_source = g_io_add_watch(channel,
				G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				browse_test_handler, context);
	g_assert(context->browse_source > 0);

	g_io_channel_unref(channel);

	context->browse_fd = sv[1];

	context->data = data;

	return context;
}

static void test_dummy(gconstpointer data)
{
	struct context *context =  create_context(0x0100, data);

	context_quit(context);
}

static bool handle_play(struct avrcp *session, bool pressed, void *user_data)
{
	return true;
}

static bool handle_volume_up(struct avrcp *session, bool pressed,
							void *user_data)
{
	return true;
}

static bool handle_channel_up(struct avrcp *session, bool pressed,
							void *user_data)
{
	return true;
}

static bool handle_select(struct avrcp *session, bool pressed, void *user_data)
{
	return true;
}

static bool handle_vendor_uniq(struct avrcp *session, bool pressed,
								void *user_data)
{
	return true;
}

static const struct avrcp_passthrough_handler passthrough_handlers[] = {
		{ AVC_PLAY, handle_play },
		{ AVC_VOLUME_UP, handle_volume_up },
		{ AVC_CHANNEL_UP, handle_channel_up },
		{ AVC_SELECT, handle_select },
		{ AVC_VENDOR_UNIQUE, handle_vendor_uniq },
		{ },
};

static int get_capabilities(struct avrcp *session, uint8_t transaction,
							void *user_data)
{
	return -EINVAL;
}

static int list_attributes(struct avrcp *session, uint8_t transaction,
							void *user_data)
{
	avrcp_list_player_attributes_rsp(session, transaction, 0, NULL);

	return 0;
}

static int get_attribute_text(struct avrcp *session, uint8_t transaction,
					uint8_t number, uint8_t *attrs,
					void *user_data)
{
	const char *text[number];

	if (number) {
		memset(text, 0, number);
		text[0] = "equalizer";
	}

	avrcp_get_player_attribute_text_rsp(session, transaction, number, attrs,
									text);

	return 0;
}

static int list_values(struct avrcp *session, uint8_t transaction,
						uint8_t attr, void *user_data)
{
	avrcp_list_player_values_rsp(session, transaction, 0, NULL);

	return -EINVAL;
}

static int get_value_text(struct avrcp *session, uint8_t transaction,
				uint8_t attr, uint8_t number, uint8_t *values,
				void *user_data)
{
	const char *text[number];

	if (number) {
		memset(text, 0, number);
		text[0] = "on";
	}

	avrcp_get_player_values_text_rsp(session, transaction, number,
								values, text);

	return -EINVAL;
}

static int get_value(struct avrcp *session, uint8_t transaction,
			uint8_t number, uint8_t *attrs, void *user_data)
{
	uint8_t values[number];

	memset(values, 0, number);

	avrcp_get_current_player_value_rsp(session, transaction, number, attrs,
									values);

	return 0;
}

static int set_value(struct avrcp *session, uint8_t transaction,
			uint8_t number, uint8_t *attrs, uint8_t *values,
			void *user_data)
{
	avrcp_set_player_value_rsp(session, transaction);

	return 0;
}

static int get_play_status(struct avrcp *session, uint8_t transaction,
							void *user_data)
{
	avrcp_get_play_status_rsp(session, transaction, 0xaaaaaaaa, 0xbbbbbbbb,
									0x00);

	return 0;
}

static int get_element_attributes(struct avrcp *session, uint8_t transaction,
					uint64_t uid, uint8_t number,
					uint32_t *attrs, void *user_data)
{
	struct context *context = user_data;

	if (g_str_has_prefix(context->data->test_name, "/TP/RCR")) {
		uint8_t params[1024];

		memset(params, 0x00, sizeof(params) / 2);
		memset(params + (sizeof(params) / 2), 0xff, sizeof(params) / 2);

		avrcp_get_element_attrs_rsp(session, transaction, params,
							sizeof(params));
	} else
		avrcp_get_element_attrs_rsp(session, transaction, NULL, 0);

	return 0;
}

static int track_changed(struct avrcp *session, uint8_t transaction,
					uint32_t interval, void *user_data)
{
	struct context *context = user_data;
	uint64_t track;

	if (g_str_equal(context->data->test_name, "/TP/NFY/BV-05-C") ||
		g_str_equal(context->data->test_name, "/TP/NFY/BV-08-C"))
		memset(&track, 0, sizeof(track));
	else
		memset(&track, 0xff, sizeof(track));

	avrcp_register_notification_rsp(session, transaction, AVC_CTYPE_INTERIM,
						AVRCP_EVENT_TRACK_CHANGED,
						&track, sizeof(track));

	avrcp_register_notification_rsp(session, transaction, AVC_CTYPE_CHANGED,
						AVRCP_EVENT_TRACK_CHANGED,
						&track, sizeof(track));

	return 0;
}

static int settings_changed(struct avrcp *session, uint8_t transaction,
					uint32_t interval, void *user_data)
{
	uint8_t settings[3];

	settings[0] = 0x01;
	settings[1] = 0x01;
	settings[2] = 0x02;

	avrcp_register_notification_rsp(session, transaction, AVC_CTYPE_INTERIM,
						AVRCP_EVENT_SETTINGS_CHANGED,
						settings, sizeof(settings));

	avrcp_register_notification_rsp(session, transaction, AVC_CTYPE_CHANGED,
						AVRCP_EVENT_SETTINGS_CHANGED,
						settings, sizeof(settings));

	return 0;
}

static int available_players_changed(struct avrcp *session, uint8_t transaction,
					uint32_t interval, void *user_data)
{
	avrcp_register_notification_rsp(session, transaction, AVC_CTYPE_INTERIM,
					AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED,
					NULL, 0);

	avrcp_register_notification_rsp(session, transaction, AVC_CTYPE_CHANGED,
					AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED,
					NULL, 0);

	return 0;
}

static int addressed_player_changed(struct avrcp *session, uint8_t transaction,
					uint32_t interval, void *user_data)
{
	uint16_t player[2];

	player[0] = 0x0001;
	player[1] = 0x0001;

	avrcp_register_notification_rsp(session, transaction, AVC_CTYPE_INTERIM,
					AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED,
					player, sizeof(player));

	player[0] = 0x0200;
	player[1] = 0x0200;

	avrcp_register_notification_rsp(session, transaction, AVC_CTYPE_CHANGED,
					AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED,
					player, sizeof(player));

	return 0;
}

static int uids_changed(struct avrcp *session, uint8_t transaction,
					uint32_t interval, void *user_data)
{
	struct context *context = user_data;
	uint16_t counter;

	if (g_str_equal(context->data->test_name, "/TP/MCN/CB/BV-09-C"))
		counter = 0x0000;
	else
		counter = 0x0001;

	avrcp_register_notification_rsp(session, transaction, AVC_CTYPE_INTERIM,
						AVRCP_EVENT_UIDS_CHANGED,
						&counter, sizeof(counter));

	if (!g_str_equal(context->data->test_name, "/TP/MCN/CB/BV-11-C") &&
		!g_str_equal(context->data->test_name, "/TP/MCN/CB/BI-05-C"))
		return 0;

	counter = 0x0200;

	avrcp_register_notification_rsp(session, transaction, AVC_CTYPE_CHANGED,
						AVRCP_EVENT_UIDS_CHANGED,
						&counter, sizeof(counter));

	return 0;
}

static int now_playing_content_changed(struct avrcp *session,
					uint8_t transaction, uint32_t interval,
					void *user_data)
{
	avrcp_register_notification_rsp(session, transaction, AVC_CTYPE_INTERIM,
					AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED,
					NULL, 0);

	avrcp_register_notification_rsp(session, transaction, AVC_CTYPE_CHANGED,
					AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED,
					NULL, 0);

	return 0;
}

static int volume_changed(struct avrcp *session, uint8_t transaction,
					uint32_t interval, void *user_data)
{
	uint8_t volume = 0x00;

	avrcp_register_notification_rsp(session, transaction, AVC_CTYPE_INTERIM,
					AVRCP_EVENT_VOLUME_CHANGED,
					&volume, sizeof(volume));

	volume = 0x01;

	avrcp_register_notification_rsp(session, transaction, AVC_CTYPE_CHANGED,
					AVRCP_EVENT_VOLUME_CHANGED,
					&volume, sizeof(volume));

	return 0;
}

static int register_notification(struct avrcp *session, uint8_t transaction,
					uint8_t event, uint32_t interval,
					void *user_data)
{
	switch (event) {
	case AVRCP_EVENT_TRACK_CHANGED:
		return track_changed(session, transaction, interval, user_data);
	case AVRCP_EVENT_SETTINGS_CHANGED:
		return settings_changed(session, transaction, interval,
								user_data);
	case AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED:
		return available_players_changed(session, transaction, interval,
								user_data);
	case AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED:
		return addressed_player_changed(session, transaction, interval,
								user_data);
	case AVRCP_EVENT_UIDS_CHANGED:
		return uids_changed(session, transaction, interval, user_data);
	case AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED:
		return now_playing_content_changed(session, transaction,
							interval, user_data);
	case AVRCP_EVENT_VOLUME_CHANGED:
		return volume_changed(session, transaction,
							interval, user_data);
	default:
		return -EINVAL;
	}
}

static int set_volume(struct avrcp *session, uint8_t transaction,
					uint8_t volume, void *user_data)
{
	avrcp_set_volume_rsp(session, transaction, volume);

	return 0;
}

static int set_addressed(struct avrcp *session, uint8_t transaction,
						uint16_t id, void *user_data)
{
	struct context *context = user_data;
	uint8_t status;

	if (g_str_equal(context->data->test_name, "/TP/MPS/BI-01-C"))
		status = AVRCP_STATUS_INVALID_PLAYER_ID;
	else
		status = AVRCP_STATUS_SUCCESS;

	avrcp_set_addressed_player_rsp(session, transaction, status);

	return 0;
}

static int set_browsed(struct avrcp *session, uint8_t transaction,
						uint16_t id, void *user_data)
{
	struct context *context = user_data;
	const char *folders[1] = { "Filesystem" };

	if (g_str_equal(context->data->test_name, "/TP/MPS/BI-02-C"))
		avrcp_set_browsed_player_rsp(session, transaction,
						AVRCP_STATUS_INVALID_PLAYER_ID,
						0, 0, 0, NULL);
	else
		avrcp_set_browsed_player_rsp(session, transaction,
						AVRCP_STATUS_SUCCESS,
						0xabcd, 0, 1, folders);

	return 0;
}

static int get_folder_items(struct avrcp *session, uint8_t transaction,
				uint8_t scope, uint32_t start, uint32_t end,
				uint16_t number, uint32_t *attrs,
				void *user_data)
{
	struct context *context = user_data;

	if (g_str_equal(context->data->test_name, "/TP/MCN/CB/BI-02-C"))
		return -ERANGE;

	if (start > 1)
		return -ERANGE;

	avrcp_get_folder_items_rsp(session, transaction, AVRCP_STATUS_SUCCESS,
						0xabcd, 0, NULL, NULL, NULL);

	return 0;
}

static int change_path(struct avrcp *session, uint8_t transaction,
					uint16_t counter, uint8_t direction,
					uint64_t uid, void *user_data)
{
	if (!uid)
		return -ENOTDIR;

	avrcp_change_path_rsp(session, transaction, AVRCP_STATUS_SUCCESS, 0);

	return 0;
}

static int get_item_attributes(struct avrcp *session, uint8_t transaction,
					uint8_t scope, uint64_t uid,
					uint16_t counter, uint8_t number,
					uint32_t *attrs, void *user_data)
{
	struct context *context = user_data;
	uint8_t status;

	if (g_str_equal(context->data->test_name, "/TP/MCN/CB/BI-05-C"))
		status = AVRCP_STATUS_UID_CHANGED;
	else
		status = AVRCP_STATUS_SUCCESS;

	avrcp_get_item_attributes_rsp(session, transaction, status, 0, NULL,
									NULL);

	return 0;
}

static int play_item(struct avrcp *session, uint8_t transaction, uint8_t scope,
			uint64_t uid, uint16_t counter, void *user_data)
{
	if (!uid)
		return -ENOENT;

	avrcp_play_item_rsp(session, transaction, AVRCP_STATUS_SUCCESS);

	return 0;
}

static int search(struct avrcp *session, uint8_t transaction,
					const char *string, void *user_data)
{
	avrcp_search_rsp(session, transaction, AVRCP_STATUS_SUCCESS, 0xaabb, 0);

	return 0;
}

static int add_to_now_playing(struct avrcp *session, uint8_t transaction,
				uint8_t scope, uint64_t uid, uint16_t counter,
				void *user_data)
{
	if (!uid)
		return -ENOENT;

	avrcp_add_to_now_playing_rsp(session, transaction,
							AVRCP_STATUS_SUCCESS);

	return 0;
}

static const struct avrcp_control_ind control_ind = {
	.get_capabilities = get_capabilities,
	.list_attributes = list_attributes,
	.get_attribute_text = get_attribute_text,
	.list_values = list_values,
	.get_value_text = get_value_text,
	.get_value = get_value,
	.set_value = set_value,
	.get_play_status = get_play_status,
	.get_element_attributes = get_element_attributes,
	.register_notification = register_notification,
	.set_volume = set_volume,
	.set_addressed = set_addressed,
	.set_browsed = set_browsed,
	.get_folder_items = get_folder_items,
	.change_path = change_path,
	.get_item_attributes = get_item_attributes,
	.play_item = play_item,
	.search = search,
	.add_to_now_playing = add_to_now_playing,
};

static void test_server(gconstpointer data)
{
	struct context *context = create_context(0x0100, data);

	avrcp_set_passthrough_handlers(context->session, passthrough_handlers,
								context);
	avrcp_register_player(context->session, &control_ind, NULL, context);

	g_idle_add(send_pdu, context);
}

static void get_folder_items_rsp(struct avrcp *session, int err,
					uint16_t counter, uint16_t number,
					uint8_t *params, void *user_data)
{
	struct context *context = user_data;

	g_assert_cmpint(err, ==, 0);
	g_assert_cmpint(counter, ==, 0xabcd);
	g_assert_cmpint(number, ==, 0);

	context_quit(context);
}

static void set_volume_rsp(struct avrcp *session, int err, uint8_t volume,
							void *user_data)
{
	struct context *context = user_data;

	g_assert_cmpint(err, ==, 0);
	g_assert_cmpint(volume, ==, 1);

	context_quit(context);
}

static bool register_notification_rsp(struct avrcp *session, int err,
					uint8_t code, uint8_t event,
					void *params, void *user_data)
{
	struct context *context = user_data;
	uint8_t *p = params;

	g_assert_cmpint(err, ==, 0);

	switch (event) {
	case AVRCP_EVENT_VOLUME_CHANGED:
		if (g_str_equal(context->data->test_name, "/TP/VLH/BV-03-C")) {
			g_assert_cmpint(p[0], ==, 0);
			break;
		} else if (code == AVC_CTYPE_INTERIM) {
			g_assert_cmpint(p[0], ==, 0);
			return true;
		}
		g_assert_cmpint(p[0], ==, 1);
		break;
	}

	context_quit(context);

	return false;
}

static const struct avrcp_control_cfm control_cfm = {
	.register_notification = register_notification_rsp,
	.set_volume = set_volume_rsp,
	.get_folder_items = get_folder_items_rsp,
};

static void test_client(gconstpointer data)
{
	struct context *context = create_context(0x0100, data);

	avrcp_register_player(context->session, NULL, &control_cfm, context);

	if (g_str_equal(context->data->test_name, "/TP/MPS/BV-01-C"))
		avrcp_set_addressed_player(context->session, 0xabcd);

	if (g_str_equal(context->data->test_name, "/TP/MPS/BV-03-C"))
		avrcp_set_browsed_player(context->session, 0xabcd);

	if (g_str_equal(context->data->test_name, "/TP/MPS/BV-06-C") ||
		g_str_equal(context->data->test_name, "/TP/MPS/BV-08-C"))
		avrcp_get_folder_items(context->session,
					AVRCP_MEDIA_PLAYER_LIST, 0, 2, 0, NULL);

	if (g_str_equal(context->data->test_name, "/TP/MPS/BV-01-I"))
		avrcp_get_folder_items(context->session,
					AVRCP_MEDIA_PLAYER_LIST, 0, 2, 0, NULL);

	if (g_str_equal(context->data->test_name, "/TP/MCN/CB/BV-01-C"))
		avrcp_get_folder_items(context->session,
					AVRCP_MEDIA_PLAYER_VFS, 0, 2, 0, NULL);

	if (g_str_equal(context->data->test_name, "/TP/MCN/CB/BV-04-C"))
		avrcp_change_path(context->session, 0x01, 0x01, 0xaabb);

	if (g_str_equal(context->data->test_name, "/TP/MCN/CB/BV-07-C"))
		avrcp_get_item_attributes(context->session,
					AVRCP_MEDIA_PLAYER_VFS, 0x01, 0xaabb,
					0, NULL);

	if (g_str_equal(context->data->test_name, "/TP/MCN/SRC/BV-01-C"))
		avrcp_search(context->session, "Country");

	if (g_str_equal(context->data->test_name, "/TP/MCN/SRC/BV-03-C"))
		avrcp_get_folder_items(context->session, AVRCP_MEDIA_SEARCH,
						0, 2, 0, NULL);

	if (g_str_equal(context->data->test_name, "/TP/MCN/SRC/BV-05-C"))
		avrcp_get_item_attributes(context->session,
					AVRCP_MEDIA_SEARCH, 0x01, 0xaabb,
					0, NULL);

	if (g_str_equal(context->data->test_name, "/TP/MCN/NP/BV-01-C"))
		avrcp_play_item(context->session, AVRCP_MEDIA_NOW_PLAYING, 1,
									1);

	if (g_str_equal(context->data->test_name, "/TP/MCN/NP/BV-03-C"))
		avrcp_add_to_now_playing(context->session,
					AVRCP_MEDIA_NOW_PLAYING, 0x01, 0xaabb);

	if (g_str_equal(context->data->test_name, "/TP/MCN/NP/BV-05-C"))
		avrcp_get_folder_items(context->session,
					AVRCP_MEDIA_NOW_PLAYING, 0, 2, 0, NULL);

	if (g_str_equal(context->data->test_name, "/TP/MCN/NP/BV-08-C"))
		avrcp_get_item_attributes(context->session,
					AVRCP_MEDIA_NOW_PLAYING, 0x01, 0xaabb,
					0, NULL);

	if (g_str_equal(context->data->test_name, "/TP/CFG/BV-01-C"))
		avrcp_get_capabilities(context->session, CAP_EVENTS_SUPPORTED);

	if (g_str_equal(context->data->test_name, "/TP/PAS/BV-01-C"))
		avrcp_list_player_attributes(context->session);

	if (g_str_equal(context->data->test_name, "/TP/PAS/BV-03-C")) {
		uint8_t attrs[2] = { AVRCP_ATTRIBUTE_EQUALIZER,
						AVRCP_ATTRIBUTE_REPEAT_MODE };

		avrcp_get_player_attribute_text(context->session, sizeof(attrs),
									attrs);
	}

	if (g_str_equal(context->data->test_name, "/TP/PAS/BV-05-C"))
		avrcp_list_player_values(context->session,
						AVRCP_ATTRIBUTE_EQUALIZER);

	if (g_str_equal(context->data->test_name, "/TP/PAS/BV-07-C")) {
		uint8_t values[2] = { AVRCP_EQUALIZER_OFF, AVRCP_EQUALIZER_ON };

		avrcp_get_player_value_text(context->session,
						AVRCP_ATTRIBUTE_EQUALIZER,
						sizeof(values), values);
	}

	if (g_str_equal(context->data->test_name, "/TP/PAS/BV-09-C")) {
		uint8_t attrs[2] = { AVRCP_ATTRIBUTE_EQUALIZER,
						AVRCP_ATTRIBUTE_REPEAT_MODE };

		avrcp_get_current_player_value(context->session, sizeof(attrs),
									attrs);
	}

	if (g_str_equal(context->data->test_name, "/TP/PAS/BV-11-C")) {
		uint8_t attrs[2] = { AVRCP_ATTRIBUTE_EQUALIZER,
						AVRCP_ATTRIBUTE_REPEAT_MODE };
		uint8_t values[2] = { 0xaa, 0xff };

		avrcp_set_player_value(context->session, sizeof(attrs), attrs,
								values);
	}

	if (g_str_equal(context->data->test_name, "/TP/MDI/BV-01-C"))
		avrcp_get_play_status(context->session);

	if (g_str_equal(context->data->test_name, "/TP/MDI/BV-03-C"))
		avrcp_get_element_attributes(context->session);

	if (g_str_equal(context->data->test_name, "/TP/NFY/BV-01-C"))
		avrcp_register_notification(context->session,
						AVRCP_EVENT_STATUS_CHANGED, 0);

	if (g_str_equal(context->data->test_name, "/TP/BGN/BV-01-I"))
		avrcp_send_passthrough(context->session, IEEEID_BTSIG,
						AVC_VENDOR_NEXT_GROUP);

	if (g_str_equal(context->data->test_name, "/TP/BGN/BV-02-I"))
		avrcp_send_passthrough(context->session, IEEEID_BTSIG,
						AVC_VENDOR_PREV_GROUP);

	if (g_str_equal(context->data->test_name, "/TP/VLH/BV-01-C"))
		avrcp_set_volume(context->session, 0x00);

	if (g_str_equal(context->data->test_name, "/TP/VLH/BV-03-C"))
		avrcp_register_notification(context->session,
						AVRCP_EVENT_VOLUME_CHANGED, 0);

	if (g_str_equal(context->data->test_name, "/TP/VLH/BI-03-C"))
		avrcp_set_volume(context->session, 0x01);

	if (g_str_equal(context->data->test_name, "/TP/VLH/BI-04-C"))
		avrcp_register_notification(context->session,
						AVRCP_EVENT_VOLUME_CHANGED, 0);

	if (g_str_equal(context->data->test_name, "/TP/PTH/BV-01-C"))
		avrcp_send_passthrough(context->session, 0, AVC_PLAY);

	if (g_str_equal(context->data->test_name, "/TP/PTH/BV-02-C"))
		avrcp_send_passthrough(context->session, 0, AVC_FAST_FORWARD);
}

int main(int argc, char *argv[])
{
	tester_init(&argc, &argv);

	__btd_log_init("*", 0);

	/* Media Player Selection Commands and Notifications */

	/* SetAddressedPlayer - CT */
	define_test("/TP/MPS/BV-01-C", test_client,
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48, 0x00,
				0x00, 0x19, 0x58, 0x60, 0x00, 0x00,
				0x02, 0xab, 0xcd));

	/* SetAddressedPlayer - TG */
	define_test("/TP/MPS/BV-02-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_SET_ADDRESSED_PLAYER,
				0x00, 0x00, 0x02, 0xab, 0xcd),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_STABLE,
				0x48, 0x00, 0x00, 0x19, 0x58,
				AVRCP_SET_ADDRESSED_PLAYER,
				0x00, 0x00, 0x01, 0x04));

	/* SetBrowsedPlayer - CT */
	define_test("/TP/MPS/BV-03-C", test_client,
			brs_pdu(0x00, 0x11, 0x0e, 0x70, 0x00, 0x02,
				0xab, 0xcd));

	/* SetBrowsedPlayer - TG */
	define_test("/TP/MPS/BV-04-C", test_server,
			brs_pdu(0x00, 0x11, 0x0e, 0x70, 0x00, 0x02,
				0xab, 0xcd),
			brs_pdu(0x02, 0x11, 0x0e, 0x70, 0x00, 0x16,
				0x04, 0xab, 0xcd, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x6a, 0x01, 0x00, 0x0a,
				0x46, 0x69, 0x6c, 0x65, 0x73, 0x79,
				0x73, 0x74, 0x65, 0x6d));

	/* AddressedPlayerChanged notification – TG */
	define_test("/TP/MPS/BV-05-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x03, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x05, 0x0b,
				0x00, 0x00, 0x00, 0x00),
			frg_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_INTERIM, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x05, 0x0b,
				0x00, 0x01, 0x00, 0x01),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_CHANGED, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x05, 0x0b,
				0x02, 0x00, 0x02, 0x00));

	/* GetFolderItems - CT */
	define_test("/TP/MPS/BV-06-C", test_client,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x0a, AVRCP_MEDIA_PLAYER_LIST,
				0x00, 0x00, 0x00, 0x00, /* start */
				0x00, 0x00, 0x00, 0x02, /* end */
				0x00),
			brs_pdu(0x02, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x05, 0x04, 0xab, 0xcd, 0x00, 0x00));

	/* AvailablePlayersChanged Notification – TG */
	define_test("/TP/MPS/BV-07-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x03, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x05, 0x0a,
				0x00, 0x00, 0x00, 0x00),
			frg_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_INTERIM, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x01, 0x0a),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_CHANGED, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x01, 0x0a));

	/* GetFolderItems - CT */
	define_test("/TP/MPS/BV-08-C", test_client,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x0a, AVRCP_MEDIA_PLAYER_LIST,
				0x00, 0x00, 0x00, 0x00, /* start */
				0x00, 0x00, 0x00, 0x02, /* end */
				0x00));

	/* GetFolderItems - TG */
	define_test("/TP/MPS/BV-09-C", test_server,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x0a, AVRCP_MEDIA_PLAYER_LIST,
				0x00, 0x00, 0x00, 0x00, /* start */
				0x00, 0x00, 0x00, 0x02, /* end */
				0x00),
			brs_pdu(0x02, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x05, 0x04, 0xab, 0xcd, 0x00, 0x00));

	/* SetAddressedPlayer - TG */
	define_test("/TP/MPS/BI-01-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_SET_ADDRESSED_PLAYER,
				0x00, 0x00, 0x02, 0xab, 0xcd),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_STABLE,
				0x48, 0x00, 0x00, 0x19, 0x58,
				AVRCP_SET_ADDRESSED_PLAYER,
				0x00, 0x00, 0x01, 0x11));

	/* SetBrowsedPlayer - TG */
	define_test("/TP/MPS/BI-02-C", test_server,
			brs_pdu(0x00, 0x11, 0x0e, 0x70, 0x00, 0x02,
				0xab, 0xcd),
			brs_pdu(0x02, 0x11, 0x0e, 0x70, 0x00, 0x01,
				0x11));

	/*
	 * Media Content Navigation Commands and Notifications for Content
	 * Browsing.
	 */

	/* GetFolderItems - Virtual FS - CT */
	define_test("/TP/MCN/CB/BV-01-C", test_client,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x0a, AVRCP_MEDIA_PLAYER_VFS,
				0x00, 0x00, 0x00, 0x00, /* start */
				0x00, 0x00, 0x00, 0x02, /* end */
				0x00));

	/* GetFolderItems - Virtual FS - TG */
	define_test("/TP/MCN/CB/BV-02-C", test_server,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x0a, AVRCP_MEDIA_PLAYER_VFS,
				0x00, 0x00, 0x00, 0x00, /* start */
				0x00, 0x00, 0x00, 0x02, /* end */
				0x00),
			brs_pdu(0x02, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x05, 0x04, 0xab, 0xcd, 0x00, 0x00));

	/* GetFolderItems - Virtual FS - TG */
	define_test("/TP/MCN/CB/BV-03-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_SET_ADDRESSED_PLAYER,
				0x00, 0x00, 0x02, 0xab, 0xcd),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_STABLE,
				0x48, 0x00, 0x00, 0x19, 0x58,
				AVRCP_SET_ADDRESSED_PLAYER,
				0x00, 0x00, 0x01, 0x04),
			brs_pdu(0x00, 0x11, 0x0e, 0x70, 0x00, 0x02,
				0xab, 0xcd),
			brs_pdu(0x02, 0x11, 0x0e, 0x70, 0x00, 0x16,
				0x04, 0xab, 0xcd, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x6a, 0x01, 0x00, 0x0a,
				0x46, 0x69, 0x6c, 0x65, 0x73, 0x79,
				0x73, 0x74, 0x65, 0x6d),
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x0a, AVRCP_MEDIA_PLAYER_VFS,
				0x00, 0x00, 0x00, 0x00, /* start */
				0x00, 0x00, 0x00, 0x02, /* end */
				0x00),
			brs_pdu(0x02, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x05, 0x04, 0xab, 0xcd, 0x00, 0x00));

	/* ChangePath - CT */
	define_test("/TP/MCN/CB/BV-04-C", test_client,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_CHANGE_PATH,
				0x00, 0x0b,
				0xaa, 0xbb,		/* counter */
				0x01,			/* direction */
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x01	/* Folder UID */));

	/* ChangePath - TG */
	define_test("/TP/MCN/CB/BV-05-C", test_server,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_CHANGE_PATH,
				0x00, 0x0b,
				0xaa, 0xbb,		/* counter */
				0x01,			/* direction */
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x01	/* Folder UID */),
			brs_pdu(0x02, 0x11, 0x0e, AVRCP_CHANGE_PATH,
				0x00, 0x05, 0x04, 0x00, 0x00, 0x00, 0x00));

	/* ChangePath - TG */
	define_test("/TP/MCN/CB/BV-06-C", test_server,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_CHANGE_PATH,
				0x00, 0x0b,
				0xaa, 0xbb,		/* counter */
				0x00,			/* direction */
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x01	/* Folder UID */),
			brs_pdu(0x02, 0x11, 0x0e, AVRCP_CHANGE_PATH,
				0x00, 0x05, 0x04, 0x00, 0x00, 0x00, 0x00));

	/* GetItemAttributes - CT */
	define_test("/TP/MCN/CB/BV-07-C", test_client,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_GET_ITEM_ATTRIBUTES,
				0x00, 0x0c, AVRCP_MEDIA_PLAYER_VFS,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x01,	/* uuid */
				0xaa, 0xbb,		/* counter */
				0x00));			/* num attr */

	/* GetItemAttributes - TG */
	define_test("/TP/MCN/CB/BV-08-C", test_server,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_GET_ITEM_ATTRIBUTES,
				0x00, 0x0c, AVRCP_MEDIA_PLAYER_VFS,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x01,	/* uuid */
				0xaa, 0xbb,		/* counter */
				0x00),			/* num attr */
			brs_pdu(0x02, 0x11, 0x0e, AVRCP_GET_ITEM_ATTRIBUTES,
				0x00, 0x02, 0x04, 0x00));

	/* UIDcounter - TG */
	define_test("/TP/MCN/CB/BV-09-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x03, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x05, 0x0c,
				0x00, 0x00, 0x00, 0x00),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_INTERIM, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x03, 0x0c,
				0x00, 0x00));

	/* UIDcounter - TG */
	define_test("/TP/MCN/CB/BV-10-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x03, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x05, 0x0c,
				0x00, 0x00, 0x00, 0x00),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_INTERIM, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x03, 0x0c,
				0x00, 0x01));

	/* UIDcounter - TG */
	define_test("/TP/MCN/CB/BV-11-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x03, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x05, 0x0c,
				0x00, 0x00, 0x00, 0x00),
			frg_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_INTERIM, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x03, 0x0c,
				0x00, 0x01),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_CHANGED, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x03, 0x0c,
				0x02, 0x00));

	/* GetFolderItems - Virtual FS - TG */
	define_test("/TP/MCN/CB/BI-01-C", test_server,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x0a, AVRCP_MEDIA_PLAYER_VFS,
				0x00, 0x00, 0x00, 0x01, /* start */
				0x00, 0x00, 0x00, 0x00, /* end */
				0x00),
			brs_pdu(0x02, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x01, 0x0b));

	/* GetFolderItems - Virtual FS - TG */
	define_test("/TP/MCN/CB/BI-02-C", test_server,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x0a, AVRCP_MEDIA_PLAYER_VFS,
				0x00, 0x00, 0x00, 0x00, /* start */
				0x00, 0x00, 0x00, 0x01, /* end */
				0x00),
			brs_pdu(0x02, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x01, 0x0b));

	/* GetFolderItems - Virtual FS - TG */
	define_test("/TP/MCN/CB/BI-03-C", test_server,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x0a, AVRCP_MEDIA_PLAYER_VFS,
				0x00, 0x00, 0x00, 0x02, /* start */
				0x00, 0x00, 0x00, 0x03, /* end */
				0x00),
			brs_pdu(0x02, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x01, 0x0b));

	/* ChangePath - TG */
	define_test("/TP/MCN/CB/BI-04-C", test_server,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_CHANGE_PATH,
				0x00, 0x0b,
				0xaa, 0xbb,		/* counter */
				0x01,			/* direction */
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00	/* Folder UID */),
			brs_pdu(0x02, 0x11, 0x0e, AVRCP_CHANGE_PATH,
				0x00, 0x01, 0x08));

	/* UIDcounter - TG */
	define_test("/TP/MCN/CB/BI-05-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x03, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x05, 0x0c,
				0x00, 0x00, 0x00, 0x00),
			frg_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_INTERIM, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x03, 0x0c,
				0x00, 0x01),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_CHANGED, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x03, 0x0c,
				0x02, 0x00),
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_GET_ITEM_ATTRIBUTES,
				0x00, 0x0c, AVRCP_MEDIA_NOW_PLAYING,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x01,
				0xaa, 0xbb,
				0x00),
			brs_pdu(0x02, 0x11, 0x0e, AVRCP_GET_ITEM_ATTRIBUTES,
				0x00, 0x01, 0x05));

	/* Media Content Navigation Commands and Notifications for Search */

	/* Search - CT */
	define_test("/TP/MCN/SRC/BV-01-C", test_client,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_SEARCH,
				0x00, 0x0b, 0x00, 0x6a,
				0x00, 0x07,
				0x43, 0x6f, 0x75, 0x6e, 0x74, 0x72, 0x79));

	define_test("/TP/MCN/SRC/BV-02-C", test_server,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_SEARCH,
				0x00, 0x0b, 0x00, 0x6a,
				0x00, 0x07,
				0x43, 0x6f, 0x75, 0x6e, 0x74, 0x72, 0x79),
			brs_pdu(0x02, 0x11, 0x0e, AVRCP_SEARCH,
				0x00, 0x07, 0x04,
				0xaa, 0xbb,		/* counter */
				0x00, 0x00, 0x00, 0x00));

	/* GetFolderItems - CT */
	define_test("/TP/MCN/SRC/BV-03-C", test_client,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x0a, AVRCP_MEDIA_SEARCH,
				0x00, 0x00, 0x00, 0x00, /* start */
				0x00, 0x00, 0x00, 0x02, /* end */
				0x00));

	/* GetFolderItems - NowPlaying - TG */
	define_test("/TP/MCN/SCR/BV-04-C", test_server,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x0a, AVRCP_MEDIA_SEARCH,
				0x00, 0x00, 0x00, 0x00, /* start */
				0x00, 0x00, 0x00, 0x02, /* end */
				0x00),
			brs_pdu(0x02, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x05, 0x04, 0xab, 0xcd, 0x00, 0x00));

	/* GetItemAttributes - CT */
	define_test("/TP/MCN/SRC/BV-05-C", test_client,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_GET_ITEM_ATTRIBUTES,
				0x00, 0x0c, AVRCP_MEDIA_SEARCH,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x01,	/* uuid */
				0xaa, 0xbb,		/* counter */
				0x00));			/* num attr */

	/* GetItemAttributes - TG */
	define_test("/TP/MCN/SRC/BV-06-C", test_server,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_GET_ITEM_ATTRIBUTES,
				0x00, 0x0c, AVRCP_MEDIA_SEARCH,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x01,	/* uid */
				0xaa, 0xbb,		/* counter */
				0x00),			/* num attr */
			brs_pdu(0x02, 0x11, 0x0e, AVRCP_GET_ITEM_ATTRIBUTES,
				0x00, 0x02, 0x04, 0x00));

	/* Media Content Navigation Commands and Notifications for NowPlaying */

	/* PlayItem - NowPlaying - CT */
	define_test("/TP/MCN/NP/BV-01-C", test_client,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_PLAY_ITEM,
				0x00, 0x0b, AVRCP_MEDIA_NOW_PLAYING,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
				0x00, 0x01));

	/* PlayItem - NowPlaying - TG */
	define_test("/TP/MCN/NP/BV-02-C", test_server,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_PLAY_ITEM,
				0x00, 0x0b, AVRCP_MEDIA_NOW_PLAYING,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
				0x00, 0x01),
			brs_pdu(0x02, 0x11, 0x0e, AVRCP_PLAY_ITEM,
				0x00, 0x01, 0x04));

	/* AddToNowPlaying - NowPlaying - CT */
	define_test("/TP/MCN/NP/BV-03-C", test_client,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_ADD_TO_NOW_PLAYING,
				0x00, 0x0b, AVRCP_MEDIA_NOW_PLAYING,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x01, /* uid */
				0xaa, 0xbb));

	/* AddToNowPlaying - NowPlaying - TG */
	define_test("/TP/MCN/NP/BV-04-C", test_server,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_ADD_TO_NOW_PLAYING,
				0x00, 0x0b, AVRCP_MEDIA_NOW_PLAYING,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x01, /* uid */
				0xaa, 0xbb),
			brs_pdu(0x02, 0x11, 0x0e, AVRCP_ADD_TO_NOW_PLAYING,
				0x00, 0x01, 0x04));

	/* GetFolderItems - NowPlaying - CT */
	define_test("/TP/MCN/NP/BV-05-C", test_client,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x0a, AVRCP_MEDIA_NOW_PLAYING,
				0x00, 0x00, 0x00, 0x00, /* start */
				0x00, 0x00, 0x00, 0x02, /* end */
				0x00));

	/* GetFolderItems - NowPlaying - TG */
	define_test("/TP/MCN/NP/BV-06-C", test_server,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x0a, AVRCP_MEDIA_NOW_PLAYING,
				0x00, 0x00, 0x00, 0x00, /* start */
				0x00, 0x00, 0x00, 0x02, /* end */
				0x00),
			brs_pdu(0x02, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x05, 0x04, 0xab, 0xcd, 0x00, 0x00));

	/* NowPlayingContentChanged Notification – TG */
	define_test("/TP/MCN/NP/BV-07-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x03, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x05, 0x09,
				0x00, 0x00, 0x00, 0x00),
			frg_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_INTERIM, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x01, 0x09),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_CHANGED, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x01, 0x09));

	/* GetItemAttributes - CT */
	define_test("/TP/MCN/NP/BV-08-C", test_client,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_GET_ITEM_ATTRIBUTES,
				0x00, 0x0c, AVRCP_MEDIA_NOW_PLAYING,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x01,	/* uid */
				0xaa, 0xbb,		/* counter */
				0x00));			/* num attr */

	/* GetItemAttributes - TG */
	define_test("/TP/MCN/CB/BV-09-C", test_server,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_GET_ITEM_ATTRIBUTES,
				0x00, 0x0c, AVRCP_MEDIA_NOW_PLAYING,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x01,	/* uid */
				0xaa, 0xbb,		/* counter */
				0x00),			/* num attr */
			brs_pdu(0x02, 0x11, 0x0e, AVRCP_GET_ITEM_ATTRIBUTES,
				0x00, 0x02, 0x04, 0x00));

	/* PlayItem - NowPlaying - TG */
	define_test("/TP/MCN/NP/BI-01-C", test_server,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_PLAY_ITEM,
				0x00, 0x0b, AVRCP_MEDIA_NOW_PLAYING,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, /* uid */
				0xaa, 0xbb),		/* counter */
			brs_pdu(0x02, 0x11, 0x0e, AVRCP_PLAY_ITEM,
				0x00, 0x01, 0x09));

	/* AddToNowPlaying - NowPlaying - TG */
	define_test("/TP/MCN/NP/BI-02-C", test_server,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_ADD_TO_NOW_PLAYING,
				0x00, 0x0b, AVRCP_MEDIA_NOW_PLAYING,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, /* uid */
				0xaa, 0xbb),		/* counter */
			brs_pdu(0x02, 0x11, 0x0e, AVRCP_ADD_TO_NOW_PLAYING,
				0x00, 0x01, 0x09));

	/* Media Player Selection IOP tests */

	/* Listing of available media players */
	define_test("/TP/MPS/BV-01-I", test_client,
			brs_pdu(0x00, 0x11, 0x0e, AVRCP_GET_FOLDER_ITEMS,
				0x00, 0x0a, AVRCP_MEDIA_PLAYER_LIST,
				0x00, 0x00, 0x00, 0x00, /* start */
				0x00, 0x00, 0x00, 0x02, /* end */
				0x00));

	/* Connection Establishment for Browsing tests */

	/*
	 * Tests are checking connection establishment and release
	 * for browsing channel. Since we are connected through socketpair
	 * the tests are dummy
	 */
	define_test("/TP/CON/BV-01-C", test_dummy, raw_pdu(0x00));
	define_test("/TP/CON/BV-02-C", test_dummy, raw_pdu(0x00));
	define_test("/TP/CON/BV-03-C", test_dummy, raw_pdu(0x00));
	define_test("/TP/CON/BV-04-C", test_dummy, raw_pdu(0x00));
	define_test("/TP/CON/BV-05-C", test_dummy, raw_pdu(0x00));

	/* Connection Establishment for Control tests */

	/*
	 * Tests are checking connection establishement and release
	 * for control channel. Since we are connected through socketpair
	 * the tests are dummy
	 */
	define_test("/TP/CEC/BV-01-I", test_dummy, raw_pdu(0x00));
	define_test("/TP/CEC/BV-02-I", test_dummy, raw_pdu(0x00));
	define_test("/TP/CRC/BV-01-I", test_dummy, raw_pdu(0x00));
	define_test("/TP/CRC/BV-02-I", test_dummy, raw_pdu(0x00));

	/* Information collection for control tests */

	define_test("/TP/ICC/BV-01-I", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0xf8, 0x30,
				0xff, 0xff, 0xff, 0xff, 0xff),
			raw_pdu(0x02, 0x11, 0x0e, 0x0c, 0xf8, 0x30,
				0x07, 0x48, 0xff, 0xff, 0xff));

	define_test("/TP/ICC/BV-02-I", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0xf8, 0x31,
				0x07, 0xff, 0xff, 0xff, 0xff),
			raw_pdu(0x02, 0x11, 0x0e, 0x0c, 0xf8, 0x31,
				0x07, 0x48, 0xff, 0xff, 0xff));

	define_test("/TP/PTT/BV-01-I", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48, 0x7c,
				0x44, 0x00),
			raw_pdu(0x02, 0x11, 0x0e, 0x09, 0x48, 0x7c,
				0x44, 0x00));

	define_test("/TP/PTT/BV-02-I", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48, 0x7c,
				AVC_VOLUME_UP, 0x00),
			raw_pdu(0x02, 0x11, 0x0e, 0x09, 0x48, 0x7c,
				AVC_VOLUME_UP, 0x00));

	define_test("/TP/PTT/BV-03-I", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48, 0x7c,
				AVC_CHANNEL_UP, 0x00),
			raw_pdu(0x02, 0x11, 0x0e, 0x09, 0x48, 0x7c,
				AVC_CHANNEL_UP, 0x00));

	define_test("/TP/PTT/BV-04-I", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48, 0x7c,
				AVC_SELECT, 0x00),
			raw_pdu(0x02, 0x11, 0x0e, 0x09, 0x48, 0x7c,
				AVC_SELECT, 0x00));

	define_test("/TP/PTT/BV-05-I", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48, 0x7c,
				AVC_PLAY, 0x00),
			raw_pdu(0x02, 0x11, 0x0e, 0x09, 0x48, 0x7c,
				AVC_PLAY, 0x00),
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48, 0x7c,
				AVC_PLAY | 0x80, 0x00),
			raw_pdu(0x02, 0x11, 0x0e, 0x09, 0x48, 0x7c,
				AVC_PLAY | 0x80, 0x00));

	/* Metadata transfer tests */

	define_test("/TP/CFG/BV-01-C", test_client,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58, 0x10, 0x00, 0x00,
				0x01, 0x03));

	define_test("/TP/CFG/BV-02-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58, 0x10, 0x00, 0x00,
				0x01, 0x02),
			raw_pdu(0x02, 0x11, 0x0e, 0x0c, 0x48, 0x00,
				0x00, 0x19, 0x58, 0x10, 0x00, 0x00,
				0x05, 0x02, 0x01, 0x00, 0x19, 0x58));

	define_test("/TP/CFG/BI-01-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58, 0x10, 0x00, 0x00,
				0x01, 0x7f),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_REJECTED,
				0x48, 0x00, 0x00, 0x19, 0x58, 0x10,
				0x00, 0x00, 0x01,
				AVRCP_STATUS_INVALID_PARAM));

	/* Player Application Settings tests */

	define_test("/TP/PAS/BV-01-C", test_client,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58, 0x11, 0x00, 0x00,
				0x00));

	define_test("/TP/PAS/BV-02-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58, 0x11, 0x00, 0x00,
				0x00),
			raw_pdu(0x02, 0x11, 0x0e, 0x0c, 0x48, 0x00,
				0x00, 0x19, 0x58, 0x11, 0x00, 0x00,
				0x01, 0x00));

	define_test("/TP/PAS/BV-03-C", test_client,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58,
				AVRCP_GET_PLAYER_ATTRIBUTE_TEXT,
				0x00, 0x00, 0x03, 0x02,
				AVRCP_ATTRIBUTE_EQUALIZER,
				AVRCP_ATTRIBUTE_REPEAT_MODE));

	define_test("/TP/PAS/BV-04-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58,
				AVRCP_GET_PLAYER_ATTRIBUTE_TEXT,
				0x00, 0x00, 0x02, 0x01, 0x01),
			raw_pdu(0x02, 0x11, 0x0e, 0x0c, 0x48, 0x00,
				0x00, 0x19, 0x58,
				AVRCP_GET_PLAYER_ATTRIBUTE_TEXT,
				0x00, 0x00, 0x0e, 0x01, 0x01, 0x00,
				0x6a, 0x09, 0x65, 0x71, 0x75, 0x61,
				0x6c, 0x69, 0x7a, 0x65, 0x72));

	define_test("/TP/PAS/BV-05-C", test_client,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58,
				AVRCP_LIST_PLAYER_VALUES,
				0x00, 0x00, 0x01,
				AVRCP_ATTRIBUTE_EQUALIZER));

	define_test("/TP/PAS/BV-06-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58,
				AVRCP_LIST_PLAYER_VALUES,
				0x00, 0x00, 0x01, AVRCP_ATTRIBUTE_EQUALIZER),
			raw_pdu(0x02, 0x11, 0x0e, 0x0c, 0x48, 0x00,
				0x00, 0x19, 0x58,
				AVRCP_LIST_PLAYER_VALUES,
				0x00, 0x00, 0x01, 0x00));

	define_test("/TP/PAS/BV-07-C", test_client,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58,
				AVRCP_GET_PLAYER_VALUE_TEXT,
				0x00, 0x00, 0x04,
				AVRCP_ATTRIBUTE_EQUALIZER, 0x02,
				AVRCP_EQUALIZER_OFF,
				AVRCP_EQUALIZER_ON));

	define_test("/TP/PAS/BV-08-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58,
				AVRCP_GET_PLAYER_VALUE_TEXT,
				0x00, 0x00, 0x03, AVRCP_ATTRIBUTE_EQUALIZER,
				0x01, 0x01),
			raw_pdu(0x02, 0x11, 0x0e, 0x0c, 0x48, 0x00,
				0x00, 0x19, 0x58,
				AVRCP_GET_PLAYER_VALUE_TEXT,
				0x00, 0x00, 0x07, 0x01, 0x01, 0x00,
				0x6a, 0x02, 0x6f, 0x6e));

	define_test("/TP/PAS/BV-09-C", test_client,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58,
				AVRCP_GET_CURRENT_PLAYER_VALUE,
				0x00, 0x00, 0x03, 0x02,
				AVRCP_ATTRIBUTE_EQUALIZER,
				AVRCP_ATTRIBUTE_REPEAT_MODE));

	define_test("/TP/PAS/BV-10-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58,
				AVRCP_GET_CURRENT_PLAYER_VALUE,
				0x00, 0x00, 0x03, 0x02,
				AVRCP_ATTRIBUTE_EQUALIZER,
				AVRCP_ATTRIBUTE_REPEAT_MODE),
			raw_pdu(0x02, 0x11, 0x0e, 0x0c, 0x48, 0x00,
				0x00, 0x19, 0x58,
				AVRCP_GET_CURRENT_PLAYER_VALUE,
				0x00, 0x00, 0x05, 0x02,
				AVRCP_ATTRIBUTE_EQUALIZER, 0x00,
				AVRCP_ATTRIBUTE_REPEAT_MODE, 0x00));

	define_test("/TP/PAS/BV-11-C", test_client,
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48, 0x00,
				0x00, 0x19, 0x58,
				AVRCP_SET_PLAYER_VALUE,
				0x00, 0x00, 0x05, 0x02,
				AVRCP_ATTRIBUTE_EQUALIZER, 0xaa,
				AVRCP_ATTRIBUTE_REPEAT_MODE, 0xff));

	/* Get player app setting attribute text invalid behavior - TG */
	define_test("/TP/PAS/BI-01-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58,
				AVRCP_GET_PLAYER_ATTRIBUTE_TEXT,
				0x00, 0x00, 0x02, 0x01,
				/* Invalid attribute id */
				0x7f),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_REJECTED,
				0x48, 0x00, 0x00, 0x19, 0x58,
				AVRCP_GET_PLAYER_ATTRIBUTE_TEXT,
				0x00, 0x00, 0x01, AVRCP_STATUS_INVALID_PARAM));

	/* List player application setting values invalid behavior - TG */
	define_test("/TP/PAS/BI-02-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58,
				AVRCP_LIST_PLAYER_VALUES,
				0x00, 0x00, 0x01,
				/* Invalid attribute id */
				0x7f),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_REJECTED,
				0x48, 0x00, 0x00, 0x19, 0x58,
				AVRCP_LIST_PLAYER_VALUES,
				0x00, 0x00, 0x01, AVRCP_STATUS_INVALID_PARAM));

	/* Get player application setting value text invalid behavior - TG */
	define_test("/TP/PAS/BI-03-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58,
				AVRCP_GET_PLAYER_VALUE_TEXT,
				0x00, 0x00, 0x03, AVRCP_ATTRIBUTE_EQUALIZER,
				0x01,
				/* Invalid setting value */
				0x7f),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_REJECTED,
				0x48, 0x00, 0x00, 0x19, 0x58,
				AVRCP_GET_PLAYER_VALUE_TEXT,
				0x00, 0x00, 0x01, AVRCP_STATUS_INVALID_PARAM));

	/* Get current player application setting value invalid behavior - TG */
	define_test("/TP/PAS/BI-04-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58,
				AVRCP_GET_CURRENT_PLAYER_VALUE,
				0x00, 0x00, 0x02, 0x01,
				/* Invalid attribute */
				0x7f),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_REJECTED,
				0x48, 0x00, 0x00, 0x19, 0x58,
				AVRCP_GET_CURRENT_PLAYER_VALUE,
				0x00, 0x00, 0x01, AVRCP_STATUS_INVALID_PARAM));

	/* Set player application setting value invalid behavior - TG */
	define_test("/TP/PAS/BI-05-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48, 0x00,
				0x00, 0x19, 0x58,
				AVRCP_SET_PLAYER_VALUE,
				0x00, 0x00, 0x03, 0x01,
				AVRCP_ATTRIBUTE_REPEAT_MODE, 0x7f),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_REJECTED,
				0x48, 0x00, 0x00, 0x19, 0x58,
				AVRCP_SET_PLAYER_VALUE,
				0x00, 0x00, 0x01, AVRCP_STATUS_INVALID_PARAM));

	/* Media Information Commands */

	/* Get play status - CT */
	define_test("/TP/MDI/BV-01-C", test_client,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_GET_PLAY_STATUS,
				0x00, 0x00, 0x00));

	/* Get play status - TG */
	define_test("/TP/MDI/BV-02-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_GET_PLAY_STATUS,
				0x00, 0x00, 0x00),
			raw_pdu(0x02, 0x11, 0x0e, 0x0c, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_GET_PLAY_STATUS,
				0x00, 0x00, 0x09,
				0xbb, 0xbb, 0xbb, 0xbb, /* duration */
				0xaa, 0xaa, 0xaa, 0xaa, /* position */
				0x00));

	/* Get element attributes - CT */
	define_test("/TP/MDI/BV-03-C", test_client,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_GET_ELEMENT_ATTRIBUTES,
				0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00));

	/* Get element attributes - TG */
	define_test("/TP/MDI/BV-04-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_GET_ELEMENT_ATTRIBUTES,
				0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00),
			raw_pdu(0x02, 0x11, 0x0e, 0x0c, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_GET_ELEMENT_ATTRIBUTES,
				0x00, 0x00, 0x00));

	/* Get element attributes - TG */
	define_test("/TP/MDI/BV-05-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_GET_ELEMENT_ATTRIBUTES,
				0x00, 0x00, 0x0d, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
				0x00, 0x00, 0x00, 0x01),
			raw_pdu(0x02, 0x11, 0x0e, 0x0c, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_GET_ELEMENT_ATTRIBUTES,
				0x00, 0x00, 0x00));

	/* Notification Commands */

	/* Register notification - CT */
	define_test("/TP/NFY/BV-01-C", test_client,
			raw_pdu(0x00, 0x11, 0x0e, 0x03, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x05, AVRCP_EVENT_STATUS_CHANGED,
				0x00, 0x00, 0x00, 0x00));

	/* Register notification - TG */
	define_test("/TP/NFY/BV-02-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x03, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x05, AVRCP_EVENT_TRACK_CHANGED,
				0x00, 0x00, 0x00, 0x00),
			frg_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_INTERIM, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x09, AVRCP_EVENT_TRACK_CHANGED,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_CHANGED, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x09, AVRCP_EVENT_TRACK_CHANGED,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff));

	/* Register notification - TG */
	define_test("/TP/NFY/BV-03-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x03, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x05,
				AVRCP_EVENT_SETTINGS_CHANGED,
				0x00, 0x00, 0x00, 0x00),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_INTERIM, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x04,
				AVRCP_EVENT_SETTINGS_CHANGED,
				0x01, 0x01, 0x02),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_CHANGED, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x04,
				AVRCP_EVENT_SETTINGS_CHANGED,
				0x01, 0x01, 0x02));

	/* Register notification - Track Changed - No Selected Track - TG */
	define_test("/TP/NFY/BV-04-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x03, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x05, AVRCP_EVENT_TRACK_CHANGED,
				0x00, 0x00, 0x00, 0x00),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_INTERIM, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x09, AVRCP_EVENT_TRACK_CHANGED,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff));

	/* Register notification - Track Changed - Track Playing - TG */
	define_test("/TP/NFY/BV-05-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x03, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x05, AVRCP_EVENT_TRACK_CHANGED,
				0x00, 0x00, 0x00, 0x00),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_INTERIM, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x09, AVRCP_EVENT_TRACK_CHANGED,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00));

	/* Register notification - Track Changed - Selected Track - TG */
	define_test("/TP/NFY/BV-08-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x03, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x05, AVRCP_EVENT_TRACK_CHANGED,
				0x00, 0x00, 0x00, 0x00),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_INTERIM, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x09, AVRCP_EVENT_TRACK_CHANGED,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00));

	/* Register notification - Register for events invalid behavior - TG */
	define_test("/TP/NFY/BI-01-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x03, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x05,
				/* Invalid event id */
				0xff,
				0x00, 0x00, 0x00, 0x00),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_REJECTED,
				0x48, 0x00, 0x00, 0x19, 0x58,
				AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x01, AVRCP_STATUS_INVALID_PARAM));

	/* Invalid commands */

	/* Invalid PDU ID - TG */
	define_test("/TP/INV/BI-01-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x03, 0x48, 0x00,
				0x00, 0x19, 0x58,
				/* Invalid PDU ID */
				0xff,
				0x00, 0x00, 0x00),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_REJECTED,
				0x48, 0x00, 0x00, 0x19, 0x58,
				0xff, 0x00, 0x00, 0x01,
				AVRCP_STATUS_INVALID_COMMAND));

	/* Invalid PDU ID - Browsing TG */
	define_test("/TP/INV/BI-02-C", test_server,
			brs_pdu(0x00, 0x11, 0x0e, 0xff, 0x00, 0x00),
			brs_pdu(0x02, 0x11, 0x0e, AVRCP_GENERAL_REJECT,
				0x00, 0x01, AVRCP_STATUS_INVALID_COMMAND));

	/* Next Group command transfer - CT */
	define_test("/TP/BGN/BV-01-I", test_client,
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48,
				AVC_OP_PASSTHROUGH,
				AVC_VENDOR_UNIQUE, 0x05, 0x00, 0x19,
				0x58, 0x00, AVC_VENDOR_NEXT_GROUP));

	/* Next Group command transfer - TG */
	define_test("/TP/BGN/BV-01-I", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48,
				AVC_OP_PASSTHROUGH,
				AVC_VENDOR_UNIQUE, 0x05, 0x00, 0x19,
				0x58, 0x00, AVC_VENDOR_NEXT_GROUP),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_ACCEPTED,
				0x48, AVC_OP_PASSTHROUGH,
				AVC_VENDOR_UNIQUE, 0x05, 0x00, 0x19,
				0x58, 0x00, AVC_VENDOR_NEXT_GROUP));

	/* Previous Group command transfer - CT */
	define_test("/TP/BGN/BV-02-I", test_client,
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48,
				AVC_OP_PASSTHROUGH,
				AVC_VENDOR_UNIQUE, 0x05, 0x00, 0x19,
				0x58, 0x00, AVC_VENDOR_PREV_GROUP));

	/* Previous Group command transfer - TG */
	define_test("/TP/BGN/BV-02-I", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48,
				AVC_OP_PASSTHROUGH,
				AVC_VENDOR_UNIQUE, 0x05, 0x00, 0x19,
				0x58, 0x00, AVC_VENDOR_PREV_GROUP),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_ACCEPTED,
				0x48, AVC_OP_PASSTHROUGH,
				AVC_VENDOR_UNIQUE, 0x05, 0x00, 0x19,
				0x58, 0x00, AVC_VENDOR_PREV_GROUP));

	/* Volume Level Handling */

	/* Set absolute volume – CT */
	define_test("/TP/VLH/BV-01-C", test_client,
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_SET_ABSOLUTE_VOLUME,
				0x00, 0x00, 0x01, 0x00));

	/* Set absolute volume – TG */
	define_test("/TP/VLH/BV-02-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_SET_ABSOLUTE_VOLUME,
				0x00, 0x00, 0x01, 0x00),
			raw_pdu(0x02, 0x11, 0x0e, 0x0c, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_SET_ABSOLUTE_VOLUME,
				0x00, 0x00, 0x01, 0x00));

	/* NotifyVolumeChange - CT */
	define_test("/TP/VLH/BV-03-C", test_client,
			raw_pdu(0x00, 0x11, 0x0e, 0x03, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x05, 0x0d,
				0x00, 0x00, 0x00, 0x00),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_INTERIM, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x02, 0x0d,
				0x00));

	/* NotifyVolumeChange - TG */
	define_test("/TP/VLH/BV-04-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x03, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x05, 0x0d,
				0x00, 0x00, 0x00, 0x00),
			frg_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_INTERIM, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x02, 0x0d,
				0x00),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_CHANGED, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x02, 0x0d,
				0x01));

	/* Set absolute volume – TG */
	define_test("/TP/VLH/BI-01-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_SET_ABSOLUTE_VOLUME,
				0x00, 0x00, 0x00),
			raw_pdu(0x02, 0x11, 0x0e, 0x0a, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_SET_ABSOLUTE_VOLUME,
				0x00, 0x00, 0x01, 0x01));

	/* Set absolute volume – TG */
	define_test("/TP/VLH/BI-02-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_SET_ABSOLUTE_VOLUME,
				0x00, 0x00, 0x01, 0x80),
			raw_pdu(0x02, 0x11, 0x0e, 0x0c, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_SET_ABSOLUTE_VOLUME,
				0x00, 0x00, 0x01, 0x00));

	/* Set Absolute Volume invalid behavior CT */
	define_test("/TP/VLH/BI-03-C", test_client,
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_SET_ABSOLUTE_VOLUME,
				0x00, 0x00, 0x01, 0x01),
			raw_pdu(0x02, 0x11, 0x0e, 0x0c, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_SET_ABSOLUTE_VOLUME,
				0x00, 0x00, 0x01, 0x81));

	/* Set Absolute Volume invalid behavior CT */
	define_test("/TP/VLH/BI-04-C", test_client,
			raw_pdu(0x00, 0x11, 0x0e, 0x03, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x05, 0x0d,
				0x00, 0x00, 0x00, 0x00),
			frg_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_INTERIM, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x02, 0x0d,
				0x00),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_CHANGED, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REGISTER_NOTIFICATION,
				0x00, 0x00, 0x02, 0x0d,
				0x81));

	/* PASS THROUGH Handling */

	/* Press and release – CT */
	define_test("/TP/PTH/BV-01-C", test_client,
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48,
				AVC_OP_PASSTHROUGH, AVC_PLAY, 0x00),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_ACCEPTED, 0x48,
				AVC_OP_PASSTHROUGH, AVC_PLAY),
			raw_pdu(0x10, 0x11, 0x0e, 0x00, 0x48,
				AVC_OP_PASSTHROUGH, AVC_PLAY | 0x80, 0x00));

	define_test("/TP/PTH/BV-02-C", test_client,
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48,
				AVC_OP_PASSTHROUGH, AVC_FAST_FORWARD, 0x00),
			raw_pdu(0x02, 0x11, 0x0e, AVC_CTYPE_ACCEPTED, 0x48,
				AVC_OP_PASSTHROUGH, AVC_FAST_FORWARD),
			raw_pdu(0x10, 0x11, 0x0e, 0x00, 0x48,
				AVC_OP_PASSTHROUGH, AVC_FAST_FORWARD | 0x80,
				0x00),
			raw_pdu(0x12, 0x11, 0x0e, AVC_CTYPE_ACCEPTED, 0x48,
				AVC_OP_PASSTHROUGH, AVC_FAST_FORWARD | 0x80),
			raw_pdu(0x20, 0x11, 0x0e, 0x00, 0x48,
				AVC_OP_PASSTHROUGH, AVC_FAST_FORWARD, 0x00));

	/* Request continuing response - TG */
	define_test("/TP/RCR/BV-02-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_GET_ELEMENT_ATTRIBUTES,
				0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00),
			cont_pdu(0x02, 0x11, 0x0e, 0x0c, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_GET_ELEMENT_ATTRIBUTES,
				0x01, 0x01, 0xf9),
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REQUEST_CONTINUING,
				0x00, 0x00, 0x01, AVRCP_GET_ELEMENT_ATTRIBUTES),
			cont_pdu(0x02, 0x11, 0x0e, 0x0c, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_GET_ELEMENT_ATTRIBUTES,
				0x02, 0x01, 0xf9),
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_REQUEST_CONTINUING,
				0x00, 0x00, 0x01, AVRCP_GET_ELEMENT_ATTRIBUTES),
			cont_pdu(0x02, 0x11, 0x0e, 0x0c, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_GET_ELEMENT_ATTRIBUTES,
				0x03, 0x00, 0x0e));

	/* Abort continuing response - TG */
	define_test("/TP/RCR/BV-04-C", test_server,
			raw_pdu(0x00, 0x11, 0x0e, 0x01, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_GET_ELEMENT_ATTRIBUTES,
				0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00),
			cont_pdu(0x02, 0x11, 0x0e, 0x0c, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_GET_ELEMENT_ATTRIBUTES,
				0x01, 0x01, 0xf9),
			raw_pdu(0x00, 0x11, 0x0e, 0x00, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_ABORT_CONTINUING,
				0x00, 0x00, 0x01, AVRCP_GET_ELEMENT_ATTRIBUTES),
			raw_pdu(0x02, 0x11, 0x0e, 0x09, 0x48, 0x00,
				0x00, 0x19, 0x58, AVRCP_ABORT_CONTINUING,
				0x00, 0x00, 0x00));

	return tester_run();
}
