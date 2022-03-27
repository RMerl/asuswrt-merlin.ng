/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011  Intel Corporation.
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
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

/* SMP command codes */
#define SMP_CMD_PAIRING_REQ	0x01
#define SMP_CMD_PAIRING_RESP	0x02
#define SMP_CMD_PAIRING_CONFIRM	0x03
#define SMP_CMD_PAIRING_RANDOM	0x04
#define SMP_CMD_PAIRING_FAILED	0x05
#define SMP_CMD_ENCRYPT_INFO	0x06
#define SMP_CMD_MASTER_IDENT	0x07
#define SMP_CMD_IDENT_INFO	0X08
#define SMP_CMD_IDENT_ADDR_INFO	0x09
#define SMP_CMD_SIGN_INFO	0x0a
#define SMP_CMD_SECURITY_REQ	0x0b

/* IO Capabilities values */
#define SMP_IO_DISPLAY_ONLY	0x00
#define SMP_IO_DISPLAY_YESNO	0x01
#define SMP_IO_KEYBOARD_ONLY	0x02
#define SMP_IO_NO_INPUT_OUTPUT	0x03
#define SMP_IO_KEYBOARD_DISPLAY	0x04

/* OOB Data Present Values */
#define SMP_OOB_NOT_PRESENT	0x00
#define SMP_OOB_PRESENT		0x01

#define SMP_DIST_ENC_KEY	0x01
#define SMP_DIST_ID_KEY		0x02
#define SMP_DIST_SIGN		0x04

#define SMP_AUTH_NONE		0x00
#define SMP_AUTH_BONDING	0x01
#define SMP_AUTH_MITM		0x04

#define SMP_REASON_PASSKEY_ENTRY_FAILED		0x01
#define SMP_REASON_OOB_NOT_AVAIL		0x02
#define SMP_REASON_AUTH_REQUIREMENTS		0x03
#define SMP_REASON_CONFIRM_FAILED		0x04
#define SMP_REASON_PAIRING_NOTSUPP		0x05
#define SMP_REASON_ENC_KEY_SIZE			0x06
#define SMP_REASON_CMD_NOTSUPP			0x07
#define SMP_REASON_UNSPECIFIED			0x08
#define SMP_REASON_REPEATED_ATTEMPTS		0x09

static const char *smpcmd2str(uint8_t cmd)
{
	switch (cmd) {
	case SMP_CMD_PAIRING_REQ:
		return "Pairing Request";
	case SMP_CMD_PAIRING_RESP:
		return "Pairing Response";
	case SMP_CMD_PAIRING_CONFIRM:
		return "Pairing Confirm";
	case SMP_CMD_PAIRING_RANDOM:
		return "Pairing Random";
	case SMP_CMD_PAIRING_FAILED:
		return "Pairing Failed";
	case SMP_CMD_ENCRYPT_INFO:
		return "Encryption Information";
	case SMP_CMD_MASTER_IDENT:
		return "Master Identification";
	case SMP_CMD_IDENT_INFO:
		return "Identity Information";
	case SMP_CMD_IDENT_ADDR_INFO:
		return "Identity Address Information";
	case SMP_CMD_SIGN_INFO:
		return "Signing Information";
	case SMP_CMD_SECURITY_REQ:
		return "Security Request";
	default:
		return "Unknown";
	}
}

static const char *smpio2str(uint8_t cap)
{
	switch(cap) {
	case SMP_IO_DISPLAY_ONLY:
		return "DisplayOnly";
	case SMP_IO_DISPLAY_YESNO:
		return "DisplayYesNo";
	case SMP_IO_KEYBOARD_ONLY:
		return "KeyboardOnly";
	case SMP_IO_NO_INPUT_OUTPUT:
		return "NoInputNoOutput";
	case SMP_IO_KEYBOARD_DISPLAY:
		return "KeyboardDisplay";
	default:
		return "Unkown";
	}
}

static const char *smpreason2str(uint8_t reason)
{
	switch (reason) {
	case SMP_REASON_PASSKEY_ENTRY_FAILED:
		return "Passkey Entry Failed";
	case SMP_REASON_OOB_NOT_AVAIL:
		return "OOB Not Available";
	case SMP_REASON_AUTH_REQUIREMENTS:
		return "Authentication Requirements";
	case SMP_REASON_CONFIRM_FAILED:
		return "Confirm Value Failed";
	case SMP_REASON_PAIRING_NOTSUPP:
		return "Pairing Not Supported";
	case SMP_REASON_ENC_KEY_SIZE:
		return "Encryption Key Size";
	case SMP_REASON_CMD_NOTSUPP:
		return "Command Not Supported";
	case SMP_REASON_UNSPECIFIED:
		return "Unspecified Reason";
	case SMP_REASON_REPEATED_ATTEMPTS:
		return "Repeated Attempts";
	default:
		return "Unkown";
	}
}

static void smp_cmd_pairing_dump(int level, struct frame *frm)
{
	uint8_t cap = p_get_u8(frm);
	uint8_t oob = p_get_u8(frm);
	uint8_t auth = p_get_u8(frm);
	uint8_t key_size = p_get_u8(frm);
	uint8_t int_dist = p_get_u8(frm);
	uint8_t resp_dist = p_get_u8(frm);

	p_indent(level, frm);
	printf("capability 0x%2.2x oob 0x%2.2x auth req 0x%2.2x\n", cap, oob,
									auth);

	p_indent(level , frm);
	printf("max key size 0x%2.2x init key dist 0x%2.2x "
		"resp key dist 0x%2.2x\n", key_size, int_dist, resp_dist);

	p_indent(level , frm);
	printf("Capability: %s (OOB data %s)\n", smpio2str(cap),
				oob == 0x00 ? "not present" : "available");

	p_indent(level , frm);
	printf("Authentication: %s (%s)\n",
			auth & SMP_AUTH_BONDING ? "Bonding" : "No Bonding",
			auth & SMP_AUTH_MITM ? "MITM Protection" :
			"No MITM Protection");

	p_indent(level , frm);
	printf("Initiator Key Distribution:  %s %s %s\n",
			int_dist & SMP_DIST_ENC_KEY ? "LTK" : "",
			int_dist & SMP_DIST_ID_KEY ? "IRK" : "",
			int_dist & SMP_DIST_SIGN ? "CSRK" : "");

	p_indent(level , frm);
	printf("Responder Key Distribution:  %s %s %s\n",
			resp_dist & SMP_DIST_ENC_KEY ? "LTK" : "",
			resp_dist & SMP_DIST_ID_KEY ? "IRK" : "",
			resp_dist & SMP_DIST_SIGN ? "CSRK" : "");
}

static void smp_cmd_pairing_confirm_dump(int level, struct frame *frm)
{
	int i;

	p_indent(level, frm);
	printf("key ");
	for (i = 0; i < 16; i++)
		printf("%2.2x", p_get_u8(frm));
	printf("\n");
}

static void smp_cmd_pairing_random_dump(int level, struct frame *frm)
{
	int i;

	p_indent(level, frm);
	printf("random ");
	for (i = 0; i < 16; i++)
		printf("%2.2x", p_get_u8(frm));
	printf("\n");
}

static void smp_cmd_pairing_failed_dump(int level, struct frame *frm)
{
	uint8_t reason = p_get_u8(frm);

	p_indent(level, frm);
	printf("reason 0x%2.2x\n", reason);

	p_indent(level, frm);
	printf("Reason %s\n", smpreason2str(reason));
}

static void smp_cmd_encrypt_info_dump(int level, struct frame *frm)
{
	int i;

	p_indent(level, frm);
	printf("LTK ");
	for (i = 0; i < 16; i++)
		printf("%2.2x", p_get_u8(frm));
	printf("\n");
}

static void smp_cmd_master_ident_dump(int level, struct frame *frm)
{
	uint16_t ediv = btohs(htons(p_get_u16(frm)));
	int i;

	p_indent(level, frm);
	printf("EDIV 0x%4.4x ", ediv);

	printf("Rand 0x");
	for (i = 0; i < 8; i++)
		printf("%2.2x", p_get_u8(frm));
	printf("\n");
}

static void smp_cmd_ident_info_dump(int level, struct frame *frm)
{
	int i;

	p_indent(level, frm);
	printf("IRK ");
	for (i = 0; i < 16; i++)
		printf("%2.2x", p_get_u8(frm));
	printf("\n");
}

static void smp_cmd_ident_addr_info_dump(int level, struct frame *frm)
{
	uint8_t type = p_get_u8(frm);
	char addr[18];

	p_indent(level, frm);
	p_ba2str((bdaddr_t *) frm, addr);
	printf("bdaddr %s (%s)\n", addr, type == 0x00 ? "Public" : "Random");
}

static void smp_cmd_sign_info_dump(int level, struct frame *frm)
{
	int i;

	p_indent(level, frm);
	printf("CSRK ");
	for (i = 0; i < 16; i++)
		printf("%2.2x", p_get_u8(frm));
	printf("\n");
}

static void smp_cmd_security_req_dump(int level, struct frame *frm)
{
	uint8_t auth = p_get_u8(frm);

	p_indent(level, frm);
	printf("auth req 0x%2.2x\n", auth);
}

void smp_dump(int level, struct frame *frm)
{
	uint8_t cmd;

	cmd = p_get_u8(frm);

	p_indent(level, frm);
	printf("SMP: %s (0x%.2x)\n", smpcmd2str(cmd), cmd);

	switch (cmd) {
	case SMP_CMD_PAIRING_REQ:
		smp_cmd_pairing_dump(level + 1, frm);
		break;
	case SMP_CMD_PAIRING_RESP:
		smp_cmd_pairing_dump(level + 1, frm);
		break;
	case SMP_CMD_PAIRING_CONFIRM:
		smp_cmd_pairing_confirm_dump(level + 1, frm);
		break;
	case SMP_CMD_PAIRING_RANDOM:
		smp_cmd_pairing_random_dump(level + 1, frm);
		break;
	case SMP_CMD_PAIRING_FAILED:
		smp_cmd_pairing_failed_dump(level + 1, frm);
		break;
	case SMP_CMD_ENCRYPT_INFO:
		smp_cmd_encrypt_info_dump(level + 1, frm);
		break;
	case SMP_CMD_MASTER_IDENT:
		smp_cmd_master_ident_dump(level + 1, frm);
		break;
	case SMP_CMD_IDENT_INFO:
		smp_cmd_ident_info_dump(level + 1, frm);
		break;
	case SMP_CMD_IDENT_ADDR_INFO:
		smp_cmd_ident_addr_info_dump(level + 1, frm);
		break;
	case SMP_CMD_SIGN_INFO:
		smp_cmd_sign_info_dump(level + 1, frm);
		break;
	case SMP_CMD_SECURITY_REQ:
		smp_cmd_security_req_dump(level + 1, frm);
		break;
	default:
		raw_dump(level, frm);
	}
}
