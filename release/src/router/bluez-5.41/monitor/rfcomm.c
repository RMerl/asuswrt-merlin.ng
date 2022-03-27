/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include "lib/bluetooth.h"

#include "src/shared/util.h"
#include "bt.h"
#include "packet.h"
#include "display.h"
#include "l2cap.h"
#include "uuid.h"
#include "keys.h"
#include "sdp.h"
#include "rfcomm.h"

static char *cr_str[] = {
	"RSP",
	"CMD"
};

/* RFCOMM frame parsing macros */
#define CR_STR(type)		cr_str[GET_CR(type)]
#define GET_LEN8(length)	((length & 0xfe) >> 1)
#define GET_LEN16(length)	((length & 0xfffe) >> 1)
#define GET_CR(type)		((type & 0x02) >> 1)
#define GET_PF(ctr)		(((ctr) >> 4) & 0x1)

/* MSC macros */
#define GET_V24_FC(sigs)	((sigs & 0x02) >> 1)
#define GET_V24_RTC(sigs)	((sigs & 0x04) >> 2)
#define GET_V24_RTR(sigs)	((sigs & 0x08) >> 3)
#define GET_V24_IC(sigs)	((sigs & 0x40) >> 6)
#define GET_V24_DV(sigs)	((sigs & 0x80) >> 7)

/* RPN macros */
#define GET_RPN_DB(parity)	(parity & 0x03)
#define GET_RPN_SB(parity)	((parity & 0x04) >> 2)
#define GET_RPN_PARITY(parity)	((parity & 0x08) >> 3)
#define GET_RPN_PTYPE(parity)	((parity & 0x30) >> 4)
#define GET_RPN_XIN(io)		(io & 0x01)
#define GET_RPN_XOUT(io)	((io & 0x02) >> 1)
#define GET_RPN_RTRI(io)	((io & 0x04) >> 2)
#define GET_RPN_RTRO(io)	((io & 0x08) >> 3)
#define GET_RPN_RTCI(io)	((io & 0x10) >> 4)
#define GET_RPN_RTCO(io)	((io & 0x20) >> 5)

/* RLS macro */
#define GET_ERROR(err)		(err & 0x0f)

/* PN macros */
#define GET_FRM_TYPE(ctrl)	((ctrl & 0x0f))
#define GET_CRT_FLOW(ctrl)	((ctrl & 0xf0) >> 4)
#define GET_PRIORITY(prio)	((prio & 0x3f))
#define GET_PN_DLCI(dlci)	((dlci & 0x3f))

struct rfcomm_lhdr {
	uint8_t address;
	uint8_t control;
	uint16_t length;
	uint8_t fcs;
	uint8_t credits; /* only for UIH frame */
} __attribute__((packed));

struct rfcomm_lmsc {
	uint8_t dlci;
	uint8_t v24_sig;
	uint8_t break_sig;
} __attribute__((packed));

struct rfcomm_rpn {
	uint8_t dlci;
	uint8_t bit_rate;
	uint8_t parity;
	uint8_t io;
	uint8_t xon;
	uint8_t xoff;
	uint16_t pm;
} __attribute__ ((packed));

struct rfcomm_rls {
	uint8_t dlci;
	uint8_t error;
} __attribute__((packed));

struct rfcomm_nsc {
	uint8_t cmd_type;
} __attribute__((packed));

struct rfcomm_lmcc {
	uint8_t type;
	uint16_t length;
} __attribute__((packed));

struct rfcomm_frame {
	struct rfcomm_lhdr hdr;
	struct rfcomm_lmcc mcc;
	struct l2cap_frame l2cap_frame;
};

static void print_rfcomm_hdr(struct rfcomm_frame *rfcomm_frame, uint8_t indent)
{
	struct rfcomm_lhdr hdr = rfcomm_frame->hdr;

	/* Address field */
	print_field("%*cAddress: 0x%2.2x cr %d dlci 0x%2.2x", indent, ' ',
					hdr.address, GET_CR(hdr.address),
					RFCOMM_GET_DLCI(hdr.address));

	/* Control field */
	print_field("%*cControl: 0x%2.2x poll/final %d", indent, ' ',
					hdr.control, GET_PF(hdr.control));

	/* Length and FCS */
	print_field("%*cLength: %d", indent, ' ', hdr.length);
	print_field("%*cFCS: 0x%2.2x", indent, ' ', hdr.fcs);
}

static inline bool mcc_test(struct rfcomm_frame *rfcomm_frame, uint8_t indent)
{
	struct l2cap_frame *frame = &rfcomm_frame->l2cap_frame;
	uint8_t data;

	printf("%*cTest Data: 0x ", indent, ' ');

	while (frame->size > 1) {
		if (!l2cap_frame_get_u8(frame, &data))
			return false;
		printf("%2.2x ", data);
	}

	printf("\n");
	return true;
}

static inline bool mcc_msc(struct rfcomm_frame *rfcomm_frame, uint8_t indent)
{
	struct l2cap_frame *frame = &rfcomm_frame->l2cap_frame;
	struct rfcomm_lmsc msc;

	if (!l2cap_frame_get_u8(frame, &msc.dlci))
		return false;

	print_field("%*cdlci %d ", indent, ' ', RFCOMM_GET_DLCI(msc.dlci));

	if (!l2cap_frame_get_u8(frame, &msc.v24_sig))
		return false;

	/* v24 control signals */
	print_field("%*cfc %d rtc %d rtr %d ic %d dv %d", indent, ' ',
		GET_V24_FC(msc.v24_sig), GET_V24_RTC(msc.v24_sig),
		GET_V24_RTR(msc.v24_sig), GET_V24_IC(msc.v24_sig),
					GET_V24_DV(msc.v24_sig));

	if (frame->size < 2)
		goto done;

	/*
	 * TODO: Implement the break signals decoding.
	 */

	packet_hexdump(frame->data, frame->size);

done:
	return true;
}

static inline bool mcc_rpn(struct rfcomm_frame *rfcomm_frame, uint8_t indent)
{
	struct l2cap_frame *frame = &rfcomm_frame->l2cap_frame;
	struct rfcomm_rpn rpn;

	if (!l2cap_frame_get_u8(frame, &rpn.dlci))
		return false;

	print_field("%*cdlci %d", indent, ' ', RFCOMM_GET_DLCI(rpn.dlci));

	if (frame->size < 7)
		goto done;

	/* port value octets (optional) */

	if (!l2cap_frame_get_u8(frame, &rpn.bit_rate))
		return false;

	if (!l2cap_frame_get_u8(frame, &rpn.parity))
		return false;

	if (!l2cap_frame_get_u8(frame, &rpn.io))
		return false;

	print_field("%*cbr %d db %d sb %d p %d pt %d xi %d xo %d", indent, ' ',
		rpn.bit_rate, GET_RPN_DB(rpn.parity), GET_RPN_SB(rpn.parity),
		GET_RPN_PARITY(rpn.parity), GET_RPN_PTYPE(rpn.parity),
		GET_RPN_XIN(rpn.io), GET_RPN_XOUT(rpn.io));

	if (!l2cap_frame_get_u8(frame, &rpn.xon))
		return false;

	if (!l2cap_frame_get_u8(frame, &rpn.xoff))
		return false;

	print_field("%*crtri %d rtro %d rtci %d rtco %d xon %d xoff %d",
		indent, ' ', GET_RPN_RTRI(rpn.io), GET_RPN_RTRO(rpn.io),
		GET_RPN_RTCI(rpn.io), GET_RPN_RTCO(rpn.io), rpn.xon,
		rpn.xoff);

	if (!l2cap_frame_get_le16(frame, &rpn.pm))
		return false;

	print_field("%*cpm 0x%04x", indent, ' ', rpn.pm);

done:
	return true;
}

static inline bool mcc_rls(struct rfcomm_frame *rfcomm_frame, uint8_t indent)
{
	struct l2cap_frame *frame = &rfcomm_frame->l2cap_frame;
	struct rfcomm_rls rls;

	if (!l2cap_frame_get_u8(frame, &rls.dlci))
		return false;

	if (!l2cap_frame_get_u8(frame, &rls.error))
		return false;

	print_field("%*cdlci %d error: %d", indent, ' ',
			RFCOMM_GET_DLCI(rls.dlci), GET_ERROR(rls.error));

	return true;
}

static inline bool mcc_pn(struct rfcomm_frame *rfcomm_frame, uint8_t indent)
{
	struct l2cap_frame *frame = &rfcomm_frame->l2cap_frame;
	struct rfcomm_pn pn;

	/* rfcomm_pn struct is defined in rfcomm.h */

	if (!l2cap_frame_get_u8(frame, &pn.dlci))
		return false;

	if (!l2cap_frame_get_u8(frame, &pn.flow_ctrl))
		return false;

	if (!l2cap_frame_get_u8(frame, &pn.priority))
		return false;

	print_field("%*cdlci %d frame_type %d credit_flow %d pri %d", indent,
			' ', GET_PN_DLCI(pn.dlci), GET_FRM_TYPE(pn.flow_ctrl),
			GET_CRT_FLOW(pn.flow_ctrl), GET_PRIORITY(pn.priority));

	if (!l2cap_frame_get_u8(frame, &pn.ack_timer))
		return false;

	if (!l2cap_frame_get_le16(frame, &pn.mtu))
		return false;

	if (!l2cap_frame_get_u8(frame, &pn.max_retrans))
		return false;

	if (!l2cap_frame_get_u8(frame, &pn.credits))
		return false;

	print_field("%*cack_timer %d frame_size %d max_retrans %d credits %d",
			indent, ' ', pn.ack_timer, pn.mtu, pn.max_retrans,
			pn.credits);

	return true;
}

static inline bool mcc_nsc(struct rfcomm_frame *rfcomm_frame, uint8_t indent)
{
	struct l2cap_frame *frame = &rfcomm_frame->l2cap_frame;
	struct rfcomm_nsc nsc;

	if (!l2cap_frame_get_u8(frame, &nsc.cmd_type))
		return false;

	print_field("%*ccr %d, mcc_cmd_type %x", indent, ' ',
		GET_CR(nsc.cmd_type), RFCOMM_GET_MCC_TYPE(nsc.cmd_type));

	return true;
}

struct mcc_data {
	uint8_t type;
	const char *str;
};

static const struct mcc_data mcc_table[] = {
	{ 0x08, "Test Command" },
	{ 0x28, "Flow Control On Command" },
	{ 0x18, "Flow Control Off Command" },
	{ 0x38, "Modem Status Command" },
	{ 0x24, "Remote Port Negotiation Command" },
	{ 0x14, "Remote Line Status" },
	{ 0x20, "DLC Parameter Negotiation" },
	{ 0x04, "Non Supported Command" },
	{ }
};

static inline bool mcc_frame(struct rfcomm_frame *rfcomm_frame, uint8_t indent)
{
	uint8_t length, ex_length, type;
	const char *type_str;
	int i;
	struct l2cap_frame *frame = &rfcomm_frame->l2cap_frame;
	struct rfcomm_lmcc mcc;
	const struct mcc_data *mcc_data = NULL;

	if (!l2cap_frame_get_u8(frame, &mcc.type) ||
			!l2cap_frame_get_u8(frame, &length))
		return false;

	if (RFCOMM_TEST_EA(length))
		mcc.length = (uint16_t) GET_LEN8(length);
	else {
		if (!l2cap_frame_get_u8(frame, &ex_length))
			return false;
		mcc.length = ((uint16_t) length << 8) | ex_length;
		mcc.length = GET_LEN16(mcc.length);
	}

	type = RFCOMM_GET_MCC_TYPE(mcc.type);

	for (i = 0; mcc_table[i].str; i++) {
		if (mcc_table[i].type == type) {
			mcc_data = &mcc_table[i];
			break;
		}
	}

	if (mcc_data)
		type_str = mcc_data->str;
	else
		type_str = "Unknown";

	print_field("%*cMCC Message type: %s %s (0x%2.2x)", indent, ' ',
				type_str, CR_STR(mcc.type), type);

	print_field("%*cLength: %d", indent+2, ' ', mcc.length);

	rfcomm_frame->mcc = mcc;

	switch (type) {
	case RFCOMM_TEST:
		return mcc_test(rfcomm_frame, indent+10);
	case RFCOMM_MSC:
		return mcc_msc(rfcomm_frame, indent+2);
	case RFCOMM_RPN:
		return mcc_rpn(rfcomm_frame, indent+2);
	case RFCOMM_RLS:
		return mcc_rls(rfcomm_frame, indent+2);
	case RFCOMM_PN:
		return mcc_pn(rfcomm_frame, indent+2);
	case RFCOMM_NSC:
		return mcc_nsc(rfcomm_frame, indent+2);
	default:
		packet_hexdump(frame->data, frame->size);
	}

	return true;
}

static bool uih_frame(struct rfcomm_frame *rfcomm_frame, uint8_t indent)
{
	uint8_t credits;
	struct l2cap_frame *frame = &rfcomm_frame->l2cap_frame;
	struct rfcomm_lhdr *hdr = &rfcomm_frame->hdr;

	if (!RFCOMM_GET_CHANNEL(hdr->address))
		return mcc_frame(rfcomm_frame, indent);

	/* fetching credits from UIH frame */
	if (GET_PF(hdr->control)) {
		if (!l2cap_frame_get_u8(frame, &credits))
			return false;
		hdr->credits = credits;
		print_field("%*cCredits: %d", indent, ' ', hdr->credits);
	}

	packet_hexdump(frame->data, frame->size);
	return true;
}

struct rfcomm_data {
	uint8_t frame;
	const char *str;
};

static const struct rfcomm_data rfcomm_table[] = {
	{ 0x2f, "Set Async Balance Mode (SABM)" },
	{ 0x63, "Unnumbered Ack (UA)" },
	{ 0x0f, "Disconnect Mode (DM)" },
	{ 0x43, "Disconnect (DISC)" },
	{ 0xef, "Unnumbered Info with Header Check (UIH)" },
	{ }
};

void rfcomm_packet(const struct l2cap_frame *frame)
{
	uint8_t ctype, length, ex_length, indent = 1;
	const char *frame_str, *frame_color;
	struct l2cap_frame *l2cap_frame, tmp_frame;
	struct rfcomm_frame rfcomm_frame;
	struct rfcomm_lhdr hdr;
	const struct rfcomm_data *rfcomm_data = NULL;
	int i;

	l2cap_frame_pull(&rfcomm_frame.l2cap_frame, frame, 0);

	l2cap_frame = &rfcomm_frame.l2cap_frame;

	if (frame->size < 4)
		goto fail;

	if (!l2cap_frame_get_u8(l2cap_frame, &hdr.address) ||
			!l2cap_frame_get_u8(l2cap_frame, &hdr.control) ||
			!l2cap_frame_get_u8(l2cap_frame, &length))
		goto fail;

	/* length maybe 1 or 2 octets */
	if (RFCOMM_TEST_EA(length))
		hdr.length = (uint16_t) GET_LEN8(length);
	else {
		if (!l2cap_frame_get_u8(l2cap_frame, &ex_length))
			goto fail;
		hdr.length = ((uint16_t)length << 8) | ex_length;
		hdr.length = GET_LEN16(hdr.length);
	}

	l2cap_frame_pull(&tmp_frame, l2cap_frame, l2cap_frame->size-1);

	if (!l2cap_frame_get_u8(&tmp_frame, &hdr.fcs))
		goto fail;

	/* Decoding frame type */
	ctype = RFCOMM_GET_TYPE(hdr.control);

	for (i = 0; rfcomm_table[i].str; i++) {
		if (rfcomm_table[i].frame == ctype) {
			rfcomm_data = &rfcomm_table[i];
			break;
		}
	}

	if (rfcomm_data) {
		if (frame->in)
			frame_color = COLOR_MAGENTA;
		else
			frame_color = COLOR_BLUE;
		frame_str = rfcomm_data->str;
	} else {
		frame_color = COLOR_WHITE_BG;
		frame_str = "Unknown";
	}

	if (!rfcomm_data) {
		packet_hexdump(frame->data, frame->size);
		return;
	}

	print_indent(6, frame_color, "RFCOMM: ", frame_str, COLOR_OFF,
						" (0x%2.2x)", ctype);

	rfcomm_frame.hdr = hdr;
	print_rfcomm_hdr(&rfcomm_frame, indent);

	/* UIH frame */
	if (ctype == 0xef)
		if (!uih_frame(&rfcomm_frame, indent))
			goto fail;

	return;

fail:
	print_text(COLOR_ERROR, "Frame too short");
	packet_hexdump(frame->data, frame->size);
	return;
}
