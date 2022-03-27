/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2015  Andrzej Kaczmarek <andrzej.kaczmarek@codecoup.pl>
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

#include "lib/bluetooth.h"

#include "src/shared/util.h"
#include "bt.h"
#include "packet.h"
#include "display.h"
#include "l2cap.h"
#include "avdtp.h"
#include "a2dp.h"

/* Message Types */
#define AVDTP_MSG_TYPE_COMMAND		0x00
#define AVDTP_MSG_TYPE_GENERAL_REJECT	0x01
#define AVDTP_MSG_TYPE_RESPONSE_ACCEPT	0x02
#define AVDTP_MSG_TYPE_RESPONSE_REJECT	0x03

/* Signal Identifiers */
#define AVDTP_DISCOVER			0x01
#define AVDTP_GET_CAPABILITIES		0x02
#define AVDTP_SET_CONFIGURATION		0x03
#define AVDTP_GET_CONFIGURATION		0x04
#define AVDTP_RECONFIGURE		0x05
#define AVDTP_OPEN			0x06
#define AVDTP_START			0x07
#define AVDTP_CLOSE			0x08
#define AVDTP_SUSPEND			0x09
#define AVDTP_ABORT			0x0a
#define AVDTP_SECURITY_CONTROL		0x0b
#define AVDTP_GET_ALL_CAPABILITIES	0x0c
#define AVDTP_DELAYREPORT		0x0d

/* Service Categories */
#define AVDTP_MEDIA_TRANSPORT		0x01
#define AVDTP_REPORTING			0x02
#define AVDTP_RECOVERY			0x03
#define AVDTP_CONTENT_PROTECTION	0x04
#define AVDTP_HEADER_COMPRESSION	0x05
#define AVDTP_MULTIPLEXING		0x06
#define AVDTP_MEDIA_CODEC		0x07
#define AVDTP_DELAY_REPORTING		0x08

struct avdtp_frame {
	uint8_t hdr;
	uint8_t sig_id;
	struct l2cap_frame l2cap_frame;
};

static inline bool is_configuration_sig_id(uint8_t sig_id)
{
	return (sig_id == AVDTP_SET_CONFIGURATION) ||
			(sig_id == AVDTP_GET_CONFIGURATION) ||
			(sig_id == AVDTP_RECONFIGURE);
}

static const char *msgtype2str(uint8_t msgtype)
{
	switch (msgtype) {
	case 0:
		return "Command";
	case 1:
		return "General Reject";
	case 2:
		return "Response Accept";
	case 3:
		return "Response Reject";
	}

	return "";
}

static const char *sigid2str(uint8_t sigid)
{
	switch (sigid) {
	case AVDTP_DISCOVER:
		return "Discover";
	case AVDTP_GET_CAPABILITIES:
		return "Get Capabilities";
	case AVDTP_SET_CONFIGURATION:
		return "Set Configuration";
	case AVDTP_GET_CONFIGURATION:
		return "Get Configuration";
	case AVDTP_RECONFIGURE:
		return "Reconfigure";
	case AVDTP_OPEN:
		return "Open";
	case AVDTP_START:
		return "Start";
	case AVDTP_CLOSE:
		return "Close";
	case AVDTP_SUSPEND:
		return "Suspend";
	case AVDTP_ABORT:
		return "Abort";
	case AVDTP_SECURITY_CONTROL:
		return "Security Control";
	case AVDTP_GET_ALL_CAPABILITIES:
		return "Get All Capabilities";
	case AVDTP_DELAYREPORT:
		return "Delay Report";
	default:
		return "Reserved";
	}
}

static const char *error2str(uint8_t error)
{
	switch (error) {
	case 0x01:
		return "BAD_HEADER_FORMAT";
	case 0x11:
		return "BAD_LENGTH";
	case 0x12:
		return "BAD_ACP_SEID";
	case 0x13:
		return "SEP_IN_USE";
	case 0x14:
		return "SEP_NOT_IN_USER";
	case 0x17:
		return "BAD_SERV_CATEGORY";
	case 0x18:
		return "BAD_PAYLOAD_FORMAT";
	case 0x19:
		return "NOT_SUPPORTED_COMMAND";
	case 0x1a:
		return "INVALID_CAPABILITIES";
	case 0x22:
		return "BAD_RECOVERY_TYPE";
	case 0x23:
		return "BAD_MEDIA_TRANSPORT_FORMAT";
	case 0x25:
		return "BAD_RECOVERY_FORMAT";
	case 0x26:
		return "BAD_ROHC_FORMAT";
	case 0x27:
		return "BAD_CP_FORMAT";
	case 0x28:
		return "BAD_MULTIPLEXING_FORMAT";
	case 0x29:
		return "UNSUPPORTED_CONFIGURATION";
	case 0x31:
		return "BAD_STATE";
	default:
		return "Unknown";
	}
}

static const char *mediatype2str(uint8_t media_type)
{
	switch (media_type) {
	case 0x00:
		return "Audio";
	case 0x01:
		return "Video";
	case 0x02:
		return "Multimedia";
	default:
		return "Reserved";
	}
}

static const char *mediacodec2str(uint8_t codec)
{
	switch (codec) {
	case 0x00:
		return "SBC";
	case 0x01:
		return "MPEG-1,2 Audio";
	case 0x02:
		return "MPEG-2,4 AAC";
	case 0x04:
		return "ATRAC Family";
	case 0xff:
		return "Non-A2DP";
	default:
		return "Reserved";
	}
}

static const char *cptype2str(uint8_t cp)
{
	switch (cp) {
	case 0x0001:
		return "DTCP";
	case 0x0002:
		return "SCMS-T";
	default:
		return "Reserved";
	}
}

static const char *servicecat2str(uint8_t service_cat)
{
	switch (service_cat) {
	case AVDTP_MEDIA_TRANSPORT:
		return "Media Transport";
	case AVDTP_REPORTING:
		return "Reporting";
	case AVDTP_RECOVERY:
		return "Recovery";
	case AVDTP_CONTENT_PROTECTION:
		return "Content Protection";
	case AVDTP_HEADER_COMPRESSION:
		return "Header Compression";
	case AVDTP_MULTIPLEXING:
		return "Multiplexing";
	case AVDTP_MEDIA_CODEC:
		return "Media Codec";
	case AVDTP_DELAY_REPORTING:
		return "Delay Reporting";
	default:
		return "Reserved";
	}
}

static bool avdtp_reject_common(struct avdtp_frame *avdtp_frame)
{
	struct l2cap_frame *frame = &avdtp_frame->l2cap_frame;
	uint8_t error;

	if (!l2cap_frame_get_u8(frame, &error))
		return false;

	print_field("Error code: %s (0x%02x)", error2str(error), error);

	return true;
}

static bool service_content_protection(struct avdtp_frame *avdtp_frame,
								uint8_t losc)
{
	struct l2cap_frame *frame = &avdtp_frame->l2cap_frame;
	uint16_t type = 0;

	if (losc < 2)
		return false;

	if (!l2cap_frame_get_le16(frame, &type))
		return false;

	losc -= 2;

	print_field("%*cContent Protection Type: %s (0x%04x)", 2, ' ',
							cptype2str(type), type);

	/* TODO: decode protection specific information */
	packet_hexdump(frame->data, losc);

	l2cap_frame_pull(frame, frame, losc);

	return true;
}

static bool service_media_codec(struct avdtp_frame *avdtp_frame, uint8_t losc)
{
	struct l2cap_frame *frame = &avdtp_frame->l2cap_frame;
	uint8_t type = 0;
	uint8_t codec = 0;

	if (losc < 2)
		return false;

	l2cap_frame_get_u8(frame, &type);
	l2cap_frame_get_u8(frame, &codec);

	losc -= 2;

	print_field("%*cMedia Type: %s (0x%02x)", 2, ' ',
					mediatype2str(type >> 4), type >> 4);

	print_field("%*cMedia Codec: %s (0x%02x)", 2, ' ',
					mediacodec2str(codec), codec);

	if (is_configuration_sig_id(avdtp_frame->sig_id))
		return a2dp_codec_cfg(codec, losc, frame);
	else
		return a2dp_codec_cap(codec, losc, frame);
}

static bool decode_capabilities(struct avdtp_frame *avdtp_frame)
{
	struct l2cap_frame *frame = &avdtp_frame->l2cap_frame;
	uint8_t service_cat;
	uint8_t losc;

	while (l2cap_frame_get_u8(frame, &service_cat)) {
		print_field("Service Category: %s (0x%02x)",
				servicecat2str(service_cat), service_cat);

		if (!l2cap_frame_get_u8(frame, &losc))
			return false;

		if (frame->size < losc)
			return false;

		switch (service_cat) {
		case AVDTP_CONTENT_PROTECTION:
			if (!service_content_protection(avdtp_frame, losc))
				return false;
			break;
		case AVDTP_MEDIA_CODEC:
			if (!service_media_codec(avdtp_frame, losc))
				return false;
			break;
		case AVDTP_MEDIA_TRANSPORT:
		case AVDTP_REPORTING:
		case AVDTP_RECOVERY:
		case AVDTP_HEADER_COMPRESSION:
		case AVDTP_MULTIPLEXING:
		case AVDTP_DELAY_REPORTING:
		default:
			packet_hexdump(frame->data, losc);
			l2cap_frame_pull(frame, frame, losc);
		}

	}

	return true;
}

static bool avdtp_discover(struct avdtp_frame *avdtp_frame)
{
	struct l2cap_frame *frame = &avdtp_frame->l2cap_frame;
	uint8_t type = avdtp_frame->hdr & 0x03;
	uint8_t seid;
	uint8_t info;

	switch (type) {
	case AVDTP_MSG_TYPE_COMMAND:
		return true;
	case AVDTP_MSG_TYPE_RESPONSE_ACCEPT:
		while (l2cap_frame_get_u8(frame, &seid)) {
			print_field("ACP SEID: %d", seid >> 2);

			if (!l2cap_frame_get_u8(frame, &info))
				return false;

			print_field("%*cMedia Type: %s (0x%02x)", 2, ' ',
					mediatype2str(info >> 4), info >> 4);
			print_field("%*cSEP Type: %s (0x%02x)", 2, ' ',
						info & 0x04 ? "SNK" : "SRC",
						(info >> 3) & 0x01);
			print_field("%*cIn use: %s", 2, ' ',
						seid & 0x02 ? "Yes" : "No");
		}
		return true;
	case AVDTP_MSG_TYPE_RESPONSE_REJECT:
		return avdtp_reject_common(avdtp_frame);
	}

	return false;
}

static bool avdtp_get_capabilities(struct avdtp_frame *avdtp_frame)
{
	struct l2cap_frame *frame = &avdtp_frame->l2cap_frame;
	uint8_t type = avdtp_frame->hdr & 0x03;
	uint8_t seid;

	switch (type) {
	case AVDTP_MSG_TYPE_COMMAND:
		if (!l2cap_frame_get_u8(frame, &seid))
			return false;

		print_field("ACP SEID: %d", seid >> 2);

		return true;
	case AVDTP_MSG_TYPE_RESPONSE_ACCEPT:
		return decode_capabilities(avdtp_frame);
	case AVDTP_MSG_TYPE_RESPONSE_REJECT:
		return avdtp_reject_common(avdtp_frame);
	}

	return false;
}

static bool avdtp_set_configuration(struct avdtp_frame *avdtp_frame)
{
	struct l2cap_frame *frame = &avdtp_frame->l2cap_frame;
	uint8_t type = avdtp_frame->hdr & 0x03;
	uint8_t acp_seid, int_seid;
	uint8_t service_cat;

	switch (type) {
	case AVDTP_MSG_TYPE_COMMAND:
		if (!l2cap_frame_get_u8(frame, &acp_seid))
			return false;

		print_field("ACP SEID: %d", acp_seid >> 2);

		if (!l2cap_frame_get_u8(frame, &int_seid))
			return false;

		print_field("INT SEID: %d", int_seid >> 2);

		return decode_capabilities(avdtp_frame);
	case AVDTP_MSG_TYPE_RESPONSE_ACCEPT:
		return true;
	case AVDTP_MSG_TYPE_RESPONSE_REJECT:
		if (!l2cap_frame_get_u8(frame, &service_cat))
			return false;

		print_field("Service Category: %s (0x%02x)",
				servicecat2str(service_cat), service_cat);

		return avdtp_reject_common(avdtp_frame);
	}

	return false;
}

static bool avdtp_get_configuration(struct avdtp_frame *avdtp_frame)
{
	struct l2cap_frame *frame = &avdtp_frame->l2cap_frame;
	uint8_t type = avdtp_frame->hdr & 0x03;
	uint8_t seid;

	switch (type) {
	case AVDTP_MSG_TYPE_COMMAND:
		if (!l2cap_frame_get_u8(frame, &seid))
			return false;

		print_field("ACP SEID: %d", seid >> 2);

		return true;
	case AVDTP_MSG_TYPE_RESPONSE_ACCEPT:
		return decode_capabilities(avdtp_frame);
	case AVDTP_MSG_TYPE_RESPONSE_REJECT:
		return avdtp_reject_common(avdtp_frame);
	}

	return false;
}

static bool avdtp_reconfigure(struct avdtp_frame *avdtp_frame)
{
	struct l2cap_frame *frame = &avdtp_frame->l2cap_frame;
	uint8_t type = avdtp_frame->hdr & 0x03;
	uint8_t seid;
	uint8_t service_cat;

	switch (type) {
	case AVDTP_MSG_TYPE_COMMAND:
		if (!l2cap_frame_get_u8(frame, &seid))
			return false;

		print_field("ACP SEID: %d", seid >> 2);

		return decode_capabilities(avdtp_frame);
	case AVDTP_MSG_TYPE_RESPONSE_ACCEPT:
		return true;
	case AVDTP_MSG_TYPE_RESPONSE_REJECT:
		if (!l2cap_frame_get_u8(frame, &service_cat))
			return false;

		print_field("Service Category: %s (0x%02x)",
				servicecat2str(service_cat), service_cat);

		return avdtp_reject_common(avdtp_frame);
	}

	return false;
}

static bool avdtp_open(struct avdtp_frame *avdtp_frame)
{
	struct l2cap_frame *frame = &avdtp_frame->l2cap_frame;
	uint8_t type = avdtp_frame->hdr & 0x03;
	uint8_t seid;

	switch (type) {
	case AVDTP_MSG_TYPE_COMMAND:
		if (!l2cap_frame_get_u8(frame, &seid))
			return false;

		print_field("ACP SEID: %d", seid >> 2);

		return true;
	case AVDTP_MSG_TYPE_RESPONSE_ACCEPT:
		return true;
	case AVDTP_MSG_TYPE_RESPONSE_REJECT:
		return avdtp_reject_common(avdtp_frame);
	}

	return false;
}

static bool avdtp_start(struct avdtp_frame *avdtp_frame)
{
	struct l2cap_frame *frame = &avdtp_frame->l2cap_frame;
	uint8_t type = avdtp_frame->hdr & 0x03;
	uint8_t seid;

	switch (type) {
	case AVDTP_MSG_TYPE_COMMAND:
		if (!l2cap_frame_get_u8(frame, &seid))
			return false;

		print_field("ACP SEID: %d", seid >> 2);

		while (l2cap_frame_get_u8(frame, &seid))
			print_field("ACP SEID: %d", seid >> 2);

		return true;
	case AVDTP_MSG_TYPE_RESPONSE_ACCEPT:
		return true;
	case AVDTP_MSG_TYPE_RESPONSE_REJECT:
		if (!l2cap_frame_get_u8(frame, &seid))
			return false;

		print_field("ACP SEID: %d", seid >> 2);

		return avdtp_reject_common(avdtp_frame);
	}

	return false;
}

static bool avdtp_close(struct avdtp_frame *avdtp_frame)
{
	struct l2cap_frame *frame = &avdtp_frame->l2cap_frame;
	uint8_t type = avdtp_frame->hdr & 0x03;
	uint8_t seid;

	switch (type) {
	case AVDTP_MSG_TYPE_COMMAND:
		if (!l2cap_frame_get_u8(frame, &seid))
			return false;

		print_field("ACP SEID: %d", seid >> 2);

		return true;
	case AVDTP_MSG_TYPE_RESPONSE_ACCEPT:
		return true;
	case AVDTP_MSG_TYPE_RESPONSE_REJECT:
		return avdtp_reject_common(avdtp_frame);
	}

	return false;
}

static bool avdtp_suspend(struct avdtp_frame *avdtp_frame)
{
	struct l2cap_frame *frame = &avdtp_frame->l2cap_frame;
	uint8_t type = avdtp_frame->hdr & 0x03;
	uint8_t seid;

	switch (type) {
	case AVDTP_MSG_TYPE_COMMAND:
		if (!l2cap_frame_get_u8(frame, &seid))
			return false;

		print_field("ACP SEID: %d", seid >> 2);

		while (l2cap_frame_get_u8(frame, &seid))
			print_field("ACP SEID: %d", seid >> 2);

		return true;
	case AVDTP_MSG_TYPE_RESPONSE_ACCEPT:
		return true;
	case AVDTP_MSG_TYPE_RESPONSE_REJECT:
		if (!l2cap_frame_get_u8(frame, &seid))
			return false;

		print_field("ACP SEID: %d", seid >> 2);

		return avdtp_reject_common(avdtp_frame);
	}

	return false;
}

static bool avdtp_abort(struct avdtp_frame *avdtp_frame)
{
	struct l2cap_frame *frame = &avdtp_frame->l2cap_frame;
	uint8_t type = avdtp_frame->hdr & 0x03;
	uint8_t seid;

	switch (type) {
	case AVDTP_MSG_TYPE_COMMAND:
		if (!l2cap_frame_get_u8(frame, &seid))
			return false;

		print_field("ACP SEID: %d", seid >> 2);

		return true;
	case AVDTP_MSG_TYPE_RESPONSE_ACCEPT:
		return true;
	}

	return false;
}

static bool avdtp_security_control(struct avdtp_frame *avdtp_frame)
{
	struct l2cap_frame *frame = &avdtp_frame->l2cap_frame;
	uint8_t type = avdtp_frame->hdr & 0x03;
	uint8_t seid;

	switch (type) {
	case AVDTP_MSG_TYPE_COMMAND:
		if (!l2cap_frame_get_u8(frame, &seid))
			return false;

		print_field("ACP SEID: %d", seid >> 2);

		/* TODO: decode more information */
		packet_hexdump(frame->data, frame->size);
		return true;
	case AVDTP_MSG_TYPE_RESPONSE_ACCEPT:
		/* TODO: decode more information */
		packet_hexdump(frame->data, frame->size);
		return true;
	case AVDTP_MSG_TYPE_RESPONSE_REJECT:
		return avdtp_reject_common(avdtp_frame);
	}

	return false;
}

static bool avdtp_delayreport(struct avdtp_frame *avdtp_frame)
{
	struct l2cap_frame *frame = &avdtp_frame->l2cap_frame;
	uint8_t type = avdtp_frame->hdr & 0x03;
	uint8_t seid;
	uint16_t delay;

	switch (type) {
	case AVDTP_MSG_TYPE_COMMAND:
		if (!l2cap_frame_get_u8(frame, &seid))
			return false;

		print_field("ACP SEID: %d", seid >> 2);

		if (!l2cap_frame_get_be16(frame, &delay))
			return false;

		print_field("Delay: %d.%dms", delay / 10, delay % 10);

		return true;
	case AVDTP_MSG_TYPE_RESPONSE_ACCEPT:
		return true;
	case AVDTP_MSG_TYPE_RESPONSE_REJECT:
		return avdtp_reject_common(avdtp_frame);
	}

	return false;
}

static bool avdtp_signalling_packet(struct avdtp_frame *avdtp_frame)
{
	struct l2cap_frame *frame = &avdtp_frame->l2cap_frame;
	const char *pdu_color;
	uint8_t hdr;
	uint8_t sig_id;
	uint8_t nosp = 0;

	if (frame->in)
		pdu_color = COLOR_MAGENTA;
	else
		pdu_color = COLOR_BLUE;

	if (!l2cap_frame_get_u8(frame, &hdr))
		return false;

	avdtp_frame->hdr = hdr;

	/* Continue Packet || End Packet */
	if (((hdr & 0x0c) == 0x08) || ((hdr & 0x0c) == 0x0c)) {
		/* TODO: handle fragmentation */
		packet_hexdump(frame->data, frame->size);
		return true;
	}

	/* Start Packet */
	if ((hdr & 0x0c) == 0x04) {
		if (!l2cap_frame_get_u8(frame, &nosp))
			return false;
	}

	if (!l2cap_frame_get_u8(frame, &sig_id))
		return false;

	sig_id &= 0x3f;

	avdtp_frame->sig_id = sig_id;

	print_indent(6, pdu_color, "AVDTP: ", sigid2str(sig_id), COLOR_OFF,
			" (0x%02x) %s (0x%02x) type 0x%02x label %d nosp %d",
			sig_id, msgtype2str(hdr & 0x03), hdr & 0x03,
			hdr & 0x0c, hdr >> 4, nosp);

	/* Start Packet */
	if ((hdr & 0x0c) == 0x04) {
		/* TODO: handle fragmentation */
		packet_hexdump(frame->data, frame->size);
		return true;
	}

	/* General Reject */
	if ((hdr & 0x03) == 0x03)
		return true;

	switch (sig_id) {
	case AVDTP_DISCOVER:
		return avdtp_discover(avdtp_frame);
	case AVDTP_GET_CAPABILITIES:
	case AVDTP_GET_ALL_CAPABILITIES:
		return avdtp_get_capabilities(avdtp_frame);
	case AVDTP_SET_CONFIGURATION:
		return avdtp_set_configuration(avdtp_frame);
	case AVDTP_GET_CONFIGURATION:
		return avdtp_get_configuration(avdtp_frame);
	case AVDTP_RECONFIGURE:
		return avdtp_reconfigure(avdtp_frame);
	case AVDTP_OPEN:
		return avdtp_open(avdtp_frame);
	case AVDTP_START:
		return avdtp_start(avdtp_frame);
	case AVDTP_CLOSE:
		return avdtp_close(avdtp_frame);
	case AVDTP_SUSPEND:
		return avdtp_suspend(avdtp_frame);
	case AVDTP_ABORT:
		return avdtp_abort(avdtp_frame);
	case AVDTP_SECURITY_CONTROL:
		return avdtp_security_control(avdtp_frame);
	case AVDTP_DELAYREPORT:
		return avdtp_delayreport(avdtp_frame);
	}

	packet_hexdump(frame->data, frame->size);

	return true;
}

void avdtp_packet(const struct l2cap_frame *frame)
{
	struct avdtp_frame avdtp_frame;
	bool ret;

	l2cap_frame_pull(&avdtp_frame.l2cap_frame, frame, 0);

	switch (frame->seq_num) {
	case 1:
		ret = avdtp_signalling_packet(&avdtp_frame);
		break;
	default:
		packet_hexdump(frame->data, frame->size);
		return;
	}

	if (!ret) {
		print_text(COLOR_ERROR, "PDU malformed");
		packet_hexdump(frame->data, frame->size);
	}
}
