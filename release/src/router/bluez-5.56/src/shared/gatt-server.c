// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Google Inc.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/uio.h>
#include <errno.h>

#include "src/shared/att.h"
#include "lib/bluetooth.h"
#include "lib/uuid.h"
#include "src/shared/queue.h"
#include "src/shared/gatt-db.h"
#include "src/shared/gatt-server.h"
#include "src/shared/gatt-helpers.h"
#include "src/shared/util.h"
#include "src/shared/timeout.h"

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

/*
 * TODO: This is an arbitrary limit. Come up with something reasonable or
 * perhaps an API to set this value if there is a use case for it.
 */
#define DEFAULT_MAX_PREP_QUEUE_LEN 30

#define NFY_MULT_TIMEOUT 10

struct async_read_op {
	struct bt_att_chan *chan;
	struct bt_gatt_server *server;
	uint8_t opcode;
	bool done;
	uint8_t *pdu;
	size_t pdu_len;
	size_t value_len;
	struct queue *db_data;
};

struct async_write_op {
	struct bt_att_chan *chan;
	struct bt_gatt_server *server;
	uint8_t opcode;
};

struct prep_write_data {
	struct bt_gatt_server *server;
	uint8_t *value;
	uint16_t handle;
	uint16_t offset;
	uint16_t length;

	bool reliable_supported;
};

static void prep_write_data_destroy(void *user_data)
{
	struct prep_write_data *data = user_data;

	free(data->value);
	free(data);
}

struct nfy_mult_data {
	unsigned int id;
	uint8_t *pdu;
	uint16_t offset;
	uint16_t len;
};

struct bt_gatt_server {
	struct gatt_db *db;
	struct bt_att *att;
	int ref_count;
	uint16_t mtu;

	unsigned int mtu_id;
	unsigned int read_by_grp_type_id;
	unsigned int read_by_type_id;
	unsigned int find_info_id;
	unsigned int find_by_type_value_id;
	unsigned int write_id;
	unsigned int write_cmd_id;
	unsigned int read_id;
	unsigned int read_blob_id;
	unsigned int read_multiple_id;
	unsigned int read_multiple_vl_id;
	unsigned int prep_write_id;
	unsigned int exec_write_id;

	uint8_t min_enc_size;

	struct queue *prep_queue;
	unsigned int max_prep_queue_len;

	bt_gatt_server_debug_func_t debug_callback;
	bt_gatt_server_destroy_func_t debug_destroy;
	void *debug_data;

	bt_gatt_server_authorize_cb_t authorize;
	void *authorize_data;

	struct nfy_mult_data *nfy_mult;
};

static void bt_gatt_server_free(struct bt_gatt_server *server)
{
	if (server->debug_destroy)
		server->debug_destroy(server->debug_data);

	bt_att_unregister(server->att, server->mtu_id);
	bt_att_unregister(server->att, server->read_by_grp_type_id);
	bt_att_unregister(server->att, server->read_by_type_id);
	bt_att_unregister(server->att, server->find_info_id);
	bt_att_unregister(server->att, server->find_by_type_value_id);
	bt_att_unregister(server->att, server->write_id);
	bt_att_unregister(server->att, server->write_cmd_id);
	bt_att_unregister(server->att, server->read_id);
	bt_att_unregister(server->att, server->read_blob_id);
	bt_att_unregister(server->att, server->read_multiple_id);
	bt_att_unregister(server->att, server->read_multiple_vl_id);
	bt_att_unregister(server->att, server->prep_write_id);
	bt_att_unregister(server->att, server->exec_write_id);

	queue_destroy(server->prep_queue, prep_write_data_destroy);

	gatt_db_unref(server->db);
	bt_att_unref(server->att);
	free(server);
}

static bool get_uuid_le(const uint8_t *uuid, size_t len, bt_uuid_t *out_uuid)
{
	uint128_t u128;

	switch (len) {
	case 2:
		bt_uuid16_create(out_uuid, get_le16(uuid));
		return true;
	case 16:
		bswap_128(uuid, &u128.data);
		bt_uuid128_create(out_uuid, u128);
		return true;
	default:
		return false;
	}

	return false;
}

static void attribute_read_cb(struct gatt_db_attribute *attrib, int err,
					const uint8_t *value, size_t length,
					void *user_data)
{
	struct iovec *iov = user_data;

	iov->iov_base = (void *) value;
	iov->iov_len = length;
}

static bool encode_read_by_grp_type_rsp(struct gatt_db *db, struct queue *q,
						struct bt_att *att,
						uint16_t mtu, uint8_t *pdu,
						uint16_t *len)
{
	int iter = 0;
	uint16_t start_handle, end_handle;
	struct iovec value;
	uint8_t data_val_len;

	*len = 0;

	while (queue_peek_head(q)) {
		struct gatt_db_attribute *attrib = queue_pop_head(q);

		value.iov_base = NULL;
		value.iov_len = 0;

		/*
		 * This should never be deferred to the read callback for
		 * primary/secondary service declarations.
		 */
		if (!gatt_db_attribute_read(attrib, 0,
						BT_ATT_OP_READ_BY_GRP_TYPE_REQ,
						att, attribute_read_cb,
						&value) || !value.iov_len)
			return false;

		/*
		 * Use the first attribute to determine the length of each
		 * attribute data unit. Stop the list when a different attribute
		 * value is seen.
		 */
		if (iter == 0) {
			data_val_len = MIN(MIN((unsigned)mtu - 6, 251),
								value.iov_len);
			pdu[0] = data_val_len + 4;
			iter++;
		} else if (value.iov_len != data_val_len)
			break;

		/* Stop if this unit would surpass the MTU */
		if (iter + data_val_len + 4 > mtu - 1)
			break;

		gatt_db_attribute_get_service_handles(attrib, &start_handle,
								&end_handle);

		put_le16(start_handle, pdu + iter);
		put_le16(end_handle, pdu + iter + 2);
		memcpy(pdu + iter + 4, value.iov_base, data_val_len);

		iter += data_val_len + 4;
	}

	*len = iter;

	return true;
}

static void read_by_grp_type_cb(struct bt_att_chan *chan, uint8_t opcode,
					const void *pdu, uint16_t length,
					void *user_data)
{
	struct bt_gatt_server *server = user_data;
	uint16_t start, end;
	bt_uuid_t type;
	bt_uuid_t prim, snd;
	uint16_t mtu = bt_att_get_mtu(server->att);
	uint8_t rsp_pdu[mtu];
	uint16_t rsp_len;
	uint8_t ecode = 0;
	uint16_t ehandle = 0;
	struct queue *q = NULL;

	if (length != 6 && length != 20) {
		ecode = BT_ATT_ERROR_INVALID_PDU;
		goto error;
	}

	q = queue_new();

	start = get_le16(pdu);
	end = get_le16(pdu + 2);
	get_uuid_le(pdu + 4, length - 4, &type);

	util_debug(server->debug_callback, server->debug_data,
				"Read By Grp Type - start: 0x%04x end: 0x%04x",
				start, end);

	if (!start || !end) {
		ecode = BT_ATT_ERROR_INVALID_HANDLE;
		goto error;
	}

	ehandle = start;

	if (start > end) {
		ecode = BT_ATT_ERROR_INVALID_HANDLE;
		goto error;
	}

	/*
	 * GATT defines that only the <<Primary Service>> and
	 * <<Secondary Service>> group types can be used for the
	 * "Read By Group Type" request (Core v4.1, Vol 3, sec 2.5.3). Return an
	 * error if any other group type is given.
	 */
	bt_uuid16_create(&prim, GATT_PRIM_SVC_UUID);
	bt_uuid16_create(&snd, GATT_SND_SVC_UUID);
	if (bt_uuid_cmp(&type, &prim) && bt_uuid_cmp(&type, &snd)) {
		ecode = BT_ATT_ERROR_UNSUPPORTED_GROUP_TYPE;
		goto error;
	}

	gatt_db_read_by_group_type(server->db, start, end, type, q);

	if (queue_isempty(q)) {
		ecode = BT_ATT_ERROR_ATTRIBUTE_NOT_FOUND;
		goto error;
	}

	if (!encode_read_by_grp_type_rsp(server->db, q, server->att, mtu,
							rsp_pdu, &rsp_len)) {
		ecode = BT_ATT_ERROR_UNLIKELY;
		goto error;
	}

	queue_destroy(q, NULL);

	bt_att_chan_send_rsp(chan, BT_ATT_OP_READ_BY_GRP_TYPE_RSP,
						rsp_pdu, rsp_len);

	return;

error:
	queue_destroy(q, NULL);
	bt_att_chan_send_error_rsp(chan, opcode, ehandle, ecode);
}

static void async_read_op_destroy(struct async_read_op *op)
{
	bt_gatt_server_unref(op->server);
	queue_destroy(op->db_data, NULL);
	free(op->pdu);
	free(op);
}

static void process_read_by_type(struct async_read_op *op);

static void read_by_type_read_complete_cb(struct gatt_db_attribute *attr,
						int err, const uint8_t *value,
						size_t len, void *user_data)
{
	struct async_read_op *op = user_data;
	struct bt_gatt_server *server = op->server;
	uint16_t mtu;
	uint16_t handle;

	mtu = bt_att_get_mtu(server->att);
	handle = gatt_db_attribute_get_handle(attr);

	/* Terminate the operation if there was an error */
	if (err) {
		bt_att_chan_send_error_rsp(op->chan, BT_ATT_OP_READ_BY_TYPE_REQ,
								handle, err);
		async_read_op_destroy(op);
		return;
	}

	if (op->pdu_len == 0) {
		op->value_len = MIN(MIN((unsigned) mtu - 4, 253), len);
		op->pdu[0] = op->value_len + 2;
		op->pdu_len++;
	} else if (len != op->value_len) {
		op->done = true;
		goto done;
	}

	/* Stop if this would surpass the MTU */
	if (op->pdu_len + op->value_len + 2 > (unsigned) mtu - 1) {
		op->done = true;
		goto done;
	}

	/* Encode the current value */
	put_le16(handle, op->pdu + op->pdu_len);
	memcpy(op->pdu + op->pdu_len + 2, value, op->value_len);

	op->pdu_len += op->value_len + 2;

	if (op->pdu_len == (unsigned) mtu - 1)
		op->done = true;

done:
	process_read_by_type(op);
}

static bool check_min_key_size(uint8_t min_size, uint8_t size)
{
	if (!min_size || !size)
		return true;

	return min_size <= size;
}

static uint8_t check_permissions(struct bt_gatt_server *server,
				struct gatt_db_attribute *attr, uint32_t mask)
{
	uint8_t enc_size;
	uint32_t perm;
	int security;

	perm = gatt_db_attribute_get_permissions(attr);

	if (perm && mask & BT_ATT_PERM_READ && !(perm & BT_ATT_PERM_READ))
		return BT_ATT_ERROR_READ_NOT_PERMITTED;

	if (perm && mask & BT_ATT_PERM_WRITE && !(perm & BT_ATT_PERM_WRITE))
		return BT_ATT_ERROR_WRITE_NOT_PERMITTED;

	perm &= mask;
	if (!perm)
		return 0;

	security = bt_att_get_security(server->att, &enc_size);
	if (security < 0)
		return BT_ATT_ERROR_UNLIKELY;

	if (perm & BT_ATT_PERM_SECURE) {
		if (security < BT_ATT_SECURITY_FIPS)
			return BT_ATT_ERROR_AUTHENTICATION;

		if (!check_min_key_size(server->min_enc_size, enc_size))
			return BT_ATT_ERROR_INSUFFICIENT_ENCRYPTION_KEY_SIZE;
	}

	if (perm & BT_ATT_PERM_AUTHEN) {
		if (security < BT_ATT_SECURITY_HIGH)
			return BT_ATT_ERROR_AUTHENTICATION;

		if (!check_min_key_size(server->min_enc_size, enc_size))
			return BT_ATT_ERROR_INSUFFICIENT_ENCRYPTION_KEY_SIZE;
	}

	if (perm & BT_ATT_PERM_ENCRYPT) {
		if (security < BT_ATT_SECURITY_MEDIUM)
			return BT_ATT_ERROR_INSUFFICIENT_ENCRYPTION;

		if (!check_min_key_size(server->min_enc_size, enc_size))
			return BT_ATT_ERROR_INSUFFICIENT_ENCRYPTION_KEY_SIZE;
	}

	return 0;
}

static void process_read_by_type(struct async_read_op *op)
{
	struct bt_gatt_server *server = op->server;
	uint8_t ecode;
	struct gatt_db_attribute *attr;

	attr = queue_pop_head(op->db_data);

	if (op->done || !attr) {
		bt_att_chan_send_rsp(op->chan, BT_ATT_OP_READ_BY_TYPE_RSP,
						op->pdu, op->pdu_len);
		async_read_op_destroy(op);
		return;
	}

	ecode = check_permissions(server, attr, BT_ATT_PERM_READ |
						BT_ATT_PERM_READ_AUTHEN |
						BT_ATT_PERM_READ_ENCRYPT);
	if (ecode)
		goto error;

	if (gatt_db_attribute_read(attr, 0, op->opcode, server->att,
					read_by_type_read_complete_cb, op))
		return;

	ecode = BT_ATT_ERROR_UNLIKELY;

error:
	bt_att_chan_send_error_rsp(op->chan, BT_ATT_OP_READ_BY_TYPE_REQ,
				gatt_db_attribute_get_handle(attr), ecode);
	async_read_op_destroy(op);
}

static void read_by_type_cb(struct bt_att_chan *chan, uint8_t opcode,
					const void *pdu, uint16_t length,
					void *user_data)
{
	struct bt_gatt_server *server = user_data;
	uint16_t start, end;
	bt_uuid_t type;
	uint16_t ehandle = 0;
	uint8_t ecode;
	struct queue *q = NULL;
	struct async_read_op *op;

	if (length != 6 && length != 20) {
		ecode = BT_ATT_ERROR_INVALID_PDU;
		goto error;
	}

	q = queue_new();

	start = get_le16(pdu);
	end = get_le16(pdu + 2);
	get_uuid_le(pdu + 4, length - 4, &type);

	util_debug(server->debug_callback, server->debug_data,
				"Read By Type - start: 0x%04x end: 0x%04x",
				start, end);

	if (!start || !end) {
		ecode = BT_ATT_ERROR_INVALID_HANDLE;
		goto error;
	}

	ehandle = start;

	if (start > end) {
		ecode = BT_ATT_ERROR_INVALID_HANDLE;
		goto error;
	}

	gatt_db_read_by_type(server->db, start, end, type, q);

	if (queue_isempty(q)) {
		ecode = BT_ATT_ERROR_ATTRIBUTE_NOT_FOUND;
		goto error;
	}

	op = new0(struct async_read_op, 1);
	op->pdu = malloc(bt_att_get_mtu(server->att));
	if (!op->pdu) {
		free(op);
		ecode = BT_ATT_ERROR_INSUFFICIENT_RESOURCES;
		goto error;
	}

	op->chan = chan;
	op->opcode = opcode;
	op->server = bt_gatt_server_ref(server);
	op->db_data = q;

	process_read_by_type(op);

	return;

error:
	bt_att_chan_send_error_rsp(chan, opcode, ehandle, ecode);
	queue_destroy(q, NULL);
}

static bool encode_find_info_rsp(struct gatt_db *db, struct queue *q,
						uint16_t mtu,
						uint8_t *pdu, uint16_t *len)
{
	uint16_t handle;
	struct gatt_db_attribute *attr;
	const bt_uuid_t *type;
	int uuid_len, cur_uuid_len;
	int iter = 0;

	*len = 0;

	while (queue_peek_head(q)) {
		attr = queue_pop_head(q);
		handle = gatt_db_attribute_get_handle(attr);
		type = gatt_db_attribute_get_type(attr);
		if (!handle || !type)
			return false;

		cur_uuid_len = bt_uuid_len(type);

		if (iter == 0) {
			switch (cur_uuid_len) {
			case 2:
				uuid_len = 2;
				pdu[0] = 0x01;
				break;
			case 4:
			case 16:
				uuid_len = 16;
				pdu[0] = 0x02;
				break;
			default:
				return false;
			}

			iter++;
		} else if (cur_uuid_len != uuid_len)
			break;

		if (iter + uuid_len + 2 > mtu - 1)
			break;

		put_le16(handle, pdu + iter);
		bt_uuid_to_le(type, pdu + iter + 2);

		iter += uuid_len + 2;
	}

	*len = iter;

	return true;
}

static void find_info_cb(struct bt_att_chan *chan, uint8_t opcode,
					const void *pdu, uint16_t length,
					void *user_data)
{
	struct bt_gatt_server *server = user_data;
	uint16_t start, end;
	uint16_t mtu = bt_att_get_mtu(server->att);
	uint8_t rsp_pdu[mtu];
	uint16_t rsp_len;
	uint8_t ecode = 0;
	uint16_t ehandle = 0;
	struct queue *q = NULL;

	if (length != 4) {
		ecode = BT_ATT_ERROR_INVALID_PDU;
		goto error;
	}

	q = queue_new();

	start = get_le16(pdu);
	end = get_le16(pdu + 2);

	util_debug(server->debug_callback, server->debug_data,
					"Find Info - start: 0x%04x end: 0x%04x",
					start, end);

	if (!start || !end) {
		ecode = BT_ATT_ERROR_INVALID_HANDLE;
		goto error;
	}

	ehandle = start;

	if (start > end) {
		ecode = BT_ATT_ERROR_INVALID_HANDLE;
		goto error;
	}

	gatt_db_find_information(server->db, start, end, q);

	if (queue_isempty(q)) {
		ecode = BT_ATT_ERROR_ATTRIBUTE_NOT_FOUND;
		goto error;
	}

	if (!encode_find_info_rsp(server->db, q, mtu, rsp_pdu, &rsp_len)) {
		ecode = BT_ATT_ERROR_UNLIKELY;
		goto error;
	}

	bt_att_chan_send_rsp(chan, BT_ATT_OP_FIND_INFO_RSP, rsp_pdu, rsp_len);

	queue_destroy(q, NULL);

	return;

error:
	bt_att_chan_send_error_rsp(chan, opcode, ehandle, ecode);
	queue_destroy(q, NULL);

}

struct find_by_type_val_data {
	uint8_t *pdu;
	uint16_t len;
	uint16_t mtu;
	uint8_t ecode;
};

static void find_by_type_val_att_cb(struct gatt_db_attribute *attrib,
								void *user_data)
{
	uint16_t handle, end_handle;
	struct find_by_type_val_data *data = user_data;

	if (data->ecode)
		return;

	if (data->len + 4 > data->mtu - 1)
		return;

	/*
	 * This OP is only valid for Primary Service per the spec
	 * page 562, so this should work.
	 */
	gatt_db_attribute_get_service_data(attrib, &handle, &end_handle, NULL,
									NULL);

	if (!handle || !end_handle) {
		data->ecode = BT_ATT_ERROR_UNLIKELY;
		return;
	}

	put_le16(handle, data->pdu + data->len);
	put_le16(end_handle, data->pdu + data->len + 2);

	data->len += 4;
}

static void find_by_type_val_cb(struct bt_att_chan *chan, uint8_t opcode,
					const void *pdu, uint16_t length,
					void *user_data)
{
	struct bt_gatt_server *server = user_data;
	uint16_t start, end, uuid16;
	struct find_by_type_val_data data;
	uint16_t mtu = bt_att_get_mtu(server->att);
	uint8_t rsp_pdu[mtu];
	uint16_t ehandle = 0;
	bt_uuid_t uuid;

	if (length < 6) {
		data.ecode = BT_ATT_ERROR_INVALID_PDU;
		goto error;
	}

	data.pdu = rsp_pdu;
	data.len = 0;
	data.mtu = mtu;
	data.ecode = 0;

	start = get_le16(pdu);
	end = get_le16(pdu + 2);
	uuid16 = get_le16(pdu + 4);

	util_debug(server->debug_callback, server->debug_data,
			"Find By Type Value - start: 0x%04x end: 0x%04x uuid: 0x%04x",
			start, end, uuid16);
	ehandle = start;
	if (start > end) {
		data.ecode = BT_ATT_ERROR_INVALID_HANDLE;
		goto error;
	}

	bt_uuid16_create(&uuid, uuid16);
	gatt_db_find_by_type_value(server->db, start, end, &uuid, pdu + 6,
							length - 6,
							find_by_type_val_att_cb,
							&data);

	if (!data.len)
		data.ecode = BT_ATT_ERROR_ATTRIBUTE_NOT_FOUND;

	if (data.ecode)
		goto error;

	bt_att_chan_send_rsp(chan, BT_ATT_OP_FIND_BY_TYPE_RSP,
					data.pdu, data.len);

	return;

error:
	bt_att_chan_send_error_rsp(chan, opcode, ehandle, data.ecode);
}

static void async_write_op_destroy(struct async_write_op *op)
{
	bt_gatt_server_unref(op->server);
	free(op);
}

static void write_complete_cb(struct gatt_db_attribute *attr, int err,
								void *user_data)
{
	struct async_write_op *op = user_data;
	struct bt_gatt_server *server = op->server;
	uint16_t handle;

	if (op->opcode == BT_ATT_OP_WRITE_CMD) {
		async_write_op_destroy(op);
		return;
	}

	util_debug(server->debug_callback, server->debug_data,
						"Write Complete: err %d", err);

	handle = gatt_db_attribute_get_handle(attr);

	if (err)
		bt_att_chan_send_error_rsp(op->chan, op->opcode, handle, err);
	else
		bt_att_chan_send_rsp(op->chan, BT_ATT_OP_WRITE_RSP, NULL, 0);

	async_write_op_destroy(op);
}

static uint8_t authorize_req(struct bt_gatt_server *server,
					uint8_t opcode, uint16_t handle)
{
	if (!server->authorize)
		return 0;

	return server->authorize(server->att, opcode, handle,
						server->authorize_data);
}

static void write_cb(struct bt_att_chan *chan, uint8_t opcode, const void *pdu,
					uint16_t length, void *user_data)
{
	struct bt_gatt_server *server = user_data;
	struct gatt_db_attribute *attr;
	uint16_t handle = 0;
	struct async_write_op *op = NULL;
	uint8_t ecode;

	if (length < 2) {
		ecode = BT_ATT_ERROR_INVALID_PDU;
		goto error;
	}

	ecode = authorize_req(server, opcode, handle);
	if (ecode)
		goto error;

	handle = get_le16(pdu);
	attr = gatt_db_get_attribute(server->db, handle);
	if (!attr) {
		ecode = BT_ATT_ERROR_INVALID_HANDLE;
		goto error;
	}

	util_debug(server->debug_callback, server->debug_data,
				"Write %s - handle: 0x%04x",
				(opcode == BT_ATT_OP_WRITE_REQ) ? "Req" : "Cmd",
				handle);

	ecode = check_permissions(server, attr, BT_ATT_PERM_WRITE |
						BT_ATT_PERM_WRITE_AUTHEN |
						BT_ATT_PERM_WRITE_ENCRYPT);
	if (ecode)
		goto error;

	op = new0(struct async_write_op, 1);
	op->chan = chan;
	op->server = bt_gatt_server_ref(server);
	op->opcode = opcode;

	if (gatt_db_attribute_write(attr, 0, pdu + 2, length - 2, opcode,
							server->att,
							write_complete_cb, op))
		return;

	async_write_op_destroy(op);

	ecode = BT_ATT_ERROR_UNLIKELY;

error:
	if (opcode == BT_ATT_OP_WRITE_CMD)
		return;

	bt_att_chan_send_error_rsp(chan, opcode, handle, ecode);
}

static uint8_t get_read_rsp_opcode(uint8_t opcode)
{

	switch (opcode) {
	case BT_ATT_OP_READ_REQ:
		return BT_ATT_OP_READ_RSP;
	case BT_ATT_OP_READ_BLOB_REQ:
		return BT_ATT_OP_READ_BLOB_RSP;
	default:
		/*
		 * Should never happen
		 *
		 * TODO: It would be nice to have a debug-mode assert macro
		 * for development builds. This way bugs could be easily catched
		 * during development and there would be self documenting code
		 * that wouldn't be crash release builds.
		 */
		return 0;
	}

	return 0;
}

static void read_complete_cb(struct gatt_db_attribute *attr, int err,
					const uint8_t *value, size_t len,
					void *user_data)
{
	struct async_read_op *op = user_data;
	struct bt_gatt_server *server = op->server;
	uint8_t rsp_opcode;
	uint16_t mtu;
	uint16_t handle;

	util_debug(server->debug_callback, server->debug_data,
				"Read Complete: err %d", err);

	mtu = bt_att_get_mtu(server->att);
	handle = gatt_db_attribute_get_handle(attr);

	if (err) {
		bt_att_chan_send_error_rsp(op->chan, op->opcode, handle, err);
		async_read_op_destroy(op);
		return;
	}

	rsp_opcode = get_read_rsp_opcode(op->opcode);

	bt_att_chan_send_rsp(op->chan, rsp_opcode, len ? value : NULL,
					MIN((unsigned int) mtu - 1, len));
	async_read_op_destroy(op);
}

static void handle_read_req(struct bt_att_chan *chan,
				struct bt_gatt_server *server, uint8_t opcode,
				uint16_t handle, uint16_t offset)
{
	struct gatt_db_attribute *attr;
	uint8_t ecode;
	struct async_read_op *op = NULL;

	ecode = authorize_req(server, opcode, handle);
	if (ecode)
		goto error;

	attr = gatt_db_get_attribute(server->db, handle);
	if (!attr) {
		ecode = BT_ATT_ERROR_INVALID_HANDLE;
		goto error;
	}

	util_debug(server->debug_callback, server->debug_data,
			"Read %sReq - handle: 0x%04x",
			opcode == BT_ATT_OP_READ_BLOB_REQ ? "Blob " : "",
			handle);

	ecode = check_permissions(server, attr, BT_ATT_PERM_READ |
						BT_ATT_PERM_READ_AUTHEN |
						BT_ATT_PERM_READ_ENCRYPT);
	if (ecode)
		goto error;

	op = new0(struct async_read_op, 1);
	op->chan = chan;
	op->opcode = opcode;
	op->server = bt_gatt_server_ref(server);

	if (gatt_db_attribute_read(attr, offset, opcode, server->att,
							read_complete_cb, op))
		return;

	ecode = BT_ATT_ERROR_UNLIKELY;

error:
	if (op)
		async_read_op_destroy(op);

	bt_att_chan_send_error_rsp(chan, opcode, handle, ecode);
}

static void read_cb(struct bt_att_chan *chan, uint8_t opcode, const void *pdu,
					uint16_t length, void *user_data)
{
	struct bt_gatt_server *server = user_data;
	uint16_t handle;

	if (length != 2) {
		bt_att_chan_send_error_rsp(chan, opcode, 0,
						BT_ATT_ERROR_INVALID_PDU);
		return;
	}

	handle = get_le16(pdu);

	handle_read_req(chan, server, opcode, handle, 0);
}

static void read_blob_cb(struct bt_att_chan *chan, uint8_t opcode,
					const void *pdu, uint16_t length,
					void *user_data)
{
	struct bt_gatt_server *server = user_data;
	uint16_t handle, offset;

	if (length != 4) {
		bt_att_chan_send_error_rsp(chan, opcode, 0,
						BT_ATT_ERROR_INVALID_PDU);
		return;
	}

	handle = get_le16(pdu);
	offset = get_le16(pdu + 2);

	handle_read_req(chan, server, opcode, handle, offset);
}

struct read_mult_data {
	struct bt_att_chan *chan;
	struct bt_gatt_server *server;
	uint8_t opcode;
	uint16_t *handles;
	size_t cur_handle;
	size_t num_handles;
	uint8_t *rsp_data;
	size_t length;
	size_t mtu;
};

static void read_mult_data_free(struct read_mult_data *data)
{
	free(data->handles);
	free(data->rsp_data);
	free(data);
}

static void read_multiple_complete_cb(struct gatt_db_attribute *attr, int err,
					const uint8_t *value, size_t len,
					void *user_data)
{
	struct read_mult_data *data = user_data;
	struct gatt_db_attribute *next_attr;
	uint16_t handle = gatt_db_attribute_get_handle(attr);
	uint8_t ecode;
	uint16_t length;

	if (err != 0) {
		ecode = err;
		goto error;
	}

	length = data->opcode == BT_ATT_OP_READ_MULT_VL_REQ ?
			MIN(len, MAX(data->mtu - data->length, 3) - 3) :
			MIN(len, data->mtu - data->length - 1);

	if (data->opcode == BT_ATT_OP_READ_MULT_VL_REQ) {
		/* The Length Value Tuple List may be truncated within the first
		 * two octets of a tuple due to the size limits of the current
		 * ATT_MTU, but the first two octets cannot be separated.
		 */
		if (data->mtu - data->length >= 3) {
			put_le16(len, data->rsp_data + data->length);
			data->length += 2;
		}
	}

	memcpy(data->rsp_data + data->length, value, length);
	data->length += length;

	data->cur_handle++;

	if (data->cur_handle == data->num_handles) {
		bt_att_chan_send_rsp(data->chan, data->opcode + 1,
						data->rsp_data, data->length);
		read_mult_data_free(data);
		return;
	}

	handle = data->handles[data->cur_handle];

	util_debug(data->server->debug_callback, data->server->debug_data,
				"%s Req - #%zu of %zu: 0x%04x",
				data->opcode == BT_ATT_OP_READ_MULT_REQ ?
				"Read Multiple" :
				"Read Multiple Variable Length",
				data->cur_handle + 1, data->num_handles,
				handle);

	next_attr = gatt_db_get_attribute(data->server->db, handle);

	if (!next_attr) {
		ecode = BT_ATT_ERROR_INVALID_HANDLE;
		goto error;
	}

	ecode = check_permissions(data->server, next_attr, BT_ATT_PERM_READ |
						BT_ATT_PERM_READ_AUTHEN |
						BT_ATT_PERM_READ_ENCRYPT);
	if (ecode)
		goto error;

	if (gatt_db_attribute_read(next_attr, 0, data->opcode,
					data->server->att,
					read_multiple_complete_cb, data))
		return;

	ecode = BT_ATT_ERROR_UNLIKELY;

error:
	bt_att_chan_send_error_rsp(data->chan, data->opcode, handle, ecode);
	read_mult_data_free(data);
}

static struct read_mult_data *read_mult_data_new(struct bt_gatt_server *server,
						struct bt_att_chan *chan,
						uint8_t opcode,
						uint16_t num_handles)
{
	struct read_mult_data *data;

	data = new0(struct read_mult_data, 1);
	data->chan = chan;
	data->opcode = opcode;
	data->handles = new0(uint16_t, num_handles);
	data->rsp_data = NULL;
	data->server = server;
	data->num_handles = num_handles;
	data->cur_handle = 0;
	data->mtu = bt_att_get_mtu(server->att);
	data->length = 0;
	data->rsp_data = new0(uint8_t, data->mtu - 1);

	return data;
}

static void read_multiple_cb(struct bt_att_chan *chan, uint8_t opcode,
					const void *pdu, uint16_t length,
					void *user_data)
{
	struct bt_gatt_server *server = user_data;
	struct gatt_db_attribute *attr;
	struct read_mult_data *data = NULL;
	uint8_t ecode;
	size_t i = 0;
	uint16_t handle = 0;

	if (length < 4) {
		ecode = BT_ATT_ERROR_INVALID_PDU;
		goto error;
	}

	data = read_mult_data_new(server, chan, opcode, length / 2);
	if (!data)
		goto error;

	for (i = 0; i < data->num_handles; i++)
		data->handles[i] = get_le16(pdu + i * 2);

	handle = data->handles[0];

	util_debug(server->debug_callback, server->debug_data,
			"%s Req - %zu handles, 1st: 0x%04x",
			data->opcode == BT_ATT_OP_READ_MULT_REQ ?
			"Read Multiple" : "Read Multiple Variable Length",
			data->num_handles, handle);

	attr = gatt_db_get_attribute(server->db, handle);

	if (!attr) {
		ecode = BT_ATT_ERROR_INVALID_HANDLE;
		goto error;
	}

	ecode = check_permissions(data->server, attr, BT_ATT_PERM_READ |
						BT_ATT_PERM_READ_AUTHEN |
						BT_ATT_PERM_READ_ENCRYPT);
	if (ecode)
		goto error;

	if (gatt_db_attribute_read(attr, 0, opcode, server->att,
					read_multiple_complete_cb, data))
		return;

	ecode = BT_ATT_ERROR_UNLIKELY;

error:
	if (data)
		read_mult_data_free(data);

	bt_att_chan_send_error_rsp(chan, opcode, handle, ecode);
}

static bool append_prep_data(struct prep_write_data *prep_data, uint16_t handle,
					uint16_t length, uint8_t *value)
{
	uint8_t *val;
	uint16_t len;

	if (!length)
		return true;

	len = prep_data->length + length;

	val = realloc(prep_data->value, len);
	if (!val)
		return false;

	memcpy(val + prep_data->length, value, length);

	prep_data->value = val;
	prep_data->length = len;

	return true;
}

static bool is_reliable_write_supported(const struct bt_gatt_server  *server,
							uint16_t handle)
{
	struct gatt_db_attribute *attr;
	uint16_t ext_prop;

	attr = gatt_db_get_attribute(server->db, handle);
	if (!attr)
		return false;

	if (!gatt_db_attribute_get_char_data(attr, NULL, NULL, NULL, &ext_prop,
									NULL))
		return false;

	return (ext_prop & BT_GATT_CHRC_EXT_PROP_RELIABLE_WRITE);
}

static bool prep_data_new(struct bt_gatt_server *server,
					uint16_t handle, uint16_t offset,
					uint16_t length, uint8_t *value)
{
	struct prep_write_data *prep_data;

	prep_data = new0(struct prep_write_data, 1);

	if (!append_prep_data(prep_data, handle, length, value)) {
		prep_write_data_destroy(prep_data);
		return false;
	}

	prep_data->server = server;
	prep_data->handle = handle;
	prep_data->offset = offset;

	/*
	 * Handle is the value handle. We need characteristic declaration
	 * handle which in BlueZ is handle_value -1
	 */
	prep_data->reliable_supported = is_reliable_write_supported(server,
								handle - 1);

	queue_push_tail(server->prep_queue, prep_data);

	return true;
}

static bool store_prep_data(struct bt_gatt_server *server,
					uint16_t handle, uint16_t offset,
					uint16_t length, uint8_t *value)
{
	struct prep_write_data *prep_data = NULL;

	/*
	 * Now lets check if prep write is a continuation of long write
	 * If so do aggregation of data
	 */
	prep_data = queue_peek_tail(server->prep_queue);
	if (prep_data && (prep_data->handle == handle) &&
			(offset == (prep_data->length + prep_data->offset)))
		return append_prep_data(prep_data, handle, length, value);

	return prep_data_new(server, handle, offset, length, value);
}

struct prep_write_complete_data {
	struct bt_att_chan *chan;
	void *pdu;
	uint16_t length;
	struct bt_gatt_server *server;
};

static void prep_write_complete_cb(struct gatt_db_attribute *attr, int err,
								void *user_data)
{
	struct prep_write_complete_data *pwcd = user_data;
	uint16_t handle = 0;
	uint16_t offset;

	handle = get_le16(pwcd->pdu);

	if (err) {
		bt_att_chan_send_error_rsp(pwcd->chan, BT_ATT_OP_PREP_WRITE_REQ,
								handle, err);
		free(pwcd->pdu);
		free(pwcd);

		return;
	}

	offset = get_le16(pwcd->pdu + 2);

	if (!store_prep_data(pwcd->server, handle, offset, pwcd->length - 4,
						&((uint8_t *) pwcd->pdu)[4]))
		bt_att_chan_send_error_rsp(pwcd->chan, BT_ATT_OP_PREP_WRITE_RSP,
					handle,
					BT_ATT_ERROR_INSUFFICIENT_RESOURCES);

	bt_att_chan_send_rsp(pwcd->chan, BT_ATT_OP_PREP_WRITE_RSP, pwcd->pdu,
								pwcd->length);

	free(pwcd->pdu);
	free(pwcd);
}

static void prep_write_cb(struct bt_att_chan *chan, uint8_t opcode,
					const void *pdu, uint16_t length,
					void *user_data)
{
	struct bt_gatt_server *server = user_data;
	uint16_t handle = 0;
	uint16_t offset;
	struct gatt_db_attribute *attr;
	struct prep_write_complete_data *pwcd;
	uint8_t ecode, status;

	if (length < 4) {
		ecode = BT_ATT_ERROR_INVALID_PDU;
		goto error;
	}

	if (queue_length(server->prep_queue) >= server->max_prep_queue_len) {
		ecode = BT_ATT_ERROR_PREPARE_QUEUE_FULL;
		goto error;
	}

	handle = get_le16(pdu);
	offset = get_le16(pdu + 2);

	attr = gatt_db_get_attribute(server->db, handle);
	if (!attr) {
		ecode = BT_ATT_ERROR_INVALID_HANDLE;
		goto error;
	}

	util_debug(server->debug_callback, server->debug_data,
				"Prep Write Req - handle: 0x%04x", handle);

	ecode = check_permissions(server, attr, BT_ATT_PERM_WRITE |
						BT_ATT_PERM_WRITE_AUTHEN |
						BT_ATT_PERM_WRITE_ENCRYPT);
	if (ecode)
		goto error;

	pwcd = new0(struct prep_write_complete_data, 1);
	pwcd->chan = chan;
	pwcd->pdu = malloc(length);
	memcpy(pwcd->pdu, pdu, length);
	pwcd->length = length;
	pwcd->server = server;

	status = gatt_db_attribute_write(attr, offset, NULL, 0,
						BT_ATT_OP_PREP_WRITE_REQ,
						server->att,
						prep_write_complete_cb, pwcd);

	if (status)
		return;

	ecode = BT_ATT_ERROR_UNLIKELY;

error:
	bt_att_chan_send_error_rsp(chan, opcode, handle, ecode);
}

struct exec_data {
	struct bt_att_chan *chan;
	struct bt_gatt_server *server;
};

static void exec_next_prep_write(struct exec_data *data, uint16_t ehandle,
								int err);

static void exec_write_complete_cb(struct gatt_db_attribute *attr, int err,
								void *user_data)
{
	struct exec_data *data = user_data;
	uint16_t handle = gatt_db_attribute_get_handle(attr);

	exec_next_prep_write(data, handle, err);
}

static void exec_next_prep_write(struct exec_data *data, uint16_t ehandle,
								int err)
{
	struct prep_write_data *next = NULL;
	struct gatt_db_attribute *attr;
	bool status;

	if (err)
		goto error;

	next = queue_pop_head(data->server->prep_queue);
	if (!next) {
		bt_att_chan_send_rsp(data->chan, BT_ATT_OP_EXEC_WRITE_RSP,
								NULL, 0);
		free(data);
		return;
	}

	attr = gatt_db_get_attribute(data->server->db, next->handle);
	if (!attr) {
		err = BT_ATT_ERROR_UNLIKELY;
		goto error;
	}

	status = gatt_db_attribute_write(attr, next->offset,
						next->value, next->length,
						BT_ATT_OP_EXEC_WRITE_REQ,
						data->server->att,
						exec_write_complete_cb, data);

	prep_write_data_destroy(next);

	if (status)
		return;

	err = BT_ATT_ERROR_UNLIKELY;

error:
	queue_remove_all(data->server->prep_queue, NULL, NULL,
						prep_write_data_destroy);

	bt_att_chan_send_error_rsp(data->chan, BT_ATT_OP_EXEC_WRITE_REQ,
								ehandle, err);
	free(data);
}

static bool find_no_reliable_characteristic(const void *data,
						const void *match_data)
{
	const struct prep_write_data *prep_data = data;

	return !prep_data->reliable_supported;
}

static void exec_write_cb(struct bt_att_chan *chan, uint8_t opcode,
					const void *pdu, uint16_t length,
					void *user_data)
{
	struct bt_gatt_server *server = user_data;
	struct exec_data *data;
	uint8_t flags;
	uint8_t ecode;
	bool write;
	uint16_t ehandle = 0;

	if (length != 1) {
		ecode = BT_ATT_ERROR_INVALID_PDU;
		goto error;
	}

	flags = ((uint8_t *) pdu)[0];

	util_debug(server->debug_callback, server->debug_data,
				"Exec Write Req - flags: 0x%02x", flags);

	if (flags == 0x00)
		write = false;
	else if (flags == 0x01)
		write = true;
	else {
		ecode = BT_ATT_ERROR_INVALID_PDU;
		goto error;
	}

	if (!write) {
		queue_remove_all(server->prep_queue, NULL, NULL,
						prep_write_data_destroy);
		bt_att_chan_send_rsp(chan, BT_ATT_OP_EXEC_WRITE_RSP, NULL, 0);
		return;
	}

	/* If there is more than one prep request, we are in reliable session */
	if (queue_length(server->prep_queue) > 1) {
		struct prep_write_data *prep_data;

		prep_data = queue_find(server->prep_queue,
					find_no_reliable_characteristic, NULL);
		if (prep_data) {
			ecode = BT_ATT_ERROR_REQUEST_NOT_SUPPORTED;
			ehandle = prep_data->handle;
			goto error;
		}
	}

	data = new0(struct exec_data, 1);
	data->chan = chan;
	data->server = server;

	exec_next_prep_write(data, 0, 0);

	return;

error:
	queue_remove_all(server->prep_queue, NULL, NULL,
						prep_write_data_destroy);
	bt_att_chan_send_error_rsp(chan, opcode, ehandle, ecode);
}

static void exchange_mtu_cb(struct bt_att_chan *chan, uint8_t opcode,
				const void *pdu, uint16_t length,
				void *user_data)
{
	struct bt_gatt_server *server = user_data;
	uint16_t client_rx_mtu;
	uint16_t final_mtu;
	uint8_t rsp_pdu[2];

	if (length != 2) {
		bt_att_chan_send_error_rsp(chan, opcode, 0,
						BT_ATT_ERROR_INVALID_PDU);
		return;
	}

	client_rx_mtu = get_le16(pdu);
	final_mtu = MAX(MIN(client_rx_mtu, server->mtu), BT_ATT_DEFAULT_LE_MTU);

	/* Respond with the server MTU */
	put_le16(server->mtu, rsp_pdu);
	bt_att_chan_send_rsp(chan, BT_ATT_OP_MTU_RSP, rsp_pdu, 2);

	/* Set MTU to be the minimum */
	server->mtu = final_mtu;
	bt_att_set_mtu(server->att, final_mtu);

	util_debug(server->debug_callback, server->debug_data,
			"MTU exchange complete, with MTU: %u", final_mtu);
}

static bool gatt_server_register_att_handlers(struct bt_gatt_server *server)
{
	/* Exchange MTU */
	server->mtu_id = bt_att_register(server->att, BT_ATT_OP_MTU_REQ,
								exchange_mtu_cb,
								server, NULL);
	if (!server->mtu_id)
		return false;

	/* Read By Group Type */
	server->read_by_grp_type_id = bt_att_register(server->att,
						BT_ATT_OP_READ_BY_GRP_TYPE_REQ,
						read_by_grp_type_cb,
						server, NULL);
	if (!server->read_by_grp_type_id)
		return false;

	/* Read By Type */
	server->read_by_type_id = bt_att_register(server->att,
						BT_ATT_OP_READ_BY_TYPE_REQ,
						read_by_type_cb,
						server, NULL);
	if (!server->read_by_type_id)
		return false;

	/* Find Information */
	server->find_info_id = bt_att_register(server->att,
							BT_ATT_OP_FIND_INFO_REQ,
							find_info_cb,
							server, NULL);
	if (!server->find_info_id)
		return false;

	/* Find By Type Value */
	server->find_by_type_value_id = bt_att_register(server->att,
						BT_ATT_OP_FIND_BY_TYPE_REQ,
						find_by_type_val_cb,
						server, NULL);

	if (!server->find_by_type_value_id)
		return false;

	/* Write Request */
	server->write_id = bt_att_register(server->att, BT_ATT_OP_WRITE_REQ,
								write_cb,
								server, NULL);
	if (!server->write_id)
		return false;

	/* Write Command */
	server->write_cmd_id = bt_att_register(server->att, BT_ATT_OP_WRITE_CMD,
								write_cb,
								server, NULL);
	if (!server->write_cmd_id)
		return false;

	/* Read Request */
	server->read_id = bt_att_register(server->att, BT_ATT_OP_READ_REQ,
								read_cb,
								server, NULL);
	if (!server->read_id)
		return false;

	/* Read Blob Request */
	server->read_blob_id = bt_att_register(server->att,
							BT_ATT_OP_READ_BLOB_REQ,
							read_blob_cb,
							server, NULL);
	if (!server->read_blob_id)
		return false;

	/* Read Multiple Request */
	server->read_multiple_id = bt_att_register(server->att,
							BT_ATT_OP_READ_MULT_REQ,
							read_multiple_cb,
							server, NULL);

	if (!server->read_multiple_id)
		return false;

	/* Read Multiple Variable Length Request */
	server->read_multiple_vl_id = bt_att_register(server->att,
						BT_ATT_OP_READ_MULT_VL_REQ,
						read_multiple_cb,
						server, NULL);

	if (!server->read_multiple_vl_id)
		return false;

	/* Prepare Write Request */
	server->prep_write_id = bt_att_register(server->att,
						BT_ATT_OP_PREP_WRITE_REQ,
						prep_write_cb, server, NULL);
	if (!server->prep_write_id)
		return false;

	/* Execute Write Request */
	server->exec_write_id = bt_att_register(server->att,
						BT_ATT_OP_EXEC_WRITE_REQ,
						exec_write_cb, server, NULL);
	if (!server->exec_write_id)
		return NULL;

	return true;
}

struct bt_gatt_server *bt_gatt_server_new(struct gatt_db *db,
					struct bt_att *att, uint16_t mtu,
					uint8_t min_enc_size)
{
	struct bt_gatt_server *server;

	if (!att || !db)
		return NULL;

	server = new0(struct bt_gatt_server, 1);
	server->db = gatt_db_ref(db);
	server->att = bt_att_ref(att);
	server->mtu = MAX(mtu, BT_ATT_DEFAULT_LE_MTU);
	server->max_prep_queue_len = DEFAULT_MAX_PREP_QUEUE_LEN;
	server->prep_queue = queue_new();
	server->min_enc_size = min_enc_size;

	if (!gatt_server_register_att_handlers(server)) {
		bt_gatt_server_free(server);
		return NULL;
	}

	return bt_gatt_server_ref(server);
}

uint16_t bt_gatt_server_get_mtu(struct bt_gatt_server *server)
{
	if (!server || !server->att)
		return 0;

	return bt_att_get_mtu(server->att);
}

struct bt_att *bt_gatt_server_get_att(struct bt_gatt_server *server)
{
	if (!server)
		return NULL;

	return server->att;
}

struct bt_gatt_server *bt_gatt_server_ref(struct bt_gatt_server *server)
{
	if (!server)
		return NULL;

	__sync_fetch_and_add(&server->ref_count, 1);

	return server;
}

void bt_gatt_server_unref(struct bt_gatt_server *server)
{
	if (!server)
		return;

	if (__sync_sub_and_fetch(&server->ref_count, 1))
		return;

	bt_gatt_server_free(server);
}

bool bt_gatt_server_set_debug(struct bt_gatt_server *server,
					bt_gatt_server_debug_func_t callback,
					void *user_data,
					bt_gatt_server_destroy_func_t destroy)
{
	if (!server)
		return false;

	if (server->debug_destroy)
		server->debug_destroy(server->debug_data);

	server->debug_callback = callback;
	server->debug_destroy = destroy;
	server->debug_data = user_data;

	return true;
}

static bool notify_multiple(void *user_data)
{
	struct bt_gatt_server *server = user_data;

	bt_att_send(server->att, BT_ATT_OP_HANDLE_NFY_MULT,
			server->nfy_mult->pdu, server->nfy_mult->offset, NULL,
			NULL, NULL);

	free(server->nfy_mult->pdu);
	free(server->nfy_mult);
	server->nfy_mult = NULL;

	return false;
}

bool bt_gatt_server_send_notification(struct bt_gatt_server *server,
					uint16_t handle, const uint8_t *value,
					uint16_t length, bool multiple)
{
	struct nfy_mult_data *data = NULL;
	bool result;

	if (!server || (length && !value))
		return false;

	if (multiple)
		data = server->nfy_mult;

	if (!data) {
		data = new0(struct nfy_mult_data, 1);
		data->len = bt_att_get_mtu(server->att) - 1;
		data->pdu = malloc(data->len);
	}

	put_le16(handle, data->pdu + data->offset);
	data->offset += 2;

	length = MIN(data->len - data->offset, length);

	if (multiple) {
		put_le16(length, data->pdu + data->offset);
		data->offset += 2;
	}

	memcpy(data->pdu + data->offset, value, length);
	data->offset += length;

	if (multiple) {
		if (!server->nfy_mult)
			server->nfy_mult = data;

		if (!server->nfy_mult->id)
			server->nfy_mult->id = timeout_add(NFY_MULT_TIMEOUT,
						   notify_multiple, server,
						   NULL);

		return true;
	}

	result = !!bt_att_send(server->att, BT_ATT_OP_HANDLE_NFY,
				data->pdu, data->offset, NULL, NULL, NULL);
	free(data->pdu);
	free(data);

	return result;
}

struct ind_data {
	bt_gatt_server_conf_func_t callback;
	bt_gatt_server_destroy_func_t destroy;
	void *user_data;
};

static void destroy_ind_data(void *user_data)
{
	struct ind_data *data = user_data;

	if (data->destroy)
		data->destroy(data->user_data);

	free(data);
}

static void conf_cb(uint8_t opcode, const void *pdu,
					uint16_t length, void *user_data)
{
	struct ind_data *data = user_data;

	if (data->callback)
		data->callback(data->user_data);
}

bool bt_gatt_server_send_indication(struct bt_gatt_server *server,
					uint16_t handle, const uint8_t *value,
					uint16_t length,
					bt_gatt_server_conf_func_t callback,
					void *user_data,
					bt_gatt_server_destroy_func_t destroy)
{
	uint16_t pdu_len;
	uint8_t *pdu;
	struct ind_data *data;
	bool result;

	if (!server || (length && !value))
		return false;

	pdu_len = MIN(bt_att_get_mtu(server->att) - 1, length + 2);
	pdu = malloc(pdu_len);
	if (!pdu)
		return false;

	data = new0(struct ind_data, 1);

	data->callback = callback;
	data->destroy = destroy;
	data->user_data = user_data;

	put_le16(handle, pdu);
	memcpy(pdu + 2, value, pdu_len - 2);

	result = !!bt_att_send(server->att, BT_ATT_OP_HANDLE_IND, pdu,
							pdu_len, conf_cb,
							data, destroy_ind_data);
	if (!result)
		destroy_ind_data(data);

	free(pdu);

	return result;
}

bool bt_gatt_server_set_authorize(struct bt_gatt_server *server,
					bt_gatt_server_authorize_cb_t cb,
					void *user_data)
{
	if (!server)
		return false;

	server->authorize = cb;
	server->authorize_data = user_data;

	return true;
}
