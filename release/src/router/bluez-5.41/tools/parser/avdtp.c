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

static char *si2str(uint8_t si)
{
	switch (si & 0x7f) {
	case 0x01:
		return "Discover";
	case 0x02:
		return "Capabilities";
	case 0x03:
		return "Set config";
	case 0x04:
		return "Get config";
	case 0x05:
		return "Reconfigure";
	case 0x06:
		return "Open";
	case 0x07:
		return "Start";
	case 0x08:
		return "Close";
	case 0x09:
		return "Suspend";
	case 0x0a:
		return "Abort";
	case 0x0b:
		return "Security";
	case 0x0c:
		return "All Capabilities";
	case 0x0d:
		return "Delay Report";
	default:
		return "Unknown";
	}
}

static char *pt2str(uint8_t hdr)
{
	switch (hdr & 0x0c) {
	case 0x00:
		return "Single";
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

static char *mt2str(uint8_t hdr)
{
	switch (hdr & 0x03) {
	case 0x00:
		return "cmd";
	case 0x02:
		return "rsp";
	case 0x03:
		return "rej";
	default:
		return "rfd";
	}
}

static char *media2str(uint8_t type)
{
	switch (type) {
	case 0:
		return "Audio";
	case 1:
		return "Video";
	case 2:
		return "Multimedia";
	default:
		return "Reserved";
	}
}

static char *codec2str(uint8_t type, uint8_t codec)
{
	switch (type) {
	case 0:
		switch (codec) {
		case 0:
			return "SBC";
		case 1:
			return "MPEG-1,2 Audio";
		case 2:
			return "MPEG-2,4 AAC";
		case 4:
			return "ATRAC family";
		case 255:
			return "non-A2DP";
		default:
			return "Reserved";
		}
		break;
	case 1:
		switch (codec) {
		case 1:
			return "H.263 baseline";
		case 2:
			return "MPEG-4 Visual Simple Profile";
		case 3:
			return "H.263 profile 3";
		case 4:
			return "H.263 profile 8";
		case 255:
			return "Non-VDP";
		default:
			return "Reserved";
		}
		break;
	}
	return "Unknown";
}

static char *vndcodec2str(uint32_t vendor, uint16_t vndcodec)
{
	if (vendor == 0x0000004f && vndcodec == 0x0001)
		return "aptX";
	else if (vendor == 0x0000012d && vndcodec == 0x00aa)
		return "LDAC";
	return "Unknown";
}

static char *cat2str(uint8_t cat)
{
	switch (cat) {
	case 1:
		return "Media Transport";
	case 2:
		return "Reporting";
	case 3:
		return "Recovery";
	case 4:
		return "Content Protection";
	case 5:
		return "Header Compression";
	case 6:
		return "Multiplexing";
	case 7:
		return "Media Codec";
	case 8:
		return "Delay Reporting";
	default:
		return "Reserved";
	}
}

static void errorcode(int level, struct frame *frm)
{
	uint8_t code;

	p_indent(level, frm);
	code = p_get_u8(frm);
	printf("Error code %d\n", code);
}

static void acp_seid(int level, struct frame *frm)
{
	uint8_t seid;

	p_indent(level, frm);
	seid = p_get_u8(frm);
	printf("ACP SEID %d\n", seid >> 2);
}

static void acp_int_seid(int level, struct frame *frm)
{
	uint8_t acp_seid, int_seid;

	p_indent(level, frm);
	acp_seid = p_get_u8(frm);
	int_seid = p_get_u8(frm);
	printf("ACP SEID %d - INT SEID %d\n", acp_seid >> 2, int_seid >> 2);
}

static void capabilities(int level, struct frame *frm)
{
	uint8_t cat, len;

	while (frm->len > 1) {
		p_indent(level, frm);
		cat = p_get_u8(frm);
		len = p_get_u8(frm);

		if (cat == 7) {
			uint8_t type, codec;
			uint16_t tmp, freq, vndcodec = 0;
			uint32_t bitrate, vendor = 0;
			int i;

			type  = p_get_u8(frm);
			codec = p_get_u8(frm);

			if (codec == 255) {
				vendor = btohl(htonl(p_get_u32(frm)));
				vndcodec = btohs(htons(p_get_u16(frm)));

				printf("%s - %s (%s)\n", cat2str(cat),
						codec2str(type, codec),
						vndcodec2str(vendor, vndcodec));
			} else {
				printf("%s - %s\n", cat2str(cat),
							codec2str(type, codec));
			}

			switch (codec) {
			case 0:
				tmp = p_get_u8(frm);
				p_indent(level + 1, frm);
				if (tmp & 0x80)
					printf("16kHz ");
				if (tmp & 0x40)
					printf("32kHz ");
				if (tmp & 0x20)
					printf("44.1kHz ");
				if (tmp & 0x10)
					printf("48kHz ");
				printf("\n");
				p_indent(level + 1, frm);
				if (tmp & 0x08)
					printf("Mono ");
				if (tmp & 0x04)
					printf("DualChannel ");
				if (tmp & 0x02)
					printf("Stereo ");
				if (tmp & 0x01)
					printf("JointStereo ");
				printf("\n");
				tmp = p_get_u8(frm);
				p_indent(level + 1, frm);
				if (tmp & 0x80)
					printf("4 ");
				if (tmp & 0x40)
					printf("8 ");
				if (tmp & 0x20)
					printf("12 ");
				if (tmp & 0x10)
					printf("16 ");
				printf("Blocks\n");
				p_indent(level + 1, frm);
				if (tmp & 0x08)
					printf("4 ");
				if (tmp & 0x04)
					printf("8 ");
				printf("Subbands\n");
				p_indent(level + 1, frm);
				if (tmp & 0x02)
					printf("SNR ");
				if (tmp & 0x01)
					printf("Loudness ");
				printf("\n");
				tmp = p_get_u8(frm);
				p_indent(level + 1, frm);
				printf("Bitpool Range %d-%d\n", tmp, p_get_u8(frm));
				break;
			case 1:
				tmp = p_get_u8(frm);
				p_indent(level + 1, frm);
				printf("Layers: ");
				if (tmp & 0x80)
					printf("1 ");
				if (tmp & 0x40)
					printf("2 ");
				if (tmp & 0x20)
					printf("3 ");
				printf("\n");
				p_indent(level + 1, frm);
				printf("CRC Protection: %s\n",
						tmp & 0x10 ? "Yes" : "No");
				p_indent(level + 1, frm);
				if (tmp & 0x08)
					printf("Mono ");
				if (tmp & 0x04)
					printf("DualChannel ");
				if (tmp & 0x02)
					printf("Stereo ");
				if (tmp & 0x01)
					printf("JointStereo ");
				printf("\n");
				tmp = p_get_u8(frm);
				p_indent(level + 1, frm);
				printf("Media Payload Format: RFC-2250 %s\n",
						tmp & 0x40 ? "RFC-3119" : "");
				p_indent(level + 1, frm);
				if (tmp & 0x20)
					printf("16kHz ");
				if (tmp & 0x10)
					printf("22.05kHz ");
				if (tmp & 0x08)
					printf("24kHz ");
				if (tmp & 0x04)
					printf("32kHz ");
				if (tmp & 0x02)
					printf("44.1kHz ");
				if (tmp & 0x01)
					printf("48kHz ");
				printf("\n");
				tmp = p_get_u16(frm);
				p_indent(level + 1, frm);
				printf("VBR: %s\n",
						tmp & 0x8000 ? "Yes" : "No");
				p_indent(level + 1, frm);
				printf("Bit Rate Indexes: ");
				if (tmp & 0x8000) {
					printf("n/a");
				} else {
					for (i = 0; i < 15; i++, tmp >>= 1)
						if (tmp & 0x0001)
							printf("%d ", i);
				}
				printf("\n");
				break;
			case 2:
				tmp = p_get_u8(frm);
				p_indent(level + 1, frm);
				if (tmp & 0x80)
					printf("MPEG-2 AAC LC ");
				if (tmp & 0x40)
					printf("MPEG-4 AAC LC ");
				if (tmp & 0x20)
					printf("MPEG-4 AAC LTP ");
				if (tmp & 0x10)
					printf("MPEG-4 AAC scalable ");
				printf("\n");
				tmp = p_get_u16(frm);
				freq = tmp >> 4;
				p_indent(level + 1, frm);
				if (freq & 0x0800)
					printf("8kHz ");
				if (freq & 0x0400)
					printf("11.025kHz ");
				if (freq & 0x0200)
					printf("12kHz ");
				if (freq & 0x0100)
					printf("16kHz ");
				if (freq & 0x0080)
					printf("22.05kHz ");
				if (freq & 0x0040)
					printf("24kHz ");
				if (freq & 0x0020)
					printf("32kHz ");
				if (freq & 0x0010)
					printf("44.1kHz ");
				if (freq & 0x0008)
					printf("48kHz ");
				if (freq & 0x0004)
					printf("64kHz ");
				if (freq & 0x0002)
					printf("88.2kHz ");
				if (freq & 0x0001)
					printf("96kHz ");
				printf("\n");
				tmp >>= 2;
				p_indent(level + 1, frm);
				if (tmp & 0x02)
					printf("1 ");
				if (tmp & 0x01)
					printf("2 ");
				printf("Channels\n");
				tmp = p_get_u8(frm);
				bitrate = ((tmp & 0x7f) << 16) | p_get_u16(frm);
				p_indent(level + 1, frm);
				printf("%ubps ", bitrate);
				printf("%s\n", tmp & 0x80 ? "VBR" : "");
				break;
			case 255:
				if (vendor == 0x0000004f &&
							vndcodec == 0x0001) {
					tmp = p_get_u8(frm);
					p_indent(level + 1, frm);
					if (tmp & 0x80)
						printf("16kHz ");
					if (tmp & 0x40)
						printf("32kHz ");
					if (tmp & 0x20)
						printf("44.1kHz ");
					if (tmp & 0x10)
						printf("48kHz ");
					printf("\n");
					p_indent(level + 1, frm);
					if (tmp & 0x02)
						printf("Stereo ");
					if (tmp & 0x01)
						printf("Mono ");
					printf("\n");
					break;
				} else {
					hex_dump(level + 1, frm, len - 8);
					frm->ptr += (len - 8);
					frm->len -= (len - 8);
				}
				break;
			default:
				hex_dump(level + 1, frm, len - 2);
				frm->ptr += (len - 2);
				frm->len -= (len - 2);
				break;
			}
		} else {
			printf("%s\n", cat2str(cat));
			hex_dump(level + 1, frm, len);

			frm->ptr += len;
			frm->len -= len;
		}
	}
}

static inline void discover(int level, uint8_t hdr, struct frame *frm)
{
	uint8_t seid, type;

	switch (hdr & 0x03) {
	case 0x02:
		while (frm->len > 1) {
			p_indent(level, frm);
			seid = p_get_u8(frm);
			type = p_get_u8(frm);
			printf("ACP SEID %d - %s %s%s\n",
				seid >> 2, media2str(type >> 4),
				type & 0x08 ? "Sink" : "Source",
				seid & 0x02 ? " (InUse)" : "");
		}
		break;
	case 0x03:
		errorcode(level, frm);
		break;
	}
}

static inline void get_capabilities(int level, uint8_t hdr, struct frame *frm)
{
	switch (hdr & 0x03) {
	case 0x00:
		acp_seid(level, frm);
		break;
	case 0x02:
		capabilities(level, frm);
		break;
	case 0x03:
		errorcode(level, frm);
		break;
	}
}

static inline void set_configuration(int level, uint8_t hdr, struct frame *frm)
{
	uint8_t cat;

	switch (hdr & 0x03) {
	case 0x00:
		acp_int_seid(level, frm);
		capabilities(level, frm);
		break;
	case 0x03:
		p_indent(level, frm);
		cat = p_get_u8(frm);
		printf("%s\n", cat2str(cat));
		errorcode(level, frm);
		break;
	}
}

static inline void get_configuration(int level, uint8_t hdr, struct frame *frm)
{
	switch (hdr & 0x03) {
	case 0x00:
		acp_seid(level, frm);
	case 0x02:
		capabilities(level, frm);
		break;
	case 0x03:
		errorcode(level, frm);
		break;
	}
}

static inline void reconfigure(int level, uint8_t hdr, struct frame *frm)
{
	uint8_t cat;

	switch (hdr & 0x03) {
	case 0x00:
		acp_seid(level, frm);
		capabilities(level, frm);
		break;
	case 0x03:
		p_indent(level, frm);
		cat = p_get_u8(frm);
		printf("%s\n", cat2str(cat));
		errorcode(level, frm);
		break;
	}
}

static inline void open_close_stream(int level, uint8_t hdr, struct frame *frm)
{
	switch (hdr & 0x03) {
	case 0x00:
		acp_seid(level, frm);
		break;
	case 0x03:
		errorcode(level, frm);
		break;
	}
}

static inline void start_suspend_stream(int level, uint8_t hdr, struct frame *frm)
{
	switch (hdr & 0x03) {
	case 0x00:
		while (frm->len > 0)
			acp_seid(level, frm);
		break;
	case 0x03:
		acp_seid(level, frm);
		errorcode(level, frm);
		break;
	}
}

static inline void abort_streaming(int level, uint8_t hdr, struct frame *frm)
{
	switch (hdr & 0x03) {
	case 0x00:
		acp_seid(level, frm);
		break;
	}
}

static inline void security(int level, uint8_t hdr, struct frame *frm)
{
	switch (hdr & 0x03) {
	case 0x00:
		acp_seid(level, frm);
	case 0x02:
		hex_dump(level + 1, frm, frm->len);
		frm->ptr += frm->len;
		frm->len = 0;
		break;
	case 0x03:
		errorcode(level, frm);
		break;
	}
}

static inline void delay_report(int level, uint8_t hdr, struct frame *frm)
{
	uint8_t seid;
	uint16_t delay;

	switch (hdr & 0x03) {
	case 0x00:
		p_indent(level, frm);
		seid = p_get_u8(frm);
		delay = p_get_u16(frm);
		printf("ACP SEID %d delay %u.%ums\n", seid >> 2,
						delay / 10, delay % 10);
		break;
	case 0x03:
		errorcode(level, frm);
		break;
	}
}

void avdtp_dump(int level, struct frame *frm)
{
	uint8_t hdr, sid, nsp, type;
	uint16_t seqn;
	uint32_t time, ssrc;

	switch (frm->num) {
	case 1:
		p_indent(level, frm);
		hdr = p_get_u8(frm);

		nsp = (hdr & 0x0c) == 0x04 ? p_get_u8(frm) : 0;
		sid = hdr & 0x08 ? 0x00 : p_get_u8(frm);

		printf("AVDTP(s): %s %s: transaction %d nsp 0x%02x\n",
			hdr & 0x08 ? pt2str(hdr) : si2str(sid),
			mt2str(hdr), hdr >> 4, nsp);

		switch (sid & 0x7f) {
		case 0x01:
			discover(level + 1, hdr, frm);
			break;
		case 0x02:
		case 0x0c:
			get_capabilities(level + 1, hdr, frm);
			break;
		case 0x03:
			set_configuration(level + 1, hdr, frm);
			break;
		case 0x04:
			get_configuration(level + 1, hdr, frm);
			break;
		case 0x05:
			reconfigure(level + 1, hdr, frm);
			break;
		case 0x06:
			open_close_stream(level + 1, hdr, frm);
			break;
		case 0x07:
			start_suspend_stream(level + 1, hdr, frm);
			break;
		case 0x08:
			open_close_stream(level + 1, hdr, frm);
			break;
		case 0x09:
			start_suspend_stream(level + 1, hdr, frm);
			break;
		case 0x0a:
			abort_streaming(level + 1, hdr, frm);
			break;
		case 0x0b:
			security(level + 1, hdr, frm);
			break;
		case 0x0d:
			delay_report(level + 1, hdr, frm);
			break;
		}

		break;

	case 2:
		p_indent(level, frm);
		hdr  = p_get_u8(frm);
		type = p_get_u8(frm);
		seqn = p_get_u16(frm);
		time = p_get_u32(frm);
		ssrc = p_get_u32(frm);

		printf("AVDTP(m): ver %d %s%scc %d %spt %d seqn %d time %d ssrc %d\n",
			hdr >> 6, hdr & 0x20 ? "pad " : "", hdr & 0x10 ? "ext " : "",
			hdr & 0xf, type & 0x80 ? "mark " : "", type & 0x7f, seqn, time, ssrc);
		break;
	}

	raw_dump(level, frm);
}
