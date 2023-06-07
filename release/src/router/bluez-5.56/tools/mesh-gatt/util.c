// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>

#include <glib.h>

#include "src/shared/shell.h"
#include "src/shared/util.h"

#include "tools/mesh-gatt/mesh-net.h"
#include "tools/mesh-gatt/node.h"
#include "tools/mesh-gatt/util.h"

void set_menu_prompt(const char *name, const char *id)
{
	char *prompt;

	prompt = g_strdup_printf(COLOR_BLUE "[%s%s%s]" COLOR_OFF "# ", name,
					id ? ": Target = " : "", id ? id : "");
	bt_shell_set_prompt(prompt);
	g_free(prompt);
}

void print_byte_array(const char *prefix, const void *ptr, int len)
{
	const uint8_t *data = ptr;
	char *line, *bytes;
	int i;

	line = g_malloc(strlen(prefix) + (16 * 3) + 2);
	sprintf(line, "%s ", prefix);
	bytes = line + strlen(prefix) + 1;

	for (i = 0; i < len; ++i) {
		sprintf(bytes, "%2.2x ", data[i]);
		if ((i + 1) % 16) {
			bytes += 3;
		} else {
			bt_shell_printf("\r%s\n", line);
			bytes = line + strlen(prefix) + 1;
		}
	}

	if (i % 16)
		bt_shell_printf("\r%s\n", line);

	g_free(line);
}

bool str2hex(const char *str, uint16_t in_len, uint8_t *out,
		uint16_t out_len)
{
	uint16_t i;

	if (in_len < out_len * 2)
		return false;

	for (i = 0; i < out_len; i++) {
		if (sscanf(&str[i * 2], "%02hhx", &out[i]) != 1)
			return false;
	}

	return true;
}

size_t hex2str(uint8_t *in, size_t in_len, char *out,
		size_t out_len)
{
	static const char hexdigits[] = "0123456789abcdef";
	size_t i;

	if(in_len * 2 > out_len - 1)
		return 0;

	for (i = 0; i < in_len; i++) {
		out[i * 2] = hexdigits[in[i] >> 4];
		out[i * 2 + 1] = hexdigits[in[i] & 0xf];
	}

	out[in_len * 2] = '\0';
	return i;
}

uint16_t mesh_opcode_set(uint32_t opcode, uint8_t *buf)
{
	if (opcode <= 0x7e) {
		buf[0] = opcode;
		return 1;
	} else if (opcode >= 0x8000 && opcode <= 0xbfff) {
		put_be16(opcode, buf);
		return 2;
	} else if (opcode >= 0xc00000 && opcode <= 0xffffff) {
		buf[0] = (opcode >> 16) & 0xff;
		put_be16(opcode, buf + 1);
		return 3;
	} else {
		bt_shell_printf("Illegal Opcode %x", opcode);
		return 0;
	}
}

bool mesh_opcode_get(const uint8_t *buf, uint16_t sz, uint32_t *opcode, int *n)
{
	if (!n || !opcode || sz < 1) return false;

	switch (buf[0] & 0xc0) {
	case 0x00:
	case 0x40:
		/* RFU */
		if (buf[0] == 0x7f)
			return false;

		*n = 1;
		*opcode = buf[0];
		break;

	case 0x80:
		if (sz < 2)
			return false;

		*n = 2;
		*opcode = get_be16(buf);
		break;

	case 0xc0:
		if (sz < 3)
			return false;

		*n = 3;
		*opcode = get_be16(buf + 1);
		*opcode |= buf[0] << 16;
		break;

	default:
		bt_shell_printf("Bad Packet:\n");
		print_byte_array("\t", (void *) buf, sz);
		return false;
	}

	return true;
}

const char *mesh_status_str(uint8_t status)
{
	switch (status) {
	case MESH_STATUS_SUCCESS: return "Success";
	case MESH_STATUS_INVALID_ADDRESS: return "Invalid Address";
	case MESH_STATUS_INVALID_MODEL: return "Invalid Model";
	case MESH_STATUS_INVALID_APPKEY: return "Invalid AppKey";
	case MESH_STATUS_INVALID_NETKEY: return "Invalid NetKey";
	case MESH_STATUS_INSUFF_RESOURCES: return "Insufficient Resources";
	case MESH_STATUS_IDX_ALREADY_STORED: return "Key Idx Already Stored";
	case MESH_STATUS_INVALID_PUB_PARAM: return "Invalid Publish Parameters";
	case MESH_STATUS_NOT_SUB_MOD: return "Not a Subscribe Model";
	case MESH_STATUS_STORAGE_FAIL: return "Storage Failure";
	case MESH_STATUS_FEAT_NOT_SUP: return "Feature Not Supported";
	case MESH_STATUS_CANNOT_UPDATE: return "Cannot Update";
	case MESH_STATUS_CANNOT_REMOVE: return "Cannot Remove";
	case MESH_STATUS_CANNOT_BIND: return "Cannot bind";
	case MESH_STATUS_UNABLE_CHANGE_STATE: return "Unable to change state";
	case MESH_STATUS_CANNOT_SET: return "Cannot set";
	case MESH_STATUS_UNSPECIFIED_ERROR: return "Unspecified error";
	case MESH_STATUS_INVALID_BINDING: return "Invalid Binding";

	default: return "Unknown";
	}
}

void print_model_pub(uint16_t ele_addr, uint32_t mod_id,
						struct mesh_publication *pub)
{
	bt_shell_printf("\tElement: %4.4x\n", ele_addr);
	bt_shell_printf("\tPub Addr: %4.4x", pub->u.addr16);
	if (mod_id > 0xffff0000)
		bt_shell_printf("\tModel: %8.8x \n", mod_id);
	else
		bt_shell_printf("\tModel: %4.4x \n",
				(uint16_t) (mod_id & 0xffff));
	bt_shell_printf("\tApp Key Idx: %4.4x", pub->app_idx);
	bt_shell_printf("\tTTL: %2.2x", pub->ttl);
}

void swap_u256_bytes(uint8_t *u256)
{
	int i;

	/* End-to-End byte reflection of 32 octet buffer */
	for (i = 0; i < 16; i++) {
		u256[i] ^= u256[31 - i];
		u256[31 - i] ^= u256[i];
		u256[i] ^= u256[31 - i];
	}
}
