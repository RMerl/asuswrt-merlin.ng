// SPDX-License-Identifier: LGPL-2.1-or-later
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

#define _GNU_SOURCE
#include <stdbool.h>
#include <glib.h>
#include <errno.h>
#include <string.h>

#include "lib/bluetooth.h"

#include "src/shared/util.h"
#include "src/log.h"

#include "avctp.h"
#include "avrcp-lib.h"


/* Packet types */
#define AVRCP_PACKET_TYPE_SINGLE		0x00
#define AVRCP_PACKET_TYPE_START			0x01
#define AVRCP_PACKET_TYPE_CONTINUING		0x02
#define AVRCP_PACKET_TYPE_END			0x03

#define AVRCP_CHARSET_UTF8	0x006a

#if __BYTE_ORDER == __LITTLE_ENDIAN

struct avrcp_header {
	uint8_t company_id[3];
	uint8_t pdu_id;
	uint8_t packet_type:2;
	uint8_t rsvd:6;
	uint16_t params_len;
	uint8_t params[0];
} __attribute__ ((packed));
#define AVRCP_HEADER_LENGTH 7

#elif __BYTE_ORDER == __BIG_ENDIAN

struct avrcp_header {
	uint8_t company_id[3];
	uint8_t pdu_id;
	uint8_t rsvd:6;
	uint8_t packet_type:2;
	uint16_t params_len;
	uint8_t params[0];
} __attribute__ ((packed));
#define AVRCP_HEADER_LENGTH 7

#else
#error "Unknown byte order"
#endif

struct avrcp_browsing_header {
	uint8_t pdu_id;
	uint16_t params_len;
	uint8_t params[0];
} __attribute__ ((packed));
#define AVRCP_BROWSING_HEADER_LENGTH 3

struct get_capabilities_req {
	uint8_t cap;
	uint8_t params[0];
} __attribute__ ((packed));

struct get_capabilities_rsp {
	uint8_t cap;
	uint8_t number;
	uint8_t params[0];
} __attribute__ ((packed));

struct list_attributes_rsp {
	uint8_t number;
	uint8_t params[0];
} __attribute__ ((packed));

struct list_values_req {
	uint8_t attr;
} __attribute__ ((packed));

struct list_values_rsp {
	uint8_t number;
	uint8_t params[0];
} __attribute__ ((packed));

struct get_value_req {
	uint8_t number;
	uint8_t attrs[0];
} __attribute__ ((packed));

struct attr_value {
	uint8_t attr;
	uint8_t value;
} __attribute__ ((packed));

struct value_rsp {
	uint8_t number;
	struct attr_value values[0];
} __attribute__ ((packed));

struct set_value_req {
	uint8_t number;
	struct attr_value values[0];
} __attribute__ ((packed));

struct get_attribute_text_req {
	uint8_t number;
	uint8_t attrs[0];
} __attribute__ ((packed));

struct text_value {
	uint8_t attr;
	uint16_t charset;
	uint8_t len;
	char data[0];
} __attribute__ ((packed));

struct get_attribute_text_rsp {
	uint8_t number;
	struct text_value values[0];
} __attribute__ ((packed));

struct get_value_text_req {
	uint8_t attr;
	uint8_t number;
	uint8_t values[0];
} __attribute__ ((packed));

struct get_value_text_rsp {
	uint8_t number;
	struct text_value values[0];
} __attribute__ ((packed));

struct media_item {
	uint32_t attr;
	uint16_t charset;
	uint16_t len;
	char data[0];
} __attribute__ ((packed));

struct get_element_attributes_req {
	uint64_t id;
	uint8_t number;
	uint32_t attrs[0];
} __attribute__ ((packed));

struct get_element_attributes_rsp {
	uint8_t number;
	struct media_item items[0];
} __attribute__ ((packed));

struct get_play_status_rsp {
	uint32_t duration;
	uint32_t position;
	uint8_t status;
} __attribute__ ((packed));

struct register_notification_req {
	uint8_t event;
	uint32_t interval;
} __attribute__ ((packed));

struct register_notification_rsp {
	uint8_t event;
	uint8_t data[0];
} __attribute__ ((packed));

struct set_volume_req {
	uint8_t value;
} __attribute__ ((packed));

struct set_volume_rsp {
	uint8_t value;
} __attribute__ ((packed));

struct set_addressed_req {
	uint16_t id;
} __attribute__ ((packed));

struct set_addressed_rsp {
	uint8_t status;
} __attribute__ ((packed));

struct set_browsed_req {
	uint16_t id;
} __attribute__ ((packed));

struct set_browsed_rsp {
	uint8_t status;
	uint16_t counter;
	uint32_t items;
	uint16_t charset;
	uint8_t depth;
	uint8_t data[0];
} __attribute__ ((packed));

struct get_folder_items_req {
	uint8_t scope;
	uint32_t start;
	uint32_t end;
	uint8_t number;
	uint32_t attrs[0];
} __attribute__ ((packed));

struct get_folder_items_rsp {
	uint8_t status;
	uint16_t counter;
	uint16_t number;
	uint8_t data[0];
} __attribute__ ((packed));

struct change_path_req {
	uint16_t counter;
	uint8_t direction;
	uint64_t uid;
} __attribute__ ((packed));

struct change_path_rsp {
	uint8_t status;
	uint32_t items;
} __attribute__ ((packed));

struct get_item_attributes_req {
	uint8_t scope;
	uint64_t uid;
	uint16_t counter;
	uint8_t number;
	uint32_t attrs[0];
} __attribute__ ((packed));

struct get_item_attributes_rsp {
	uint8_t status;
	uint8_t number;
	struct media_item items[0];
} __attribute__ ((packed));

struct play_item_req {
	uint8_t scope;
	uint64_t uid;
	uint16_t counter;
} __attribute__ ((packed));

struct play_item_rsp {
	uint8_t status;
} __attribute__ ((packed));

struct search_req {
	uint16_t charset;
	uint16_t len;
	char string[0];
} __attribute__ ((packed));

struct search_rsp {
	uint8_t status;
	uint16_t counter;
	uint32_t items;
} __attribute__ ((packed));

struct add_to_now_playing_req {
	uint8_t scope;
	uint64_t uid;
	uint16_t counter;
} __attribute__ ((packed));

struct add_to_now_playing_rsp {
	uint8_t status;
} __attribute__ ((packed));

struct avrcp_control_handler {
	uint8_t id;
	uint8_t code;
	uint8_t rsp;
	ssize_t (*func) (struct avrcp *session, uint8_t transaction,
			uint16_t params_len, uint8_t *params, void *user_data);
};

struct avrcp_browsing_handler {
	uint8_t id;
	ssize_t (*func) (struct avrcp *session, uint8_t transaction,
			uint16_t params_len, uint8_t *params, void *user_data);
};

struct avrcp_continuing {
	uint8_t pdu_id;
	struct iovec pdu;
};

struct avrcp {
	struct avctp *conn;
	struct avrcp_player *player;

	const struct avrcp_control_handler *control_handlers;
	void *control_data;
	unsigned int control_id;
	uint16_t control_mtu;

	struct avrcp_continuing *continuing;

	const struct avrcp_passthrough_handler *passthrough_handlers;
	void *passthrough_data;
	unsigned int passthrough_id;

	const struct avrcp_browsing_handler *browsing_handlers;
	void *browsing_data;
	unsigned int browsing_id;

	avrcp_destroy_cb_t destroy;
	void *destroy_data;
};

struct avrcp_player {
	const struct avrcp_control_ind *ind;
	const struct avrcp_control_cfm *cfm;

	void *user_data;
};

static inline uint32_t ntoh24(const uint8_t src[3])
{
	return src[0] << 16 | src[1] << 8 | src[2];
}

static inline void hton24(uint8_t dst[3], uint32_t src)
{
	dst[0] = (src & 0xff0000) >> 16;
	dst[1] = (src & 0x00ff00) >> 8;
	dst[2] = (src & 0x0000ff);
}

static void continuing_free(struct avrcp_continuing *continuing)
{
	g_free(continuing->pdu.iov_base);
	g_free(continuing);
}

void avrcp_shutdown(struct avrcp *session)
{
	if (session->conn) {
		if (session->control_id > 0)
			avctp_unregister_pdu_handler(session->conn,
							session->control_id);
		if (session->passthrough_id > 0)
			avctp_unregister_passthrough_handler(session->conn,
						session->passthrough_id);

		if (session->browsing_id > 0)
			avctp_unregister_browsing_pdu_handler(session->conn,
							session->browsing_id);

		/* clear destroy callback that would call shutdown again */
		avctp_set_destroy_cb(session->conn, NULL, NULL);
		avctp_shutdown(session->conn);
	}

	if (session->destroy)
		session->destroy(session->destroy_data);

	if (session->continuing)
		continuing_free(session->continuing);

	g_free(session->player);
	g_free(session);
}

static struct avrcp_header *parse_pdu(uint8_t *operands, size_t operand_count)
{
	struct avrcp_header *pdu;

	if (!operands || operand_count < sizeof(*pdu)) {
		error("AVRCP: packet too small (%zu bytes)", operand_count);
		return NULL;
	}

	pdu = (void *) operands;
	pdu->params_len = ntohs(pdu->params_len);

	if (operand_count != pdu->params_len + sizeof(*pdu)) {
		error("AVRCP: invalid parameter length (%u bytes)",
							pdu->params_len);
		return NULL;
	}

	return pdu;
}

static struct avrcp_browsing_header *parse_browsing_pdu(uint8_t *operands,
							size_t operand_count)
{
	struct avrcp_browsing_header *pdu;

	if (!operands || operand_count < sizeof(*pdu)) {
		error("AVRCP: packet too small (%zu bytes)", operand_count);
		return NULL;
	}

	pdu = (void *) operands;
	pdu->params_len = ntohs(pdu->params_len);

	if (operand_count != pdu->params_len + sizeof(*pdu)) {
		error("AVRCP: invalid parameter length (%u bytes)",
							pdu->params_len);
		return NULL;
	}

	return pdu;
}

static uint8_t errno2status(int err)
{
	switch (err) {
	case -ENOSYS:
		return AVRCP_STATUS_INVALID_COMMAND;
	case -EINVAL:
		return AVRCP_STATUS_INVALID_PARAM;
	case 0:
		return AVRCP_STATUS_SUCCESS;
	case -ENOTDIR:
		return AVRCP_STATUS_NOT_DIRECTORY;
	case -EBADRQC:
		return AVRCP_STATUS_INVALID_SCOPE;
	case -ERANGE:
		return AVRCP_STATUS_OUT_OF_BOUNDS;
	case -ENOENT:
		return AVRCP_STATUS_DOES_NOT_EXIST;
	default:
		return AVRCP_STATUS_INTERNAL_ERROR;
	}
}

static ssize_t handle_vendordep_pdu(struct avctp *conn, uint8_t transaction,
					uint8_t *code, uint8_t *subunit,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	struct avrcp *session = user_data;
	const struct avrcp_control_handler *handler;
	struct avrcp_header *pdu;
	uint32_t company_id;
	ssize_t ret;

	pdu = parse_pdu(operands, operand_count);
	if (!pdu) {
		pdu = (void *) operands;
		pdu->params[0] = AVRCP_STATUS_INVALID_COMMAND;
		goto reject;
	}

	company_id = ntoh24(pdu->company_id);
	if (company_id != IEEEID_BTSIG) {
		*code = AVC_CTYPE_NOT_IMPLEMENTED;
		return 0;
	}

	DBG("AVRCP PDU 0x%02X, len 0x%04X", pdu->pdu_id, pdu->params_len);

	pdu->packet_type = 0;
	pdu->rsvd = 0;

	if (!session->control_handlers)
		goto reject;

	for (handler = session->control_handlers; handler->id; handler++) {
		if (handler->id == pdu->pdu_id)
			break;
	}

	if (handler->id != pdu->pdu_id || handler->code != *code) {
		pdu->params[0] = AVRCP_STATUS_INVALID_COMMAND;
		goto reject;
	}

	if (!handler->func) {
		pdu->params[0] = AVRCP_STATUS_INVALID_PARAM;
		goto reject;
	}

	ret = handler->func(session, transaction, pdu->params_len, pdu->params,
							session->control_data);
	if (ret == 0)
		return -EAGAIN;

	if (ret < 0) {
		if (ret == -EAGAIN)
			return ret;
		pdu->params[0] = errno2status(ret);
		goto reject;
	}

	*code = handler->rsp;
	pdu->params_len = htons(ret);

	return AVRCP_HEADER_LENGTH + ret;

reject:
	pdu->params_len = htons(1);
	*code = AVC_CTYPE_REJECTED;

	return AVRCP_HEADER_LENGTH + 1;
}

static bool handle_passthrough_pdu(struct avctp *conn, uint8_t op,
						bool pressed, void *user_data)
{
	struct avrcp *session = user_data;
	const struct avrcp_passthrough_handler *handler;

	if (!session->passthrough_handlers)
		return false;

	for (handler = session->passthrough_handlers; handler->func;
								handler++) {
		if (handler->op == op)
			break;
	}

	if (handler->func == NULL)
		return false;

	return handler->func(session, pressed, session->passthrough_data);
}

static void disconnect_cb(void *data)
{
	struct avrcp *session = data;

	session->conn = NULL;

	avrcp_shutdown(session);
}

struct avrcp *avrcp_new(int fd, size_t imtu, size_t omtu, uint16_t version)
{
	struct avrcp *session;

	session = g_new0(struct avrcp, 1);

	session->conn = avctp_new(fd, imtu, omtu, version);
	if (!session->conn) {
		g_free(session);
		return NULL;
	}

	session->passthrough_id = avctp_register_passthrough_handler(
							session->conn,
							handle_passthrough_pdu,
							session);
	session->control_id = avctp_register_pdu_handler(session->conn,
							AVC_OP_VENDORDEP,
							handle_vendordep_pdu,
							session);
	session->control_mtu = omtu - AVC_DATA_OFFSET;

	/*
	 * 27.1.2 AV/C Command Frame
	 * An AV/C command frame contains up to 512 octets of data
	 */
	if (session->control_mtu > AVC_DATA_MTU)
		session->control_mtu = AVC_DATA_MTU;

	avctp_set_destroy_cb(session->conn, disconnect_cb, session);

	return session;
}

static ssize_t handle_browsing_pdu(struct avctp *conn,
					uint8_t transaction, uint8_t *operands,
					size_t operand_count, void *user_data)
{
	struct avrcp *session = user_data;
	const struct avrcp_browsing_handler *handler;
	struct avrcp_browsing_header *pdu;
	int ret;

	pdu = parse_browsing_pdu(operands, operand_count);
	if (!pdu) {
		pdu = (void *) operands;
		pdu->params[0] = AVRCP_STATUS_INVALID_COMMAND;
		goto reject;
	}

	DBG("AVRCP Browsing PDU 0x%02X, len 0x%04X", pdu->pdu_id,
							pdu->params_len);

	if (!session->browsing_handlers) {
		pdu->pdu_id = AVRCP_GENERAL_REJECT;
		pdu->params[0] = AVRCP_STATUS_INTERNAL_ERROR;
		goto reject;
	}

	for (handler = session->browsing_handlers; handler->id; handler++) {
		if (handler->id == pdu->pdu_id)
			break;
	}

	if (handler->id != pdu->pdu_id) {
		pdu->pdu_id = AVRCP_GENERAL_REJECT;
		pdu->params[0] = AVRCP_STATUS_INVALID_COMMAND;
		goto reject;
	}

	if (!handler->func) {
		pdu->params[0] = AVRCP_STATUS_INVALID_PARAM;
		goto reject;
	}

	ret = handler->func(session, transaction, pdu->params_len, pdu->params,
							session->control_data);
	if (ret == 0)
		return -EAGAIN;

	if (ret < 0) {
		if (ret == -EAGAIN)
			return ret;
		pdu->params[0] = errno2status(ret);
		goto reject;
	}

	pdu->params_len = htons(ret);

	return AVRCP_BROWSING_HEADER_LENGTH + ret;

reject:
	pdu->params_len = htons(1);

	return AVRCP_BROWSING_HEADER_LENGTH + 1;
}

static void browsing_disconnect_cb(void *data)
{
	struct avrcp *session = data;

	session->browsing_id = 0;
}

int avrcp_connect_browsing(struct avrcp *session, int fd, size_t imtu,
								size_t omtu)
{
	int err;

	err = avctp_connect_browsing(session->conn, fd, imtu, omtu);
	if (err < 0)
		return err;

	session->browsing_id = avctp_register_browsing_pdu_handler(
							session->conn,
							handle_browsing_pdu,
							session,
							browsing_disconnect_cb);

	return 0;
}

void avrcp_set_destroy_cb(struct avrcp *session, avrcp_destroy_cb_t cb,
							void *user_data)
{
	session->destroy = cb;
	session->destroy_data = user_data;
}

static ssize_t get_capabilities(struct avrcp *session, uint8_t transaction,
				uint16_t params_len, uint8_t *params,
				void *user_data)
{
	struct avrcp_player *player = user_data;
	struct get_capabilities_req *req;

	if (!params || params_len != sizeof(*req))
		return -EINVAL;

	req = (void *) params;

	switch (req->cap) {
	case CAP_COMPANY_ID:
		req->params[0] = 1;
		hton24(&req->params[1], IEEEID_BTSIG);
		return 5;
	case CAP_EVENTS_SUPPORTED:
		if (!player->ind || !player->ind->get_capabilities)
			return -ENOSYS;
		return player->ind->get_capabilities(session, transaction,
							player->user_data);
	}

	return -EINVAL;
}

static ssize_t list_attributes(struct avrcp *session, uint8_t transaction,
				uint16_t params_len, uint8_t *params,
				void *user_data)
{
	struct avrcp_player *player = user_data;

	DBG("");

	if (!player->ind || !player->ind->list_attributes)
		return -ENOSYS;

	return player->ind->list_attributes(session, transaction,
							player->user_data);
}

static bool check_attributes(uint8_t number, const uint8_t *attrs)
{
	int i;

	for (i = 0; i < number; i++) {
		if (attrs[i] > AVRCP_ATTRIBUTE_LAST ||
					attrs[i] == AVRCP_ATTRIBUTE_ILEGAL)
			return false;
	}

	return true;
}

static ssize_t get_attribute_text(struct avrcp *session, uint8_t transaction,
					uint16_t params_len, uint8_t *params,
					void *user_data)
{
	struct avrcp_player *player = user_data;
	struct get_attribute_text_req *req;

	DBG("");

	if (!player->ind || !player->ind->get_attribute_text)
		return -ENOSYS;

	if (!params || params_len < sizeof(*req))
		return -EINVAL;

	req = (void *) params;
	if (params_len != sizeof(*req) + req->number)
		return -EINVAL;

	if (!check_attributes(req->number, req->attrs))
		return -EINVAL;

	return player->ind->get_attribute_text(session, transaction,
						req->number, req->attrs,
						player->user_data);
}

static ssize_t list_values(struct avrcp *session, uint8_t transaction,
					uint16_t params_len, uint8_t *params,
					void *user_data)
{
	struct avrcp_player *player = user_data;
	struct list_values_req *req;

	DBG("");

	if (!params || params_len != sizeof(*req))
		return -EINVAL;

	req = (void *) params;
	if (req->attr > AVRCP_ATTRIBUTE_LAST ||
					req->attr == AVRCP_ATTRIBUTE_ILEGAL)
		return -EINVAL;

	if (!player->ind || !player->ind->list_values)
		return -ENOSYS;

	return player->ind->list_values(session, transaction, req->attr,
							player->user_data);
}

static bool check_value(uint8_t attr, uint8_t number, const uint8_t *values)
{
	int i;

	for (i = 0; i < number; i++) {
		/* Check for invalid value */
		switch (attr) {
		case AVRCP_ATTRIBUTE_EQUALIZER:
			if (values[i] < AVRCP_EQUALIZER_OFF ||
						values[i] > AVRCP_EQUALIZER_ON)
				return false;
			break;
		case AVRCP_ATTRIBUTE_REPEAT_MODE:
			if (values[i] < AVRCP_REPEAT_MODE_OFF ||
					values[i] > AVRCP_REPEAT_MODE_GROUP)
				return false;
			break;
		case AVRCP_ATTRIBUTE_SHUFFLE:
			if (values[i] < AVRCP_SHUFFLE_OFF ||
					values[i] > AVRCP_SHUFFLE_GROUP)
				return false;
			break;
		case AVRCP_ATTRIBUTE_SCAN:
			if (values[i] < AVRCP_SCAN_OFF ||
					values[i] > AVRCP_SCAN_GROUP)
				return false;
			break;
		}
	}

	return true;
}

static ssize_t get_value_text(struct avrcp *session, uint8_t transaction,
					uint16_t params_len, uint8_t *params,
					void *user_data)
{
	struct avrcp_player *player = user_data;
	struct get_value_text_req *req;

	DBG("");

	if (!player->ind || !player->ind->get_value_text)
		return -ENOSYS;

	if (!params || params_len < sizeof(*req))
		return -EINVAL;

	req = (void *) params;
	if (params_len != sizeof(*req) + req->number)
		return -EINVAL;

	if (req->number > AVRCP_ATTRIBUTE_LAST ||
					req->number == AVRCP_ATTRIBUTE_ILEGAL)
		return -EINVAL;

	if (!check_value(req->attr, req->number, req->values))
		return -EINVAL;

	return player->ind->get_value_text(session, transaction, params[0],
						params[1], &params[2],
						player->user_data);
}

static ssize_t get_value(struct avrcp *session, uint8_t transaction,
					uint16_t params_len, uint8_t *params,
					void *user_data)
{
	struct avrcp_player *player = user_data;
	struct get_value_req *req;

	DBG("");

	if (!player->ind || !player->ind->get_value)
		return -ENOSYS;

	if (!params || params_len < sizeof(*req))
		return -EINVAL;

	req = (void *) params;
	if (params_len < sizeof(*req) + req->number)
		return -EINVAL;

	if (!check_attributes(req->number, req->attrs))
		return -EINVAL;

	return player->ind->get_value(session, transaction, params[0],
					&params[1], player->user_data);
}

static ssize_t set_value(struct avrcp *session, uint8_t transaction,
					uint16_t params_len, uint8_t *params,
					void *user_data)
{
	struct avrcp_player *player = user_data;
	struct set_value_req *req;
	uint8_t attrs[AVRCP_ATTRIBUTE_LAST];
	uint8_t values[AVRCP_ATTRIBUTE_LAST];
	int i;

	DBG("");

	if (!player->ind || !player->ind->set_value)
		return -ENOSYS;

	if (!params || params_len < sizeof(*req))
		return -EINVAL;

	req = (void *) params;
	if (params_len < sizeof(*req) + req->number * sizeof(*req->values))
		return -EINVAL;

	for (i = 0; i < req->number; i++) {
		attrs[i] = req->values[i].attr;
		values[i] = req->values[i].value;

		if (!check_value(attrs[i], 1, &values[i]))
			return -EINVAL;
	}

	return player->ind->set_value(session, transaction, req->number,
					attrs, values, player->user_data);
}

static ssize_t get_play_status(struct avrcp *session, uint8_t transaction,
					uint16_t params_len, uint8_t *params,
					void *user_data)
{
	struct avrcp_player *player = user_data;

	DBG("");

	if (!player->ind || !player->ind->get_play_status)
		return -ENOSYS;

	return player->ind->get_play_status(session, transaction,
							player->user_data);
}

static bool parse_attributes(struct get_element_attributes_req *req,
					uint16_t params_len, uint8_t number,
					uint32_t *attrs)
{
	int i;

	for (i = 0; i < number && params_len >= sizeof(*attrs); i++,
					params_len -= sizeof(*attrs)) {
		attrs[i] = be32_to_cpu(req->attrs[i]);

		if (attrs[i] == AVRCP_MEDIA_ATTRIBUTE_ILLEGAL ||
				attrs[i] > AVRCP_MEDIA_ATTRIBUTE_LAST)
			return false;
	}

	return true;
}

static ssize_t get_element_attributes(struct avrcp *session,
						uint8_t transaction,
						uint16_t params_len,
						uint8_t *params,
						void *user_data)
{
	struct avrcp_player *player = user_data;
	struct get_element_attributes_req *req;
	uint64_t uid;
	uint32_t attrs[AVRCP_MEDIA_ATTRIBUTE_LAST];

	DBG("");

	if (!player->ind || !player->ind->get_element_attributes)
		return -ENOSYS;

	req = (void *) params;
	if (!params || params_len < sizeof(*req))
		return -EINVAL;

	if (!parse_attributes(req, params_len - sizeof(*req),
							req->number, attrs))
		return -EINVAL;

	uid = get_be64(params);

	return player->ind->get_element_attributes(session, transaction, uid,
							req->number, attrs,
							player->user_data);
}

static ssize_t register_notification(struct avrcp *session, uint8_t transaction,
					uint16_t params_len, uint8_t *params,
					void *user_data)
{
	struct avrcp_player *player = user_data;
	struct register_notification_req *req;
	uint32_t interval;

	DBG("");

	if (!player->ind || !player->ind->register_notification)
		return -ENOSYS;

	if (!params || params_len != sizeof(*req))
		return -EINVAL;

	req = (void *) params;

	interval = be32_to_cpu(req->interval);

	return player->ind->register_notification(session, transaction,
							req->event, interval,
							player->user_data);
}

static ssize_t set_volume(struct avrcp *session, uint8_t transaction,
					uint16_t params_len, uint8_t *params,
					void *user_data)
{
	struct avrcp_player *player = user_data;
	struct set_volume_req *req;
	uint8_t volume;

	DBG("");

	if (!player->ind || !player->ind->set_volume)
		return -ENOSYS;

	if (!params || params_len != sizeof(volume))
		return -EINVAL;

	req = (void *) params;

	volume = req->value & 0x7f;

	return player->ind->set_volume(session, transaction, volume,
							player->user_data);
}

static ssize_t set_addressed(struct avrcp *session, uint8_t transaction,
					uint16_t params_len, uint8_t *params,
					void *user_data)
{
	struct avrcp_player *player = user_data;
	struct set_addressed_req *req;
	uint16_t id;

	DBG("");

	if (!player->ind || !player->ind->set_addressed)
		return -ENOSYS;

	if (!params || params_len != sizeof(*req))
		return -EINVAL;

	req = (void *) params;

	id = be16_to_cpu(req->id);

	return player->ind->set_addressed(session, transaction, id,
							player->user_data);
}

static void continuing_new(struct avrcp *session, uint8_t pdu_id,
					const struct iovec *iov, int iov_cnt,
					size_t offset)
{
	struct avrcp_continuing *continuing;
	int i;
	size_t len = 0;

	continuing = g_new0(struct avrcp_continuing, 1);
	continuing->pdu_id = pdu_id;

	for (i = 0; i < iov_cnt; i++) {
		if (i == 0 && offset) {
			len += iov[i].iov_len - offset;
			continue;
		}

		len += iov[i].iov_len;
	}

	continuing->pdu.iov_base = g_malloc0(len);

	DBG("len %zu", len);

	for (i = 0; i < iov_cnt; i++) {
		if (i == 0 && offset) {
			memcpy(continuing->pdu.iov_base,
						iov[i].iov_base + offset,
						iov[i].iov_len - offset);
			continuing->pdu.iov_len += iov[i].iov_len - offset;
			continue;
		}

		memcpy(continuing->pdu.iov_base + continuing->pdu.iov_len,
					iov[i].iov_base, iov[i].iov_len);
		continuing->pdu.iov_len += iov[i].iov_len;
	}

	session->continuing = continuing;
}

static int avrcp_send_internal(struct avrcp *session, uint8_t transaction,
					uint8_t code, uint8_t subunit,
					uint8_t pdu_id, uint8_t type,
					const struct iovec *iov, int iov_cnt)
{
	struct iovec pdu[iov_cnt + 1];
	struct avrcp_header hdr;
	int i;

	/*
	 * If a receiver receives a start fragment or non-fragmented AVRCP
	 * Specific AV/C message when it already has an incomplete fragment
	 * from that sender then the receiver shall consider the first PDU
	 * aborted.
	 */
	if (session->continuing) {
		continuing_free(session->continuing);
		session->continuing = NULL;
	}

	memset(&hdr, 0, sizeof(hdr));

	pdu[0].iov_base = &hdr;
	pdu[0].iov_len = sizeof(hdr);

	hdr.packet_type = type;

	for (i = 0; i < iov_cnt; i++) {
		pdu[i + 1].iov_base = iov[i].iov_base;

		if (pdu[0].iov_len + hdr.params_len + iov[i].iov_len <=
							session->control_mtu) {
			pdu[i + 1].iov_len = iov[i].iov_len;
			hdr.params_len += iov[i].iov_len;
			if (hdr.packet_type != AVRCP_PACKET_TYPE_SINGLE)
				hdr.packet_type = AVRCP_PACKET_TYPE_END;
			continue;
		}

		/*
		 * Only send what can fit and store the remaining in the
		 * continuing iovec
		 */
		pdu[i + 1].iov_len = session->control_mtu -
					(pdu[0].iov_len + hdr.params_len);
		hdr.params_len += pdu[i + 1].iov_len;

		continuing_new(session, pdu_id, &iov[i], iov_cnt - i,
							pdu[i + 1].iov_len);

		hdr.packet_type = hdr.packet_type != AVRCP_PACKET_TYPE_SINGLE ?
						AVRCP_PACKET_TYPE_CONTINUING :
						AVRCP_PACKET_TYPE_START;
		break;
	}

	hton24(hdr.company_id, IEEEID_BTSIG);
	hdr.pdu_id = pdu_id;
	hdr.params_len = htons(hdr.params_len);

	return avctp_send_vendor(session->conn, transaction, code, subunit,
							pdu, iov_cnt + 1);
}

static ssize_t request_continuing(struct avrcp *session, uint8_t transaction,
					uint16_t params_len, uint8_t *params,
					void *user_data)
{
	struct iovec iov;
	int err;

	DBG("");

	if (!params || params_len != 1 || !session->continuing ||
				session->continuing->pdu_id != params[0])
		return -EINVAL;

	iov.iov_base = session->continuing->pdu.iov_base;
	iov.iov_len = session->continuing->pdu.iov_len;

	DBG("len %zu", iov.iov_len);

	session->continuing->pdu.iov_base = NULL;

	err = avrcp_send_internal(session, transaction, AVC_CTYPE_STABLE,
					AVC_SUBUNIT_PANEL, params[0],
					AVRCP_PACKET_TYPE_CONTINUING, &iov, 1);

	g_free(iov.iov_base);

	if (err < 0)
		return -EINVAL;

	return 0;
}

static ssize_t abort_continuing(struct avrcp *session, uint8_t transaction,
					uint16_t params_len, uint8_t *params,
					void *user_data)
{
	DBG("");

	if (!params || params_len != 1 || !session->continuing)
		return -EINVAL;

	continuing_free(session->continuing);
	session->continuing = NULL;

	avrcp_send_internal(session, transaction, AVC_CTYPE_ACCEPTED,
				AVC_SUBUNIT_PANEL, AVRCP_ABORT_CONTINUING,
				AVRCP_PACKET_TYPE_SINGLE, NULL, 0);

	return 0;
}

static const struct avrcp_control_handler player_handlers[] = {
		{ AVRCP_GET_CAPABILITIES,
					AVC_CTYPE_STATUS, AVC_CTYPE_STABLE,
					get_capabilities },
		{ AVRCP_LIST_PLAYER_ATTRIBUTES,
					AVC_CTYPE_STATUS, AVC_CTYPE_STABLE,
					list_attributes },
		{ AVRCP_GET_PLAYER_ATTRIBUTE_TEXT,
					AVC_CTYPE_STATUS, AVC_CTYPE_STABLE,
					get_attribute_text },
		{ AVRCP_LIST_PLAYER_VALUES,
					AVC_CTYPE_STATUS, AVC_CTYPE_STABLE,
					list_values },
		{ AVRCP_GET_PLAYER_VALUE_TEXT,
					AVC_CTYPE_STATUS, AVC_CTYPE_STABLE,
					get_value_text },
		{ AVRCP_GET_CURRENT_PLAYER_VALUE,
					AVC_CTYPE_STATUS, AVC_CTYPE_STABLE,
					get_value },
		{ AVRCP_SET_PLAYER_VALUE,
					AVC_CTYPE_CONTROL, AVC_CTYPE_STABLE,
					set_value },
		{ AVRCP_GET_PLAY_STATUS,
					AVC_CTYPE_STATUS, AVC_CTYPE_STABLE,
					get_play_status },
		{ AVRCP_GET_ELEMENT_ATTRIBUTES,
					AVC_CTYPE_STATUS, AVC_CTYPE_STABLE,
					get_element_attributes },
		{ AVRCP_REGISTER_NOTIFICATION,
					AVC_CTYPE_NOTIFY, AVC_CTYPE_INTERIM,
					register_notification },
		{ AVRCP_SET_ABSOLUTE_VOLUME,
					AVC_CTYPE_CONTROL, AVC_CTYPE_STABLE,
					set_volume },
		{ AVRCP_SET_ADDRESSED_PLAYER,
					AVC_CTYPE_CONTROL, AVC_CTYPE_STABLE,
					set_addressed },
		{ AVRCP_REQUEST_CONTINUING,
					AVC_CTYPE_CONTROL, AVC_CTYPE_STABLE,
					request_continuing },
		{ AVRCP_ABORT_CONTINUING,
					AVC_CTYPE_CONTROL, AVC_CTYPE_ACCEPTED,
					abort_continuing },
		{ },
};

static void avrcp_set_control_handlers(struct avrcp *session,
				const struct avrcp_control_handler *handlers,
				void *user_data)
{
	session->control_handlers = handlers;
	session->control_data = user_data;
}

static ssize_t set_browsed(struct avrcp *session, uint8_t transaction,
					uint16_t params_len, uint8_t *params,
					void *user_data)
{
	struct avrcp_player *player = user_data;
	struct set_browsed_req *req;
	uint16_t id;

	DBG("");

	if (!player->ind || !player->ind->set_browsed)
		return -ENOSYS;

	if (!params || params_len != sizeof(*req))
		return -EINVAL;

	req = (void *) params;

	id = be16_to_cpu(req->id);

	return player->ind->set_browsed(session, transaction, id,
							player->user_data);
}

static ssize_t get_folder_items(struct avrcp *session, uint8_t transaction,
					uint16_t params_len, uint8_t *params,
					void *user_data)
{
	struct avrcp_player *player = user_data;
	struct get_folder_items_req *req;
	uint32_t start, end;
	uint16_t number;
	uint32_t attrs[AVRCP_MEDIA_ATTRIBUTE_LAST];
	int i;

	DBG("");

	if (!player->ind || !player->ind->get_folder_items)
		return -ENOSYS;

	if (!params || params_len < sizeof(*req))
		return -EINVAL;

	req = (void *) params;

	if (req->scope > AVRCP_MEDIA_NOW_PLAYING)
		return -EBADRQC;

	start = be32_to_cpu(req->start);
	end = be32_to_cpu(req->end);

	if (start > end)
		return -ERANGE;

	number = be16_to_cpu(req->number);

	for (i = 0; i < number; i++) {
		attrs[i] = be32_to_cpu(req->attrs[i]);

		if (attrs[i] == AVRCP_MEDIA_ATTRIBUTE_ILLEGAL ||
				attrs[i] > AVRCP_MEDIA_ATTRIBUTE_LAST)
			return -EINVAL;
	}

	return player->ind->get_folder_items(session, transaction, req->scope,
						start, end, number, attrs,
						player->user_data);
}

static ssize_t change_path(struct avrcp *session, uint8_t transaction,
					uint16_t params_len, uint8_t *params,
					void *user_data)
{
	struct avrcp_player *player = user_data;
	struct change_path_req *req;
	uint16_t counter;
	uint64_t uid;

	DBG("");

	if (!player->ind || !player->ind->change_path)
		return -ENOSYS;

	if (!params || params_len < sizeof(*req))
		return -EINVAL;

	req = (void *) params;

	counter = be16_to_cpu(req->counter);
	uid = be64_to_cpu(req->uid);

	return player->ind->change_path(session, transaction, counter,
					req->direction, uid, player->user_data);
}

static ssize_t get_item_attributes(struct avrcp *session, uint8_t transaction,
					uint16_t params_len, uint8_t *params,
					void *user_data)
{
	struct avrcp_player *player = user_data;
	struct get_item_attributes_req *req;
	uint64_t uid;
	uint16_t counter;
	uint32_t attrs[AVRCP_MEDIA_ATTRIBUTE_LAST];
	int i;

	DBG("");

	if (!player->ind || !player->ind->get_item_attributes)
		return -ENOSYS;

	if (!params || params_len < sizeof(*req))
		return -EINVAL;

	req = (void *) params;

	if (req->scope > AVRCP_MEDIA_NOW_PLAYING)
		return -EBADRQC;

	uid = be64_to_cpu(req->uid);
	counter = be16_to_cpu(req->counter);

	for (i = 0; i < req->number; i++) {
		attrs[i] = be32_to_cpu(req->attrs[i]);

		if (attrs[i] == AVRCP_MEDIA_ATTRIBUTE_ILLEGAL ||
				attrs[i] > AVRCP_MEDIA_ATTRIBUTE_LAST)
			return -EINVAL;
	}

	return player->ind->get_item_attributes(session, transaction,
						req->scope, uid, counter,
						req->number, attrs,
						player->user_data);
}

static ssize_t play_item(struct avrcp *session, uint8_t transaction,
					uint16_t params_len, uint8_t *params,
					void *user_data)
{
	struct avrcp_player *player = user_data;
	struct play_item_req *req;
	uint64_t uid;
	uint16_t counter;

	DBG("");

	if (!player->ind || !player->ind->play_item)
		return -ENOSYS;

	if (!params || params_len < sizeof(*req))
		return -EINVAL;

	req = (void *) params;

	if (req->scope > AVRCP_MEDIA_NOW_PLAYING)
		return -EBADRQC;

	uid = be64_to_cpu(req->uid);
	counter = be16_to_cpu(req->counter);

	return player->ind->play_item(session, transaction, req->scope, uid,
						counter, player->user_data);
}

static ssize_t search(struct avrcp *session, uint8_t transaction,
					uint16_t params_len, uint8_t *params,
					void *user_data)
{
	struct avrcp_player *player = user_data;
	struct search_req *req;
	char *string;
	uint16_t len;
	int ret;

	DBG("");

	if (!player->ind || !player->ind->search)
		return -ENOSYS;

	if (!params || params_len < sizeof(*req))
		return -EINVAL;

	req = (void *) params;

	len = be16_to_cpu(req->len);
	if (!len)
		return -EINVAL;

	string = strndup(req->string, len);

	ret = player->ind->search(session, transaction, string,
							player->user_data);

	free(string);

	return ret;
}

static ssize_t add_to_now_playing(struct avrcp *session, uint8_t transaction,
					uint16_t params_len, uint8_t *params,
					void *user_data)
{
	struct avrcp_player *player = user_data;
	struct add_to_now_playing_req *req;
	uint64_t uid;
	uint16_t counter;

	DBG("");

	if (!player->ind || !player->ind->add_to_now_playing)
		return -ENOSYS;

	if (!params || params_len < sizeof(*req))
		return -EINVAL;

	req = (void *) params;

	if (req->scope > AVRCP_MEDIA_NOW_PLAYING)
		return -EBADRQC;

	uid = be64_to_cpu(req->uid);
	counter = be16_to_cpu(req->counter);

	return player->ind->add_to_now_playing(session, transaction, req->scope,
							uid, counter,
							player->user_data);
}

static const struct avrcp_browsing_handler browsing_handlers[] = {
		{ AVRCP_SET_BROWSED_PLAYER, set_browsed },
		{ AVRCP_GET_FOLDER_ITEMS, get_folder_items },
		{ AVRCP_CHANGE_PATH, change_path },
		{ AVRCP_GET_ITEM_ATTRIBUTES, get_item_attributes },
		{ AVRCP_PLAY_ITEM, play_item },
		{ AVRCP_SEARCH, search },
		{ AVRCP_ADD_TO_NOW_PLAYING, add_to_now_playing },
		{ },
};

static void avrcp_set_browsing_handlers(struct avrcp *session,
				const struct avrcp_browsing_handler *handlers,
				void *user_data)
{
	session->browsing_handlers = handlers;
	session->browsing_data = user_data;
}

void avrcp_register_player(struct avrcp *session,
				const struct avrcp_control_ind *ind,
				const struct avrcp_control_cfm *cfm,
				void *user_data)
{
	struct avrcp_player *player;

	player = g_new0(struct avrcp_player, 1);
	player->ind = ind;
	player->cfm = cfm;
	player->user_data = user_data;

	avrcp_set_control_handlers(session, player_handlers, player);
	avrcp_set_browsing_handlers(session, browsing_handlers, player);
	session->player = player;
}

void avrcp_set_passthrough_handlers(struct avrcp *session,
			const struct avrcp_passthrough_handler *handlers,
			void *user_data)
{
	session->passthrough_handlers = handlers;
	session->passthrough_data = user_data;
}

int avrcp_init_uinput(struct avrcp *session, const char *name,
							const char *address)
{
	return avctp_init_uinput(session->conn, name, address);
}

int avrcp_send(struct avrcp *session, uint8_t transaction, uint8_t code,
					uint8_t subunit, uint8_t pdu_id,
					const struct iovec *iov, int iov_cnt)
{
	return avrcp_send_internal(session, transaction, code, subunit, pdu_id,
					AVRCP_PACKET_TYPE_SINGLE, iov, iov_cnt);
}

static int status2errno(uint8_t status)
{
	switch (status) {
	case AVRCP_STATUS_INVALID_COMMAND:
		return -ENOSYS;
	case AVRCP_STATUS_INVALID_PARAM:
		return -EINVAL;
	case AVRCP_STATUS_SUCCESS:
		return 0;
	case AVRCP_STATUS_NOT_DIRECTORY:
		return -ENOTDIR;
	case AVRCP_STATUS_INVALID_SCOPE:
		return -EBADRQC;
	case AVRCP_STATUS_OUT_OF_BOUNDS:
		return -ERANGE;
	case AVRCP_STATUS_INTERNAL_ERROR:
	case AVRCP_STATUS_INVALID_PLAYER_ID:
	case AVRCP_STATUS_PLAYER_NOT_BROWSABLE:
	case AVRCP_STATUS_NO_AVAILABLE_PLAYERS:
	case AVRCP_STATUS_ADDRESSED_PLAYER_CHANGED:
		return -EPERM;
	default:
		return -EPROTO;
	}
}

static int parse_status(struct avrcp_header *pdu)
{
	if (pdu->params_len < 1)
		return -EPROTO;

	return status2errno(pdu->params[0]);
}

static int parse_browsing_status(struct avrcp_browsing_header *pdu)
{
	if (pdu->params_len < 1)
		return -EPROTO;

	return status2errno(pdu->params[0]);
}

static int avrcp_send_req(struct avrcp *session, uint8_t code, uint8_t subunit,
					uint8_t pdu_id, const struct iovec *iov,
					int iov_cnt, avctp_rsp_cb func,
					void *user_data)
{
	struct iovec pdu[iov_cnt + 1];
	struct avrcp_header hdr;
	int i;

	memset(&hdr, 0, sizeof(hdr));

	pdu[0].iov_base = &hdr;
	pdu[0].iov_len = sizeof(hdr);

	for (i = 0; i < iov_cnt; i++) {
		pdu[i + 1].iov_base = iov[i].iov_base;
		pdu[i + 1].iov_len = iov[i].iov_len;
		hdr.params_len += iov[i].iov_len;
	}

	hton24(hdr.company_id, IEEEID_BTSIG);
	hdr.pdu_id = pdu_id;
	hdr.packet_type = AVRCP_PACKET_TYPE_SINGLE;
	hdr.params_len = htons(hdr.params_len);

	return avctp_send_vendor_req(session->conn, code, subunit, pdu,
						iov_cnt + 1, func, user_data);
}

static int avrcp_send_browsing_req(struct avrcp *session, uint8_t pdu_id,
					const struct iovec *iov, int iov_cnt,
					avctp_browsing_rsp_cb func,
					void *user_data)
{
	struct iovec pdu[iov_cnt + 1];
	struct avrcp_browsing_header hdr;
	int i;

	memset(&hdr, 0, sizeof(hdr));

	for (i = 0; i < iov_cnt; i++) {
		pdu[i + 1].iov_base = iov[i].iov_base;
		pdu[i + 1].iov_len = iov[i].iov_len;
		hdr.params_len += iov[i].iov_len;
	}

	hdr.pdu_id = pdu_id;
	hdr.params_len = htons(hdr.params_len);

	pdu[0].iov_base = &hdr;
	pdu[0].iov_len = sizeof(hdr);

	return avctp_send_browsing_req(session->conn, pdu, iov_cnt + 1,
							func, user_data);
}

static int avrcp_send_browsing(struct avrcp *session, uint8_t transaction,
				uint8_t pdu_id, const struct iovec *iov,
				int iov_cnt)
{
	struct iovec pdu[iov_cnt + 1];
	struct avrcp_browsing_header hdr;
	int i;

	memset(&hdr, 0, sizeof(hdr));

	for (i = 0; i < iov_cnt; i++) {
		pdu[i + 1].iov_base = iov[i].iov_base;
		pdu[i + 1].iov_len = iov[i].iov_len;
		hdr.params_len += iov[i].iov_len;
	}

	hdr.pdu_id = pdu_id;
	hdr.params_len = htons(hdr.params_len);

	pdu[0].iov_base = &hdr;
	pdu[0].iov_len = sizeof(hdr);

	return avctp_send_browsing(session->conn, transaction, pdu,
								iov_cnt + 1);
}

static gboolean get_capabilities_rsp(struct avctp *conn,
					uint8_t code, uint8_t subunit,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->player;
	struct avrcp_header *pdu;
	struct get_capabilities_rsp *rsp;
	uint8_t number = 0;
	uint8_t *params = NULL;
	int err;

	DBG("");

	if (!player || !player->cfm || !player->cfm->get_capabilities)
		return FALSE;

	pdu = parse_pdu(operands, operand_count);
	if (!pdu) {
		err = -EPROTO;
		goto done;
	}

	if (code == AVC_CTYPE_REJECTED) {
		err = parse_status(pdu);
		goto done;
	}

	if (pdu->params_len < sizeof(*rsp)) {
		err = -EPROTO;
		goto done;
	}

	rsp = (void *) pdu->params;

	switch (rsp->cap) {
	case CAP_COMPANY_ID:
	case CAP_EVENTS_SUPPORTED:
		break;
	default:
		err = -EPROTO;
		goto done;
	}

	if (rsp->number > 0) {
		number = rsp->number;
		params = rsp->params;
	}

	err = 0;

done:
	player->cfm->get_capabilities(session, err, number, params,
							player->user_data);

	return FALSE;
}


int avrcp_get_capabilities(struct avrcp *session, uint8_t param)
{
	struct iovec iov;
	struct get_capabilities_req req;

	req.cap = param;

	iov.iov_base = &req;
	iov.iov_len = sizeof(req);

	return avrcp_send_req(session, AVC_CTYPE_STATUS, AVC_SUBUNIT_PANEL,
					AVRCP_GET_CAPABILITIES, &iov, 1,
					get_capabilities_rsp, session);
}

static gboolean register_notification_rsp(struct avctp *conn,
					uint8_t code, uint8_t subunit,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->player;
	struct avrcp_header *pdu;
	struct register_notification_rsp *rsp;
	uint8_t event = 0;
	uint16_t value16, value16_2[2];
	uint32_t value32;
	uint64_t value64;
	uint8_t *params = NULL;
	int err;

	DBG("");

	if (!player || !player->cfm || !player->cfm->register_notification)
		return FALSE;

	pdu = parse_pdu(operands, operand_count);
	if (!pdu) {
		err = -EPROTO;
		goto done;
	}

	if (code == AVC_CTYPE_REJECTED) {
		err = parse_status(pdu);
		goto done;
	}

	if (pdu->params_len < sizeof(*rsp)) {
		err = -EPROTO;
		goto done;
	}

	rsp = (void *) pdu->params;
	event = rsp->event;

	if (event > AVRCP_EVENT_LAST) {
		err = -EPROTO;
		goto done;
	}

	switch (event) {
	case AVRCP_EVENT_STATUS_CHANGED:
		if (pdu->params_len != sizeof(*rsp) + sizeof(uint8_t)) {
			err = -EPROTO;
			goto done;
		}
		params = rsp->data;
		break;
	case AVRCP_EVENT_VOLUME_CHANGED:
		if (pdu->params_len != sizeof(*rsp) + sizeof(uint8_t)) {
			err = -EPROTO;
			goto done;
		}
		params = rsp->data;
		params[0] &= 0x7f;
		break;
	case AVRCP_EVENT_TRACK_CHANGED:
		if (pdu->params_len != sizeof(*rsp) + sizeof(value64)) {
			err = -EPROTO;
			goto done;
		}
		value64 = get_be64(rsp->data);
		params = (uint8_t *) &value64;
		break;
	case AVRCP_EVENT_PLAYBACK_POS_CHANGED:
		if (pdu->params_len != sizeof(*rsp) + sizeof(value32)) {
			err = -EPROTO;
			goto done;
		}
		value32 = get_be32(rsp->data);
		params = (uint8_t *) &value32;
		break;
	case AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED:
		if (pdu->params_len < sizeof(*rsp) + sizeof(value16_2)) {
			err = -EPROTO;
			goto done;
		}
		value16_2[0] = get_be16(rsp->data);
		value16_2[1] = get_be16(rsp->data + 2);
		params = (uint8_t *) value16_2;
		break;
	case AVRCP_EVENT_SETTINGS_CHANGED:
		if (pdu->params_len < sizeof(*rsp) + sizeof(uint8_t)) {
			err = -EPROTO;
			goto done;
		}
		params = rsp->data;
		break;
	case AVRCP_EVENT_UIDS_CHANGED:
		if (pdu->params_len < sizeof(*rsp) + sizeof(value16)) {
			err = -EPROTO;
			goto done;
		}
		value16 = get_be16(rsp->data);
		params = (uint8_t *) &value16;
		break;
	}

	err = 0;

done:
	return player->cfm->register_notification(session, err, code, event,
						params, player->user_data);
}

int avrcp_register_notification(struct avrcp *session, uint8_t event,
							uint32_t interval)
{
	struct iovec iov;
	struct register_notification_req req;

	if (event > AVRCP_EVENT_LAST)
		return -EINVAL;

	req.event = event;
	req.interval = cpu_to_be32(interval);

	iov.iov_base = &req;
	iov.iov_len = sizeof(req);

	return avrcp_send_req(session, AVC_CTYPE_NOTIFY, AVC_SUBUNIT_PANEL,
				AVRCP_REGISTER_NOTIFICATION, &iov, 1,
				register_notification_rsp, session);
}

static gboolean list_attributes_rsp(struct avctp *conn,
					uint8_t code, uint8_t subunit,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->player;
	struct avrcp_header *pdu = (void *) operands;
	struct list_attributes_rsp *rsp;
	uint8_t number = 0;
	uint8_t *attrs = NULL;
	int err;

	DBG("");

	if (!player || !player->cfm || !player->cfm->list_attributes)
		return FALSE;

	pdu = parse_pdu(operands, operand_count);
	if (!pdu) {
		err = -EPROTO;
		goto done;
	}

	if (code == AVC_CTYPE_REJECTED) {
		err = parse_status(pdu);
		goto done;
	}

	rsp = (void *) pdu->params;

	if (pdu->params_len < sizeof(*rsp)) {
		err = -EPROTO;
		goto done;
	}

	number = rsp->number;
	if (number > 0)
		attrs = rsp->params;

	err = 0;

done:
	player->cfm->list_attributes(session, err, number, attrs,
							player->user_data);

	return FALSE;
}

int avrcp_list_player_attributes(struct avrcp *session)
{
	return avrcp_send_req(session, AVC_CTYPE_STATUS, AVC_SUBUNIT_PANEL,
				AVRCP_LIST_PLAYER_ATTRIBUTES, NULL, 0,
				list_attributes_rsp, session);
}

static int parse_text_rsp(struct avrcp_header *pdu, uint8_t *number,
					uint8_t *attrs, char **text)
{
	uint8_t *ptr;
	uint16_t params_len;
	int i;

	if (pdu->params_len < 1)
		return -EPROTO;

	*number = pdu->params[0];
	if (*number > AVRCP_ATTRIBUTE_LAST) {
		*number = 0;
		return -EPROTO;
	}

	params_len = pdu->params_len - 1;
	for (i = 0, ptr = &pdu->params[1]; i < *number && params_len > 0; i++) {
		struct text_value *val;

		if (params_len < sizeof(*val))
			goto fail;

		val = (void *) ptr;

		attrs[i] = val->attr;

		params_len -= sizeof(*val);
		ptr += sizeof(*val);

		if (val->len > params_len)
			goto fail;

		if (val->len > 0) {
			text[i] = g_strndup(val->data, val->len);
			params_len -= val->len;
			ptr += val->len;
		}
	}

	if (i != *number)
		goto fail;

	return 0;

fail:
	for (i -= 1; i >= 0; i--)
		g_free(text[i]);

	*number = 0;

	return -EPROTO;
}

static gboolean get_attribute_text_rsp(struct avctp *conn,
					uint8_t code, uint8_t subunit,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->player;
	struct avrcp_header *pdu;
	uint8_t number = 0;
	uint8_t attrs[AVRCP_ATTRIBUTE_LAST];
	char *text[AVRCP_ATTRIBUTE_LAST];
	int err;

	DBG("");

	if (!player || !player->cfm || !player->cfm->get_attribute_text)
		return FALSE;

	pdu = parse_pdu(operands, operand_count);
	if (!pdu) {
		err = -EPROTO;
		goto done;
	}

	if (code == AVC_CTYPE_REJECTED) {
		err = parse_status(pdu);
		goto done;
	}

	err = parse_text_rsp(pdu, &number, attrs, text);

done:
	player->cfm->get_attribute_text(session, err, number, attrs, text,
							player->user_data);

	return FALSE;
}

int avrcp_get_player_attribute_text(struct avrcp *session, uint8_t number,
								uint8_t *attrs)
{
	struct iovec iov[2];

	if (!number || number > AVRCP_ATTRIBUTE_LAST)
		return -EINVAL;

	iov[0].iov_base = &number;
	iov[0].iov_len = sizeof(number);

	iov[1].iov_base = attrs;
	iov[1].iov_len = number;

	return avrcp_send_req(session, AVC_CTYPE_STATUS, AVC_SUBUNIT_PANEL,
				AVRCP_GET_PLAYER_ATTRIBUTE_TEXT, iov, 2,
				get_attribute_text_rsp, session);
}

static gboolean list_values_rsp(struct avctp *conn,
					uint8_t code, uint8_t subunit,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->player;
	struct avrcp_header *pdu;
	struct list_values_rsp *rsp;
	uint8_t number = 0;
	uint8_t *values = NULL;
	int err;

	DBG("");

	if (!player || !player->cfm || !player->cfm->list_values)
		return FALSE;

	pdu = parse_pdu(operands, operand_count);
	if (!pdu) {
		err = -EPROTO;
		goto done;
	}

	if (code == AVC_CTYPE_REJECTED) {
		err = parse_status(pdu);
		goto done;
	}

	if (pdu->params_len < sizeof(*rsp)) {
		err = -EPROTO;
		goto done;
	}

	rsp = (void *) pdu->params;

	if (rsp->number > 0) {
		number = rsp->number;
		values = rsp->params;
	}

	err = 0;

done:
	player->cfm->list_values(session, err, number, values,
							player->user_data);

	return FALSE;
}

int avrcp_list_player_values(struct avrcp *session, uint8_t attr)
{
	struct iovec iov;

	iov.iov_base = &attr;
	iov.iov_len = sizeof(attr);

	return avrcp_send_req(session, AVC_CTYPE_STATUS, AVC_SUBUNIT_PANEL,
					AVRCP_LIST_PLAYER_VALUES, &iov, 1,
					list_values_rsp, session);
}

static gboolean get_value_text_rsp(struct avctp *conn,
					uint8_t code, uint8_t subunit,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->player;
	struct avrcp_header *pdu;
	uint8_t number = 0;
	uint8_t values[AVRCP_ATTRIBUTE_LAST];
	char *text[AVRCP_ATTRIBUTE_LAST];
	int err;

	DBG("");

	if (!player || !player->cfm || !player->cfm->get_value_text)
		return FALSE;

	pdu = parse_pdu(operands, operand_count);
	if (!pdu) {
		err = -EPROTO;
		goto done;
	}

	if (code == AVC_CTYPE_REJECTED) {
		err = parse_status(pdu);
		goto done;
	}

	err = parse_text_rsp(pdu, &number, values, text);

done:
	player->cfm->get_value_text(session, err, number, values, text,
							player->user_data);

	return FALSE;
}

int avrcp_get_player_value_text(struct avrcp *session, uint8_t attr,
					uint8_t number, uint8_t *values)
{
	struct iovec iov[2];
	struct get_value_text_req req;

	if (!number)
		return -EINVAL;

	req.attr = attr;
	req.number = number;

	iov[0].iov_base = &req;
	iov[0].iov_len = sizeof(req);

	iov[1].iov_base = values;
	iov[1].iov_len = number;

	return avrcp_send_req(session, AVC_CTYPE_STATUS, AVC_SUBUNIT_PANEL,
					AVRCP_GET_PLAYER_VALUE_TEXT, iov, 2,
					get_value_text_rsp, session);
}

static int parse_value(struct avrcp_header *pdu, uint8_t *number,
					uint8_t *attrs, uint8_t *values)
{
	int i;
	struct value_rsp *rsp;

	if (pdu->params_len < sizeof(*rsp))
		return -EPROTO;

	rsp = (void *) pdu->params;

	/*
	 * Check if PDU is big enough to hold the number of (attribute, value)
	 * tuples.
	 */
	if (rsp->number > AVRCP_ATTRIBUTE_LAST ||
			sizeof(*rsp) + rsp->number * 2 != pdu->params_len) {
		*number = 0;
		return -EPROTO;
	}

	for (i = 0; i < rsp->number; i++) {
		attrs[i] = rsp->values[i].attr;
		values[i] = rsp->values[i].value;
	}

	*number = rsp->number;

	return 0;
}

static gboolean get_value_rsp(struct avctp *conn,
					uint8_t code, uint8_t subunit,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->player;
	struct avrcp_header *pdu;
	uint8_t number = 0;
	uint8_t attrs[AVRCP_ATTRIBUTE_LAST];
	uint8_t values[AVRCP_ATTRIBUTE_LAST];
	int err;

	DBG("");

	if (!player || !player->cfm || !player->cfm->get_value)
		return FALSE;

	pdu = parse_pdu(operands, operand_count);
	if (!pdu) {
		err = -EPROTO;
		goto done;
	}

	if (code == AVC_CTYPE_REJECTED) {
		err = parse_status(pdu);
		goto done;
	}

	err = parse_value(pdu, &number, attrs, values);

done:
	player->cfm->get_value(session, err, number, attrs, values,
							player->user_data);

	return FALSE;
}

int avrcp_get_current_player_value(struct avrcp *session, uint8_t number,
							uint8_t *attrs)

{
	struct iovec iov[2];

	if (number > AVRCP_ATTRIBUTE_LAST)
		return -EINVAL;

	iov[0].iov_base = &number;
	iov[0].iov_len = sizeof(number);

	iov[1].iov_base = attrs;
	iov[1].iov_len = number;

	return avrcp_send_req(session, AVC_CTYPE_STATUS, AVC_SUBUNIT_PANEL,
				AVRCP_GET_CURRENT_PLAYER_VALUE, iov, 2,
				get_value_rsp, session);
}

static gboolean set_value_rsp(struct avctp *conn,
					uint8_t code, uint8_t subunit,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->player;
	struct avrcp_header *pdu;
	int err;

	DBG("");

	if (!player || !player->cfm || !player->cfm->set_value)
		return FALSE;

	pdu = parse_pdu(operands, operand_count);
	if (!pdu) {
		err = -EPROTO;
		goto done;
	}

	if (code == AVC_CTYPE_REJECTED) {
		err = parse_status(pdu);
		goto done;
	}

	err = 0;

done:
	player->cfm->set_value(session, err, player->user_data);

	return FALSE;
}

int avrcp_set_player_value(struct avrcp *session, uint8_t number,
					uint8_t *attrs, uint8_t *values)
{
	struct iovec iov[2];
	struct attr_value val[AVRCP_ATTRIBUTE_LAST];
	int i;

	if (number > AVRCP_ATTRIBUTE_LAST)
		return -EINVAL;

	iov[0].iov_base = &number;
	iov[0].iov_len = sizeof(number);

	for (i = 0; i < number; i++) {
		val[i].attr = attrs[i];
		val[i].value = values[i];
	}

	iov[1].iov_base = val;
	iov[1].iov_len = sizeof(*val) * number;

	return avrcp_send_req(session, AVC_CTYPE_CONTROL, AVC_SUBUNIT_PANEL,
					AVRCP_SET_PLAYER_VALUE, iov, 2,
					set_value_rsp, session);
}

static gboolean get_play_status_rsp(struct avctp *conn,
					uint8_t code, uint8_t subunit,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->player;
	struct avrcp_header *pdu;
	struct get_play_status_rsp *rsp;
	uint8_t status = 0;
	uint32_t position = 0;
	uint32_t duration = 0;
	int err;

	DBG("");

	if (!player || !player->cfm || !player->cfm->get_play_status)
		return FALSE;

	pdu = parse_pdu(operands, operand_count);
	if (!pdu) {
		err = -EPROTO;
		goto done;
	}

	if (code == AVC_CTYPE_REJECTED) {
		err = parse_status(pdu);
		goto done;
	}

	if (pdu->params_len < sizeof(*rsp)) {
		err = -EPROTO;
		goto done;
	}

	rsp = (void *) pdu->params;

	duration = be32_to_cpu(rsp->duration);
	position = be32_to_cpu(rsp->position);
	status = rsp->status;
	err = 0;

done:
	player->cfm->get_play_status(session, err, status, position, duration,
							player->user_data);

	return FALSE;
}

int avrcp_get_play_status(struct avrcp *session)
{
	return avrcp_send_req(session, AVC_CTYPE_STATUS, AVC_SUBUNIT_PANEL,
				AVRCP_GET_PLAY_STATUS, NULL, 0,
				get_play_status_rsp, session);
}

static gboolean set_volume_rsp(struct avctp *conn,
					uint8_t code, uint8_t subunit,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->player;
	struct avrcp_header *pdu;
	struct set_volume_rsp *rsp;
	uint8_t value = 0;
	int err;

	DBG("");

	if (!player || !player->cfm || !player->cfm->set_volume)
		return FALSE;

	pdu = parse_pdu(operands, operand_count);
	if (!pdu) {
		err = -EPROTO;
		goto done;
	}

	if (code == AVC_CTYPE_REJECTED) {
		err = parse_status(pdu);
		goto done;
	}

	if (pdu->params_len < sizeof(*rsp)) {
		err = -EPROTO;
		goto done;
	}

	rsp = (void *) pdu->params;

	value = rsp->value & 0x7f;
	err = 0;

done:
	player->cfm->set_volume(session, err, value, player->user_data);

	return FALSE;
}

int avrcp_set_volume(struct avrcp *session, uint8_t volume)
{
	struct iovec iov;

	iov.iov_base = &volume;
	iov.iov_len = sizeof(volume);

	return avrcp_send_req(session, AVC_CTYPE_CONTROL, AVC_SUBUNIT_PANEL,
				AVRCP_SET_ABSOLUTE_VOLUME, &iov, 1,
				set_volume_rsp, session);
}

static int parse_attribute_list(uint8_t *params, uint16_t params_len,
				uint8_t number, uint32_t *attrs, char **text)
{
	struct media_item *item;
	int i;

	if (number > AVRCP_MEDIA_ATTRIBUTE_LAST)
		return -EPROTO;

	for (i = 0; i < number && params_len >= sizeof(*item); i++) {
		item = (void *) params;

		item->attr = be32_to_cpu(item->attr);
		item->charset = be16_to_cpu(item->charset);
		item->len = be16_to_cpu(item->len);

		params_len -= sizeof(*item);
		params += sizeof(*item);
		if (item->len > params_len)
			goto fail;

		if (item->len > 0) {
			text[i] = g_strndup(item->data, item->len);
			attrs[i] = item->attr;
			params_len -= item->len;
			params += item->len;
		} else {
			text[i] = NULL;
			attrs[i] = 0;
		}
	}

	return 0;

fail:
	for (i -= 1; i >= 0; i--)
		g_free(text[i]);

	return -EPROTO;
}

static void free_attribute_list(uint8_t number, char **text)
{
	while(number--)
		g_free(text[number]);
}

static int parse_elements(struct avrcp_header *pdu, uint8_t *number,
						uint32_t *attrs, char **text)
{
	struct get_element_attributes_rsp *rsp;

	if (pdu->params_len < sizeof(*rsp))
		return -EPROTO;

	rsp = (void *) pdu->params;
	if (rsp->number > AVRCP_MEDIA_ATTRIBUTE_LAST)
		return -EPROTO;

	*number = rsp->number;

	return parse_attribute_list(pdu->params + sizeof(*rsp),
						pdu->params_len - sizeof(*rsp),
						*number, attrs, text);
}

static int parse_items(struct avrcp_browsing_header *pdu, uint8_t *number,
						uint32_t *attrs, char **text)
{
	struct get_item_attributes_rsp *rsp;

	if (pdu->params_len < sizeof(*rsp))
		return -EPROTO;

	rsp = (void *) pdu->params;

	if (rsp->number > AVRCP_MEDIA_ATTRIBUTE_LAST)
		return -EPROTO;

	*number = rsp->number;

	return parse_attribute_list(pdu->params + sizeof(*rsp),
						pdu->params_len - sizeof(*rsp),
						*number, attrs, text);
}

static gboolean get_element_attributes_rsp(struct avctp *conn,
					uint8_t code, uint8_t subunit,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->player;
	struct avrcp_header *pdu;
	uint8_t number = 0;
	uint32_t attrs[AVRCP_MEDIA_ATTRIBUTE_LAST];
	char *text[AVRCP_MEDIA_ATTRIBUTE_LAST];
	int err;

	DBG("");

	if (!player || !player->cfm || !player->cfm->get_element_attributes)
		return FALSE;

	pdu = parse_pdu(operands, operand_count);
	if (!pdu) {
		err = -EPROTO;
		goto done;
	}

	if (code == AVC_CTYPE_REJECTED) {
		err = parse_status(pdu);
		goto done;
	}

	err = parse_elements(pdu, &number, attrs, text);

done:
	player->cfm->get_element_attributes(session, err, number, attrs, text,
							player->user_data);

	if (err == 0)
		free_attribute_list(number, text);

	return FALSE;
}

int avrcp_get_element_attributes(struct avrcp *session)
{
	struct iovec iov;
	struct get_element_attributes_req req;

	/* This returns all attributes */
	memset(&req, 0, sizeof(req));

	iov.iov_base = &req;
	iov.iov_len = sizeof(req);

	return avrcp_send_req(session, AVC_CTYPE_STATUS, AVC_SUBUNIT_PANEL,
				AVRCP_GET_ELEMENT_ATTRIBUTES, &iov, 1,
				get_element_attributes_rsp, session);
}

static gboolean set_addressed_rsp(struct avctp *conn,
					uint8_t code, uint8_t subunit,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->player;
	struct avrcp_header *pdu;
	int err;

	DBG("");

	if (!player || !player->cfm || !player->cfm->set_addressed)
		return FALSE;

	pdu = parse_pdu(operands, operand_count);
	if (!pdu) {
		err = -EPROTO;
		goto done;
	}

	err = parse_status(pdu);

done:
	player->cfm->set_addressed(session, err, player->user_data);

	return FALSE;
}

int avrcp_set_addressed_player(struct avrcp *session, uint16_t player_id)
{
	struct iovec iov;
	struct set_addressed_req req;

	req.id = cpu_to_be16(player_id);

	iov.iov_base = &req;
	iov.iov_len = sizeof(req);

	return avrcp_send_req(session, AVC_CTYPE_CONTROL, AVC_SUBUNIT_PANEL,
					AVRCP_SET_ADDRESSED_PLAYER, &iov, 1,
					set_addressed_rsp, session);
}

static char *parse_folder_list(uint8_t *params, uint16_t params_len,
								uint8_t depth)
{
	char **folders, *path;
	uint8_t count;
	size_t i;

	folders = g_new0(char *, depth + 2);
	folders[0] = g_strdup("/Filesystem");

	for (i = 0, count = 1; count <= depth && i < params_len; count++) {
		uint8_t len;

		len = params[i++];

		if (i + len > params_len || len == 0) {
			g_strfreev(folders);
			return NULL;
		}

		folders[count] = g_memdup(&params[i], len);
		i += len;
	}

	path = g_build_pathv("/", folders);
	g_strfreev(folders);

	return path;
}

static gboolean set_browsed_rsp(struct avctp *conn, uint8_t *operands,
					size_t operand_count, void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->player;
	struct avrcp_browsing_header *pdu;
	struct set_browsed_rsp *rsp;
	uint16_t counter = 0;
	uint32_t items = 0;
	char *path = NULL;
	int err;

	DBG("");

	if (!player || !player->cfm || !player->cfm->set_browsed)
		return FALSE;

	pdu = parse_browsing_pdu(operands, operand_count);
	if (!pdu) {
		err = -EPROTO;
		goto done;
	}

	err = parse_browsing_status(pdu);
	if (err < 0)
		goto done;

	if (pdu->params_len < sizeof(*rsp)) {
		err = -EPROTO;
		goto done;
	}

	rsp = (void *) pdu->params;

	counter = be16_to_cpu(rsp->counter);
	items = be32_to_cpu(rsp->items);

	path = parse_folder_list(rsp->data, pdu->params_len - sizeof(*rsp),
								rsp->depth);
	if (!path)
		err = -EPROTO;

done:
	player->cfm->set_browsed(session, err, counter, items, path,
							player->user_data);

	return FALSE;
}

int avrcp_set_browsed_player(struct avrcp *session, uint16_t player_id)
{
	struct iovec iov;
	struct set_browsed_req req;

	req.id = cpu_to_be16(player_id);

	iov.iov_base = &req;
	iov.iov_len = sizeof(req);

	return avrcp_send_browsing_req(session, AVRCP_SET_BROWSED_PLAYER,
					&iov, 1, set_browsed_rsp, session);
}

static gboolean get_folder_items_rsp(struct avctp *conn,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->player;
	struct avrcp_browsing_header *pdu;
	struct get_folder_items_rsp *rsp;
	uint16_t counter = 0, number = 0;
	uint8_t *params = NULL;
	int err;

	DBG("");

	if (!player || !player->cfm || !player->cfm->get_folder_items)
		return FALSE;

	pdu = parse_browsing_pdu(operands, operand_count);
	if (!pdu) {
		err = -EPROTO;
		goto done;
	}

	err = parse_browsing_status(pdu);
	if (err < 0)
		goto done;

	if (pdu->params_len < sizeof(*rsp)) {
		err = -EPROTO;
		goto done;
	}

	rsp = (void *) pdu->params;

	counter = be16_to_cpu(rsp->counter);
	number = be16_to_cpu(rsp->number);
	params = rsp->data;

	/* FIXME: Add proper parsing for each item type */

done:
	player->cfm->get_folder_items(session, err, counter, number, params,
							player->user_data);

	return FALSE;
}

int avrcp_get_folder_items(struct avrcp *session, uint8_t scope,
				uint32_t start, uint32_t end, uint8_t number,
				uint32_t *attrs)
{

	struct iovec iov[2];
	struct get_folder_items_req req;
	int i;

	req.scope = scope;
	req.start = cpu_to_be32(start);
	req.end = cpu_to_be32(end);
	req.number = number;

	iov[0].iov_base = &req;
	iov[0].iov_len = sizeof(req);

	if (!number)
		return avrcp_send_browsing_req(session, AVRCP_GET_FOLDER_ITEMS,
						iov, 1, get_folder_items_rsp,
						session);

	for (i = 0; i < number; i++)
		attrs[i] = cpu_to_be32(attrs[i]);

	iov[1].iov_base = attrs;
	iov[1].iov_len = number * sizeof(*attrs);

	return avrcp_send_browsing_req(session, AVRCP_GET_FOLDER_ITEMS,
					iov, 2, get_folder_items_rsp, session);
}

static gboolean change_path_rsp(struct avctp *conn, uint8_t *operands,
					size_t operand_count, void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->player;
	struct avrcp_browsing_header *pdu;
	struct change_path_rsp *rsp;
	uint32_t items = 0;
	int err;

	DBG("");

	if (!player || !player->cfm || !player->cfm->change_path)
		return FALSE;

	pdu = parse_browsing_pdu(operands, operand_count);
	if (!pdu) {
		err = -EPROTO;
		goto done;
	}

	err = parse_browsing_status(pdu);
	if (err < 0)
		goto done;

	if (pdu->params_len < sizeof(*rsp)) {
		err = -EPROTO;
		goto done;
	}

	rsp = (void *) pdu->params;

	items = be32_to_cpu(rsp->items);

done:
	player->cfm->change_path(session, err, items, player->user_data);

	return FALSE;
}

int avrcp_change_path(struct avrcp *session, uint8_t direction, uint64_t uid,
							uint16_t counter)
{
	struct iovec iov;
	struct change_path_req req;

	req.counter = cpu_to_be16(counter);
	req.direction = direction;
	req.uid = cpu_to_be64(uid);

	iov.iov_base = &req;
	iov.iov_len = sizeof(req);

	return avrcp_send_browsing_req(session, AVRCP_CHANGE_PATH,
					&iov, 1, change_path_rsp, session);
}

static gboolean get_item_attributes_rsp(struct avctp *conn, uint8_t *operands,
					size_t operand_count, void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->player;
	struct avrcp_browsing_header *pdu;
	uint8_t number = 0;
	uint32_t attrs[AVRCP_MEDIA_ATTRIBUTE_LAST];
	char *text[AVRCP_MEDIA_ATTRIBUTE_LAST];
	int err;

	DBG("");

	if (!player || !player->cfm || !player->cfm->get_item_attributes)
		return FALSE;

	pdu = parse_browsing_pdu(operands, operand_count);
	if (!pdu) {
		err = -EPROTO;
		goto done;
	}

	err = parse_browsing_status(pdu);
	if (err < 0)
		goto done;

	err = parse_items(pdu, &number, attrs, text);

done:
	player->cfm->get_item_attributes(session, err, number, attrs, text,
							player->user_data);

	if (err == 0)
		free_attribute_list(number, text);

	return FALSE;
}

int avrcp_get_item_attributes(struct avrcp *session, uint8_t scope,
				uint64_t uid, uint16_t counter, uint8_t number,
				uint32_t *attrs)
{
	struct iovec iov[2];
	struct get_item_attributes_req req;
	int i;

	req.scope = scope;
	req.uid = cpu_to_be64(uid);
	req.counter = cpu_to_be16(counter);
	req.number = number;

	iov[0].iov_base = &req;
	iov[0].iov_len = sizeof(req);

	if (!number)
		return avrcp_send_browsing_req(session,
						AVRCP_GET_ITEM_ATTRIBUTES,
						iov, 1, get_item_attributes_rsp,
						session);

	if (number > AVRCP_MEDIA_ATTRIBUTE_LAST)
		return -EINVAL;

	for (i = 0; i < number; i++) {
		if (attrs[i] > AVRCP_MEDIA_ATTRIBUTE_LAST ||
				attrs[i] == AVRCP_MEDIA_ATTRIBUTE_ILLEGAL)
			return -EINVAL;
		attrs[i] = cpu_to_be32(attrs[i]);
	}

	iov[1].iov_base = attrs;
	iov[1].iov_len = number * sizeof(*attrs);

	return avrcp_send_browsing_req(session, AVRCP_GET_ITEM_ATTRIBUTES,
					iov, 2, get_item_attributes_rsp,
					session);
}

static gboolean play_item_rsp(struct avctp *conn, uint8_t *operands,
					size_t operand_count, void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->player;
	struct avrcp_browsing_header *pdu;
	int err;

	DBG("");

	if (!player || !player->cfm || !player->cfm->play_item)
		return FALSE;

	pdu = parse_browsing_pdu(operands, operand_count);
	if (!pdu) {
		err = -EPROTO;
		goto done;
	}

	err = parse_browsing_status(pdu);

done:
	player->cfm->play_item(session, err, player->user_data);

	return FALSE;
}

int avrcp_play_item(struct avrcp *session, uint8_t scope, uint64_t uid,
							uint16_t counter)
{
	struct iovec iov;
	struct play_item_req req;

	if (scope > AVRCP_MEDIA_NOW_PLAYING)
		return -EINVAL;

	req.scope = scope;
	req.uid = cpu_to_be64(uid);
	req.counter = cpu_to_be16(counter);

	iov.iov_base = &req;
	iov.iov_len = sizeof(req);

	return avrcp_send_browsing_req(session, AVRCP_PLAY_ITEM, &iov, 1,
						play_item_rsp, session);
}

static gboolean search_rsp(struct avctp *conn, uint8_t *operands,
					size_t operand_count, void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->player;
	struct avrcp_browsing_header *pdu;
	struct search_rsp *rsp;
	uint16_t counter = 0;
	uint32_t items = 0;
	int err;

	DBG("");

	if (!player || !player->cfm || !player->cfm->search)
		return FALSE;

	pdu = parse_browsing_pdu(operands, operand_count);
	if (!pdu) {
		err = -EPROTO;
		goto done;
	}

	err = parse_browsing_status(pdu);
	if (err < 0)
		goto done;

	if (pdu->params_len < sizeof(*rsp)) {
		err = -EPROTO;
		goto done;
	}

	rsp = (void *) pdu->params;

	counter = be16_to_cpu(rsp->counter);
	items = be32_to_cpu(rsp->items);

	err = 0;

done:
	player->cfm->search(session, err, counter, items, player->user_data);

	return FALSE;
}

int avrcp_search(struct avrcp *session, const char *string)
{
	struct iovec iov[2];
	struct search_req req;
	size_t len;

	if (!string)
		return -EINVAL;

	len = strnlen(string, UINT8_MAX);

	req.charset = cpu_to_be16(AVRCP_CHARSET_UTF8);
	req.len = cpu_to_be16(len);

	iov[0].iov_base = &req;
	iov[0].iov_len = sizeof(req);

	iov[1].iov_base = (void *) string;
	iov[1].iov_len = len;

	return avrcp_send_browsing_req(session, AVRCP_SEARCH, iov, 2,
							search_rsp, session);
}

static gboolean add_to_now_playing_rsp(struct avctp *conn, uint8_t *operands,
					size_t operand_count, void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->player;
	struct avrcp_browsing_header *pdu;
	int err;

	DBG("");

	if (!player || !player->cfm || !player->cfm->add_to_now_playing)
		return FALSE;

	pdu = parse_browsing_pdu(operands, operand_count);
	if (!pdu) {
		err = -EPROTO;
		goto done;
	}

	err = parse_browsing_status(pdu);

done:
	player->cfm->add_to_now_playing(session, err, player->user_data);

	return FALSE;
}

int avrcp_add_to_now_playing(struct avrcp *session, uint8_t scope, uint64_t uid,
							uint16_t counter)
{
	struct iovec iov;
	struct add_to_now_playing_req req;

	if (scope > AVRCP_MEDIA_NOW_PLAYING)
		return -EINVAL;

	req.scope = scope;
	req.uid = cpu_to_be64(uid);
	req.counter = cpu_to_be16(counter);

	iov.iov_base = &req;
	iov.iov_len = sizeof(req);

	return avrcp_send_browsing_req(session, AVRCP_ADD_TO_NOW_PLAYING,
					&iov, 1, add_to_now_playing_rsp,
					session);
}

int avrcp_get_capabilities_rsp(struct avrcp *session, uint8_t transaction,
						uint8_t number, uint8_t *events)
{
	struct iovec iov[2];
	struct get_capabilities_rsp rsp;

	if (number > AVRCP_EVENT_LAST)
		return -EINVAL;

	rsp.cap = CAP_EVENTS_SUPPORTED;
	rsp.number = number;

	iov[0].iov_base = &rsp;
	iov[0].iov_len = sizeof(rsp);

	iov[1].iov_base = events;
	iov[1].iov_len = number;

	return avrcp_send(session, transaction, AVC_CTYPE_STABLE,
				AVC_SUBUNIT_PANEL, AVRCP_GET_CAPABILITIES,
				iov, 2);
}

int avrcp_list_player_attributes_rsp(struct avrcp *session, uint8_t transaction,
					uint8_t number, uint8_t *attrs)
{
	struct iovec iov[2];
	struct list_attributes_rsp rsp;

	if (number > AVRCP_ATTRIBUTE_LAST)
		return -EINVAL;

	rsp.number = number;

	iov[0].iov_base = &rsp;
	iov[0].iov_len = sizeof(rsp);

	if (!number)
		return avrcp_send(session, transaction, AVC_CTYPE_STABLE,
				AVC_SUBUNIT_PANEL, AVRCP_LIST_PLAYER_ATTRIBUTES,
				iov, 1);

	iov[1].iov_base = attrs;
	iov[1].iov_len = number;

	return avrcp_send(session, transaction, AVC_CTYPE_STABLE,
				AVC_SUBUNIT_PANEL, AVRCP_LIST_PLAYER_ATTRIBUTES,
				iov, 2);
}

int avrcp_get_player_attribute_text_rsp(struct avrcp *session,
					uint8_t transaction, uint8_t number,
					uint8_t *attrs, const char **text)
{
	struct iovec iov[1 + AVRCP_ATTRIBUTE_LAST * 2];
	struct text_value val[AVRCP_ATTRIBUTE_LAST];
	int i;

	if (number > AVRCP_ATTRIBUTE_LAST)
		return -EINVAL;

	iov[0].iov_base = &number;
	iov[0].iov_len = sizeof(number);

	for (i = 0; i < number; i++) {
		uint8_t len = 0;

		if (attrs[i] > AVRCP_ATTRIBUTE_LAST ||
					attrs[i] == AVRCP_ATTRIBUTE_ILEGAL)
			return -EINVAL;

		if (text[i])
			len = strlen(text[i]);

		val[i].attr = attrs[i];
		val[i].charset = cpu_to_be16(AVRCP_CHARSET_UTF8);
		val[i].len = len;

		iov[i + 1].iov_base = &val[i];
		iov[i + 1].iov_len = sizeof(val[i]);

		iov[i + 2].iov_base = (void *) text[i];
		iov[i + 2].iov_len = len;
	}

	return avrcp_send(session, transaction, AVC_CTYPE_STABLE,
			AVC_SUBUNIT_PANEL, AVRCP_GET_PLAYER_ATTRIBUTE_TEXT,
			iov, 1 + i * 2);
}

int avrcp_list_player_values_rsp(struct avrcp *session, uint8_t transaction,
					uint8_t number, uint8_t *values)
{
	struct iovec iov[2];

	if (number > AVRCP_ATTRIBUTE_LAST)
		return -EINVAL;

	iov[0].iov_base = &number;
	iov[0].iov_len = sizeof(number);

	iov[1].iov_base = values;
	iov[1].iov_len = number;

	return avrcp_send(session, transaction, AVC_CTYPE_STABLE,
				AVC_SUBUNIT_PANEL, AVRCP_LIST_PLAYER_VALUES,
				iov, 2);
}

int avrcp_get_play_status_rsp(struct avrcp *session, uint8_t transaction,
				uint32_t position, uint32_t duration,
				uint8_t status)
{
	struct iovec iov;
	struct get_play_status_rsp rsp;

	rsp.duration = cpu_to_be32(duration);
	rsp.position = cpu_to_be32(position);
	rsp.status = status;

	iov.iov_base = &rsp;
	iov.iov_len = sizeof(rsp);

	return avrcp_send(session, transaction, AVC_CTYPE_STABLE,
				AVC_SUBUNIT_PANEL, AVRCP_GET_PLAY_STATUS,
				&iov, 1);
}

int avrcp_get_player_values_text_rsp(struct avrcp *session,
					uint8_t transaction, uint8_t number,
					uint8_t *values, const char **text)
{
	struct iovec iov[1 + AVRCP_ATTRIBUTE_LAST * 2];
	struct text_value val[AVRCP_ATTRIBUTE_LAST];
	int i;

	if (number > AVRCP_ATTRIBUTE_LAST)
		return -EINVAL;

	iov[0].iov_base = &number;
	iov[0].iov_len = sizeof(number);

	for (i = 0; i < number; i++) {
		uint8_t len = 0;

		if (text[i])
			len = strlen(text[i]);

		val[i].attr = values[i];
		val[i].charset = cpu_to_be16(AVRCP_CHARSET_UTF8);
		val[i].len = len;

		iov[i + 1].iov_base = &val[i];
		iov[i + 1].iov_len = sizeof(val[i]);

		iov[i + 2].iov_base = (void *) text[i];
		iov[i + 2].iov_len = len;
	}

	return avrcp_send(session, transaction, AVC_CTYPE_STABLE,
				AVC_SUBUNIT_PANEL, AVRCP_GET_PLAYER_VALUE_TEXT,
				iov, 1 + i * 2);
}

int avrcp_get_current_player_value_rsp(struct avrcp *session,
					uint8_t transaction, uint8_t number,
					uint8_t *attrs, uint8_t *values)
{
	struct iovec iov[1 + AVRCP_ATTRIBUTE_LAST];
	struct attr_value val[AVRCP_ATTRIBUTE_LAST];
	int i;

	if (number > AVRCP_ATTRIBUTE_LAST)
		return -EINVAL;

	iov[0].iov_base = &number;
	iov[0].iov_len = sizeof(number);

	for (i = 0; i < number; i++) {
		val[i].attr = attrs[i];
		val[i].value = values[i];

		iov[i + 1].iov_base = &val[i];
		iov[i + 1].iov_len = sizeof(val[i]);
	}

	return avrcp_send(session, transaction, AVC_CTYPE_STABLE,
			AVC_SUBUNIT_PANEL, AVRCP_GET_CURRENT_PLAYER_VALUE,
			iov, 1 + i);
}

int avrcp_set_player_value_rsp(struct avrcp *session, uint8_t transaction)
{
	return avrcp_send(session, transaction, AVC_CTYPE_STABLE,
			AVC_SUBUNIT_PANEL, AVRCP_SET_PLAYER_VALUE, NULL, 0);
}

int avrcp_get_element_attrs_rsp(struct avrcp *session, uint8_t transaction,
					uint8_t *params, size_t params_len)
{
	struct iovec iov;

	iov.iov_base = params;
	iov.iov_len = params_len;

	return avrcp_send(session, transaction, AVC_CTYPE_STABLE,
				AVC_SUBUNIT_PANEL, AVRCP_GET_ELEMENT_ATTRIBUTES,
				&iov, 1);
}

int avrcp_register_notification_rsp(struct avrcp *session, uint8_t transaction,
					uint8_t code, uint8_t event,
					void *data, size_t len)
{
	struct iovec iov[2];
	uint16_t *player;
	uint8_t *volume;

	if (event > AVRCP_EVENT_LAST)
		return -EINVAL;

	iov[0].iov_base = &event;
	iov[0].iov_len = sizeof(event);

	switch (event) {
	case AVRCP_EVENT_STATUS_CHANGED:
		if (len != sizeof(uint8_t))
			return -EINVAL;
		break;
	case AVRCP_EVENT_VOLUME_CHANGED:
		if (len != sizeof(uint8_t))
			return -EINVAL;
		volume = data;
		if (volume[0] > 127)
			return -EINVAL;
		break;
	case AVRCP_EVENT_TRACK_CHANGED:
		if (len != sizeof(uint64_t))
			return -EINVAL;

		put_be64(*(uint64_t *) data, data);
		break;
	case AVRCP_EVENT_PLAYBACK_POS_CHANGED:
		if (len != sizeof(uint32_t))
			return -EINVAL;

		put_be32(*(uint32_t *) data, data);
		break;
	case AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED:
		if (len != 4)
			return -EINVAL;

		player = data;
		player[0] = cpu_to_be16(player[0]);
		player[1] = cpu_to_be16(player[1]);

		break;
	case AVRCP_EVENT_SETTINGS_CHANGED:
		if (len < sizeof(uint8_t))
			return -EINVAL;
		break;
	case AVRCP_EVENT_UIDS_CHANGED:
		if (len != sizeof(uint16_t))
			return -EINVAL;

		put_be16(*(uint16_t *) data, data);
		break;
	default:
		return avrcp_send(session, transaction, code, AVC_SUBUNIT_PANEL,
					AVRCP_REGISTER_NOTIFICATION, iov, 1);
	}

	iov[1].iov_base = data;
	iov[1].iov_len = len;

	return avrcp_send(session, transaction, code, AVC_SUBUNIT_PANEL,
					AVRCP_REGISTER_NOTIFICATION, iov, 2);
}

int avrcp_set_volume_rsp(struct avrcp *session, uint8_t transaction,
							uint8_t volume)
{
	struct iovec iov;

	if (volume > 127)
		return -EINVAL;

	iov.iov_base = &volume;
	iov.iov_len = sizeof(volume);

	return avrcp_send(session, transaction, AVC_CTYPE_STABLE,
				AVC_SUBUNIT_PANEL, AVRCP_SET_ABSOLUTE_VOLUME,
				&iov, 1);
}

int avrcp_set_addressed_player_rsp(struct avrcp *session, uint8_t transaction,
							uint8_t status)
{
	struct iovec iov;

	iov.iov_base = &status;
	iov.iov_len = sizeof(status);

	return avrcp_send(session, transaction, AVC_CTYPE_STABLE,
				AVC_SUBUNIT_PANEL, AVRCP_SET_ADDRESSED_PLAYER,
				&iov, 1);
}

static int avrcp_status_rsp(struct avrcp *session, uint8_t transaction,
						uint8_t pdu_id, uint8_t status)
{
	struct iovec iov;

	if (status > AVRCP_STATUS_ADDRESSED_PLAYER_CHANGED)
		return -EINVAL;

	iov.iov_base = &status;
	iov.iov_len = sizeof(status);

	return avrcp_send_browsing(session, transaction, pdu_id, &iov, 1);
}

int avrcp_set_browsed_player_rsp(struct avrcp *session, uint8_t transaction,
					uint8_t status, uint16_t counter,
					uint32_t items, uint8_t depth,
					const char **folders)
{
	struct iovec iov[UINT8_MAX * 2 + 1];
	struct set_browsed_rsp rsp;
	uint16_t len[UINT8_MAX];
	int i;

	if (status != AVRCP_STATUS_SUCCESS)
		return avrcp_status_rsp(session, transaction,
					AVRCP_SET_BROWSED_PLAYER, status);

	rsp.status = status;
	rsp.counter = cpu_to_be16(counter);
	rsp.items = cpu_to_be32(items);
	rsp.charset = cpu_to_be16(AVRCP_CHARSET_UTF8);
	rsp.depth = depth;

	iov[0].iov_base = &rsp;
	iov[0].iov_len = sizeof(rsp);

	if (!depth)
		return avrcp_send_browsing(session, transaction,
						AVRCP_SET_BROWSED_PLAYER,
						iov, 1);

	for (i = 0; i < depth; i++) {
		if (!folders[i])
			return -EINVAL;

		len[i] = strlen(folders[i]);

		iov[i * 2 + 2].iov_base = (void *) folders[i];
		iov[i * 2 + 2].iov_len = len[i];

		len[i] = cpu_to_be16(len[i]);

		iov[i * 2 + 1].iov_base = &len[i];
		iov[i * 2 + 1].iov_len = sizeof(len[i]);
	}

	return avrcp_send_browsing(session, transaction,
					AVRCP_SET_BROWSED_PLAYER, iov,
					depth * 2 + 1);
}

int avrcp_get_folder_items_rsp(struct avrcp *session, uint8_t transaction,
					uint8_t status, uint16_t counter,
					uint8_t number, uint8_t *type,
					uint16_t *len, uint8_t **params)
{
	struct iovec iov[UINT8_MAX * 2 + 1];
	struct get_folder_items_rsp rsp;
	uint8_t item[UINT8_MAX][3];
	int i;

	if (status != AVRCP_STATUS_SUCCESS)
		return avrcp_status_rsp(session, transaction,
					AVRCP_GET_FOLDER_ITEMS, status);

	rsp.status = status;
	rsp.counter = cpu_to_be16(counter);
	rsp.number = cpu_to_be16(number);

	iov[0].iov_base = &rsp;
	iov[0].iov_len = sizeof(rsp);

	for (i = 0; i < number; i++) {
		item[i][0] = type[i];
		put_be16(len[i], &item[i][1]);

		iov[i * 2 + 1].iov_base = item[i];
		iov[i * 2 + 1].iov_len = sizeof(item[i]);

		iov[i * 2 + 2].iov_base = params[i];
		iov[i * 2 + 2].iov_len = len[i];
	}

	return avrcp_send_browsing(session, transaction, AVRCP_GET_FOLDER_ITEMS,
							iov, number * 2 + 1);
}

int avrcp_change_path_rsp(struct avrcp *session, uint8_t transaction,
						uint8_t status, uint32_t items)
{
	struct iovec iov;
	struct change_path_rsp rsp;

	if (status != AVRCP_STATUS_SUCCESS)
		return avrcp_status_rsp(session, transaction, AVRCP_CHANGE_PATH,
									status);

	rsp.status = status;
	rsp.items = cpu_to_be32(items);

	iov.iov_base = &rsp;
	iov.iov_len = sizeof(rsp);

	return avrcp_send_browsing(session, transaction, AVRCP_CHANGE_PATH,
								&iov, 1);
}

static bool pack_attribute_list(struct iovec *iov, uint8_t number,
					uint32_t *attrs, const char **text)
{
	int i;
	struct media_item val[AVRCP_MEDIA_ATTRIBUTE_LAST];

	for (i = 0; i < number; i++) {
		uint16_t len = 0;

		if (attrs[i] > AVRCP_MEDIA_ATTRIBUTE_LAST ||
				attrs[i] == AVRCP_MEDIA_ATTRIBUTE_ILLEGAL)
			return false;

		if (text[i])
			len = strlen(text[i]);

		val[i].attr = cpu_to_be32(attrs[i]);
		val[i].charset = cpu_to_be16(AVRCP_CHARSET_UTF8);
		val[i].len = cpu_to_be16(len);

		iov[i].iov_base = &val[i];
		iov[i].iov_len = sizeof(val[i]);

		iov[i + 1].iov_base = (void *) text[i];
		iov[i + 1].iov_len = len;
	}

	return true;
}

int avrcp_get_item_attributes_rsp(struct avrcp *session, uint8_t transaction,
					uint8_t status, uint8_t number,
					uint32_t *attrs, const char **text)
{
	struct iovec iov[AVRCP_MEDIA_ATTRIBUTE_LAST * 2 + 1];
	struct get_item_attributes_rsp rsp;

	if (number > AVRCP_MEDIA_ATTRIBUTE_LAST)
		return -EINVAL;

	if (status != AVRCP_STATUS_SUCCESS)
		return avrcp_status_rsp(session, transaction,
					AVRCP_GET_ITEM_ATTRIBUTES, status);

	rsp.status = status;
	rsp.number = number;

	iov[0].iov_base = &rsp;
	iov[0].iov_len = sizeof(rsp);

	if (!pack_attribute_list(&iov[1], number, attrs, text))
		return -EINVAL;

	return avrcp_send_browsing(session, transaction,
					AVRCP_GET_ITEM_ATTRIBUTES, iov,
					number * 2 + 1);
}

int avrcp_play_item_rsp(struct avrcp *session, uint8_t transaction,
								uint8_t status)
{
	return avrcp_status_rsp(session, transaction, AVRCP_PLAY_ITEM,
								status);
}

int avrcp_search_rsp(struct avrcp *session, uint8_t transaction, uint8_t status,
					uint16_t counter, uint32_t items)
{
	struct iovec iov;
	struct search_rsp rsp;

	if (status != AVRCP_STATUS_SUCCESS)
		return avrcp_status_rsp(session, transaction, AVRCP_SEARCH,
								status);

	rsp.status = status;
	rsp.counter = cpu_to_be16(counter);
	rsp.items = cpu_to_be32(items);

	iov.iov_base = &rsp;
	iov.iov_len = sizeof(rsp);

	return avrcp_send_browsing(session, transaction, AVRCP_SEARCH,
								&iov, 1);
}

int avrcp_add_to_now_playing_rsp(struct avrcp *session, uint8_t transaction,
								uint8_t status)
{
	return avrcp_status_rsp(session, transaction, AVRCP_ADD_TO_NOW_PLAYING,
								status);
}

int avrcp_send_passthrough(struct avrcp *session, uint32_t vendor, uint8_t op)
{
	uint8_t params[5];

	if (!vendor)
		return avctp_send_passthrough(session->conn, op, NULL, 0);

	hton24(params, vendor);
	put_be16(op, &params[3]);

	return avctp_send_passthrough(session->conn, AVC_VENDOR_UNIQUE, params,
								sizeof(params));
}
