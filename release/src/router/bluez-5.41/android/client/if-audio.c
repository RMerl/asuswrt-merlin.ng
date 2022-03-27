/*
 * Copyright (C) 2014 Intel Corporation
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

#include <pthread.h>
#include <unistd.h>
#include <math.h>

#include "if-main.h"
#include "../hal-utils.h"

audio_hw_device_t *if_audio = NULL;
static struct audio_stream_out *stream_out = NULL;

static size_t buffer_size = 0;
static pthread_t play_thread = 0;
static pthread_mutex_t outstream_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;

enum state {
	STATE_STOPPED,
	STATE_STOPPING,
	STATE_PLAYING,
	STATE_SUSPENDED,
	STATE_MAX
};

SINTMAP(audio_channel_mask_t, -1, "(AUDIO_CHANNEL_INVALID)")
	DELEMENT(AUDIO_CHANNEL_OUT_FRONT_LEFT),
	DELEMENT(AUDIO_CHANNEL_OUT_FRONT_RIGHT),
	DELEMENT(AUDIO_CHANNEL_OUT_FRONT_CENTER),
	DELEMENT(AUDIO_CHANNEL_OUT_LOW_FREQUENCY),
	DELEMENT(AUDIO_CHANNEL_OUT_BACK_LEFT),
	DELEMENT(AUDIO_CHANNEL_OUT_BACK_RIGHT),
	DELEMENT(AUDIO_CHANNEL_OUT_FRONT_LEFT_OF_CENTER),
	DELEMENT(AUDIO_CHANNEL_OUT_FRONT_RIGHT_OF_CENTER),
	DELEMENT(AUDIO_CHANNEL_OUT_BACK_CENTER),
	DELEMENT(AUDIO_CHANNEL_OUT_SIDE_LEFT),
	DELEMENT(AUDIO_CHANNEL_OUT_SIDE_RIGHT),
	DELEMENT(AUDIO_CHANNEL_OUT_TOP_CENTER),
	DELEMENT(AUDIO_CHANNEL_OUT_TOP_FRONT_LEFT),
	DELEMENT(AUDIO_CHANNEL_OUT_TOP_FRONT_CENTER),
	DELEMENT(AUDIO_CHANNEL_OUT_TOP_FRONT_RIGHT),
	DELEMENT(AUDIO_CHANNEL_OUT_TOP_BACK_LEFT),
	DELEMENT(AUDIO_CHANNEL_OUT_TOP_BACK_CENTER),
	DELEMENT(AUDIO_CHANNEL_OUT_TOP_BACK_RIGHT),
	DELEMENT(AUDIO_CHANNEL_OUT_MONO),
	DELEMENT(AUDIO_CHANNEL_OUT_STEREO),
	DELEMENT(AUDIO_CHANNEL_OUT_QUAD),
#if ANDROID_VERSION < PLATFORM_VER(5, 0, 0)
	DELEMENT(AUDIO_CHANNEL_OUT_SURROUND),
#else
	DELEMENT(AUDIO_CHANNEL_OUT_QUAD_BACK),
	DELEMENT(AUDIO_CHANNEL_OUT_QUAD_SIDE),
	DELEMENT(AUDIO_CHANNEL_OUT_5POINT1_BACK),
	DELEMENT(AUDIO_CHANNEL_OUT_5POINT1_SIDE),
#endif
	DELEMENT(AUDIO_CHANNEL_OUT_5POINT1),
	DELEMENT(AUDIO_CHANNEL_OUT_7POINT1),
	DELEMENT(AUDIO_CHANNEL_OUT_ALL),
	DELEMENT(AUDIO_CHANNEL_OUT_FRONT_LEFT),
	DELEMENT(AUDIO_CHANNEL_OUT_FRONT_LEFT),
	DELEMENT(AUDIO_CHANNEL_OUT_FRONT_LEFT),
	DELEMENT(AUDIO_CHANNEL_OUT_FRONT_LEFT),
	DELEMENT(AUDIO_CHANNEL_OUT_FRONT_LEFT),
ENDMAP

SINTMAP(audio_format_t, -1, "(AUDIO_FORMAT_INVALID)")
	DELEMENT(AUDIO_FORMAT_DEFAULT),
	DELEMENT(AUDIO_FORMAT_PCM),
	DELEMENT(AUDIO_FORMAT_MP3),
	DELEMENT(AUDIO_FORMAT_AMR_NB),
	DELEMENT(AUDIO_FORMAT_AMR_WB),
	DELEMENT(AUDIO_FORMAT_AAC),
	DELEMENT(AUDIO_FORMAT_HE_AAC_V1),
	DELEMENT(AUDIO_FORMAT_HE_AAC_V2),
	DELEMENT(AUDIO_FORMAT_VORBIS),
	DELEMENT(AUDIO_FORMAT_MAIN_MASK),
	DELEMENT(AUDIO_FORMAT_SUB_MASK),
	DELEMENT(AUDIO_FORMAT_PCM_16_BIT),
	DELEMENT(AUDIO_FORMAT_PCM_8_BIT),
	DELEMENT(AUDIO_FORMAT_PCM_32_BIT),
	DELEMENT(AUDIO_FORMAT_PCM_8_24_BIT),
ENDMAP

static int current_state = STATE_STOPPED;

#define SAMPLERATE 44100
static short sample[SAMPLERATE];
static uint16_t sample_pos;

static void init_p(int argc, const char **argv)
{
	int err;
	const hw_module_t *module;
	audio_hw_device_t *device;

	err = hw_get_module_by_class(AUDIO_HARDWARE_MODULE_ID,
					AUDIO_HARDWARE_MODULE_ID_A2DP, &module);
	if (err) {
		haltest_error("hw_get_module_by_class returned %d\n", err);
		return;
	}

	err = audio_hw_device_open(module, &device);
	if (err) {
		haltest_error("audio_hw_device_open returned %d\n", err);
		return;
	}

	if_audio = device;
}

static int feed_from_file(short *buffer, void *data)
{
	FILE *in = data;
	return fread(buffer, buffer_size, 1, in);
}

static int feed_from_generator(short *buffer, void *data)
{
	size_t i = 0;
	float volume = 0.5;
	float *freq = data;
	float f = 1;

	if (freq)
		f = *freq;

	/* buffer_size is in bytes but we are using buffer of shorts (2 bytes)*/
	for (i = 0; i < buffer_size / sizeof(*buffer) - 1;) {
		if (sample_pos >= SAMPLERATE)
			sample_pos = sample_pos % SAMPLERATE;

		/* Use the same sample for both channels */
		buffer[i++] = sample[sample_pos] * volume;
		buffer[i++] = sample[sample_pos] * volume;

		sample_pos += f;
	}

	return buffer_size;
}

static void prepare_sample(void)
{
	int x;
	double s;

	haltest_info("Preparing audio sample...\n");

	for (x = 0; x < SAMPLERATE; x++) {
		/* prepare sinusoidal 1Hz sample */
		s = (2.0 * 3.14159) * ((double)x / SAMPLERATE);
		s = sin(s);

		/* remap <-1, 1> to signed 16bit PCM range */
		sample[x] = s * 32767;
	}

	sample_pos = 0;
}

static void *playback_thread(void *data)
{
	int (*filbuff_cb) (short*, void*);
	short buffer[buffer_size / sizeof(short)];
	size_t len = 0;
	ssize_t w_len = 0;
	FILE *in = data;
	void *cb_data = NULL;
	float freq = 440.0;

	/* Use file or fall back to generator */
	if (in) {
		filbuff_cb = feed_from_file;
		cb_data = in;
	} else {
		prepare_sample();
		filbuff_cb = feed_from_generator;
		cb_data = &freq;
	}

	pthread_mutex_lock(&state_mutex);
	current_state = STATE_PLAYING;
	pthread_mutex_unlock(&state_mutex);

	do {
		pthread_mutex_lock(&state_mutex);

		if (current_state == STATE_STOPPING) {
			pthread_mutex_unlock(&state_mutex);
			break;
		} else if (current_state == STATE_SUSPENDED) {
			pthread_mutex_unlock(&state_mutex);
			usleep(500);
			continue;
		}

		pthread_mutex_unlock(&state_mutex);

		len = filbuff_cb(buffer, cb_data);

		pthread_mutex_lock(&outstream_mutex);
		if (!stream_out) {
			pthread_mutex_unlock(&outstream_mutex);
			break;
		}

		w_len = stream_out->write(stream_out, buffer, buffer_size);
		pthread_mutex_unlock(&outstream_mutex);
	} while (len && w_len > 0);

	if (in)
		fclose(in);

	pthread_mutex_lock(&state_mutex);
	current_state = STATE_STOPPED;
	pthread_mutex_unlock(&state_mutex);

	haltest_info("Done playing.\n");

	return NULL;
}

static void play_p(int argc, const char **argv)
{
	const char *fname = NULL;
	FILE *in = NULL;

	RETURN_IF_NULL(if_audio);
	RETURN_IF_NULL(stream_out);

	if (argc < 3) {
		haltest_error("Invalid audio file path.\n");
		haltest_info("Using sound generator.\n");
	} else {
		fname = argv[2];
		in = fopen(fname, "r");

		if (in == NULL) {
			haltest_error("Cannot open file: %s\n", fname);
			return;
		}
		haltest_info("Playing file: %s\n", fname);
	}

	if (buffer_size == 0) {
		haltest_error("Invalid buffer size. Was stream_out opened?\n");
		goto fail;
	}

	pthread_mutex_lock(&state_mutex);
	if (current_state != STATE_STOPPED) {
		haltest_error("Already playing or stream suspended!\n");
		pthread_mutex_unlock(&state_mutex);
		goto fail;
	}
	pthread_mutex_unlock(&state_mutex);

	if (pthread_create(&play_thread, NULL, playback_thread, in) != 0) {
		haltest_error("Cannot create playback thread!\n");
		goto fail;
	}

	return;
fail:
	if (in)
		fclose(in);
}

static void stop_p(int argc, const char **argv)
{
	pthread_mutex_lock(&state_mutex);
	if (current_state == STATE_STOPPED || current_state == STATE_STOPPING) {
		pthread_mutex_unlock(&state_mutex);
		return;
	}

	current_state = STATE_STOPPING;
	pthread_mutex_unlock(&state_mutex);

	pthread_mutex_lock(&outstream_mutex);
	stream_out->common.standby(&stream_out->common);
	pthread_mutex_unlock(&outstream_mutex);
}

static void open_output_stream_p(int argc, const char **argv)
{
	int err;

	RETURN_IF_NULL(if_audio);

	pthread_mutex_lock(&state_mutex);
	if (current_state == STATE_PLAYING) {
		haltest_error("Already playing!\n");
		pthread_mutex_unlock(&state_mutex);
		return;
	}
	pthread_mutex_unlock(&state_mutex);

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	err = if_audio->open_output_stream(if_audio,
						0,
						AUDIO_DEVICE_OUT_ALL_A2DP,
						AUDIO_OUTPUT_FLAG_NONE,
						NULL,
						&stream_out, NULL);
#else
	err = if_audio->open_output_stream(if_audio,
						0,
						AUDIO_DEVICE_OUT_ALL_A2DP,
						AUDIO_OUTPUT_FLAG_NONE,
						NULL,
						&stream_out);
#endif
	if (err < 0) {
		haltest_error("open output stream returned %d\n", err);
		return;
	}

	buffer_size = stream_out->common.get_buffer_size(&stream_out->common);
	if (buffer_size == 0)
		haltest_error("Invalid buffer size received!\n");
	else
		haltest_info("Using buffer size: %zu\n", buffer_size);
}

static void close_output_stream_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_audio);
	RETURN_IF_NULL(stream_out);

	stop_p(argc, argv);

	haltest_info("Waiting for playback thread...\n");
	pthread_join(play_thread, NULL);

	if_audio->close_output_stream(if_audio, stream_out);

	stream_out = NULL;
	buffer_size = 0;
}

static void cleanup_p(int argc, const char **argv)
{
	int err;

	RETURN_IF_NULL(if_audio);

	pthread_mutex_lock(&state_mutex);
	if (current_state != STATE_STOPPED) {
		pthread_mutex_unlock(&state_mutex);
		close_output_stream_p(0, NULL);
	} else {
		pthread_mutex_unlock(&state_mutex);
	}

	err = audio_hw_device_close(if_audio);
	if (err < 0) {
		haltest_error("audio_hw_device_close returned %d\n", err);
		return;
	}

	if_audio = NULL;
}

static void suspend_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_audio);
	RETURN_IF_NULL(stream_out);

	pthread_mutex_lock(&state_mutex);
	if (current_state != STATE_PLAYING) {
		pthread_mutex_unlock(&state_mutex);
		return;
	}
	current_state = STATE_SUSPENDED;
	pthread_mutex_unlock(&state_mutex);

	pthread_mutex_lock(&outstream_mutex);
	stream_out->common.standby(&stream_out->common);
	pthread_mutex_unlock(&outstream_mutex);
}

static void resume_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_audio);
	RETURN_IF_NULL(stream_out);

	pthread_mutex_lock(&state_mutex);
	if (current_state == STATE_SUSPENDED)
		current_state = STATE_PLAYING;
	pthread_mutex_unlock(&state_mutex);
}

static void get_latency_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_audio);
	RETURN_IF_NULL(stream_out);

	haltest_info("Output audio stream latency: %d\n",
					stream_out->get_latency(stream_out));
}

static void get_buffer_size_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_audio);
	RETURN_IF_NULL(stream_out);

	haltest_info("Current output buffer size: %zu\n",
		stream_out->common.get_buffer_size(&stream_out->common));
}

static void get_channels_p(int argc, const char **argv)
{
	audio_channel_mask_t channels;

	RETURN_IF_NULL(if_audio);
	RETURN_IF_NULL(stream_out);

	channels = stream_out->common.get_channels(&stream_out->common);

	haltest_info("Channels: %s\n", audio_channel_mask_t2str(channels));
}

static void get_format_p(int argc, const char **argv)
{
	audio_format_t format;

	RETURN_IF_NULL(if_audio);
	RETURN_IF_NULL(stream_out);

	format = stream_out->common.get_format(&stream_out->common);

	haltest_info("Format: %s\n", audio_format_t2str(format));
}

static void get_sample_rate_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_audio);
	RETURN_IF_NULL(stream_out);

	haltest_info("Current sample rate: %d\n",
		stream_out->common.get_sample_rate(&stream_out->common));
}

static void get_parameters_p(int argc, const char **argv)
{
	const char *keystr;

	RETURN_IF_NULL(if_audio);
	RETURN_IF_NULL(stream_out);

	if (argc < 3) {
		haltest_info("No keys given.\n");
		keystr = "";
	} else {
		keystr = argv[2];
	}

	haltest_info("Current parameters: %s\n",
			stream_out->common.get_parameters(&stream_out->common,
								keystr));
}

static void set_parameters_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_audio);
	RETURN_IF_NULL(stream_out);

	if (argc < 3) {
		haltest_error("No key=value; pairs given.\n");
		return;
	}

	stream_out->common.set_parameters(&stream_out->common, argv[2]);
}

static void set_sample_rate_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_audio);
	RETURN_IF_NULL(stream_out);

	if (argc < 3)
		return;

	stream_out->common.set_sample_rate(&stream_out->common, atoi(argv[2]));
}

static void init_check_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_audio);

	haltest_info("Init check result: %d\n", if_audio->init_check(if_audio));
}

static struct method methods[] = {
	STD_METHOD(init),
	STD_METHOD(cleanup),
	STD_METHOD(open_output_stream),
	STD_METHOD(close_output_stream),
	STD_METHODH(play, "<path to pcm file>"),
	STD_METHOD(stop),
	STD_METHOD(suspend),
	STD_METHOD(resume),
	STD_METHOD(get_latency),
	STD_METHOD(get_buffer_size),
	STD_METHOD(get_channels),
	STD_METHOD(get_format),
	STD_METHOD(get_sample_rate),
	STD_METHODH(get_parameters, "<A2dpSuspended;closing>"),
	STD_METHODH(set_parameters, "<A2dpSuspended=value;closing=value>"),
	STD_METHODH(set_sample_rate, "<sample rate>"),
	STD_METHOD(init_check),
	END_METHOD
};

const struct interface audio_if = {
	.name = "audio",
	.methods = methods
};
