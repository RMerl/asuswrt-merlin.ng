/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2013-2014  Intel Corporation. All rights reserved.
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

#include "lib/bluetooth.h"
#include "btio/btio.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"
#include "src/sdp-client.h"
#include "src/sdpd.h"
#include "src/log.h"

#include "hal-msg.h"
#include "ipc-common.h"
#include "ipc.h"
#include "utils.h"
#include "bluetooth.h"
#include "socket.h"

#define RFCOMM_CHANNEL_MAX 30

#define OPP_DEFAULT_CHANNEL	9
#define HSP_AG_DEFAULT_CHANNEL	12
#define HFP_AG_DEFAULT_CHANNEL	13
#define PBAP_DEFAULT_CHANNEL	15
#define MAP_MAS_DEFAULT_CHANNEL	16

#define SVC_HINT_OBEX 0x10

/* Hardcoded MAP stuff needed for MAS SMS Instance.*/
#define DEFAULT_MAS_INSTANCE	0x00

#define MAP_MSG_TYPE_SMS_GSM	0x02
#define MAP_MSG_TYPE_SMS_CDMA	0x04
#define DEFAULT_MAS_MSG_TYPE	(MAP_MSG_TYPE_SMS_GSM | MAP_MSG_TYPE_SMS_CDMA)

static struct ipc *hal_ipc = NULL;
struct rfcomm_sock {
	int channel;	/* RFCOMM channel */
	BtIOSecLevel sec_level;

	/* for socket to BT */
	int bt_sock;
	guint bt_watch;

	/* for socket to HAL */
	int jv_sock;
	guint jv_watch;

	bdaddr_t dst;
	uint32_t service_handle;

	uint8_t *buf;
	int buf_size;
};

struct rfcomm_channel {
	bool reserved;
	struct rfcomm_sock *rfsock;
};

static bdaddr_t adapter_addr;

static uint8_t hal_mode = HAL_MODE_SOCKET_DEFAULT;

static const uint8_t zero_uuid[16] = { 0 };

/* Simple list of RFCOMM connected sockets */
static GList *connections = NULL;

static struct rfcomm_channel servers[RFCOMM_CHANNEL_MAX + 1];

static uint32_t test_sdp_record_uuid16 = 0;
static uint32_t test_sdp_record_uuid32 = 0;
static uint32_t test_sdp_record_uuid128 = 0;

static int rfsock_set_buffer(struct rfcomm_sock *rfsock)
{
	socklen_t len = sizeof(int);
	int rcv, snd, size, err;

	err = getsockopt(rfsock->bt_sock, SOL_SOCKET, SO_RCVBUF, &rcv, &len);
	if (err < 0) {
		int err = -errno;
		error("getsockopt(SO_RCVBUF): %s", strerror(-err));
		return err;
	}

	err = getsockopt(rfsock->bt_sock, SOL_SOCKET, SO_SNDBUF, &snd, &len);
	if (err < 0) {
		int err = -errno;
		error("getsockopt(SO_SNDBUF): %s", strerror(-err));
		return err;
	}

	size = MAX(rcv, snd);

	DBG("Set buffer size %d", size);

	rfsock->buf = g_malloc(size);
	rfsock->buf_size = size;

	return 0;
}

static void cleanup_rfsock(gpointer data)
{
	struct rfcomm_sock *rfsock = data;

	DBG("rfsock %p bt_sock %d jv_sock %d", rfsock, rfsock->bt_sock,
							rfsock->jv_sock);

	if (rfsock->jv_sock >= 0)
		if (close(rfsock->jv_sock) < 0)
			error("close() fd %d failed: %s", rfsock->jv_sock,
							strerror(errno));

	if (rfsock->bt_sock >= 0)
		if (close(rfsock->bt_sock) < 0)
			error("close() fd %d: failed: %s", rfsock->bt_sock,
							strerror(errno));

	if (rfsock->bt_watch > 0)
		if (!g_source_remove(rfsock->bt_watch))
			error("bt_watch source was not found");

	if (rfsock->jv_watch > 0)
		if (!g_source_remove(rfsock->jv_watch))
			error("stack_watch source was not found");

	if (rfsock->service_handle)
		bt_adapter_remove_record(rfsock->service_handle);

	if (rfsock->buf)
		g_free(rfsock->buf);

	g_free(rfsock);
}

static struct rfcomm_sock *create_rfsock(int bt_sock, int *hal_sock)
{
	int fds[2] = {-1, -1};
	struct rfcomm_sock *rfsock;

	if (socketpair(AF_LOCAL, SOCK_STREAM, 0, fds) < 0) {
		error("socketpair(): %s", strerror(errno));
		*hal_sock = -1;
		return NULL;
	}

	rfsock = g_new0(struct rfcomm_sock, 1);
	rfsock->jv_sock = fds[0];
	*hal_sock = fds[1];
	rfsock->bt_sock = bt_sock;

	DBG("rfsock %p", rfsock);

	if (bt_sock < 0)
		return rfsock;

	if (rfsock_set_buffer(rfsock) < 0) {
		cleanup_rfsock(rfsock);
		return NULL;
	}

	return rfsock;
}

static sdp_record_t *create_rfcomm_record(uint8_t chan, uuid_t *uuid,
						const char *svc_name,
						bool has_obex)
{
	sdp_list_t *svclass_id;
	sdp_list_t *seq, *proto_seq, *pbg_seq;
	sdp_list_t *proto[3];
	uuid_t l2cap_uuid, rfcomm_uuid, obex_uuid, pbg_uuid;
	sdp_data_t *channel;
	sdp_record_t *record;

	record = sdp_record_alloc();
	if (!record)
		return NULL;

	record->handle =  sdp_next_handle();

	svclass_id = sdp_list_append(NULL, uuid);
	sdp_set_service_classes(record, svclass_id);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[0] = sdp_list_append(NULL, &l2cap_uuid);
	seq = sdp_list_append(NULL, proto[0]);

	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	proto[1] = sdp_list_append(NULL, &rfcomm_uuid);
	channel = sdp_data_alloc(SDP_UINT8, &chan);
	proto[1] = sdp_list_append(proto[1], channel);
	seq = sdp_list_append(seq, proto[1]);

	if (has_obex) {
		sdp_uuid16_create(&obex_uuid, OBEX_UUID);
		proto[2] = sdp_list_append(NULL, &obex_uuid);
		seq = sdp_list_append(seq, proto[2]);
	}

	proto_seq = sdp_list_append(NULL, seq);
	sdp_set_access_protos(record, proto_seq);

	sdp_uuid16_create(&pbg_uuid, PUBLIC_BROWSE_GROUP);
	pbg_seq = sdp_list_append(NULL, &pbg_uuid);
	sdp_set_browse_groups(record, pbg_seq);

	if (svc_name)
		sdp_set_info_attr(record, svc_name, NULL, NULL);

	sdp_data_free(channel);
	sdp_list_free(proto[0], NULL);
	sdp_list_free(proto[1], NULL);
	if (has_obex)
		sdp_list_free(proto[2], NULL);
	sdp_list_free(seq, NULL);
	sdp_list_free(proto_seq, NULL);
	sdp_list_free(pbg_seq, NULL);
	sdp_list_free(svclass_id, NULL);

	return record;
}

static sdp_record_t *create_opp_record(uint8_t chan, const char *svc_name)
{
	uint8_t formats[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xff };
	uint8_t dtd = SDP_UINT8;
	uuid_t uuid;
	sdp_list_t *seq;
	sdp_profile_desc_t profile[1];
	void *dtds[sizeof(formats)], *values[sizeof(formats)];
	sdp_data_t *formats_list;
	sdp_record_t *record;
	size_t i;

	sdp_uuid16_create(&uuid, OBEX_OBJPUSH_SVCLASS_ID);

	record = create_rfcomm_record(chan, &uuid, svc_name, true);
	if (!record)
		return NULL;

	sdp_uuid16_create(&profile[0].uuid, OBEX_OBJPUSH_PROFILE_ID);
	profile[0].version = 0x0100;
	seq = sdp_list_append(NULL, profile);
	sdp_set_profile_descs(record, seq);

	for (i = 0; i < sizeof(formats); i++) {
		dtds[i] = &dtd;
		values[i] = &formats[i];
	}
	formats_list = sdp_seq_alloc(dtds, values, sizeof(formats));
	sdp_attr_add(record, SDP_ATTR_SUPPORTED_FORMATS_LIST, formats_list);

	sdp_list_free(seq, NULL);

	return record;
}

static sdp_record_t *create_pbap_record(uint8_t chan, const char *svc_name)
{
	sdp_list_t *seq;
	sdp_profile_desc_t profile[1];
	uint8_t formats = 0x01;
	sdp_record_t *record;
	uuid_t uuid;

	sdp_uuid16_create(&uuid, PBAP_PSE_SVCLASS_ID);

	record = create_rfcomm_record(chan, &uuid, svc_name, true);
	if (!record)
		return NULL;

	sdp_uuid16_create(&profile[0].uuid, PBAP_PROFILE_ID);
	profile[0].version = 0x0101;
	seq = sdp_list_append(NULL, profile);
	sdp_set_profile_descs(record, seq);

	sdp_attr_add_new(record, SDP_ATTR_SUPPORTED_REPOSITORIES, SDP_UINT8,
								&formats);

	sdp_list_free(seq, NULL);

	return record;
}

static sdp_record_t *create_mas_record(uint8_t chan, const char *svc_name)
{
	sdp_list_t *seq;
	sdp_profile_desc_t profile[1];
	uint8_t minst, mtype;
	sdp_record_t *record;
	uuid_t uuid;
	int cnt, ret;

	switch (hal_mode) {
	case HAL_MODE_SOCKET_DYNAMIC_MAP:
		/*
		 * Service name for MAP is passed as XXYYname
		 * XX - instance
		 * YY - message type
		 */
		ret = sscanf(svc_name, "%02hhx%02hhx%n", &minst, &mtype, &cnt);
		if (ret != 2 || cnt != 4)
			return NULL;

		svc_name += 4;
		break;
	case HAL_MODE_SOCKET_DEFAULT:
		minst = DEFAULT_MAS_INSTANCE;
		mtype = DEFAULT_MAS_MSG_TYPE;
		break;
	default:
		return NULL;
	}

	sdp_uuid16_create(&uuid, MAP_MSE_SVCLASS_ID);

	record = create_rfcomm_record(chan, &uuid, svc_name, true);
	if (!record)
		return NULL;

	sdp_uuid16_create(&profile[0].uuid, MAP_PROFILE_ID);
	profile[0].version = 0x0101;
	seq = sdp_list_append(NULL, profile);
	sdp_set_profile_descs(record, seq);

	sdp_attr_add_new(record, SDP_ATTR_MAS_INSTANCE_ID, SDP_UINT8, &minst);
	sdp_attr_add_new(record, SDP_ATTR_SUPPORTED_MESSAGE_TYPES, SDP_UINT8,
									&mtype);

	sdp_list_free(seq, NULL);

	return record;
}

static sdp_record_t *create_spp_record(uint8_t chan, const char *svc_name)
{
	sdp_record_t *record;
	uuid_t uuid;

	sdp_uuid16_create(&uuid, SERIAL_PORT_SVCLASS_ID);

	record = create_rfcomm_record(chan, &uuid, svc_name, false);
	if (!record)
		return NULL;

	return record;
}

static sdp_record_t *create_app_record(uint8_t chan,
						const uint8_t *app_uuid,
						const char *svc_name)
{
	sdp_record_t *record;
	uuid_t uuid;

	sdp_uuid128_create(&uuid, app_uuid);
	sdp_uuid128_to_uuid(&uuid);

	record = create_rfcomm_record(chan, &uuid, svc_name, false);
	if (!record)
		return NULL;

	return record;
}

static const struct profile_info {
	uint8_t		uuid[16];
	uint8_t		channel;
	uint8_t		svc_hint;
	BtIOSecLevel	sec_level;
	sdp_record_t *	(*create_record)(uint8_t chan, const char *svc_name);
} profiles[] = {
	{
		.uuid = {
			0x00, 0x00, 0x11, 0x08, 0x00, 0x00, 0x10, 0x00,
			0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB
		},
		.channel = HSP_AG_DEFAULT_CHANNEL,
		.svc_hint = 0,
		.sec_level = BT_IO_SEC_MEDIUM,
		.create_record = NULL
	}, {
		.uuid = {
			0x00, 0x00, 0x11, 0x1F, 0x00, 0x00, 0x10, 0x00,
			0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB
		},
		.channel = HFP_AG_DEFAULT_CHANNEL,
		.svc_hint = 0,
		.sec_level = BT_IO_SEC_MEDIUM,
		.create_record = NULL
	}, {
		.uuid = {
			0x00, 0x00, 0x11, 0x2F, 0x00, 0x00, 0x10, 0x00,
			0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB
		},
		.channel = PBAP_DEFAULT_CHANNEL,
		.svc_hint = SVC_HINT_OBEX,
		.sec_level = BT_IO_SEC_MEDIUM,
		.create_record = create_pbap_record
	}, {
		.uuid = {
			0x00, 0x00, 0x11, 0x05, 0x00, 0x00, 0x10, 0x00,
			0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB
		  },
		.channel = OPP_DEFAULT_CHANNEL,
		.svc_hint = SVC_HINT_OBEX,
		.sec_level = BT_IO_SEC_LOW,
		.create_record = create_opp_record
	}, {
		.uuid = {
			0x00, 0x00, 0x11, 0x32, 0x00, 0x00, 0x10, 0x00,
			0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB
		},
		.channel = MAP_MAS_DEFAULT_CHANNEL,
		.svc_hint = SVC_HINT_OBEX,
		.sec_level = BT_IO_SEC_MEDIUM,
		.create_record = create_mas_record
	}, {
		.uuid = {
			0x00, 0x00, 0x11, 0x01, 0x00, 0x00, 0x10, 0x00,
			0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB
		},
		.channel = 0,
		.svc_hint = 0,
		.sec_level = BT_IO_SEC_MEDIUM,
		.create_record = create_spp_record
	},
};

static uint32_t sdp_service_register(uint8_t channel, const uint8_t *uuid,
					const struct profile_info *profile,
					const void *svc_name)
{
	sdp_record_t *record = NULL;
	uint8_t svc_hint = 0;

	if (profile && profile->create_record) {
		record = profile->create_record(channel, svc_name);
		svc_hint = profile->svc_hint;
	} else if (uuid) {
		record = create_app_record(channel, uuid, svc_name);
	}

	if (!record)
		return 0;

	if (bt_adapter_add_record(record, svc_hint) < 0) {
		error("Failed to register on SDP record");
		sdp_record_free(record);
		return 0;
	}

	return record->handle;
}

static int bt_sock_send_fd(int sock_fd, const void *buf, int len, int send_fd)
{
	ssize_t ret;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	struct iovec iv;
	char cmsgbuf[CMSG_SPACE(sizeof(int))];

	DBG("len %d sock_fd %d send_fd %d", len, sock_fd, send_fd);

	if (sock_fd == -1 || send_fd == -1)
		return -1;

	memset(&msg, 0, sizeof(msg));
	memset(cmsgbuf, 0, sizeof(cmsgbuf));

	msg.msg_control = cmsgbuf;
	msg.msg_controllen = sizeof(cmsgbuf);

	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(send_fd));

	memcpy(CMSG_DATA(cmsg), &send_fd, sizeof(send_fd));

	iv.iov_base = (unsigned char *) buf;
	iv.iov_len = len;

	msg.msg_iov = &iv;
	msg.msg_iovlen = 1;

	ret = sendmsg(sock_fd, &msg, MSG_NOSIGNAL);
	if (ret < 0) {
		error("sendmsg(): sock_fd %d send_fd %d: %s",
					sock_fd, send_fd, strerror(errno));
		return ret;
	}

	return ret;
}

static const struct profile_info *get_profile_by_uuid(const uint8_t *uuid)
{
	unsigned int i;

	for (i = 0; i < G_N_ELEMENTS(profiles); i++) {
		if (!memcmp(profiles[i].uuid, uuid, 16))
			return &profiles[i];
	}

	return NULL;
}

static int try_write_all(int fd, unsigned char *buf, int len)
{
	int sent = 0;

	while (len > 0) {
		int written;

		written = write(fd, buf, len);
		if (written < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			return -1;
		}

		if (!written)
			return 0;

		len -= written; buf += written; sent += written;
	}

	return sent;
}

static gboolean jv_sock_client_event_cb(GIOChannel *io, GIOCondition cond,
								gpointer data)
{
	struct rfcomm_sock *rfsock = data;
	int len, sent;

	if (cond & G_IO_HUP) {
		DBG("Socket %d hang up", g_io_channel_unix_get_fd(io));
		goto fail;
	}

	if (cond & (G_IO_ERR | G_IO_NVAL)) {
		error("Socket %d error", g_io_channel_unix_get_fd(io));
		goto fail;
	}

	len = read(rfsock->jv_sock, rfsock->buf, rfsock->buf_size);
	if (len <= 0) {
		error("read(): %s", strerror(errno));
		/* Read again */
		return TRUE;
	}

	sent = try_write_all(rfsock->bt_sock, rfsock->buf, len);
	if (sent < 0) {
		error("write(): %s", strerror(errno));
		goto fail;
	}

	return TRUE;
fail:
	DBG("rfsock %p jv_sock %d cond %d", rfsock, rfsock->jv_sock, cond);

	connections = g_list_remove(connections, rfsock);
	cleanup_rfsock(rfsock);

	return FALSE;
}

static gboolean bt_sock_event_cb(GIOChannel *io, GIOCondition cond,
								gpointer data)
{
	struct rfcomm_sock *rfsock = data;
	int len, sent;

	if (cond & G_IO_HUP) {
		DBG("Socket %d hang up", g_io_channel_unix_get_fd(io));
		goto fail;
	}

	if (cond & (G_IO_ERR | G_IO_NVAL)) {
		error("Socket %d error", g_io_channel_unix_get_fd(io));
		goto fail;
	}

	len = read(rfsock->bt_sock, rfsock->buf, rfsock->buf_size);
	if (len <= 0) {
		error("read(): %s", strerror(errno));
		/* Read again */
		return TRUE;
	}

	sent = try_write_all(rfsock->jv_sock, rfsock->buf, len);
	if (sent < 0) {
		error("write(): %s", strerror(errno));
		goto fail;
	}

	return TRUE;
fail:
	DBG("rfsock %p bt_sock %d cond %d", rfsock, rfsock->bt_sock, cond);

	connections = g_list_remove(connections, rfsock);
	cleanup_rfsock(rfsock);

	return FALSE;
}

static bool sock_send_accept(struct rfcomm_sock *rfsock, bdaddr_t *bdaddr,
							int fd_accepted)
{
	struct hal_sock_connect_signal cmd;
	int len;

	DBG("");

	cmd.size = sizeof(cmd);
	bdaddr2android(bdaddr, cmd.bdaddr);
	cmd.channel = rfsock->channel;
	cmd.status = 0;

	len = bt_sock_send_fd(rfsock->jv_sock, &cmd, sizeof(cmd), fd_accepted);
	if (len != sizeof(cmd)) {
		error("Error sending accept signal");
		return false;
	}

	return true;
}

static gboolean jv_sock_server_event_cb(GIOChannel *io, GIOCondition cond,
								gpointer data)
{
	struct rfcomm_sock *rfsock = data;

	DBG("rfsock %p jv_sock %d cond %d", rfsock, rfsock->jv_sock, cond);

	if (cond & G_IO_NVAL)
		return FALSE;

	if (cond & (G_IO_ERR | G_IO_HUP)) {
		servers[rfsock->channel].rfsock = NULL;
		cleanup_rfsock(rfsock);
	}

	return FALSE;
}

static void accept_cb(GIOChannel *io, GError *err, gpointer user_data)
{
	struct rfcomm_sock *rfsock = user_data;
	struct rfcomm_sock *new_rfsock;
	GIOChannel *jv_io;
	GError *gerr = NULL;
	bdaddr_t dst;
	char address[18];
	int new_sock;
	int hal_sock;
	guint id;
	GIOCondition cond;

	if (err) {
		error("%s", err->message);
		return;
	}

	bt_io_get(io, &gerr,
			BT_IO_OPT_DEST_BDADDR, &dst,
			BT_IO_OPT_INVALID);
	if (gerr) {
		error("%s", gerr->message);
		g_error_free(gerr);
		g_io_channel_shutdown(io, TRUE, NULL);
		return;
	}

	ba2str(&dst, address);
	DBG("Incoming connection from %s on channel %d (rfsock %p)", address,
						rfsock->channel, rfsock);

	new_sock = g_io_channel_unix_get_fd(io);
	new_rfsock = create_rfsock(new_sock, &hal_sock);
	if (!new_rfsock) {
		g_io_channel_shutdown(io, TRUE, NULL);
		return;
	}

	DBG("new rfsock %p bt_sock %d jv_sock %d hal_sock %d", new_rfsock,
			new_rfsock->bt_sock, new_rfsock->jv_sock, hal_sock);

	if (!sock_send_accept(rfsock, &dst, hal_sock)) {
		cleanup_rfsock(new_rfsock);
		return;
	}

	connections = g_list_append(connections, new_rfsock);

	/* Handle events from Android */
	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	jv_io = g_io_channel_unix_new(new_rfsock->jv_sock);
	id = g_io_add_watch(jv_io, cond, jv_sock_client_event_cb, new_rfsock);
	g_io_channel_unref(jv_io);

	new_rfsock->jv_watch = id;

	/* Handle rfcomm events */
	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	id = g_io_add_watch(io, cond, bt_sock_event_cb, new_rfsock);
	g_io_channel_set_close_on_unref(io, FALSE);

	new_rfsock->bt_watch = id;
}

static int find_free_channel(void)
{
	int ch;

	/* channel 0 is reserver so we don't use it */
	for (ch = 1; ch <= RFCOMM_CHANNEL_MAX; ch++) {
		struct rfcomm_channel *srv = &servers[ch];

		if (!srv->reserved && srv->rfsock == NULL)
			return ch;
	}

	return 0;
}

static BtIOSecLevel get_sec_level(uint8_t flags)
{
	/*
	 * HAL_SOCK_FLAG_AUTH should require MITM but in our case setting
	 * security to BT_IO_SEC_HIGH would also require 16-digits PIN code
	 * for pre-2.1 devices which is not what Android expects. For this
	 * reason we ignore this flag to not break apps which use "secure"
	 * sockets (have both auth and encrypt flags set, there is no public
	 * API in Android which should provide proper high security socket).
	 */
	return flags & HAL_SOCK_FLAG_ENCRYPT ? BT_IO_SEC_MEDIUM :
							BT_IO_SEC_LOW;
}

static uint8_t rfcomm_listen(int chan, const uint8_t *name, const uint8_t *uuid,
						uint8_t flags, int *hal_sock)
{
	const struct profile_info *profile;
	struct rfcomm_sock *rfsock = NULL;
	BtIOSecLevel sec_level;
	GIOChannel *io, *jv_io;
	GIOCondition cond;
	GError *err = NULL;
	guint id;
	uuid_t uu;
	char uuid_str[32];

	sdp_uuid128_create(&uu, uuid);
	sdp_uuid2strn(&uu, uuid_str, sizeof(uuid_str));

	DBG("chan %d flags 0x%02x uuid %s name %s", chan, flags, uuid_str,
									name);

	if ((!memcmp(uuid, zero_uuid, sizeof(zero_uuid)) && chan <= 0) ||
			(chan > RFCOMM_CHANNEL_MAX)) {
		error("Invalid rfcomm listen params");
		return HAL_STATUS_INVALID;
	}

	profile = get_profile_by_uuid(uuid);
	if (!profile) {
		sec_level = get_sec_level(flags);
	} else {
		if (!profile->create_record)
			return HAL_STATUS_INVALID;

		chan = profile->channel;
		sec_level = profile->sec_level;
	}

	if (chan <= 0)
		chan = find_free_channel();

	if (!chan) {
		error("No free channels");
		return HAL_STATUS_BUSY;
	}

	if (servers[chan].rfsock != NULL) {
		error("Channel already registered (%d)", chan);
		return HAL_STATUS_BUSY;
	}

	DBG("chan %d sec_level %d", chan, sec_level);

	rfsock = create_rfsock(-1, hal_sock);
	if (!rfsock)
		return HAL_STATUS_FAILED;

	rfsock->channel = chan;

	io = bt_io_listen(accept_cb, NULL, rfsock, NULL, &err,
				BT_IO_OPT_SOURCE_BDADDR, &adapter_addr,
				BT_IO_OPT_CHANNEL, chan,
				BT_IO_OPT_SEC_LEVEL, sec_level,
				BT_IO_OPT_INVALID);
	if (!io) {
		error("Failed listen: %s", err->message);
		g_error_free(err);
		goto failed;
	}

	rfsock->bt_sock = g_io_channel_unix_get_fd(io);

	g_io_channel_set_close_on_unref(io, FALSE);
	g_io_channel_unref(io);

	/* Handle events from Android */
	cond = G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	jv_io = g_io_channel_unix_new(rfsock->jv_sock);
	id = g_io_add_watch_full(jv_io, G_PRIORITY_HIGH, cond,
					jv_sock_server_event_cb, rfsock,
					NULL);
	g_io_channel_unref(jv_io);

	rfsock->jv_watch = id;

	DBG("rfsock %p bt_sock %d jv_sock %d hal_sock %d", rfsock,
								rfsock->bt_sock,
								rfsock->jv_sock,
								*hal_sock);

	if (write(rfsock->jv_sock, &chan, sizeof(chan)) != sizeof(chan)) {
		error("Error sending RFCOMM channel");
		goto failed;
	}

	rfsock->service_handle = sdp_service_register(chan, uuid, profile,
									name);

	servers[chan].rfsock = rfsock;

	return HAL_STATUS_SUCCESS;

failed:

	cleanup_rfsock(rfsock);
	close(*hal_sock);
	return HAL_STATUS_FAILED;
}

static uint32_t add_test_record(uuid_t *uuid)
{
	sdp_record_t *record;
	sdp_list_t *svclass_id;
	sdp_list_t *seq, *pbg_seq, *proto_seq, *ap_seq;
	sdp_list_t *proto, *proto1, *aproto;
	uuid_t l2cap_uuid, pbg_uuid, ap_uuid;

	record = sdp_record_alloc();
	if (!record)
		return 0;

	record->handle =  sdp_next_handle();

	svclass_id = sdp_list_append(NULL, uuid);
	sdp_set_service_classes(record, svclass_id);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto = sdp_list_append(NULL, &l2cap_uuid);
	seq = sdp_list_append(NULL, proto);

	proto_seq = sdp_list_append(NULL, seq);
	sdp_set_access_protos(record, proto_seq);

	sdp_uuid16_create(&pbg_uuid, PUBLIC_BROWSE_GROUP);
	pbg_seq = sdp_list_append(NULL, &pbg_uuid);
	sdp_set_browse_groups(record, pbg_seq);

	/* Additional Protocol Descriptor List */
	sdp_uuid16_create(&ap_uuid, L2CAP_UUID);
	proto1 = sdp_list_append(NULL, &ap_uuid);
	ap_seq = sdp_list_append(NULL, proto1);
	aproto = sdp_list_append(NULL, ap_seq);
	sdp_set_add_access_protos(record, aproto);

	sdp_set_service_id(record, *uuid);
	sdp_set_record_state(record, 0);
	sdp_set_service_ttl(record, 0);
	sdp_set_service_avail(record, 0);
	sdp_set_url_attr(record, "http://www.bluez.org",
				"http://www.bluez.org", "http://www.bluez.org");

	sdp_list_free(proto, NULL);
	sdp_list_free(seq, NULL);
	sdp_list_free(proto_seq, NULL);
	sdp_list_free(pbg_seq, NULL);
	sdp_list_free(svclass_id, NULL);

	if (bt_adapter_add_record(record, 0) < 0) {
		sdp_record_free(record);
		return 0;
	}

	return record->handle;
}

static void test_sdp_cleanup(void)
{
	if (test_sdp_record_uuid16) {
		bt_adapter_remove_record(test_sdp_record_uuid16);
		test_sdp_record_uuid16 = 0;
	}

	if (test_sdp_record_uuid32) {
		bt_adapter_remove_record(test_sdp_record_uuid32);
		test_sdp_record_uuid32 = 0;
	}

	if (test_sdp_record_uuid128) {
		bt_adapter_remove_record(test_sdp_record_uuid128);
		test_sdp_record_uuid128 = 0;
	}
}

static void test_sdp_init(void)
{
	char uuid128[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	uuid_t u;

	sdp_uuid16_create(&u, 0xffff);
	test_sdp_record_uuid16 = add_test_record(&u);

	sdp_uuid32_create(&u, 0xffffffff);
	test_sdp_record_uuid32 = add_test_record(&u);

	sdp_uuid128_create(&u, uuid128);
	test_sdp_record_uuid128 = add_test_record(&u);
}

static uint8_t l2cap_listen(int chan, const uint8_t *name, const uint8_t *uuid,
						uint8_t flags, int *hal_sock)
{
	/* TODO be more strict here? */
	if (strcmp("BlueZ", (const char *) name)) {
		error("socket: Only SDP test supported on L2CAP");
		return HAL_STATUS_UNSUPPORTED;
	}

	test_sdp_cleanup();
	test_sdp_init();

	*hal_sock = -1;

	return HAL_STATUS_SUCCESS;
}

static void handle_listen(const void *buf, uint16_t len)
{
	const struct hal_cmd_socket_listen *cmd = buf;
	uint8_t status;
	int hal_sock;

	switch (cmd->type) {
	case HAL_SOCK_RFCOMM:
		status = rfcomm_listen(cmd->channel, cmd->name, cmd->uuid,
							cmd->flags, &hal_sock);
		break;
	case HAL_SOCK_L2CAP:
		status = l2cap_listen(cmd->channel, cmd->name, cmd->uuid,
							cmd->flags, &hal_sock);
		break;
	case HAL_SOCK_SCO:
		status = HAL_STATUS_UNSUPPORTED;
		break;
	default:
		status = HAL_STATUS_INVALID;
		break;
	}

	if (status != HAL_STATUS_SUCCESS)
		goto failed;

	ipc_send_rsp_full(hal_ipc, HAL_SERVICE_ID_SOCKET, HAL_OP_SOCKET_LISTEN,
							0, NULL, hal_sock);
	close(hal_sock);
	return;

failed:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_SOCKET, HAL_OP_SOCKET_LISTEN,
									status);
}

static bool sock_send_connect(struct rfcomm_sock *rfsock, bdaddr_t *bdaddr)
{
	struct hal_sock_connect_signal cmd;
	int len;

	DBG("");

	memset(&cmd, 0, sizeof(cmd));
	cmd.size = sizeof(cmd);
	bdaddr2android(bdaddr, cmd.bdaddr);
	cmd.channel = rfsock->channel;
	cmd.status = 0;

	len = write(rfsock->jv_sock, &cmd, sizeof(cmd));
	if (len < 0) {
		error("%s", strerror(errno));
		return false;
	}

	if (len != sizeof(cmd)) {
		error("Error sending connect signal");
		return false;
	}

	return true;
}

static void connect_cb(GIOChannel *io, GError *err, gpointer user_data)
{
	struct rfcomm_sock *rfsock = user_data;
	bdaddr_t *dst = &rfsock->dst;
	GIOChannel *jv_io;
	char address[18];
	guint id;
	GIOCondition cond;

	if (err) {
		error("%s", err->message);
		goto fail;
	}

	ba2str(dst, address);
	DBG("Connected to %s on channel %d (rfsock %p)", address,
						rfsock->channel, rfsock);

	if (!sock_send_connect(rfsock, dst))
		goto fail;

	/* Handle events from Android */
	cond = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	jv_io = g_io_channel_unix_new(rfsock->jv_sock);
	id = g_io_add_watch(jv_io, cond, jv_sock_client_event_cb, rfsock);
	g_io_channel_unref(jv_io);

	rfsock->jv_watch = id;

	/* Handle rfcomm events */
	cond = G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL;
	id = g_io_add_watch(io, cond, bt_sock_event_cb, rfsock);
	g_io_channel_set_close_on_unref(io, FALSE);

	rfsock->bt_watch = id;

	return;
fail:
	connections = g_list_remove(connections, rfsock);
	cleanup_rfsock(rfsock);
}

static bool do_rfcomm_connect(struct rfcomm_sock *rfsock, int chan)
{
	GIOChannel *io;
	GError *gerr = NULL;

	DBG("rfsock %p sec_level %d chan %d", rfsock, rfsock->sec_level, chan);

	io = bt_io_connect(connect_cb, rfsock, NULL, &gerr,
				BT_IO_OPT_SOURCE_BDADDR, &adapter_addr,
				BT_IO_OPT_DEST_BDADDR, &rfsock->dst,
				BT_IO_OPT_CHANNEL, chan,
				BT_IO_OPT_SEC_LEVEL, rfsock->sec_level,
				BT_IO_OPT_INVALID);
	if (!io) {
		error("Failed connect: %s", gerr->message);
		g_error_free(gerr);
		return false;
	}

	g_io_channel_set_close_on_unref(io, FALSE);
	g_io_channel_unref(io);

	if (write(rfsock->jv_sock, &chan, sizeof(chan)) != sizeof(chan)) {
		error("Error sending RFCOMM channel");
		return false;
	}

	rfsock->bt_sock = g_io_channel_unix_get_fd(io);
	rfsock_set_buffer(rfsock);
	rfsock->channel = chan;
	connections = g_list_append(connections, rfsock);

	return true;
}

static void sdp_search_cb(sdp_list_t *recs, int err, gpointer data)
{
	struct rfcomm_sock *rfsock = data;
	sdp_list_t *list;
	int chan;

	DBG("");

	if (err < 0) {
		error("Unable to get SDP record: %s", strerror(-err));
		goto fail;
	}

	if (!recs || !recs->data) {
		error("No SDP records found");
		goto fail;
	}

	for (list = recs; list != NULL; list = list->next) {
		sdp_record_t *rec = list->data;
		sdp_list_t *protos;

		if (sdp_get_access_protos(rec, &protos) < 0) {
			error("Unable to get proto list");
			goto fail;
		}

		chan = sdp_get_proto_port(protos, RFCOMM_UUID);

		sdp_list_foreach(protos, (sdp_list_func_t) sdp_list_free,
									NULL);
		sdp_list_free(protos, NULL);

		if (chan)
			break;
	}

	if (chan <= 0) {
		error("Could not get RFCOMM channel %d", chan);
		goto fail;
	}

	DBG("Got RFCOMM channel %d", chan);

	if (do_rfcomm_connect(rfsock, chan))
		return;
fail:
	cleanup_rfsock(rfsock);
}

static uint8_t connect_rfcomm(const bdaddr_t *addr, int chan,
					const uint8_t *uuid, uint8_t flags,
					int *hal_sock)
{
	struct rfcomm_sock *rfsock;
	char address[18];
	uuid_t uu;
	char uuid_str[32];

	sdp_uuid128_create(&uu, uuid);
	sdp_uuid2strn(&uu, uuid_str, sizeof(uuid_str));
	ba2str(addr, address);

	DBG("addr %s chan %d flags 0x%02x uuid %s", address, chan, flags,
								uuid_str);

	if ((!memcmp(uuid, zero_uuid, sizeof(zero_uuid)) && chan <= 0) ||
						!bacmp(addr, BDADDR_ANY)) {
		error("Invalid rfcomm connect params");
		return HAL_STATUS_INVALID;
	}

	rfsock = create_rfsock(-1, hal_sock);
	if (!rfsock)
		return HAL_STATUS_FAILED;

	DBG("rfsock %p jv_sock %d hal_sock %d", rfsock, rfsock->jv_sock,
							*hal_sock);

	rfsock->sec_level = get_sec_level(flags);

	bacpy(&rfsock->dst, addr);

	if (!memcmp(uuid, zero_uuid, sizeof(zero_uuid))) {
		if (!do_rfcomm_connect(rfsock, chan))
			goto failed;
	} else {

		if (bt_search_service(&adapter_addr, &rfsock->dst, &uu,
					sdp_search_cb, rfsock, NULL, 0) < 0) {
			error("Failed to search SDP records");
			goto failed;
		}
	}

	return HAL_STATUS_SUCCESS;

failed:
	cleanup_rfsock(rfsock);
	close(*hal_sock);
	return HAL_STATUS_FAILED;
}

static void handle_connect(const void *buf, uint16_t len)
{
	const struct hal_cmd_socket_connect *cmd = buf;
	bdaddr_t bdaddr;
	uint8_t status;
	int hal_sock;

	DBG("");

	android2bdaddr(cmd->bdaddr, &bdaddr);

	switch (cmd->type) {
	case HAL_SOCK_RFCOMM:
		status = connect_rfcomm(&bdaddr, cmd->channel, cmd->uuid,
							cmd->flags, &hal_sock);
		break;
	case HAL_SOCK_SCO:
	case HAL_SOCK_L2CAP:
		status = HAL_STATUS_UNSUPPORTED;
		break;
	default:
		status = HAL_STATUS_INVALID;
		break;
	}

	if (status != HAL_STATUS_SUCCESS)
		goto failed;

	ipc_send_rsp_full(hal_ipc, HAL_SERVICE_ID_SOCKET, HAL_OP_SOCKET_CONNECT,
							0, NULL, hal_sock);
	close(hal_sock);
	return;

failed:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_SOCKET, HAL_OP_SOCKET_CONNECT,
									status);

}

static const struct ipc_handler cmd_handlers[] = {
	/* HAL_OP_SOCKET_LISTEN */
	{ handle_listen, false, sizeof(struct hal_cmd_socket_listen) },
	/* HAL_OP_SOCKET_CONNECT */
	{ handle_connect, false, sizeof(struct hal_cmd_socket_connect) },
};

void bt_socket_register(struct ipc *ipc, const bdaddr_t *addr, uint8_t mode)
{
	size_t i;

	DBG("");

	hal_mode = mode;

	/*
	 * make sure channels assigned for profiles are reserved and not used
	 * for app services
	 */
	for (i = 0; i < G_N_ELEMENTS(profiles); i++)
		if (profiles[i].channel)
			servers[profiles[i].channel].reserved = true;

	bacpy(&adapter_addr, addr);

	hal_ipc = ipc;
	ipc_register(hal_ipc, HAL_SERVICE_ID_SOCKET, cmd_handlers,
						G_N_ELEMENTS(cmd_handlers));
}

void bt_socket_unregister(void)
{
	int ch;

	DBG("");

	test_sdp_cleanup();

	g_list_free_full(connections, cleanup_rfsock);

	for (ch = 0; ch <= RFCOMM_CHANNEL_MAX; ch++)
		if (servers[ch].rfsock)
			cleanup_rfsock(servers[ch].rfsock);

	memset(servers, 0, sizeof(servers));

	ipc_unregister(hal_ipc, HAL_SERVICE_ID_SOCKET);
	hal_ipc = NULL;
}
