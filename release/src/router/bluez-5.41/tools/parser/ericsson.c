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

#include "parser.h"

void ericsson_dump(int level, struct frame *frm)
{
	uint8_t event = p_get_u8(frm);
	uint8_t *buf = (uint8_t *) frm->ptr;

	if (event != 0x10) {
		p_indent(level, frm);
		printf("Ericsson: event 0x%2.2x\n", event);
		raw_dump(level, frm);
	}

	frm->master = !(buf[0] & 0x01);
	frm->handle = buf[1] | (buf[2] << 8);

	buf[5] = (buf[5] << 1) | (buf[3] & 0x01);

	frm->ptr += 5;
	frm->len -= 5;

	lmp_dump(level, frm);
}
