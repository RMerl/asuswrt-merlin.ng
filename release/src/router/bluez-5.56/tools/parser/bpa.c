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

#define BPA_U8(frm)  (p_get_u8(frm))
#define BPA_U16(frm) (btohs(htons(p_get_u16(frm))))
#define BPA_U32(frm) (btohl(htonl(p_get_u32(frm))))

void bpa_dump(int level, struct frame *frm)
{
	uint8_t id, status, channel;
	uint16_t num, len;
	uint32_t time;

	id = p_get_u8(frm);
	num = p_get_u16(frm);
	len = BPA_U16(frm);

	status  = p_get_u8(frm);
	time    = p_get_u32(frm);
	channel = p_get_u8(frm);

	p_indent(level, frm);
	printf("BPA: id %d num %d len %u status 0x%02x time %d channel %d\n",
		id, num, len, status, time, channel);

	raw_dump(level, frm);
}
