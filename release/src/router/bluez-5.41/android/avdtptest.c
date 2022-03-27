/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014 Intel Corporation
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
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"

#include "btio/btio.h"
#include "src/shared/util.h"
#include "src/shared/queue.h"
#include "avdtp.h"

static GMainLoop *mainloop = NULL;
static int dev_role = AVDTP_SEP_TYPE_SOURCE;
static bool preconf = false;
static struct avdtp *avdtp = NULL;
struct avdtp_stream *avdtp_stream = NULL;
struct avdtp_local_sep *local_sep = NULL;
struct avdtp_remote_sep *remote_sep = NULL;
static GIOChannel *io = NULL;
static bool reject = false;
static bdaddr_t src;
static bdaddr_t dst;
static uint16_t version = 0x0103;
static guint media_player = 0;
static guint media_recorder = 0;
static guint idle_id = 0;
static struct queue *lseps = NULL;

static bool fragment = false;

static enum {
	CMD_GET_CONF,
	CMD_OPEN,
	CMD_START,
	CMD_SUSPEND,
	CMD_CLOSE,
	CMD_ABORT,
	CMD_DELAY,
	CMD_NONE,
} command = CMD_NONE;

static const char sbc_codec[] = {0x00, 0x00, 0x11, 0x15, 0x02, 0x40};
static const char sbc_media_frame[] = {
	0x00, 0x60, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	0x01, 0x9c, 0xfd, 0x40, 0xbd, 0xde, 0xa9, 0x75, 0x43, 0x20, 0x87, 0x64,
	0x44, 0x32, 0x7f, 0xbe, 0xf7, 0x76, 0xfe, 0xf7, 0xbb, 0xbb, 0x7f, 0xbe,
	0xf7, 0x76, 0xfe, 0xf7, 0xbb, 0xbb, 0x7f, 0xbe, 0xf7, 0x76, 0xfe, 0xf7,
	0xbb, 0xbb, 0x80, 0x3e, 0xf7, 0x76, 0xfe, 0xf7, 0xbb, 0xbb, 0x83, 0x41,
	0x07, 0x77, 0x09, 0x07, 0x43, 0xb3, 0x81, 0xbc, 0xf8, 0x77, 0x02, 0xe5,
	0xa4, 0x3a, 0xa0, 0xcb, 0x38, 0xbb, 0x57, 0x90, 0xd9, 0x08, 0x9c, 0x1d,
	0x86, 0x59, 0x01, 0x0c, 0x21, 0x44, 0x68, 0x35, 0xa8, 0x57, 0x97, 0x0e,
	0x9b, 0xbb, 0x62, 0xc4, 0xca, 0x57, 0x04, 0xa1, 0xca, 0x3b, 0xa3, 0x48,
	0xd2, 0x66, 0x11, 0x33, 0x6a, 0x3b, 0xb4, 0xbb, 0x08, 0x77, 0x17, 0x03,
	0xb4, 0x3b, 0x79, 0x3b, 0x46, 0x97, 0x0e, 0xf7, 0x3d, 0xbb, 0x3d, 0x49,
	0x25, 0x86, 0x88, 0xb4, 0xad, 0x3b, 0x62, 0xbb, 0xa4, 0x47, 0x29, 0x99,
	0x3b, 0x3b, 0xaf, 0xc6, 0xd4, 0x37, 0x68, 0x94, 0x0a, 0xbb
	};

static void parse_command(const char *cmd)
{
	if (!strncmp(cmd, "getconf", sizeof("getconf"))) {
		command = CMD_GET_CONF;
	} else if (!strncmp(cmd, "open", sizeof("open"))) {
		command = CMD_OPEN;
	} else if (!strncmp(cmd, "start", sizeof("start"))) {
		command = CMD_START;
	} else if (!strncmp(cmd, "suspend", sizeof("suspend"))) {
		command = CMD_SUSPEND;
	} else if (!strncmp(cmd, "close", sizeof("close"))) {
		command = CMD_CLOSE;
	} else if (!strncmp(cmd, "abort", sizeof("abort"))) {
		command = CMD_ABORT;
	} else if (!strncmp(cmd, "delay", sizeof("delay"))) {
		command = CMD_DELAY;
	} else {
		printf("Unknown command '%s'\n", cmd);
		printf("(getconf open start suspend close abort delay)\n");
		exit(1);
	}
}

static void send_command(void)
{
	avdtp_state_t state = avdtp_sep_get_state(local_sep);

	switch (command) {
	case CMD_GET_CONF:
		avdtp_get_configuration(avdtp, avdtp_stream);
		break;
	case CMD_OPEN:
		if (state == AVDTP_STATE_CONFIGURED)
			avdtp_open(avdtp, avdtp_stream);
		break;
	case CMD_START:
		if (state == AVDTP_STATE_OPEN)
			avdtp_start(avdtp, avdtp_stream);
		break;
	case CMD_SUSPEND:
		if (state == AVDTP_STATE_STREAMING)
			avdtp_suspend(avdtp , avdtp_stream);
		break;
	case CMD_CLOSE:
		if (state == AVDTP_STATE_STREAMING)
			avdtp_close(avdtp, avdtp_stream, FALSE);
		break;
	case CMD_ABORT:
		avdtp_abort(avdtp , avdtp_stream);
		break;
	case CMD_DELAY:
		avdtp_delay_report(avdtp , avdtp_stream , 250);
		break;
	case CMD_NONE:
	default:
		break;
	}
}

static gboolean media_writer(gpointer user_data)
{
	uint16_t omtu;
	int fd;
	int to_write;

	if (!avdtp_stream_get_transport(avdtp_stream, &fd, NULL, &omtu, NULL))
		return TRUE;

	if (omtu < sizeof(sbc_media_frame))
		to_write = omtu;
	else
		to_write = sizeof(sbc_media_frame);

	if (write(fd, sbc_media_frame, to_write) < 0)
		return TRUE;

	send_command();

	return TRUE;
}

static bool start_media_player(void)
{
	int fd;
	uint16_t omtu;

	printf("Media streaming started\n");

	if (media_player || !avdtp_stream)
		return false;

	if (!avdtp_stream_get_transport(avdtp_stream, &fd, NULL, &omtu, NULL))
		return false;

	media_player = g_timeout_add(200, media_writer, NULL);
	if (!media_player)
		return false;

	return true;
}

static void stop_media_player(void)
{
	if (!media_player)
		return;

	printf("Media streaming stopped\n");

	g_source_remove(media_player);
	media_player = 0;
}

#if __BYTE_ORDER == __LITTLE_ENDIAN

struct rtp_header {
	unsigned cc:4;
	unsigned x:1;
	unsigned p:1;
	unsigned v:2;

	unsigned pt:7;
	unsigned m:1;

	uint16_t sequence_number;
	uint32_t timestamp;
	uint32_t ssrc;
	uint32_t csrc[0];
} __attribute__ ((packed));

#elif __BYTE_ORDER == __BIG_ENDIAN

struct rtp_header {
	unsigned v:2;
	unsigned p:1;
	unsigned x:1;
	unsigned cc:4;

	unsigned m:1;
	unsigned pt:7;

	uint16_t sequence_number;
	uint32_t timestamp;
	uint32_t ssrc;
	uint32_t csrc[0];
} __attribute__ ((packed));

#else
#error "Unknown byte order"
#endif

static gboolean media_reader(GIOChannel *source, GIOCondition condition,
								gpointer data)
{
	char buf[UINT16_MAX];
	struct rtp_header *rtp = (void *) buf;
	static bool decode = false;
	uint16_t imtu;
	int fd, ret;

	if (!avdtp_stream_get_transport(avdtp_stream, &fd, &imtu, NULL, NULL))
		return TRUE;

	ret = read(fd, buf, imtu);
	if (ret < 0) {
		printf("Reading failed (%s)\n", strerror(errno));
		return TRUE;
	}

	if (ret < (int) sizeof(*rtp)) {
		printf("Not enough media data received (%u bytes)", ret);
		return TRUE;
	}

	if (!decode) {
		printf("V=%u P=%u X=%u CC=%u M=%u PT=%u SeqNr=%d\n",
			rtp->v, rtp->p, rtp->x, rtp->cc, rtp->m, rtp->pt,
			be16_to_cpu(rtp->sequence_number));
		decode = true;
	}

	send_command();

	return TRUE;
}

static bool start_media_recorder(void)
{
	int fd;
	uint16_t omtu;
	GIOChannel *chan;

	printf("Media recording started\n");

	if (media_recorder || !avdtp_stream)
		return false;

	if (!avdtp_stream_get_transport(avdtp_stream, &fd, NULL, &omtu, NULL))
		return false;

	chan = g_io_channel_unix_new(fd);

	media_recorder = g_io_add_watch(chan, G_IO_IN, media_reader, NULL);
	g_io_channel_unref(chan);

	if (!media_recorder)
		return false;

	return true;
}

static void stop_media_recorder(void)
{
	if (!media_recorder)
		return;

	printf("Media recording stopped\n");

	g_source_remove(media_recorder);
	media_recorder = 0;
}

static void set_configuration_cfm(struct avdtp *session,
					struct avdtp_local_sep *lsep,
					struct avdtp_stream *stream,
					struct avdtp_error *err,
					void *user_data)
{
	printf("%s\n", __func__);

	if (preconf)
		avdtp_open(avdtp, avdtp_stream);
}

static void get_configuration_cfm(struct avdtp *session,
					struct avdtp_local_sep *lsep,
					struct avdtp_stream *stream,
					struct avdtp_error *err,
					void *user_data)
	{
	printf("%s\n", __func__);
}

static void disconnect_cb(void *user_data)
{
	printf("Disconnected\n");

	g_main_loop_quit(mainloop);
}

static void discover_cb(struct avdtp *session, GSList *seps,
				struct avdtp_error *err, void *user_data)
{
	struct avdtp_service_capability *service;
	GSList *caps = NULL;
	int ret;

	remote_sep = avdtp_find_remote_sep(avdtp, local_sep);
	if (!remote_sep) {
		printf("Unable to find matching endpoint\n");
		avdtp_shutdown(session);
		return;
	}

	printf("Matching endpoint found\n");

	service = avdtp_service_cap_new(AVDTP_MEDIA_TRANSPORT, NULL, 0);
	caps = g_slist_append(caps, service);

	service = avdtp_service_cap_new(AVDTP_MEDIA_CODEC, sbc_codec,
							sizeof(sbc_codec));
	caps = g_slist_append(caps, service);

	ret = avdtp_set_configuration(avdtp, remote_sep, local_sep, caps,
								&avdtp_stream);

	g_slist_free_full(caps, g_free);

	if (ret < 0) {
		printf("Failed to set configuration (%s)\n", strerror(-ret));
		avdtp_shutdown(session);
	}
}

static void connect_cb(GIOChannel *chan, GError *err, gpointer user_data)
{
	uint16_t imtu, omtu;
	GError *gerr = NULL;
	int fd;

	if (err) {
		printf("%s\n", err->message);
		g_main_loop_quit(mainloop);
		return;
	}

	bt_io_get(chan, &gerr,
			BT_IO_OPT_IMTU, &imtu,
			BT_IO_OPT_OMTU, &omtu,
			BT_IO_OPT_DEST_BDADDR, &dst,
			BT_IO_OPT_INVALID);
	if (gerr) {
		printf("%s\n", gerr->message);
		g_main_loop_quit(mainloop);
		return;
	}

	printf("Connected (imtu=%d omtu=%d)\n", imtu, omtu);

	fd = g_io_channel_unix_get_fd(chan);

	if (avdtp && avdtp_stream) {
		if (!avdtp_stream_set_transport(avdtp_stream, fd, imtu, omtu)) {
			printf("avdtp_stream_set_transport: failed\n");
			g_main_loop_quit(mainloop);
		}

		g_io_channel_set_close_on_unref(chan, FALSE);

		send_command();

		return;
	}

	avdtp = avdtp_new(fd, imtu, omtu, version, lseps);
	if (!avdtp) {
		printf("Failed to create avdtp instance\n");
		g_main_loop_quit(mainloop);
		return;
	}

	avdtp_add_disconnect_cb(avdtp, disconnect_cb, NULL);

	if (preconf) {
		int ret;

		ret = avdtp_discover(avdtp, discover_cb, NULL);
		if (ret < 0) {
			printf("avdtp_discover failed: %s", strerror(-ret));
			g_main_loop_quit(mainloop);
		}
	}
}

static GIOChannel *do_connect(GError **err)
{
	if (fragment)
		return bt_io_connect(connect_cb, NULL, NULL, err,
					BT_IO_OPT_SOURCE_BDADDR, &src,
					BT_IO_OPT_DEST_BDADDR, &dst,
					BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
					BT_IO_OPT_PSM, AVDTP_PSM,
					BT_IO_OPT_MTU, 48,
					BT_IO_OPT_INVALID);

	return bt_io_connect(connect_cb, NULL, NULL, err,
				BT_IO_OPT_SOURCE_BDADDR, &src,
				BT_IO_OPT_DEST_BDADDR, &dst,
				BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
				BT_IO_OPT_PSM, AVDTP_PSM,
				BT_IO_OPT_INVALID);
}

static void open_cfm(struct avdtp *session, struct avdtp_local_sep *lsep,
			struct avdtp_stream *stream, struct avdtp_error *err,
			void *user_data)
{
	GError *gerr = NULL;

	printf("%s\n", __func__);

	do_connect(&gerr);
	if (gerr) {
		printf("connect failed: %s\n", gerr->message);
		g_error_free(gerr);
		g_main_loop_quit(mainloop);
	}
}

static void start_cfm(struct avdtp *session, struct avdtp_local_sep *lsep,
			struct avdtp_stream *stream, struct avdtp_error *err,
			void *user_data)
{
	printf("%s\n", __func__);

	if (dev_role == AVDTP_SEP_TYPE_SOURCE)
		start_media_player();
	else
		start_media_recorder();
}

static void suspend_cfm(struct avdtp *session, struct avdtp_local_sep *lsep,
			struct avdtp_stream *stream,
			struct avdtp_error *err, void *user_data)
{
	printf("%s\n", __func__);

	if (dev_role == AVDTP_SEP_TYPE_SOURCE)
		stop_media_player();
	else
		stop_media_recorder();
}

static void close_cfm(struct avdtp *session, struct avdtp_local_sep *lsep,
			struct avdtp_stream *stream,
			struct avdtp_error *err, void *user_data)
{
	printf("%s\n", __func__);

	if (dev_role == AVDTP_SEP_TYPE_SOURCE)
		stop_media_player();
	else
		stop_media_recorder();

	avdtp_stream = NULL;
}

static void abort_cfm(struct avdtp *session, struct avdtp_local_sep *lsep,
			struct avdtp_stream *stream,
			struct avdtp_error *err, void *user_data)
{
	printf("%s\n", __func__);

	if (dev_role == AVDTP_SEP_TYPE_SOURCE)
		stop_media_player();
	else
		stop_media_recorder();

	avdtp_stream = NULL;
}

static void reconfigure_cfm(struct avdtp *session,
				struct avdtp_local_sep *lsep,
				struct avdtp_stream *stream,
				struct avdtp_error *err, void *user_data)
{
	printf("%s\n", __func__);
}

static void delay_report_cfm(struct avdtp *session,
				struct avdtp_local_sep *lsep,
				struct avdtp_stream *stream,
				struct avdtp_error *err, void *user_data)
{
	printf("%s\n", __func__);
}

static struct avdtp_sep_cfm sep_cfm = {
	.set_configuration	= set_configuration_cfm,
	.get_configuration	= get_configuration_cfm,
	.open			= open_cfm,
	.start			= start_cfm,
	.suspend		= suspend_cfm,
	.close			= close_cfm,
	.abort			= abort_cfm,
	.reconfigure		= reconfigure_cfm,
	.delay_report		= delay_report_cfm,
};

static gboolean get_capability_ind(struct avdtp *session,
					struct avdtp_local_sep *sep,
					GSList **caps, uint8_t *err,
					void *user_data)
{
	struct avdtp_service_capability *service;
	int i;

	printf("%s\n", __func__);

	if (idle_id > 0) {
		g_source_remove(idle_id);
		idle_id = 0;
	}

	if (reject)
		return FALSE;

	*caps = NULL;

	service = avdtp_service_cap_new(AVDTP_MEDIA_TRANSPORT, NULL, 0);
	*caps = g_slist_append(*caps, service);

	service = avdtp_service_cap_new(AVDTP_MEDIA_CODEC, sbc_codec,
						sizeof(sbc_codec));
	*caps = g_slist_append(*caps, service);

	if (fragment)
		for (i = 0; i < 10; i++) {
			service = avdtp_service_cap_new(AVDTP_MEDIA_CODEC,
							sbc_codec,
							sizeof(sbc_codec));
			*caps = g_slist_append(*caps, service);
		}

	return TRUE;
}

static gboolean set_configuration_ind(struct avdtp *session,
					struct avdtp_local_sep *lsep,
					struct avdtp_stream *stream,
					GSList *caps,
					avdtp_set_configuration_cb cb,
					void *user_data)
{
	printf("%s\n", __func__);

	if (reject)
		return FALSE;

	if (idle_id > 0) {
		g_source_remove(idle_id);
		idle_id = 0;
	}

	avdtp_stream = stream;

	cb(session, stream, NULL);

	send_command();

	return TRUE;
}

static gboolean get_configuration_ind(struct avdtp *session,
					struct avdtp_local_sep *lsep,
					uint8_t *err, void *user_data)
{
	printf("%s\n", __func__);

	if (reject)
		return FALSE;

	return TRUE;
}

static gboolean open_ind(struct avdtp *session, struct avdtp_local_sep *lsep,
				struct avdtp_stream *stream, uint8_t *err,
				void *user_data)
{
	printf("%s\n", __func__);

	if (reject)
		return FALSE;

	send_command();

	return TRUE;
}

static gboolean start_ind(struct avdtp *session, struct avdtp_local_sep *lsep,
				struct avdtp_stream *stream, uint8_t *err,
				void *user_data)
{
	printf("%s\n", __func__);

	if (reject)
		return FALSE;

	if (dev_role == AVDTP_SEP_TYPE_SOURCE)
		start_media_player();
	else
		start_media_recorder();

	send_command();

	return TRUE;
}

static gboolean suspend_ind(struct avdtp *session,
				struct avdtp_local_sep *sep,
				struct avdtp_stream *stream, uint8_t *err,
				void *user_data)
{
	printf("%s\n", __func__);

	if (reject)
		return FALSE;

	if (dev_role == AVDTP_SEP_TYPE_SOURCE)
		stop_media_player();
	else
		stop_media_recorder();

	return TRUE;
}

static gboolean close_ind(struct avdtp *session, struct avdtp_local_sep *sep,
				struct avdtp_stream *stream, uint8_t *err,
				void *user_data)
{
	printf("%s\n", __func__);

	if (reject)
		return FALSE;

	if (dev_role == AVDTP_SEP_TYPE_SOURCE)
		stop_media_player();
	else
		stop_media_recorder();

	avdtp_stream = NULL;

	return TRUE;
}

static void abort_ind(struct avdtp *session, struct avdtp_local_sep *sep,
			struct avdtp_stream *stream, uint8_t *err,
			void *user_data)
{
	printf("%s\n", __func__);

	if (dev_role == AVDTP_SEP_TYPE_SOURCE)
		stop_media_player();
	else
		stop_media_recorder();

	avdtp_stream = NULL;
}

static gboolean reconfigure_ind(struct avdtp *session,
				struct avdtp_local_sep *lsep,
				uint8_t *err, void *user_data)
{
	printf("%s\n", __func__);

	if (reject)
		return FALSE;

	return TRUE;
}

static gboolean delayreport_ind(struct avdtp *session,
				struct avdtp_local_sep *lsep,
				uint8_t rseid, uint16_t delay,
				uint8_t *err, void *user_data)
{
	printf("%s\n", __func__);

	if (reject)
		return FALSE;

	return TRUE;
}

static struct avdtp_sep_ind sep_ind = {
	.get_capability		= get_capability_ind,
	.set_configuration	= set_configuration_ind,
	.get_configuration	= get_configuration_ind,
	.open			= open_ind,
	.close			= close_ind,
	.start			= start_ind,
	.suspend		= suspend_ind,
	.abort			= abort_ind,
	.reconfigure		= reconfigure_ind,
	.delayreport		= delayreport_ind,
};

static void usage(void)
{
	printf("avdtptest - AVDTP testing ver %s\n", VERSION);
	printf("Usage:\n"
		"\tavdtptest [options]\n");
	printf("options:\n"
		"\t-d <device_role>   SRC (source) or SINK (sink)\n"
		"\t-i <hcidev>        HCI adapter\n"
		"\t-c <bdaddr>        connect\n"
		"\t-l                 listen\n"
		"\t-r                 reject commands\n"
		"\t-f                 fragment\n"
		"\t-p                 configure stream\n"
		"\t-s <command>       send command\n"
		"\t-v <version>       set version (0x0100, 0x0102, 0x0103\n");
}

static struct option main_options[] = {
	{ "help",		0, 0, 'h' },
	{ "device_role",	1, 0, 'd' },
	{ "adapter",		1, 0, 'i' },
	{ "connect",		1, 0, 'c' },
	{ "listen",		0, 0, 'l' },
	{ "reject",		0, 0, 'r' },
	{ "fragment",		0, 0, 'f' },
	{ "preconf",		0, 0, 'p' },
	{ "send",		1, 0, 's' },
	{ "version",		1, 0, 'v' },
	{ 0, 0, 0, 0 }
};

static GIOChannel *do_listen(GError **err)
{
	if (fragment)
		return bt_io_listen(connect_cb, NULL, NULL, NULL, err,
					BT_IO_OPT_SOURCE_BDADDR, &src,
					BT_IO_OPT_PSM, AVDTP_PSM,
					BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
					BT_IO_OPT_MTU, 48,
					BT_IO_OPT_INVALID);

	return bt_io_listen(connect_cb, NULL, NULL, NULL, err,
					BT_IO_OPT_SOURCE_BDADDR, &src,
					BT_IO_OPT_PSM, AVDTP_PSM,
					BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
					BT_IO_OPT_INVALID);
}

int main(int argc, char *argv[])
{
	GError *err = NULL;
	int opt;

	bacpy(&src, BDADDR_ANY);
	bacpy(&dst, BDADDR_ANY);

	mainloop = g_main_loop_new(NULL, FALSE);
	if (!mainloop) {
		printf("Failed to create main loop\n");

		exit(1);
	}

	while ((opt = getopt_long(argc, argv, "d:hi:s:c:v:lrfp",
						main_options, NULL)) != EOF) {
		switch (opt) {
		case 'i':
			if (!strncmp(optarg, "hci", 3))
				hci_devba(atoi(optarg + 3), &src);
			else
				str2ba(optarg, &src);
			break;
		case 'd':
			if (!strncasecmp(optarg, "SRC", sizeof("SRC"))) {
				dev_role = AVDTP_SEP_TYPE_SOURCE;
			} else if (!strncasecmp(optarg, "SINK",
							sizeof("SINK"))) {
				dev_role = AVDTP_SEP_TYPE_SINK;
			} else {
				usage();
				exit(1);
			}
			break;
		case 'c':
			if (str2ba(optarg, &dst) < 0) {
				usage();
				exit(1);
			}
			break;
		case 'l':
			bacpy(&dst, BDADDR_ANY);
			break;
		case 'r':
			reject = true;
			break;
		case 'f':
			fragment = true;
			break;
		case 'p':
			preconf = true;
			break;
		case 's':
			parse_command(optarg);
			break;
		case 'v':
			version = strtol(optarg, NULL, 0);
			if (version != 0x0100 && version != 0x0102 &&
							version != 0x0103) {
				printf("invalid version\n");
				exit(1);
			}

			break;
		case 'h':
			usage();
			exit(0);
		default:
			usage();
			exit(1);
		}
	}

	lseps = queue_new();

	local_sep = avdtp_register_sep(lseps, dev_role, AVDTP_MEDIA_TYPE_AUDIO,
					0x00, TRUE, &sep_ind, &sep_cfm, NULL);
	if (!local_sep) {
		printf("Failed to register sep\n");
		exit(1);
	}

	queue_push_tail(lseps, local_sep);

	if (!bacmp(&dst, BDADDR_ANY)) {
		printf("Listening...\n");
		io = do_listen(&err);
	} else {
		printf("Connecting...\n");
		io = do_connect(&err);
	}

	if (!io) {
		printf("Failed: %s\n", err->message);
		g_error_free(err);
		exit(1);
	}

	g_main_loop_run(mainloop);

	printf("Done\n");

	queue_destroy(lseps, NULL);

	avdtp_unref(avdtp);
	avdtp = NULL;

	g_main_loop_unref(mainloop);
	mainloop = NULL;

	return 0;
}
