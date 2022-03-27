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
#include "sdp.h"

static char *pt2str(uint8_t hdr)
{
	switch (hdr & 0x0c) {
	case 0x00:
		return "";
	case 0x04:
		return "Start";
	case 0x08:
		return "Cont";
	case 0x0c:
		return "End";
	default:
		return "Unk";
	}
}

void avctp_dump(int level, struct frame *frm, uint16_t psm)
{
	uint8_t hdr;
	uint16_t pid;

	p_indent(level, frm);

	hdr = p_get_u8(frm);
	pid = p_get_u16(frm);

	printf("AVCTP %s: %s %s: pt 0x%02x transaction %d pid 0x%04x\n",
				psm == 23 ? "Control" : "Browsing",
				hdr & 0x02 ? "Response" : "Command",
				pt2str(hdr), hdr & 0x0c, hdr >> 4, pid);

	if (pid == SDP_UUID_AV_REMOTE || pid == SDP_UUID_AV_REMOTE_TARGET)
		avrcp_dump(level + 1, frm, hdr, psm);
	else
		raw_dump(level + 1, frm);
}
