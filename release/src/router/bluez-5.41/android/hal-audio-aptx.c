/*
 * Copyright (C) 2014 Tieto Poland
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

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <dlfcn.h>

#include "audio-msg.h"
#include "hal-audio.h"
#include "hal-log.h"
#include "profiles/audio/a2dp-codecs.h"

#define APTX_SO_NAME	"libbt-aptx.so"

struct aptx_data {
	a2dp_aptx_t aptx;

	void *enc;
};

static const a2dp_aptx_t aptx_presets[] = {
	{
		.info = {
			.vendor_id = APTX_VENDOR_ID,
			.codec_id = APTX_CODEC_ID,
		},
		.frequency = APTX_SAMPLING_FREQ_44100 |
						APTX_SAMPLING_FREQ_48000,
		.channel_mode = APTX_CHANNEL_MODE_STEREO,
	},
	{
		.info = {
			.vendor_id = APTX_VENDOR_ID,
			.codec_id = APTX_CODEC_ID,
		},
		.frequency = APTX_SAMPLING_FREQ_48000,
		.channel_mode = APTX_CHANNEL_MODE_STEREO,
	},
	{
		.info = {
			.vendor_id = APTX_VENDOR_ID,
			.codec_id = APTX_CODEC_ID,
		},
		.frequency = APTX_SAMPLING_FREQ_44100,
		.channel_mode = APTX_CHANNEL_MODE_STEREO,
	},
};

static void *aptx_handle;
static int aptx_btencsize;
static int (*aptx_init)(void *, short);
static int (*aptx_encode)(void *, void *, void *, void *);

static bool aptx_load(void)
{
	const char * (*aptx_version)(void);
	const char * (*aptx_build)(void);
	int (*aptx_sizeofenc)(void);

	aptx_handle = dlopen(APTX_SO_NAME, RTLD_LAZY);
	if (!aptx_handle) {
		error("APTX: failed to open library %s (%s)", APTX_SO_NAME,
								dlerror());
		return false;
	}

	aptx_version = dlsym(aptx_handle, "aptxbtenc_version");
	aptx_build = dlsym(aptx_handle, "aptxbtenc_build");

	if (aptx_version && aptx_build)
		info("APTX: using library version %s build %s", aptx_version(),
								aptx_build());
	else
		warn("APTX: cannot retrieve library version");

	aptx_sizeofenc = dlsym(aptx_handle, "SizeofAptxbtenc");
	aptx_init = dlsym(aptx_handle, "aptxbtenc_init");
	aptx_encode = dlsym(aptx_handle, "aptxbtenc_encodestereo");
	if (!aptx_sizeofenc || !aptx_init || !aptx_encode) {
		error("APTX: failed initialize library");
		dlclose(aptx_handle);
		aptx_handle = NULL;
		return false;
	}
	aptx_btencsize = aptx_sizeofenc();

	info("APTX: codec library initialized (size=%d)", aptx_btencsize);

	return true;
}

static void aptx_unload(void)
{
	if (!aptx_handle)
		return;

	dlclose(aptx_handle);
	aptx_handle = NULL;
}

static int aptx_get_presets(struct audio_preset *preset, size_t *len)
{
	int i;
	int count;
	size_t new_len = 0;
	uint8_t *ptr = (uint8_t *) preset;
	size_t preset_size = sizeof(*preset) + sizeof(a2dp_aptx_t);

	DBG("");

	count = sizeof(aptx_presets) / sizeof(aptx_presets[0]);

	for (i = 0; i < count; i++) {
		preset = (struct audio_preset *) ptr;

		if (new_len + preset_size > *len)
			break;

		preset->len = sizeof(a2dp_aptx_t);
		memcpy(preset->data, &aptx_presets[i], preset->len);

		new_len += preset_size;
		ptr += preset_size;
	}

	*len = new_len;

	return i;
}

static bool aptx_codec_init(struct audio_preset *preset, uint16_t payload_len,
							void **codec_data)
{
	struct aptx_data *aptx_data;

	DBG("");

	if (preset->len != sizeof(a2dp_aptx_t)) {
		error("APTX: preset size mismatch");
		return false;
	}

	aptx_data = malloc(sizeof(*aptx_data));
	if (!aptx_data)
		return false;

	memset(aptx_data, 0, sizeof(*aptx_data));
	memcpy(&aptx_data->aptx, preset->data, preset->len);

	aptx_data->enc = calloc(1, aptx_btencsize);
	if (!aptx_data->enc) {
		error("APTX: failed to create encoder");
		free(aptx_data);
		return false;
	}

	/* 1 = big-endian, this is what devices are using */
	aptx_init(aptx_data->enc, 1);

	*codec_data = aptx_data;

	return true;
}

static bool aptx_cleanup(void *codec_data)
{
	struct aptx_data *aptx_data = (struct aptx_data *) codec_data;

	free(aptx_data->enc);
	free(codec_data);

	return true;
}

static bool aptx_get_config(void *codec_data, struct audio_input_config *config)
{
	struct aptx_data *aptx_data = (struct aptx_data *) codec_data;

	config->rate = aptx_data->aptx.frequency & APTX_SAMPLING_FREQ_48000 ?
								48000 : 44100;
	config->channels = AUDIO_CHANNEL_OUT_STEREO;
	config->format = AUDIO_FORMAT_PCM_16_BIT;

	return true;
}

static size_t aptx_get_buffer_size(void *codec_data)
{
	/* TODO: return actual value */
	return 0;
}

static size_t aptx_get_mediapacket_duration(void *codec_data)
{
	/* TODO: return actual value */
	return 0;
}

static ssize_t aptx_encode_mediapacket(void *codec_data, const uint8_t *buffer,
					size_t len, struct media_packet *mp,
					size_t mp_data_len, size_t *written)
{
	struct aptx_data *aptx_data = (struct aptx_data *) codec_data;
	const int16_t *ptr = (const void *) buffer;
	size_t bytes_in = 0;
	size_t bytes_out = 0;

	while ((len - bytes_in) >= 16 && (mp_data_len - bytes_out) >= 4) {
		int pcm_l[4], pcm_r[4];
		int i;

		for (i = 0; i < 4; i++) {
			pcm_l[i] = ptr[0];
			pcm_r[i] = ptr[1];
			ptr += 2;
		}

		aptx_encode(aptx_data->enc, pcm_l, pcm_r, &mp->data[bytes_out]);

		bytes_in += 16;
		bytes_out += 4;
	}

	*written = bytes_out;

	return bytes_in;
}

static bool aptx_update_qos(void *codec_data, uint8_t op)
{
	/*
	 * aptX has constant bitrate of 352kbps (with constant 4:1 compression
	 * ratio) thus QoS is not possible here.
	 */

	return false;
}

static const struct audio_codec codec = {
	.type = A2DP_CODEC_VENDOR,
	.use_rtp = false,

	.load = aptx_load,
	.unload = aptx_unload,

	.get_presets = aptx_get_presets,

	.init = aptx_codec_init,
	.cleanup = aptx_cleanup,
	.get_config = aptx_get_config,
	.get_buffer_size = aptx_get_buffer_size,
	.get_mediapacket_duration = aptx_get_mediapacket_duration,
	.encode_mediapacket = aptx_encode_mediapacket,
	.update_qos = aptx_update_qos,
};

const struct audio_codec *codec_aptx(void)
{
	return &codec;
}
