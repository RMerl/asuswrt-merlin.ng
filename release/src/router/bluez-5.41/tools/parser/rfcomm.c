/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2001-2002  Wayne Lee <waynelee@qualcomm.com>
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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "rfcomm.h"
#include "sdp.h"

static char *cr_str[] = {
	"RSP",
	"CMD"
};

#define CR_STR(mcc_head) cr_str[mcc_head->type.cr]
#define GET_DLCI(addr) ((addr.server_chn << 1) | (addr.d & 1))

static void print_rfcomm_hdr(long_frame_head* head, uint8_t *ptr, int len)
{
	address_field addr = head->addr;
	uint8_t ctr = head->control;
	uint16_t ilen = head->length.bits.len;
	uint8_t pf, dlci, fcs;

	dlci     = GET_DLCI(addr);
	pf       = GET_PF(ctr);
	fcs      = *(ptr + len - 1);

	printf("cr %d dlci %d pf %d ilen %d fcs 0x%x ", addr.cr, dlci, pf, ilen, fcs); 
}

static void print_mcc(mcc_long_frame_head* mcc_head)
{
	printf("mcc_len %d\n", mcc_head->length.bits.len);
}

static inline void mcc_test(int level, uint8_t *ptr, int len,
				long_frame_head *head, mcc_long_frame_head *mcc_head)
{
	printf("TEST %s: ", CR_STR(mcc_head));
	print_rfcomm_hdr(head, ptr, len);
	print_mcc(mcc_head);

	p_indent(level, 0);
	printf("%*cTest data: 0x ", level, ' ');

	while (len > 1) {
		printf("%2.2x ", (uint8_t)*ptr);
		len--;
		ptr++;
	}

	printf("\n");
}
static inline void mcc_fcon(int level, uint8_t *ptr, int len,
				long_frame_head *head, mcc_long_frame_head *mcc_head)
{
	printf("FCON %s: ", CR_STR(mcc_head));
	print_rfcomm_hdr(head, ptr, len);
	print_mcc(mcc_head);
}

static inline void mcc_fcoff(int level, uint8_t *ptr, int len,
				long_frame_head *head, mcc_long_frame_head *mcc_head)
{
	printf("FCOFF %s: ", CR_STR(mcc_head));
	print_rfcomm_hdr(head, ptr, len);
	print_mcc(mcc_head);
}

static inline void mcc_msc(int level, uint8_t *ptr, unsigned int len,
				long_frame_head *head, mcc_long_frame_head *mcc_head)
{
	msc_msg *msc = (void*) (ptr - STRUCT_END(msc_msg, mcc_s_head));

	printf("MSC %s: ", CR_STR(mcc_head));
	print_rfcomm_hdr(head, ptr, len);
	print_mcc(mcc_head);
	p_indent(level, 0);
	printf("dlci %d fc %d rtc %d rtr %d ic %d dv %d",
		GET_DLCI(msc->dlci), msc->v24_sigs.fc, msc->v24_sigs.rtc, 
		msc->v24_sigs.rtr, msc->v24_sigs.ic, msc->v24_sigs.dv );

	/* Assuming that break_signals field is _not declared_ in struct msc_msg... */
	if (len > STRUCT_OFFSET(msc_msg, fcs) - STRUCT_END(msc_msg, v24_sigs)) {
		break_signals *brk = (break_signals *)
			(ptr + STRUCT_END(msc_msg, v24_sigs));
		printf(" b1 %d b2 %d b3 %d len %d\n",
			brk->b1, brk->b2, brk->b3, brk->len);
	} else
		printf("\n");
}

static inline void mcc_rpn(int level, uint8_t *ptr, unsigned int len,
				long_frame_head *head, mcc_long_frame_head *mcc_head)
{
	rpn_msg *rpn = (void *) (ptr - STRUCT_END(rpn_msg, mcc_s_head));

	printf("RPN %s: ", CR_STR(mcc_head));
	print_rfcomm_hdr(head, ptr, len);
	print_mcc(mcc_head);

	p_indent(level, 0);
	printf("dlci %d ", GET_DLCI(rpn->dlci));

	/* Assuming that rpn_val is _declared_ as a member of rpn_msg... */
	if (len <= STRUCT_OFFSET(rpn_msg, rpn_val) - STRUCT_END(rpn_msg, mcc_s_head)) {
		printf("\n");
		return;
	}

	printf("br %d db %d sb %d p %d pt %d xi %d xo %d\n",
		rpn->rpn_val.bit_rate, rpn->rpn_val.data_bits, 
		rpn->rpn_val.stop_bit, rpn->rpn_val.parity,
		rpn->rpn_val.parity_type, rpn->rpn_val.xon_input,
		rpn->rpn_val.xon_output);

	p_indent(level, 0);
	printf("rtri %d rtro %d rtci %d rtco %d xon %d xoff %d pm 0x%04x\n",
		rpn->rpn_val.rtr_input, rpn->rpn_val.rtr_output,
		rpn->rpn_val.rtc_input, rpn->rpn_val.rtc_output,
		rpn->rpn_val.xon, rpn->rpn_val.xoff, btohs(rpn->rpn_val.pm));
}

static inline void mcc_rls(int level, uint8_t *ptr, int len,
				long_frame_head *head, mcc_long_frame_head *mcc_head)
{
	rls_msg* rls = (void*) (ptr - STRUCT_END(rls_msg, mcc_s_head));

	printf("RLS %s: ", CR_STR(mcc_head));
	print_rfcomm_hdr(head, ptr, len);
	print_mcc(mcc_head);
	printf("dlci %d error: %d", GET_DLCI(rls->dlci), rls->error);
}

static inline void mcc_pn(int level, uint8_t *ptr, int len,
				long_frame_head *head, mcc_long_frame_head *mcc_head)
{
	pn_msg *pn = (void*) (ptr - STRUCT_END(pn_msg, mcc_s_head));

	printf("PN %s: ", CR_STR(mcc_head));
	print_rfcomm_hdr(head, ptr, len);
	print_mcc(mcc_head);

	p_indent(level, 0);
	printf("dlci %d frame_type %d credit_flow %d pri %d ack_timer %d\n",
		pn->dlci, pn->frame_type, pn->credit_flow, pn->prior, pn->ack_timer);
	p_indent(level, 0);
	printf("frame_size %d max_retrans %d credits %d\n",
		btohs(pn->frame_size), pn->max_nbrof_retrans, pn->credits);
}

static inline void mcc_nsc(int level, uint8_t *ptr, int len,
				long_frame_head *head, mcc_long_frame_head *mcc_head)
{

	nsc_msg *nsc = (void*) (ptr - STRUCT_END(nsc_msg, mcc_s_head));

	printf("NSC %s: ", CR_STR(mcc_head));
	print_rfcomm_hdr(head, ptr, len);
	print_mcc(mcc_head);

	p_indent(level, 0);
	printf("cr %d, mcc_cmd_type %x\n", 
		nsc->command_type.cr, nsc->command_type.type );
}

static inline void mcc_frame(int level, struct frame *frm, long_frame_head *head)
{
	mcc_short_frame_head *mcc_short_head_p = frm->ptr;
	mcc_long_frame_head mcc_head;
	uint8_t hdr_size;

	if ( mcc_short_head_p->length.ea == EA ) {
		mcc_head.type = mcc_short_head_p->type;
		mcc_head.length.bits.len = mcc_short_head_p->length.len;
		hdr_size = sizeof(mcc_short_frame_head);
	} else {
		mcc_head = *(mcc_long_frame_head *)frm->ptr;
		mcc_head.length.val = btohs(mcc_head.length.val);
		hdr_size = sizeof(mcc_long_frame_head);
	}

	frm->ptr += hdr_size;
	frm->len -= hdr_size;

	p_indent(level, frm);
	printf("RFCOMM(s): ");

	switch (mcc_head.type.type) {
	case TEST:
		mcc_test(level, frm->ptr, frm->len, head, &mcc_head);
		break;
	case FCON:
		mcc_fcon(level, frm->ptr, frm->len, head, &mcc_head);
		break;
	case FCOFF:
		mcc_fcoff(level, frm->ptr, frm->len, head, &mcc_head);
		break;
	case MSC:
		mcc_msc(level, frm->ptr, frm->len, head, &mcc_head);
		break;
	case RPN:
		mcc_rpn(level, frm->ptr, frm->len, head, &mcc_head);
		break;
	case RLS:
		mcc_rls(level, frm->ptr, frm->len, head, &mcc_head);
		break;
	case PN:
		mcc_pn(level, frm->ptr, frm->len, head, &mcc_head);
		break;
	case NSC:
		mcc_nsc(level, frm->ptr, frm->len, head, &mcc_head);
		break;
	default:
		printf("MCC message type 0x%02x: ", mcc_head.type.type);
		print_rfcomm_hdr(head, frm->ptr, frm->len);
		printf("\n");

		frm->len--;
		raw_dump(level, frm); 
	}
}

static inline void uih_frame(int level, struct frame *frm, long_frame_head *head)
{
	uint32_t proto;

	if (!head->addr.server_chn) {
		mcc_frame(level, frm, head); 
	} else {
		p_indent(level, frm);
		printf("RFCOMM(d): UIH: ");
		print_rfcomm_hdr(head, frm->ptr, frm->len);
		if (GET_PF(head->control)) {
			printf("credits %d\n", *(uint8_t *)(frm->ptr));
			frm->ptr++;
			frm->len--;
		} else
			printf("\n");

		frm->len--;
		frm->dlci = GET_DLCI(head->addr);
		frm->channel = head->addr.server_chn;

		proto = get_proto(frm->handle, RFCOMM_PSM, frm->channel);

		if (frm->len > 0) {
			switch (proto) {
			case SDP_UUID_OBEX:
				if (!p_filter(FILT_OBEX))
					obex_dump(level + 1, frm);
				else
					raw_dump(level, frm);
				break;

			case SDP_UUID_LAN_ACCESS_PPP:
			case SDP_UUID_DIALUP_NETWORKING:
				if (!p_filter(FILT_PPP))
					ppp_dump(level + 1, frm);
				else
					raw_dump(level, frm);
				break;

			case SDP_UUID_SIM_ACCESS:
				if (!p_filter(FILT_SAP))
					sap_dump(level + 1, frm);
				else
					raw_dump(level, frm);
				break;

			default:
				if (p_filter(FILT_RFCOMM))
					break;

				raw_dump(level, frm);
				break;
			}
		}
	}
}

void rfcomm_dump(int level, struct frame *frm)
{
	uint8_t hdr_size, ctr_type;
	short_frame_head *short_head_p = (void *) frm->ptr;
	long_frame_head head;

	if (short_head_p->length.ea == EA) {
		head.addr = short_head_p->addr;
		head.control = short_head_p->control;
		head.length.bits.len = short_head_p->length.len;
		hdr_size = sizeof(short_frame_head);
	} else {
		head = *(long_frame_head *) frm->ptr;
		head.length.val = btohs(head.length.val);
		hdr_size = sizeof(long_frame_head);
	}

	frm->ptr += hdr_size;
	frm->len -= hdr_size;

	ctr_type = CLR_PF(head.control);

	if (ctr_type == UIH) {
		uih_frame(level, frm, &head);
	} else {
		p_indent(level, frm); 
		printf("RFCOMM(s): ");

		switch (ctr_type) {
		case SABM:
			printf("SABM: ");
			break;
		case UA:
			printf("UA: ");
			break;
		case DM:
			printf("DM: ");
			break;
		case DISC:
			printf("DISC: ");
			del_frame(frm->handle, GET_DLCI(head.addr));
			break;
		default:
			printf("ERR: ");
		}
		print_rfcomm_hdr(&head, frm->ptr, frm->len);
		printf("\n");
	}
}
