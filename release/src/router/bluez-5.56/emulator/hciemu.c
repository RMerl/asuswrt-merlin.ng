// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/socket.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"

#include "monitor/bt.h"
#include "emulator/btdev.h"
#include "emulator/bthost.h"
#include "src/shared/util.h"
#include "src/shared/queue.h"
#include "emulator/hciemu.h"

struct hciemu_client {
	struct bthost *host;
	struct btdev *dev;
	guint start_source;
	guint host_source;
	guint source;
};

struct hciemu {
	int ref_count;
	enum btdev_type btdev_type;
	struct btdev *dev;
	struct queue *clients;
	guint source;
	struct queue *post_command_hooks;
	char bdaddr_str[18];

	hciemu_debug_func_t debug_callback;
	hciemu_destroy_func_t debug_destroy;
	void *debug_data;
};

struct hciemu_command_hook {
	hciemu_command_func_t function;
	void *user_data;
};

static void destroy_command_hook(void *data)
{
	struct hciemu_command_hook *hook = data;

	free(hook);
}

struct run_data {
	uint16_t opcode;
	const void *data;
	uint8_t len;
};

static void run_command_hook(void *data, void *user_data)
{
	struct hciemu_command_hook *hook = data;
	struct run_data *run_data = user_data;

	if (hook->function)
		hook->function(run_data->opcode, run_data->data,
					run_data->len, hook->user_data);
}

static void master_command_callback(uint16_t opcode,
				const void *data, uint8_t len,
				btdev_callback callback, void *user_data)
{
	struct hciemu *hciemu = user_data;
	struct run_data run_data = { .opcode = opcode,
						.data = data, .len = len };

	btdev_command_default(callback);

	queue_foreach(hciemu->post_command_hooks, run_command_hook, &run_data);
}

static void client_command_callback(uint16_t opcode,
				const void *data, uint8_t len,
				btdev_callback callback, void *user_data)
{
	btdev_command_default(callback);
}

static void writev_callback(const struct iovec *iov, int iovlen,
								void *user_data)
{
	GIOChannel *channel = user_data;
	ssize_t written;
	int fd;

	fd = g_io_channel_unix_get_fd(channel);

	written = writev(fd, iov, iovlen);
	if (written < 0)
		return;
}

static gboolean receive_bthost(GIOChannel *channel, GIOCondition condition,
							gpointer user_data)
{
	struct bthost *bthost = user_data;
	unsigned char buf[4096];
	ssize_t len;
	int fd;

	if (condition & (G_IO_NVAL | G_IO_ERR | G_IO_HUP))
		return FALSE;

	fd = g_io_channel_unix_get_fd(channel);

	len = read(fd, buf, sizeof(buf));
	if (len < 0)
		return FALSE;

	bthost_receive_h4(bthost, buf, len);

	return TRUE;
}

static guint create_source_bthost(int fd, struct bthost *bthost)
{
	GIOChannel *channel;
	guint source;

	channel = g_io_channel_unix_new(fd);

	g_io_channel_set_close_on_unref(channel, TRUE);
	g_io_channel_set_encoding(channel, NULL, NULL);
	g_io_channel_set_buffered(channel, FALSE);

	bthost_set_send_handler(bthost, writev_callback, channel);

	source = g_io_add_watch_full(channel, G_PRIORITY_DEFAULT,
				G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				receive_bthost, bthost, NULL);

	g_io_channel_unref(channel);

	return source;
}

static gboolean receive_btdev(GIOChannel *channel, GIOCondition condition,
							gpointer user_data)
{
	struct btdev *btdev = user_data;
	unsigned char buf[4096];
	ssize_t len;
	int fd;

	if (condition & (G_IO_NVAL | G_IO_ERR | G_IO_HUP))
		return FALSE;

	fd = g_io_channel_unix_get_fd(channel);

	len = read(fd, buf, sizeof(buf));
	if (len < 0) {
		if (errno == EAGAIN || errno == EINTR)
			return TRUE;

		return FALSE;
	}

	if (len < 1)
		return FALSE;

	switch (buf[0]) {
	case BT_H4_CMD_PKT:
	case BT_H4_ACL_PKT:
	case BT_H4_SCO_PKT:
		btdev_receive_h4(btdev, buf, len);
		break;
	}

	return TRUE;
}

static guint create_source_btdev(int fd, struct btdev *btdev)
{
	GIOChannel *channel;
	guint source;

	channel = g_io_channel_unix_new(fd);

	g_io_channel_set_close_on_unref(channel, TRUE);
	g_io_channel_set_encoding(channel, NULL, NULL);
	g_io_channel_set_buffered(channel, FALSE);

	btdev_set_send_handler(btdev, writev_callback, channel);

	source = g_io_add_watch_full(channel, G_PRIORITY_DEFAULT,
				G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				receive_btdev, btdev, NULL);

	g_io_channel_unref(channel);

	return source;
}

static bool create_vhci(struct hciemu *hciemu)
{
	struct btdev *btdev;
	uint8_t create_req[2];
	ssize_t written;
	int fd;

	btdev = btdev_create(hciemu->btdev_type, 0x00);
	if (!btdev)
		return false;

	btdev_set_command_handler(btdev, master_command_callback, hciemu);

	fd = open("/dev/vhci", O_RDWR | O_NONBLOCK | O_CLOEXEC);
	if (fd < 0) {
		perror("Opening /dev/vhci failed");
		btdev_destroy(btdev);
		return false;
	}

	create_req[0] = HCI_VENDOR_PKT;
	create_req[1] = HCI_PRIMARY;

	written = write(fd, create_req, sizeof(create_req));
	if (written < 0) {
		close(fd);
		btdev_destroy(btdev);
		return false;
	}

	hciemu->dev = btdev;

	hciemu->source = create_source_btdev(fd, btdev);

	return true;
}

struct hciemu_client *hciemu_get_client(struct hciemu *hciemu, int num)
{
	const struct queue_entry *entry;

	if (!hciemu)
		return NULL;

	for (entry = queue_get_entries(hciemu->clients); entry;
					entry = entry->next, num--) {
		if (!num)
			return entry->data;
	}

	return NULL;
}

struct bthost *hciemu_client_host(struct hciemu_client *client)
{
	if (!client)
		return NULL;

	return client->host;
}

struct bthost *hciemu_client_get_host(struct hciemu *hciemu)
{
	struct hciemu_client *client;

	if (!hciemu)
		return NULL;

	client = hciemu_get_client(hciemu, 0);

	return hciemu_client_host(client);
}

static gboolean start_host(gpointer user_data)
{
	struct hciemu_client *client = user_data;

	client->start_source = 0;

	bthost_start(client->host);

	return FALSE;
}

static void hciemu_client_destroy(void *data)
{
	struct hciemu_client *client = data;

	if (client->start_source)
		g_source_remove(client->start_source);

	g_source_remove(client->host_source);
	g_source_remove(client->source);

	bthost_destroy(client->host);
	btdev_destroy(client->dev);

	free(client);
}

static struct hciemu_client *hciemu_client_new(struct hciemu *hciemu,
							uint8_t id)
{
	struct hciemu_client *client;
	int sv[2];

	client = new0(struct hciemu_client, 1);
	if (!client)
		return NULL;

	client->dev = btdev_create(hciemu->btdev_type, id++);
	if (!client->dev) {
		free(client);
		return NULL;
	}

	client->host = bthost_create();
	if (!client->host) {
		btdev_destroy(client->dev);
		free(client);
		return NULL;
	}

	btdev_set_command_handler(client->dev, client_command_callback, client);

	if (socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_NONBLOCK | SOCK_CLOEXEC,
								0, sv) < 0) {
		bthost_destroy(client->host);
		btdev_destroy(client->dev);
		return NULL;
	}

	client->source = create_source_btdev(sv[0], client->dev);
	client->host_source = create_source_bthost(sv[1], client->host);
	client->start_source = g_idle_add(start_host, client);

	return client;
}

struct hciemu *hciemu_new_num(enum hciemu_type type, uint8_t num)
{

	struct hciemu *hciemu;
	int i;

	if (!num)
		return NULL;

	hciemu = new0(struct hciemu, 1);
	if (!hciemu)
		return NULL;

	switch (type) {
	case HCIEMU_TYPE_BREDRLE:
		hciemu->btdev_type = BTDEV_TYPE_BREDRLE;
		break;
	case HCIEMU_TYPE_BREDR:
		hciemu->btdev_type = BTDEV_TYPE_BREDR;
		break;
	case HCIEMU_TYPE_LE:
		hciemu->btdev_type = BTDEV_TYPE_LE;
		break;
	case HCIEMU_TYPE_LEGACY:
		hciemu->btdev_type = BTDEV_TYPE_BREDR20;
		break;
	case HCIEMU_TYPE_BREDRLE50:
		hciemu->btdev_type = BTDEV_TYPE_BREDRLE50;
		break;
	case HCIEMU_TYPE_BREDRLE52:
		hciemu->btdev_type = BTDEV_TYPE_BREDRLE52;
		break;
	default:
		return NULL;
	}

	hciemu->post_command_hooks = queue_new();
	if (!hciemu->post_command_hooks) {
		free(hciemu);
		return NULL;
	}

	if (!create_vhci(hciemu)) {
		queue_destroy(hciemu->post_command_hooks, NULL);
		free(hciemu);
		return NULL;
	}

	hciemu->clients = queue_new();

	for (i = 0; i < num; i++) {
		struct hciemu_client *client = hciemu_client_new(hciemu, i);

		if (!client) {
			queue_destroy(hciemu->clients, hciemu_client_destroy);
			break;
		}

		queue_push_tail(hciemu->clients, client);
	}

	return hciemu_ref(hciemu);
}

struct hciemu *hciemu_new(enum hciemu_type type)
{
	return hciemu_new_num(type, 1);
}

struct hciemu *hciemu_ref(struct hciemu *hciemu)
{
	if (!hciemu)
		return NULL;

	__sync_fetch_and_add(&hciemu->ref_count, 1);

	return hciemu;
}

void hciemu_unref(struct hciemu *hciemu)
{
	if (!hciemu)
		return;

	if (__sync_sub_and_fetch(&hciemu->ref_count, 1))
		return;

	queue_destroy(hciemu->post_command_hooks, destroy_command_hook);
	queue_destroy(hciemu->clients, hciemu_client_destroy);

	g_source_remove(hciemu->source);
	btdev_destroy(hciemu->dev);

	free(hciemu);
}

static void bthost_debug(const char *str, void *user_data)
{
	struct hciemu *hciemu = user_data;

	util_debug(hciemu->debug_callback, hciemu->debug_data,
					"bthost: %s", str);
}

static void btdev_master_debug(const char *str, void *user_data)
{
	struct hciemu *hciemu = user_data;

	util_debug(hciemu->debug_callback, hciemu->debug_data,
					"btdev: %s", str);
}

static void btdev_client_debug(const char *str, void *user_data)
{
	struct hciemu *hciemu = user_data;

	util_debug(hciemu->debug_callback, hciemu->debug_data,
					"btdev[bthost]: %s", str);
}

static void hciemu_client_set_debug(void *data, void *user_data)
{
	struct hciemu_client *client = data;
	struct hciemu *hciemu = user_data;

	btdev_set_debug(client->dev, btdev_client_debug, hciemu, NULL);
	bthost_set_debug(client->host, bthost_debug, hciemu, NULL);
}

bool hciemu_set_debug(struct hciemu *hciemu, hciemu_debug_func_t callback,
			void *user_data, hciemu_destroy_func_t destroy)
{
	if (!hciemu)
		return false;

	if (hciemu->debug_destroy)
		hciemu->debug_destroy(hciemu->debug_data);

	hciemu->debug_callback = callback;
	hciemu->debug_destroy = destroy;
	hciemu->debug_data = user_data;

	btdev_set_debug(hciemu->dev, btdev_master_debug, hciemu, NULL);

	queue_foreach(hciemu->clients, hciemu_client_set_debug, hciemu);

	return true;
}

const char *hciemu_get_address(struct hciemu *hciemu)
{
	const uint8_t *addr;

	if (!hciemu || !hciemu->dev)
		return NULL;

	addr = btdev_get_bdaddr(hciemu->dev);
	sprintf(hciemu->bdaddr_str, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
			addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
	return hciemu->bdaddr_str;
}

uint8_t *hciemu_get_features(struct hciemu *hciemu)
{
	if (!hciemu || !hciemu->dev)
		return NULL;

	return btdev_get_features(hciemu->dev);
}

const uint8_t *hciemu_get_master_bdaddr(struct hciemu *hciemu)
{
	if (!hciemu || !hciemu->dev)
		return NULL;

	return btdev_get_bdaddr(hciemu->dev);
}

const uint8_t *hciemu_client_bdaddr(struct hciemu_client *client)
{
	if (!client)
		return NULL;

	return btdev_get_bdaddr(client->dev);
}

const uint8_t *hciemu_get_client_bdaddr(struct hciemu *hciemu)
{
	struct hciemu_client *client;

	if (!hciemu)
		return NULL;

	client = hciemu_get_client(hciemu, 0);

	return hciemu_client_bdaddr(client);
}

uint8_t hciemu_get_master_scan_enable(struct hciemu *hciemu)
{
	if (!hciemu || !hciemu->dev)
		return 0;

	return btdev_get_scan_enable(hciemu->dev);
}

uint8_t hciemu_get_master_le_scan_enable(struct hciemu *hciemu)
{
	if (!hciemu || !hciemu->dev)
		return 0;

	return btdev_get_le_scan_enable(hciemu->dev);
}

void hciemu_set_master_le_states(struct hciemu *hciemu, const uint8_t *le_states)
{
	if (!hciemu || !hciemu->dev)
		return;

	btdev_set_le_states(hciemu->dev, le_states);
}

bool hciemu_add_master_post_command_hook(struct hciemu *hciemu,
			hciemu_command_func_t function, void *user_data)
{
	struct hciemu_command_hook *hook;

	if (!hciemu)
		return false;

	hook = new0(struct hciemu_command_hook, 1);
	if (!hook)
		return false;

	hook->function = function;
	hook->user_data = user_data;

	if (!queue_push_tail(hciemu->post_command_hooks, hook)) {
		free(hook);
		return false;
	}

	return true;
}

bool hciemu_clear_master_post_command_hooks(struct hciemu *hciemu)
{
	if (!hciemu)
		return false;

	queue_remove_all(hciemu->post_command_hooks,
					NULL, NULL, destroy_command_hook);
	return true;
}

int hciemu_add_hook(struct hciemu *hciemu, enum hciemu_hook_type type,
				uint16_t opcode, hciemu_hook_func_t function,
				void *user_data)
{
	enum btdev_hook_type hook_type;

	if (!hciemu)
		return -1;

	switch (type) {
	case HCIEMU_HOOK_PRE_CMD:
		hook_type = BTDEV_HOOK_PRE_CMD;
		break;
	case HCIEMU_HOOK_POST_CMD:
		hook_type = BTDEV_HOOK_POST_CMD;
		break;
	case HCIEMU_HOOK_PRE_EVT:
		hook_type = BTDEV_HOOK_PRE_EVT;
		break;
	case HCIEMU_HOOK_POST_EVT:
		hook_type = BTDEV_HOOK_POST_EVT;
		break;
	default:
		return -1;
	}

	return btdev_add_hook(hciemu->dev, hook_type, opcode, function,
								user_data);
}

bool hciemu_del_hook(struct hciemu *hciemu, enum hciemu_hook_type type,
								uint16_t opcode)
{
	enum btdev_hook_type hook_type;

	if (!hciemu)
		return false;

	switch (type) {
	case HCIEMU_HOOK_PRE_CMD:
		hook_type = BTDEV_HOOK_PRE_CMD;
		break;
	case HCIEMU_HOOK_POST_CMD:
		hook_type = BTDEV_HOOK_POST_CMD;
		break;
	case HCIEMU_HOOK_PRE_EVT:
		hook_type = BTDEV_HOOK_PRE_EVT;
		break;
	case HCIEMU_HOOK_POST_EVT:
		hook_type = BTDEV_HOOK_POST_EVT;
		break;
	default:
		return false;
	}

	return btdev_del_hook(hciemu->dev, hook_type, opcode);
}
