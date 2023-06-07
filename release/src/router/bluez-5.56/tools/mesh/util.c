// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017, 2019  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#include <ell/ell.h>

#include "src/shared/shell.h"
#include "src/shared/util.h"

#include "mesh/mesh-defs.h"

#include "tools/mesh/util.h"

void set_menu_prompt(const char *name, const char *id)
{
	char *prompt;

	prompt = l_strdup_printf(COLOR_BLUE "[%s%s%s]" COLOR_OFF "# ", name,
					id ? ": Target = " : "", id ? id : "");
	bt_shell_set_prompt(prompt);
	l_free(prompt);
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
	}

	bt_shell_printf("Illegal Opcode %x", opcode);
	return 0;
}

bool mesh_opcode_get(const uint8_t *buf, uint16_t sz, uint32_t *opcode, int *n)
{
	if (!n || !opcode || sz < 1)
		return false;

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
		bt_shell_printf("Bad opcode");
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
	case MESH_STATUS_FEATURE_NO_SUPPORT: return "Feature Not Supported";
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

const char *sig_model_string(uint16_t sig_model_id)
{
	switch (sig_model_id) {
	case 0x0000: return "Configuration Server";
	case 0x0001: return "Configuration Client";
	case 0x0002: return "Health Server";
	case 0x0003: return "Health Client";
	case 0x1000: return "Generic OnOff Server";
	case 0x1001: return "Generic OnOff Client";
	case 0x1002: return "Generic Level Server";
	case 0x1003: return "Generic Level Client";
	case 0x1004: return "Generic Default Transition Time Server";
	case 0x1005: return "Generic Default Transition Time Client";
	case 0x1006: return "Generic Power OnOff Server";
	case 0x1007: return "Generic Power OnOff Setup Server";
	case 0x1008: return "Generic Power OnOff Client";
	case 0x1009: return "Generic Power Level Server";
	case 0x100A: return "Generic Power Level Setup Server";
	case 0x100B: return "Generic Power Level Client";
	case 0x100C: return "Generic Battery Server";
	case 0x100D: return "Generic Battery Client";
	case 0x100E: return "Generic Location Server";
	case 0x100F: return "Generic Location Setup Server";
	case 0x1010: return "Generic Location Client";
	case 0x1011: return "Generic Admin Property Server";
	case 0x1012: return "Generic Manufacturer Property Server";
	case 0x1013: return "Generic User Property Server";
	case 0x1014: return "Generic Client Property Server";
	case 0x1015: return "Generic Property Client";
	case 0x1100: return "Sensor Server";
	case 0x1101: return "Sensor Setup Server";
	case 0x1102: return "Sensor Client";
	case 0x1200: return "Time Server";
	case 0x1201: return "Time Setup Server";
	case 0x1202: return "Time Client";
	case 0x1203: return "Scene Server";
	case 0x1204: return "Scene Setup Server";
	case 0x1205: return "Scene Client";
	case 0x1206: return "Scheduler Server";
	case 0x1207: return "Scheduler Setup Server";
	case 0x1208: return "Scheduler Client";
	case 0x1300: return "Light Lightness Server";
	case 0x1301: return "Light Lightness Setup Server";
	case 0x1302: return "Light Lightness Client";
	case 0x1303: return "Light CTL Server";
	case 0x1304: return "Light CTL Setup Server";
	case 0x1305: return "Light CTL Client";
	case 0x1306: return "Light CTL Temperature Server";
	case 0x1307: return "Light HSL Server";
	case 0x1308: return "Light HSL Setup Server";
	case 0x1309: return "Light HSL Client";
	case 0x130A: return "Light HSL Hue Server";
	case 0x130B: return "Light HSL Saturation Server";
	case 0x130C: return "Light xyL Server";
	case 0x130D: return "Light xyL Setup Server";
	case 0x130E: return "Light xyL Client";
	case 0x130F: return "Light LC Server";
	case 0x1310: return "Light LC Setup Server";
	case 0x1311: return "Light LC Client";

	default: return "Unknown";
	}
}
