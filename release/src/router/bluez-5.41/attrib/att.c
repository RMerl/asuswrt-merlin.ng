/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2010  Nokia Corporation
 *  Copyright (C) 2010  Marcel Holtmann <marcel@holtmann.org>
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
#include "config.h"
#endif

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/uuid.h"

#include "src/shared/util.h"
#include "att.h"

static inline void put_uuid_le(const bt_uuid_t *src, void *dst)
{
	if (src->type == BT_UUID16)
		put_le16(src->value.u16, dst);
	else
		/* Convert from 128-bit BE to LE */
		bswap_128(&src->value.u128, dst);
}

const char *att_ecode2str(uint8_t status)
{
	switch (status)  {
	case ATT_ECODE_INVALID_HANDLE:
		return "Invalid handle";
	case ATT_ECODE_READ_NOT_PERM:
		return "Attribute can't be read";
	case ATT_ECODE_WRITE_NOT_PERM:
		return "Attribute can't be written";
	case ATT_ECODE_INVALID_PDU:
		return "Attribute PDU was invalid";
	case ATT_ECODE_AUTHENTICATION:
		return "Attribute requires authentication before read/write";
	case ATT_ECODE_REQ_NOT_SUPP:
		return "Server doesn't support the request received";
	case ATT_ECODE_INVALID_OFFSET:
		return "Offset past the end of the attribute";
	case ATT_ECODE_AUTHORIZATION:
		return "Attribute requires authorization before read/write";
	case ATT_ECODE_PREP_QUEUE_FULL:
		return "Too many prepare writes have been queued";
	case ATT_ECODE_ATTR_NOT_FOUND:
		return "No attribute found within the given range";
	case ATT_ECODE_ATTR_NOT_LONG:
		return "Attribute can't be read/written using Read Blob Req";
	case ATT_ECODE_INSUFF_ENCR_KEY_SIZE:
		return "Encryption Key Size is insufficient";
	case ATT_ECODE_INVAL_ATTR_VALUE_LEN:
		return "Attribute value length is invalid";
	case ATT_ECODE_UNLIKELY:
		return "Request attribute has encountered an unlikely error";
	case ATT_ECODE_INSUFF_ENC:
		return "Encryption required before read/write";
	case ATT_ECODE_UNSUPP_GRP_TYPE:
		return "Attribute type is not a supported grouping attribute";
	case ATT_ECODE_INSUFF_RESOURCES:
		return "Insufficient Resources to complete the request";
	case ATT_ECODE_IO:
		return "Internal application error: I/O";
	case ATT_ECODE_TIMEOUT:
		return "A timeout occured";
	case ATT_ECODE_ABORTED:
		return "The operation was aborted";
	default:
		return "Unexpected error code";
	}
}

void att_data_list_free(struct att_data_list *list)
{
	if (list == NULL)
		return;

	if (list->data) {
		int i;
		for (i = 0; i < list->num; i++)
			g_free(list->data[i]);
	}

	g_free(list->data);
	g_free(list);
}

struct att_data_list *att_data_list_alloc(uint16_t num, uint16_t len)
{
	struct att_data_list *list;
	int i;

	if (len > UINT8_MAX)
		return NULL;

	list = g_new0(struct att_data_list, 1);
	list->len = len;
	list->num = num;

	list->data = g_malloc0(sizeof(uint8_t *) * num);

	for (i = 0; i < num; i++)
		list->data[i] = g_malloc0(sizeof(uint8_t) * len);

	return list;
}

static void get_uuid(uint8_t type, const void *val, bt_uuid_t *uuid)
{
	if (type == BT_UUID16)
		bt_uuid16_create(uuid, get_le16(val));
	else {
		uint128_t u128;

		/* Convert from 128-bit LE to BE */
		bswap_128(val, &u128);
		bt_uuid128_create(uuid, u128);
	}
}

uint16_t enc_read_by_grp_req(uint16_t start, uint16_t end, bt_uuid_t *uuid,
						uint8_t *pdu, size_t len)
{
	uint16_t uuid_len;

	if (!uuid)
		return 0;

	if (uuid->type == BT_UUID16)
		uuid_len = 2;
	else if (uuid->type == BT_UUID128)
		uuid_len = 16;
	else
		return 0;

	/* Attribute Opcode (1 octet) */
	pdu[0] = ATT_OP_READ_BY_GROUP_REQ;
	/* Starting Handle (2 octets) */
	put_le16(start, &pdu[1]);
	/* Ending Handle (2 octets) */
	put_le16(end, &pdu[3]);
	/* Attribute Group Type (2 or 16 octet UUID) */
	put_uuid_le(uuid, &pdu[5]);

	return 5 + uuid_len;
}

uint16_t dec_read_by_grp_req(const uint8_t *pdu, size_t len, uint16_t *start,
						uint16_t *end, bt_uuid_t *uuid)
{
	const size_t min_len = sizeof(pdu[0]) + sizeof(*start) + sizeof(*end);
	uint8_t type;

	if (pdu == NULL)
		return 0;

	if (start == NULL || end == NULL || uuid == NULL)
		return 0;

	if (pdu[0] != ATT_OP_READ_BY_GROUP_REQ)
		return 0;

	if (len == (min_len + 2))
		type = BT_UUID16;
	else if (len == (min_len + 16))
		type = BT_UUID128;
	else
		return 0;

	*start = get_le16(&pdu[1]);
	*end = get_le16(&pdu[3]);

	get_uuid(type, &pdu[5], uuid);

	return len;
}

uint16_t enc_read_by_grp_resp(struct att_data_list *list, uint8_t *pdu,
								size_t len)
{
	int i;
	uint16_t w;
	uint8_t *ptr;

	if (list == NULL)
		return 0;

	if (len < list->len + sizeof(uint8_t) * 2)
		return 0;

	pdu[0] = ATT_OP_READ_BY_GROUP_RESP;
	pdu[1] = list->len;

	ptr = &pdu[2];

	for (i = 0, w = 2; i < list->num && w + list->len <= len; i++) {
		memcpy(ptr, list->data[i], list->len);
		ptr += list->len;
		w += list->len;
	}

	return w;
}

struct att_data_list *dec_read_by_grp_resp(const uint8_t *pdu, size_t len)
{
	struct att_data_list *list;
	const uint8_t *ptr;
	uint16_t elen, num;
	int i;

	if (pdu[0] != ATT_OP_READ_BY_GROUP_RESP)
		return NULL;

	/* PDU must contain at least:
	 * - Attribute Opcode (1 octet)
	 * - Length (1 octet)
	 * - Attribute Data List (at least one entry):
	 *   - Attribute Handle (2 octets)
	 *   - End Group Handle (2 octets)
	 *   - Attribute Value (at least 1 octet) */
	if (len < 7)
		return NULL;

	elen = pdu[1];
	/* Minimum Attribute Data List size */
	if (elen < 5)
		return NULL;

	/* Reject incomplete Attribute Data List */
	if ((len - 2) % elen)
		return NULL;

	num = (len - 2) / elen;
	list = att_data_list_alloc(num, elen);
	if (list == NULL)
		return NULL;

	ptr = &pdu[2];

	for (i = 0; i < num; i++) {
		memcpy(list->data[i], ptr, list->len);
		ptr += list->len;
	}

	return list;
}

uint16_t enc_find_by_type_req(uint16_t start, uint16_t end, bt_uuid_t *uuid,
					const uint8_t *value, size_t vlen,
					uint8_t *pdu, size_t len)
{
	uint16_t min_len = sizeof(pdu[0]) + sizeof(start) + sizeof(end) +
							sizeof(uint16_t);

	if (pdu == NULL)
		return 0;

	if (!uuid)
		return 0;

	if (uuid->type != BT_UUID16)
		return 0;

	if (vlen > len - min_len)
		vlen = len - min_len;

	pdu[0] = ATT_OP_FIND_BY_TYPE_REQ;
	put_le16(start, &pdu[1]);
	put_le16(end, &pdu[3]);
	put_le16(uuid->value.u16, &pdu[5]);

	if (vlen > 0) {
		memcpy(&pdu[7], value, vlen);
		return min_len + vlen;
	}

	return min_len;
}

uint16_t dec_find_by_type_req(const uint8_t *pdu, size_t len, uint16_t *start,
						uint16_t *end, bt_uuid_t *uuid,
						uint8_t *value, size_t *vlen)
{
	if (pdu == NULL)
		return 0;

	if (len < 7)
		return 0;

	/* Attribute Opcode (1 octet) */
	if (pdu[0] != ATT_OP_FIND_BY_TYPE_REQ)
		return 0;

	/* First requested handle number (2 octets) */
	*start = get_le16(&pdu[1]);
	/* Last requested handle number (2 octets) */
	*end = get_le16(&pdu[3]);
	/* 16-bit UUID to find (2 octets) */
	bt_uuid16_create(uuid, get_le16(&pdu[5]));

	/* Attribute value to find */
	*vlen = len - 7;
	if (*vlen > 0)
		memcpy(value, pdu + 7, *vlen);

	return len;
}

uint16_t enc_find_by_type_resp(GSList *matches, uint8_t *pdu, size_t len)
{
	GSList *l;
	uint16_t offset;

	if (!pdu)
		return 0;

	pdu[0] = ATT_OP_FIND_BY_TYPE_RESP;

	for (l = matches, offset = 1;
				l && len >= (offset + sizeof(uint16_t) * 2);
				l = l->next, offset += sizeof(uint16_t) * 2) {
		struct att_range *range = l->data;

		put_le16(range->start, &pdu[offset]);
		put_le16(range->end, &pdu[offset + 2]);
	}

	return offset;
}

GSList *dec_find_by_type_resp(const uint8_t *pdu, size_t len)
{
	struct att_range *range;
	GSList *matches;
	off_t offset;

	/* PDU should contain at least:
	 * - Attribute Opcode (1 octet)
	 * - Handles Information List (at least one entry):
	 *   - Found Attribute Handle (2 octets)
	 *   - Group End Handle (2 octets) */
	if (pdu == NULL || len < 5)
		return NULL;

	if (pdu[0] != ATT_OP_FIND_BY_TYPE_RESP)
		return NULL;

	/* Reject incomplete Handles Information List */
	if ((len - 1) % 4)
		return NULL;

	for (offset = 1, matches = NULL;
				len >= (offset + sizeof(uint16_t) * 2);
				offset += sizeof(uint16_t) * 2) {
		range = g_new0(struct att_range, 1);
		range->start = get_le16(&pdu[offset]);
		range->end = get_le16(&pdu[offset + 2]);

		matches = g_slist_append(matches, range);
	}

	return matches;
}

uint16_t enc_read_by_type_req(uint16_t start, uint16_t end, bt_uuid_t *uuid,
						uint8_t *pdu, size_t len)
{
	uint16_t uuid_len;

	if (!uuid)
		return 0;

	if (uuid->type == BT_UUID16)
		uuid_len = 2;
	else if (uuid->type == BT_UUID128)
		uuid_len = 16;
	else
		return 0;

	/* Attribute Opcode (1 octet) */
	pdu[0] = ATT_OP_READ_BY_TYPE_REQ;
	/* Starting Handle (2 octets) */
	put_le16(start, &pdu[1]);
	/* Ending Handle (2 octets) */
	put_le16(end, &pdu[3]);
	/* Attribute Type (2 or 16 octet UUID) */
	put_uuid_le(uuid, &pdu[5]);

	return 5 + uuid_len;
}

uint16_t dec_read_by_type_req(const uint8_t *pdu, size_t len, uint16_t *start,
						uint16_t *end, bt_uuid_t *uuid)
{
	const size_t min_len = sizeof(pdu[0]) + sizeof(*start) + sizeof(*end);
	uint8_t type;

	if (pdu == NULL)
		return 0;

	if (start == NULL || end == NULL || uuid == NULL)
		return 0;

	if (len == (min_len + 2))
		type = BT_UUID16;
	else if (len == (min_len + 16))
		type = BT_UUID128;
	else
		return 0;

	if (pdu[0] != ATT_OP_READ_BY_TYPE_REQ)
		return 0;

	*start = get_le16(&pdu[1]);
	*end = get_le16(&pdu[3]);

	get_uuid(type, &pdu[5], uuid);

	return len;
}

uint16_t enc_read_by_type_resp(struct att_data_list *list, uint8_t *pdu,
								size_t len)
{
	uint8_t *ptr;
	size_t i, w, l;

	if (list == NULL)
		return 0;

	if (pdu == NULL)
		return 0;

	l = MIN(len - 2, list->len);

	pdu[0] = ATT_OP_READ_BY_TYPE_RESP;
	pdu[1] = l;
	ptr = &pdu[2];

	for (i = 0, w = 2; i < list->num && w + l <= len; i++) {
		memcpy(ptr, list->data[i], l);
		ptr += l;
		w += l;
	}

	return w;
}

struct att_data_list *dec_read_by_type_resp(const uint8_t *pdu, size_t len)
{
	struct att_data_list *list;
	const uint8_t *ptr;
	uint16_t elen, num;
	int i;

	if (pdu[0] != ATT_OP_READ_BY_TYPE_RESP)
		return NULL;

	/* PDU must contain at least:
	 * - Attribute Opcode (1 octet)
	 * - Length (1 octet)
	 * - Attribute Data List (at least one entry):
	 *   - Attribute Handle (2 octets)
	 *   - Attribute Value (at least 1 octet) */
	if (len < 5)
		return NULL;

	elen = pdu[1];
	/* Minimum Attribute Data List size */
	if (elen < 3)
		return NULL;

	/* Reject incomplete Attribute Data List */
	if ((len - 2) % elen)
		return NULL;

	num = (len - 2) / elen;
	list = att_data_list_alloc(num, elen);
	if (list == NULL)
		return NULL;

	ptr = &pdu[2];

	for (i = 0; i < num; i++) {
		memcpy(list->data[i], ptr, list->len);
		ptr += list->len;
	}

	return list;
}

uint16_t enc_write_cmd(uint16_t handle, const uint8_t *value, size_t vlen,
						uint8_t *pdu, size_t len)
{
	const uint16_t min_len = sizeof(pdu[0]) + sizeof(handle);

	if (pdu == NULL)
		return 0;

	if (vlen > len - min_len)
		vlen = len - min_len;

	pdu[0] = ATT_OP_WRITE_CMD;
	put_le16(handle, &pdu[1]);

	if (vlen > 0) {
		memcpy(&pdu[3], value, vlen);
		return min_len + vlen;
	}

	return min_len;
}

uint16_t dec_write_cmd(const uint8_t *pdu, size_t len, uint16_t *handle,
						uint8_t *value, size_t *vlen)
{
	const uint16_t min_len = sizeof(pdu[0]) + sizeof(*handle);

	if (pdu == NULL)
		return 0;

	if (value == NULL || vlen == NULL || handle == NULL)
		return 0;

	if (len < min_len)
		return 0;

	if (pdu[0] != ATT_OP_WRITE_CMD)
		return 0;

	*handle = get_le16(&pdu[1]);
	memcpy(value, pdu + min_len, len - min_len);
	*vlen = len - min_len;

	return len;
}

uint16_t enc_signed_write_cmd(uint16_t handle, const uint8_t *value,
					size_t vlen, struct bt_crypto *crypto,
					const uint8_t csrk[16],
					uint32_t sign_cnt,
					uint8_t *pdu, size_t len)
{
	const uint16_t hdr_len = sizeof(pdu[0]) + sizeof(handle);
	const uint16_t min_len =  hdr_len + ATT_SIGNATURE_LEN;

	if (pdu == NULL)
		return 0;

	if (vlen > len - min_len)
		vlen = len - min_len;

	pdu[0] = ATT_OP_SIGNED_WRITE_CMD;
	put_le16(handle, &pdu[1]);

	if (vlen > 0)
		memcpy(&pdu[hdr_len], value, vlen);

	if (!bt_crypto_sign_att(crypto, csrk, pdu, hdr_len + vlen, sign_cnt,
							&pdu[hdr_len + vlen]))
		return 0;

	return min_len + vlen;
}

uint16_t dec_signed_write_cmd(const uint8_t *pdu, size_t len,
						uint16_t *handle,
						uint8_t *value, size_t *vlen,
						uint8_t signature[12])
{
	const uint16_t hdr_len = sizeof(pdu[0]) + sizeof(*handle);
	const uint16_t min_len =  hdr_len + ATT_SIGNATURE_LEN;


	if (pdu == NULL)
		return 0;

	if (value == NULL || vlen == NULL || handle == NULL)
		return 0;

	if (len < min_len)
		return 0;

	if (pdu[0] != ATT_OP_SIGNED_WRITE_CMD)
		return 0;

	*vlen = len - min_len;
	*handle = get_le16(&pdu[1]);
	memcpy(value, pdu + hdr_len, *vlen);

	memcpy(signature, pdu + hdr_len + *vlen, ATT_SIGNATURE_LEN);

	return len;
}

uint16_t enc_write_req(uint16_t handle, const uint8_t *value, size_t vlen,
						uint8_t *pdu, size_t len)
{
	const uint16_t min_len = sizeof(pdu[0]) + sizeof(handle);

	if (pdu == NULL)
		return 0;

	if (vlen > len - min_len)
		vlen = len - min_len;

	pdu[0] = ATT_OP_WRITE_REQ;
	put_le16(handle, &pdu[1]);

	if (vlen > 0) {
		memcpy(&pdu[3], value, vlen);
		return min_len + vlen;
	}

	return min_len;
}

uint16_t dec_write_req(const uint8_t *pdu, size_t len, uint16_t *handle,
						uint8_t *value, size_t *vlen)
{
	const uint16_t min_len = sizeof(pdu[0]) + sizeof(*handle);

	if (pdu == NULL)
		return 0;

	if (value == NULL || vlen == NULL || handle == NULL)
		return 0;

	if (len < min_len)
		return 0;

	if (pdu[0] != ATT_OP_WRITE_REQ)
		return 0;

	*handle = get_le16(&pdu[1]);
	*vlen = len - min_len;
	if (*vlen > 0)
		memcpy(value, pdu + min_len, *vlen);

	return len;
}

uint16_t enc_write_resp(uint8_t *pdu)
{
	if (pdu == NULL)
		return 0;

	pdu[0] = ATT_OP_WRITE_RESP;

	return sizeof(pdu[0]);
}

uint16_t dec_write_resp(const uint8_t *pdu, size_t len)
{
	if (pdu == NULL)
		return 0;

	if (pdu[0] != ATT_OP_WRITE_RESP)
		return 0;

	return len;
}

uint16_t enc_read_req(uint16_t handle, uint8_t *pdu, size_t len)
{
	if (pdu == NULL)
		return 0;

	/* Attribute Opcode (1 octet) */
	pdu[0] = ATT_OP_READ_REQ;
	/* Attribute Handle (2 octets) */
	put_le16(handle, &pdu[1]);

	return 3;
}

uint16_t enc_read_blob_req(uint16_t handle, uint16_t offset, uint8_t *pdu,
								size_t len)
{
	if (pdu == NULL)
		return 0;

	/* Attribute Opcode (1 octet) */
	pdu[0] = ATT_OP_READ_BLOB_REQ;
	/* Attribute Handle (2 octets) */
	put_le16(handle, &pdu[1]);
	/* Value Offset (2 octets) */
	put_le16(offset, &pdu[3]);

	return 5;
}

uint16_t dec_read_req(const uint8_t *pdu, size_t len, uint16_t *handle)
{
	const uint16_t min_len = sizeof(pdu[0]) + sizeof(*handle);

	if (pdu == NULL)
		return 0;

	if (handle == NULL)
		return 0;

	if (len < min_len)
		return 0;

	if (pdu[0] != ATT_OP_READ_REQ)
		return 0;

	*handle = get_le16(&pdu[1]);

	return min_len;
}

uint16_t dec_read_blob_req(const uint8_t *pdu, size_t len, uint16_t *handle,
							uint16_t *offset)
{
	const uint16_t min_len = sizeof(pdu[0]) + sizeof(*handle) +
							sizeof(*offset);

	if (pdu == NULL)
		return 0;

	if (handle == NULL)
		return 0;

	if (offset == NULL)
		return 0;

	if (len < min_len)
		return 0;

	if (pdu[0] != ATT_OP_READ_BLOB_REQ)
		return 0;

	*handle = get_le16(&pdu[1]);
	*offset = get_le16(&pdu[3]);

	return min_len;
}

uint16_t enc_read_resp(uint8_t *value, size_t vlen, uint8_t *pdu, size_t len)
{
	if (pdu == NULL)
		return 0;

	/* If the attribute value length is longer than the allowed PDU size,
	 * send only the octets that fit on the PDU. The remaining octets can
	 * be requested using the Read Blob Request. */
	if (vlen > len - 1)
		vlen = len - 1;

	pdu[0] = ATT_OP_READ_RESP;

	memcpy(pdu + 1, value, vlen);

	return vlen + 1;
}

uint16_t enc_read_blob_resp(uint8_t *value, size_t vlen, uint16_t offset,
						uint8_t *pdu, size_t len)
{
	if (pdu == NULL)
		return 0;

	vlen -= offset;
	if (vlen > len - 1)
		vlen = len - 1;

	pdu[0] = ATT_OP_READ_BLOB_RESP;

	memcpy(pdu + 1, &value[offset], vlen);

	return vlen + 1;
}

ssize_t dec_read_resp(const uint8_t *pdu, size_t len, uint8_t *value,
								size_t vlen)
{
	if (pdu == NULL)
		return -EINVAL;

	if (pdu[0] != ATT_OP_READ_RESP)
		return -EINVAL;

	if (value == NULL)
		return len - 1;

	if (vlen < (len - 1))
		return -ENOBUFS;

	memcpy(value, pdu + 1, len - 1);

	return len - 1;
}

uint16_t enc_error_resp(uint8_t opcode, uint16_t handle, uint8_t status,
						uint8_t *pdu, size_t len)
{
	/* Attribute Opcode (1 octet) */
	pdu[0] = ATT_OP_ERROR;
	/* Request Opcode In Error (1 octet) */
	pdu[1] = opcode;
	/* Attribute Handle In Error (2 octets) */
	put_le16(handle, &pdu[2]);
	/* Error Code (1 octet) */
	pdu[4] = status;

	return 5;
}

uint16_t enc_find_info_req(uint16_t start, uint16_t end, uint8_t *pdu,
								size_t len)
{
	if (pdu == NULL)
		return 0;

	/* Attribute Opcode (1 octet) */
	pdu[0] = ATT_OP_FIND_INFO_REQ;
	/* Starting Handle (2 octets) */
	put_le16(start, &pdu[1]);
	/* Ending Handle (2 octets) */
	put_le16(end, &pdu[3]);

	return 5;
}

uint16_t dec_find_info_req(const uint8_t *pdu, size_t len, uint16_t *start,
								uint16_t *end)
{
	const uint16_t min_len = sizeof(pdu[0]) + sizeof(*start) + sizeof(*end);

	if (pdu == NULL)
		return 0;

	if (len < min_len)
		return 0;

	if (start == NULL || end == NULL)
		return 0;

	if (pdu[0] != ATT_OP_FIND_INFO_REQ)
		return 0;

	*start = get_le16(&pdu[1]);
	*end = get_le16(&pdu[3]);

	return min_len;
}

uint16_t enc_find_info_resp(uint8_t format, struct att_data_list *list,
						uint8_t *pdu, size_t len)
{
	uint8_t *ptr;
	size_t i, w;

	if (pdu == NULL)
		return 0;

	if (list == NULL)
		return 0;

	if (len < list->len + sizeof(uint8_t) * 2)
		return 0;

	pdu[0] = ATT_OP_FIND_INFO_RESP;
	pdu[1] = format;
	ptr = (void *) &pdu[2];

	for (i = 0, w = 2; i < list->num && w + list->len <= len; i++) {
		memcpy(ptr, list->data[i], list->len);
		ptr += list->len;
		w += list->len;
	}

	return w;
}

struct att_data_list *dec_find_info_resp(const uint8_t *pdu, size_t len,
							uint8_t *format)
{
	struct att_data_list *list;
	uint8_t *ptr;
	uint16_t elen, num;
	int i;

	if (pdu == NULL)
		return 0;

	if (format == NULL)
		return 0;

	if (pdu[0] != ATT_OP_FIND_INFO_RESP)
		return 0;

	*format = pdu[1];
	elen = sizeof(pdu[0]) + sizeof(*format);
	if (*format == 0x01)
		elen += 2;
	else if (*format == 0x02)
		elen += 16;

	num = (len - 2) / elen;

	ptr = (void *) &pdu[2];

	list = att_data_list_alloc(num, elen);
	if (list == NULL)
		return NULL;

	for (i = 0; i < num; i++) {
		memcpy(list->data[i], ptr, list->len);
		ptr += list->len;
	}

	return list;
}

uint16_t enc_notification(uint16_t handle, uint8_t *value, size_t vlen,
						uint8_t *pdu, size_t len)
{
	const uint16_t min_len = sizeof(pdu[0]) + sizeof(uint16_t);

	if (pdu == NULL)
		return 0;

	if (len < (vlen + min_len))
		return 0;

	pdu[0] = ATT_OP_HANDLE_NOTIFY;
	put_le16(handle, &pdu[1]);
	memcpy(&pdu[3], value, vlen);

	return vlen + min_len;
}

uint16_t enc_indication(uint16_t handle, uint8_t *value, size_t vlen,
						uint8_t *pdu, size_t len)
{
	const uint16_t min_len = sizeof(pdu[0]) + sizeof(uint16_t);

	if (pdu == NULL)
		return 0;

	if (len < (vlen + min_len))
		return 0;

	pdu[0] = ATT_OP_HANDLE_IND;
	put_le16(handle, &pdu[1]);
	memcpy(&pdu[3], value, vlen);

	return vlen + min_len;
}

uint16_t dec_indication(const uint8_t *pdu, size_t len, uint16_t *handle,
						uint8_t *value, size_t vlen)
{
	const uint16_t min_len = sizeof(pdu[0]) + sizeof(uint16_t);
	uint16_t dlen;

	if (pdu == NULL)
		return 0;

	if (pdu[0] != ATT_OP_HANDLE_IND)
		return 0;

	if (len < min_len)
		return 0;

	dlen = MIN(len - min_len, vlen);

	if (handle)
		*handle = get_le16(&pdu[1]);

	memcpy(value, &pdu[3], dlen);

	return dlen;
}

uint16_t enc_confirmation(uint8_t *pdu, size_t len)
{
	if (pdu == NULL)
		return 0;

	/* Attribute Opcode (1 octet) */
	pdu[0] = ATT_OP_HANDLE_CNF;

	return 1;
}

uint16_t enc_mtu_req(uint16_t mtu, uint8_t *pdu, size_t len)
{
	if (pdu == NULL)
		return 0;

	/* Attribute Opcode (1 octet) */
	pdu[0] = ATT_OP_MTU_REQ;
	/* Client Rx MTU (2 octets) */
	put_le16(mtu, &pdu[1]);

	return 3;
}

uint16_t dec_mtu_req(const uint8_t *pdu, size_t len, uint16_t *mtu)
{
	const uint16_t min_len = sizeof(pdu[0]) + sizeof(*mtu);

	if (pdu == NULL)
		return 0;

	if (mtu == NULL)
		return 0;

	if (len < min_len)
		return 0;

	if (pdu[0] != ATT_OP_MTU_REQ)
		return 0;

	*mtu = get_le16(&pdu[1]);

	return min_len;
}

uint16_t enc_mtu_resp(uint16_t mtu, uint8_t *pdu, size_t len)
{
	if (pdu == NULL)
		return 0;

	/* Attribute Opcode (1 octet) */
	pdu[0] = ATT_OP_MTU_RESP;
	/* Server Rx MTU (2 octets) */
	put_le16(mtu, &pdu[1]);

	return 3;
}

uint16_t dec_mtu_resp(const uint8_t *pdu, size_t len, uint16_t *mtu)
{
	const uint16_t min_len = sizeof(pdu[0]) + sizeof(*mtu);

	if (pdu == NULL)
		return 0;

	if (mtu == NULL)
		return 0;

	if (len < min_len)
		return 0;

	if (pdu[0] != ATT_OP_MTU_RESP)
		return 0;

	*mtu = get_le16(&pdu[1]);

	return min_len;
}

uint16_t enc_prep_write_req(uint16_t handle, uint16_t offset,
					const uint8_t *value, size_t vlen,
					uint8_t *pdu, size_t len)
{
	const uint16_t min_len = sizeof(pdu[0]) + sizeof(handle) +
								sizeof(offset);

	if (pdu == NULL)
		return 0;

	if (vlen > len - min_len)
		vlen = len - min_len;

	pdu[0] = ATT_OP_PREP_WRITE_REQ;
	put_le16(handle, &pdu[1]);
	put_le16(offset, &pdu[3]);

	if (vlen > 0) {
		memcpy(&pdu[5], value, vlen);
		return min_len + vlen;
	}

	return min_len;
}

uint16_t dec_prep_write_req(const uint8_t *pdu, size_t len, uint16_t *handle,
				uint16_t *offset, uint8_t *value, size_t *vlen)
{
	const uint16_t min_len = sizeof(pdu[0]) + sizeof(*handle) +
							sizeof(*offset);

	if (pdu == NULL)
		return 0;

	if (handle == NULL || offset == NULL || value == NULL || vlen == NULL)
		return 0;

	if (len < min_len)
		return 0;

	if (pdu[0] != ATT_OP_PREP_WRITE_REQ)
		return 0;

	*handle = get_le16(&pdu[1]);
	*offset = get_le16(&pdu[3]);

	*vlen = len - min_len;
	if (*vlen > 0)
		memcpy(value, pdu + min_len, *vlen);

	return len;
}

uint16_t enc_prep_write_resp(uint16_t handle, uint16_t offset,
					const uint8_t *value, size_t vlen,
					uint8_t *pdu, size_t len)
{
	const uint16_t min_len = sizeof(pdu[0]) + sizeof(handle) +
								sizeof(offset);

	if (pdu == NULL)
		return 0;

	if (vlen > len - min_len)
		vlen = len - min_len;

	pdu[0] = ATT_OP_PREP_WRITE_RESP;
	put_le16(handle, &pdu[1]);
	put_le16(offset, &pdu[3]);

	if (vlen > 0) {
		memcpy(&pdu[5], value, vlen);
		return min_len + vlen;
	}

	return min_len;
}

uint16_t dec_prep_write_resp(const uint8_t *pdu, size_t len, uint16_t *handle,
				uint16_t *offset, uint8_t *value, size_t *vlen)
{
	const uint16_t min_len = sizeof(pdu[0]) + sizeof(*handle) +
								sizeof(*offset);

	if (pdu == NULL)
		return 0;

	if (handle == NULL || offset == NULL || value == NULL || vlen == NULL)
		return 0;

	if (len < min_len)
		return 0;

	if (pdu[0] != ATT_OP_PREP_WRITE_REQ)
		return 0;

	*handle = get_le16(&pdu[1]);
	*offset = get_le16(&pdu[3]);
	*vlen = len - min_len;
	if (*vlen > 0)
		memcpy(value, pdu + min_len, *vlen);

	return len;
}

uint16_t enc_exec_write_req(uint8_t flags, uint8_t *pdu, size_t len)
{
	if (pdu == NULL)
		return 0;

	if (flags > 1)
		return 0;

	/* Attribute Opcode (1 octet) */
	pdu[0] = ATT_OP_EXEC_WRITE_REQ;
	/* Flags (1 octet) */
	pdu[1] = flags;

	return 2;
}

uint16_t dec_exec_write_req(const uint8_t *pdu, size_t len, uint8_t *flags)
{
	const uint16_t min_len = sizeof(pdu[0]) + sizeof(*flags);

	if (pdu == NULL)
		return 0;

	if (flags == NULL)
		return 0;

	if (len < min_len)
		return 0;

	if (pdu[0] != ATT_OP_EXEC_WRITE_REQ)
		return 0;

	*flags = pdu[1];

	return min_len;
}

uint16_t enc_exec_write_resp(uint8_t *pdu)
{
	if (pdu == NULL)
		return 0;

	/* Attribute Opcode (1 octet) */
	pdu[0] = ATT_OP_EXEC_WRITE_RESP;

	return 1;
}

uint16_t dec_exec_write_resp(const uint8_t *pdu, size_t len)
{
	const uint16_t min_len = sizeof(pdu[0]);

	if (pdu == NULL)
		return 0;

	if (len < min_len)
		return 0;

	if (pdu[0] != ATT_OP_EXEC_WRITE_RESP)
		return 0;

	return len;
}
