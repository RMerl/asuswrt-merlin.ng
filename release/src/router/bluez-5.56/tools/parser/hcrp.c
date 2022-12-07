// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2011  Marcel Holtmann <marcel@holtmann.org>
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

static char *pid2str(uint16_t pid)
{
	switch (pid) {
	case 0x0001:
		return "CreditGrant";
	case 0x0002:
		return "CreditRequest";
	case 0x0003:
		return "CreditReturn";
	case 0x0004:
		return "CreditQuery";
	case 0x0005:
		return "GetLPTStatus";
	case 0x0006:
		return "Get1284ID";
	case 0x0007:
		return "SoftReset";
	case 0x0008:
		return "HardRest";
	case 0x0009:
		return "RegisterNotification";
	case 0x000A:
		return "NotificationConnectionAlive";
	default:
		return "Reserved";
	}
}

static char *status2str(uint16_t status)
{
	switch (status) {
	case 0x0000:
		return "Feature unsupported";
	case 0x0001:
		return "Success";
	case 0x0002:
		return "Credit synchronization error";
	case 0xFFFF:
		return "Generic error";
	default:
		return "Unknown";
	}
}

void hcrp_dump(int level, struct frame *frm)
{
	uint16_t pid, tid, plen, status;
	uint32_t credits;

	pid = p_get_u16(frm);
	tid = p_get_u16(frm);
	plen = p_get_u16(frm);

	p_indent(level, frm);

	printf("HCRP %s %s: tid 0x%x plen %d",
			pid2str(pid), frm->in ? "rsp" : "cmd",  tid, plen);

	if (frm->in) {
		status = p_get_u16(frm);
		printf(" status %d (%s)\n", status, status2str(status));
	} else
		printf("\n");

	if (pid == 0x0001 && !frm->in) {
		credits = p_get_u32(frm);
		p_indent(level + 1, frm);
		printf("credits %d\n", credits);
	}

	if (pid == 0x0002 && frm->in) {
		credits = p_get_u32(frm);
		p_indent(level + 1, frm);
		printf("credits %d\n", credits);
	}

	raw_dump(level + 1, frm);
}
