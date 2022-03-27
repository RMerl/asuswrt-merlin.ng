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
#include "a2dp.h"

#define BASE_INDENT	4

/* Codec Types */
#define A2DP_CODEC_SBC		0x00
#define A2DP_CODEC_MPEG12	0x01
#define A2DP_CODEC_MPEG24	0x02
#define A2DP_CODEC_ATRAC	0x04
#define A2DP_CODEC_VENDOR	0xff

/* Vendor Specific A2DP Codecs */
#define APTX_VENDOR_ID		0x0000004f
#define APTX_CODEC_ID		0x0001
#define LDAC_VENDOR_ID		0x0000012d
#define LDAC_CODEC_ID		0x00aa

struct bit_desc {
	uint8_t bit_num;
	const char *str;
};

static const struct bit_desc sbc_frequency_table[] = {
	{  7, "16000" },
	{  6, "32000" },
	{  5, "44100" },
	{  4, "48000" },
	{ }
};

static const struct bit_desc sbc_channel_mode_table[] = {
	{  3, "Mono" },
	{  2, "Dual Channel" },
	{  1, "Stereo" },
	{  0, "Joint Stereo" },
	{ }
};

static const struct bit_desc sbc_blocklen_table[] = {
	{  7, "4" },
	{  6, "8" },
	{  5, "12" },
	{  4, "16" },
	{ }
};

static const struct bit_desc sbc_subbands_table[] = {
	{  3, "4" },
	{  2, "8" },
	{ }
};

static const struct bit_desc sbc_allocation_table[] = {
	{  1, "SNR" },
	{  0, "Loudness" },
	{ }
};

static const struct bit_desc mpeg12_layer_table[] = {
	{  7, "Layer I (mp1)" },
	{  6, "Layer II (mp2)" },
	{  5, "Layer III (mp3)" },
	{ }
};

static const struct bit_desc mpeg12_channel_mode_table[] = {
	{  3, "Mono" },
	{  2, "Dual Channel" },
	{  1, "Stereo" },
	{  0, "Joint Stereo" },
	{ }
};

static const struct bit_desc mpeg12_frequency_table[] = {
	{  5, "16000" },
	{  4, "22050" },
	{  3, "24000" },
	{  2, "32000" },
	{  1, "44100" },
	{  0, "48000" },
	{ }
};

static const struct bit_desc mpeg12_bitrate_table[] = {
	{ 14, "1110" },
	{ 13, "1101" },
	{ 12, "1100" },
	{ 11, "1011" },
	{ 10, "1010" },
	{  9, "1001" },
	{  8, "1000" },
	{  7, "0111" },
	{  6, "0110" },
	{  5, "0101" },
	{  4, "0100" },
	{  3, "0011" },
	{  2, "0010" },
	{  1, "0001" },
	{  0, "0000" },
	{ }
};

static const struct bit_desc aac_object_type_table[] = {
	{  7, "MPEG-2 AAC LC" },
	{  6, "MPEG-4 AAC LC" },
	{  5, "MPEG-4 AAC LTP" },
	{  4, "MPEG-4 AAC scalable" },
	{  3, "RFA (b3)" },
	{  2, "RFA (b2)" },
	{  1, "RFA (b1)" },
	{  0, "RFA (b0)" },
	{ }
};

static const struct bit_desc aac_frequency_table[] = {
	{ 15, "8000" },
	{ 14, "11025" },
	{ 13, "12000" },
	{ 12, "16000" },
	{ 11, "22050" },
	{ 10, "24000" },
	{  9, "32000" },
	{  8, "44100" },
	{  7, "48000" },
	{  6, "64000" },
	{  5, "88200" },
	{  4, "96000" },
	{ }
};

static const struct bit_desc aac_channels_table[] = {
	{  3, "1" },
	{  2, "2" },
	{ }
};

static const struct bit_desc aptx_frequency_table[] = {
	{  7, "16000" },
	{  6, "32000" },
	{  5, "44100" },
	{  4, "48000" },
	{ }
};

static const struct bit_desc aptx_channel_mode_table[] = {
	{  0, "Mono" },
	{  1, "Stereo" },
	{ }
};

static void print_value_bits(uint8_t indent, uint32_t value,
						const struct bit_desc *table)
{
	int i;

	for (i = 0; table[i].str; i++) {
		if (value & (1 << table[i].bit_num))
			print_field("%*c%s", indent + 2, ' ', table[i].str);
	}
}

static const char *find_value_bit(uint32_t value,
						const struct bit_desc *table)
{
	int i;

	for (i = 0; table[i].str; i++) {
		if (value & (1 << table[i].bit_num))
			return table[i].str;
	}

	return "Unknown";
}

static const char *vndcodec2str(uint32_t vendor_id, uint16_t codec_id)
{
	if (vendor_id == APTX_VENDOR_ID && codec_id == APTX_CODEC_ID)
		return "aptX";
	else if (vendor_id == LDAC_VENDOR_ID && codec_id == LDAC_CODEC_ID)
		return "LDAC";

	return "Unknown";
}

static bool codec_sbc_cap(uint8_t losc, struct l2cap_frame *frame)
{
	uint8_t cap = 0;

	if (losc != 4)
		return false;

	l2cap_frame_get_u8(frame, &cap);

	print_field("%*cFrequency: 0x%02x", BASE_INDENT, ' ', cap & 0xf0);
	print_value_bits(BASE_INDENT, cap & 0xf0, sbc_frequency_table);

	print_field("%*cChannel Mode: 0x%02x", BASE_INDENT, ' ', cap & 0x0f);
	print_value_bits(BASE_INDENT, cap & 0x0f, sbc_channel_mode_table);

	l2cap_frame_get_u8(frame, &cap);

	print_field("%*cBlock Length: 0x%02x", BASE_INDENT, ' ', cap & 0xf0);
	print_value_bits(BASE_INDENT, cap & 0xf0, sbc_blocklen_table);

	print_field("%*cSubbands: 0x%02x", BASE_INDENT, ' ', cap & 0x0c);
	print_value_bits(BASE_INDENT, cap & 0x0c, sbc_subbands_table);

	print_field("%*cAllocation Method: 0x%02x", BASE_INDENT, ' ',
								cap & 0x03);
	print_value_bits(BASE_INDENT, cap & 0x03, sbc_allocation_table);

	l2cap_frame_get_u8(frame, &cap);

	print_field("%*cMinimum Bitpool: %d", BASE_INDENT, ' ', cap);

	l2cap_frame_get_u8(frame, &cap);

	print_field("%*cMaximum Bitpool: %d", BASE_INDENT, ' ', cap);

	return true;
}

static bool codec_sbc_cfg(uint8_t losc, struct l2cap_frame *frame)
{
	uint8_t cap = 0;

	if (losc != 4)
		return false;

	l2cap_frame_get_u8(frame, &cap);

	print_field("%*cFrequency: %s (0x%02x)", BASE_INDENT, ' ',
			find_value_bit(cap & 0xf0, sbc_frequency_table),
			cap & 0xf0);

	print_field("%*cChannel Mode: %s (0x%02x)", BASE_INDENT, ' ',
			find_value_bit(cap & 0x0f, sbc_channel_mode_table),
			cap & 0x0f);

	l2cap_frame_get_u8(frame, &cap);

	print_field("%*cBlock Length: %s (0x%02x)", BASE_INDENT, ' ',
			find_value_bit(cap & 0xf0, sbc_blocklen_table),
			cap & 0xf0);

	print_field("%*cSubbands: %s (0x%02x)", BASE_INDENT, ' ',
			find_value_bit(cap & 0x0c, sbc_subbands_table),
			cap & 0x0c);

	print_field("%*cAllocation Method: %s (0x%02x)", BASE_INDENT, ' ',
			find_value_bit(cap & 0x03, sbc_allocation_table),
			cap & 0x03);

	l2cap_frame_get_u8(frame, &cap);

	print_field("%*cMinimum Bitpool: %d", BASE_INDENT, ' ', cap);

	l2cap_frame_get_u8(frame, &cap);

	print_field("%*cMaximum Bitpool: %d", BASE_INDENT, ' ', cap);

	return true;
}

static bool codec_mpeg12_cap(uint8_t losc, struct l2cap_frame *frame)
{
	uint16_t cap = 0;
	uint8_t layer;
	uint8_t chan;
	uint8_t freq;
	uint16_t bitrate;
	bool crc, mpf, vbr;

	if (losc != 4)
		return false;

	l2cap_frame_get_be16(frame, &cap);

	layer = (cap >> 8) & 0xe0;
	crc = cap & 0x1000;
	chan = (cap >> 8) & 0x0f;
	mpf = cap & 0x0040;
	freq = cap & 0x003f;

	l2cap_frame_get_be16(frame, &cap);

	vbr = cap & 0x8000;
	bitrate = cap & 0x7fff;

	print_field("%*cLayer: 0x%02x", BASE_INDENT, ' ', layer);
	print_value_bits(BASE_INDENT, layer, mpeg12_layer_table);

	print_field("%*cCRC: %s", BASE_INDENT, ' ', crc ? "Yes" : "No");

	print_field("%*cChannel Mode: 0x%02x", BASE_INDENT, ' ', chan);
	print_value_bits(BASE_INDENT, chan, mpeg12_channel_mode_table);

	print_field("%*cMedia Payload Format: %s", BASE_INDENT, ' ',
					mpf ? "RFC-2250 RFC-3119" : "RFC-2250");

	print_field("%*cFrequency: 0x%02x", BASE_INDENT, ' ', freq);
	print_value_bits(BASE_INDENT, freq, mpeg12_frequency_table);

	if (!vbr) {
		print_field("%*cBitrate Index: 0x%04x", BASE_INDENT, ' ',
								bitrate);
		print_value_bits(BASE_INDENT, freq, mpeg12_bitrate_table);
	}

	print_field("%*cVBR: %s", BASE_INDENT, ' ', vbr ? "Yes" : "No");

	return true;
}

static bool codec_mpeg12_cfg(uint8_t losc, struct l2cap_frame *frame)
{
	uint16_t cap = 0;
	uint8_t layer;
	uint8_t chan;
	uint8_t freq;
	uint16_t bitrate;
	bool crc, mpf, vbr;

	if (losc != 4)
		return false;

	l2cap_frame_get_be16(frame, &cap);

	layer = (cap >> 8) & 0xe0;
	crc = cap & 0x1000;
	chan = (cap >> 8) & 0x0f;
	mpf = cap & 0x0040;
	freq = cap & 0x003f;

	l2cap_frame_get_be16(frame, &cap);

	vbr = cap & 0x8000;
	bitrate = cap & 0x7fff;

	print_field("%*cLayer: %s (0x%02x)", BASE_INDENT, ' ',
				find_value_bit(layer, mpeg12_layer_table),
				layer);

	print_field("%*cCRC: %s", BASE_INDENT, ' ', crc ? "Yes" : "No");

	print_field("%*cChannel Mode: %s (0x%02x)", BASE_INDENT, ' ',
				find_value_bit(chan, mpeg12_channel_mode_table),
				chan);

	print_field("%*cMedia Payload Format: %s", BASE_INDENT, ' ',
					mpf ? "RFC-2250 RFC-3119" : "RFC-2250");

	print_field("%*cFrequency: %s (0x%02x)", BASE_INDENT, ' ',
				find_value_bit(freq, mpeg12_frequency_table),
				freq);

	if (!vbr)
		print_field("%*cBitrate Index: %s (0x%04x)", BASE_INDENT, ' ',
				find_value_bit(freq, mpeg12_bitrate_table),
				bitrate);

	print_field("%*cVBR: %s", BASE_INDENT, ' ', vbr ? "Yes" : "No");

	return true;
}

static bool codec_aac_cap(uint8_t losc, struct l2cap_frame *frame)
{
	uint16_t cap = 0;
	uint8_t type;
	uint16_t freq;
	uint8_t chan;
	uint32_t bitrate;
	bool vbr;

	if (losc != 6)
		return false;

	l2cap_frame_get_be16(frame, &cap);

	type = cap >> 8;
	freq = cap << 8;

	l2cap_frame_get_be16(frame, &cap);

	freq |= (cap >> 8) & 0xf0;
	chan = (cap >> 8) & 0x0c;
	bitrate = (cap << 16) & 0x7f0000;
	vbr = cap & 0x0080;

	l2cap_frame_get_be16(frame, &cap);

	bitrate |= cap;

	print_field("%*cObject Type: 0x%02x", BASE_INDENT, ' ', type);
	print_value_bits(BASE_INDENT, type, aac_object_type_table);

	print_field("%*cFrequency: 0x%02x", BASE_INDENT, ' ', freq);
	print_value_bits(BASE_INDENT, freq, aac_frequency_table);

	print_field("%*cChannels: 0x%02x", BASE_INDENT, ' ', chan);
	print_value_bits(BASE_INDENT, chan, aac_channels_table);

	print_field("%*cBitrate: %ubps", BASE_INDENT, ' ', bitrate);
	print_field("%*cVBR: %s", BASE_INDENT, ' ', vbr ? "Yes" : "No");

	return true;
}

static bool codec_aac_cfg(uint8_t losc, struct l2cap_frame *frame)
{
	uint16_t cap = 0;
	uint8_t type;
	uint16_t freq;
	uint8_t chan;
	uint32_t bitrate;
	bool vbr;

	if (losc != 6)
		return false;

	l2cap_frame_get_be16(frame, &cap);

	type = cap >> 8;
	freq = cap << 8;

	l2cap_frame_get_be16(frame, &cap);

	freq |= (cap >> 8) & 0xf0;
	chan = (cap >> 8) & 0x0c;
	bitrate = (cap << 16) & 0x7f0000;
	vbr = cap & 0x0080;

	l2cap_frame_get_be16(frame, &cap);

	bitrate |= cap;

	print_field("%*cObject Type: %s (0x%02x)", BASE_INDENT, ' ',
			find_value_bit(type, aac_object_type_table), type);

	print_field("%*cFrequency: %s (0x%02x)", BASE_INDENT, ' ',
			find_value_bit(freq, aac_frequency_table), freq);

	print_field("%*cChannels: %s (0x%02x)", BASE_INDENT, ' ',
			find_value_bit(chan, aac_channels_table), chan);

	print_field("%*cBitrate: %ubps", BASE_INDENT, ' ', bitrate);
	print_field("%*cVBR: %s", BASE_INDENT, ' ', vbr ? "Yes" : "No");

	return true;
}

static bool codec_vendor_aptx_cap(uint8_t losc, struct l2cap_frame *frame)
{
	uint8_t cap = 0;

	if (losc != 1)
		return false;

	l2cap_frame_get_u8(frame, &cap);

	print_field("%*cFrequency: 0x%02x", BASE_INDENT + 2, ' ', cap & 0xf0);
	print_value_bits(BASE_INDENT + 2, cap & 0xf0, aptx_frequency_table);

	print_field("%*cChannel Mode: 0x%02x", BASE_INDENT + 2, ' ',
								cap & 0x0f);
	print_value_bits(BASE_INDENT + 2, cap & 0x0f, aptx_channel_mode_table);

	return true;
}

static bool codec_vendor_ldac(uint8_t losc, struct l2cap_frame *frame)
{
	uint16_t cap = 0;

	if (losc != 2)
		return false;

	l2cap_frame_get_le16(frame, &cap);

	print_field("%*cUnknown: 0x%04x", BASE_INDENT + 2, ' ', cap);

	return true;
}

static bool codec_vendor_cap(uint8_t losc, struct l2cap_frame *frame)
{
	uint32_t vendor_id = 0;
	uint16_t codec_id = 0;

	if (losc < 6)
		return false;

	l2cap_frame_get_le32(frame, &vendor_id);
	l2cap_frame_get_le16(frame, &codec_id);

	losc -= 6;

	print_field("%*cVendor ID: %s (0x%08x)", BASE_INDENT, ' ',
					bt_compidtostr(vendor_id),  vendor_id);

	print_field("%*cVendor Specific Codec ID: %s (0x%04x)", BASE_INDENT,
			' ', vndcodec2str(vendor_id, codec_id), codec_id);

	if (vendor_id == APTX_VENDOR_ID && codec_id == APTX_CODEC_ID)
		return codec_vendor_aptx_cap(losc, frame);
	else if (vendor_id == LDAC_VENDOR_ID && codec_id == LDAC_CODEC_ID)
		return codec_vendor_ldac(losc, frame);

	packet_hexdump(frame->data, losc);
	l2cap_frame_pull(frame, frame, losc);

	return true;
}

static bool codec_vendor_aptx_cfg(uint8_t losc, struct l2cap_frame *frame)
{
	uint8_t cap = 0;

	if (losc != 1)
		return false;

	l2cap_frame_get_u8(frame, &cap);

	print_field("%*cFrequency: %s (0x%02x)", BASE_INDENT + 2, ' ',
			find_value_bit(cap & 0xf0, aptx_frequency_table),
			cap & 0xf0);

	print_field("%*cChannel Mode: %s (0x%02x)", BASE_INDENT + 2, ' ',
			find_value_bit(cap & 0x0f, aptx_channel_mode_table),
			cap & 0x0f);

	return true;
}

static bool codec_vendor_cfg(uint8_t losc, struct l2cap_frame *frame)
{
	uint32_t vendor_id = 0;
	uint16_t codec_id = 0;

	if (losc < 6)
		return false;

	l2cap_frame_get_le32(frame, &vendor_id);
	l2cap_frame_get_le16(frame, &codec_id);

	losc -= 6;

	print_field("%*cVendor ID: %s (0x%08x)", BASE_INDENT, ' ',
					bt_compidtostr(vendor_id),  vendor_id);

	print_field("%*cVendor Specific Codec ID: %s (0x%04x)", BASE_INDENT,
			' ', vndcodec2str(vendor_id, codec_id), codec_id);

	if (vendor_id == APTX_VENDOR_ID && codec_id == APTX_CODEC_ID)
		return codec_vendor_aptx_cfg(losc, frame);
	else if (vendor_id == LDAC_VENDOR_ID && codec_id == LDAC_CODEC_ID)
		return codec_vendor_ldac(losc, frame);

	packet_hexdump(frame->data, losc);
	l2cap_frame_pull(frame, frame, losc);

	return true;
}

bool a2dp_codec_cap(uint8_t codec, uint8_t losc, struct l2cap_frame *frame)
{
	switch (codec) {
	case A2DP_CODEC_SBC:
		return codec_sbc_cap(losc, frame);
	case A2DP_CODEC_MPEG12:
		return codec_mpeg12_cap(losc, frame);
	case A2DP_CODEC_MPEG24:
		return codec_aac_cap(losc, frame);
	case A2DP_CODEC_VENDOR:
		return codec_vendor_cap(losc, frame);
	default:
		packet_hexdump(frame->data, losc);
		l2cap_frame_pull(frame, frame, losc);
		return true;
	}
}

bool a2dp_codec_cfg(uint8_t codec, uint8_t losc, struct l2cap_frame *frame)
{
	switch (codec) {
	case A2DP_CODEC_SBC:
		return codec_sbc_cfg(losc, frame);
	case A2DP_CODEC_MPEG12:
		return codec_mpeg12_cfg(losc, frame);
	case A2DP_CODEC_MPEG24:
		return codec_aac_cfg(losc, frame);
	case A2DP_CODEC_VENDOR:
		return codec_vendor_cfg(losc, frame);
	default:
		packet_hexdump(frame->data, losc);
		l2cap_frame_pull(frame, frame, losc);
		return true;
	}
}
