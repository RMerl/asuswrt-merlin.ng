/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2002-2011  Marcel Holtmann <marcel@holtmann.org>
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

#define TABLE_SIZE 10

static struct {
	uint16_t handle;
	uint16_t cid;
	struct frame msg[16];
} table[TABLE_SIZE];

static void add_segment(uint8_t bid, struct frame *frm, int len)
{
	uint16_t handle = frm->handle, cid = frm->cid;
	struct frame *msg;
	void *data;
	int i, pos = -1;

	if (bid > 15)
		return;

	for (i = 0; i < TABLE_SIZE; i++) {
		if (table[i].handle == handle && table[i].cid == cid) {
			pos = i;
			break;
		}

		if (pos < 0 && !table[i].handle && !table[i].cid)
			pos = i;
	}

	if (pos < 0)
		return;

	table[pos].handle = handle;
	table[pos].cid    = cid;
	msg = &table[pos].msg[bid];

	data = malloc(msg->data_len + len);
	if (!data)
		return;

	if (msg->data_len > 0)
		memcpy(data, msg->data, msg->data_len);

	memcpy(data + msg->data_len, frm->ptr, len);
	free(msg->data);
	msg->data = data;
	msg->data_len += len;
	msg->ptr = msg->data;
	msg->len = msg->data_len;
	msg->in  = frm->in;
	msg->ts  = frm->ts;
	msg->handle = handle;
	msg->cid    = cid;
}

static void free_segment(uint8_t bid, struct frame *frm)
{
	uint16_t handle = frm->handle, cid = frm->cid;
	struct frame *msg;
	int i, len = 0, pos = -1;

	if (bid > 15)
		return;

	for (i = 0; i < TABLE_SIZE; i++)
		if (table[i].handle == handle && table[i].cid == cid) {
			pos = i;
			break;
		}

	if (pos < 0)
		return;

	msg = &table[pos].msg[bid];

	if (msg->data)
		free(msg->data);

	msg->data = NULL;
	msg->data_len = 0;

	for (i = 0; i < 16; i++)
		len += table[pos].msg[i].data_len;

	if (!len) {
		table[pos].handle = 0;
		table[pos].cid = 0;
	}
}

static struct frame *get_segment(uint8_t bid, struct frame *frm)
{
	uint16_t handle = frm->handle, cid = frm->cid;
	int i;

	if (bid > 15)
		return NULL;

	for (i = 0; i < TABLE_SIZE; i++)
		if (table[i].handle == handle && table[i].cid == cid)
			return &table[i].msg[bid];

	return NULL;
}

static char *bst2str(uint8_t bst)
{
	switch (bst) {
	case 0x00:
		return "complete CAPI Message";
	case 0x01:
		return "segmented CAPI Message";
	case 0x02:
		return "error";
	case 0x03:
		return "reserved";
	default:
		return "unknown";
	}
}

void cmtp_dump(int level, struct frame *frm)
{
	struct frame *msg;
	uint8_t hdr, bid;
	uint16_t len;

	while (frm->len > 0) {

		hdr = p_get_u8(frm);
		bid = (hdr & 0x3c) >> 2;

		switch ((hdr & 0xc0) >> 6) {
		case 0x01:
			len = p_get_u8(frm);
			break;
		case 0x02:
			len = htons(p_get_u16(frm));
			break;
		default:
			len = 0;
			break;
		}

		p_indent(level, frm);

		printf("CMTP: %s: id %d len %d\n", bst2str(hdr & 0x03), bid, len);

		switch (hdr & 0x03) {
		case 0x00:
			add_segment(bid, frm, len);
			msg = get_segment(bid, frm);
			if (!msg)
				break;

			if (!p_filter(FILT_CAPI))
				capi_dump(level + 1, msg);
			else
				raw_dump(level, msg);

			free_segment(bid, frm);
			break;
		case 0x01:
			add_segment(bid, frm, len);
			break;
		default:
			free_segment(bid, frm);
			break;
		}

		frm->ptr += len;
		frm->len -= len;
	}
}
