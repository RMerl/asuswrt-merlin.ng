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

#include "src/shared/crypto.h"

/* Len of signature in write signed packet */
#define ATT_SIGNATURE_LEN		12

/* Attribute Protocol Opcodes */
#define ATT_OP_ERROR			0x01
#define ATT_OP_MTU_REQ			0x02
#define ATT_OP_MTU_RESP			0x03
#define ATT_OP_FIND_INFO_REQ		0x04
#define ATT_OP_FIND_INFO_RESP		0x05
#define ATT_OP_FIND_BY_TYPE_REQ		0x06
#define ATT_OP_FIND_BY_TYPE_RESP	0x07
#define ATT_OP_READ_BY_TYPE_REQ		0x08
#define ATT_OP_READ_BY_TYPE_RESP	0x09
#define ATT_OP_READ_REQ			0x0A
#define ATT_OP_READ_RESP		0x0B
#define ATT_OP_READ_BLOB_REQ		0x0C
#define ATT_OP_READ_BLOB_RESP		0x0D
#define ATT_OP_READ_MULTI_REQ		0x0E
#define ATT_OP_READ_MULTI_RESP		0x0F
#define ATT_OP_READ_BY_GROUP_REQ	0x10
#define ATT_OP_READ_BY_GROUP_RESP	0x11
#define ATT_OP_WRITE_REQ		0x12
#define ATT_OP_WRITE_RESP		0x13
#define ATT_OP_WRITE_CMD		0x52
#define ATT_OP_PREP_WRITE_REQ		0x16
#define ATT_OP_PREP_WRITE_RESP		0x17
#define ATT_OP_EXEC_WRITE_REQ		0x18
#define ATT_OP_EXEC_WRITE_RESP		0x19
#define ATT_OP_HANDLE_NOTIFY		0x1B
#define ATT_OP_HANDLE_IND		0x1D
#define ATT_OP_HANDLE_CNF		0x1E
#define ATT_OP_SIGNED_WRITE_CMD		0xD2

/* Error codes for Error response PDU */
#define ATT_ECODE_INVALID_HANDLE		0x01
#define ATT_ECODE_READ_NOT_PERM			0x02
#define ATT_ECODE_WRITE_NOT_PERM		0x03
#define ATT_ECODE_INVALID_PDU			0x04
#define ATT_ECODE_AUTHENTICATION		0x05
#define ATT_ECODE_REQ_NOT_SUPP			0x06
#define ATT_ECODE_INVALID_OFFSET		0x07
#define ATT_ECODE_AUTHORIZATION			0x08
#define ATT_ECODE_PREP_QUEUE_FULL		0x09
#define ATT_ECODE_ATTR_NOT_FOUND		0x0A
#define ATT_ECODE_ATTR_NOT_LONG			0x0B
#define ATT_ECODE_INSUFF_ENCR_KEY_SIZE		0x0C
#define ATT_ECODE_INVAL_ATTR_VALUE_LEN		0x0D
#define ATT_ECODE_UNLIKELY			0x0E
#define ATT_ECODE_INSUFF_ENC			0x0F
#define ATT_ECODE_UNSUPP_GRP_TYPE		0x10
#define ATT_ECODE_INSUFF_RESOURCES		0x11
/* Application error */
#define ATT_ECODE_IO				0x80
#define ATT_ECODE_TIMEOUT			0x81
#define ATT_ECODE_ABORTED			0x82

#define ATT_MAX_VALUE_LEN			512
#define ATT_DEFAULT_L2CAP_MTU			48
#define ATT_DEFAULT_LE_MTU			23

#define ATT_CID					4
#define ATT_PSM					31

/* Flags for Execute Write Request Operation */
#define ATT_CANCEL_ALL_PREP_WRITES		0x00
#define ATT_WRITE_ALL_PREP_WRITES		0x01

/* Find Information Response Formats */
#define ATT_FIND_INFO_RESP_FMT_16BIT		0x01
#define ATT_FIND_INFO_RESP_FMT_128BIT		0x02

struct att_data_list {
	uint16_t num;
	uint16_t len;
	uint8_t **data;
};

struct att_range {
	uint16_t start;
	uint16_t end;
};

struct att_data_list *att_data_list_alloc(uint16_t num, uint16_t len);
void att_data_list_free(struct att_data_list *list);

const char *att_ecode2str(uint8_t status);
uint16_t enc_read_by_grp_req(uint16_t start, uint16_t end, bt_uuid_t *uuid,
						uint8_t *pdu, size_t len);
uint16_t dec_read_by_grp_req(const uint8_t *pdu, size_t len, uint16_t *start,
					uint16_t *end, bt_uuid_t *uuid);
uint16_t enc_read_by_grp_resp(struct att_data_list *list, uint8_t *pdu,
								size_t len);
uint16_t enc_find_by_type_req(uint16_t start, uint16_t end, bt_uuid_t *uuid,
				const uint8_t *value, size_t vlen, uint8_t *pdu,
				size_t len);
uint16_t dec_find_by_type_req(const uint8_t *pdu, size_t len, uint16_t *start,
		uint16_t *end, bt_uuid_t *uuid, uint8_t *value, size_t *vlen);
uint16_t enc_find_by_type_resp(GSList *ranges, uint8_t *pdu, size_t len);
GSList *dec_find_by_type_resp(const uint8_t *pdu, size_t len);
struct att_data_list *dec_read_by_grp_resp(const uint8_t *pdu, size_t len);
uint16_t enc_read_by_type_req(uint16_t start, uint16_t end, bt_uuid_t *uuid,
						uint8_t *pdu, size_t len);
uint16_t dec_read_by_type_req(const uint8_t *pdu, size_t len, uint16_t *start,
					uint16_t *end, bt_uuid_t *uuid);
uint16_t enc_read_by_type_resp(struct att_data_list *list, uint8_t *pdu,
								size_t len);
uint16_t enc_write_cmd(uint16_t handle, const uint8_t *value, size_t vlen,
						uint8_t *pdu, size_t len);
uint16_t dec_write_cmd(const uint8_t *pdu, size_t len, uint16_t *handle,
						uint8_t *value, size_t *vlen);
uint16_t enc_signed_write_cmd(uint16_t handle,
					const uint8_t *value, size_t vlen,
					struct bt_crypto *crypto,
					const uint8_t csrk[16],
					uint32_t sign_cnt,
					uint8_t *pdu, size_t len);
uint16_t dec_signed_write_cmd(const uint8_t *pdu, size_t len,
						uint16_t *handle,
						uint8_t *value, size_t *vlen,
						uint8_t signature[12]);
struct att_data_list *dec_read_by_type_resp(const uint8_t *pdu, size_t len);
uint16_t enc_write_req(uint16_t handle, const uint8_t *value, size_t vlen,
						uint8_t *pdu, size_t len);
uint16_t dec_write_req(const uint8_t *pdu, size_t len, uint16_t *handle,
						uint8_t *value, size_t *vlen);
uint16_t enc_write_resp(uint8_t *pdu);
uint16_t dec_write_resp(const uint8_t *pdu, size_t len);
uint16_t enc_read_req(uint16_t handle, uint8_t *pdu, size_t len);
uint16_t enc_read_blob_req(uint16_t handle, uint16_t offset, uint8_t *pdu,
								size_t len);
uint16_t dec_read_req(const uint8_t *pdu, size_t len, uint16_t *handle);
uint16_t dec_read_blob_req(const uint8_t *pdu, size_t len, uint16_t *handle,
							uint16_t *offset);
uint16_t enc_read_resp(uint8_t *value, size_t vlen, uint8_t *pdu, size_t len);
uint16_t enc_read_blob_resp(uint8_t *value, size_t vlen, uint16_t offset,
						uint8_t *pdu, size_t len);
ssize_t dec_read_resp(const uint8_t *pdu, size_t len, uint8_t *value,
								size_t vlen);
uint16_t enc_error_resp(uint8_t opcode, uint16_t handle, uint8_t status,
						uint8_t *pdu, size_t len);
uint16_t enc_find_info_req(uint16_t start, uint16_t end, uint8_t *pdu,
								size_t len);
uint16_t dec_find_info_req(const uint8_t *pdu, size_t len, uint16_t *start,
								uint16_t *end);
uint16_t enc_find_info_resp(uint8_t format, struct att_data_list *list,
						uint8_t *pdu, size_t len);
struct att_data_list *dec_find_info_resp(const uint8_t *pdu, size_t len,
							uint8_t *format);
uint16_t enc_notification(uint16_t handle, uint8_t *value, size_t vlen,
						uint8_t *pdu, size_t len);
uint16_t enc_indication(uint16_t handle, uint8_t *value, size_t vlen,
						uint8_t *pdu, size_t len);
uint16_t dec_indication(const uint8_t *pdu, size_t len, uint16_t *handle,
						uint8_t *value, size_t vlen);
uint16_t enc_confirmation(uint8_t *pdu, size_t len);

uint16_t enc_mtu_req(uint16_t mtu, uint8_t *pdu, size_t len);
uint16_t dec_mtu_req(const uint8_t *pdu, size_t len, uint16_t *mtu);
uint16_t enc_mtu_resp(uint16_t mtu, uint8_t *pdu, size_t len);
uint16_t dec_mtu_resp(const uint8_t *pdu, size_t len, uint16_t *mtu);

uint16_t enc_prep_write_req(uint16_t handle, uint16_t offset,
					const uint8_t *value, size_t vlen,
					uint8_t *pdu, size_t len);
uint16_t dec_prep_write_req(const uint8_t *pdu, size_t len, uint16_t *handle,
				uint16_t *offset, uint8_t *value, size_t *vlen);
uint16_t enc_prep_write_resp(uint16_t handle, uint16_t offset,
					const uint8_t *value, size_t vlen,
					uint8_t *pdu, size_t len);
uint16_t dec_prep_write_resp(const uint8_t *pdu, size_t len, uint16_t *handle,
						uint16_t *offset, uint8_t *value,
						size_t *vlen);
uint16_t enc_exec_write_req(uint8_t flags, uint8_t *pdu, size_t len);
uint16_t dec_exec_write_req(const uint8_t *pdu, size_t len, uint8_t *flags);
uint16_t enc_exec_write_resp(uint8_t *pdu);
uint16_t dec_exec_write_resp(const uint8_t *pdu, size_t len);
