/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2011  Marcel Holtmann <marcel@holtmann.org>
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
