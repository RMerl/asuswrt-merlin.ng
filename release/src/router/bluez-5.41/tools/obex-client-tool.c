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
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "gobex/gobex.h"
#include "btio/btio.h"

static GMainLoop *main_loop = NULL;
static GObex *obex = NULL;

static gboolean option_packet = FALSE;
static gboolean option_bluetooth = FALSE;
static char *option_source = NULL;
static char *option_dest = NULL;
static int option_channel = -1;
static int option_imtu = -1;
static int option_omtu = -1;

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
	{ "source", 's', 0, G_OPTION_ARG_STRING,
			&option_source, "Bluetooth adapter address",
			"00:..." },
	{ "destination", 'd', 0, G_OPTION_ARG_STRING,
			&option_dest, "Remote bluetooth address",
			"00:..." },
	{ "channel", 'c', 0, G_OPTION_ARG_INT,
			&option_channel, "Transport channel", "CHANNEL" },
	{ "packet", 'p', 0, G_OPTION_ARG_NONE,
			&option_packet, "Packet based transport" },
	{ "stream", 's', G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE,
			&option_packet, "Stream based transport" },
	{ "input-mtu", 'i', 0, G_OPTION_ARG_INT,
			&option_imtu, "Transport input MTU", "MTU" },
	{ "output-mtu", 'o', 0, G_OPTION_ARG_INT,
			&option_omtu, "Transport output MTU", "MTU" },
	{ NULL },
};

static void conn_complete(GObex *obex, GError *err, GObexPacket *rsp,
							gpointer user_data)
{
	if (err != NULL)
		g_print("Connect failed: %s\n", err->message);
	else
		g_print("Connect succeeded\n");
}

static void cmd_connect(int argc, char **argv)
{
	g_obex_connect(obex, conn_complete, NULL, NULL, G_OBEX_HDR_INVALID);
}

struct transfer_data {
	int fd;
};

static void transfer_complete(GObex *obex, GError *err, gpointer user_data)
{
	struct transfer_data *data = user_data;

	if (err != NULL)
		g_printerr("failed: %s\n", err->message);
	else
		g_print("transfer succeeded\n");

	close(data->fd);
	g_free(data);
}

static gssize put_data_cb(void *buf, gsize len, gpointer user_data)
{
	struct transfer_data *data = user_data;

	return read(data->fd, buf, len);
}

static void cmd_put(int argc, char **argv)
{
	struct transfer_data *data;
	GError *err = NULL;
	int fd;

	if (argc < 2) {
		g_printerr("Filename required\n");
		return;
	}

	fd = open(argv[1], O_RDONLY | O_NOCTTY, 0);
	if (fd < 0) {
		g_printerr("open: %s\n", strerror(errno));
		return;
	}

	data = g_new0(struct transfer_data, 1);
	data->fd = fd;

	g_obex_put_req(obex, put_data_cb, transfer_complete, data, &err,
						G_OBEX_HDR_NAME, argv[1],
						G_OBEX_HDR_INVALID);
	if (err != NULL) {
		g_printerr("put failed: %s\n", err->message);
		g_error_free(err);
		close(data->fd);
		g_free(data);
	}
}

static gboolean get_data_cb(const void *buf, gsize len, gpointer user_data)
{
	struct transfer_data *data = user_data;

	if (write(data->fd, buf, len) < 0) {
		g_printerr("write: %s\n", strerror(errno));
		return FALSE;
	}

	return TRUE;
}

static void cmd_get(int argc, char **argv)
{
	struct transfer_data *data;
	GError *err = NULL;
	int fd;

	if (argc < 2) {
		g_printerr("Filename required\n");
		return;
	}

	fd = open(argv[1], O_WRONLY | O_CREAT | O_NOCTTY, 0600);
	if (fd < 0) {
		g_printerr("open: %s\n", strerror(errno));
		return;
	}

	data = g_new0(struct transfer_data, 1);
	data->fd = fd;

	g_obex_get_req(obex, get_data_cb, transfer_complete, data, &err,
						G_OBEX_HDR_NAME, argv[1],
						G_OBEX_HDR_INVALID);
	if (err != NULL) {
		g_printerr("get failed: %s\n", err->message);
		g_error_free(err);
		close(data->fd);
		g_free(data);
	}
}

static void cmd_help(int argc, char **argv);

static void cmd_exit(int argc, char **argv)
{
	g_main_loop_quit(main_loop);
}

static struct {
	const char *cmd;
	void (*func)(int argc, char **argv);
	const char *params;
	const char *desc;
} commands[] = {
	{ "help",	cmd_help,	"",		"Show this help"},
	{ "exit",	cmd_exit,	"",		"Exit application" },
	{ "quit",	cmd_exit,	"",		"Exit application" },
	{ "connect",	cmd_connect,	"[target]",	"OBEX Connect" },
	{ "put",	cmd_put,	"<file>",	"Send a file" },
	{ "get",	cmd_get,	"<file>",	"Receive a file" },
	{ NULL },
};

static void cmd_help(int argc, char **argv)
{
	int i;

	for (i = 0; commands[i].cmd; i++)
		printf("%-15s %-30s %s\n", commands[i].cmd,
				commands[i].params, commands[i].desc);
}

static void parse_line(char *line_read)
{
	char **argvp;
	int argcp;
	int i;

	if (line_read == NULL) {
		g_print("\n");
		g_main_loop_quit(main_loop);
		return;
	}

	line_read = g_strstrip(line_read);

	if (*line_read == '\0') {
		free(line_read);
		return;
	}

	add_history(line_read);

	g_shell_parse_argv(line_read, &argcp, &argvp, NULL);

	free(line_read);

	for (i = 0; commands[i].cmd; i++)
		if (strcasecmp(commands[i].cmd, argvp[0]) == 0)
			break;

	if (commands[i].cmd)
		commands[i].func(argcp, argvp);
	else
		g_print("%s: command not found\n", argvp[0]);

	g_strfreev(argvp);
}

static gboolean prompt_read(GIOChannel *chan, GIOCondition cond,
							gpointer user_data)
{
	if (cond & (G_IO_HUP | G_IO_ERR | G_IO_NVAL)) {
		g_main_loop_quit(main_loop);
		return FALSE;
	}

	rl_callback_read_char();

	return TRUE;
}

static void disconn_func(GObex *obex, GError *err, gpointer user_data)
{
	g_printerr("Disconnected: %s\n", err ? err->message : "(no error)");
	g_main_loop_quit(main_loop);
}

static void transport_connect(GIOChannel *io, GObexTransportType transport)
{
	GIOChannel *input;
	GIOCondition events;

	g_io_channel_set_flags(io, G_IO_FLAG_NONBLOCK, NULL);
	g_io_channel_set_close_on_unref(io, TRUE);

	obex = g_obex_new(io, transport, option_imtu, option_omtu);
	g_obex_set_disconnect_function(obex, disconn_func, NULL);

	input = g_io_channel_unix_new(STDIN_FILENO);
	g_io_channel_set_close_on_unref(input, TRUE);
	events = G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL;
	g_io_add_watch(input, events, prompt_read, NULL);
	g_io_channel_unref(input);
	rl_callback_handler_install("client> ", parse_line);
}

static GIOChannel *unix_connect(GObexTransportType transport)
{
	GIOChannel *io;
	struct sockaddr_un addr = {
		AF_UNIX, "\0/gobex/server"
	};
	int sk, err, sock_type;

	if (option_packet)
		sock_type = SOCK_SEQPACKET;
	else
		sock_type = SOCK_STREAM;

	sk = socket(PF_LOCAL, sock_type, 0);
	if (sk < 0) {
		err = errno;
		g_printerr("Can't create unix socket: %s (%d)\n",
						strerror(err), err);
		return NULL;
	}

	if (connect(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		err = errno;
		g_printerr("connect: %s (%d)\n", strerror(err), err);
		return NULL;
	}

	io = g_io_channel_unix_new(sk);

	g_print("Unix socket created: %d\n", sk);

	transport_connect(io, transport);

	return io;
}

static void conn_callback(GIOChannel *io, GError *err, gpointer user_data)
{
	GObexTransportType transport = GPOINTER_TO_UINT(user_data);

	if (err != NULL) {
		g_printerr("%s\n", err->message);
		return;
	}

	g_print("Bluetooth socket connected\n");

	transport_connect(io, transport);
}

static GIOChannel *l2cap_connect(GObexTransportType transport, GError **err)
{
	if (option_source)
		return bt_io_connect(conn_callback,
					GUINT_TO_POINTER(transport),
					NULL, err,
					BT_IO_OPT_SOURCE, option_source,
					BT_IO_OPT_DEST, option_dest,
					BT_IO_OPT_PSM, option_channel,
					BT_IO_OPT_MODE, BT_IO_MODE_ERTM,
					BT_IO_OPT_OMTU, option_omtu,
					BT_IO_OPT_IMTU, option_imtu,
					BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
					BT_IO_OPT_INVALID);

	return bt_io_connect(conn_callback,
					GUINT_TO_POINTER(transport),
					NULL, err,
					BT_IO_OPT_DEST, option_dest,
					BT_IO_OPT_PSM, option_channel,
					BT_IO_OPT_MODE, BT_IO_MODE_ERTM,
					BT_IO_OPT_OMTU, option_omtu,
					BT_IO_OPT_IMTU, option_imtu,
					BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
					BT_IO_OPT_INVALID);
}

static GIOChannel *rfcomm_connect(GObexTransportType transport, GError **err)
{
	if (option_source)
		return bt_io_connect(conn_callback,
					GUINT_TO_POINTER(transport),
					NULL, err,
					BT_IO_OPT_SOURCE, option_source,
					BT_IO_OPT_DEST, option_dest,
					BT_IO_OPT_CHANNEL, option_channel,
					BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
					BT_IO_OPT_INVALID);

	return bt_io_connect(conn_callback,
					GUINT_TO_POINTER(transport),
					NULL, err,
					BT_IO_OPT_DEST, option_dest,
					BT_IO_OPT_CHANNEL, option_channel,
					BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
					BT_IO_OPT_INVALID);
}

static GIOChannel *bluetooth_connect(GObexTransportType transport)
{
	GIOChannel *io;
	GError *err = NULL;

	if (option_dest == NULL || option_channel < 0)
		return NULL;

	if (option_channel > 31)
		io = l2cap_connect(transport, &err);
	else
		io = rfcomm_connect(transport, &err);

	if (io != NULL)
		return io;

	g_printerr("%s\n", err->message);
	g_error_free(err);
	return NULL;
}

int main(int argc, char *argv[])
{
	GOptionContext *context;
	GError *err = NULL;
	struct sigaction sa;
	GIOChannel *io;
	GObexTransportType transport;

	context = g_option_context_new(NULL);
	g_option_context_add_main_entries(context, options, NULL);

	g_option_context_parse(context, &argc, &argv, &err);
	if (err != NULL) {
		g_printerr("%s\n", err->message);
		g_error_free(err);
		g_option_context_free(context);
		exit(EXIT_FAILURE);
	}

	if (option_packet)
		transport = G_OBEX_TRANSPORT_PACKET;
	else
		transport = G_OBEX_TRANSPORT_STREAM;

	if (option_bluetooth)
		io = bluetooth_connect(transport);
	else
		io = unix_connect(transport);

	if (io == NULL) {
		g_option_context_free(context);
		exit(EXIT_FAILURE);
	}

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sig_term;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	main_loop = g_main_loop_new(NULL, FALSE);

	g_main_loop_run(main_loop);

	rl_callback_handler_remove();
	clear_history();
	g_obex_unref(obex);
	g_option_context_free(context);
	g_main_loop_unref(main_loop);

	exit(EXIT_SUCCESS);
}
