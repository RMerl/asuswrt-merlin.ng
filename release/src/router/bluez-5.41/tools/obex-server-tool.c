/*
 *
 *  OBEX library with GLib integration
 *
 *  Copyright (C) 2011  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
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
# include "config.h"
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "gobex/gobex.h"
#include "btio/btio.h"

static GMainLoop *main_loop = NULL;

static GSList *clients = NULL;

static gboolean option_packet = FALSE;
static gboolean option_bluetooth = FALSE;
static int option_channel = -1;
static int option_imtu = -1;
static int option_omtu = -1;
static char *option_root = NULL;

static void sig_term(int sig)
{
	g_print("Terminating due to signal %d\n", sig);
	g_main_loop_quit(main_loop);
}

static GOptionEntry options[] = {
	{ "unix", 'u', G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE,
			&option_bluetooth, "Use a UNIX socket" },
	{ "bluetooth", 'b', 0, G_OPTION_ARG_NONE,
			&option_bluetooth, "Use Bluetooth" },
	{ "channel", 'c', 0, G_OPTION_ARG_INT,
			&option_channel, "Transport channel", "CHANNEL" },
	{ "packet", 'p', 0, G_OPTION_ARG_NONE,
			&option_packet, "Packet based transport" },
	{ "stream", 's', G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE,
			&option_packet, "Stream based transport" },
	{ "root", 'r', 0, G_OPTION_ARG_STRING,
			&option_root, "Root dir", "/..." },
	{ "input-mtu", 'i', 0, G_OPTION_ARG_INT,
			&option_imtu, "Transport input MTU", "MTU" },
	{ "output-mtu", 'o', 0, G_OPTION_ARG_INT,
			&option_omtu, "Transport output MTU", "MTU" },
	{ NULL },
};

static void disconn_func(GObex *obex, GError *err, gpointer user_data)
{
	g_print("Client disconnected: %s\n", err ? err->message : "<no err>");
	clients = g_slist_remove(clients, obex);
	g_obex_unref(obex);
}

struct transfer_data {
	int fd;
};

static void transfer_complete(GObex *obex, GError *err, gpointer user_data)
{
	struct transfer_data *data = user_data;

	if (err != NULL)
		g_printerr("transfer failed: %s\n", err->message);
	else
		g_print("transfer succeeded\n");

	close(data->fd);
	g_free(data);
}

static gboolean recv_data(const void *buf, gsize len, gpointer user_data)
{
	struct transfer_data *data = user_data;

	g_print("received %zu bytes of data\n", len);

	if (write(data->fd, buf, len) < 0) {
		g_printerr("write: %s\n", strerror(errno));
		return FALSE;
	}

	return TRUE;
}

static void handle_put(GObex *obex, GObexPacket *req, gpointer user_data)
{
	GError *err = NULL;
	GObexHeader *hdr;
	const char *type, *name;
	struct transfer_data *data;
	gsize type_len;

	hdr = g_obex_packet_get_header(req, G_OBEX_HDR_TYPE);
	if (hdr != NULL) {
		g_obex_header_get_bytes(hdr, (const guint8 **) &type,
								&type_len);
		if (type[type_len - 1] != '\0') {
			g_printerr("non-nul terminated type header\n");
			type = NULL;
		}
	} else
		type = NULL;

	hdr = g_obex_packet_get_header(req, G_OBEX_HDR_NAME);
	if (hdr != NULL)
		g_obex_header_get_unicode(hdr, &name);
	else
		name = NULL;

	g_print("put type \"%s\" name \"%s\"\n", type ? type : "",
							name ? name : "");

	data = g_new0(struct transfer_data, 1);

	data->fd = open(name, O_WRONLY | O_CREAT | O_NOCTTY, 0600);
	if (data->fd < 0) {
		g_printerr("open(%s): %s\n", name, strerror(errno));
		g_free(data);
		g_obex_send_rsp(obex, G_OBEX_RSP_FORBIDDEN, NULL,
							G_OBEX_HDR_INVALID);
		return;
	}

	g_obex_put_rsp(obex, req, recv_data, transfer_complete, data, &err,
							G_OBEX_HDR_INVALID);
	if (err != NULL) {
		g_printerr("Unable to send response: %s\n", err->message);
		g_error_free(err);
		g_free(data);
	}
}

static gssize send_data(void *buf, gsize len, gpointer user_data)
{
	struct transfer_data *data = user_data;
	gssize ret;

	ret = read(data->fd, buf, len);
	g_print("sending %zu bytes of data\n", ret);

	return ret;
}

static void handle_get(GObex *obex, GObexPacket *req, gpointer user_data)
{
	GError *err = NULL;
	struct transfer_data *data;
	const char *type, *name;
	GObexHeader *hdr;
	gsize type_len;

	hdr = g_obex_packet_get_header(req, G_OBEX_HDR_TYPE);
	if (hdr != NULL) {
		g_obex_header_get_bytes(hdr, (const guint8 **) &type,
								&type_len);
		if (type[type_len - 1] != '\0') {
			g_printerr("non-nul terminated type header\n");
			type = NULL;
		}
	} else
		type = NULL;

	hdr = g_obex_packet_get_header(req, G_OBEX_HDR_NAME);
	if (hdr != NULL)
		g_obex_header_get_unicode(hdr, &name);
	else
		name = NULL;

	g_print("get type \"%s\" name \"%s\"\n", type ? type : "",
							name ? name : "");

	data = g_new0(struct transfer_data, 1);

	data->fd = open(name, O_RDONLY | O_NOCTTY, 0);
	if (data->fd < 0) {
		g_printerr("open(%s): %s\n", name, strerror(errno));
		g_free(data);
		g_obex_send_rsp(obex, G_OBEX_RSP_FORBIDDEN, NULL,
							G_OBEX_HDR_INVALID);
		return;
	}

	g_obex_get_rsp(obex, send_data, transfer_complete, data, &err,
							G_OBEX_HDR_INVALID);
	if (err != NULL) {
		g_printerr("Unable to send response: %s\n", err->message);
		g_error_free(err);
		g_free(data);
	}
}

static void handle_connect(GObex *obex, GObexPacket *req, gpointer user_data)
{
	GObexPacket *rsp;

	g_print("connect\n");

	rsp = g_obex_packet_new(G_OBEX_RSP_SUCCESS, TRUE, G_OBEX_HDR_INVALID);
	g_obex_send(obex, rsp, NULL);
}

static void transport_accept(GIOChannel *io)
{
	GObex *obex;
	GObexTransportType transport;

	g_io_channel_set_flags(io, G_IO_FLAG_NONBLOCK, NULL);
	g_io_channel_set_close_on_unref(io, TRUE);

	if (option_packet)
		transport = G_OBEX_TRANSPORT_PACKET;
	else
		transport = G_OBEX_TRANSPORT_STREAM;

	obex = g_obex_new(io, transport, option_imtu, option_omtu);
	g_obex_set_disconnect_function(obex, disconn_func, NULL);
	g_obex_add_request_function(obex, G_OBEX_OP_PUT, handle_put, NULL);
	g_obex_add_request_function(obex, G_OBEX_OP_GET, handle_get, NULL);
	g_obex_add_request_function(obex, G_OBEX_OP_CONNECT, handle_connect,
									NULL);
	clients = g_slist_append(clients, obex);
}

static gboolean unix_accept(GIOChannel *chan, GIOCondition cond, gpointer data)
{
	struct sockaddr_un addr;
	socklen_t addrlen;
	int sk, cli_sk;
	GIOChannel *io;

	if (cond & G_IO_NVAL)
		return FALSE;

	if (cond & (G_IO_HUP | G_IO_ERR)) {
		g_io_channel_shutdown(chan, TRUE, NULL);
		return FALSE;
	}

	sk = g_io_channel_unix_get_fd(chan);

	memset(&addr, 0, sizeof(addr));
	addrlen = sizeof(addr);

	cli_sk = accept(sk, (struct sockaddr *) &addr, &addrlen);
	if (cli_sk < 0) {
		g_printerr("accept: %s (%d)\n", strerror(errno), errno);
		return TRUE;
	}

	g_print("Accepted new client connection on unix socket (fd=%d)\n",
								cli_sk);

	io = g_io_channel_unix_new(cli_sk);

	transport_accept(io);
	g_io_channel_unref(io);

	return TRUE;
}

static void bluetooth_accept(GIOChannel *io, GError *err, gpointer data)
{
	if (err) {
		g_printerr("accept: %s\n", err->message);
		return;
	}

	g_print("Accepted new client connection on bluetooth socket\n");

	transport_accept(io);
}

static gboolean bluetooth_watch(GIOChannel *chan, GIOCondition cond, gpointer data)
{
	if (cond & G_IO_NVAL)
		return FALSE;

	g_io_channel_shutdown(chan, TRUE, NULL);
	return FALSE;
}

static GIOChannel *l2cap_listen(GError **err)
{
	return bt_io_listen(bluetooth_accept, NULL, NULL,
					NULL, err,
					BT_IO_OPT_PSM, option_channel,
					BT_IO_OPT_MODE, BT_IO_MODE_ERTM,
					BT_IO_OPT_OMTU, option_omtu,
					BT_IO_OPT_IMTU, option_imtu,
					BT_IO_OPT_INVALID);
}

static GIOChannel *rfcomm_listen(GError **err)
{
	return bt_io_listen(bluetooth_accept, NULL, NULL,
					NULL, err,
					BT_IO_OPT_CHANNEL, option_channel,
					BT_IO_OPT_INVALID);
}

static guint bluetooth_listen(void)
{
	GIOChannel *io;
	guint id;
	GError *err = NULL;

	if (option_channel == -1) {
		g_printerr("Bluetooth channel not set\n");
		return 0;
	}

	if (option_packet || option_channel > 31)
		io = l2cap_listen(&err);
	else
		io = rfcomm_listen(&err);

	if (io == NULL) {
		g_printerr("%s\n", err->message);
		g_error_free(err);
		return 0;
	}

	g_print("Bluetooth socket created\n");

	id = g_io_add_watch(io, G_IO_HUP | G_IO_ERR | G_IO_NVAL,
							bluetooth_watch, NULL);

	g_io_channel_set_flags(io, G_IO_FLAG_NONBLOCK, NULL);
	g_io_channel_set_close_on_unref(io, TRUE);
	g_io_channel_unref(io);

	return id;
}

static guint unix_listen(void)
{
	GIOChannel *io;
	struct sockaddr_un addr = {
		AF_UNIX, "\0/gobex/server"
	};
	int sk, err, sock_type;
	guint id;

	if (option_packet)
		sock_type = SOCK_SEQPACKET;
	else
		sock_type = SOCK_STREAM;

	sk = socket(PF_LOCAL, sock_type, 0);
	if (sk < 0) {
		err = errno;
		g_printerr("Can't create unix socket: %s (%d)\n",
						strerror(err), err);
		return 0;
	}

	if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		g_printerr("Can't bind unix socket: %s (%d)\n",
						strerror(errno), errno);
		close(sk);
		return 0;
	}

	if (listen(sk, 1) < 0) {
		g_printerr("Can't listen on unix socket: %s (%d)\n",
						strerror(errno), errno);
		close(sk);
		return 0;
	}

	g_print("Unix socket created: %d\n", sk);

	io = g_io_channel_unix_new(sk);
	id = g_io_add_watch(io, G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
							unix_accept, NULL);

	g_io_channel_set_flags(io, G_IO_FLAG_NONBLOCK, NULL);
	g_io_channel_set_close_on_unref(io, TRUE);
	g_io_channel_unref(io);

	return id;
}

int main(int argc, char *argv[])
{
	GOptionContext *context;
	GError *err = NULL;
	struct sigaction sa;
	guint server_id;

	context = g_option_context_new(NULL);
	g_option_context_add_main_entries(context, options, NULL);

	g_option_context_parse(context, &argc, &argv, &err);
	if (err != NULL) {
		g_printerr("%s\n", err->message);
		g_error_free(err);
		exit(EXIT_FAILURE);
	}

	if (option_root && chdir(option_root) < 0) {
		perror("chdir:");
		exit(EXIT_FAILURE);
	}

	if (option_bluetooth)
		server_id = bluetooth_listen();
	else
		server_id = unix_listen();

	if (server_id == 0)
		exit(EXIT_FAILURE);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sig_term;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	main_loop = g_main_loop_new(NULL, FALSE);

	g_main_loop_run(main_loop);

	g_source_remove(server_id);
	g_slist_free_full(clients, (GDestroyNotify) g_obex_unref);
	g_option_context_free(context);
	g_main_loop_unref(main_loop);

	exit(EXIT_SUCCESS);
}
