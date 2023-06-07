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

#include "src/shared/queue.h"
#include "src/shared/att.h"
#include "lib/bluetooth.h"
#include "lib/uuid.h"
#include "src/shared/gatt-helpers.h"
#include "src/shared/util.h"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

struct bt_gatt_result {
	uint8_t opcode;
	void *pdu;
	uint16_t pdu_len;
	uint16_t data_len;

	void *op;  /* Discovery operation data */

	struct bt_gatt_result *next;
};

static struct bt_gatt_result *result_create(uint8_t opcode, const void *pdu,
							uint16_t pdu_len,
							uint16_t data_len,
							void *op)
{
	struct bt_gatt_result *result;

	result = new0(struct bt_gatt_result, 1);
	result->pdu = malloc(pdu_len);
	if (!result->pdu) {
		free(result);
		return NULL;
	}

	result->opcode = opcode;
	result->pdu_len = pdu_len;
	result->data_len = data_len;
	result->op = op;

	memcpy(result->pdu, pdu, pdu_len);

	return result;
}

static void result_destroy(struct bt_gatt_result *result)
{
	struct bt_gatt_result *next;

	while (result) {
		next = result->next;

		free(result->pdu);
		free(result);

		result = next;
	}
}

static unsigned int result_element_count(struct bt_gatt_result *result)
{
	unsigned int count = 0;
	struct bt_gatt_result *cur;

	cur = result;

	while (cur) {
		count += cur->pdu_len / cur->data_len;
		cur = cur->next;
	}

	return count;
}

unsigned int bt_gatt_result_service_count(struct bt_gatt_result *result)
{
	if (!result)
		return 0;

	if (result->opcode != BT_ATT_OP_READ_BY_GRP_TYPE_RSP &&
			result->opcode != BT_ATT_OP_FIND_BY_TYPE_RSP)
		return 0;

	return result_element_count(result);
}

unsigned int bt_gatt_result_characteristic_count(struct bt_gatt_result *result)
{
	if (!result)
		return 0;

	if (result->opcode != BT_ATT_OP_READ_BY_TYPE_RSP)
		return 0;

	/*
	 * Data length contains 7 or 21 octets:
	 * 2 octets: Attribute handle
	 * 1 octet: Characteristic properties
	 * 2 octets: Characteristic value handle
	 * 2 or 16 octets: characteristic UUID
	 */
	if (result->data_len != 21 && result->data_len != 7)
		return 0;

	return result_element_count(result);
}

unsigned int bt_gatt_result_descriptor_count(struct bt_gatt_result *result)
{
	if (!result)
		return 0;

	if (result->opcode != BT_ATT_OP_FIND_INFO_RSP)
		return 0;

	return result_element_count(result);
}

unsigned int bt_gatt_result_included_count(struct bt_gatt_result *result)
{
	struct bt_gatt_result *cur;
	unsigned int count = 0;

	if (!result)
		return 0;

	if (result->opcode != BT_ATT_OP_READ_BY_TYPE_RSP)
		return 0;

	/*
	 * Data length can be of length 6 or 8 octets:
	 * 2 octets - include service handle
	 * 2 octets - start handle of included service
	 * 2 octets - end handle of included service
	 * 2 octets (optionally) - 16 bit Bluetooth UUID
	 */
	if (result->data_len != 6 && result->data_len != 8)
		return 0;

	for (cur = result; cur; cur = cur->next)
		if (cur->opcode == BT_ATT_OP_READ_BY_TYPE_RSP)
			count += cur->pdu_len / cur->data_len;

	return count;
}

bool bt_gatt_iter_init(struct bt_gatt_iter *iter, struct bt_gatt_result *result)
{
	if (!iter || !result)
		return false;

	iter->result = result;
	iter->pos = 0;

	return true;
}

static const uint8_t bt_base_uuid[16] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
	0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB
};

static bool convert_uuid_le(const uint8_t *src, size_t len, uint8_t dst[16])
{
	if (len == 16) {
		bswap_128(src, dst);
		return true;
	}

	if (len != 2)
		return false;

	memcpy(dst, bt_base_uuid, sizeof(bt_base_uuid));
	dst[2] = src[1];
	dst[3] = src[0];

	return true;
}

struct bt_gatt_request {
	struct bt_att *att;
	unsigned int id;
	uint16_t start_handle;
	uint16_t end_handle;
	int ref_count;
	bt_uuid_t uuid;
	uint16_t service_type;
	struct bt_gatt_result *result_head;
	struct bt_gatt_result *result_tail;
	bt_gatt_request_callback_t callback;
	void *user_data;
	bt_gatt_destroy_func_t destroy;
};

static struct bt_gatt_result *result_append(uint8_t opcode, const void *pdu,
						uint16_t pdu_len,
						uint16_t data_len,
						struct bt_gatt_request *op)
{
	struct bt_gatt_result *result;

	result = result_create(opcode, pdu, pdu_len, data_len, op);
	if (!result)
		return NULL;

	if (!op->result_head)
		op->result_head = op->result_tail = result;
	else {
		op->result_tail->next = result;
		op->result_tail = result;
	}

	return result;
}

bool bt_gatt_iter_next_included_service(struct bt_gatt_iter *iter,
				uint16_t *handle, uint16_t *start_handle,
				uint16_t *end_handle, uint8_t uuid[16])
{
	struct bt_gatt_result *read_result;
	struct bt_gatt_request *op;
	const void *pdu_ptr;
	int i = 0;

	if (!iter || !iter->result || !handle || !start_handle || !end_handle
								|| !uuid)
		return false;


	if (iter->result->opcode != BT_ATT_OP_READ_BY_TYPE_RSP)
		return false;

	/* UUID in discovery_op is set in read_by_type and service_discovery */
	op = iter->result->op;
	if (op->uuid.type != BT_UUID_UNSPEC)
		return false;
	/*
	 * iter->result points to READ_BY_TYPE_RSP with data length containing:
	 * 2 octets - include service handle
	 * 2 octets - start handle of included service
	 * 2 octets - end handle of included service
	 * optional 2  octets - Bluetooth UUID
	 */
	if (iter->result->data_len != 8 && iter->result->data_len != 6)
		return false;

	pdu_ptr = iter->result->pdu + iter->pos;

	/* This result contains 16 bit UUID */
	if (iter->result->data_len == 8) {
		*handle = get_le16(pdu_ptr);
		*start_handle = get_le16(pdu_ptr + 2);
		*end_handle = get_le16(pdu_ptr + 4);
		convert_uuid_le(pdu_ptr + 6, 2, uuid);

		iter->pos += iter->result->data_len;

		if (iter->pos == iter->result->pdu_len) {
			iter->result = iter->result->next;
			iter->pos = 0;
		}

		return true;
	}

	*handle = get_le16(pdu_ptr);
	*start_handle = get_le16(pdu_ptr + 2);
	*end_handle = get_le16(pdu_ptr + 4);
	read_result = iter->result;

	/*
	 * Find READ_RSP with include service UUID.
	 * If number of current data set in READ_BY_TYPE_RSP is n, then we must
	 * go to n'th PDU next to current item->result
	 */
	for (read_result = read_result->next; read_result; i++) {
		if (i >= (iter->pos / iter->result->data_len))
			break;

		read_result = read_result->next;
	}

	if (!read_result)
		return false;

	convert_uuid_le(read_result->pdu, read_result->data_len, uuid);
	iter->pos += iter->result->data_len;
	if (iter->pos == iter->result->pdu_len) {
		iter->result = read_result->next;
		iter->pos = 0;
	}

	return true;
}

bool bt_gatt_iter_next_service(struct bt_gatt_iter *iter,
				uint16_t *start_handle, uint16_t *end_handle,
				uint8_t uuid[16])
{
	struct bt_gatt_request *op;
	const void *pdu_ptr;
	bt_uuid_t tmp;

	if (!iter || !iter->result || !start_handle || !end_handle || !uuid)
		return false;

	op = iter->result->op;
	pdu_ptr = iter->result->pdu + iter->pos;

	switch (iter->result->opcode) {
	case BT_ATT_OP_READ_BY_GRP_TYPE_RSP:
		*start_handle = get_le16(pdu_ptr);
		*end_handle = get_le16(pdu_ptr + 2);
		convert_uuid_le(pdu_ptr + 4, iter->result->data_len - 4, uuid);
		break;
	case BT_ATT_OP_FIND_BY_TYPE_RSP:
		*start_handle = get_le16(pdu_ptr);
		*end_handle = get_le16(pdu_ptr + 2);

		bt_uuid_to_uuid128(&op->uuid, &tmp);
		memcpy(uuid, tmp.value.u128.data, 16);
		break;
	default:
		return false;
	}


	iter->pos += iter->result->data_len;
	if (iter->pos == iter->result->pdu_len) {
		iter->result = iter->result->next;
		iter->pos = 0;
	}

	return true;
}

bool bt_gatt_iter_next_characteristic(struct bt_gatt_iter *iter,
				uint16_t *start_handle, uint16_t *end_handle,
				uint16_t *value_handle, uint8_t *properties,
				uint8_t uuid[16])
{
	struct bt_gatt_request *op;
	const void *pdu_ptr;

	if (!iter || !iter->result || !start_handle || !end_handle ||
					!value_handle || !properties || !uuid)
		return false;

	if (iter->result->opcode != BT_ATT_OP_READ_BY_TYPE_RSP)
		return false;

	/* UUID in discovery_op is set in read_by_type and service_discovery */
	op = iter->result->op;
	if (op->uuid.type != BT_UUID_UNSPEC)
		return false;
	/*
	 * Data length contains 7 or 21 octets:
	 * 2 octets: Attribute handle
	 * 1 octet: Characteristic properties
	 * 2 octets: Characteristic value handle
	 * 2 or 16 octets: characteristic UUID
	 */
	if (iter->result->data_len != 21 && iter->result->data_len != 7)
		return false;

	pdu_ptr = iter->result->pdu + iter->pos;

	*start_handle = get_le16(pdu_ptr);
	*properties = ((uint8_t *) pdu_ptr)[2];
	*value_handle = get_le16(pdu_ptr + 3);
	convert_uuid_le(pdu_ptr + 5, iter->result->data_len - 5, uuid);

	iter->pos += iter->result->data_len;
	if (iter->pos == iter->result->pdu_len) {
		iter->result = iter->result->next;
		iter->pos = 0;
	}

	if (!iter->result) {
		*end_handle = op->end_handle;
		return true;
	}

	*end_handle = get_le16(iter->result->pdu + iter->pos) - 1;

	return true;
}

bool bt_gatt_iter_next_descriptor(struct bt_gatt_iter *iter, uint16_t *handle,
							uint8_t uuid[16])
{
	const void *pdu_ptr;

	if (!iter || !iter->result || !handle || !uuid)
		return false;

	if (iter->result->opcode != BT_ATT_OP_FIND_INFO_RSP)
		return false;

	pdu_ptr = iter->result->pdu + iter->pos;

	*handle = get_le16(pdu_ptr);
	convert_uuid_le(pdu_ptr + 2, iter->result->data_len - 2, uuid);

	iter->pos += iter->result->data_len;
	if (iter->pos == iter->result->pdu_len) {
		iter->result = iter->result->next;
		iter->pos = 0;
	}

	return true;
}

bool bt_gatt_iter_next_read_by_type(struct bt_gatt_iter *iter,
				uint16_t *handle, uint16_t *length,
				const uint8_t **value)
{
	struct bt_gatt_request *op;
	const void *pdu_ptr;

	if (!iter || !iter->result || !handle || !length || !value)
		return false;

	if (iter->result->opcode != BT_ATT_OP_READ_BY_TYPE_RSP)
		return false;

	/*
	 * Check if UUID is set, otherwise results can contain characteristic
	 * discovery service or included service discovery results
	 */
	op = iter->result->op;
	if (op->uuid.type == BT_UUID_UNSPEC)
		return false;

	pdu_ptr = iter->result->pdu + iter->pos;

	*handle = get_le16(pdu_ptr);
	*length = iter->result->data_len - 2;
	*value = pdu_ptr + 2;

	iter->pos += iter->result->data_len;
	if (iter->pos == iter->result->pdu_len) {
		iter->result = iter->result->next;
		iter->pos = 0;
	}

	return true;
}

struct mtu_op {
	struct bt_att *att;
	uint16_t client_rx_mtu;
	bt_gatt_result_callback_t callback;
	void *user_data;
	bt_gatt_destroy_func_t destroy;
};

static void destroy_mtu_op(void *user_data)
{
	struct mtu_op *op = user_data;

	if (op->destroy)
		op->destroy(op->user_data);

	free(op);
}

static uint8_t process_error(const void *pdu, uint16_t length)
{
	const struct bt_att_pdu_error_rsp *error_pdu;

	if (!pdu || length != sizeof(struct bt_att_pdu_error_rsp))
		return 0;

	error_pdu = pdu;

	return error_pdu->ecode;
}

static void mtu_cb(uint8_t opcode, const void *pdu, uint16_t length,
								void *user_data)
{
	struct mtu_op *op = user_data;
	bool success = true;
	uint8_t att_ecode = 0;
	uint16_t server_rx_mtu;

	if (opcode == BT_ATT_OP_ERROR_RSP) {
		success = false;
		att_ecode = process_error(pdu, length);
		goto done;
	}

	if (opcode != BT_ATT_OP_MTU_RSP || !pdu || length != 2) {
		success = false;
		goto done;
	}

	server_rx_mtu = get_le16(pdu);
	bt_att_set_mtu(op->att, MIN(op->client_rx_mtu, server_rx_mtu));

done:
	if (op->callback)
		op->callback(success, att_ecode, op->user_data);
}

unsigned int bt_gatt_exchange_mtu(struct bt_att *att, uint16_t client_rx_mtu,
					bt_gatt_result_callback_t callback,
					void *user_data,
					bt_gatt_destroy_func_t destroy)
{
	struct mtu_op *op;
	uint8_t pdu[2];
	unsigned int id;

	if (!att || !client_rx_mtu)
		return false;

	op = new0(struct mtu_op, 1);
	op->att = att;
	op->client_rx_mtu = client_rx_mtu;
	op->callback = callback;
	op->user_data = user_data;
	op->destroy = destroy;

	put_le16(client_rx_mtu, pdu);

	id = bt_att_send(att, BT_ATT_OP_MTU_REQ, pdu, sizeof(pdu), mtu_cb, op,
								destroy_mtu_op);
	if (!id)
		free(op);

	return id;
}

static inline int get_uuid_len(const bt_uuid_t *uuid)
{
	if (!uuid)
		return 0;

	return (uuid->type == BT_UUID16) ? 2 : 16;
}

struct bt_gatt_request *bt_gatt_request_ref(struct bt_gatt_request *req)
{
	if (!req)
		return NULL;

	__sync_fetch_and_add(&req->ref_count, 1);

	return req;
}

void bt_gatt_request_unref(struct bt_gatt_request *req)
{
	if (!req)
		return;

	if (__sync_sub_and_fetch(&req->ref_count, 1))
		return;

	bt_gatt_request_cancel(req);

	if (req->destroy)
		req->destroy(req->user_data);

	result_destroy(req->result_head);

	free(req);
}

void bt_gatt_request_cancel(struct bt_gatt_request *req)
{
	if (!req)
		return;

	if (!req->id)
		return;

	bt_att_cancel(req->att, req->id);
	req->id = 0;
}

static void async_req_unref(void *data)
{
	struct bt_gatt_request *req = data;

	bt_gatt_request_unref(req);
}

static void discovery_op_complete(struct bt_gatt_request *op, bool success,
								uint8_t ecode)
{
	/* Reset success if there is some result to report */
	if (ecode == BT_ATT_ERROR_ATTRIBUTE_NOT_FOUND && op->result_head)
		success = true;

	if (op->callback)
		op->callback(success, ecode, success ? op->result_head : NULL,
								op->user_data);

	if (!op->id)
		async_req_unref(op);
	else
		op->id = 0;

}

static void read_by_grp_type_cb(uint8_t opcode, const void *pdu,
					uint16_t length, void *user_data)
{
	struct bt_gatt_request *op = user_data;
	bool success;
	uint8_t att_ecode = 0;
	struct bt_gatt_result *cur_result;
	size_t data_length;
	size_t list_length;
	uint16_t last_end;

	if (opcode == BT_ATT_OP_ERROR_RSP) {
		success = false;
		att_ecode = process_error(pdu, length);
		goto done;
	}

	/* PDU must contain at least the following (sans opcode):
	 * - Attr Data Length (1 octet)
	 * - Attr Data List (at least 6 octets):
	 *   -- 2 octets: Attribute handle
	 *   -- 2 octets: End group handle
	 *   -- 2 or 16 octets: service UUID
	 */
	if (opcode != BT_ATT_OP_READ_BY_GRP_TYPE_RSP || !pdu || length < 7) {
		success = false;
		goto done;
	}

	data_length = ((uint8_t *) pdu)[0];
	list_length = length - 1;

	if ((data_length != 6 && data_length != 20) ||
					(list_length % data_length)) {
		success = false;
		goto done;
	}

	/* PDU is correctly formatted. Get the last end handle to process the
	 * next request and store the PDU.
	 */
	cur_result = result_append(opcode, pdu + 1, list_length, data_length,
									op);
	if (!cur_result) {
		success = false;
		goto done;
	}

	last_end = get_le16(pdu + length - data_length + 2);

	/*
	 * If last handle is lower from previous start handle then it is smth
	 * wrong. Let's stop search, otherwise we might enter infinite loop.
	 */
	if (last_end < op->start_handle) {
		success = false;
		goto done;
	}

	op->start_handle = last_end + 1;

	if (last_end < op->end_handle) {
		uint8_t pdu[6];

		put_le16(op->start_handle, pdu);
		put_le16(op->end_handle, pdu + 2);
		put_le16(op->service_type, pdu + 4);

		op->id = bt_att_send(op->att, BT_ATT_OP_READ_BY_GRP_TYPE_REQ,
						pdu, sizeof(pdu),
						read_by_grp_type_cb,
						bt_gatt_request_ref(op),
						async_req_unref);
		if (op->id)
			return;

		success = false;
		goto done;
	}

	/* Some devices incorrectly return 0xffff as the end group handle when
	 * the read-by-group-type request is performed within a smaller range.
	 * Manually set the end group handle that we report in the result to the
	 * end handle in the original request.
	 */
	if (last_end == 0xffff && last_end != op->end_handle)
		put_le16(op->end_handle,
				cur_result->pdu + length - data_length + 1);

	success = true;

done:
	discovery_op_complete(op, success, att_ecode);
}

static void find_by_type_val_cb(uint8_t opcode, const void *pdu,
					uint16_t length, void *user_data)
{
	struct bt_gatt_request *op = user_data;
	bool success;
	uint8_t att_ecode = 0;
	uint16_t last_end;

	if (opcode == BT_ATT_OP_ERROR_RSP) {
		success = false;
		att_ecode = process_error(pdu, length);
		goto done;
	}

	/* PDU must contain 4 bytes and it must be a multiple of 4, where each
	 * 4 bytes contain the 16-bit attribute and group end handles.
	 */
	if (opcode != BT_ATT_OP_FIND_BY_TYPE_RSP || !pdu || !length ||
								length % 4) {
		success = false;
		goto done;
	}

	if (!result_append(opcode, pdu, length, 4, op)) {
		success = false;
		goto done;
	}

	/*
	 * Each data set contains:
	 * 2 octets with start handle
	 * 2 octets with end handle
	 * last_end is end handle of last data set
	 */
	last_end = get_le16(pdu + length - 2);

	/*
	* If last handle is lower from previous start handle then it is smth
	* wrong. Let's stop search, otherwise we might enter infinite loop.
	*/
	if (last_end < op->start_handle) {
		success = false;
		goto done;
	}

	op->start_handle = last_end + 1;

	if (last_end < op->end_handle) {
		uint8_t pdu[6 + get_uuid_len(&op->uuid)];

		put_le16(op->start_handle, pdu);
		put_le16(op->end_handle, pdu + 2);
		put_le16(op->service_type, pdu + 4);
		bt_uuid_to_le(&op->uuid, pdu + 6);

		op->id = bt_att_send(op->att, BT_ATT_OP_FIND_BY_TYPE_REQ,
						pdu, sizeof(pdu),
						find_by_type_val_cb,
						bt_gatt_request_ref(op),
						async_req_unref);
		if (op->id)
			return;

		success = false;
		goto done;
	}

	success = true;

done:
	discovery_op_complete(op, success, att_ecode);
}

static struct bt_gatt_request *discover_services(struct bt_att *att,
					bt_uuid_t *uuid,
					uint16_t start, uint16_t end,
					bt_gatt_request_callback_t callback,
					void *user_data,
					bt_gatt_destroy_func_t destroy,
					bool primary)
{
	struct bt_gatt_request *op;

	if (!att)
		return NULL;

	op = new0(struct bt_gatt_request, 1);
	op->att = att;
	op->start_handle = start;
	op->end_handle = end;
	op->callback = callback;
	op->user_data = user_data;
	op->destroy = destroy;
	/* set service uuid to primary or secondary */
	op->service_type = primary ? GATT_PRIM_SVC_UUID : GATT_SND_SVC_UUID;

	/* If UUID is NULL, then discover all primary services */
	if (!uuid) {
		uint8_t pdu[6];

		put_le16(start, pdu);
		put_le16(end, pdu + 2);
		put_le16(op->service_type, pdu + 4);

		op->id = bt_att_send(att, BT_ATT_OP_READ_BY_GRP_TYPE_REQ,
						pdu, sizeof(pdu),
						read_by_grp_type_cb,
						bt_gatt_request_ref(op),
						async_req_unref);
	} else {
		uint8_t pdu[6 + get_uuid_len(uuid)];

		if (uuid->type == BT_UUID_UNSPEC) {
			free(op);
			return NULL;
		}

		/* Discover by UUID */
		op->uuid = *uuid;

		put_le16(start, pdu);
		put_le16(end, pdu + 2);
		put_le16(op->service_type, pdu + 4);
		bt_uuid_to_le(&op->uuid, pdu + 6);

		op->id = bt_att_send(att, BT_ATT_OP_FIND_BY_TYPE_REQ,
						pdu, sizeof(pdu),
						find_by_type_val_cb,
						bt_gatt_request_ref(op),
						async_req_unref);
	}

	if (!op->id) {
		free(op);
		return NULL;
	}

	return bt_gatt_request_ref(op);
}

struct bt_gatt_request *bt_gatt_discover_all_primary_services(
					struct bt_att *att, bt_uuid_t *uuid,
					bt_gatt_request_callback_t callback,
					void *user_data,
					bt_gatt_destroy_func_t destroy)
{
	return bt_gatt_discover_primary_services(att, uuid, 0x0001, 0xffff,
							callback, user_data,
							destroy);
}

struct bt_gatt_request *bt_gatt_discover_primary_services(
					struct bt_att *att, bt_uuid_t *uuid,
					uint16_t start, uint16_t end,
					bt_gatt_request_callback_t callback,
					void *user_data,
					bt_gatt_destroy_func_t destroy)
{
	return discover_services(att, uuid, start, end, callback, user_data,
								destroy, true);
}

struct bt_gatt_request *bt_gatt_discover_secondary_services(
					struct bt_att *att, bt_uuid_t *uuid,
					uint16_t start, uint16_t end,
					bt_gatt_request_callback_t callback,
					void *user_data,
					bt_gatt_destroy_func_t destroy)
{
	return discover_services(att, uuid, start, end, callback, user_data,
								destroy, false);
}

struct read_incl_data {
	struct bt_gatt_request *op;
	struct bt_gatt_result *result;
	int pos;
	int ref_count;
};

static struct read_incl_data *new_read_included(struct bt_gatt_result *res)
{
	struct read_incl_data *data;

	data = new0(struct read_incl_data, 1);
	data->op = bt_gatt_request_ref(res->op);
	data->result = res;

	return data;
};

static struct read_incl_data *read_included_ref(struct read_incl_data *data)
{
	__sync_fetch_and_add(&data->ref_count, 1);

	return data;
}

static void read_included_unref(void *data)
{
	struct read_incl_data *read_data = data;

	if (__sync_sub_and_fetch(&read_data->ref_count, 1))
		return;

	async_req_unref(read_data->op);

	free(read_data);
}

static void discover_included_cb(uint8_t opcode, const void *pdu,
					uint16_t length, void *user_data);

static void read_included_cb(uint8_t opcode, const void *pdu,
					uint16_t length, void *user_data)
{
	struct read_incl_data *data = user_data;
	struct bt_gatt_request *op = data->op;
	uint8_t att_ecode = 0;
	uint8_t read_pdu[2];
	bool success;

	if (opcode == BT_ATT_OP_ERROR_RSP) {
		success = false;
		att_ecode = process_error(pdu, length);
		goto done;
	}

	if (opcode != BT_ATT_OP_READ_RSP || (!pdu && length)) {
		success = false;
		goto done;
	}

	/*
	 * UUID should be in 128 bit format, as it couldn't be read in
	 * READ_BY_TYPE request
	 */
	if (length != 16) {
		success = false;
		goto done;
	}

	if (!result_append(opcode, pdu, length, length, op)) {
		success = false;
		goto done;
	}

	if (data->pos == data->result->pdu_len) {
		uint16_t last_handle;
		uint8_t pdu[6];

		last_handle = get_le16(data->result->pdu + data->pos -
							data->result->data_len);
		if (last_handle == op->end_handle) {
			success = true;
			goto done;
		}

		put_le16(last_handle + 1, pdu);
		put_le16(op->end_handle, pdu + 2);
		put_le16(GATT_INCLUDE_UUID, pdu + 4);

		op->id = bt_att_send(op->att, BT_ATT_OP_READ_BY_TYPE_REQ,
						pdu, sizeof(pdu),
						discover_included_cb,
						bt_gatt_request_ref(op),
						async_req_unref);
		if (op->id)
			return;

		success = false;
		goto done;
	}

	memcpy(read_pdu, data->result->pdu + data->pos + 2, sizeof(uint16_t));

	data->pos += data->result->data_len;

	if (bt_att_send(op->att, BT_ATT_OP_READ_REQ, read_pdu, sizeof(read_pdu),
				read_included_cb, read_included_ref(data),
				read_included_unref))
		return;

	read_included_unref(data);
	success = false;

done:
	discovery_op_complete(op, success, att_ecode);
}

static void read_included(struct read_incl_data *data)
{
	struct bt_gatt_request *op = data->op;
	uint8_t pdu[2];

	memcpy(pdu, data->result->pdu + 2, sizeof(uint16_t));

	data->pos += data->result->data_len;

	if (bt_att_send(op->att, BT_ATT_OP_READ_REQ, pdu, sizeof(pdu),
							read_included_cb,
							read_included_ref(data),
							read_included_unref))
		return;

	if (op->callback)
		op->callback(false, 0, NULL, data->op->user_data);

	read_included_unref(data);
}

static void discover_included_cb(uint8_t opcode, const void *pdu,
					uint16_t length, void *user_data)
{
	struct bt_gatt_request *op = user_data;
	struct bt_gatt_result *cur_result;
	uint8_t att_ecode = 0;
	uint16_t last_handle;
	size_t data_length;
	bool success;

	if (opcode == BT_ATT_OP_ERROR_RSP) {
		att_ecode = process_error(pdu, length);
		success = false;
		goto failed;
	}

	if (opcode != BT_ATT_OP_READ_BY_TYPE_RSP || !pdu || length < 6) {
		success = false;
		goto failed;
	}

	data_length = ((const uint8_t *) pdu)[0];

	/*
	 * Check if PDU contains data sets with length declared in the beginning
	 * of frame and if this length is correct.
	 * Data set length may be 6 or 8 octets:
	 * 2 octets - include service handle
	 * 2 octets - start handle of included service
	 * 2 octets - end handle of included service
	 * optional 2 octets - Bluetooth UUID of included service
	 */
	if ((data_length != 8 && data_length != 6) ||
						(length - 1) % data_length) {
		success = false;
		goto failed;
	}

	cur_result = result_append(opcode, pdu + 1, length - 1, data_length,
									op);
	if (!cur_result) {
		success = false;
		goto failed;
	}

	if (data_length == 6) {
		struct read_incl_data *data;

		data = new_read_included(cur_result);
		if (!data) {
			success = false;
			goto failed;
		}

		read_included(data);
		return;
	}

	last_handle = get_le16(pdu + length - data_length);

	/*
	 * If last handle is lower from previous start handle then it is smth
	 * wrong. Let's stop search, otherwise we might enter infinite loop.
	 */
	if (last_handle < op->start_handle) {
		success = false;
		goto failed;
	}

	op->start_handle = last_handle + 1;
	if (last_handle != op->end_handle) {
		uint8_t pdu[6];

		put_le16(op->start_handle, pdu);
		put_le16(op->end_handle, pdu + 2);
		put_le16(GATT_INCLUDE_UUID, pdu + 4);

		op->id = bt_att_send(op->att, BT_ATT_OP_READ_BY_TYPE_REQ,
						pdu, sizeof(pdu),
						discover_included_cb,
						bt_gatt_request_ref(op),
						async_req_unref);
		if (op->id)
			return;

		success = false;
		goto failed;
	}

	success = true;

failed:
	discovery_op_complete(op, success, att_ecode);
}

struct bt_gatt_request *bt_gatt_discover_included_services(struct bt_att *att,
					uint16_t start, uint16_t end,
					bt_gatt_request_callback_t callback,
					void *user_data,
					bt_gatt_destroy_func_t destroy)
{
	struct bt_gatt_request *op;
	uint8_t pdu[6];

	if (!att)
		return false;

	op = new0(struct bt_gatt_request, 1);
	op->att = att;
	op->callback = callback;
	op->user_data = user_data;
	op->destroy = destroy;
	op->start_handle = start;
	op->end_handle = end;

	put_le16(start, pdu);
	put_le16(end, pdu + 2);
	put_le16(GATT_INCLUDE_UUID, pdu + 4);

	op->id = bt_att_send(att, BT_ATT_OP_READ_BY_TYPE_REQ, pdu, sizeof(pdu),
				discover_included_cb, bt_gatt_request_ref(op),
				async_req_unref);
	if (!op->id) {
		free(op);
		return NULL;
	}

	return bt_gatt_request_ref(op);
}

static void discover_chrcs_cb(uint8_t opcode, const void *pdu,
					uint16_t length, void *user_data)
{
	struct bt_gatt_request *op = user_data;
	bool success;
	uint8_t att_ecode = 0;
	size_t data_length;
	uint16_t last_handle;

	if (opcode == BT_ATT_OP_ERROR_RSP) {
		success = false;
		att_ecode = process_error(pdu, length);
		goto done;
	}

	/* PDU must contain at least the following (sans opcode):
	 * - Attr Data Length (1 octet)
	 * - Attr Data List (at least 7 octets):
	 *   -- 2 octets: Attribute handle
	 *   -- 1 octet: Characteristic properties
	 *   -- 2 octets: Characteristic value handle
	 *   -- 2 or 16 octets: characteristic UUID
	 */
	if (opcode != BT_ATT_OP_READ_BY_TYPE_RSP || !pdu || length < 8) {
		success = false;
		goto done;
	}

	data_length = ((uint8_t *) pdu)[0];

	if ((data_length != 7 && data_length != 21) ||
					((length - 1) % data_length)) {
		success = false;
		goto done;
	}

	if (!result_append(opcode, pdu + 1, length - 1,
							data_length, op)) {
		success = false;
		goto done;
	}
	last_handle = get_le16(pdu + length - data_length);

	/*
	 * If last handle is lower from previous start handle then it is smth
	 * wrong. Let's stop search, otherwise we might enter infinite loop.
	 */
	if (last_handle < op->start_handle) {
		success = false;
		goto done;
	}

	op->start_handle = last_handle + 1;

	if (last_handle != op->end_handle) {
		uint8_t pdu[6];

		put_le16(op->start_handle, pdu);
		put_le16(op->end_handle, pdu + 2);
		put_le16(GATT_CHARAC_UUID, pdu + 4);

		op->id = bt_att_send(op->att, BT_ATT_OP_READ_BY_TYPE_REQ,
						pdu, sizeof(pdu),
						discover_chrcs_cb,
						bt_gatt_request_ref(op),
						async_req_unref);
		if (op->id)
			return;

		success = false;
		goto done;
	}

	success = true;

done:
	discovery_op_complete(op, success, att_ecode);
}

struct bt_gatt_request *bt_gatt_discover_characteristics(struct bt_att *att,
					uint16_t start, uint16_t end,
					bt_gatt_request_callback_t callback,
					void *user_data,
					bt_gatt_destroy_func_t destroy)
{
	struct bt_gatt_request *op;
	uint8_t pdu[6];

	if (!att)
		return false;

	op = new0(struct bt_gatt_request, 1);
	op->att = att;
	op->callback = callback;
	op->user_data = user_data;
	op->destroy = destroy;
	op->start_handle = start;
	op->end_handle = end;

	put_le16(start, pdu);
	put_le16(end, pdu + 2);
	put_le16(GATT_CHARAC_UUID, pdu + 4);

	op->id = bt_att_send(att, BT_ATT_OP_READ_BY_TYPE_REQ, pdu, sizeof(pdu),
				discover_chrcs_cb, bt_gatt_request_ref(op),
				async_req_unref);
	if (!op->id) {
		free(op);
		return NULL;
	}

	return bt_gatt_request_ref(op);
}

static void read_by_type_cb(uint8_t opcode, const void *pdu,
					uint16_t length, void *user_data)
{
	struct bt_gatt_request *op = user_data;
	bool success;
	uint8_t att_ecode = 0;
	size_t data_length;
	uint16_t last_handle;

	if (opcode == BT_ATT_OP_ERROR_RSP) {
		att_ecode = process_error(pdu, length);
		success = false;
		goto done;
	}

	if (opcode != BT_ATT_OP_READ_BY_TYPE_RSP || !pdu) {
		success = false;
		att_ecode = 0;
		goto done;
	}

	data_length = ((uint8_t *) pdu)[0];
	if (((length - 1) % data_length)) {
		success = false;
		att_ecode = 0;
		goto done;
	}

	if (!result_append(opcode, pdu + 1, length - 1, data_length, op)) {
		success = false;
		att_ecode = 0;
		goto done;
	}

	last_handle = get_le16(pdu + length - data_length);

	/*
	 * If last handle is lower from previous start handle then it is smth
	 * wrong. Let's stop search, otherwise we might enter infinite loop.
	 */
	if (last_handle < op->start_handle) {
		success = false;
		goto done;
	}

	op->start_handle = last_handle + 1;

	if (last_handle != op->end_handle) {
		uint8_t pdu[4 + get_uuid_len(&op->uuid)];

		put_le16(op->start_handle, pdu);
		put_le16(op->end_handle, pdu + 2);
		bt_uuid_to_le(&op->uuid, pdu + 4);

		op->id = bt_att_send(op->att, BT_ATT_OP_READ_BY_TYPE_REQ,
						pdu, sizeof(pdu),
						read_by_type_cb,
						bt_gatt_request_ref(op),
						async_req_unref);
		if (op->id)
			return;

		success = false;
		goto done;
	}

	success = true;

done:
	discovery_op_complete(op, success, att_ecode);
}

bool bt_gatt_read_by_type(struct bt_att *att, uint16_t start, uint16_t end,
					const bt_uuid_t *uuid,
					bt_gatt_request_callback_t callback,
					void *user_data,
					bt_gatt_destroy_func_t destroy)
{
	struct bt_gatt_request *op;
	uint8_t pdu[4 + get_uuid_len(uuid)];

	if (!att || !uuid || uuid->type == BT_UUID_UNSPEC)
		return false;

	op = new0(struct bt_gatt_request, 1);
	op->att = att;
	op->callback = callback;
	op->user_data = user_data;
	op->destroy = destroy;
	op->start_handle = start;
	op->end_handle = end;
	op->uuid = *uuid;

	put_le16(start, pdu);
	put_le16(end, pdu + 2);
	bt_uuid_to_le(uuid, pdu + 4);

	op->id = bt_att_send(att, BT_ATT_OP_READ_BY_TYPE_REQ, pdu, sizeof(pdu),
						read_by_type_cb,
						bt_gatt_request_ref(op),
						async_req_unref);
	if (op->id)
		return true;

	free(op);
	return false;
}

static void discover_descs_cb(uint8_t opcode, const void *pdu,
					uint16_t length, void *user_data)
{
	struct bt_gatt_request *op = user_data;
	bool success;
	uint8_t att_ecode = 0;
	uint8_t format;
	uint16_t last_handle;
	size_t data_length;

	if (opcode == BT_ATT_OP_ERROR_RSP) {
		success = false;
		att_ecode = process_error(pdu, length);
		goto done;
	}

	/* The PDU should contain the following data (sans opcode):
	 * - Format (1 octet)
	 * - Attr Data List (at least 4 octets):
	 *   -- 2 octets: Attribute handle
	 *   -- 2 or 16 octets: UUID.
	 */
	if (opcode != BT_ATT_OP_FIND_INFO_RSP || !pdu || length < 5) {
		success = false;
		goto done;
	}

	format = ((uint8_t *) pdu)[0];

	if (format == 0x01)
		data_length = 4;
	else if (format == 0x02)
		data_length = 18;
	else {
		success = false;
		goto done;
	}

	if ((length - 1) % data_length) {
		success = false;
		goto done;
	}

	if (!result_append(opcode, pdu + 1, length - 1, data_length, op)) {
		success = false;
		goto done;
	}

	last_handle = get_le16(pdu + length - data_length);

	/*
	 * If last handle is lower from previous start handle then it is smth
	 * wrong. Let's stop search, otherwise we might enter infinite loop.
	 */
	if (last_handle < op->start_handle) {
		success = false;
		goto done;
	}

	op->start_handle = last_handle + 1;

	if (last_handle != op->end_handle) {
		uint8_t pdu[4];

		put_le16(op->start_handle, pdu);
		put_le16(op->end_handle, pdu + 2);

		op->id = bt_att_send(op->att, BT_ATT_OP_FIND_INFO_REQ,
						pdu, sizeof(pdu),
						discover_descs_cb,
						bt_gatt_request_ref(op),
						async_req_unref);
		if (op->id)
			return;

		success = false;
		goto done;
	}

	success = true;

done:
	discovery_op_complete(op, success, att_ecode);
}

struct bt_gatt_request *bt_gatt_discover_descriptors(struct bt_att *att,
					uint16_t start, uint16_t end,
					bt_gatt_request_callback_t callback,
					void *user_data,
					bt_gatt_destroy_func_t destroy)
{
	struct bt_gatt_request *op;
	uint8_t pdu[4];

	if (!att)
		return false;

	op = new0(struct bt_gatt_request, 1);
	op->att = att;
	op->callback = callback;
	op->user_data = user_data;
	op->destroy = destroy;
	op->start_handle = start;
	op->end_handle = end;

	put_le16(start, pdu);
	put_le16(end, pdu + 2);

	op->id = bt_att_send(att, BT_ATT_OP_FIND_INFO_REQ, pdu, sizeof(pdu),
						discover_descs_cb,
						bt_gatt_request_ref(op),
						async_req_unref);
	if (!op->id) {
		free(op);
		return NULL;
	}

	return bt_gatt_request_ref(op);
}
