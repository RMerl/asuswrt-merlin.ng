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

#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#include <sbc/sbc.h>
#include "audio-msg.h"
#include "hal-audio.h"
#include "hal-log.h"
#include "../profiles/audio/a2dp-codecs.h"

#define MAX_FRAMES_IN_PAYLOAD 15

#define SBC_QUALITY_MIN_BITPOOL	33
#define SBC_QUALITY_STEP	5


#if __BYTE_ORDER == __LITTLE_ENDIAN

struct rtp_payload {
	unsigned frame_count:4;
	unsigned rfa0:1;
	unsigned is_last_fragment:1;
	unsigned is_first_fragment:1;
	unsigned is_fragmented:1;
} __attribute__ ((packed));

#elif __BYTE_ORDER == __BIG_ENDIAN

struct rtp_payload {
	unsigned is_fragmented:1;
	unsigned is_first_fragment:1;
	unsigned is_last_fragment:1;
	unsigned rfa0:1;
	unsigned frame_count:4;
} __attribute__ ((packed));

#else
#error "Unknown byte order"
#endif

struct media_packet_sbc {
	struct media_packet_rtp hdr;
	struct rtp_payload payload;
	uint8_t data[0];
};

struct sbc_data {
	a2dp_sbc_t sbc;

	sbc_t enc;

	uint16_t payload_len;

	size_t in_frame_len;
	size_t in_buf_size;

	size_t out_frame_len;

	unsigned frame_duration;
	unsigned frames_per_packet;
};

static const a2dp_sbc_t sbc_presets[] = {
	{
		.frequency = SBC_SAMPLING_FREQ_44100 | SBC_SAMPLING_FREQ_48000,
		.channel_mode = SBC_CHANNEL_MODE_MONO |
				SBC_CHANNEL_MODE_DUAL_CHANNEL |
				SBC_CHANNEL_MODE_STEREO |
				SBC_CHANNEL_MODE_JOINT_STEREO,
		.subbands = SBC_SUBBANDS_4 | SBC_SUBBANDS_8,
		.allocation_method = SBC_ALLOCATION_SNR |
					SBC_ALLOCATION_LOUDNESS,
		.block_length = SBC_BLOCK_LENGTH_4 | SBC_BLOCK_LENGTH_8 |
				SBC_BLOCK_LENGTH_12 | SBC_BLOCK_LENGTH_16,
		.min_bitpool = MIN_BITPOOL,
		.max_bitpool = MAX_BITPOOL
	},
	{
		.frequency = SBC_SAMPLING_FREQ_44100,
		.channel_mode = SBC_CHANNEL_MODE_JOINT_STEREO,
		.subbands = SBC_SUBBANDS_8,
		.allocation_method = SBC_ALLOCATION_LOUDNESS,
		.block_length = SBC_BLOCK_LENGTH_16,
		.min_bitpool = MIN_BITPOOL,
		.max_bitpool = MAX_BITPOOL
	},
	{
		.frequency = SBC_SAMPLING_FREQ_48000,
		.channel_mode = SBC_CHANNEL_MODE_JOINT_STEREO,
		.subbands = SBC_SUBBANDS_8,
		.allocation_method = SBC_ALLOCATION_LOUDNESS,
		.block_length = SBC_BLOCK_LENGTH_16,
		.min_bitpool = MIN_BITPOOL,
		.max_bitpool = MAX_BITPOOL
	},
};

static int sbc_get_presets(struct audio_preset *preset, size_t *len)
{
	int i;
	int count;
	size_t new_len = 0;
	uint8_t *ptr = (uint8_t *) preset;
	size_t preset_size = sizeof(*preset) + sizeof(a2dp_sbc_t);

	count = sizeof(sbc_presets) / sizeof(sbc_presets[0]);

	for (i = 0; i < count; i++) {
		preset = (struct audio_preset *) ptr;

		if (new_len + preset_size > *len)
			break;

		preset->len = sizeof(a2dp_sbc_t);
		memcpy(preset->data, &sbc_presets[i], preset->len);

		new_len += preset_size;
		ptr += preset_size;
	}

	*len = new_len;

	return i;
}

static int sbc_freq2int(uint8_t freq)
{
	switch (freq) {
	case SBC_SAMPLING_FREQ_16000:
		return 16000;
	case SBC_SAMPLING_FREQ_32000:
		return 32000;
	case SBC_SAMPLING_FREQ_44100:
		return 44100;
	case SBC_SAMPLING_FREQ_48000:
		return 48000;
	default:
		return 0;
	}
}

static const char *sbc_mode2str(uint8_t mode)
{
	switch (mode) {
	case SBC_CHANNEL_MODE_MONO:
		return "Mono";
	case SBC_CHANNEL_MODE_DUAL_CHANNEL:
		return "DualChannel";
	case SBC_CHANNEL_MODE_STEREO:
		return "Stereo";
	case SBC_CHANNEL_MODE_JOINT_STEREO:
		return "JointStereo";
	default:
		return "(unknown)";
	}
}

static int sbc_blocks2int(uint8_t blocks)
{
	switch (blocks) {
	case SBC_BLOCK_LENGTH_4:
		return 4;
	case SBC_BLOCK_LENGTH_8:
		return 8;
	case SBC_BLOCK_LENGTH_12:
		return 12;
	case SBC_BLOCK_LENGTH_16:
		return 16;
	default:
		return 0;
	}
}

static int sbc_subbands2int(uint8_t subbands)
{
	switch (subbands) {
	case SBC_SUBBANDS_4:
		return 4;
	case SBC_SUBBANDS_8:
		return 8;
	default:
		return 0;
	}
}

static const char *sbc_allocation2str(uint8_t allocation)
{
	switch (allocation) {
	case SBC_ALLOCATION_SNR:
		return "SNR";
	case SBC_ALLOCATION_LOUDNESS:
		return "Loudness";
	default:
		return "(unknown)";
	}
}

static void sbc_init_encoder(struct sbc_data *sbc_data)
{
	a2dp_sbc_t *in = &sbc_data->sbc;
	sbc_t *out = &sbc_data->enc;

	sbc_init_a2dp(out, 0L, in, sizeof(*in));

	out->endian = SBC_LE;
	out->bitpool = in->max_bitpool;

	DBG("frequency=%d channel_mode=%s block_length=%d subbands=%d allocation=%s bitpool=%d-%d",
			sbc_freq2int(in->frequency),
			sbc_mode2str(in->channel_mode),
			sbc_blocks2int(in->block_length),
			sbc_subbands2int(in->subbands),
			sbc_allocation2str(in->allocation_method),
			in->min_bitpool, in->max_bitpool);
}

static void sbc_codec_calculate(struct sbc_data *sbc_data)
{
	size_t in_frame_len;
	size_t out_frame_len;
	size_t num_frames;

	in_frame_len = sbc_get_codesize(&sbc_data->enc);
	out_frame_len = sbc_get_frame_length(&sbc_data->enc);
	num_frames = sbc_data->payload_len / out_frame_len;

	if (num_frames > MAX_FRAMES_IN_PAYLOAD)
		num_frames = MAX_FRAMES_IN_PAYLOAD;

	sbc_data->in_frame_len = in_frame_len;
	sbc_data->in_buf_size = num_frames * in_frame_len;

	sbc_data->out_frame_len = out_frame_len;

	sbc_data->frame_duration = sbc_get_frame_duration(&sbc_data->enc);
	sbc_data->frames_per_packet = num_frames;

	DBG("in_frame_len=%zu out_frame_len=%zu frames_per_packet=%zu",
				in_frame_len, out_frame_len, num_frames);
}

static bool sbc_codec_init(struct audio_preset *preset, uint16_t payload_len,
							void **codec_data)
{
	struct sbc_data *sbc_data;

	if (preset->len != sizeof(a2dp_sbc_t)) {
		error("SBC: preset size mismatch");
		return false;
	}

	sbc_data = calloc(sizeof(struct sbc_data), 1);
	if (!sbc_data)
		return false;

	memcpy(&sbc_data->sbc, preset->data, preset->len);

	sbc_init_encoder(sbc_data);

	sbc_data->payload_len = payload_len;

	sbc_codec_calculate(sbc_data);

	*codec_data = sbc_data;

	return true;
}

static bool sbc_cleanup(void *codec_data)
{
	struct sbc_data *sbc_data = (struct sbc_data *) codec_data;

	sbc_finish(&sbc_data->enc);
	free(codec_data);

	return true;
}

static bool sbc_get_config(void *codec_data, struct audio_input_config *config)
{
	struct sbc_data *sbc_data = (struct sbc_data *) codec_data;

	switch (sbc_data->sbc.frequency) {
	case SBC_SAMPLING_FREQ_16000:
		config->rate = 16000;
		break;
	case SBC_SAMPLING_FREQ_32000:
		config->rate = 32000;
		break;
	case SBC_SAMPLING_FREQ_44100:
		config->rate = 44100;
		break;
	case SBC_SAMPLING_FREQ_48000:
		config->rate = 48000;
		break;
	default:
		return false;
	}
	config->channels = sbc_data->sbc.channel_mode == SBC_CHANNEL_MODE_MONO ?
				AUDIO_CHANNEL_OUT_MONO :
				AUDIO_CHANNEL_OUT_STEREO;
	config->format = AUDIO_FORMAT_PCM_16_BIT;

	return true;
}

static size_t sbc_get_buffer_size(void *codec_data)
{
	struct sbc_data *sbc_data = (struct sbc_data *) codec_data;

	return sbc_data->in_buf_size;
}

static size_t sbc_get_mediapacket_duration(void *codec_data)
{
	struct sbc_data *sbc_data = (struct sbc_data *) codec_data;

	return sbc_data->frame_duration * sbc_data->frames_per_packet;
}

static ssize_t sbc_encode_mediapacket(void *codec_data, const uint8_t *buffer,
					size_t len, struct media_packet *mp,
					size_t mp_data_len, size_t *written)
{
	struct sbc_data *sbc_data = (struct sbc_data *) codec_data;
	struct media_packet_sbc *mp_sbc = (struct media_packet_sbc *) mp;
	size_t consumed = 0;
	size_t encoded = 0;
	uint8_t frame_count = 0;

	mp_data_len -= sizeof(mp_sbc->payload);

	while (len - consumed >= sbc_data->in_frame_len &&
			mp_data_len - encoded >= sbc_data->out_frame_len &&
			frame_count < sbc_data->frames_per_packet) {
		ssize_t read;
		ssize_t written = 0;

		read = sbc_encode(&sbc_data->enc, buffer + consumed,
				sbc_data->in_frame_len, mp_sbc->data + encoded,
				mp_data_len - encoded, &written);

		if (read < 0) {
			error("SBC: failed to encode block at frame %d (%zd)",
							frame_count, read);
			break;
		}

		frame_count++;
		consumed += read;
		encoded += written;
	}

	*written = encoded + sizeof(mp_sbc->payload);
	mp_sbc->payload.frame_count = frame_count;

	return consumed;
}

static bool sbc_update_qos(void *codec_data, uint8_t op)
{
	struct sbc_data *sbc_data = (struct sbc_data *) codec_data;
	uint8_t curr_bitpool = sbc_data->enc.bitpool;
	uint8_t new_bitpool = curr_bitpool;

	switch (op) {
	case QOS_POLICY_DEFAULT:
		new_bitpool = sbc_data->sbc.max_bitpool;
		break;

	case QOS_POLICY_DECREASE:
		if (curr_bitpool > SBC_QUALITY_MIN_BITPOOL) {
			new_bitpool = curr_bitpool - SBC_QUALITY_STEP;
			if (new_bitpool < SBC_QUALITY_MIN_BITPOOL)
				new_bitpool = SBC_QUALITY_MIN_BITPOOL;
		}
		break;
	}

	if (new_bitpool == curr_bitpool)
		return false;

	sbc_data->enc.bitpool = new_bitpool;

	sbc_codec_calculate(sbc_data);

	info("SBC: bitpool changed: %d -> %d", curr_bitpool, new_bitpool);

	return true;
}

static const struct audio_codec codec = {
	.type = A2DP_CODEC_SBC,
	.use_rtp = true,

	.get_presets = sbc_get_presets,

	.init = sbc_codec_init,
	.cleanup = sbc_cleanup,
	.get_config = sbc_get_config,
	.get_buffer_size = sbc_get_buffer_size,
	.get_mediapacket_duration = sbc_get_mediapacket_duration,
	.encode_mediapacket = sbc_encode_mediapacket,
	.update_qos = sbc_update_qos,
};

const struct audio_codec *codec_sbc(void)
{
	return &codec;
}
