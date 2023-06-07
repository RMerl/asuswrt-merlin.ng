// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2018       Pali Rohár <pali.rohar@gmail.com>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"
#include "lib/l2cap.h"

#include "profiles/audio/a2dp-codecs.h"

#define AVDTP_PSM			25

/* Commands */
#define AVDTP_DISCOVER			0x01
#define AVDTP_GET_CAPABILITIES		0x02

#define AVDTP_PKT_TYPE_SINGLE		0x00

#define AVDTP_MSG_TYPE_COMMAND		0x00

/* SEP capability categories */
#define AVDTP_MEDIA_TRANSPORT		0x01
#define AVDTP_REPORTING			0x02
#define AVDTP_RECOVERY			0x03
#define AVDTP_CONTENT_PROTECTION	0x04
#define AVDTP_HEADER_COMPRESSION	0x05
#define AVDTP_MULTIPLEXING		0x06
#define AVDTP_MEDIA_CODEC		0x07

/* SEP types definitions */
#define AVDTP_SEP_TYPE_SOURCE		0x00
#define AVDTP_SEP_TYPE_SINK		0x01

/* Media types definitions */
#define AVDTP_MEDIA_TYPE_AUDIO		0x00
#define AVDTP_MEDIA_TYPE_VIDEO		0x01
#define AVDTP_MEDIA_TYPE_MULTIMEDIA	0x02

/* Content Protection types definitions */
#define AVDTP_CONTENT_PROTECTION_TYPE_DTCP	0x0001
#define AVDTP_CONTENT_PROTECTION_TYPE_SCMS_T	0x0002

struct avdtp_service_capability {
	uint8_t category;
	uint8_t length;
	uint8_t data[0];
} __attribute__ ((packed));

#if __BYTE_ORDER == __LITTLE_ENDIAN

struct avdtp_header {
	uint8_t message_type:2;
	uint8_t packet_type:2;
	uint8_t transaction:4;
	uint8_t signal_id:6;
	uint8_t rfa0:2;
} __attribute__ ((packed));

struct seid_info {
	uint8_t rfa0:1;
	uint8_t inuse:1;
	uint8_t seid:6;
	uint8_t rfa2:3;
	uint8_t type:1;
	uint8_t media_type:4;
} __attribute__ ((packed));

struct seid_req {
	struct avdtp_header header;
	uint8_t rfa0:2;
	uint8_t acp_seid:6;
} __attribute__ ((packed));

struct avdtp_media_codec_capability {
	uint8_t rfa0:4;
	uint8_t media_type:4;
	uint8_t media_codec_type;
	uint8_t data[0];
} __attribute__ ((packed));

#elif __BYTE_ORDER == __BIG_ENDIAN

struct avdtp_header {
	uint8_t transaction:4;
	uint8_t packet_type:2;
	uint8_t message_type:2;
	uint8_t rfa0:2;
	uint8_t signal_id:6;
} __attribute__ ((packed));

struct seid_info {
	uint8_t seid:6;
	uint8_t inuse:1;
	uint8_t rfa0:1;
	uint8_t media_type:4;
	uint8_t type:1;
	uint8_t rfa2:3;
} __attribute__ ((packed));

struct seid_req {
	struct avdtp_header header;
	uint8_t acp_seid:6;
	uint8_t rfa0:2;
} __attribute__ ((packed));

struct avdtp_media_codec_capability {
	uint8_t media_type:4;
	uint8_t rfa0:4;
	uint8_t media_codec_type;
	uint8_t data[0];
} __attribute__ ((packed));

#else
#error "Unknown byte order"
#endif

struct discover_resp {
	struct avdtp_header header;
	struct seid_info seps[0];
} __attribute__ ((packed));

struct getcap_resp {
	struct avdtp_header header;
	uint8_t caps[0];
} __attribute__ ((packed));

struct avdtp_content_protection_capability {
	uint16_t content_protection_type;
	uint8_t data[0];
} __attribute__ ((packed));

static void print_aptx_common(a2dp_aptx_t *aptx)
{
	printf("\n\t\t\tFrequencies: ");
	if (aptx->frequency & APTX_SAMPLING_FREQ_16000)
		printf("16kHz ");
	if (aptx->frequency & APTX_SAMPLING_FREQ_32000)
		printf("32kHz ");
	if (aptx->frequency & APTX_SAMPLING_FREQ_44100)
		printf("44.1kHz ");
	if (aptx->frequency & APTX_SAMPLING_FREQ_48000)
		printf("48kHz ");

	printf("\n\t\t\tChannel modes: ");
	if (aptx->channel_mode & APTX_CHANNEL_MODE_MONO)
		printf("Mono ");
	if (aptx->channel_mode & APTX_CHANNEL_MODE_STEREO)
		printf("Stereo ");
}

static void print_aptx(a2dp_aptx_t *aptx, uint8_t size)
{
	printf("\t\tVendor Specific Value (aptX)");

	if (size < sizeof(*aptx)) {
		printf(" (broken)\n");
		return;
	}

	print_aptx_common(aptx);

	printf("\n");
}

static void print_faststream(a2dp_faststream_t *faststream, uint8_t size)
{
	printf("\t\tVendor Specific Value (FastStream)");

	if (size < sizeof(*faststream)) {
		printf(" (broken)\n");
		return;
	}

	printf("\n\t\t\tDirections: ");
	if (faststream->direction & FASTSTREAM_DIRECTION_SINK)
		printf("sink ");
	if (faststream->direction & FASTSTREAM_DIRECTION_SOURCE)
		printf("source ");

	if (faststream->direction & FASTSTREAM_DIRECTION_SINK) {
		printf("\n\t\t\tSink Frequencies: ");
		if (faststream->sink_frequency &
				FASTSTREAM_SINK_SAMPLING_FREQ_44100)
			printf("44.1kHz ");
		if (faststream->sink_frequency &
				FASTSTREAM_SINK_SAMPLING_FREQ_48000)
			printf("48kHz ");
	}

	if (faststream->direction & FASTSTREAM_DIRECTION_SOURCE) {
		printf("\n\t\t\tSource Frequencies: ");
		if (faststream->source_frequency &
				FASTSTREAM_SOURCE_SAMPLING_FREQ_16000)
			printf("16kHz ");
	}

	printf("\n");
}

static void print_aptx_ll(a2dp_aptx_ll_t *aptx_ll, uint8_t size)
{
	a2dp_aptx_ll_new_caps_t *aptx_ll_new;

	printf("\t\tVendor Specific Value (aptX Low Latency)");

	if (size < sizeof(*aptx_ll)) {
		printf(" (broken)\n");
		return;
	}

	print_aptx_common(&aptx_ll->aptx);

	printf("\n\t\tBidirectional link: %s",
			aptx_ll->bidirect_link ? "Yes" : "No");

	aptx_ll_new = &aptx_ll->new_caps[0];
	if (aptx_ll->has_new_caps &&
	    size >= sizeof(*aptx_ll) + sizeof(*aptx_ll_new)) {
		printf("\n\t\tTarget codec buffer level: %u",
			(unsigned int)aptx_ll_new->target_level2 |
			((unsigned int)(aptx_ll_new->target_level1) << 8));
		printf("\n\t\tInitial codec buffer level: %u",
			(unsigned int)aptx_ll_new->initial_level2 |
			((unsigned int)(aptx_ll_new->initial_level1) << 8));
		printf("\n\t\tSRA max rate: %g",
			aptx_ll_new->sra_max_rate / 10000.0);
		printf("\n\t\tSRA averaging time: %us",
			(unsigned int)aptx_ll_new->sra_avg_time);
		printf("\n\t\tGood working codec buffer level: %u",
			(unsigned int)aptx_ll_new->good_working_level2 |
			((unsigned int)(aptx_ll_new->good_working_level1) << 8)
			);
	}

	printf("\n");
}

static void print_aptx_hd(a2dp_aptx_hd_t *aptx_hd, uint8_t size)
{
	printf("\t\tVendor Specific Value (aptX HD)");

	if (size < sizeof(*aptx_hd)) {
		printf(" (broken)\n");
		return;
	}

	print_aptx_common(&aptx_hd->aptx);

	printf("\n");
}

static void print_ldac(a2dp_ldac_t *ldac, uint8_t size)
{
	printf("\t\tVendor Specific Value (LDAC)");

	if (size < sizeof(*ldac)) {
		printf(" (broken)\n");
		return;
	}

	printf("\n\t\t\tFrequencies: ");
	if (ldac->frequency & LDAC_SAMPLING_FREQ_44100)
		printf("44.1kHz ");
	if (ldac->frequency & LDAC_SAMPLING_FREQ_48000)
		printf("48kHz ");
	if (ldac->frequency & LDAC_SAMPLING_FREQ_88200)
		printf("88.2kHz ");
	if (ldac->frequency & LDAC_SAMPLING_FREQ_96000)
		printf("96kHz ");
	if (ldac->frequency & LDAC_SAMPLING_FREQ_176400)
		printf("176.4kHz ");
	if (ldac->frequency & LDAC_SAMPLING_FREQ_192000)
		printf("192kHz ");

	printf("\n\t\t\tChannel modes: ");
	if (ldac->channel_mode & LDAC_CHANNEL_MODE_MONO)
		printf("Mono ");
	if (ldac->channel_mode & LDAC_CHANNEL_MODE_DUAL)
		printf("Dual ");
	if (ldac->channel_mode & LDAC_CHANNEL_MODE_STEREO)
		printf("Stereo ");

	printf("\n");
}

static void print_vendor(a2dp_vendor_codec_t *vendor, uint8_t size)
{
	uint32_t vendor_id;
	uint16_t codec_id;
	int i;

	if (size < sizeof(*vendor)) {
		printf("\tMedia Codec: Vendor Specific A2DP Codec (broken)");
		return;
	}

	vendor_id = A2DP_GET_VENDOR_ID(*vendor);
	codec_id = A2DP_GET_CODEC_ID(*vendor);

	printf("\tMedia Codec: Vendor Specific A2DP Codec");

	printf("\n\t\tVendor ID 0x%08x", vendor_id);

	printf("\n\t\tVendor Specific Codec ID 0x%04x", codec_id);

	printf("\n\t\tVendor Specific Data:");
	for (i = 6; i < size; ++i)
		printf(" 0x%.02x", ((unsigned char *)vendor)[i]);
	printf("\n");

	if (vendor_id == APTX_VENDOR_ID && codec_id == APTX_CODEC_ID)
		print_aptx((void *) vendor, size);
	else if (vendor_id == FASTSTREAM_VENDOR_ID &&
			codec_id == FASTSTREAM_CODEC_ID)
		print_faststream((void *) vendor, size);
	else if (vendor_id == APTX_LL_VENDOR_ID && codec_id == APTX_LL_CODEC_ID)
		print_aptx_ll((void *) vendor, size);
	else if (vendor_id == APTX_HD_VENDOR_ID && codec_id == APTX_HD_CODEC_ID)
		print_aptx_hd((void *) vendor, size);
	else if (vendor_id == LDAC_VENDOR_ID && codec_id == LDAC_CODEC_ID)
		print_ldac((void *) vendor, size);
}

static void print_mpeg24(a2dp_aac_t *aac, uint8_t size)
{
	unsigned int freq, bitrate;

	if (size < sizeof(*aac)) {
		printf("\tMedia Codec: MPEG24 (broken)\n");
		return;
	}

	freq = AAC_GET_FREQUENCY(*aac);
	bitrate = AAC_GET_BITRATE(*aac);

	printf("\tMedia Codec: MPEG24\n\t\tObject Types: ");

	if (aac->object_type & AAC_OBJECT_TYPE_MPEG2_AAC_LC)
		printf("MPEG-2 AAC LC ");
	if (aac->object_type & AAC_OBJECT_TYPE_MPEG4_AAC_LC)
		printf("MPEG-4 AAC LC ");
	if (aac->object_type & AAC_OBJECT_TYPE_MPEG4_AAC_LTP)
		printf("MPEG-4 AAC LTP ");
	if (aac->object_type & AAC_OBJECT_TYPE_MPEG4_AAC_SCA)
		printf("MPEG-4 AAC scalable ");

	printf("\n\t\tFrequencies: ");
	if (freq & AAC_SAMPLING_FREQ_8000)
		printf("8kHz ");
	if (freq & AAC_SAMPLING_FREQ_11025)
		printf("11.025kHz ");
	if (freq & AAC_SAMPLING_FREQ_12000)
		printf("12kHz ");
	if (freq & AAC_SAMPLING_FREQ_16000)
		printf("16kHz ");
	if (freq & AAC_SAMPLING_FREQ_22050)
		printf("22.05kHz ");
	if (freq & AAC_SAMPLING_FREQ_24000)
		printf("24kHz ");
	if (freq & AAC_SAMPLING_FREQ_32000)
		printf("32kHz ");
	if (freq & AAC_SAMPLING_FREQ_44100)
		printf("44.1kHz ");
	if (freq & AAC_SAMPLING_FREQ_48000)
		printf("48kHz ");
	if (freq & AAC_SAMPLING_FREQ_64000)
		printf("64kHz ");
	if (freq & AAC_SAMPLING_FREQ_88200)
		printf("88.2kHz ");
	if (freq & AAC_SAMPLING_FREQ_96000)
		printf("96kHz ");

	printf("\n\t\tChannels: ");
	if (aac->channels & AAC_CHANNELS_1)
		printf("1 ");
	if (aac->channels & AAC_CHANNELS_2)
		printf("2 ");

	printf("\n\t\tBitrate: %u", bitrate);

	printf("\n\t\tVBR: %s", aac->vbr ? "Yes\n" : "No\n");
}

static void print_mpeg12(a2dp_mpeg_t *mpeg, uint8_t size)
{
	uint16_t bitrate;

	if (size < sizeof(*mpeg)) {
		printf("\tMedia Codec: MPEG12 (broken)\n");
		return;
	}

	bitrate = MPEG_GET_BITRATE(*mpeg);

	printf("\tMedia Codec: MPEG12\n\t\tChannel Modes: ");

	if (mpeg->channel_mode & MPEG_CHANNEL_MODE_MONO)
		printf("Mono ");
	if (mpeg->channel_mode & MPEG_CHANNEL_MODE_DUAL_CHANNEL)
		printf("DualChannel ");
	if (mpeg->channel_mode & MPEG_CHANNEL_MODE_STEREO)
		printf("Stereo ");
	if (mpeg->channel_mode & MPEG_CHANNEL_MODE_JOINT_STEREO)
		printf("JointStereo");

	printf("\n\t\tFrequencies: ");
	if (mpeg->frequency & MPEG_SAMPLING_FREQ_16000)
		printf("16Khz ");
	if (mpeg->frequency & MPEG_SAMPLING_FREQ_22050)
		printf("22.05Khz ");
	if (mpeg->frequency & MPEG_SAMPLING_FREQ_24000)
		printf("24Khz ");
	if (mpeg->frequency & MPEG_SAMPLING_FREQ_32000)
		printf("32Khz ");
	if (mpeg->frequency & MPEG_SAMPLING_FREQ_44100)
		printf("44.1Khz ");
	if (mpeg->frequency & MPEG_SAMPLING_FREQ_48000)
		printf("48Khz ");

	printf("\n\t\tCRC: %s", mpeg->crc ? "Yes" : "No");

	printf("\n\t\tLayer: ");
	if (mpeg->layer & MPEG_LAYER_MP1)
		printf("1 ");
	if (mpeg->layer & MPEG_LAYER_MP2)
		printf("2 ");
	if (mpeg->layer & MPEG_LAYER_MP3)
		printf("3 ");

	if (bitrate & MPEG_BIT_RATE_FREE) {
		printf("\n\t\tBit Rate: Free format");
	} else {
		if (mpeg->layer & MPEG_LAYER_MP1) {
			printf("\n\t\tLayer 1 Bit Rate: ");
			if (bitrate & MPEG_MP1_BIT_RATE_32000)
				printf("32kbps ");
			if (bitrate & MPEG_MP1_BIT_RATE_64000)
				printf("64kbps ");
			if (bitrate & MPEG_MP1_BIT_RATE_96000)
				printf("96kbps ");
			if (bitrate & MPEG_MP1_BIT_RATE_128000)
				printf("128kbps ");
			if (bitrate & MPEG_MP1_BIT_RATE_160000)
				printf("160kbps ");
			if (bitrate & MPEG_MP1_BIT_RATE_192000)
				printf("192kbps ");
			if (bitrate & MPEG_MP1_BIT_RATE_224000)
				printf("224kbps ");
			if (bitrate & MPEG_MP1_BIT_RATE_256000)
				printf("256kbps ");
			if (bitrate & MPEG_MP1_BIT_RATE_320000)
				printf("320kbps ");
			if (bitrate & MPEG_MP1_BIT_RATE_352000)
				printf("352kbps ");
			if (bitrate & MPEG_MP1_BIT_RATE_384000)
				printf("384kbps ");
			if (bitrate & MPEG_MP1_BIT_RATE_416000)
				printf("416kbps ");
			if (bitrate & MPEG_MP1_BIT_RATE_448000)
				printf("448kbps ");
		}

		if (mpeg->layer & MPEG_LAYER_MP2) {
			printf("\n\t\tLayer 2 Bit Rate: ");
			if (bitrate & MPEG_MP2_BIT_RATE_32000)
				printf("32kbps ");
			if (bitrate & MPEG_MP2_BIT_RATE_48000)
				printf("48kbps ");
			if (bitrate & MPEG_MP2_BIT_RATE_56000)
				printf("56kbps ");
			if (bitrate & MPEG_MP2_BIT_RATE_64000)
				printf("64kbps ");
			if (bitrate & MPEG_MP2_BIT_RATE_80000)
				printf("80kbps ");
			if (bitrate & MPEG_MP2_BIT_RATE_96000)
				printf("96kbps ");
			if (bitrate & MPEG_MP2_BIT_RATE_112000)
				printf("112kbps ");
			if (bitrate & MPEG_MP2_BIT_RATE_128000)
				printf("128kbps ");
			if (bitrate & MPEG_MP2_BIT_RATE_160000)
				printf("160kbps ");
			if (bitrate & MPEG_MP2_BIT_RATE_192000)
				printf("192kbps ");
			if (bitrate & MPEG_MP2_BIT_RATE_224000)
				printf("224kbps ");
			if (bitrate & MPEG_MP2_BIT_RATE_256000)
				printf("256kbps ");
			if (bitrate & MPEG_MP2_BIT_RATE_320000)
				printf("320kbps ");
			if (bitrate & MPEG_MP2_BIT_RATE_384000)
				printf("384kbps ");
		}

		if (mpeg->layer & MPEG_LAYER_MP3) {
			printf("\n\t\tLayer 3 Bit Rate: ");
			if (bitrate & MPEG_MP3_BIT_RATE_32000)
				printf("32kbps ");
			if (bitrate & MPEG_MP3_BIT_RATE_40000)
				printf("40kbps ");
			if (bitrate & MPEG_MP3_BIT_RATE_48000)
				printf("48kbps ");
			if (bitrate & MPEG_MP3_BIT_RATE_56000)
				printf("56kbps ");
			if (bitrate & MPEG_MP3_BIT_RATE_64000)
				printf("64kbps ");
			if (bitrate & MPEG_MP3_BIT_RATE_80000)
				printf("80kbps ");
			if (bitrate & MPEG_MP3_BIT_RATE_96000)
				printf("96kbps ");
			if (bitrate & MPEG_MP3_BIT_RATE_112000)
				printf("112kbps ");
			if (bitrate & MPEG_MP3_BIT_RATE_128000)
				printf("128kbps ");
			if (bitrate & MPEG_MP3_BIT_RATE_160000)
				printf("160kbps ");
			if (bitrate & MPEG_MP3_BIT_RATE_192000)
				printf("192kbps ");
			if (bitrate & MPEG_MP3_BIT_RATE_224000)
				printf("224kbps ");
			if (bitrate & MPEG_MP3_BIT_RATE_256000)
				printf("256kbps ");
			if (bitrate & MPEG_MP3_BIT_RATE_320000)
				printf("320kbps ");
		}
	}

	printf("\n\t\tVBR: %s", mpeg->vbr ? "Yes" : "No");

	printf("\n\t\tPayload Format: ");
	if (mpeg->mpf)
		printf("RFC-2250 RFC-3119\n");
	else
		printf("RFC-2250\n");
}

static void print_sbc(a2dp_sbc_t *sbc, uint8_t size)
{
	if (size < sizeof(*sbc)) {
		printf("\tMedia Codec: SBC (broken)\n");
		return;
	}

	printf("\tMedia Codec: SBC\n\t\tChannel Modes: ");

	if (sbc->channel_mode & SBC_CHANNEL_MODE_MONO)
		printf("Mono ");
	if (sbc->channel_mode & SBC_CHANNEL_MODE_DUAL_CHANNEL)
		printf("DualChannel ");
	if (sbc->channel_mode & SBC_CHANNEL_MODE_STEREO)
		printf("Stereo ");
	if (sbc->channel_mode & SBC_CHANNEL_MODE_JOINT_STEREO)
		printf("JointStereo");

	printf("\n\t\tFrequencies: ");
	if (sbc->frequency & SBC_SAMPLING_FREQ_16000)
		printf("16Khz ");
	if (sbc->frequency & SBC_SAMPLING_FREQ_32000)
		printf("32Khz ");
	if (sbc->frequency & SBC_SAMPLING_FREQ_44100)
		printf("44.1Khz ");
	if (sbc->frequency & SBC_SAMPLING_FREQ_48000)
		printf("48Khz ");

	printf("\n\t\tSubbands: ");
	if (sbc->allocation_method & SBC_SUBBANDS_4)
		printf("4 ");
	if (sbc->allocation_method & SBC_SUBBANDS_8)
		printf("8");

	printf("\n\t\tBlocks: ");
	if (sbc->block_length & SBC_BLOCK_LENGTH_4)
		printf("4 ");
	if (sbc->block_length & SBC_BLOCK_LENGTH_8)
		printf("8 ");
	if (sbc->block_length & SBC_BLOCK_LENGTH_12)
		printf("12 ");
	if (sbc->block_length & SBC_BLOCK_LENGTH_16)
		printf("16 ");

	printf("\n\t\tBitpool Range: %d-%d\n",
				sbc->min_bitpool, sbc->max_bitpool);
}

static void print_media_codec(
			struct avdtp_media_codec_capability *cap,
			uint8_t size)
{
	int i;

	if (size < sizeof(*cap)) {
		printf("\tMedia Codec: Unknown (broken)\n");
		return;
	}

	switch (cap->media_codec_type) {
	case A2DP_CODEC_SBC:
		print_sbc((void *) cap->data, size - 2);
		break;
	case A2DP_CODEC_MPEG12:
		print_mpeg12((void *) cap->data, size - 2);
		break;
	case A2DP_CODEC_MPEG24:
		print_mpeg24((void *) cap->data, size - 2);
		break;
	case A2DP_CODEC_VENDOR:
		print_vendor((void *) cap->data, size - 2);
		break;
	default:
		printf("\tMedia Codec: Unknown\n");
		printf("\t\tCodec Data:");
		for (i = 0; i < size - 2; ++i)
			printf(" 0x%.02x", ((unsigned char *)cap->data)[i]);
		printf("\n");
	}
}

static void print_content_protection(
				struct avdtp_content_protection_capability *cap,
				uint8_t size)
{
	printf("\tContent Protection: ");

	if (size < sizeof(*cap)) {
		printf("Unknown (broken)\n");
		return;
	}

	switch (btohs(cap->content_protection_type)) {
	case AVDTP_CONTENT_PROTECTION_TYPE_DTCP:
		printf("DTCP");
		break;
	case AVDTP_CONTENT_PROTECTION_TYPE_SCMS_T:
		printf("SCMS-T");
		break;
	default:
		printf("Unknown");
	}

	printf("\n");
}

static void print_caps(void *data, int size)
{
	int processed;
	int i;

	for (processed = 0; processed + 2 < size;) {
		struct avdtp_service_capability *cap;

		cap = data;

		if (processed + 2 + cap->length > size) {
			printf("Invalid capability data in getcap resp\n");
			break;
		}

		switch (cap->category) {
		case AVDTP_MEDIA_TRANSPORT:
		case AVDTP_REPORTING:
		case AVDTP_RECOVERY:
		case AVDTP_MULTIPLEXING:
			/* FIXME: Add proper functions */
			break;
		default:
			printf("\tUnknown category: %d\n", cap->category);
			printf("\t\tData:");
			for (i = 0; i < cap->length; ++i)
				printf(" 0x%.02x",
					((unsigned char *)cap->data)[i]);
			printf("\n");
			break;
		case AVDTP_MEDIA_CODEC:
			print_media_codec((void *) cap->data, cap->length);
			break;
		case AVDTP_CONTENT_PROTECTION:
			print_content_protection((void *) cap->data,
						cap->length);
			break;
		}

		processed += 2 + cap->length;
		data += 2 + cap->length;
	}
}

static void init_request(struct avdtp_header *header, int request_id)
{
	static int transaction = 0;

	header->packet_type = AVDTP_PKT_TYPE_SINGLE;
	header->message_type = AVDTP_MSG_TYPE_COMMAND;
	header->transaction = transaction;
	header->signal_id = request_id;

	/* clear rfa bits */
	header->rfa0 = 0;

	transaction = (transaction + 1) % 16;
}

static ssize_t avdtp_send(int sk, void *data, int len)
{
	ssize_t ret;

	ret = send(sk, data, len, 0);

	if (ret < 0)
		ret = -errno;
	else if (ret != len)
		ret = -EIO;

	if (ret < 0) {
		printf("Unable to send message: %s (%zd)\n",
						strerror(-ret), -ret);
		return ret;
	}

	return ret;
}

static ssize_t avdtp_receive(int sk, void *data, int len)
{
	ssize_t ret;

	ret = recv(sk, data, len, 0);

	if (ret < 0) {
		printf("Unable to receive message: %s (%d)\n",
						strerror(errno), errno);
		return -errno;
	}

	return ret;
}

static ssize_t avdtp_get_caps(int sk, int seid)
{
	struct seid_req req;
	char buffer[1024];
	struct getcap_resp *caps = (void *) buffer;
	ssize_t ret;

	memset(&req, 0, sizeof(req));
	init_request(&req.header, AVDTP_GET_CAPABILITIES);
	req.acp_seid = seid;

	ret = avdtp_send(sk, &req, sizeof(req));
	if (ret < 0)
		return ret;

	memset(&buffer, 0, sizeof(buffer));
	ret = avdtp_receive(sk, caps, sizeof(buffer));
	if (ret < 0)
		return ret;

	if ((size_t) ret < (sizeof(struct getcap_resp) + 4 +
			sizeof(struct avdtp_media_codec_capability))) {
		printf("Invalid capabilities\n");
		return -1;
	}

	print_caps(caps->caps, ret - sizeof(struct getcap_resp));

	return 0;
}

static ssize_t avdtp_discover(int sk)
{
	struct avdtp_header req;
	char buffer[256];
	struct discover_resp *discover = (void *) buffer;
	int seps, i;
	ssize_t ret;

	memset(&req, 0, sizeof(req));
	init_request(&req, AVDTP_DISCOVER);

	ret = avdtp_send(sk, &req, sizeof(req));
	if (ret < 0)
		return ret;

	memset(&buffer, 0, sizeof(buffer));
	ret = avdtp_receive(sk, discover, sizeof(buffer));
	if (ret < 0)
		return ret;

	seps = (ret - sizeof(struct avdtp_header)) / sizeof(struct seid_info);
	for (i = 0; i < seps; i++) {
		const char *type, *media;

		switch (discover->seps[i].type) {
		case AVDTP_SEP_TYPE_SOURCE:
			type = "Source";
			break;
		case AVDTP_SEP_TYPE_SINK:
			type = "Sink";
			break;
		default:
			type = "Invalid";
		}

		switch (discover->seps[i].media_type) {
		case AVDTP_MEDIA_TYPE_AUDIO:
			media = "Audio";
			break;
		case AVDTP_MEDIA_TYPE_VIDEO:
			media = "Video";
			break;
		case AVDTP_MEDIA_TYPE_MULTIMEDIA:
			media = "Multimedia";
			break;
		default:
			media = "Invalid";
		}

		printf("Stream End-Point #%d: %s %s %s\n",
					discover->seps[i].seid, media, type,
					discover->seps[i].inuse ? "*" : "");

		avdtp_get_caps(sk, discover->seps[i].seid);
	}

	return 0;
}

static int l2cap_connect(bdaddr_t *src, bdaddr_t *dst)
{
	struct sockaddr_l2 l2a;
	int sk;

	memset(&l2a, 0, sizeof(l2a));
	l2a.l2_family = AF_BLUETOOTH;
	bacpy(&l2a.l2_bdaddr, src);

	sk = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	if (sk < 0) {
		printf("Cannot create L2CAP socket. %s(%d)\n", strerror(errno),
				errno);
		return -errno;
	}

	if (bind(sk, (struct sockaddr *) &l2a, sizeof(l2a)) < 0) {
		printf("Bind failed. %s (%d)\n", strerror(errno), errno);
		close(sk);
		return -errno;
	}

	memset(&l2a, 0, sizeof(l2a));
	l2a.l2_family = AF_BLUETOOTH;
	bacpy(&l2a.l2_bdaddr, dst);
	l2a.l2_psm = htobs(AVDTP_PSM);

	if (connect(sk, (struct sockaddr *) &l2a, sizeof(l2a)) < 0) {
		printf("Connect failed. %s(%d)\n", strerror(errno), errno);
		close(sk);
		return -errno;
	}

	return sk;
}

static void usage(void)
{
	printf("avinfo - Audio/Video Info Tool ver %s\n", VERSION);
	printf("Usage:\n"
		"\tavinfo [options] <remote address>\n");
	printf("Options:\n"
		"\t-h\t\tDisplay help\n"
		"\t-i\t\tSpecify source interface\n");
}

static struct option main_options[] = {
	{ "help",	0, 0, 'h' },
	{ "device",	1, 0, 'i' },
	{ 0, 0, 0, 0 }
};

int main(int argc, char *argv[])
{
	bdaddr_t src, dst;
	int opt, sk, dev_id;

	if (argc < 2) {
		usage();
		exit(0);
	}

	bacpy(&src, BDADDR_ANY);
	dev_id = hci_get_route(&src);
	if ((dev_id < 0) || (hci_devba(dev_id, &src) < 0)) {
		printf("Cannot find any local adapter\n");
		exit(-1);
	}

	while ((opt = getopt_long(argc, argv, "+i:h", main_options, NULL)) != -1) {
		switch (opt) {
		case 'i':
			if (!strncmp(optarg, "hci", 3))
				hci_devba(atoi(optarg + 3), &src);
			else
				str2ba(optarg, &src);
			break;

		case 'h':
		default:
			usage();
			exit(0);
		}
	}

	printf("Connecting ... \n");

	if (bachk(argv[optind]) < 0) {
		printf("Invalid argument\n");
		exit(1);
	}

	str2ba(argv[optind], &dst);
	sk = l2cap_connect(&src, &dst);
	if (sk < 0)
		exit(1);

	if (avdtp_discover(sk) < 0)
		exit(1);

	return 0;
}
