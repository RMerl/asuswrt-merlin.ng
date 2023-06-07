// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2003-2011  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

static char *type2str(uint8_t head)
{
	switch (head & 0xf0) {
	case 0x00:
		return "Handshake";
	case 0x10:
		return "Control";
	case 0x40:
		return "Get report";
	case 0x50:
		return "Set report";
	case 0x60:
		return "Get protocol";
	case 0x70:
		return "Set protocol";
	case 0x80:
		return "Get idle";
	case 0x90:
		return "Set idle";
	case 0xa0:
		return "Data";
	case 0xb0:
		return "Data continuation";
	default:
		return "Reserved";
	}
}

static char *result2str(uint8_t head)
{
	switch (head & 0x0f) {
	case 0x00:
		return "Successful";
	case 0x01:
		return "Not ready";
	case 0x02:
		return "Invalid report ID";
	case 0x03:
		return "Unsupported request";
	case 0x04:
		return "Invalid parameter";
	case 0x0e:
		return "Unknown";
	case 0x0f:
		return "Fatal";
	default:
		return "Reserved";
	}
}

static char *operation2str(uint8_t head)
{
	switch (head & 0x0f) {
	case 0x00:
		return "No operation";
	case 0x01:
		return "Hard reset";
	case 0x02:
		return "Soft reset";
	case 0x03:
		return "Suspend";
	case 0x04:
		return "Exit suspend";
	case 0x05:
		return "Virtual cable unplug";
	default:
		return "Reserved";
	}
}

static char *report2str(uint8_t head)
{
	switch (head & 0x03) {
	case 0x00:
		return "Other report";
	case 0x01:
		return "Input report";
	case 0x02:
		return "Output report";
	case 0x03:
		return "Feature report";
	default:
		return "Reserved";
	}
}

static char *protocol2str(uint8_t head)
{
	switch (head & 0x01) {
	case 0x00:
		return "Boot protocol";
	case 0x01:
		return "Report protocol";
	default:
		return "Reserved";
	}
}

void hidp_dump(int level, struct frame *frm)
{
	uint8_t hdr;
	char *param;

	hdr = p_get_u8(frm);

	switch (hdr & 0xf0) {
	case 0x00:
		param = result2str(hdr);
		break;
	case 0x10:
		param = operation2str(hdr);
		break;
	case 0x60:
	case 0x70:
		param = protocol2str(hdr);
		break;
	case 0x40:
	case 0x50:
	case 0xa0:
	case 0xb0:
		param = report2str(hdr);
		break;
	default:
		param = "";
		break;
	}

	p_indent(level, frm);

	printf("HIDP: %s: %s\n", type2str(hdr), param);

	raw_dump(level, frm);
}
