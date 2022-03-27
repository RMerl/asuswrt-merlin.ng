/*
 * Copyright (C) 2013 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <time.h>
#include <hardware/audio.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN

struct rtp_header {
	unsigned cc:4;
	unsigned x:1;
	unsigned p:1;
	unsigned v:2;

	unsigned pt:7;
	unsigned m:1;

	uint16_t sequence_number;
	uint32_t timestamp;
	uint32_t ssrc;
	uint32_t csrc[0];
} __attribute__ ((packed));

#elif __BYTE_ORDER == __BIG_ENDIAN

struct rtp_header {
	unsigned v:2;
	unsigned p:1;
	unsigned x:1;
	unsigned cc:4;

	unsigned m:1;
	unsigned pt:7;

	uint16_t sequence_number;
	uint32_t timestamp;
	uint32_t ssrc;
	uint32_t csrc[0];
} __attribute__ ((packed));

#else
#error "Unknown byte order"
#endif

struct media_packet {
	uint8_t data[0];
};

struct media_packet_rtp {
	struct rtp_header hdr;
	uint8_t data[0];
};

struct audio_input_config {
	uint32_t rate;
	uint32_t channels;
	audio_format_t format;
};

struct audio_codec {
	uint8_t type;
	bool use_rtp;

	bool (*load) (void);
	void (*unload) (void);

	int (*get_presets) (struct audio_preset *preset, size_t *len);

	bool (*init) (struct audio_preset *preset, uint16_t mtu,
				void **codec_data);
	bool (*cleanup) (void *codec_data);
	bool (*get_config) (void *codec_data,
					struct audio_input_config *config);
	size_t (*get_buffer_size) (void *codec_data);
	size_t (*get_mediapacket_duration) (void *codec_data);
	ssize_t (*encode_mediapacket) (void *codec_data, const uint8_t *buffer,
					size_t len, struct media_packet *mp,
					size_t mp_data_len, size_t *written);
	bool (*update_qos) (void *codec_data, uint8_t op);
};

#define QOS_POLICY_DEFAULT	0x00
#define QOS_POLICY_DECREASE	0x01

typedef const struct audio_codec * (*audio_codec_get_t) (void);

const struct audio_codec *codec_sbc(void);
const struct audio_codec *codec_aptx(void);
