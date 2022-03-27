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

#ifndef __PARSER_H
#define __PARSER_H

#include <time.h>
#include <sys/time.h>
#include <netinet/in.h>

#include "lib/bluetooth.h"
#include "src/shared/util.h"

struct frame {
	void		*data;
	uint32_t	data_len;
	void		*ptr;
	uint32_t	len;
	uint16_t	dev_id;
	uint8_t		in;
	uint8_t		master;
	uint16_t	handle;
	uint16_t	cid;
	uint16_t	num;
	uint8_t		dlci;
	uint8_t		channel;
	unsigned long	flags;
	struct timeval	ts;
	int		pppdump_fd;
	int		audio_fd;
};

/* Parser flags */
#define DUMP_WIDTH	20

#define DUMP_ASCII	0x0001
#define DUMP_HEX	0x0002
#define DUMP_EXT	0x0004
#define DUMP_RAW	0x0008
#define DUMP_BPA	0x0010
#define DUMP_TSTAMP	0x0100
#define DUMP_VERBOSE	0x0200
#define DUMP_BTSNOOP	0x1000
#define DUMP_PKTLOG	0x2000
#define DUMP_NOVENDOR	0x4000
#define DUMP_TYPE_MASK	(DUMP_ASCII | DUMP_HEX | DUMP_EXT)

/* Parser filter */
#define FILT_LMP	0x0001
#define FILT_HCI	0x0002
#define FILT_SCO	0x0004
#define FILT_L2CAP	0x0008
#define FILT_RFCOMM	0x0010
#define FILT_SDP	0x0020
#define FILT_BNEP	0x0040
#define FILT_CMTP	0x0080
#define FILT_HIDP	0x0100
#define FILT_HCRP	0x0200
#define FILT_AVDTP	0x0400
#define FILT_AVCTP	0x0800
#define FILT_ATT 	0x1000
#define FILT_SMP	0x2000
#define FILT_A2MP	0x4000

#define FILT_OBEX	0x00010000
#define FILT_CAPI	0x00020000
#define FILT_PPP	0x00040000
#define FILT_SAP	0x00080000
#define FILT_ERICSSON	0x10000000
#define FILT_CSR	0x1000000a
#define FILT_DGA	0x1000000c

#define STRUCT_OFFSET(type, member)  ((uint8_t *)&(((type *)NULL)->member) - \
                                     (uint8_t *)((type *)NULL))

#define STRUCT_END(type, member)     (STRUCT_OFFSET(type, member) + \
                                     sizeof(((type *)NULL)->member))

#define DEFAULT_COMPID	65535

struct parser_t {
	unsigned long flags;
	unsigned long filter;
	unsigned short defpsm;
	unsigned short defcompid;
	int state;
	int pppdump_fd;
	int audio_fd;
};

extern struct parser_t parser;

void init_parser(unsigned long flags, unsigned long filter,
		unsigned short defpsm, unsigned short defcompid,
		int pppdump_fd, int audio_fd);

static inline int p_filter(unsigned long f)
{
	return !(parser.filter & f);
}

static inline void p_indent(int level, struct frame *f)
{
	if (level < 0) {
		parser.state = 0;
		return;
	}

	if (!parser.state) {
		if (parser.flags & DUMP_TSTAMP) {
			if (parser.flags & DUMP_VERBOSE) {
				struct tm tm;
				time_t t = f->ts.tv_sec;
				localtime_r(&t, &tm);
				printf("%04d-%02d-%02d %02d:%02d:%02d.%06lu ",
					tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
					tm.tm_hour, tm.tm_min, tm.tm_sec, f->ts.tv_usec);
			} else
				printf("%8lu.%06lu ", f->ts.tv_sec, f->ts.tv_usec);
		}
		printf("%c ", (f->in ? '>' : '<'));
		parser.state = 1;
	} else 
		printf("  ");

	if (level)
		printf("%*c", (level*2), ' ');
}

static inline void p_ba2str(const bdaddr_t *ba, char *str)
{
	if (parser.flags & DUMP_NOVENDOR) {
		uint8_t b[6];

		baswap((bdaddr_t *) b, ba);
		sprintf(str, "%2.2X:%2.2X:%2.2X:*:*:*", b[0], b[1], b[2]);
	} else
		ba2str(ba, str);
}

/* get_uXX functions do byte swaping */

static inline uint8_t p_get_u8(struct frame *frm)
{
	uint8_t *u8_ptr = frm->ptr;
	frm->ptr += 1;
	frm->len -= 1;
	return *u8_ptr;
}

static inline uint16_t p_get_u16(struct frame *frm)
{
	uint16_t *u16_ptr = frm->ptr;
	frm->ptr += 2;
	frm->len -= 2;
	return get_be16(u16_ptr);
}

static inline uint32_t p_get_u32(struct frame *frm)
{
	uint32_t *u32_ptr = frm->ptr;
	frm->ptr += 4;
	frm->len -= 4;
	return get_be32(u32_ptr);
}

static inline uint64_t p_get_u64(struct frame *frm)
{
	uint64_t *u64_ptr = frm->ptr;
	uint64_t u64 = get_unaligned(u64_ptr), tmp;
	frm->ptr += 8;
	frm->len -= 8;
	tmp = ntohl(u64 & 0xffffffff);
	u64 = (tmp << 32) | ntohl(u64 >> 32);
	return u64;
}

static inline void p_get_u128(struct frame *frm, uint64_t *l, uint64_t *h)
{
	*h = p_get_u64(frm);
	*l = p_get_u64(frm);
}

char *get_uuid_name(int uuid);

void set_proto(uint16_t handle, uint16_t psm, uint8_t channel, uint32_t proto);
uint32_t get_proto(uint16_t handle, uint16_t psm, uint8_t channel);

struct frame *add_frame(struct frame *frm);
void del_frame(uint16_t handle, uint8_t dlci);

uint8_t get_opcode(uint16_t handle, uint8_t dlci);
void set_opcode(uint16_t handle, uint8_t dlci, uint8_t opcode);

uint8_t get_status(uint16_t handle, uint8_t dlci);
void set_status(uint16_t handle, uint8_t dlci, uint8_t status);

void l2cap_clear(uint16_t handle);

void ascii_dump(int level, struct frame *frm, int num);
void hex_dump(int level, struct frame *frm, int num);
void ext_dump(int level, struct frame *frm, int num);
void raw_dump(int level, struct frame *frm);
void raw_ndump(int level, struct frame *frm, int num);

void lmp_dump(int level, struct frame *frm);
void hci_dump(int level, struct frame *frm);
void l2cap_dump(int level, struct frame *frm);
void rfcomm_dump(int level, struct frame *frm);
void sdp_dump(int level, struct frame *frm);
void bnep_dump(int level, struct frame *frm);
void cmtp_dump(int level, struct frame *frm);
void hidp_dump(int level, struct frame *frm);
void hcrp_dump(int level, struct frame *frm);
void avdtp_dump(int level, struct frame *frm);
void avctp_dump(int level, struct frame *frm, uint16_t psm);
void avrcp_dump(int level, struct frame *frm, uint8_t hdr, uint16_t psm);
void att_dump(int level, struct frame *frm);
void smp_dump(int level, struct frame *frm);
void sap_dump(int level, struct frame *frm);

void obex_dump(int level, struct frame *frm);
void capi_dump(int level, struct frame *frm);
void ppp_dump(int level, struct frame *frm);
void arp_dump(int level, struct frame *frm);
void ip_dump(int level, struct frame *frm);
void ericsson_dump(int level, struct frame *frm);
void csr_dump(int level, struct frame *frm);
void bpa_dump(int level, struct frame *frm);

void amp_assoc_dump(int level, uint8_t *assoc, uint16_t len);

static inline void parse(struct frame *frm)
{
	p_indent(-1, NULL);
	if (parser.flags & DUMP_RAW)
		raw_dump(0, frm);
	else
		hci_dump(0, frm);
	fflush(stdout);
}

#endif /* __PARSER_H */
