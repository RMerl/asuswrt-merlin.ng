/*
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  SAP Driver for ST-Ericsson U8500 platform
 *
 *  Copyright (C) 2010-2011 ST-Ericsson SA
 *
 *  Author: Waldemar Rymarkiewicz <waldemar.rymarkiewicz@tieto.com> for
 *  ST-Ericsson.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <glib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/un.h>

#include "src/log.h"
#include "sap.h"

#define STE_SIMD_SOCK  "/dev/socket/catd_a"
#define STE_CLIENT_TAG 0x0000

#define sap_error(fmt, arg...) do { \
	error("STE U8500 SAP: " fmt, ## arg); \
	} while (0)

#define sap_info(fmt, arg...) do { \
	info("STE U8500 SAP: " fmt, ## arg); \
	} while (0)

struct ste_message {
	uint16_t len;
	uint16_t id;
	uint32_t client_tag;
	uint8_t payload[0];
} __attribute__((packed));

#define STE_MSG_PAYLOAD_SIZE(msg) (msg->len - sizeof(*msg) + sizeof(msg->len))

enum ste_protocol {
	STE_START_SAP_REQ	= 0x2D01,
	STE_START_SAP_RSP	= 0x2E01,
	STE_END_SAP_REQ		= 0x2D02,
	STE_END_SAP_RSP		= 0x2E02,
	STE_POWER_OFF_REQ	= 0x2D03,
	STE_POWER_OFF_RSP	= 0x2E03,
	STE_POWER_ON_REQ	= 0x2D04,
	STE_POWER_ON_RSP	= 0x2E04,
	STE_RESET_REQ		= 0x2D05,
	STE_RESET_RSP		= 0x2E05,
	STE_SEND_APDU_REQ	= 0x2D06,
	STE_SEND_APDU_RSP	= 0x2E06,
	STE_GET_ATR_REQ		= 0x2D07,
	STE_GET_ATR_RSP		= 0x2E07,
	STE_GET_STATUS_REQ	= 0x2D08,
	STE_GET_STATUS_RSP	= 0x2E08,
	STE_STATUS_IND		= 0x2F02,
	STE_SIM_READY_IND	= 0x2F03,
};

enum ste_msg {
	STE_SEND_APDU_MSG,
	STE_GET_ATR_MSG,
	STE_POWER_OFF_MSG,
	STE_POWER_ON_MSG,
	STE_RESET_MSG,
	STE_GET_STATUS_MSG,
	STE_MSG_MAX,
};

enum ste_status {
	STE_STATUS_OK		= 0x00000000,
	STE_STATUS_FAILURE	= 0x00000001,
	STE_STATUS_BUSY_CALL	= 0x00000002,
};

enum ste_card_status {
	STE_CARD_STATUS_UNKNOWN		= 0x00,
	STE_CARD_STATUS_ACTIVE		= 0x01,
	STE_CARD_STATUS_NOT_ACTIVE	= 0x02,
	STE_CARD_STATUS_MISSING		= 0x03,
	STE_CARD_STATUS_INVALID		= 0x04,
	STE_CARD_STATUS_DISCONNECTED	= 0x05,
};

/* Card reader status bits as described in GSM 11.14, Section 12.33
 * Bits 0-2 are for card reader identity and always zeros. */
#define ICC_READER_REMOVABLE	(1 << 3)
#define ICC_READER_PRESENT	(1 << 4)
#define ICC_READER_ID1		(1 << 5)
#define ICC_READER_CARD_PRESENT	(1 << 6)
#define ICC_READER_CARD_POWERED	(1 << 7)

enum ste_state {
	STE_DISABLED,		/* Reader not present or removed */
	STE_POWERED_OFF,	/* Card in the reader but powered off */
	STE_NO_CARD,		/* No card in the reader */
	STE_ENABLED,		/* Card in the reader and powered on */
	STE_SIM_BUSY,		/* Modem is busy with ongoing call.*/
	STE_STATE_MAX
};

struct ste_u8500 {
	GIOChannel *io;
	enum ste_state state;
	void *sap_data;
};

typedef int(*recv_state_change_cb)(void *sap, uint8_t result);
typedef int(*recv_pdu_cb)(void *sap, uint8_t result, uint8_t *data,
								uint16_t len);

static struct ste_u8500 u8500;

static const uint8_t sim2sap_result[STE_MSG_MAX][STE_STATE_MAX] = {
	/* SAP results for SEND APDU message */
	{
		SAP_RESULT_ERROR_NOT_ACCESSIBLE,	/* STE_DISABLED */
		SAP_RESULT_ERROR_POWERED_OFF,		/* STE_POWERED_OFF */
		SAP_RESULT_ERROR_CARD_REMOVED,		/* STE_NO_CARD */
		SAP_RESULT_ERROR_NO_REASON		/* STE_ENABLED */
	},

	/* SAP results for GET_ATR message */
	{
		SAP_RESULT_ERROR_NO_REASON,
		SAP_RESULT_ERROR_POWERED_OFF,
		SAP_RESULT_ERROR_CARD_REMOVED,
		SAP_RESULT_ERROR_NO_REASON
	},

	/* SAP results POWER OFF message */
	{
		SAP_RESULT_ERROR_NO_REASON,
		SAP_RESULT_ERROR_POWERED_OFF,
		SAP_RESULT_ERROR_CARD_REMOVED,
		SAP_RESULT_ERROR_NO_REASON
	},

	/* SAP results POWER ON message */
	{
		SAP_RESULT_ERROR_NO_REASON,
		SAP_RESULT_ERROR_NOT_ACCESSIBLE,
		SAP_RESULT_ERROR_CARD_REMOVED,
		SAP_RESULT_ERROR_POWERED_ON
	},

	/* SAP results SIM RESET message */
	{
		SAP_RESULT_ERROR_NO_REASON,
		SAP_RESULT_ERROR_POWERED_OFF,
		SAP_RESULT_ERROR_CARD_REMOVED,
		SAP_RESULT_ERROR_NOT_ACCESSIBLE
	},

	/* SAP results GET STATUS message */
	{
		SAP_RESULT_ERROR_NO_REASON,
		SAP_RESULT_ERROR_NO_REASON,
		SAP_RESULT_ERROR_NO_REASON,
		SAP_RESULT_ERROR_NO_REASON
	}
};

static uint8_t get_sap_result(enum ste_msg msg, uint32_t status)
{
	if (!u8500.io)
		return SAP_RESULT_ERROR_NO_REASON;

	switch (status) {
	case STE_STATUS_OK:
		return SAP_RESULT_OK;

	case STE_STATUS_FAILURE:
		return sim2sap_result[msg][u8500.state];

	default:
		DBG("Can't convert a result (status %u)", status);
		return SAP_RESULT_ERROR_NO_REASON;
	}
}

static int get_sap_reader_status(uint32_t card_status, uint8_t *icc_status)
{
	/* Card reader is present, not removable and not ID-1 size. */
	*icc_status = ICC_READER_PRESENT;

	switch (card_status) {
	case STE_CARD_STATUS_ACTIVE:
		*icc_status |= ICC_READER_CARD_POWERED;

	case STE_CARD_STATUS_NOT_ACTIVE:
	case STE_CARD_STATUS_INVALID:
		*icc_status |= ICC_READER_CARD_PRESENT;

	case STE_CARD_STATUS_MISSING:
	case STE_CARD_STATUS_DISCONNECTED:
		return 0;

	default:
		DBG("Can't convert reader status %u", card_status);

	case STE_CARD_STATUS_UNKNOWN:
		return -1;
	}
}

static uint8_t get_sap_status_change(uint32_t card_status)
{
	if (!u8500.io)
		return SAP_STATUS_CHANGE_UNKNOWN_ERROR;

	switch (card_status) {
	case STE_CARD_STATUS_UNKNOWN:
		return SAP_STATUS_CHANGE_UNKNOWN_ERROR;

	case STE_CARD_STATUS_ACTIVE:
		u8500.state = STE_ENABLED;
		return SAP_STATUS_CHANGE_CARD_RESET;

	case STE_CARD_STATUS_NOT_ACTIVE:
		u8500.state = STE_DISABLED;
		return SAP_STATUS_CHANGE_CARD_NOT_ACCESSIBLE;

	case STE_CARD_STATUS_MISSING:
		u8500.state = STE_DISABLED;
		return SAP_STATUS_CHANGE_CARD_REMOVED;

	case STE_CARD_STATUS_INVALID:
		u8500.state = STE_DISABLED;
		return SAP_STATUS_CHANGE_CARD_NOT_ACCESSIBLE;

	default:
		DBG("Can't convert status change %u", card_status);
		return SAP_STATUS_CHANGE_UNKNOWN_ERROR;
	}
}

static int send_message(GIOChannel *io, void *buf, size_t size)
{
	gsize written;

	SAP_VDBG("io %p, size %zu", io, size);

	if (g_io_channel_write_chars(io, buf, size, &written, NULL) !=
							G_IO_STATUS_NORMAL)
		return -EIO;

	return written;
}

static int send_request(GIOChannel *io, uint16_t id,
						struct sap_parameter *param)
{
	int ret;
	struct ste_message *msg;
	size_t size = sizeof(*msg);

	SAP_VDBG("io %p", io);

	if (param)
		size += param->len;

	msg = g_try_malloc0(size);
	if (!msg) {
		sap_error("sending request failed: %s", strerror(ENOMEM));
		return -ENOMEM;
	}

	msg->len = size - sizeof(msg->len);
	msg->id = id;
	msg->client_tag = STE_CLIENT_TAG;

	if (param)
		memcpy(msg->payload, param->val, param->len);

	ret = send_message(io, msg, size);
	if (ret < 0) {
		sap_error("sending request failed: %s", strerror(-ret));
	} else if (ret != (int) size) {
		sap_error("sending request failed: %d out of %zu bytes sent",
								ret, size);
		ret = -EIO;
	}

	g_free(msg);

	return ret;
}

static void recv_status(uint32_t status)
{
	sap_status_ind(u8500.sap_data, get_sap_status_change(status));
}

static void recv_card_status(uint32_t status, uint8_t *param)
{
	uint32_t card_status;
	uint8_t result;
	uint8_t iccrs;

	if (status != STE_STATUS_OK)
		return;

	memcpy(&card_status, param, sizeof(card_status));

	if (get_sap_reader_status(card_status, &iccrs) < 0)
		result = SAP_RESULT_ERROR_NO_REASON;
	else
		result = get_sap_result(STE_GET_STATUS_MSG, status);

	sap_transfer_card_reader_status_rsp(u8500.sap_data, result, iccrs);
}

static void recv_state_change(uint32_t ste_msg, uint32_t status,
			uint32_t new_state, recv_state_change_cb callback)
{
	if (status != STE_STATUS_OK)
		return;

	u8500.state = new_state;

	if (callback)
		callback(u8500.sap_data, get_sap_result(ste_msg, status));
}

static void recv_pdu(uint32_t ste_msg, struct ste_message *msg, uint32_t status,
					uint8_t *param, recv_pdu_cb callback)
{
	uint8_t *data = NULL;
	uint8_t result;
	int size = 0;

	if (status == STE_STATUS_OK) {
		data = param;
		size = STE_MSG_PAYLOAD_SIZE(msg) - sizeof(status);
	}

	result = get_sap_result(ste_msg, status);

	if (callback)
		callback(u8500.sap_data, result, data, size);
}

static void simd_close(void)
{
	DBG("io %p", u8500.io);

	if (u8500.io) {
		g_io_channel_shutdown(u8500.io, TRUE, NULL);
		g_io_channel_unref(u8500.io);
	}

	u8500.state = STE_DISABLED;
	u8500.io = NULL;
	u8500.sap_data = NULL;
}

static void recv_sim_ready(void)
{
	sap_info("sim is ready. Try to connect again");

	if (send_request(u8500.io, STE_START_SAP_REQ, NULL) < 0) {
		sap_connect_rsp(u8500.sap_data, SAP_STATUS_CONNECTION_FAILED);
		simd_close();
	}
}

static void recv_connect_rsp(uint32_t status)
{
	switch (status) {
	case STE_STATUS_OK:
		if (u8500.state != STE_SIM_BUSY)
			sap_connect_rsp(u8500.sap_data, SAP_STATUS_OK);
		break;
	case STE_STATUS_BUSY_CALL:
		if (u8500.state != STE_SIM_BUSY) {
			sap_connect_rsp(u8500.sap_data,
						SAP_STATUS_OK_ONGOING_CALL);

			u8500.state = STE_SIM_BUSY;
		}
		break;
	default:
		sap_connect_rsp(u8500.sap_data, SAP_STATUS_CONNECTION_FAILED);
		simd_close();
		break;
	}
}

static void recv_response(struct ste_message *msg)
{
	uint32_t status;
	uint8_t *param;

	SAP_VDBG("msg_id 0x%x", msg->id);

	if (msg->id == STE_END_SAP_RSP) {
		sap_disconnect_rsp(u8500.sap_data);
		simd_close();
		return;
	}

	param = msg->payload;
	memcpy(&status, param, sizeof(status));
	param += sizeof(status);

	SAP_VDBG("status 0x%x", status);

	switch (msg->id) {
	case STE_START_SAP_RSP:
		recv_connect_rsp(status);
		break;
	case STE_SEND_APDU_RSP:
		recv_pdu(STE_SEND_APDU_MSG, msg, status, param,
							sap_transfer_apdu_rsp);
		break;

	case STE_GET_ATR_RSP:
		recv_pdu(STE_GET_ATR_MSG, msg, status, param,
							sap_transfer_atr_rsp);
		break;

	case STE_POWER_OFF_RSP:
		recv_state_change(STE_POWER_OFF_MSG, status, STE_POWERED_OFF,
							sap_power_sim_off_rsp);
		break;

	case STE_POWER_ON_RSP:
		recv_state_change(STE_POWER_ON_MSG, status, STE_ENABLED,
							sap_power_sim_on_rsp);
		break;

	case STE_RESET_RSP:
		recv_state_change(STE_RESET_MSG, status, STE_ENABLED,
							sap_reset_sim_rsp);
		break;

	case STE_GET_STATUS_RSP:
		recv_card_status(status, param);
		break;

	case STE_STATUS_IND:
		recv_status(status);
		break;

	case STE_SIM_READY_IND:
		recv_sim_ready();
		break;

	default:
		sap_error("unsupported message received (id 0x%x)", msg->id);
	}
}

static int recv_message(void *buf, size_t size)
{
	uint8_t *iter = buf;
	struct ste_message *msg = buf;

	do {
		SAP_VDBG("size %zu msg->len %u.", size, msg->len);

		if (size < sizeof(*msg)) {
			sap_error("invalid message received (%zu bytes)", size);
			return -EBADMSG;
		}

		/* Message must be complete. */
		if (size < (msg->len + sizeof(msg->len))) {
			sap_error("incomplete message received (%zu bytes)",
									size);
			return -EBADMSG;
		}

		recv_response(msg);

		/* Reduce total buffer size by just handled frame size. */
		size -= msg->len + sizeof(msg->len);

		/* Move msg pointer to the next message if any. */
		iter += msg->len + sizeof(msg->len);
		msg = (struct ste_message *)iter;
	} while (size > 0);

	return 0;
}

static gboolean simd_data_cb(GIOChannel *io, GIOCondition cond, gpointer data)
{
	char buf[SAP_BUF_SIZE];
	gsize bytes_read;
	GIOStatus gstatus;

	if (cond & (G_IO_NVAL | G_IO_HUP | G_IO_ERR)) {
		DBG("Error condition on sim socket (0x%x)", cond);
		return FALSE;
	}

	gstatus = g_io_channel_read_chars(io, buf, sizeof(buf), &bytes_read,
									NULL);
	if (gstatus != G_IO_STATUS_NORMAL) {
		sap_error("error while reading from channel (%d)", gstatus);
		return TRUE;
	}

	if (recv_message(buf, bytes_read) < 0)
		sap_error("error while parsing STE Sim message");

	return TRUE;
}

static void simd_watch(int sock, void *sap_data)
{
	GIOChannel *io;

	DBG("sock %d, sap_data %p ", sock, sap_data);

	io = g_io_channel_unix_new(sock);

	g_io_channel_set_close_on_unref(io, TRUE);
	g_io_channel_set_encoding(io, NULL, NULL);
	g_io_channel_set_buffered(io, FALSE);

	u8500.io = io;
	u8500.sap_data = sap_data;
	u8500.state = STE_DISABLED;

	g_io_add_watch_full(io, G_PRIORITY_DEFAULT,
			G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL,
			simd_data_cb, NULL, NULL);
}

static int simd_connect(void *sap_data)
{
	struct sockaddr_un addr;
	int sock;
	int err;

	/* Already connected to simd */
	if (u8500.io)
		return -EALREADY;

	sock = socket(PF_UNIX, SOCK_STREAM, 0);
	if (sock < 0) {
		err = -errno;
		sap_error("creating socket failed: %s", strerror(-err));
		return err;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	memcpy(addr.sun_path, STE_SIMD_SOCK, sizeof(STE_SIMD_SOCK) - 1);

	if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		err = -errno;
		sap_error("connect to the socket failed: %s", strerror(-err));
		goto failed;
	}

	if (fcntl(sock, F_SETFL, O_NONBLOCK) > 0) {
		err = -errno;
		sap_error("setting up socket failed: %s", strerror(-err));
		goto failed;
	}

	simd_watch(sock, sap_data);

	return 0;

failed:
	close(sock);
	return err;
}

void sap_connect_req(void *sap_device, uint16_t maxmsgsize)
{
	DBG("sap_device %p maxmsgsize %u", sap_device, maxmsgsize);

	sap_info("connect request");

	if (simd_connect(sap_device) < 0) {
		sap_connect_rsp(sap_device, SAP_STATUS_CONNECTION_FAILED);
		return;
	}

	if (send_request(u8500.io, STE_START_SAP_REQ, NULL) < 0) {
		sap_connect_rsp(sap_device, SAP_STATUS_CONNECTION_FAILED);
		simd_close();
	}
}

void sap_disconnect_req(void *sap_device, uint8_t linkloss)
{
	DBG("sap_device %p linkloss %u", sap_device, linkloss);

	sap_info("disconnect request %s", linkloss ? "by link loss" : "");

	if (u8500.state == STE_DISABLED) {
		sap_disconnect_rsp(sap_device);
		simd_close();
		return;
	}

	if (linkloss) {
		simd_close();
		return;
	}

	if (send_request(u8500.io, STE_END_SAP_REQ, NULL) < 0) {
		sap_disconnect_rsp(sap_device);
		return;
	}
}

void sap_transfer_apdu_req(void *sap_device, struct sap_parameter *param)
{
	uint8_t result;

	SAP_VDBG("sap_device %p param %p", sap_device, param);

	if (u8500.state != STE_ENABLED) {
		result = get_sap_result(STE_SEND_APDU_MSG, STE_STATUS_FAILURE);
		sap_transfer_apdu_rsp(sap_device, result, NULL, 0);
		return;
	}

	if (send_request(u8500.io, STE_SEND_APDU_REQ, param) < 0)
		sap_transfer_apdu_rsp(sap_device, SAP_RESULT_ERROR_NO_DATA,
								NULL, 0);
}

void sap_transfer_atr_req(void *sap_device)
{
	uint8_t result;

	DBG("sap_device %p", sap_device);

	if (u8500.state != STE_ENABLED) {
		result = get_sap_result(STE_GET_ATR_MSG, STE_STATUS_FAILURE);
		sap_transfer_atr_rsp(sap_device, result, NULL, 0);
		return;
	}

	if (send_request(u8500.io, STE_GET_ATR_REQ, NULL) < 0)
		sap_transfer_atr_rsp(sap_device, SAP_RESULT_ERROR_NO_DATA, NULL,
									0);
}

void sap_power_sim_off_req(void *sap_device)
{
	uint8_t result;

	DBG("sap_device %p", sap_device);

	if (u8500.state != STE_ENABLED) {
		result = get_sap_result(STE_POWER_OFF_MSG, STE_STATUS_FAILURE);
		sap_power_sim_off_rsp(sap_device, result);
		return;
	}

	if (send_request(u8500.io, STE_POWER_OFF_REQ, NULL) < 0)
		sap_power_sim_off_rsp(sap_device, SAP_RESULT_ERROR_NO_REASON);
}

void sap_power_sim_on_req(void *sap_device)
{
	uint8_t result;

	DBG("sap_device %p", sap_device);

	if (u8500.state != STE_POWERED_OFF) {
		result = get_sap_result(STE_POWER_ON_MSG, STE_STATUS_FAILURE);
		sap_power_sim_on_rsp(sap_device, result);
		return;
	}

	if (send_request(u8500.io, STE_POWER_ON_REQ, NULL) < 0)
		sap_power_sim_on_rsp(sap_device, SAP_RESULT_ERROR_NO_REASON);
}

void sap_reset_sim_req(void *sap_device)
{
	uint8_t result;

	DBG("sap_device %p", sap_device);

	if (u8500.state != STE_ENABLED) {
		result = get_sap_result(STE_RESET_MSG, STE_STATUS_FAILURE);
		sap_reset_sim_rsp(sap_device, result);
		return;
	}

	if (send_request(u8500.io, STE_RESET_REQ, NULL) < 0)
		sap_reset_sim_rsp(sap_device, SAP_RESULT_ERROR_NO_REASON);
}

void sap_transfer_card_reader_status_req(void *sap_device)
{
	uint8_t result;

	DBG("sap_device %p", sap_device);

	if (u8500.state == STE_DISABLED) {
		result = get_sap_result(STE_GET_STATUS_MSG, STE_STATUS_FAILURE);
		sap_transfer_card_reader_status_rsp(sap_device, result, 0);
		return;
	}

	if (send_request(u8500.io, STE_GET_STATUS_REQ, NULL) < 0)
		sap_transfer_card_reader_status_rsp(sap_device,
						SAP_RESULT_ERROR_NO_DATA, 0);
}

void sap_set_transport_protocol_req(void *sap_device,
						struct sap_parameter *param)
{
	DBG("sap_device %p", sap_device);

	sap_transport_protocol_rsp(sap_device, SAP_RESULT_NOT_SUPPORTED);
}

int sap_init(void)
{
	u8500.state = STE_DISABLED;
	info("STE U8500 SAP driver initialized");
	return 0;
}

void sap_exit(void)
{
}
