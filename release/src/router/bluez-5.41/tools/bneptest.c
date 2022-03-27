/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2015 Intel Corporation
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
#include <signal.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <netinet/in.h>
#include <linux/if_bridge.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"

#include "src/log.h"
#include "src/shared/util.h"
#include "btio/btio.h"
#include "lib/bnep.h"
#include "profiles/network/bnep.h"

enum {
	MODE_LISTEN,
	MODE_CONNECT,
};

static GMainLoop *mloop;
static GIOChannel *bnep_io;
static struct bnep *session;

static int mode;
static bool no_close_after_disconn;
static int send_frame_timeout;

static bdaddr_t src_addr, dst_addr;
static char iface[16];
static char bridge[16];
static bool send_ctrl_msg_type_set = false;
static uint8_t ctrl_msg_type = 0x00;
static bool send_bnep_msg_type_set = false;
static uint8_t bnep_msg_type = 0x00;
static int ctrl_msg_retransmition_nb = 0;
static int bnep_msg_retransmission_nb = 0;
static uint16_t local_role = BNEP_SVC_PANU;
static uint16_t remote_role = BNEP_SVC_NAP;
static uint16_t ntw_proto_down_range = 0x0000;
static uint16_t ntw_proto_up_range = 0xdc05;
static uint16_t ntw_proto_type = 0x0000;
static uint8_t mcast_addr_down_range[6];
static uint8_t mcast_addr_up_range[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
static uint8_t src_hw_addr[6];
static uint8_t dst_hw_addr[6];
static uint8_t general_frame_payload[] = "abcdef0123456789_bnep_test_data";

static int set_forward_delay(int sk)
{
	unsigned long args[4] = { BRCTL_SET_BRIDGE_FORWARD_DELAY, 0, 0, 0 };
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, bridge, IFNAMSIZ - 1);
	ifr.ifr_data = (char *) args;

	if (ioctl(sk, SIOCDEVPRIVATE, &ifr) < 0) {
		error("setting forward delay failed: %d (%s)",
							errno, strerror(errno));
		return -1;
	}

	return 0;
}

static int nap_create_bridge(void)
{
	int sk, err;

	sk = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (sk < 0)
		return -EOPNOTSUPP;

	if (ioctl(sk, SIOCBRADDBR, bridge) < 0) {
		if (errno != EEXIST) {
			close(sk);
			return -EOPNOTSUPP;
		}
	}

	err = set_forward_delay(sk);
	if (err < 0) {
		printf("failed to set forward delay\n");
		ioctl(sk, SIOCBRDELBR, bridge);
	}

	close(sk);

	return err;
}

static int cleanup(void)
{
	bnep_cleanup();

	if (mode == MODE_LISTEN)
		bnep_server_delete(bridge, iface, &dst_addr);

	if (bnep_io) {
		g_io_channel_shutdown(bnep_io, TRUE, NULL);
		g_io_channel_unref(bnep_io);
		bnep_io = NULL;
	}

	return 0;
}

static gboolean bnep_watchdog_cb(GIOChannel *chan, GIOCondition cond,
							gpointer user_data)
{
	printf("%s\n", __func__);

	if (no_close_after_disconn)
		return FALSE;

	/* Cleanup since it's called when disconnected l2cap */
	if (cleanup() < 0) {
		printf("cleanup went wrong...\n");
		return FALSE;
	}

	g_main_loop_quit(mloop);
	return FALSE;
}

static ssize_t send_compressed_frame(int sk, uint8_t type)
{
	uint8_t frame[100];

	printf("%s\n", __func__);

	if (send_frame_timeout > 0) {
		printf("waiting %d seconds before sending msg\n",
							send_frame_timeout);
		sleep(send_frame_timeout);
	}

	frame[0] = type;
	memcpy(&frame[1], dst_hw_addr, sizeof(dst_hw_addr));
	memcpy(&frame[7], src_hw_addr, sizeof(src_hw_addr));
	frame[13] = ntw_proto_type & 0xff;
	frame[14] = (ntw_proto_type >> 8);
	memcpy(&frame[15], general_frame_payload,
						sizeof(general_frame_payload));

	/* TODO - set frame payload by user */
	return send(sk, frame, 15 + sizeof(general_frame_payload), 0);
}

static ssize_t send_general_frame(int sk)
{
	uint8_t frame[100];

	printf("%s\n", __func__);

	if (send_frame_timeout > 0) {
		printf("waiting %d seconds before sending msg\n",
							send_frame_timeout);
		sleep(send_frame_timeout);
	}

	frame[0] = BNEP_GENERAL;
	memcpy(&frame[1], dst_hw_addr, sizeof(dst_hw_addr));
	memcpy(&frame[7], src_hw_addr, sizeof(src_hw_addr));
	frame[13] = ntw_proto_type & 0xff;
	frame[14] = (ntw_proto_type >> 8);
	memcpy(&frame[15], general_frame_payload,
						sizeof(general_frame_payload));

	/* TODO - set frame payload by user */
	return send(sk, frame, 15 + sizeof(general_frame_payload), 0);
}

static ssize_t send_ctrl_frame(int sk)
{
	/*
	 * Max buff size = type(1byte) + ctrl(1byte) + len(2byte) +
	 * mcast_addr_down(6byte) + mcast_addr_up(6byte)
	 */
	uint8_t buff[16];
	struct bnep_set_filter_req *frame = (void *) buff;
	int err;

	printf("%s\n", __func__);

	if (send_frame_timeout > 0) {
		printf("waiting %d seconds before sending msg\n",
						send_frame_timeout);
		sleep(send_frame_timeout);
	}

	switch (ctrl_msg_type) {
	case BNEP_FILTER_NET_TYPE_SET:
		frame->type = BNEP_CONTROL;
		frame->ctrl = ctrl_msg_type;
		frame->len = htons(sizeof(ntw_proto_down_range) +
						sizeof(ntw_proto_up_range));
		memcpy(frame->list, &ntw_proto_down_range,
						sizeof(ntw_proto_down_range));
		memcpy(frame->list + sizeof(ntw_proto_down_range),
			&ntw_proto_up_range, sizeof(ntw_proto_up_range));

		err = send(sk, frame, sizeof(*frame) +
						sizeof(ntw_proto_down_range) +
						sizeof(ntw_proto_up_range), 0);
		break;
	case BNEP_FILTER_MULT_ADDR_SET:
		frame->type = BNEP_CONTROL;
		frame->ctrl = ctrl_msg_type;
		frame->len = htons(sizeof(mcast_addr_down_range) +
						sizeof(mcast_addr_up_range));
		memcpy(frame->list, mcast_addr_down_range,
						sizeof(mcast_addr_down_range));
		memcpy(frame->list + sizeof(mcast_addr_down_range),
			mcast_addr_up_range, sizeof(mcast_addr_up_range));

		err = send(sk, frame, sizeof(*frame) +
					sizeof(mcast_addr_down_range) +
					sizeof(mcast_addr_up_range), 0);
		break;
	default:
		err = -1;
		break;
	}

	return err;
}

static int send_bnep_frame(int sk)
{
	int err;

	switch (bnep_msg_type) {
	case BNEP_GENERAL:
		err = send_general_frame(sk);
		break;
	case BNEP_COMPRESSED:
		err = send_compressed_frame(sk, BNEP_COMPRESSED);
		break;
	case BNEP_COMPRESSED_SRC_ONLY:
		err = send_compressed_frame(sk,
					BNEP_COMPRESSED_SRC_ONLY);
		break;
	case BNEP_COMPRESSED_DST_ONLY:
		err = send_compressed_frame(sk,
					BNEP_COMPRESSED_DST_ONLY);
		break;
	default:
		printf("wrong bnep_msg_type 0x%02x\n", bnep_msg_type);
		err = -EINVAL;
		break;
	}

	return err;
}

static void handle_bnep_msg_send(int sk)
{
	if (send_ctrl_msg_type_set) {
		do {
			if (send_ctrl_frame(sk) < 0)
				printf("sending ctrl frame error: %s (%d)\n",
							strerror(errno), errno);
		} while (ctrl_msg_retransmition_nb--);
	}

	if (send_bnep_msg_type_set) {
		do {
			if (send_bnep_frame(sk) < 0)
				printf("sending bnep frame error: %s (%d)\n",
							strerror(errno), errno);
		} while (bnep_msg_retransmission_nb--);
	}
}

static gboolean setup_bnep_cb(GIOChannel *chan, GIOCondition cond,
							gpointer user_data)
{
	uint8_t packet[BNEP_MTU];
	int sk, n, err;

	printf("%s\n", __func__);

	if (cond & (G_IO_ERR | G_IO_HUP | G_IO_NVAL)) {
		error("hangup or error or inval on BNEP socket");
		return FALSE;
	}

	sk = g_io_channel_unix_get_fd(chan);

	/* Reading BNEP_SETUP_CONNECTION_REQUEST_MSG */
	n = recv(sk, packet, sizeof(packet), MSG_PEEK);
	if (n < 0) {
		error("read(): %s(%d)", strerror(errno), errno);
		return FALSE;
	}

	err = nap_create_bridge();
	if (err < 0) {
		error("failed to create bridge: %s (%d)", strerror(-err), err);
		return FALSE;
	}

	if (bnep_server_add(sk, (err < 0) ? NULL : bridge, iface, &dst_addr,
							packet, n) < 0) {
		printf("server_connadd failed\n");
		cleanup();
		return FALSE;
	}

	g_io_add_watch(chan, G_IO_HUP | G_IO_ERR | G_IO_NVAL, bnep_watchdog_cb,
									NULL);

	handle_bnep_msg_send(sk);

	g_io_channel_unref(bnep_io);
	bnep_io = NULL;

	return FALSE;
}

static void connect_cb(GIOChannel *chan, GError *err, gpointer user_data)
{
	printf("%s\n", __func__);

	if (err) {
		error("%s", err->message);
		return;
	}

	g_io_add_watch(chan, G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
							setup_bnep_cb, NULL);
}

static void connected_client_cb(char *iface, int err, void *data)
{
	int sk = PTR_TO_INT(data);

	printf("%s\n", __func__);

	handle_bnep_msg_send(sk);
}

static void disconnected_client_cb(void *data)
{
	printf("%s\n", __func__);

	if (no_close_after_disconn)
		return;

	/* Cleanup since it's called when disconnected l2cap */
	if (cleanup() < 0) {
		printf("cleanup went wrong...\n");
		return;
	}

	g_main_loop_quit(mloop);
}

static void connect_client_cb(GIOChannel *chan, GError *err, gpointer user_data)
{
	int perr;
	int sk;

	sk = g_io_channel_unix_get_fd(bnep_io);

	session = bnep_new(sk, local_role, remote_role, bridge);
	if (!session) {
		printf("cannot create bnep session\n");
		return;
	}

	perr = bnep_connect(session, connected_client_cb,
				disconnected_client_cb, INT_TO_PTR(sk), NULL);
	if (perr < 0)
		printf("cannot initiate bnep connection\n");
}

static void confirm_cb(GIOChannel *chan, gpointer data)
{
	GError *err = NULL;
	char address[18];

	printf("%s\n", __func__);

	bt_io_get(chan, &err, BT_IO_OPT_DEST_BDADDR, &dst_addr, BT_IO_OPT_DEST,
						address, BT_IO_OPT_INVALID);
	if (err) {
		error("%s", err->message);
		g_error_free(err);
		return;
	}

	printf("incoming connection from: %s\n", address);

	bnep_io = g_io_channel_ref(chan);
	g_io_channel_set_close_on_unref(bnep_io, TRUE);

	if (!bt_io_accept(bnep_io, connect_cb, NULL, NULL, &err)) {
		error("bt_io_accept: %s", err->message);
		g_error_free(err);
		g_io_channel_unref(bnep_io);
	}
}

static int bnep_server_listen(void)
{
	GError *gerr = NULL;

	printf("%s\n", __func__);

	bnep_io = bt_io_listen(NULL, confirm_cb, NULL, NULL, &gerr,
					BT_IO_OPT_SOURCE_BDADDR, &src_addr,
					BT_IO_OPT_PSM, BNEP_PSM,
					BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
					BT_IO_OPT_OMTU, BNEP_MTU,
					BT_IO_OPT_IMTU, BNEP_MTU,
					BT_IO_OPT_INVALID);
	if (!bnep_io) {
		printf("can't start server listening: err %s\n", gerr->message);
		g_error_free(gerr);
		return -1;
	}

	return 0;
}

static int bnep_client_connect(void)
{
	GError *gerr = NULL;
	char bdastr[18];

	printf("%s\n", __func__);

	ba2str(&dst_addr, bdastr);
	printf("connecting %s\n", bdastr);

	bnep_io = bt_io_connect(connect_client_cb, NULL, NULL, &gerr,
					BT_IO_OPT_SOURCE_BDADDR, &src_addr,
					BT_IO_OPT_DEST_BDADDR, &dst_addr,
					BT_IO_OPT_PSM, BNEP_PSM,
					BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_MEDIUM,
					BT_IO_OPT_OMTU, BNEP_MTU,
					BT_IO_OPT_IMTU, BNEP_MTU,
					BT_IO_OPT_INVALID);
	if (!bnep_io) {
		printf("cannot connect: err %s\n", gerr->message);
		g_error_free(gerr);
		return -1;
	}

	return 0;
}

static void exit_handler(int sig)
{
	printf("got sig = %d, cleaning up...\n", sig);

	if (cleanup() < 0)
		printf("cleanup failure...\n");
	else
		printf("cleanup successful - exit\n");

	exit(0);
}

static void usage(void)
{
	printf("bneptest - BNEP testing ver %s\n", VERSION);
	printf("Usage:\n"
		"\tbneptest [-i] -b <bridge name> -n <iface name>"
				" <connection mode> [send_ctrl_cmd] [options]\n"
		"\t-i hci dev number <hci number>, def. 0\n"
		"\t-b bridge name <string>\n"
		"\t-n interface name <string>\n");
	printf("Connect Mode:\n"
		"\t-c connect <dst_addr>\n"
		"\t-r remote role <16 bit svc value>\n"
		"\t-l local role <16 bit svc valu>\n");
	printf("Listen Mode:\n"
		"\t-s start server listening\n");
	printf("Send control command:\n"
		"\t-t send message type <control msg type>, def. 0\n"
		"\t-e start network protocol type range <16 bit val>, def. 0\n"
		"\t-d end network protocol type range <16 bit val>, def. 1500\n"
		"\t-g start multicast addr range <xx:xx:xx:xx:xx:xx>, def. 0\n"
		"\t-j end multicast addr range <xx:xx:xx:xx:xx:xx>, def. f\n"
		"\t-y number of ctrl frame retransmission <integer>, def. 0\n"
		"\t-u number of bnep frame retransmission <integer>, def. 0\n");
	printf("Send bnep generic frame:\n"
		"\t-w send bnep generic frame <bnep generic type>, def. 0\n"
		"\t-k set src mac addr <xx:xx:xx:xx:xx:xx>, def. 0\n"
		"\t-f set dst mac addr <xx:xx:xx:xx:xx:xx>, def. 0\n");
	printf("Options:\n"
		"\t-T send message timeout after setup <seconds>\n"
		"\t-N don't close bneptest after disconnect\n");
}

static struct option main_options[] = {
	{ "device",			1, 0, 'i' },
	{ "listen",			0, 0, 's' },
	{ "connect",			1, 0, 'c' },
	{ "snd_ctrl_msg_type",		1, 0, 't' },
	{ "snd_bnep_msg_type",		1, 0, 'w' },
	{ "src_hw_addr",		1, 0, 'k' },
	{ "dst_hw_addr",		1, 0, 'f' },
	{ "send_timeout",		1, 0, 'T' },
	{ "ntw_proto_down_range",	1, 0, 'd' },
	{ "ntw_proto_up_range",		1, 0, 'e' },
	{ "mcast_addr_down_range",	1, 0, 'g' },
	{ "mcast_addr_up_range",	1, 0, 'j' },
	{ "local_role",			1, 0, 'l' },
	{ "remote_role",		1, 0, 'r' },
	{ "bridge name",		1, 0, 'b' },
	{ "iface name",			1, 0, 'n' },
	{ "no_close",			0, 0, 'N' },
	{ "retrans_ctrl_nb",		0, 0, 'y' },
	{ "retrans_bnep_nb",		0, 0, 'u' },
	{ "help",			0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

int main(int argc, char *argv[])
{
	int opt, i;
	int err;
	bool is_set_b_name = false, is_set_i_name = false;

	DBG("");

	signal(SIGINT, exit_handler);

	hci_devba(0, &src_addr);
	bacpy(&src_addr, BDADDR_ANY);

	mloop = g_main_loop_new(NULL, FALSE);
	if (!mloop) {
		printf("cannot create main loop\n");

		exit(1);
	}

	while ((opt = getopt_long(argc, argv,
				"+i:c:b:n:t:T:d:e:g:j:k:f:w:l:r:y:u:Nsh",
				main_options, NULL)) != EOF) {
		switch (opt) {
		case 'i':
			if (!strncmp(optarg, "hci", 3))
				hci_devba(atoi(optarg + 3), &src_addr);
			else
				str2ba(optarg, &src_addr);
			break;
		case 's':
			mode = MODE_LISTEN;
			break;
		case 'c':
			str2ba(optarg, &dst_addr);
			mode = MODE_CONNECT;
			break;
		case 't':
			send_ctrl_msg_type_set = true;
			ctrl_msg_type = atoi(optarg);
			break;
		case 'w':
			send_bnep_msg_type_set = true;
			bnep_msg_type = atoi(optarg);
			break;
		case 'k':
			for (i = 0; i <= 5; i++, optarg += 3)
				src_hw_addr[i] = strtol(optarg, NULL, 16);
			break;
		case 'f':
			for (i = 0; i <= 5; i++, optarg += 3)
				dst_hw_addr[i] = strtol(optarg, NULL, 16);
			break;
		case 'T':
			send_frame_timeout = atoi(optarg);
			break;
		case 'd':
			ntw_proto_down_range = htons(atoi(optarg));
			break;
		case 'e':
			ntw_proto_up_range = htons(atoi(optarg));
			break;
		case 'g':
			for (i = 5; i >= 0; i--, optarg += 3)
				mcast_addr_down_range[i] =
						strtol(optarg, NULL, 16);
			break;
		case 'j':
			for (i = 5; i >= 0; i--, optarg += 3)
				mcast_addr_up_range[i] =
						strtol(optarg, NULL, 16);
			break;
		case 'l':
			local_role = atoi(optarg);
			break;
		case 'r':
			remote_role = atoi(optarg);
			break;
		case 'b':
			strncpy(bridge, optarg, 16);
			bridge[15] = '\0';
			is_set_b_name = true;
			break;
		case 'n':
			strncpy(iface, optarg, 14);
			strcat(iface, "\%d");
			iface[15] = '\0';
			is_set_i_name = true;
			break;
		case 'N':
			no_close_after_disconn = true;
			break;
		case 'y':
			ctrl_msg_retransmition_nb = atoi(optarg);
			break;
		case 'u':
			bnep_msg_retransmission_nb = atoi(optarg);
			break;
		case 'h':
		default:
			usage();
			exit(0);
		}
	}

	if (!is_set_b_name || !is_set_i_name) {
		printf("bridge, interface name must be set!\n");
		exit(1);
	}

	switch (mode) {
	case MODE_CONNECT:
		err = bnep_init();
		if (err < 0) {
			printf("cannot initialize bnep\n");
			exit(1);
		}
		err = bnep_client_connect();
		if (err < 0)
			exit(1);

		break;
	case MODE_LISTEN:
		err = bnep_init();
		if (err < 0) {
			printf("cannot initialize bnep\n");
			exit(1);
		}
		err = bnep_server_listen();
		if (err < 0)
			exit(1);

		break;
	default:
		printf("connect/listen mode not set, exit...\n");
		exit(1);
	}

	g_main_loop_run(mloop);

	printf("Done\n");

	g_main_loop_unref(mloop);

	return 0;
}
