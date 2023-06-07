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
