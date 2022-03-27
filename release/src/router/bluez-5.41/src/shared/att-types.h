/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Google Inc.
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

#include <stdint.h>

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#define BT_ATT_SECURITY_AUTO	0
#define BT_ATT_SECURITY_LOW	1
#define BT_ATT_SECURITY_MEDIUM	2
#define BT_ATT_SECURITY_HIGH	3
#define BT_ATT_SECURITY_FIPS	4

#define BT_ATT_DEFAULT_LE_MTU	23
#define BT_ATT_MAX_LE_MTU	517
#define BT_ATT_MAX_VALUE_LEN	512

#define BT_ATT_LINK_BREDR	0x00
#define BT_ATT_LINK_LE		0x01
#define BT_ATT_LINK_LOCAL	0xff

/* ATT protocol opcodes */
#define BT_ATT_OP_ERROR_RSP			0x01
#define BT_ATT_OP_MTU_REQ			0x02
#define BT_ATT_OP_MTU_RSP			0x03
#define BT_ATT_OP_FIND_INFO_REQ			0x04
#define BT_ATT_OP_FIND_INFO_RSP			0x05
#define BT_ATT_OP_FIND_BY_TYPE_REQ		0x06
#define BT_ATT_OP_FIND_BY_TYPE_RSP		0x07
#define BT_ATT_OP_READ_BY_TYPE_REQ		0x08
#define BT_ATT_OP_READ_BY_TYPE_RSP		0x09
#define BT_ATT_OP_READ_REQ			0x0a
#define BT_ATT_OP_READ_RSP			0x0b
#define BT_ATT_OP_READ_BLOB_REQ			0x0c
#define BT_ATT_OP_READ_BLOB_RSP			0x0d
#define BT_ATT_OP_READ_MULT_REQ			0x0e
#define BT_ATT_OP_READ_MULT_RSP			0x0f
#define BT_ATT_OP_READ_BY_GRP_TYPE_REQ		0x10
#define BT_ATT_OP_READ_BY_GRP_TYPE_RSP		0x11
#define BT_ATT_OP_WRITE_REQ			0x12
#define BT_ATT_OP_WRITE_RSP			0x13
#define BT_ATT_OP_WRITE_CMD			0x52
#define BT_ATT_OP_SIGNED_WRITE_CMD		0xD2
#define BT_ATT_OP_PREP_WRITE_REQ		0x16
#define BT_ATT_OP_PREP_WRITE_RSP		0x17
#define BT_ATT_OP_EXEC_WRITE_REQ		0x18
#define BT_ATT_OP_EXEC_WRITE_RSP		0x19
#define BT_ATT_OP_HANDLE_VAL_NOT		0x1B
#define BT_ATT_OP_HANDLE_VAL_IND		0x1D
#define BT_ATT_OP_HANDLE_VAL_CONF		0x1E

/* Packed struct definitions for ATT protocol PDUs */
/* TODO: Complete these definitions for all opcodes */
struct bt_att_pdu_error_rsp {
	uint8_t opcode;
	uint16_t handle;
	uint8_t ecode;
} __packed;

/* Special opcode to receive all requests (legacy servers) */
#define BT_ATT_ALL_REQUESTS 0x00

/* Error codes for Error response PDU */
#define BT_ATT_ERROR_INVALID_HANDLE			0x01
#define BT_ATT_ERROR_READ_NOT_PERMITTED			0x02
#define BT_ATT_ERROR_WRITE_NOT_PERMITTED		0x03
#define BT_ATT_ERROR_INVALID_PDU			0x04
#define BT_ATT_ERROR_AUTHENTICATION			0x05
#define BT_ATT_ERROR_REQUEST_NOT_SUPPORTED		0x06
#define BT_ATT_ERROR_INVALID_OFFSET			0x07
#define BT_ATT_ERROR_AUTHORIZATION			0x08
#define BT_ATT_ERROR_PREPARE_QUEUE_FULL			0x09
#define BT_ATT_ERROR_ATTRIBUTE_NOT_FOUND		0x0A
#define BT_ATT_ERROR_ATTRIBUTE_NOT_LONG			0x0B
#define BT_ATT_ERROR_INSUFFICIENT_ENCRYPTION_KEY_SIZE	0x0C
#define BT_ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LEN	0x0D
#define BT_ATT_ERROR_UNLIKELY				0x0E
#define BT_ATT_ERROR_INSUFFICIENT_ENCRYPTION		0x0F
#define BT_ATT_ERROR_UNSUPPORTED_GROUP_TYPE		0x10
#define BT_ATT_ERROR_INSUFFICIENT_RESOURCES		0x11

/*
 * Common Profile and Service Error Code descriptions (see Supplement to the
 * Bluetooth Core Specification, sections 1.2 and 2). The error codes within
 * 0xE0-0xFC are reserved for future use. The remaining 3 are defined as the
 * following:
 */
#define BT_ERROR_CCC_IMPROPERLY_CONFIGURED      0xfd
#define BT_ERROR_ALREADY_IN_PROGRESS            0xfe
#define BT_ERROR_OUT_OF_RANGE                   0xff

/*
 * ATT attribute permission bitfield values. Permissions are grouped as
 * "Access", "Encryption", "Authentication", and "Authorization". A bitmask of
 * permissions is a byte that encodes a combination of these.
 */
#define BT_ATT_PERM_READ		0x01
#define BT_ATT_PERM_WRITE		0x02
#define BT_ATT_PERM_READ_ENCRYPT	0x04
#define BT_ATT_PERM_WRITE_ENCRYPT	0x08
#define BT_ATT_PERM_ENCRYPT		(BT_ATT_PERM_READ_ENCRYPT | \
					BT_ATT_PERM_WRITE_ENCRYPT)
#define BT_ATT_PERM_READ_AUTHEN		0x10
#define BT_ATT_PERM_WRITE_AUTHEN	0x20
#define BT_ATT_PERM_AUTHEN		(BT_ATT_PERM_READ_AUTHEN | \
					BT_ATT_PERM_WRITE_AUTHEN)
#define BT_ATT_PERM_AUTHOR		0x40
#define BT_ATT_PERM_NONE		0x80
#define BT_ATT_PERM_READ_SECURE		0x0100
#define BT_ATT_PERM_WRITE_SECURE	0x0200
#define BT_ATT_PERM_SECURE		(BT_ATT_PERM_READ_SECURE | \
					BT_ATT_PERM_WRITE_SECURE)

/* GATT Characteristic Properties Bitfield values */
#define BT_GATT_CHRC_PROP_BROADCAST			0x01
#define BT_GATT_CHRC_PROP_READ				0x02
#define BT_GATT_CHRC_PROP_WRITE_WITHOUT_RESP		0x04
#define BT_GATT_CHRC_PROP_WRITE				0x08
#define BT_GATT_CHRC_PROP_NOTIFY			0x10
#define BT_GATT_CHRC_PROP_INDICATE			0x20
#define BT_GATT_CHRC_PROP_AUTH				0x40
#define BT_GATT_CHRC_PROP_EXT_PROP			0x80

/* GATT Characteristic Extended Properties Bitfield values */
#define BT_GATT_CHRC_EXT_PROP_RELIABLE_WRITE		0x01
#define BT_GATT_CHRC_EXT_PROP_WRITABLE_AUX		0x02
#define BT_GATT_CHRC_EXT_PROP_ENC_READ			0x04
#define BT_GATT_CHRC_EXT_PROP_ENC_WRITE			0x08
#define BT_GATT_CHRC_EXT_PROP_ENC	(BT_GATT_CHRC_EXT_PROP_ENC_READ | \
					BT_GATT_CHRC_EXT_PROP_ENC_WRITE)
#define BT_GATT_CHRC_EXT_PROP_AUTH_READ			0x10
#define BT_GATT_CHRC_EXT_PROP_AUTH_WRITE		0x20
#define BT_GATT_CHRC_EXT_PROP_AUTH	(BT_GATT_CHRC_EXT_PROP_AUTH_READ | \
					BT_GATT_CHRC_EXT_PROP_AUTH_WRITE)
