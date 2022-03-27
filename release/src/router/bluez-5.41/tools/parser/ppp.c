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

#define PPP_U8(frm)  (get_u8(frm))
#define PPP_U16(frm) (btohs(htons(get_u16(frm))))
#define PPP_U32(frm) (btohl(htonl(get_u32(frm))))

static int ppp_traffic = 0;

static unsigned char ppp_magic1[] = { 0x7e, 0xff, 0x03, 0xc0, 0x21 };
static unsigned char ppp_magic2[] = { 0x7e, 0xff, 0x7d, 0x23, 0xc0, 0x21 };
static unsigned char ppp_magic3[] = { 0x7e, 0x7d, 0xdf, 0x7d, 0x23, 0xc0, 0x21 };

static inline int check_for_ppp_traffic(unsigned char *data, int size)
{
	unsigned int i;

	for (i = 0; i < size - sizeof(ppp_magic1); i++)
		if (!memcmp(data + i, ppp_magic1, sizeof(ppp_magic1))) {
			ppp_traffic = 1;
			return i;
		}

	for (i = 0; i < size - sizeof(ppp_magic2); i++)
		if (!memcmp(data + i, ppp_magic2, sizeof(ppp_magic2))) {
			ppp_traffic = 1;
			return i;
		}

	for (i = 0; i < size - sizeof(ppp_magic3); i++)
		if (!memcmp(data + i, ppp_magic3, sizeof(ppp_magic3))) {
			ppp_traffic = 1;
			return i;
		}

	return -1;
}

static inline char *dir2str(uint8_t in)
{
	return in ? "DCE" : "DTE";
}

static inline char *proto2str(uint16_t proto)
{
	switch (proto) {
	case 0x0001:
		return "Padding Protocol";
	case 0x0021:
		return "IP";
	case 0x8021:
		return "IP Control Protocol";
	case 0x80fd:
		return "Compression Control Protocol";
	case 0xc021:
		return "Link Control Protocol";
	case 0xc023:
		return "Password Authentication Protocol";
	case 0xc025:
		return "Link Quality Report";
	case 0xc223:
		return "Challenge Handshake Authentication Protocol";
	default:
		return "Unknown Protocol";
	}
}

static void hdlc_dump(int level, struct frame *frm)
{
	uint8_t addr = p_get_u8(frm);
	uint8_t ctrl = p_get_u8(frm);
	uint16_t fcs, proto;

	fcs = get_unaligned((uint16_t *) (frm->ptr + frm->len - 2));
	frm->len -= 2;

	p_indent(level, frm);

	if (addr != 0xff || ctrl != 0x03) {
		frm->ptr -= 2;
		frm->len += 2;
		printf("HDLC: %s: len %d fcs 0x%04x\n",
				dir2str(frm->in), frm->len, fcs);
	} else
		printf("HDLC: %s: addr 0x%02x ctrl 0x%02x len %d fcs 0x%04x\n",
				dir2str(frm->in), addr, ctrl, frm->len, fcs);

	if (*((uint8_t *) frm->ptr) & 0x80)
		proto = p_get_u16(frm);
	else
		proto = p_get_u8(frm);

	p_indent(level + 1, frm);
	printf("PPP: %s (0x%04x): len %d\n", proto2str(proto), proto, frm->len);

	raw_dump(level + 1, frm);
}

static inline void unslip_frame(int level, struct frame *frm, int len)
{
	struct frame msg;
	unsigned char *data, *ptr;
	int i, p = 0;

	data = malloc(len * 2);
	if (!data)
		return;

	ptr = frm->ptr;

	for (i = 0; i < len; i++) {
		if (ptr[i] == 0x7d) {
			data[p++] = ptr[i + 1] ^ 0x20;
			i++;
		} else
			data[p++] = ptr[i];
	}

	memset(&msg, 0, sizeof(msg));
	msg.data     = data;
	msg.data_len = len * 2;
	msg.ptr      = msg.data;
	msg.len      = p;
	msg.in       = frm->in;
	msg.ts       = frm->ts;
	msg.handle   = frm->handle;
	msg.cid      = frm->cid;

	hdlc_dump(level, &msg);

	free(data);
}

void ppp_dump(int level, struct frame *frm)
{
	void *ptr, *end;
	int err, len, pos = 0;

	if (frm->pppdump_fd > fileno(stderr)) {
		unsigned char id;
		uint16_t len = htons(frm->len);
		uint32_t ts = htonl(frm->ts.tv_sec & 0xffffffff);

		id = 0x07;
		err = write(frm->pppdump_fd, &id, 1);
		if (err < 0)
			return;

		err = write(frm->pppdump_fd, &ts, 4);
		if (err < 0)
			return;

		id = frm->in ? 0x02 : 0x01;
		err = write(frm->pppdump_fd, &id, 1);
		if (err < 0)
			return;
		err = write(frm->pppdump_fd, &len, 2);
		if (err < 0)
			return;
		err = write(frm->pppdump_fd, frm->ptr, frm->len);
		if (err < 0)
			return;
	}

	if (!ppp_traffic) {
		pos = check_for_ppp_traffic(frm->ptr, frm->len);
		if (pos < 0) {
			raw_dump(level, frm);
			return;
		}

		if (pos > 0) {
			raw_ndump(level, frm, pos);
			frm->ptr += pos;
			frm->len -= pos;
		}
	}

	frm = add_frame(frm);

	while (frm->len > 0) {
		ptr = memchr(frm->ptr, 0x7e, frm->len);
		if (!ptr)
			break;

		if (frm->ptr != ptr) {
			frm->len -= (ptr - frm->ptr);
			frm->ptr = ptr;
		}

		end = memchr(frm->ptr + 1, 0x7e, frm->len - 1);
		if (!end)
			break;

		len = end - ptr - 1;

		frm->ptr++;
		frm->len--;

		if (len > 0) {
			unslip_frame(level, frm, len);

			frm->ptr += len;
			frm->len -= len;
		}
	}
}
