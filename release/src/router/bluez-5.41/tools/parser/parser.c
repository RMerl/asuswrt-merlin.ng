/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2000-2002  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2003-2011  Marcel Holtmann <marcel@holtmann.org>
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
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "rfcomm.h"

struct parser_t parser;

void init_parser(unsigned long flags, unsigned long filter,
		unsigned short defpsm, unsigned short defcompid,
		int pppdump_fd, int audio_fd)
{
	if ((flags & DUMP_RAW) && !(flags & DUMP_TYPE_MASK))
		flags |= DUMP_HEX;

	parser.flags      = flags;
	parser.filter     = filter;
	parser.defpsm     = defpsm;
	parser.defcompid  = defcompid;
	parser.state      = 0;
	parser.pppdump_fd = pppdump_fd;
	parser.audio_fd   = audio_fd;
}

#define PROTO_TABLE_SIZE 20

static struct {
	uint16_t handle;
	uint16_t psm;
	uint8_t  channel;
	uint32_t proto;
} proto_table[PROTO_TABLE_SIZE];

void set_proto(uint16_t handle, uint16_t psm, uint8_t channel, uint32_t proto)
{
	int i, pos = -1;

	if (psm > 0 && psm < 0x1000 && !channel)
		return;

	if (!psm && channel)
		psm = RFCOMM_PSM; 

	for (i = 0; i < PROTO_TABLE_SIZE; i++) {
		if (proto_table[i].handle == handle && proto_table[i].psm == psm && proto_table[i].channel == channel) {
			pos = i;
			break;
		}

		if (pos < 0 && !proto_table[i].handle && !proto_table[i].psm && !proto_table[i].channel)
			pos = i;
	}

	if (pos < 0)
		return;

	proto_table[pos].handle  = handle;
	proto_table[pos].psm     = psm;
	proto_table[pos].channel = channel;
	proto_table[pos].proto   = proto;
}

uint32_t get_proto(uint16_t handle, uint16_t psm, uint8_t channel)
{
	int i, pos = -1;

	if (!psm && channel)
		psm = RFCOMM_PSM;

	for (i = 0; i < PROTO_TABLE_SIZE; i++) {
		if (proto_table[i].handle == handle && proto_table[i].psm == psm && proto_table[i].channel == channel)
			return proto_table[i].proto;

		if (!proto_table[i].handle) {
			if (proto_table[i].psm == psm && proto_table[i].channel == channel)
				pos = i;
		}
	}

	return (pos < 0) ? 0 : proto_table[pos].proto;
}

#define FRAME_TABLE_SIZE 20

static struct {
	uint16_t handle;
	uint8_t dlci;
	uint8_t opcode;
	uint8_t status;
	struct frame frm;
} frame_table[FRAME_TABLE_SIZE];

void del_frame(uint16_t handle, uint8_t dlci)
{
	int i;

	for (i = 0; i < FRAME_TABLE_SIZE; i++)
		if (frame_table[i].handle == handle &&
					frame_table[i].dlci == dlci) {
			frame_table[i].handle = 0;
			frame_table[i].dlci   = 0;
			frame_table[i].opcode = 0;
			frame_table[i].status = 0;
			if (frame_table[i].frm.data)
				free(frame_table[i].frm.data);
			memset(&frame_table[i].frm, 0, sizeof(struct frame));
			break;
		}
}

struct frame *add_frame(struct frame *frm)
{
	struct frame *fr;
	void *data;
	int i, pos = -1;

	for (i = 0; i < FRAME_TABLE_SIZE; i++) {
		if (frame_table[i].handle == frm->handle &&
					frame_table[i].dlci == frm->dlci) {
			pos = i;
			break;
		}

		if (pos < 0 && !frame_table[i].handle && !frame_table[i].dlci)
			pos = i;
	}

	if (pos < 0)
		return frm;

	frame_table[pos].handle = frm->handle;
	frame_table[pos].dlci   = frm->dlci;
	fr = &frame_table[pos].frm;

	data = malloc(fr->len + frm->len);
	if (!data) {
		perror("Can't allocate frame stream buffer");
		del_frame(frm->handle, frm->dlci);
		return frm;
	}

	if (fr->len > 0)
		memcpy(data, fr->ptr, fr->len);

	if (frm->len > 0)
		memcpy(data + fr->len, frm->ptr, frm->len);

	if (fr->data)
		free(fr->data);

	fr->data       = data;
	fr->data_len   = fr->len + frm->len;
	fr->len        = fr->data_len;
	fr->ptr        = fr->data;
	fr->dev_id     = frm->dev_id;
	fr->in         = frm->in;
	fr->ts         = frm->ts;
	fr->handle     = frm->handle;
	fr->cid        = frm->cid;
	fr->num        = frm->num;
	fr->dlci       = frm->dlci;
	fr->channel    = frm->channel;
	fr->pppdump_fd = frm->pppdump_fd;
	fr->audio_fd   = frm->audio_fd;

	return fr;
}

uint8_t get_opcode(uint16_t handle, uint8_t dlci)
{
	int i;

	for (i = 0; i < FRAME_TABLE_SIZE; i++)
		if (frame_table[i].handle == handle &&
					frame_table[i].dlci == dlci)
			return frame_table[i].opcode;

	return 0x00;
}

void set_opcode(uint16_t handle, uint8_t dlci, uint8_t opcode)
{
	int i;

	for (i = 0; i < FRAME_TABLE_SIZE; i++)
		if (frame_table[i].handle == handle && 
					frame_table[i].dlci == dlci) {
			frame_table[i].opcode = opcode;
			break;
		}
}

uint8_t get_status(uint16_t handle, uint8_t dlci)
{
	int i;

	for (i = 0; i < FRAME_TABLE_SIZE; i++)
		if (frame_table[i].handle == handle &&
					frame_table[i].dlci == dlci)
			return frame_table[i].status;

	return 0x00;
}

void set_status(uint16_t handle, uint8_t dlci, uint8_t status)
{
	int i;

	for (i = 0; i < FRAME_TABLE_SIZE; i++)
		if (frame_table[i].handle == handle &&
					frame_table[i].dlci == dlci) {
			frame_table[i].status = status;
			break;
		}
}

void ascii_dump(int level, struct frame *frm, int num)
{
	unsigned char *buf = frm->ptr;
	register int i, n;

	if ((num < 0) || (num > (int) frm->len))
		num = frm->len;

	for (i = 0, n = 1; i < num; i++, n++) {
		if (n == 1)
			p_indent(level, frm);
		printf("%1c ", isprint(buf[i]) ? buf[i] : '.');
		if (n == DUMP_WIDTH) {
			printf("\n");
			n = 0;
		}
	}
	if (i && n != 1)
		printf("\n");
}

void hex_dump(int level, struct frame *frm, int num)
{
	unsigned char *buf = frm->ptr;
	register int i, n;

	if ((num < 0) || (num > (int) frm->len))
		num = frm->len;

	for (i = 0, n = 1; i < num; i++, n++) {
		if (n == 1)
			p_indent(level, frm);
		printf("%2.2X ", buf[i]);
		if (n == DUMP_WIDTH) {
			printf("\n");
			n = 0;
		}
	}
	if (i && n != 1)
		printf("\n");
}

void ext_dump(int level, struct frame *frm, int num)
{
	unsigned char *buf = frm->ptr;
	register int i, n = 0, size;

	if ((num < 0) || (num > (int) frm->len))
		num = frm->len;

	while (num > 0) {
		p_indent(level, frm);
		printf("%04x: ", n);

		size = num > 16 ? 16 : num;

		for (i = 0; i < size; i++)
			printf("%02x%s", buf[i], (i + 1) % 8 ? " " : "  ");
		for (i = size; i < 16; i++)
			printf("  %s", (i + 1) % 8 ? " " : "  ");

		for (i = 0; i < size; i++)
			printf("%1c", isprint(buf[i]) ? buf[i] : '.');
		printf("\n");

		buf  += size;
		num  -= size;
		n    += size;
	}
}

void raw_ndump(int level, struct frame *frm, int num)
{
	if (!frm->len)
		return;

	switch (parser.flags & DUMP_TYPE_MASK) {
	case DUMP_ASCII:
		ascii_dump(level, frm, num);
		break;

	case DUMP_HEX:
		hex_dump(level, frm, num);
		break;

	case DUMP_EXT:
		ext_dump(level, frm, num);
		break;
	}
}

void raw_dump(int level, struct frame *frm)
{
	raw_ndump(level, frm, -1);
}
