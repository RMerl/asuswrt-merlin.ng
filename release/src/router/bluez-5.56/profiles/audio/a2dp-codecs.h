/* SPDX-License-Identifier: LGPL-2.1-or-later */
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

#include <endian.h>
#include <stdint.h>

#define A2DP_CODEC_SBC			0x00
#define A2DP_CODEC_MPEG12		0x01
#define A2DP_CODEC_MPEG24		0x02
#define A2DP_CODEC_ATRAC		0x04
#define A2DP_CODEC_VENDOR		0xFF

#define SBC_SAMPLING_FREQ_16000		(1 << 3)
#define SBC_SAMPLING_FREQ_32000		(1 << 2)
#define SBC_SAMPLING_FREQ_44100		(1 << 1)
#define SBC_SAMPLING_FREQ_48000		1

#define SBC_CHANNEL_MODE_MONO		(1 << 3)
#define SBC_CHANNEL_MODE_DUAL_CHANNEL	(1 << 2)
#define SBC_CHANNEL_MODE_STEREO		(1 << 1)
#define SBC_CHANNEL_MODE_JOINT_STEREO	1

#define SBC_BLOCK_LENGTH_4		(1 << 3)
#define SBC_BLOCK_LENGTH_8		(1 << 2)
#define SBC_BLOCK_LENGTH_12		(1 << 1)
#define SBC_BLOCK_LENGTH_16		1

#define SBC_SUBBANDS_4			(1 << 1)
#define SBC_SUBBANDS_8			1

#define SBC_ALLOCATION_SNR		(1 << 1)
#define SBC_ALLOCATION_LOUDNESS		1

#define SBC_MIN_BITPOOL			2
#define SBC_MAX_BITPOOL			250

/* Other settings:
 * Block length = 16
 * Allocation method = Loudness
 * Subbands = 8
 */
#define SBC_BITPOOL_MQ_MONO_44100		19
#define SBC_BITPOOL_MQ_MONO_48000		18
#define SBC_BITPOOL_MQ_JOINT_STEREO_44100	35
#define SBC_BITPOOL_MQ_JOINT_STEREO_48000	33
#define SBC_BITPOOL_HQ_MONO_44100		31
#define SBC_BITPOOL_HQ_MONO_48000		29
#define SBC_BITPOOL_HQ_JOINT_STEREO_44100	53
#define SBC_BITPOOL_HQ_JOINT_STEREO_48000	51

#define MPEG_CHANNEL_MODE_MONO		(1 << 3)
#define MPEG_CHANNEL_MODE_DUAL_CHANNEL	(1 << 2)
#define MPEG_CHANNEL_MODE_STEREO	(1 << 1)
#define MPEG_CHANNEL_MODE_JOINT_STEREO	1

#define MPEG_LAYER_MP1			(1 << 2)
#define MPEG_LAYER_MP2			(1 << 1)
#define MPEG_LAYER_MP3			1

#define MPEG_SAMPLING_FREQ_16000	(1 << 5)
#define MPEG_SAMPLING_FREQ_22050	(1 << 4)
#define MPEG_SAMPLING_FREQ_24000	(1 << 3)
#define MPEG_SAMPLING_FREQ_32000	(1 << 2)
#define MPEG_SAMPLING_FREQ_44100	(1 << 1)
#define MPEG_SAMPLING_FREQ_48000	1

#define MPEG_BIT_RATE_INDEX_0		(1 << 0)
#define MPEG_BIT_RATE_INDEX_1		(1 << 1)
#define MPEG_BIT_RATE_INDEX_2		(1 << 2)
#define MPEG_BIT_RATE_INDEX_3		(1 << 3)
#define MPEG_BIT_RATE_INDEX_4		(1 << 4)
#define MPEG_BIT_RATE_INDEX_5		(1 << 5)
#define MPEG_BIT_RATE_INDEX_6		(1 << 6)
#define MPEG_BIT_RATE_INDEX_7		(1 << 7)
#define MPEG_BIT_RATE_INDEX_8		(1 << 8)
#define MPEG_BIT_RATE_INDEX_9		(1 << 9)
#define MPEG_BIT_RATE_INDEX_10		(1 << 10)
#define MPEG_BIT_RATE_INDEX_11		(1 << 11)
#define MPEG_BIT_RATE_INDEX_12		(1 << 12)
#define MPEG_BIT_RATE_INDEX_13		(1 << 13)
#define MPEG_BIT_RATE_INDEX_14		(1 << 14)

#define MPEG_MP1_BIT_RATE_32000		MPEG_BIT_RATE_INDEX_1
#define MPEG_MP1_BIT_RATE_64000		MPEG_BIT_RATE_INDEX_2
#define MPEG_MP1_BIT_RATE_96000		MPEG_BIT_RATE_INDEX_3
#define MPEG_MP1_BIT_RATE_128000	MPEG_BIT_RATE_INDEX_4
#define MPEG_MP1_BIT_RATE_160000	MPEG_BIT_RATE_INDEX_5
#define MPEG_MP1_BIT_RATE_192000	MPEG_BIT_RATE_INDEX_6
#define MPEG_MP1_BIT_RATE_224000	MPEG_BIT_RATE_INDEX_7
#define MPEG_MP1_BIT_RATE_256000	MPEG_BIT_RATE_INDEX_8
#define MPEG_MP1_BIT_RATE_288000	MPEG_BIT_RATE_INDEX_9
#define MPEG_MP1_BIT_RATE_320000	MPEG_BIT_RATE_INDEX_10
#define MPEG_MP1_BIT_RATE_352000	MPEG_BIT_RATE_INDEX_11
#define MPEG_MP1_BIT_RATE_384000	MPEG_BIT_RATE_INDEX_12
#define MPEG_MP1_BIT_RATE_416000	MPEG_BIT_RATE_INDEX_13
#define MPEG_MP1_BIT_RATE_448000	MPEG_BIT_RATE_INDEX_14

#define MPEG_MP2_BIT_RATE_32000		MPEG_BIT_RATE_INDEX_1
#define MPEG_MP2_BIT_RATE_48000		MPEG_BIT_RATE_INDEX_2
#define MPEG_MP2_BIT_RATE_56000		MPEG_BIT_RATE_INDEX_3
#define MPEG_MP2_BIT_RATE_64000		MPEG_BIT_RATE_INDEX_4
#define MPEG_MP2_BIT_RATE_80000		MPEG_BIT_RATE_INDEX_5
#define MPEG_MP2_BIT_RATE_96000		MPEG_BIT_RATE_INDEX_6
#define MPEG_MP2_BIT_RATE_112000	MPEG_BIT_RATE_INDEX_7
#define MPEG_MP2_BIT_RATE_128000	MPEG_BIT_RATE_INDEX_8
#define MPEG_MP2_BIT_RATE_160000	MPEG_BIT_RATE_INDEX_9
#define MPEG_MP2_BIT_RATE_192000	MPEG_BIT_RATE_INDEX_10
#define MPEG_MP2_BIT_RATE_224000	MPEG_BIT_RATE_INDEX_11
#define MPEG_MP2_BIT_RATE_256000	MPEG_BIT_RATE_INDEX_12
#define MPEG_MP2_BIT_RATE_320000	MPEG_BIT_RATE_INDEX_13
#define MPEG_MP2_BIT_RATE_384000	MPEG_BIT_RATE_INDEX_14

#define MPEG_MP3_BIT_RATE_32000		MPEG_BIT_RATE_INDEX_1
#define MPEG_MP3_BIT_RATE_40000		MPEG_BIT_RATE_INDEX_2
#define MPEG_MP3_BIT_RATE_48000		MPEG_BIT_RATE_INDEX_3
#define MPEG_MP3_BIT_RATE_56000		MPEG_BIT_RATE_INDEX_4
#define MPEG_MP3_BIT_RATE_64000		MPEG_BIT_RATE_INDEX_5
#define MPEG_MP3_BIT_RATE_80000		MPEG_BIT_RATE_INDEX_6
#define MPEG_MP3_BIT_RATE_96000		MPEG_BIT_RATE_INDEX_7
#define MPEG_MP3_BIT_RATE_112000	MPEG_BIT_RATE_INDEX_8
#define MPEG_MP3_BIT_RATE_128000	MPEG_BIT_RATE_INDEX_9
#define MPEG_MP3_BIT_RATE_160000	MPEG_BIT_RATE_INDEX_10
#define MPEG_MP3_BIT_RATE_192000	MPEG_BIT_RATE_INDEX_11
#define MPEG_MP3_BIT_RATE_224000	MPEG_BIT_RATE_INDEX_12
#define MPEG_MP3_BIT_RATE_256000	MPEG_BIT_RATE_INDEX_13
#define MPEG_MP3_BIT_RATE_320000	MPEG_BIT_RATE_INDEX_14

#define MPEG_BIT_RATE_FREE		MPEG_BIT_RATE_INDEX_0

#define MPEG_GET_BITRATE(a) ((uint16_t)(a).bitrate1 << 8 | (a).bitrate2)
#define MPEG_SET_BITRATE(a, b) \
	do { \
		(a).bitrate1 = ((b) >> 8) & 0x7f; \
		(a).bitrate2 = (b) & 0xff; \
	} while (0)

#define AAC_OBJECT_TYPE_MPEG2_AAC_LC	0x80
#define AAC_OBJECT_TYPE_MPEG4_AAC_LC	0x40
#define AAC_OBJECT_TYPE_MPEG4_AAC_LTP	0x20
#define AAC_OBJECT_TYPE_MPEG4_AAC_SCA	0x10

#define AAC_SAMPLING_FREQ_8000		0x0800
#define AAC_SAMPLING_FREQ_11025		0x0400
#define AAC_SAMPLING_FREQ_12000		0x0200
#define AAC_SAMPLING_FREQ_16000		0x0100
#define AAC_SAMPLING_FREQ_22050		0x0080
#define AAC_SAMPLING_FREQ_24000		0x0040
#define AAC_SAMPLING_FREQ_32000		0x0020
#define AAC_SAMPLING_FREQ_44100		0x0010
#define AAC_SAMPLING_FREQ_48000		0x0008
#define AAC_SAMPLING_FREQ_64000		0x0004
#define AAC_SAMPLING_FREQ_88200		0x0002
#define AAC_SAMPLING_FREQ_96000		0x0001

#define AAC_CHANNELS_1			0x02
#define AAC_CHANNELS_2			0x01

#define AAC_GET_BITRATE(a) ((a).bitrate1 << 16 | \
					(a).bitrate2 << 8 | (a).bitrate3)
#define AAC_GET_FREQUENCY(a) ((a).frequency1 << 4 | (a).frequency2)

#define AAC_SET_BITRATE(a, b) \
	do { \
		(a).bitrate1 = (b >> 16) & 0x7f; \
		(a).bitrate2 = (b >> 8) & 0xff; \
		(a).bitrate3 = b & 0xff; \
	} while (0)
#define AAC_SET_FREQUENCY(a, f) \
	do { \
		(a).frequency1 = (f >> 4) & 0xff; \
		(a).frequency2 = f & 0x0f; \
	} while (0)

#define AAC_INIT_BITRATE(b) \
	.bitrate1 = (b >> 16) & 0x7f, \
	.bitrate2 = (b >> 8) & 0xff, \
	.bitrate3 = b & 0xff,
#define AAC_INIT_FREQUENCY(f) \
	.frequency1 = (f >> 4) & 0xff, \
	.frequency2 = f & 0x0f,

#define APTX_VENDOR_ID			0x0000004f
#define APTX_CODEC_ID			0x0001

#define APTX_CHANNEL_MODE_MONO		0x01
#define APTX_CHANNEL_MODE_STEREO	0x02

#define APTX_SAMPLING_FREQ_16000	0x08
#define APTX_SAMPLING_FREQ_32000	0x04
#define APTX_SAMPLING_FREQ_44100	0x02
#define APTX_SAMPLING_FREQ_48000	0x01

#define FASTSTREAM_VENDOR_ID		0x0000000a
#define FASTSTREAM_CODEC_ID		0x0001

#define FASTSTREAM_DIRECTION_SINK	0x1
#define FASTSTREAM_DIRECTION_SOURCE	0x2

#define FASTSTREAM_SINK_SAMPLING_FREQ_44100	0x2
#define FASTSTREAM_SINK_SAMPLING_FREQ_48000	0x1

#define FASTSTREAM_SOURCE_SAMPLING_FREQ_16000	0x2

#define APTX_LL_VENDOR_ID		0x0000000a
#define APTX_LL_CODEC_ID		0x0002

/* Default parameters for aptX Low Latency encoder */

/* Target codec buffer level = 180 */
#define APTX_LL_TARGET_LEVEL2	0xb4
#define APTX_LL_TARGET_LEVEL1	0x00

/* Initial codec buffer level = 360 */
#define APTX_LL_INITIAL_LEVEL2	0x68
#define APTX_LL_INITIAL_LEVEL1	0x01

/* SRA max rate 0.005 * 10000 = 50 */
#define APTX_LL_SRA_MAX_RATE		0x32

/* SRA averaging time = 1s */
#define APTX_LL_SRA_AVG_TIME		0x01

/* Good working codec buffer level = 180 */
#define APTX_LL_GOOD_WORKING_LEVEL2	0xB4
#define APTX_LL_GOOD_WORKING_LEVEL1	0x00

#define APTX_HD_VENDOR_ID		0x000000D7
#define APTX_HD_CODEC_ID		0x0024

#define LDAC_VENDOR_ID			0x0000012d
#define LDAC_CODEC_ID			0x00aa

#define LDAC_SAMPLING_FREQ_44100	0x20
#define LDAC_SAMPLING_FREQ_48000	0x10
#define LDAC_SAMPLING_FREQ_88200	0x08
#define LDAC_SAMPLING_FREQ_96000	0x04
#define LDAC_SAMPLING_FREQ_176400	0x02
#define LDAC_SAMPLING_FREQ_192000	0x01

#define LDAC_CHANNEL_MODE_MONO		0x04
#define LDAC_CHANNEL_MODE_DUAL		0x02
#define LDAC_CHANNEL_MODE_STEREO	0x01

typedef struct {
	uint8_t vendor_id4;
	uint8_t vendor_id3;
	uint8_t vendor_id2;
	uint8_t vendor_id1;
	uint8_t codec_id2;
	uint8_t codec_id1;
} __attribute__ ((packed)) a2dp_vendor_codec_t;

#define A2DP_GET_VENDOR_ID(a) ( \
		(((uint32_t)(a).vendor_id4) <<  0) | \
		(((uint32_t)(a).vendor_id3) <<  8) | \
		(((uint32_t)(a).vendor_id2) << 16) | \
		(((uint32_t)(a).vendor_id1) << 24) \
	)
#define A2DP_GET_CODEC_ID(a) ((a).codec_id2 | (((uint16_t)(a).codec_id1) << 8))
#define A2DP_SET_VENDOR_ID_CODEC_ID(v, c) ((a2dp_vendor_codec_t){ \
		.vendor_id4 = (((v) >>  0) & 0xff), \
		.vendor_id3 = (((v) >>  8) & 0xff), \
		.vendor_id2 = (((v) >> 16) & 0xff), \
		.vendor_id1 = (((v) >> 24) & 0xff), \
		.codec_id2 = (((c) >> 0) & 0xff), \
		.codec_id1 = (((c) >> 8) & 0xff), \
	})

typedef struct {
	uint8_t reserved;
	uint8_t target_level2;
	uint8_t target_level1;
	uint8_t initial_level2;
	uint8_t initial_level1;
	uint8_t sra_max_rate;
	uint8_t sra_avg_time;
	uint8_t good_working_level2;
	uint8_t good_working_level1;
} __attribute__ ((packed)) a2dp_aptx_ll_new_caps_t;

typedef struct {
	a2dp_vendor_codec_t info;
	uint8_t frequency;
	uint8_t channel_mode;
} __attribute__ ((packed)) a2dp_ldac_t;

#if defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && \
	__BYTE_ORDER == __LITTLE_ENDIAN

typedef struct {
	uint8_t channel_mode:4;
	uint8_t frequency:4;
	uint8_t allocation_method:2;
	uint8_t subbands:2;
	uint8_t block_length:4;
	uint8_t min_bitpool;
	uint8_t max_bitpool;
} __attribute__ ((packed)) a2dp_sbc_t;

typedef struct {
	uint8_t channel_mode:4;
	uint8_t crc:1;
	uint8_t layer:3;
	uint8_t frequency:6;
	uint8_t mpf:1;
	uint8_t rfa:1;
	uint8_t bitrate1:7;
	uint8_t vbr:1;
	uint8_t bitrate2;
} __attribute__ ((packed)) a2dp_mpeg_t;

typedef struct {
	uint8_t object_type;
	uint8_t frequency1;
	uint8_t rfa:2;
	uint8_t channels:2;
	uint8_t frequency2:4;
	uint8_t bitrate1:7;
	uint8_t vbr:1;
	uint8_t bitrate2;
	uint8_t bitrate3;
} __attribute__ ((packed)) a2dp_aac_t;

typedef struct {
	a2dp_vendor_codec_t info;
	uint8_t channel_mode:4;
	uint8_t frequency:4;
} __attribute__ ((packed)) a2dp_aptx_t;

typedef struct {
	a2dp_vendor_codec_t info;
	uint8_t direction;
	uint8_t sink_frequency:4;
	uint8_t source_frequency:4;
} __attribute__ ((packed)) a2dp_faststream_t;

typedef struct {
	a2dp_aptx_t aptx;
	uint8_t bidirect_link:1;
	uint8_t has_new_caps:1;
	uint8_t reserved:6;
	a2dp_aptx_ll_new_caps_t new_caps[0];
} __attribute__ ((packed)) a2dp_aptx_ll_t;

#elif defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) && \
	__BYTE_ORDER == __BIG_ENDIAN

typedef struct {
	uint8_t frequency:4;
	uint8_t channel_mode:4;
	uint8_t block_length:4;
	uint8_t subbands:2;
	uint8_t allocation_method:2;
	uint8_t min_bitpool;
	uint8_t max_bitpool;
} __attribute__ ((packed)) a2dp_sbc_t;

typedef struct {
	uint8_t layer:3;
	uint8_t crc:1;
	uint8_t channel_mode:4;
	uint8_t rfa:1;
	uint8_t mpf:1;
	uint8_t frequency:6;
	uint8_t vbr:1;
	uint8_t bitrate1:7;
	uint8_t bitrate2;
} __attribute__ ((packed)) a2dp_mpeg_t;

typedef struct {
	uint8_t object_type;
	uint8_t frequency1;
	uint8_t frequency2:4;
	uint8_t channels:2;
	uint8_t rfa:2;
	uint8_t vbr:1;
	uint8_t bitrate1:7;
	uint8_t bitrate2;
	uint8_t bitrate3;
} __attribute__ ((packed)) a2dp_aac_t;

typedef struct {
	a2dp_vendor_codec_t info;
	uint8_t frequency:4;
	uint8_t channel_mode:4;
} __attribute__ ((packed)) a2dp_aptx_t;

typedef struct {
	a2dp_vendor_codec_t info;
	uint8_t direction;
	uint8_t source_frequency:4;
	uint8_t sink_frequency:4;
} __attribute__ ((packed)) a2dp_faststream_t;

typedef struct {
	a2dp_aptx_t aptx;
	uint8_t reserved:6;
	uint8_t has_new_caps:1;
	uint8_t bidirect_link:1;
	a2dp_aptx_ll_new_caps_t new_caps[0];
} __attribute__ ((packed)) a2dp_aptx_ll_t;

#else
#error "Unknown byte order"
#endif

typedef struct {
	a2dp_aptx_t aptx;
	uint8_t reserved0;
	uint8_t reserved1;
	uint8_t reserved2;
	uint8_t reserved3;
} __attribute__ ((packed)) a2dp_aptx_hd_t;
