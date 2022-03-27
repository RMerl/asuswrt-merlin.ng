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
#include "avctp.h"
#include "avdtp.h"
#include "rfcomm.h"
#include "bnep.h"

/* L2CAP Control Field bit masks */
#define L2CAP_CTRL_SAR_MASK		0xC000
#define L2CAP_CTRL_REQSEQ_MASK		0x3F00
#define L2CAP_CTRL_TXSEQ_MASK		0x007E
#define L2CAP_CTRL_SUPERVISE_MASK	0x000C

#define L2CAP_CTRL_RETRANS		0x0080
#define L2CAP_CTRL_FINAL		0x0080
#define L2CAP_CTRL_POLL			0x0010
#define L2CAP_CTRL_FRAME_TYPE		0x0001 /* I- or S-Frame */

#define L2CAP_CTRL_TXSEQ_SHIFT		1
#define L2CAP_CTRL_SUPER_SHIFT		2
#define L2CAP_CTRL_REQSEQ_SHIFT		8
#define L2CAP_CTRL_SAR_SHIFT		14

#define L2CAP_EXT_CTRL_TXSEQ_MASK	0xFFFC0000
#define L2CAP_EXT_CTRL_SAR_MASK		0x00030000
#define L2CAP_EXT_CTRL_SUPERVISE_MASK	0x00030000
#define L2CAP_EXT_CTRL_REQSEQ_MASK	0x0000FFFC

#define L2CAP_EXT_CTRL_POLL		0x00040000
#define L2CAP_EXT_CTRL_FINAL		0x00000002
#define L2CAP_EXT_CTRL_FRAME_TYPE	0x00000001 /* I- or S-Frame */

#define L2CAP_EXT_CTRL_REQSEQ_SHIFT	2
#define L2CAP_EXT_CTRL_SAR_SHIFT	16
#define L2CAP_EXT_CTRL_SUPER_SHIFT	16
#define L2CAP_EXT_CTRL_TXSEQ_SHIFT	18

/* L2CAP Supervisory Function */
#define L2CAP_SUPER_RR		0x00
#define L2CAP_SUPER_REJ		0x01
#define L2CAP_SUPER_RNR		0x02
#define L2CAP_SUPER_SREJ	0x03

/* L2CAP Segmentation and Reassembly */
#define L2CAP_SAR_UNSEGMENTED	0x00
#define L2CAP_SAR_START		0x01
#define L2CAP_SAR_END		0x02
#define L2CAP_SAR_CONTINUE	0x03

#define MAX_CHAN 64

struct chan_data {
	uint16_t index;
	uint16_t handle;
	uint8_t ident;
	uint16_t scid;
	uint16_t dcid;
	uint16_t psm;
	uint8_t  ctrlid;
	uint8_t  mode;
	uint8_t  ext_ctrl;
	uint8_t  seq_num;
};

static struct chan_data chan_list[MAX_CHAN];

static void assign_scid(const struct l2cap_frame *frame,
				uint16_t scid, uint16_t psm, uint8_t ctrlid)
{
	int i, n = -1;
	uint8_t seq_num = 1;

	for (i = 0; i < MAX_CHAN; i++) {
		if (n < 0 && chan_list[i].handle == 0x0000) {
			n = i;
			continue;
		}

		if (chan_list[i].index != frame->index)
			continue;

		if (chan_list[i].handle != frame->handle)
			continue;

		if (chan_list[i].psm == psm)
			seq_num++;

		/* Don't break on match - we still need to go through all
		 * channels to find proper seq_num.
		 */
		if (frame->in) {
			if (chan_list[i].dcid == scid)
				n = i;
		} else {
			if (chan_list[i].scid == scid)
				n = i;
		}
	}

	if (n < 0)
		return;

	memset(&chan_list[n], 0, sizeof(chan_list[n]));
	chan_list[n].index = frame->index;
	chan_list[n].handle = frame->handle;
	chan_list[n].ident = frame->ident;

	if (frame->in)
		chan_list[n].dcid = scid;
	else
		chan_list[n].scid = scid;

	chan_list[n].psm = psm;
	chan_list[n].ctrlid = ctrlid;
	chan_list[n].mode = 0;

	chan_list[n].seq_num = seq_num;
}

static void release_scid(const struct l2cap_frame *frame, uint16_t scid)
{
	int i;

	for (i = 0; i < MAX_CHAN; i++) {
		if (chan_list[i].index != frame->index)
			continue;

		if (chan_list[i].handle != frame->handle)
			continue;

		if (frame->in) {
			if (chan_list[i].scid == scid) {
				chan_list[i].handle = 0;
				break;
			}
		} else {
			if (chan_list[i].dcid == scid) {
				chan_list[i].handle = 0;
				break;
			}
		}
	}
}

static void assign_dcid(const struct l2cap_frame *frame, uint16_t dcid,
								uint16_t scid)
{
	int i;

	for (i = 0; i < MAX_CHAN; i++) {
		if (chan_list[i].index != frame->index)
			continue;

		if (chan_list[i].handle != frame->handle)
			continue;

		if (frame->ident != 0 && chan_list[i].ident != frame->ident)
			continue;

		if (frame->in) {
			if (scid) {
				if (chan_list[i].scid == scid) {
					chan_list[i].dcid = dcid;
					break;
				}
			} else {
				if (chan_list[i].scid && !chan_list[i].dcid) {
					chan_list[i].dcid = dcid;
					break;
				}
			}
		} else {
			if (scid) {
				if (chan_list[i].dcid == scid) {
					chan_list[i].scid = dcid;
					break;
				}
			} else {
				if (chan_list[i].dcid && !chan_list[i].scid) {
					chan_list[i].scid = dcid;
					break;
				}
			}
		}
	}
}

static void assign_mode(const struct l2cap_frame *frame,
					uint8_t mode, uint16_t dcid)
{
	int i;

	for (i = 0; i < MAX_CHAN; i++) {
		if (chan_list[i].index != frame->index)
			continue;

		if (chan_list[i].handle != frame->handle)
			continue;

		if (frame->in) {
			if (chan_list[i].scid == dcid) {
				chan_list[i].mode = mode;
				break;
			}
		} else {
			if (chan_list[i].dcid == dcid) {
				chan_list[i].mode = mode;
				break;
			}
		}
	}
}

static int get_chan_data_index(const struct l2cap_frame *frame)
{
	int i;

	for (i = 0; i < MAX_CHAN; i++) {
		if (chan_list[i].index != frame->index &&
					chan_list[i].ctrlid == 0)
			continue;

		if (chan_list[i].ctrlid != 0 &&
					chan_list[i].ctrlid != frame->index)
			continue;

		if (chan_list[i].handle != frame->handle)
			continue;

		if (frame->in) {
			if (chan_list[i].scid == frame->cid)
				return i;
		} else {
			if (chan_list[i].dcid == frame->cid)
				return i;
		}
	}

	return -1;
}

static uint16_t get_psm(const struct l2cap_frame *frame)
{
	int i = get_chan_data_index(frame);

	if (i < 0)
		return 0;

	return chan_list[i].psm;
}

static uint8_t get_mode(const struct l2cap_frame *frame)
{
	int i = get_chan_data_index(frame);

	if (i < 0)
		return 0;

	return chan_list[i].mode;
}

static uint16_t get_chan(const struct l2cap_frame *frame)
{
	int i = get_chan_data_index(frame);

	if (i < 0)
		return 0;

	return i;
}

static uint8_t get_seq_num(const struct l2cap_frame *frame)
{
	int i = get_chan_data_index(frame);

	if (i < 0)
		return 0;

	return chan_list[i].seq_num;
}

static void assign_ext_ctrl(const struct l2cap_frame *frame,
					uint8_t ext_ctrl, uint16_t dcid)
{
	int i;

	for (i = 0; i < MAX_CHAN; i++) {
		if (chan_list[i].index != frame->index)
			continue;

		if (chan_list[i].handle != frame->handle)
			continue;

		if (frame->in) {
			if (chan_list[i].scid == dcid) {
				chan_list[i].ext_ctrl = ext_ctrl;
				break;
			}
		} else {
			if (chan_list[i].dcid == dcid) {
				chan_list[i].ext_ctrl = ext_ctrl;
				break;
			}
		}
	}
}

static uint8_t get_ext_ctrl(const struct l2cap_frame *frame)
{
	int i = get_chan_data_index(frame);

	if (i < 0)
		return 0;

	return chan_list[i].ext_ctrl;
}

static char *sar2str(uint8_t sar)
{
	switch (sar) {
	case L2CAP_SAR_UNSEGMENTED:
		return "Unsegmented";
	case L2CAP_SAR_START:
		return "Start";
	case L2CAP_SAR_END:
		return "End";
	case L2CAP_SAR_CONTINUE:
		return "Continuation";
	default:
		return "Bad SAR";
	}
}

static char *supervisory2str(uint8_t supervisory)
{
	switch (supervisory) {
	case L2CAP_SUPER_RR:
		return "Receiver Ready (RR)";
	case L2CAP_SUPER_REJ:
		return "Reject (REJ)";
	case L2CAP_SUPER_RNR:
		return "Receiver Not Ready (RNR)";
	case L2CAP_SUPER_SREJ:
		return "Select Reject (SREJ)";
	default:
		return "Bad Supervisory";
	}
}

static void l2cap_ctrl_ext_parse(struct l2cap_frame *frame, uint32_t ctrl)
{
	printf("      %s:",
		ctrl & L2CAP_EXT_CTRL_FRAME_TYPE ? "S-frame" : "I-frame");

	if (ctrl & L2CAP_EXT_CTRL_FRAME_TYPE) {
		printf(" %s",
		supervisory2str((ctrl & L2CAP_EXT_CTRL_SUPERVISE_MASK) >>
						L2CAP_EXT_CTRL_SUPER_SHIFT));

		if (ctrl & L2CAP_EXT_CTRL_POLL)
			printf(" P-bit");
	} else {
		uint8_t sar = (ctrl & L2CAP_EXT_CTRL_SAR_MASK) >>
						L2CAP_EXT_CTRL_SAR_SHIFT;
		printf(" %s", sar2str(sar));
		if (sar == L2CAP_SAR_START) {
			uint16_t len;

			if (!l2cap_frame_get_le16(frame, &len))
				return;

			printf(" (len %d)", len);
		}
		printf(" TxSeq %d", (ctrl & L2CAP_EXT_CTRL_TXSEQ_MASK) >>
						L2CAP_EXT_CTRL_TXSEQ_SHIFT);
	}

	printf(" ReqSeq %d", (ctrl & L2CAP_EXT_CTRL_REQSEQ_MASK) >>
						L2CAP_EXT_CTRL_REQSEQ_SHIFT);

	if (ctrl & L2CAP_EXT_CTRL_FINAL)
		printf(" F-bit");
}

static void l2cap_ctrl_parse(struct l2cap_frame *frame, uint32_t ctrl)
{
	printf("      %s:",
			ctrl & L2CAP_CTRL_FRAME_TYPE ? "S-frame" : "I-frame");

	if (ctrl & 0x01) {
		printf(" %s",
			supervisory2str((ctrl & L2CAP_CTRL_SUPERVISE_MASK) >>
						L2CAP_CTRL_SUPER_SHIFT));

		if (ctrl & L2CAP_CTRL_POLL)
			printf(" P-bit");
	} else {
		uint8_t sar;

		sar = (ctrl & L2CAP_CTRL_SAR_MASK) >> L2CAP_CTRL_SAR_SHIFT;
		printf(" %s", sar2str(sar));
		if (sar == L2CAP_SAR_START) {
			uint16_t len;

			if (!l2cap_frame_get_le16(frame, &len))
				return;

			printf(" (len %d)", len);
		}
		printf(" TxSeq %d", (ctrl & L2CAP_CTRL_TXSEQ_MASK) >>
						L2CAP_CTRL_TXSEQ_SHIFT);
	}

	printf(" ReqSeq %d", (ctrl & L2CAP_CTRL_REQSEQ_MASK) >>
						L2CAP_CTRL_REQSEQ_SHIFT);

	if (ctrl & L2CAP_CTRL_FINAL)
		printf(" F-bit");
}

#define MAX_INDEX 16

struct index_data {
	void *frag_buf;
	uint16_t frag_pos;
	uint16_t frag_len;
	uint16_t frag_cid;
};

static struct index_data index_list[MAX_INDEX][2];

static void clear_fragment_buffer(uint16_t index, bool in)
{
	free(index_list[index][in].frag_buf);
	index_list[index][in].frag_buf = NULL;
	index_list[index][in].frag_pos = 0;
	index_list[index][in].frag_len = 0;
}

static void print_psm(uint16_t psm)
{
	print_field("PSM: %d (0x%4.4x)", le16_to_cpu(psm), le16_to_cpu(psm));
}

static void print_cid(const char *type, uint16_t cid)
{
	print_field("%s CID: %d", type, le16_to_cpu(cid));
}

static void print_reject_reason(uint16_t reason)
{
	const char *str;

	switch (le16_to_cpu(reason)) {
	case 0x0000:
		str = "Command not understood";
		break;
	case 0x0001:
		str = "Signaling MTU exceeded";
		break;
	case 0x0002:
		str = "Invalid CID in request";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Reason: %s (0x%4.4x)", str, le16_to_cpu(reason));
}

static void print_conn_result(uint16_t result)
{
	const char *str;

	switch (le16_to_cpu(result)) {
	case 0x0000:
		str = "Connection successful";
		break;
	case 0x0001:
		str = "Connection pending";
		break;
	case 0x0002:
		str = "Connection refused - PSM not supported";
		break;
	case 0x0003:
		str = "Connection refused - security block";
		break;
	case 0x0004:
		str = "Connection refused - no resources available";
		break;
	case 0x0006:
		str = "Connection refused - Invalid Source CID";
		break;
	case 0x0007:
		str = "Connection refused - Source CID already allocated";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Result: %s (0x%4.4x)", str, le16_to_cpu(result));
}

static void print_le_conn_result(uint16_t result)
{
	const char *str;

	switch (le16_to_cpu(result)) {
	case 0x0000:
		str = "Connection successful";
		break;
	case 0x0002:
		str = "Connection refused - PSM not supported";
		break;
	case 0x0004:
		str = "Connection refused - no resources available";
		break;
	case 0x0005:
		str = "Connection refused - insufficient authentication";
		break;
	case 0x0006:
		str = "Connection refused - insufficient authorization";
		break;
	case 0x0007:
		str = "Connection refused - insufficient encryption key size";
		break;
	case 0x0008:
		str = "Connection refused - insufficient encryption";
		break;
	case 0x0009:
		str = "Connection refused - Invalid Source CID";
		break;
	case 0x0010:
		str = "Connection refused - Source CID already allocated";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Result: %s (0x%4.4x)", str, le16_to_cpu(result));
}

static void print_create_chan_result(uint16_t result)
{
	const char *str;

	switch (le16_to_cpu(result)) {
	case 0x0000:
		str = "Connection successful";
		break;
	case 0x0001:
		str = "Connection pending";
		break;
	case 0x0002:
		str = "Connection refused - PSM not supported";
		break;
	case 0x0003:
		str = "Connection refused - security block";
		break;
	case 0x0004:
		str = "Connection refused - no resources available";
		break;
	case 0x0005:
		str = "Connection refused - Controller ID not supported";
		break;
	case 0x0006:
		str = "Connection refused - Invalid Source CID";
		break;
	case 0x0007:
		str = "Connection refused - Source CID already allocated";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Result: %s (0x%4.4x)", str, le16_to_cpu(result));
}

static void print_conn_status(uint16_t status)
{
        const char *str;

	switch (le16_to_cpu(status)) {
	case 0x0000:
		str = "No further information available";
		break;
	case 0x0001:
		str = "Authentication pending";
		break;
	case 0x0002:
		str = "Authorization pending";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Status: %s (0x%4.4x)", str, le16_to_cpu(status));
}

static void print_config_flags(uint16_t flags)
{
	const char *str;

	if (le16_to_cpu(flags) & 0x0001)
		str = " (continuation)";
	else
		str = "";

	print_field("Flags: 0x%4.4x%s", le16_to_cpu(flags), str);
}

static void print_config_result(uint16_t result)
{
	const char *str;

	switch (le16_to_cpu(result)) {
	case 0x0000:
		str = "Success";
		break;
	case 0x0001:
		str = "Failure - unacceptable parameters";
		break;
	case 0x0002:
		str = "Failure - rejected";
		break;
	case 0x0003:
		str = "Failure - unknown options";
		break;
	case 0x0004:
		str = "Pending";
		break;
	case 0x0005:
		str = "Failure - flow spec rejected";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Result: %s (0x%4.4x)", str, le16_to_cpu(result));
}

static struct {
	uint8_t type;
	uint8_t len;
	const char *str;
} options_table[] = {
	{ 0x01,  2, "Maximum Transmission Unit"		},
	{ 0x02,  2, "Flush Timeout"			},
	{ 0x03, 22, "Quality of Service"		},
	{ 0x04,  9, "Retransmission and Flow Control"	},
	{ 0x05,  1, "Frame Check Sequence"		},
	{ 0x06, 16, "Extended Flow Specification"	},
	{ 0x07,  2, "Extended Window Size"		},
        { }
};

static void print_config_options(const struct l2cap_frame *frame,
				uint8_t offset, uint16_t cid, bool response)
{
	const uint8_t *data = frame->data + offset;
	uint16_t size = frame->size - offset;
	uint16_t consumed = 0;

	while (consumed < size - 2) {
		const char *str = "Unknown";
		uint8_t type = data[consumed] & 0x7f;
		uint8_t hint = data[consumed] & 0x80;
		uint8_t len = data[consumed + 1];
		uint8_t expect_len = 0;
		int i;

		for (i = 0; options_table[i].str; i++) {
			if (options_table[i].type == type) {
				str = options_table[i].str;
				expect_len = options_table[i].len;
				break;
			}
		}

		print_field("Option: %s (0x%2.2x) [%s]", str, type,
						hint ? "hint" : "mandatory");

		if (expect_len == 0) {
			consumed += 2;
			break;
		}

		if (len != expect_len) {
			print_text(COLOR_ERROR, "wrong option size (%d != %d)",
							len, expect_len);
			consumed += 2;
			break;
		}

		switch (type) {
		case 0x01:
			print_field("  MTU: %d",
					get_le16(data + consumed + 2));
			break;
		case 0x02:
			print_field("  Flush timeout: %d",
					get_le16(data + consumed + 2));
			break;
		case 0x03:
			switch (data[consumed + 3]) {
			case 0x00:
				str = "No Traffic";
				break;
			case 0x01:
				str = "Best Effort";
				break;
			case 0x02:
				str = "Guaranteed";
				break;
			default:
				str = "Reserved";
				break;
			}
			print_field("  Flags: 0x%2.2x", data[consumed + 2]);
			print_field("  Service type: %s (0x%2.2x)",
						str, data[consumed + 3]);
			print_field("  Token rate: 0x%8.8x",
					get_le32(data + consumed + 4));
			print_field("  Token bucket size: 0x%8.8x",
					get_le32(data + consumed + 8));
			print_field("  Peak bandwidth: 0x%8.8x",
					get_le32(data + consumed + 12));
			print_field("  Latency: 0x%8.8x",
					get_le32(data + consumed + 16));
			print_field("  Delay variation: 0x%8.8x",
					get_le32(data + consumed + 20));
                        break;
		case 0x04:
			if (response)
				assign_mode(frame, data[consumed + 2], cid);

			switch (data[consumed + 2]) {
			case 0x00:
				str = "Basic";
				break;
			case 0x01:
				str = "Retransmission";
				break;
			case 0x02:
				str = "Flow control";
				break;
			case 0x03:
				str = "Enhanced retransmission";
				break;
			case 0x04:
				str = "Streaming";
				break;
			default:
				str = "Reserved";
				break;
			}
			print_field("  Mode: %s (0x%2.2x)",
						str, data[consumed + 2]);
			print_field("  TX window size: %d", data[consumed + 3]);
			print_field("  Max transmit: %d", data[consumed + 4]);
			print_field("  Retransmission timeout: %d",
					get_le16(data + consumed + 5));
			print_field("  Monitor timeout: %d",
					get_le16(data + consumed + 7));
			print_field("  Maximum PDU size: %d",
					get_le16(data + consumed + 9));
			break;
		case 0x05:
			switch (data[consumed + 2]) {
			case 0x00:
				str = "No FCS";
				break;
			case 0x01:
				str = "16-bit FCS";
				break;
			default:
				str = "Reserved";
				break;
			}
			print_field("  FCS: %s (0x%2.2d)",
						str, data[consumed + 2]);
			break;
		case 0x06:
			switch (data[consumed + 3]) {
			case 0x00:
				str = "No traffic";
				break;
			case 0x01:
				str = "Best effort";
				break;
			case 0x02:
				str = "Guaranteed";
				break;
			default:
				str = "Reserved";
				break;
			}
			print_field("  Identifier: 0x%2.2x",
						data[consumed + 2]);
			print_field("  Service type: %s (0x%2.2x)",
						str, data[consumed + 3]);
			print_field("  Maximum SDU size: 0x%4.4x",
					get_le16(data + consumed + 4));
			print_field("  SDU inter-arrival time: 0x%8.8x",
					get_le32(data + consumed + 6));
			print_field("  Access latency: 0x%8.8x",
					get_le32(data + consumed + 10));
			print_field("  Flush timeout: 0x%8.8x",
					get_le32(data + consumed + 14));
			break;
		case 0x07:
			print_field("  Extended window size: %d",
					get_le16(data + consumed + 2));
			assign_ext_ctrl(frame, 1, cid);
			break;
		default:
			packet_hexdump(data + consumed + 2, len);
			break;
		}

		consumed += len + 2;
	}

	if (consumed < size)
		packet_hexdump(data + consumed, size - consumed);
}

static void print_info_type(uint16_t type)
{
	const char *str;

	switch (le16_to_cpu(type)) {
	case 0x0001:
		str = "Connectionless MTU";
		break;
	case 0x0002:
		str = "Extended features supported";
		break;
	case 0x0003:
		str = "Fixed channels supported";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Type: %s (0x%4.4x)", str, le16_to_cpu(type));
}

static void print_info_result(uint16_t result)
{
	const char *str;

	switch (le16_to_cpu(result)) {
	case 0x0000:
		str = "Success";
		break;
	case 0x0001:
		str = "Not supported";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Result: %s (0x%4.4x)", str, le16_to_cpu(result));
}

static struct {
	uint8_t bit;
	const char *str;
} features_table[] = {
	{  0, "Flow control mode"			},
	{  1, "Retransmission mode"			},
	{  2, "Bi-directional QoS"			},
	{  3, "Enhanced Retransmission Mode"		},
	{  4, "Streaming Mode"				},
	{  5, "FCS Option"				},
	{  6, "Extended Flow Specification for BR/EDR"	},
	{  7, "Fixed Channels"				},
	{  8, "Extended Window Size"			},
	{  9, "Unicast Connectionless Data Reception"	},
	{ 31, "Reserved for feature mask extension"	},
	{ }
};

static void print_features(uint32_t features)
{
	uint32_t mask = features;
	int i;

	print_field("Features: 0x%8.8x", features);

	for (i = 0; features_table[i].str; i++) {
		if (features & (1 << features_table[i].bit)) {
			print_field("  %s", features_table[i].str);
			mask &= ~(1 << features_table[i].bit);
		}
	}

	if (mask)
		print_field("  Unknown features (0x%8.8x)", mask);
}

static struct {
	uint16_t cid;
	const char *str;
} channels_table[] = {
	{ 0x0000, "Null identifier"		},
	{ 0x0001, "L2CAP Signaling (BR/EDR)"	},
	{ 0x0002, "Connectionless reception"	},
	{ 0x0003, "AMP Manager Protocol"	},
	{ 0x0004, "Attribute Protocol"		},
	{ 0x0005, "L2CAP Signaling (LE)"	},
	{ 0x0006, "Security Manager (LE)"	},
	{ 0x0007, "Security Manager (BR/EDR)"	},
	{ 0x003f, "AMP Test Manager"		},
	{ }
};

static void print_channels(uint64_t channels)
{
	uint64_t mask = channels;
	int i;

	print_field("Channels: 0x%16.16" PRIx64, channels);

	for (i = 0; channels_table[i].str; i++) {
		if (channels & (1 << channels_table[i].cid)) {
			print_field("  %s", channels_table[i].str);
			mask &= ~(1 << channels_table[i].cid);
		}
	}

	if (mask)
		print_field("  Unknown channels (0x%8.8" PRIx64 ")", mask);
}

static void print_move_result(uint16_t result)
{
	const char *str;

	switch (le16_to_cpu(result)) {
	case 0x0000:
		str = "Move success";
		break;
	case 0x0001:
		str = "Move pending";
		break;
	case 0x0002:
		str = "Move refused - Controller ID not supported";
		break;
	case 0x0003:
		str = "Move refused - new Controller ID is same";
		break;
	case 0x0004:
		str = "Move refused - Configuration not supported";
		break;
	case 0x0005:
		str = "Move refused - Move Channel collision";
		break;
	case 0x0006:
		str = "Move refused - Channel not allowed to be moved";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Result: %s (0x%4.4x)", str, le16_to_cpu(result));
}

static void print_move_cfm_result(uint16_t result)
{
	const char *str;

	switch (le16_to_cpu(result)) {
	case 0x0000:
		str = "Move success - both sides succeed";
		break;
	case 0x0001:
		str = "Move failure - one or both sides refuse";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Result: %s (0x%4.4x)", str, le16_to_cpu(result));
}

static void print_conn_param_result(uint16_t result)
{
	const char *str;

	switch (le16_to_cpu(result)) {
	case 0x0000:
		str = "Connection Parameters accepted";
		break;
	case 0x0001:
		str = "Connection Parameters rejected";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Result: %s (0x%4.4x)", str, le16_to_cpu(result));
}

static void sig_cmd_reject(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_pdu_cmd_reject *pdu = frame->data;
	const void *data = frame->data;
	uint16_t size = frame->size;
	uint16_t scid, dcid;

	print_reject_reason(pdu->reason);

	data += sizeof(*pdu);
	size -= sizeof(*pdu);

	switch (le16_to_cpu(pdu->reason)) {
	case 0x0000:
		if (size != 0) {
			print_text(COLOR_ERROR, "invalid data size");
			packet_hexdump(data, size);
			break;
		}
		break;
	case 0x0001:
		if (size != 2) {
			print_text(COLOR_ERROR, "invalid data size");
			packet_hexdump(data, size);
			break;
		}
		print_field("MTU: %d", get_le16(data));
		break;
	case 0x0002:
		if (size != 4) {
			print_text(COLOR_ERROR, "invalid data size");
			packet_hexdump(data, size);
			break;
		}
		dcid = get_le16(data);
		scid = get_le16(data + 2);
		print_cid("Destination", cpu_to_le16(dcid));
		print_cid("Source", cpu_to_le16(scid));
		break;
	default:
		packet_hexdump(data, size);
		break;
	}
}

static void sig_conn_req(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_pdu_conn_req *pdu = frame->data;

	print_psm(pdu->psm);
	print_cid("Source", pdu->scid);

	assign_scid(frame, le16_to_cpu(pdu->scid), le16_to_cpu(pdu->psm), 0);
}

static void sig_conn_rsp(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_pdu_conn_rsp *pdu = frame->data;

	print_cid("Destination", pdu->dcid);
	print_cid("Source", pdu->scid);
	print_conn_result(pdu->result);
	print_conn_status(pdu->status);

	assign_dcid(frame, le16_to_cpu(pdu->dcid), le16_to_cpu(pdu->scid));
}

static void sig_config_req(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_pdu_config_req *pdu = frame->data;

	print_cid("Destination", pdu->dcid);
	print_config_flags(pdu->flags);
	print_config_options(frame, 4, le16_to_cpu(pdu->dcid), false);
}

static void sig_config_rsp(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_pdu_config_rsp *pdu = frame->data;

	print_cid("Source", pdu->scid);
	print_config_flags(pdu->flags);
	print_config_result(pdu->result);
	print_config_options(frame, 6, le16_to_cpu(pdu->scid), true);
}

static void sig_disconn_req(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_pdu_disconn_req *pdu = frame->data;

	print_cid("Destination", pdu->dcid);
	print_cid("Source", pdu->scid);
}

static void sig_disconn_rsp(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_pdu_disconn_rsp *pdu = frame->data;

	print_cid("Destination", pdu->dcid);
	print_cid("Source", pdu->scid);

	release_scid(frame, le16_to_cpu(pdu->scid));
}

static void sig_echo_req(const struct l2cap_frame *frame)
{
	packet_hexdump(frame->data, frame->size);
}

static void sig_echo_rsp(const struct l2cap_frame *frame)
{
	packet_hexdump(frame->data, frame->size);
}

static void sig_info_req(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_pdu_info_req *pdu = frame->data;

	print_info_type(pdu->type);
}

static void sig_info_rsp(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_pdu_info_rsp *pdu = frame->data;
	const void *data = frame->data;
	uint16_t size = frame->size;

	print_info_type(pdu->type);
	print_info_result(pdu->result);

	data += sizeof(*pdu);
	size -= sizeof(*pdu);

	if (le16_to_cpu(pdu->result) != 0x0000) {
		if (size > 0) {
			print_text(COLOR_ERROR, "invalid data size");
			packet_hexdump(data, size);
		}
		return;
	}

	switch (le16_to_cpu(pdu->type)) {
	case 0x0001:
		if (size != 2) {
			print_text(COLOR_ERROR, "invalid data size");
			packet_hexdump(data, size);
			break;
		}
		print_field("MTU: %d", get_le16(data));
		break;
	case 0x0002:
		if (size != 4) {
			print_text(COLOR_ERROR, "invalid data size");
			packet_hexdump(data, size);
			break;
		}
		print_features(get_le32(data));
		break;
	case 0x0003:
		if (size != 8) {
			print_text(COLOR_ERROR, "invalid data size");
			packet_hexdump(data, size);
			break;
		}
		print_channels(get_le64(data));
		break;
	default:
		packet_hexdump(data, size);
		break;
	}
}

static void sig_create_chan_req(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_pdu_create_chan_req *pdu = frame->data;

	print_psm(pdu->psm);
	print_cid("Source", pdu->scid);
	print_field("Controller ID: %d", pdu->ctrlid);

	assign_scid(frame, le16_to_cpu(pdu->scid), le16_to_cpu(pdu->psm),
								pdu->ctrlid);
}

static void sig_create_chan_rsp(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_pdu_create_chan_rsp *pdu = frame->data;

	print_cid("Destination", pdu->dcid);
	print_cid("Source", pdu->scid);
	print_create_chan_result(pdu->result);
	print_conn_status(pdu->status);

	assign_dcid(frame, le16_to_cpu(pdu->dcid), le16_to_cpu(pdu->scid));
}

static void sig_move_chan_req(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_pdu_move_chan_req *pdu = frame->data;

	print_cid("Initiator", pdu->icid);
	print_field("Controller ID: %d", pdu->ctrlid);
}

static void sig_move_chan_rsp(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_pdu_move_chan_rsp *pdu = frame->data;

	print_cid("Initiator", pdu->icid);
	print_move_result(pdu->result);
}

static void sig_move_chan_cfm(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_pdu_move_chan_cfm *pdu = frame->data;

	print_cid("Initiator", pdu->icid);
	print_move_cfm_result(pdu->result);
}

static void sig_move_chan_cfm_rsp(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_pdu_move_chan_cfm_rsp *pdu = frame->data;

	print_cid("Initiator", pdu->icid);
}

static void sig_conn_param_req(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_pdu_conn_param_req *pdu = frame->data;

	print_field("Min interval: %d", le16_to_cpu(pdu->min_interval));
	print_field("Max interval: %d", le16_to_cpu(pdu->max_interval));
	print_field("Slave latency: %d", le16_to_cpu(pdu->latency));
	print_field("Timeout multiplier: %d", le16_to_cpu(pdu->timeout));
}

static void sig_conn_param_rsp(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_pdu_conn_param_rsp *pdu = frame->data;

	print_conn_param_result(pdu->result);
}

static void sig_le_conn_req(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_pdu_le_conn_req *pdu = frame->data;

	print_psm(pdu->psm);
	print_cid("Source", pdu->scid);
	print_field("MTU: %u", le16_to_cpu(pdu->mtu));
	print_field("MPS: %u", le16_to_cpu(pdu->mps));
	print_field("Credits: %u", le16_to_cpu(pdu->credits));

	assign_scid(frame, le16_to_cpu(pdu->scid), le16_to_cpu(pdu->psm), 0);
}

static void sig_le_conn_rsp(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_pdu_le_conn_rsp *pdu = frame->data;

	print_cid("Destination", pdu->dcid);
	print_field("MTU: %u", le16_to_cpu(pdu->mtu));
	print_field("MPS: %u", le16_to_cpu(pdu->mps));
	print_field("Credits: %u", le16_to_cpu(pdu->credits));
	print_le_conn_result(pdu->result);

	assign_dcid(frame, le16_to_cpu(pdu->dcid), 0);
}

static void sig_le_flowctl_creds(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_pdu_le_flowctl_creds *pdu = frame->data;

	print_cid("Source", pdu->cid);
	print_field("Credits: %u", le16_to_cpu(pdu->credits));
}

struct sig_opcode_data {
	uint8_t opcode;
	const char *str;
	void (*func) (const struct l2cap_frame *frame);
	uint16_t size;
	bool fixed;
};

static const struct sig_opcode_data bredr_sig_opcode_table[] = {
	{ 0x01, "Command Reject",
			sig_cmd_reject, 2, false },
	{ 0x02, "Connection Request",
			sig_conn_req, 4, true },
	{ 0x03, "Connection Response",
			sig_conn_rsp, 8, true },
	{ 0x04, "Configure Request",
			sig_config_req, 4, false },
	{ 0x05, "Configure Response",
			sig_config_rsp, 6, false },
	{ 0x06, "Disconnection Request",
			sig_disconn_req, 4, true },
	{ 0x07, "Disconnection Response",
			sig_disconn_rsp, 4, true },
	{ 0x08, "Echo Request",
			sig_echo_req, 0, false },
	{ 0x09, "Echo Response",
			sig_echo_rsp, 0, false },
	{ 0x0a, "Information Request",
			sig_info_req, 2, true },
	{ 0x0b, "Information Response",
			sig_info_rsp, 4, false },
	{ 0x0c, "Create Channel Request",
			sig_create_chan_req, 5, true },
	{ 0x0d, "Create Channel Response",
			sig_create_chan_rsp, 8, true },
	{ 0x0e, "Move Channel Request",
			sig_move_chan_req, 3, true },
	{ 0x0f, "Move Channel Response",
			sig_move_chan_rsp, 4, true },
	{ 0x10, "Move Channel Confirmation",
			sig_move_chan_cfm, 4, true },
	{ 0x11, "Move Channel Confirmation Response",
			sig_move_chan_cfm_rsp, 2, true },
	{ },
};

static const struct sig_opcode_data le_sig_opcode_table[] = {
	{ 0x01, "Command Reject",
			sig_cmd_reject, 2, false },
	{ 0x06, "Disconnection Request",
			sig_disconn_req, 4, true },
	{ 0x07, "Disconnection Response",
			sig_disconn_rsp, 4, true },
	{ 0x12, "Connection Parameter Update Request",
			sig_conn_param_req, 8, true },
	{ 0x13, "Connection Parameter Update Response",
			sig_conn_param_rsp, 2, true },
	{ 0x14, "LE Connection Request",
			sig_le_conn_req, 10, true },
	{ 0x15, "LE Connection Response",
			sig_le_conn_rsp, 10, true },
	{ 0x16, "LE Flow Control Credit",
			sig_le_flowctl_creds, 4, true },
	{ },
};

static void l2cap_frame_init(struct l2cap_frame *frame, uint16_t index, bool in,
				uint16_t handle, uint8_t ident,
				uint16_t cid, const void *data, uint16_t size)
{
	frame->index   = index;
	frame->in      = in;
	frame->handle  = handle;
	frame->ident   = ident;
	frame->cid     = cid;
	frame->data    = data;
	frame->size    = size;
	frame->psm     = get_psm(frame);
	frame->mode    = get_mode(frame);
	frame->chan    = get_chan(frame);
	frame->seq_num = get_seq_num(frame);
}

static void bredr_sig_packet(uint16_t index, bool in, uint16_t handle,
				uint16_t cid, const void *data, uint16_t size)
{
	struct l2cap_frame frame;

	while (size > 0) {
		const struct bt_l2cap_hdr_sig *hdr = data;
		const struct sig_opcode_data *opcode_data = NULL;
		const char *opcode_color, *opcode_str;
		uint16_t len;
		int i;

		if (size < 4) {
			print_text(COLOR_ERROR, "malformed signal packet");
			packet_hexdump(data, size);
			return;
		}

		len = le16_to_cpu(hdr->len);

		data += 4;
		size -= 4;

		if (size < len) {
			print_text(COLOR_ERROR, "invalid signal packet size");
			packet_hexdump(data, size);
			return;
		}

		for (i = 0; bredr_sig_opcode_table[i].str; i++) {
			if (bredr_sig_opcode_table[i].opcode == hdr->code) {
				opcode_data = &bredr_sig_opcode_table[i];
				break;
			}
		}

		if (opcode_data) {
			if (opcode_data->func) {
				if (in)
					opcode_color = COLOR_MAGENTA;
				else
					opcode_color = COLOR_BLUE;
			} else
				opcode_color = COLOR_WHITE_BG;
			opcode_str = opcode_data->str;
		} else {
			opcode_color = COLOR_WHITE_BG;
			opcode_str = "Unknown";
		}

		print_indent(6, opcode_color, "L2CAP: ", opcode_str,
					COLOR_OFF,
					" (0x%2.2x) ident %d len %d",
					hdr->code, hdr->ident, len);

		if (!opcode_data || !opcode_data->func) {
			packet_hexdump(data, len);
			data += len;
			size -= len;
			return;
		}

		if (opcode_data->fixed) {
			if (len != opcode_data->size) {
				print_text(COLOR_ERROR, "invalid size");
				packet_hexdump(data, len);
				data += len;
				size -= len;
				continue;
			}
		} else {
			if (len < opcode_data->size) {
				print_text(COLOR_ERROR, "too short packet");
				packet_hexdump(data, size);
				data += len;
				size -= len;
				continue;
			}
		}

		l2cap_frame_init(&frame, index, in, handle, hdr->ident, cid,
								data, len);
		opcode_data->func(&frame);

		data += len;
		size -= len;
	}

	packet_hexdump(data, size);
}

static void le_sig_packet(uint16_t index, bool in, uint16_t handle,
				uint16_t cid, const void *data, uint16_t size)
{
	struct l2cap_frame frame;
	const struct bt_l2cap_hdr_sig *hdr = data;
	const struct sig_opcode_data *opcode_data = NULL;
	const char *opcode_color, *opcode_str;
	uint16_t len;
	int i;

	if (size < 4) {
		print_text(COLOR_ERROR, "malformed signal packet");
		packet_hexdump(data, size);
		return;
	}

	len = le16_to_cpu(hdr->len);

	data += 4;
	size -= 4;

	if (size != len) {
		print_text(COLOR_ERROR, "invalid signal packet size");
		packet_hexdump(data, size);
		return;
	}

	for (i = 0; le_sig_opcode_table[i].str; i++) {
		if (le_sig_opcode_table[i].opcode == hdr->code) {
			opcode_data = &le_sig_opcode_table[i];
			break;
		}
	}

	if (opcode_data) {
		if (opcode_data->func) {
			if (in)
				opcode_color = COLOR_MAGENTA;
			else
				opcode_color = COLOR_BLUE;
		} else
			opcode_color = COLOR_WHITE_BG;
		opcode_str = opcode_data->str;
	} else {
		opcode_color = COLOR_WHITE_BG;
		opcode_str = "Unknown";
	}

	print_indent(6, opcode_color, "LE L2CAP: ", opcode_str, COLOR_OFF,
					" (0x%2.2x) ident %d len %d",
					hdr->code, hdr->ident, len);

	if (!opcode_data || !opcode_data->func) {
		packet_hexdump(data, len);
		return;
	}

	if (opcode_data->fixed) {
		if (len != opcode_data->size) {
			print_text(COLOR_ERROR, "invalid size");
			packet_hexdump(data, len);
			return;
		}
	} else {
		if (len < opcode_data->size) {
			print_text(COLOR_ERROR, "too short packet");
			packet_hexdump(data, size);
			return;
		}
	}

	l2cap_frame_init(&frame, index, in, handle, hdr->ident, cid, data, len);
	opcode_data->func(&frame);
}

static void connless_packet(uint16_t index, bool in, uint16_t handle,
				uint16_t cid, const void *data, uint16_t size)
{
	struct l2cap_frame frame;
	const struct bt_l2cap_hdr_connless *hdr = data;
	uint16_t psm;

	if (size < 2) {
		print_text(COLOR_ERROR, "malformed connectionless packet");
		packet_hexdump(data, size);
		return;
	}

	psm = le16_to_cpu(hdr->psm);

	data += 2;
	size -= 2;

	print_indent(6, COLOR_CYAN, "L2CAP: Connectionless", "", COLOR_OFF,
						" len %d [PSM %d]", size, psm);

	switch (psm) {
	default:
		packet_hexdump(data, size);
		break;
	}

	l2cap_frame_init(&frame, index, in, handle, 0, cid, data, size);
}

static void print_controller_list(const uint8_t *data, uint16_t size)
{
	while (size > 2) {
		const char *str;

		print_field("Controller ID: %d", data[0]);

		switch (data[1]) {
		case 0x00:
			str = "Primary BR/EDR Controller";
			break;
		case 0x01:
			str = "802.11 AMP Controller";
			break;
		default:
			str = "Reserved";
			break;
		}

		print_field("  Type: %s (0x%2.2x)", str, data[1]);

		switch (data[2]) {
		case 0x00:
			str = "Present";
			break;
		case 0x01:
			str = "Bluetooth only";
			break;
		case 0x02:
			str = "No capacity";
			break;
		case 0x03:
			str = "Low capacity";
			break;
		case 0x04:
			str = "Medium capacity";
			break;
		case 0x05:
			str = "High capacity";
			break;
		case 0x06:
			str = "Full capacity";
			break;
		default:
			str = "Reserved";
			break;
		}

		print_field("  Status: %s (0x%2.2x)", str, data[2]);

		data += 3;
		size -= 3;
	}

	packet_hexdump(data, size);
}

static void amp_cmd_reject(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_amp_cmd_reject *pdu = frame->data;

	print_field("Reason: 0x%4.4x", le16_to_cpu(pdu->reason));
}

static void amp_discover_req(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_amp_discover_req *pdu = frame->data;

	print_field("MTU/MPS size: %d", le16_to_cpu(pdu->size));
	print_field("Extended feature mask: 0x%4.4x",
					le16_to_cpu(pdu->features));
}

static void amp_discover_rsp(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_amp_discover_rsp *pdu = frame->data;

	print_field("MTU/MPS size: %d", le16_to_cpu(pdu->size));
	print_field("Extended feature mask: 0x%4.4x",
					le16_to_cpu(pdu->features));

	print_controller_list(frame->data + 4, frame->size - 4);
}

static void amp_change_notify(const struct l2cap_frame *frame)
{
	print_controller_list(frame->data, frame->size);
}

static void amp_change_response(const struct l2cap_frame *frame)
{
}

static void amp_get_info_req(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_amp_get_info_req *pdu = frame->data;

	print_field("Controller ID: %d", pdu->ctrlid);
}

static void amp_get_info_rsp(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_amp_get_info_rsp *pdu = frame->data;
	const char *str;

	print_field("Controller ID: %d", pdu->ctrlid);

	switch (pdu->status) {
	case 0x00:
		str = "Success";
		break;
	case 0x01:
		str = "Invalid Controller ID";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Status: %s (0x%2.2x)", str, pdu->status);

	print_field("Total bandwidth: %d kbps", le32_to_cpu(pdu->total_bw));
	print_field("Max guaranteed bandwidth: %d kbps",
						le32_to_cpu(pdu->max_bw));
	print_field("Min latency: %d", le32_to_cpu(pdu->min_latency));

	print_field("PAL capabilities: 0x%4.4x", le16_to_cpu(pdu->pal_cap));
	print_field("Max ASSOC length: %d", le16_to_cpu(pdu->max_assoc_len));
}

static void amp_get_assoc_req(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_amp_get_assoc_req *pdu = frame->data;

	print_field("Controller ID: %d", pdu->ctrlid);
}

static void amp_get_assoc_rsp(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_amp_get_assoc_rsp *pdu = frame->data;
	const char *str;

	print_field("Controller ID: %d", pdu->ctrlid);

	switch (pdu->status) {
	case 0x00:
		str = "Success";
		break;
	case 0x01:
		str = "Invalid Controller ID";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Status: %s (0x%2.2x)", str, pdu->status);

	packet_hexdump(frame->data + 2, frame->size - 2);
}

static void amp_create_phy_link_req(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_amp_create_phy_link_req *pdu = frame->data;

	print_field("Local controller ID: %d", pdu->local_ctrlid);
	print_field("Remote controller ID: %d", pdu->remote_ctrlid);

	packet_hexdump(frame->data + 2, frame->size - 2);
}

static void amp_create_phy_link_rsp(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_amp_create_phy_link_rsp *pdu = frame->data;
	const char *str;

	print_field("Local controller ID: %d", pdu->local_ctrlid);
	print_field("Remote controller ID: %d", pdu->remote_ctrlid);

	switch (pdu->status) {
	case 0x00:
		str = "Success";
		break;
	case 0x01:
		str = "Invalid Controller ID";
		break;
	case 0x02:
		str = "Failed - Unable to start link creation";
		break;
	case 0x03:
		str = "Failed - Collision occurred";
		break;
	case 0x04:
		str = "Failed - Disconnected link packet received";
		break;
	case 0x05:
		str = "Failed - Link already exists";
		break;
	case 0x06:
		str = "Failed - Security violation";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Status: %s (0x%2.2x)", str, pdu->status);
}

static void amp_disconn_phy_link_req(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_amp_disconn_phy_link_req *pdu = frame->data;

	print_field("Local controller ID: %d", pdu->local_ctrlid);
	print_field("Remote controller ID: %d", pdu->remote_ctrlid);
}

static void amp_disconn_phy_link_rsp(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_amp_disconn_phy_link_rsp *pdu = frame->data;
	const char *str;

	print_field("Local controller ID: %d", pdu->local_ctrlid);
	print_field("Remote controller ID: %d", pdu->remote_ctrlid);

	switch (pdu->status) {
	case 0x00:
		str = "Success";
		break;
	case 0x01:
		str = "Invalid Controller ID";
		break;
	case 0x02:
		str = "Failed - No link exists";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Status: %s (0x%2.2x)", str, pdu->status);
}

struct amp_opcode_data {
	uint8_t opcode;
	const char *str;
	void (*func) (const struct l2cap_frame *frame);
	uint16_t size;
	bool fixed;
};

static const struct amp_opcode_data amp_opcode_table[] = {
	{ 0x01, "Command Reject",
			amp_cmd_reject, 2, false },
	{ 0x02, "Discover Request",
			amp_discover_req, 4, true },
	{ 0x03, "Discover Response",
			amp_discover_rsp, 7, false },
	{ 0x04, "Change Notify",
			amp_change_notify, 3, false },
	{ 0x05, "Change Response",
			amp_change_response, 0, true },
	{ 0x06, "Get Info Request",
			amp_get_info_req, 1, true },
	{ 0x07, "Get Info Response",
			amp_get_info_rsp, 18, true },
	{ 0x08, "Get Assoc Request",
			amp_get_assoc_req, 1, true },
	{ 0x09, "Get Assoc Response",
			amp_get_assoc_rsp, 2, false },
	{ 0x0a, "Create Physical Link Request",
			amp_create_phy_link_req, 2, false },
	{ 0x0b, "Create Physical Link Response",
			amp_create_phy_link_rsp, 3, true },
	{ 0x0c, "Disconnect Physical Link Request",
			amp_disconn_phy_link_req, 2, true },
	{ 0x0d, "Disconnect Physical Link Response",
			amp_disconn_phy_link_rsp, 3, true },
	{ },
};

static void amp_packet(uint16_t index, bool in, uint16_t handle,
			uint16_t cid, const void *data, uint16_t size)
{
	struct l2cap_frame frame;
	uint16_t control, fcs, len;
	uint8_t opcode, ident;
	const struct amp_opcode_data *opcode_data = NULL;
	const char *opcode_color, *opcode_str;
	int i;

	if (size < 4) {
		print_text(COLOR_ERROR, "malformed info frame packet");
		packet_hexdump(data, size);
		return;
	}

	control = get_le16(data);
	fcs = get_le16(data + size - 2);

	print_indent(6, COLOR_CYAN, "Channel:", "", COLOR_OFF,
				" %d dlen %d control 0x%4.4x fcs 0x%4.4x",
						3, size, control, fcs);

	if (control & 0x01)
		return;

	if (size < 8) {
		print_text(COLOR_ERROR, "malformed manager packet");
		packet_hexdump(data, size);
		return;
	}

	opcode = *((const uint8_t *) (data + 2));
	ident = *((const uint8_t *) (data + 3));
	len = get_le16(data + 4);

	if (len != size - 8) {
		print_text(COLOR_ERROR, "invalid manager packet size");
		packet_hexdump(data +  2, size - 4);
		return;
	}

	for (i = 0; amp_opcode_table[i].str; i++) {
		if (amp_opcode_table[i].opcode == opcode) {
			opcode_data = &amp_opcode_table[i];
			break;
		}
	}

	if (opcode_data) {
		if (opcode_data->func) {
			if (in)
				opcode_color = COLOR_MAGENTA;
			else
				opcode_color = COLOR_BLUE;
		} else
			opcode_color = COLOR_WHITE_BG;
		opcode_str = opcode_data->str;
	} else {
		opcode_color = COLOR_WHITE_BG;
		opcode_str = "Unknown";
	}

	print_indent(6, opcode_color, "AMP: ", opcode_str, COLOR_OFF,
			" (0x%2.2x) ident %d len %d", opcode, ident, len);

	if (!opcode_data || !opcode_data->func) {
		packet_hexdump(data + 6, size - 8);
		return;
	}

	if (opcode_data->fixed) {
		if (len != opcode_data->size) {
			print_text(COLOR_ERROR, "invalid size");
			packet_hexdump(data + 6, size - 8);
			return;
		}
	} else {
		if (len < opcode_data->size) {
			print_text(COLOR_ERROR, "too short packet");
			packet_hexdump(data + 6, size - 8);
			return;
		}
	}

	l2cap_frame_init(&frame, index, in, handle, 0, cid, data + 6, len);
	opcode_data->func(&frame);
}

static void print_hex_field(const char *label, const uint8_t *data,
								uint8_t len)
{
	char *str;
	uint8_t i;

	str = (char *)malloc(len * 2 + 1);
	if (!str)
		return;
	str[0] = '\0';

	for (i = 0; i < len; i++)
		sprintf(str + (i * 2), "%2.2x", data[i]);

	print_field("%s: %s", label, str);
	free(str);
}

static void print_uuid(const char *label, const void *data, uint16_t size)
{
	const char *str;
	char uuidstr[36];

	switch (size) {
	case 2:
		str = uuid16_to_str(get_le16(data));
		print_field("%s: %s (0x%4.4x)", label, str, get_le16(data));
		break;
	case 4:
		str = uuid32_to_str(get_le32(data));
		print_field("%s: %s (0x%8.8x)", label, str, get_le32(data));
		break;
	case 16:
		sprintf(uuidstr, "%8.8x-%4.4x-%4.4x-%4.4x-%8.8x%4.4x",
				get_le32(data + 12), get_le16(data + 10),
				get_le16(data + 8), get_le16(data + 6),
				get_le32(data + 2), get_le16(data + 0));
		str = uuidstr_to_str(uuidstr);
		print_field("%s: %s (%s)", label, str, uuidstr);
		break;
	default:
		packet_hexdump(data, size);
		break;
	}
}

static void print_handle_range(const char *label, const void *data)
{
	print_field("%s: 0x%4.4x-0x%4.4x", label,
				get_le16(data), get_le16(data + 2));
}

static void print_data_list(const char *label, uint8_t length,
					const void *data, uint16_t size)
{
	uint8_t count;

	if (length == 0)
		return;

	count = size / length;

	print_field("%s: %u entr%s", label, count, count == 1 ? "y" : "ies");

	while (size >= length) {
		print_field("Handle: 0x%4.4x", get_le16(data));
		print_hex_field("Value", data + 2, length - 2);

		data += length;
		size -= length;
	}

	packet_hexdump(data, size);
}

static void print_attribute_info(uint16_t type, const void *data, uint16_t len)
{
	const char *str = uuid16_to_str(type);

	print_field("%s: %s (0x%4.4x)", "Attribute type", str, type);

	switch (type) {
	case 0x2800:	/* Primary Service */
	case 0x2801:	/* Secondary Service */
		print_uuid("  UUID", data, len);
		break;
	case 0x2802:	/* Include */
		if (len < 4) {
			print_hex_field("  Value", data, len);
			break;
		}
		print_handle_range("  Handle range", data);
		print_uuid("  UUID", data + 4, len - 4);
		break;
	case 0x2803:	/* Characteristic */
		if (len < 3) {
			print_hex_field("  Value", data, len);
			break;
		}
		print_field("  Properties: 0x%2.2x", *((uint8_t *) data));
		print_field("  Handle: 0x%2.2x", get_le16(data + 1));
		print_uuid("  UUID", data + 3, len - 3);
		break;
	default:
		print_hex_field("Value", data, len);
		break;
	}
}

static const char *att_opcode_to_str(uint8_t opcode);

static void att_error_response(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_att_error_response *pdu = frame->data;
	const char *str;

	switch (pdu->error) {
	case 0x01:
		str = "Invalid Handle";
		break;
	case 0x02:
		str = "Read Not Permitted";
		break;
	case 0x03:
		str = "Write Not Permitted";
		break;
	case 0x04:
		str = "Invalid PDU";
		break;
	case 0x05:
		str = "Insufficient Authentication";
		break;
	case 0x06:
		str = "Request Not Supported";
		break;
	case 0x07:
		str = "Invalid Offset";
		break;
	case 0x08:
		str = "Insufficient Authorization";
		break;
	case 0x09:
		str = "Prepare Queue Full";
		break;
	case 0x0a:
		str = "Attribute Not Found";
		break;
	case 0x0b:
		str = "Attribute Not Long";
		break;
	case 0x0c:
		str = "Insufficient Encryption Key Size";
		break;
	case 0x0d:
		str = "Invalid Attribute Value Length";
		break;
	case 0x0e:
		str = "Unlikely Error";
		break;
	case 0x0f:
		str = "Insufficient Encryption";
		break;
	case 0x10:
		str = "Unsupported Group Type";
		break;
	case 0x11:
		str = "Insufficient Resources";
		break;
	case 0xfd:
		str = "CCC Improperly Configured";
		break;
	case 0xfe:
		str = "Procedure Already in Progress";
		break;
	case 0xff:
		str = "Out of Range";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("%s (0x%2.2x)", att_opcode_to_str(pdu->request),
							pdu->request);
	print_field("Handle: 0x%4.4x", le16_to_cpu(pdu->handle));
	print_field("Error: %s (0x%2.2x)", str, pdu->error);
}

static void att_exchange_mtu_req(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_att_exchange_mtu_req *pdu = frame->data;

	print_field("Client RX MTU: %d", le16_to_cpu(pdu->mtu));
}

static void att_exchange_mtu_rsp(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_att_exchange_mtu_rsp *pdu = frame->data;

	print_field("Server RX MTU: %d", le16_to_cpu(pdu->mtu));
}

static void att_find_info_req(const struct l2cap_frame *frame)
{
	print_handle_range("Handle range", frame->data);
}

static const char *att_format_str(uint8_t format)
{
	switch (format) {
	case 0x01:
		return "UUID-16";
	case 0x02:
		return "UUID-128";
	default:
		return "unknown";
	}
}

static uint16_t print_info_data_16(const void *data, uint16_t len)
{
	while (len >= 4) {
		print_field("Handle: 0x%4.4x", get_le16(data));
		print_uuid("UUID", data + 2, 2);
		data += 4;
		len -= 4;
	}

	return len;
}

static uint16_t print_info_data_128(const void *data, uint16_t len)
{
	while (len >= 18) {
		print_field("Handle: 0x%4.4x", get_le16(data));
		print_uuid("UUID", data + 2, 16);
		data += 18;
		len -= 18;
	}

	return len;
}

static void att_find_info_rsp(const struct l2cap_frame *frame)
{
	const uint8_t *format = frame->data;
	uint16_t len;

	print_field("Format: %s (0x%2.2x)", att_format_str(*format), *format);

	if (*format == 0x01)
		len = print_info_data_16(frame->data + 1, frame->size - 1);
	else if (*format == 0x02)
		len = print_info_data_128(frame->data + 1, frame->size - 1);
	else
		len = frame->size - 1;

	packet_hexdump(frame->data + (frame->size - len), len);
}

static void att_find_by_type_val_req(const struct l2cap_frame *frame)
{
	uint16_t type;

	print_handle_range("Handle range", frame->data);

	type = get_le16(frame->data + 4);
	print_attribute_info(type, frame->data + 6, frame->size - 6);
}

static void att_find_by_type_val_rsp(const struct l2cap_frame *frame)
{
	const uint8_t *ptr = frame->data;
	uint16_t len = frame->size;

	while (len >= 4) {
		print_handle_range("Handle range", ptr);
		ptr += 4;
		len -= 4;
	}

	packet_hexdump(ptr, len);
}

static void att_read_type_req(const struct l2cap_frame *frame)
{
	print_handle_range("Handle range", frame->data);
	print_uuid("Attribute type", frame->data + 4, frame->size - 4);
}

static void att_read_type_rsp(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_att_read_group_type_rsp *pdu = frame->data;

	print_field("Attribute data length: %d", pdu->length);
	print_data_list("Attribute data list", pdu->length,
					frame->data + 1, frame->size - 1);
}

static void att_read_req(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_att_read_req *pdu = frame->data;

	print_field("Handle: 0x%4.4x", le16_to_cpu(pdu->handle));
}

static void att_read_rsp(const struct l2cap_frame *frame)
{
	print_hex_field("Value", frame->data, frame->size);
}

static void att_read_blob_req(const struct l2cap_frame *frame)
{
	print_field("Handle: 0x%4.4x", get_le16(frame->data));
	print_field("Offset: 0x%4.4x", get_le16(frame->data + 2));
}

static void att_read_blob_rsp(const struct l2cap_frame *frame)
{
	packet_hexdump(frame->data, frame->size);
}

static void att_read_multiple_req(const struct l2cap_frame *frame)
{
	int i, count;

	count = frame->size / 2;

	for (i = 0; i < count; i++)
		print_field("Handle: 0x%4.4x",
					get_le16(frame->data + (i * 2)));
}

static void att_read_group_type_req(const struct l2cap_frame *frame)
{
	print_handle_range("Handle range", frame->data);
	print_uuid("Attribute group type", frame->data + 4, frame->size - 4);
}

static void print_group_list(const char *label, uint8_t length,
					const void *data, uint16_t size)
{
	uint8_t count;

	if (length == 0)
		return;

	count = size / length;

	print_field("%s: %u entr%s", label, count, count == 1 ? "y" : "ies");

	while (size >= length) {
		print_handle_range("Handle range", data);
		print_uuid("UUID", data + 4, length - 4);

		data += length;
		size -= length;
	}

	packet_hexdump(data, size);
}

static void att_read_group_type_rsp(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_att_read_group_type_rsp *pdu = frame->data;

	print_field("Attribute data length: %d", pdu->length);
	print_group_list("Attribute group list", pdu->length,
					frame->data + 1, frame->size - 1);
}

static void att_write_req(const struct l2cap_frame *frame)
{
	print_field("Handle: 0x%4.4x", get_le16(frame->data));
	print_hex_field("  Data", frame->data + 2, frame->size - 2);
}

static void att_write_rsp(const struct l2cap_frame *frame)
{
}

static void att_prepare_write_req(const struct l2cap_frame *frame)
{
	print_field("Handle: 0x%4.4x", get_le16(frame->data));
	print_field("Offset: 0x%4.4x", get_le16(frame->data + 2));
	print_hex_field("  Data", frame->data + 4, frame->size - 4);
}

static void att_prepare_write_rsp(const struct l2cap_frame *frame)
{
	print_field("Handle: 0x%4.4x", get_le16(frame->data));
	print_field("Offset: 0x%4.4x", get_le16(frame->data + 2));
	print_hex_field("  Data", frame->data + 4, frame->size - 4);
}

static void att_execute_write_req(const struct l2cap_frame *frame)
{
	uint8_t flags = *(uint8_t *) frame->data;
	const char *flags_str;

	switch (flags) {
	case 0x00:
		flags_str = "Cancel all prepared writes";
		break;
	case 0x01:
		flags_str = "Immediately write all pending values";
		break;
	default:
		flags_str = "Unknown";
		break;
	}

	print_field("Flags: %s (0x%02x)", flags_str, flags);
}

static void att_handle_value_notify(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_att_handle_value_notify *pdu = frame->data;

	print_field("Handle: 0x%4.4x", le16_to_cpu(pdu->handle));
	print_hex_field("  Data", frame->data + 2, frame->size - 2);
}

static void att_handle_value_ind(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_att_handle_value_ind *pdu = frame->data;

	print_field("Handle: 0x%4.4x", le16_to_cpu(pdu->handle));
	print_hex_field("  Data", frame->data + 2, frame->size - 2);
}

static void att_handle_value_conf(const struct l2cap_frame *frame)
{
}

static void att_write_command(const struct l2cap_frame *frame)
{
	print_field("Handle: 0x%4.4x", get_le16(frame->data));
	print_hex_field("  Data", frame->data + 2, frame->size - 2);
}

static void att_signed_write_command(const struct l2cap_frame *frame)
{
	print_field("Handle: 0x%4.4x", get_le16(frame->data));
	print_hex_field("  Data", frame->data + 2, frame->size - 2 - 12);
	print_hex_field("  Signature", frame->data + frame->size - 12, 12);
}

struct att_opcode_data {
	uint8_t opcode;
	const char *str;
	void (*func) (const struct l2cap_frame *frame);
	uint8_t size;
	bool fixed;
};

static const struct att_opcode_data att_opcode_table[] = {
	{ 0x01, "Error Response",
			att_error_response, 4, true },
	{ 0x02, "Exchange MTU Request",
			att_exchange_mtu_req, 2, true },
	{ 0x03, "Exchange MTU Response",
			att_exchange_mtu_rsp, 2, true },
	{ 0x04, "Find Information Request",
			att_find_info_req, 4, true },
	{ 0x05, "Find Information Response",
			att_find_info_rsp, 5, false },
	{ 0x06, "Find By Type Value Request",
			att_find_by_type_val_req, 6, false },
	{ 0x07, "Find By Type Value Response",
			att_find_by_type_val_rsp, 4, false },
	{ 0x08, "Read By Type Request",
			att_read_type_req, 6, false },
	{ 0x09, "Read By Type Response",
			att_read_type_rsp, 3, false },
	{ 0x0a, "Read Request",
			att_read_req, 2, true },
	{ 0x0b, "Read Response",
			att_read_rsp, 0, false },
	{ 0x0c, "Read Blob Request",
			att_read_blob_req, 4, true },
	{ 0x0d, "Read Blob Response",
			att_read_blob_rsp, 0, false },
	{ 0x0e, "Read Multiple Request",
			att_read_multiple_req, 4, false },
	{ 0x0f, "Read Multiple Response"	},
	{ 0x10, "Read By Group Type Request",
			att_read_group_type_req, 6, false },
	{ 0x11, "Read By Group Type Response",
			att_read_group_type_rsp, 4, false },
	{ 0x12, "Write Request"	,
			att_write_req, 2, false	},
	{ 0x13, "Write Response",
			att_write_rsp, 0, true	},
	{ 0x16, "Prepare Write Request",
			att_prepare_write_req, 4, false },
	{ 0x17, "Prepare Write Response",
			att_prepare_write_rsp, 4, false },
	{ 0x18, "Execute Write Request",
			att_execute_write_req, 1, true },
	{ 0x19, "Execute Write Response"	},
	{ 0x1b, "Handle Value Notification",
			att_handle_value_notify, 2, false },
	{ 0x1d, "Handle Value Indication",
			att_handle_value_ind, 2, false },
	{ 0x1e, "Handle Value Confirmation",
			att_handle_value_conf, 0, true },
	{ 0x52, "Write Command",
			att_write_command, 2, false },
	{ 0xd2, "Signed Write Command", att_signed_write_command, 14, false },
	{ }
};

static const char *att_opcode_to_str(uint8_t opcode)
{
	int i;

	for (i = 0; att_opcode_table[i].str; i++) {
		if (att_opcode_table[i].opcode == opcode)
			return att_opcode_table[i].str;
	}

	return "Unknown";
}

static void att_packet(uint16_t index, bool in, uint16_t handle,
			uint16_t cid, const void *data, uint16_t size)
{
	struct l2cap_frame frame;
	uint8_t opcode = *((const uint8_t *) data);
	const struct att_opcode_data *opcode_data = NULL;
	const char *opcode_color, *opcode_str;
	int i;

	if (size < 1) {
		print_text(COLOR_ERROR, "malformed attribute packet");
		packet_hexdump(data, size);
		return;
	}

	for (i = 0; att_opcode_table[i].str; i++) {
		if (att_opcode_table[i].opcode == opcode) {
			opcode_data = &att_opcode_table[i];
			break;
		}
	}

	if (opcode_data) {
		if (opcode_data->func) {
			if (in)
				opcode_color = COLOR_MAGENTA;
			else
				opcode_color = COLOR_BLUE;
		} else
			opcode_color = COLOR_WHITE_BG;
		opcode_str = opcode_data->str;
	} else {
		opcode_color = COLOR_WHITE_BG;
		opcode_str = "Unknown";
	}

	print_indent(6, opcode_color, "ATT: ", opcode_str, COLOR_OFF,
				" (0x%2.2x) len %d", opcode, size - 1);

	if (!opcode_data || !opcode_data->func) {
		packet_hexdump(data + 1, size - 1);
		return;
	}

	if (opcode_data->fixed) {
		if (size - 1 != opcode_data->size) {
			print_text(COLOR_ERROR, "invalid size");
			packet_hexdump(data + 1, size - 1);
			return;
		}
	} else {
		if (size - 1 < opcode_data->size) {
			print_text(COLOR_ERROR, "too short packet");
			packet_hexdump(data + 1, size - 1);
			return;
		}
	}

	l2cap_frame_init(&frame, index, in, handle, 0, cid, data + 1, size - 1);
	opcode_data->func(&frame);
}

static void print_addr(const uint8_t *addr, uint8_t addr_type)
{
	const char *str;

	switch (addr_type) {
	case 0x00:
		print_field("Address: %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
						addr[5], addr[4], addr[3],
						addr[2], addr[1], addr[0]);
		break;
	case 0x01:
		switch ((addr[5] & 0xc0) >> 6) {
		case 0x00:
			str = "Non-Resolvable";
			break;
		case 0x01:
			str = "Resolvable";
			break;
		case 0x03:
			str = "Static";
			break;
		default:
			str = "Reserved";
			break;
		}

		print_field("Address: %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X"
					" (%s)", addr[5], addr[4], addr[3],
					addr[2], addr[1], addr[0], str);
		break;
	default:
		print_field("Address: %2.2X-%2.2X-%2.2X-%2.2X-%2.2X-%2.2X",
						addr[5], addr[4], addr[3],
						addr[2], addr[1], addr[0]);
		break;
	}
}

static void print_addr_type(uint8_t addr_type)
{
	const char *str;

	switch (addr_type) {
	case 0x00:
		str = "Public";
		break;
	case 0x01:
		str = "Random";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Address type: %s (0x%2.2x)", str, addr_type);
}

static void print_smp_io_capa(uint8_t io_capa)
{
	const char *str;

	switch (io_capa) {
	case 0x00:
		str = "DisplayOnly";
		break;
	case 0x01:
		str = "DisplayYesNo";
		break;
	case 0x02:
		str = "KeyboardOnly";
		break;
	case 0x03:
		str = "NoInputNoOutput";
		break;
	case 0x04:
		str = "KeyboardDisplay";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("IO capability: %s (0x%2.2x)", str, io_capa);
}

static void print_smp_oob_data(uint8_t oob_data)
{
	const char *str;

	switch (oob_data) {
	case 0x00:
		str = "Authentication data not present";
		break;
	case 0x01:
		str = "Authentication data from remote device present";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("OOB data: %s (0x%2.2x)", str, oob_data);
}

static void print_smp_auth_req(uint8_t auth_req)
{
	const char *bond, *mitm, *sc, *kp;

	switch (auth_req & 0x03) {
	case 0x00:
		bond = "No bonding";
		break;
	case 0x01:
		bond = "Bonding";
		break;
	default:
		bond = "Reserved";
		break;
	}

	if ((auth_req & 0x04))
		mitm = "MITM";
	else
		mitm = "No MITM";

	if ((auth_req & 0x08))
		sc = "SC";
	else
		sc = "Legacy";

	if ((auth_req & 0x10))
		kp = "Keypresses";
	else
		kp = "No Keypresses";

	print_field("Authentication requirement: %s, %s, %s, %s (0x%2.2x)",
						bond, mitm, sc, kp, auth_req);
}

static void print_smp_key_dist(const char *label, uint8_t dist)
{
	char str[27];

	if (!(dist & 0x07)) {
		strcpy(str, "<none> ");
	} else {
		str[0] = '\0';
		if (dist & 0x01)
			strcat(str, "EncKey ");
		if (dist & 0x02)
			strcat(str, "IdKey ");
		if (dist & 0x04)
			strcat(str, "Sign ");
		if (dist & 0x08)
			strcat(str, "LinkKey ");
	}

	print_field("%s: %s(0x%2.2x)", label, str, dist);
}

static void smp_pairing_request(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_smp_pairing_request *pdu = frame->data;

	print_smp_io_capa(pdu->io_capa);
	print_smp_oob_data(pdu->oob_data);
	print_smp_auth_req(pdu->auth_req);

	print_field("Max encryption key size: %d", pdu->max_key_size);
	print_smp_key_dist("Initiator key distribution", pdu->init_key_dist);
	print_smp_key_dist("Responder key distribution", pdu->resp_key_dist);
}

static void smp_pairing_response(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_smp_pairing_response *pdu = frame->data;

	print_smp_io_capa(pdu->io_capa);
	print_smp_oob_data(pdu->oob_data);
	print_smp_auth_req(pdu->auth_req);

	print_field("Max encryption key size: %d", pdu->max_key_size);
	print_smp_key_dist("Initiator key distribution", pdu->init_key_dist);
	print_smp_key_dist("Responder key distribution", pdu->resp_key_dist);
}

static void smp_pairing_confirm(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_smp_pairing_confirm *pdu = frame->data;

	print_hex_field("Confim value", pdu->value, 16);
}

static void smp_pairing_random(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_smp_pairing_random *pdu = frame->data;

	print_hex_field("Random value", pdu->value, 16);
}

static void smp_pairing_failed(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_smp_pairing_failed *pdu = frame->data;
	const char *str;

	switch (pdu->reason) {
	case 0x01:
		str = "Passkey entry failed";
		break;
	case 0x02:
		str = "OOB not available";
		break;
	case 0x03:
		str = "Authentication requirements";
		break;
	case 0x04:
		str = "Confirm value failed";
		break;
	case 0x05:
		str = "Pairing not supported";
		break;
	case 0x06:
		str = "Encryption key size";
		break;
	case 0x07:
		str = "Command not supported";
		break;
	case 0x08:
		str = "Unspecified reason";
		break;
	case 0x09:
		str = "Repeated attempts";
		break;
	case 0x0a:
		str = "Invalid parameters";
		break;
	case 0x0b:
		str = "DHKey check failed";
		break;
	case 0x0c:
		str = "Numeric comparison failed";
		break;
	case 0x0d:
		str = "BR/EDR pairing in progress";
		break;
	case 0x0e:
		str = "Cross-transport Key Derivation/Generation not allowed";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Reason: %s (0x%2.2x)", str, pdu->reason);
}

static void smp_encrypt_info(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_smp_encrypt_info *pdu = frame->data;

	print_hex_field("Long term key", pdu->ltk, 16);
}

static void smp_master_ident(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_smp_master_ident *pdu = frame->data;

	print_field("EDIV: 0x%4.4x", le16_to_cpu(pdu->ediv));
	print_field("Rand: 0x%16.16" PRIx64, le64_to_cpu(pdu->rand));
}

static void smp_ident_info(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_smp_ident_info *pdu = frame->data;

	print_hex_field("Identity resolving key", pdu->irk, 16);

	keys_update_identity_key(pdu->irk);
}

static void smp_ident_addr_info(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_smp_ident_addr_info *pdu = frame->data;

	print_addr_type(pdu->addr_type);
	print_addr(pdu->addr, pdu->addr_type);

	keys_update_identity_addr(pdu->addr, pdu->addr_type);
}

static void smp_signing_info(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_smp_signing_info *pdu = frame->data;

	print_hex_field("Signature key", pdu->csrk, 16);
}

static void smp_security_request(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_smp_security_request *pdu = frame->data;

	print_smp_auth_req(pdu->auth_req);
}

static void smp_pairing_public_key(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_smp_public_key *pdu = frame->data;

	print_hex_field("X", pdu->x, 32);
	print_hex_field("Y", pdu->y, 32);
}

static void smp_pairing_dhkey_check(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_smp_dhkey_check *pdu = frame->data;

	print_hex_field("E", pdu->e, 16);
}

static void smp_pairing_keypress_notification(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_smp_keypress_notify *pdu = frame->data;
	const char *str;

	switch (pdu->type) {
	case 0x00:
		str = "Passkey entry started";
		break;
	case 0x01:
		str = "Passkey digit entered";
		break;
	case 0x02:
		str = "Passkey digit erased";
		break;
	case 0x03:
		str = "Passkey cleared";
		break;
	case 0x04:
		str = "Passkey entry completed";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Type: %s (0x%2.2x)", str, pdu->type);
}

struct smp_opcode_data {
	uint8_t opcode;
	const char *str;
	void (*func) (const struct l2cap_frame *frame);
	uint8_t size;
	bool fixed;
};

static const struct smp_opcode_data smp_opcode_table[] = {
	{ 0x01, "Pairing Request",
			smp_pairing_request, 6, true },
	{ 0x02, "Pairing Response",
			smp_pairing_response, 6, true },
	{ 0x03, "Pairing Confirm",
			smp_pairing_confirm, 16, true },
	{ 0x04, "Pairing Random",
			smp_pairing_random, 16, true },
	{ 0x05, "Pairing Failed",
			smp_pairing_failed, 1, true },
	{ 0x06, "Encryption Information",
			smp_encrypt_info, 16, true },
	{ 0x07, "Master Identification",
			smp_master_ident, 10, true },
	{ 0x08, "Identity Information",
			smp_ident_info, 16, true },
	{ 0x09, "Identity Address Information",
			smp_ident_addr_info, 7, true },
	{ 0x0a, "Signing Information",
			smp_signing_info, 16, true },
	{ 0x0b, "Security Request",
			smp_security_request, 1, true },
	{ 0x0c, "Pairing Public Key",
			smp_pairing_public_key, 64, true },
	{ 0x0d, "Pairing DHKey Check",
			smp_pairing_dhkey_check, 16, true },
	{ 0x0e, "Pairing Keypress Notification",
			smp_pairing_keypress_notification, 1, true },
	{ }
};

static void smp_packet(uint16_t index, bool in, uint16_t handle,
			uint16_t cid, const void *data, uint16_t size)
{
	struct l2cap_frame frame;
	uint8_t opcode = *((const uint8_t *) data);
	const struct smp_opcode_data *opcode_data = NULL;
	const char *opcode_color, *opcode_str;
	int i;

	if (size < 1) {
		print_text(COLOR_ERROR, "malformed attribute packet");
		packet_hexdump(data, size);
		return;
	}

	for (i = 0; smp_opcode_table[i].str; i++) {
		if (smp_opcode_table[i].opcode == opcode) {
			opcode_data = &smp_opcode_table[i];
			break;
		}
	}

	if (opcode_data) {
		if (opcode_data->func) {
			if (in)
				opcode_color = COLOR_MAGENTA;
			else
				opcode_color = COLOR_BLUE;
		} else
			opcode_color = COLOR_WHITE_BG;
		opcode_str = opcode_data->str;
	} else {
		opcode_color = COLOR_WHITE_BG;
		opcode_str = "Unknown";
	}

	print_indent(6, opcode_color, cid == 0x0006 ? "SMP: " : "BR/EDR SMP: ",
				opcode_str, COLOR_OFF, " (0x%2.2x) len %d",
				opcode, size - 1);

	if (!opcode_data || !opcode_data->func) {
		packet_hexdump(data + 1, size - 1);
		return;
	}

	if (opcode_data->fixed) {
		if (size - 1 != opcode_data->size) {
			print_text(COLOR_ERROR, "invalid size");
			packet_hexdump(data + 1, size - 1);
			return;
		}
	} else {
		if (size - 1 < opcode_data->size) {
			print_text(COLOR_ERROR, "too short packet");
			packet_hexdump(data + 1, size - 1);
			return;
		}
	}

	l2cap_frame_init(&frame, index, in, handle, 0, cid, data + 1, size - 1);
	opcode_data->func(&frame);
}

static void l2cap_frame(uint16_t index, bool in, uint16_t handle,
			uint16_t cid, const void *data, uint16_t size)
{
	struct l2cap_frame frame;
	uint32_t ctrl32 = 0;
	uint16_t ctrl16 = 0;
	uint8_t ext_ctrl;

	switch (cid) {
	case 0x0001:
		bredr_sig_packet(index, in, handle, cid, data, size);
		break;
	case 0x0002:
		connless_packet(index, in, handle, cid, data, size);
		break;
	case 0x0003:
		amp_packet(index, in, handle, cid, data, size);
		break;
	case 0x0004:
		att_packet(index, in, handle, cid, data, size);
		break;
	case 0x0005:
		le_sig_packet(index, in, handle, cid, data, size);
		break;
	case 0x0006:
	case 0x0007:
		smp_packet(index, in, handle, cid, data, size);
		break;
	default:
		l2cap_frame_init(&frame, index, in, handle, 0, cid, data, size);

		if (frame.mode > 0) {
			ext_ctrl = get_ext_ctrl(&frame);

			if (ext_ctrl) {
				if (!l2cap_frame_get_le32(&frame, &ctrl32))
					return;

				print_indent(6, COLOR_CYAN, "Channel:", "",
						COLOR_OFF, " %d len %d"
						" ext_ctrl 0x%8.8x"
						" [PSM %d mode %d] {chan %d}",
						cid, size, ctrl32, frame.psm,
						frame.mode, frame.chan);

				l2cap_ctrl_ext_parse(&frame, ctrl32);
			} else {
				if (!l2cap_frame_get_le16(&frame, &ctrl16))
					return;

				print_indent(6, COLOR_CYAN, "Channel:", "",
						COLOR_OFF, " %d len %d"
						" ctrl 0x%4.4x"
						" [PSM %d mode %d] {chan %d}",
						cid, size, ctrl16, frame.psm,
						frame.mode, frame.chan);

				l2cap_ctrl_parse(&frame, ctrl16);
			}

			printf("\n");
		} else {
			print_indent(6, COLOR_CYAN, "Channel:", "", COLOR_OFF,
					" %d len %d [PSM %d mode %d] {chan %d}",
						cid, size, frame.psm,
						frame.mode, frame.chan);
		}

		switch (frame.psm) {
		case 0x0001:
			sdp_packet(&frame);
			break;
		case 0x0003:
			rfcomm_packet(&frame);
			break;
		case 0x000f:
			bnep_packet(&frame);
			break;
		case 0x001f:
			att_packet(index, in, handle, cid, data, size);
			break;
		case 0x0017:
		case 0x001B:
			avctp_packet(&frame);
			break;
		case 0x0019:
			avdtp_packet(&frame);
			break;
		default:
			packet_hexdump(data, size);
			break;
		}
		break;
	}
}

void l2cap_packet(uint16_t index, bool in, uint16_t handle, uint8_t flags,
					const void *data, uint16_t size)
{
	const struct bt_l2cap_hdr *hdr = data;
	uint16_t len, cid;

	if (index > MAX_INDEX - 1) {
		print_text(COLOR_ERROR, "controller index too large");
		packet_hexdump(data, size);
		return;
	}

	switch (flags) {
	case 0x00:	/* start of a non-automatically-flushable PDU */
	case 0x02:	/* start of an automatically-flushable PDU */
		if (index_list[index][in].frag_len) {
			print_text(COLOR_ERROR, "unexpected start frame");
			packet_hexdump(data, size);
			clear_fragment_buffer(index, in);
			return;
		}

		if (size < sizeof(*hdr)) {
			print_text(COLOR_ERROR, "frame too short");
			packet_hexdump(data, size);
			return;
		}

		len = le16_to_cpu(hdr->len);
		cid = le16_to_cpu(hdr->cid);

		data += sizeof(*hdr);
		size -= sizeof(*hdr);

		if (len == size) {
			/* complete frame */
			l2cap_frame(index, in, handle, cid, data, len);
			return;
		}

		if (size > len) {
			print_text(COLOR_ERROR, "frame too long");
			packet_hexdump(data, size);
			return;
		}

		index_list[index][in].frag_buf = malloc(len);
		if (!index_list[index][in].frag_buf) {
			print_text(COLOR_ERROR, "failed buffer allocation");
			packet_hexdump(data, size);
			return;
		}

		memcpy(index_list[index][in].frag_buf, data, size);
		index_list[index][in].frag_pos = size;
		index_list[index][in].frag_len = len - size;
		index_list[index][in].frag_cid = cid;
		break;

	case 0x01:	/* continuing fragment */
		if (!index_list[index][in].frag_len) {
			print_text(COLOR_ERROR, "unexpected continuation");
			packet_hexdump(data, size);
			return;
		}

		if (size > index_list[index][in].frag_len) {
			print_text(COLOR_ERROR, "fragment too long");
			packet_hexdump(data, size);
			clear_fragment_buffer(index, in);
			return;
		}

		memcpy(index_list[index][in].frag_buf +
				index_list[index][in].frag_pos, data, size);
		index_list[index][in].frag_pos += size;
		index_list[index][in].frag_len -= size;

		if (!index_list[index][in].frag_len) {
			/* complete frame */
			l2cap_frame(index, in, handle,
					index_list[index][in].frag_cid,
					index_list[index][in].frag_buf,
					index_list[index][in].frag_pos);
			clear_fragment_buffer(index, in);
			return;
		}
		break;

	case 0x03:	/* complete automatically-flushable PDU */
		if (index_list[index][in].frag_len) {
			print_text(COLOR_ERROR, "unexpected complete frame");
			packet_hexdump(data, size);
			clear_fragment_buffer(index, in);
			return;
		}

		if (size < sizeof(*hdr)) {
			print_text(COLOR_ERROR, "frame too short");
			packet_hexdump(data, size);
			return;
		}

		len = le16_to_cpu(hdr->len);
		cid = le16_to_cpu(hdr->cid);

		data += sizeof(*hdr);
		size -= sizeof(*hdr);

		if (len != size) {
			print_text(COLOR_ERROR, "wrong frame size");
			packet_hexdump(data, size);
			return;
		}

		/* complete frame */
		l2cap_frame(index, in, handle, cid, data, len);
		break;

	default:
		print_text(COLOR_ERROR, "invalid packet flags (0x%2.2x)",
								flags);
		packet_hexdump(data, size);
		return;
	}
}
