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

#include <errno.h>
#include <pthread.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <hardware/audio.h>
#include <hardware/hardware.h>
#include <audio_utils/resampler.h>

#include "hal-utils.h"
#include "sco-msg.h"
#include "ipc-common.h"
#include "hal-log.h"
#include "hal.h"

#define AUDIO_STREAM_DEFAULT_RATE	44100
#define AUDIO_STREAM_SCO_RATE		8000
#define AUDIO_STREAM_DEFAULT_FORMAT	AUDIO_FORMAT_PCM_16_BIT

#define OUT_BUFFER_SIZE			2560
#define OUT_STREAM_FRAMES		2560
#define IN_STREAM_FRAMES		5292

#define SOCKET_POLL_TIMEOUT_MS		500

static int listen_sk = -1;
static int ipc_sk = -1;

static int sco_fd = -1;
static uint16_t sco_mtu = 0;
static pthread_mutex_t sco_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_t ipc_th = 0;
static pthread_mutex_t sk_mutex = PTHREAD_MUTEX_INITIALIZER;

static struct sco_stream_in *sco_stream_in = NULL;
static struct sco_stream_out *sco_stream_out = NULL;

struct sco_audio_config {
	uint32_t rate;
	uint32_t channels;
	uint32_t frame_num;
	audio_format_t format;
};

struct sco_stream_out {
	struct audio_stream_out stream;

	struct sco_audio_config cfg;

	uint8_t *downmix_buf;
	uint8_t *cache;
	size_t cache_len;

	size_t samples;
	struct timespec start;

	struct resampler_itfe *resampler;
	int16_t *resample_buf;
	uint32_t resample_frame_num;

	bt_bdaddr_t bd_addr;
};

static void sco_close_socket(void)
{
	DBG("sco fd %d", sco_fd);

	if (sco_fd < 0)
		return;

	shutdown(sco_fd, SHUT_RDWR);
	close(sco_fd);
	sco_fd = -1;
}

struct sco_stream_in {
	struct audio_stream_in stream;

	struct sco_audio_config cfg;

	struct resampler_itfe *resampler;
	int16_t *resample_buf;
	uint32_t resample_frame_num;

	bt_bdaddr_t bd_addr;
};

struct sco_dev {
	struct audio_hw_device dev;
	struct sco_stream_out *out;
	struct sco_stream_in *in;
};

/*
 * return the minimum frame numbers from resampling between BT stack's rate
 * and audio flinger's. For output stream, 'output' shall be true, otherwise
 * false for input streams at audio flinger side.
 */
static size_t get_resample_frame_num(uint32_t sco_rate, uint32_t rate,
						size_t frame_num, bool output)
{
	size_t resample_frames_num = frame_num * sco_rate / rate + output;

	DBG("resampler: sco_rate %d frame_num %zd rate %d resample frames %zd",
				sco_rate, frame_num, rate, resample_frames_num);

	return resample_frames_num;
}

/* SCO IPC functions */

static int sco_ipc_cmd(uint8_t service_id, uint8_t opcode, uint16_t len,
			void *param, size_t *rsp_len, void *rsp, int *fd)
{
	ssize_t ret;
	struct msghdr msg;
	struct iovec iv[2];
	struct ipc_hdr cmd;
	char cmsgbuf[CMSG_SPACE(sizeof(int))];
	struct ipc_status s;
	size_t s_len = sizeof(s);

	pthread_mutex_lock(&sk_mutex);

	if (ipc_sk < 0) {
		error("sco: Invalid cmd socket passed to sco_ipc_cmd");
		goto failed;
	}

	if (!rsp || !rsp_len) {
		memset(&s, 0, s_len);
		rsp_len = &s_len;
		rsp = &s;
	}

	memset(&msg, 0, sizeof(msg));
	memset(&cmd, 0, sizeof(cmd));

	cmd.service_id = service_id;
	cmd.opcode = opcode;
	cmd.len = len;

	iv[0].iov_base = &cmd;
	iv[0].iov_len = sizeof(cmd);

	iv[1].iov_base = param;
	iv[1].iov_len = len;

	msg.msg_iov = iv;
	msg.msg_iovlen = 2;

	ret = sendmsg(ipc_sk, &msg, 0);
	if (ret < 0) {
		error("sco: Sending command failed:%s", strerror(errno));
		goto failed;
	}

	/* socket was shutdown */
	if (ret == 0) {
		error("sco: Command socket closed");
		goto failed;
	}

	memset(&msg, 0, sizeof(msg));
	memset(&cmd, 0, sizeof(cmd));

	iv[0].iov_base = &cmd;
	iv[0].iov_len = sizeof(cmd);

	iv[1].iov_base = rsp;
	iv[1].iov_len = *rsp_len;

	msg.msg_iov = iv;
	msg.msg_iovlen = 2;

	if (fd) {
		memset(cmsgbuf, 0, sizeof(cmsgbuf));
		msg.msg_control = cmsgbuf;
		msg.msg_controllen = sizeof(cmsgbuf);
	}

	ret = recvmsg(ipc_sk, &msg, 0);
	if (ret < 0) {
		error("sco: Receiving command response failed:%s",
							strerror(errno));
		goto failed;
	}

	if (ret < (ssize_t) sizeof(cmd)) {
		error("sco: Too small response received(%zd bytes)", ret);
		goto failed;
	}

	if (cmd.service_id != service_id) {
		error("sco: Invalid service id (%u vs %u)", cmd.service_id,
								service_id);
		goto failed;
	}

	if (ret != (ssize_t) (sizeof(cmd) + cmd.len)) {
		error("sco: Malformed response received(%zd bytes)", ret);
		goto failed;
	}

	if (cmd.opcode != opcode && cmd.opcode != SCO_OP_STATUS) {
		error("sco: Invalid opcode received (%u vs %u)",
						cmd.opcode, opcode);
		goto failed;
	}

	if (cmd.opcode == SCO_OP_STATUS) {
		struct ipc_status *s = rsp;

		if (sizeof(*s) != cmd.len) {
			error("sco: Invalid status length");
			goto failed;
		}

		if (s->code == SCO_STATUS_SUCCESS) {
			error("sco: Invalid success status response");
			goto failed;
		}

		pthread_mutex_unlock(&sk_mutex);

		return s->code;
	}

	pthread_mutex_unlock(&sk_mutex);

	/* Receive auxiliary data in msg */
	if (fd) {
		struct cmsghdr *cmsg;

		*fd = -1;

		for (cmsg = CMSG_FIRSTHDR(&msg); cmsg;
					cmsg = CMSG_NXTHDR(&msg, cmsg)) {
			if (cmsg->cmsg_level == SOL_SOCKET
					&& cmsg->cmsg_type == SCM_RIGHTS) {
				memcpy(fd, CMSG_DATA(cmsg), sizeof(int));
				break;
			}
		}

		if (*fd < 0)
			goto failed;
	}

	*rsp_len = cmd.len;

	return SCO_STATUS_SUCCESS;

failed:
	/* Some serious issue happen on IPC - recover */
	shutdown(ipc_sk, SHUT_RDWR);
	pthread_mutex_unlock(&sk_mutex);

	return SCO_STATUS_FAILED;
}

static int ipc_get_sco_fd(bt_bdaddr_t *bd_addr)
{
	int ret = SCO_STATUS_SUCCESS;

	pthread_mutex_lock(&sco_mutex);

	if (sco_fd < 0) {
		struct sco_cmd_get_fd cmd;
		struct sco_rsp_get_fd rsp;
		size_t rsp_len = sizeof(rsp);

		DBG("Getting SCO fd");

		memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

		ret = sco_ipc_cmd(SCO_SERVICE_ID, SCO_OP_GET_FD, sizeof(cmd),
						&cmd, &rsp_len, &rsp, &sco_fd);

		/* Sometimes mtu returned is wrong */
		sco_mtu = /* rsp.mtu */ 48;
	}

	pthread_mutex_unlock(&sco_mutex);

	return ret;
}

/* Audio stream functions */

static void downmix_to_mono(struct sco_stream_out *out, const uint8_t *buffer,
							size_t frame_num)
{
	const int16_t *input = (const void *) buffer;
	int16_t *output = (void *) out->downmix_buf;
	size_t i;

	for (i = 0; i < frame_num; i++) {
		int16_t l = get_le16(&input[i * 2]);
		int16_t r = get_le16(&input[i * 2 + 1]);

		put_le16((l + r) / 2, &output[i]);
	}
}

static uint64_t timespec_diff_us(struct timespec *a, struct timespec *b)
{
	struct timespec res;

	res.tv_sec = a->tv_sec - b->tv_sec;
	res.tv_nsec = a->tv_nsec - b->tv_nsec;

	if (res.tv_nsec < 0) {
		res.tv_sec--;
		res.tv_nsec += 1000000000ll; /* 1sec */
	}

	return res.tv_sec * 1000000ll + res.tv_nsec / 1000ll;
}

static bool write_data(struct sco_stream_out *out, const uint8_t *buffer,
								size_t bytes)
{
	struct pollfd pfd;
	size_t len, written = 0;
	int ret;
	uint8_t *p;
	uint64_t audio_sent_us, audio_passed_us;

	pfd.fd = sco_fd;
	pfd.events = POLLOUT | POLLHUP | POLLNVAL;

	while (bytes > written) {
		struct timespec now;

		/* poll for sending */
		if (poll(&pfd, 1, SOCKET_POLL_TIMEOUT_MS) == 0) {
			DBG("timeout fd %d", sco_fd);
			return false;
		}

		if (pfd.revents & (POLLHUP | POLLNVAL)) {
			error("error fd %d, events 0x%x", sco_fd, pfd.revents);
			return false;
		}

		len = bytes - written > sco_mtu ? sco_mtu : bytes - written;

		clock_gettime(CLOCK_REALTIME, &now);
		/* Mark start of the stream */
		if (!out->samples)
			memcpy(&out->start, &now, sizeof(out->start));

		audio_sent_us = out->samples * 1000000ll / AUDIO_STREAM_SCO_RATE;
		audio_passed_us = timespec_diff_us(&now, &out->start);
		if ((int) (audio_sent_us - audio_passed_us) > 1500) {
			struct timespec timeout = {0,
						(audio_sent_us -
						audio_passed_us) * 1000};
			DBG("Sleeping for %d ms",
					(int) (audio_sent_us - audio_passed_us));
			nanosleep(&timeout, NULL);
		} else if ((int)(audio_passed_us - audio_sent_us) > 50000) {
			DBG("\n\nResync\n\n");
			out->samples = 0;
			memcpy(&out->start, &now, sizeof(out->start));
		}

		if (out->cache_len) {
			DBG("First packet cache_len %zd", out->cache_len);
			memcpy(out->cache + out->cache_len, buffer,
						sco_mtu - out->cache_len);
			p = out->cache;
		} else {
			if (bytes - written >= sco_mtu)
				p = (void *) buffer + written;
			else {
				memcpy(out->cache, buffer + written,
							bytes - written);
				out->cache_len = bytes - written;
				DBG("Last packet, cache %zd bytes",
							bytes - written);
				written += bytes - written;
				continue;
			}
		}

		ret = write(sco_fd, p, len);
		if (ret > 0) {
			if (out->cache_len) {
				written = sco_mtu - out->cache_len;
				out->cache_len = 0;
			} else
				written += ret;

			out->samples += ret / 2;

			DBG("written %d samples %zd total %zd bytes",
					ret, out->samples, written);
			continue;
		}

		if (errno == EAGAIN) {
			ret = errno;
			warn("write failed (%d)", ret);
			continue;
		}

		if (errno != EINTR) {
			ret = errno;
			error("write failed (%d) fd %d bytes %zd", ret, sco_fd,
									bytes);
			return false;
		}
	}

	DBG("written %zd bytes", bytes);

	return true;
}

static ssize_t out_write(struct audio_stream_out *stream, const void *buffer,
								size_t bytes)
{
	struct sco_stream_out *out = (struct sco_stream_out *) stream;
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	size_t frame_num = bytes / audio_stream_out_frame_size(stream);
#else
	size_t frame_num = bytes / audio_stream_frame_size(&out->stream.common);
#endif
	size_t output_frame_num = frame_num;
	void *send_buf = out->downmix_buf;
	size_t total;

	DBG("write to fd %d bytes %zu", sco_fd, bytes);

	if (ipc_get_sco_fd(&out->bd_addr) != SCO_STATUS_SUCCESS)
		return -1;

	if (!out->downmix_buf) {
		error("sco: downmix buffer not initialized");
		return -1;
	}

	downmix_to_mono(out, buffer, frame_num);

	if (out->resampler) {
		int ret;

		/* limit resampler's output within what resample buf can hold */
		output_frame_num = out->resample_frame_num;

		ret = out->resampler->resample_from_input(out->resampler,
							send_buf,
							&frame_num,
							out->resample_buf,
							&output_frame_num);
		if (ret) {
			error("Failed to resample frames: %zd input %zd (%s)",
				frame_num, output_frame_num, strerror(ret));
			return -1;
		}

		send_buf = out->resample_buf;

		DBG("Resampled: frame_num %zd, output_frame_num %zd",
						frame_num, output_frame_num);
	}

	total = output_frame_num * sizeof(int16_t) * 1;

	DBG("total %zd", total);

	if (!write_data(out, send_buf, total))
		return -1;

	return bytes;
}

static uint32_t out_get_sample_rate(const struct audio_stream *stream)
{
	struct sco_stream_out *out = (struct sco_stream_out *) stream;

	DBG("rate %u", out->cfg.rate);

	return out->cfg.rate;
}

static int out_set_sample_rate(struct audio_stream *stream, uint32_t rate)
{
	DBG("rate %u", rate);

	return 0;
}

static size_t out_get_buffer_size(const struct audio_stream *stream)
{
	struct sco_stream_out *out = (struct sco_stream_out *) stream;
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	size_t size = audio_stream_out_frame_size(&out->stream) *
							out->cfg.frame_num;
#else
	size_t size = audio_stream_frame_size(&out->stream.common) *
							out->cfg.frame_num;
#endif

	/* buffer size without resampling */
	if (out->cfg.rate == AUDIO_STREAM_SCO_RATE)
		size = 576 * 2;

	DBG("buf size %zd", size);

	return size;
}

static uint32_t out_get_channels(const struct audio_stream *stream)
{
	struct sco_stream_out *out = (struct sco_stream_out *) stream;

	DBG("channels num: %u", popcount(out->cfg.channels));

	return out->cfg.channels;
}

static audio_format_t out_get_format(const struct audio_stream *stream)
{
	struct sco_stream_out *out = (struct sco_stream_out *) stream;

	DBG("format: %u", out->cfg.format);

	return out->cfg.format;
}

static int out_set_format(struct audio_stream *stream, audio_format_t format)
{
	DBG("");

	return -ENOSYS;
}

static int out_standby(struct audio_stream *stream)
{
	DBG("");

	return 0;
}

static int out_dump(const struct audio_stream *stream, int fd)
{
	DBG("");

	return -ENOSYS;
}

static int out_set_parameters(struct audio_stream *stream, const char *kvpairs)
{
	DBG("%s", kvpairs);

	return 0;
}

static char *out_get_parameters(const struct audio_stream *stream,
							const char *keys)
{
	DBG("");

	return strdup("");
}

static uint32_t out_get_latency(const struct audio_stream_out *stream)
{
	DBG("");

	return 0;
}

static int out_set_volume(struct audio_stream_out *stream, float left,
								float right)
{
	DBG("");

	return -ENOSYS;
}

static int out_get_render_position(const struct audio_stream_out *stream,
							uint32_t *dsp_frames)
{
	DBG("");

	return -ENOSYS;
}

static int out_add_audio_effect(const struct audio_stream *stream,
							effect_handle_t effect)
{
	DBG("");

	return -ENOSYS;
}

static int out_remove_audio_effect(const struct audio_stream *stream,
							effect_handle_t effect)
{
	DBG("");

	return -ENOSYS;
}

static int sco_open_output_stream_real(struct audio_hw_device *dev,
					audio_io_handle_t handle,
					audio_devices_t devices,
					audio_output_flags_t flags,
					struct audio_config *config,
					struct audio_stream_out **stream_out,
					const char *address)
{
	struct sco_dev *adev = (struct sco_dev *) dev;
	struct sco_stream_out *out;
	int chan_num, ret;
	size_t resample_size;

	DBG("config %p device flags 0x%02x", config, devices);

	if (sco_stream_out) {
		DBG("stream_out already open");
		return -EIO;
	}

	out = calloc(1, sizeof(struct sco_stream_out));
	if (!out)
		return -ENOMEM;

	DBG("stream %p sco fd %d mtu %u", out, sco_fd, sco_mtu);

	out->stream.common.get_sample_rate = out_get_sample_rate;
	out->stream.common.set_sample_rate = out_set_sample_rate;
	out->stream.common.get_buffer_size = out_get_buffer_size;
	out->stream.common.get_channels = out_get_channels;
	out->stream.common.get_format = out_get_format;
	out->stream.common.set_format = out_set_format;
	out->stream.common.standby = out_standby;
	out->stream.common.dump = out_dump;
	out->stream.common.set_parameters = out_set_parameters;
	out->stream.common.get_parameters = out_get_parameters;
	out->stream.common.add_audio_effect = out_add_audio_effect;
	out->stream.common.remove_audio_effect = out_remove_audio_effect;
	out->stream.get_latency = out_get_latency;
	out->stream.set_volume = out_set_volume;
	out->stream.write = out_write;
	out->stream.get_render_position = out_get_render_position;

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	if (address) {
		DBG("address %s", address);

		str2bt_bdaddr_t(address, &out->bd_addr);
	}
#endif

	if (ipc_get_sco_fd(&out->bd_addr) != SCO_STATUS_SUCCESS)
		DBG("SCO is not connected yet; get fd on write()");

	if (config) {
		DBG("config: rate %u chan mask %x format %d offload %p",
				config->sample_rate, config->channel_mask,
				config->format, &config->offload_info);

		out->cfg.format = config->format;
		out->cfg.channels = config->channel_mask;
		out->cfg.rate = config->sample_rate;
	} else {
		out->cfg.format = AUDIO_STREAM_DEFAULT_FORMAT;
		out->cfg.channels = AUDIO_CHANNEL_OUT_STEREO;
		out->cfg.rate = AUDIO_STREAM_DEFAULT_RATE;
	}

	out->cfg.frame_num = OUT_STREAM_FRAMES;

	out->downmix_buf = malloc(out_get_buffer_size(&out->stream.common));
	if (!out->downmix_buf) {
		free(out);
		return -ENOMEM;
	}

	out->cache = malloc(sco_mtu);
	if (!out->cache) {
		free(out->downmix_buf);
		free(out);
		return -ENOMEM;
	}

	if (out->cfg.rate == AUDIO_STREAM_SCO_RATE)
		goto skip_resampler;

	/* Channel numbers for resampler */
	chan_num = 1;

	ret = create_resampler(out->cfg.rate, AUDIO_STREAM_SCO_RATE, chan_num,
						RESAMPLER_QUALITY_DEFAULT, NULL,
						&out->resampler);
	if (ret) {
		error("Failed to create resampler (%s)", strerror(-ret));
		goto failed;
	}

	out->resample_frame_num = get_resample_frame_num(AUDIO_STREAM_SCO_RATE,
							out->cfg.rate,
							out->cfg.frame_num, 1);

	if (!out->resample_frame_num) {
		error("frame num is too small to resample, discard it");
		goto failed;
	}

	resample_size = sizeof(int16_t) * chan_num * out->resample_frame_num;

	out->resample_buf = malloc(resample_size);
	if (!out->resample_buf) {
		error("failed to allocate resample buffer for %u frames",
						out->resample_frame_num);
		goto failed;
	}

	DBG("Resampler: input %d output %d chan %d frames %u size %zd",
				out->cfg.rate, AUDIO_STREAM_SCO_RATE, chan_num,
				out->resample_frame_num, resample_size);
skip_resampler:
	*stream_out = &out->stream;
	adev->out = out;
	sco_stream_out = out;

	return 0;
failed:
	if (out->resampler)
		release_resampler(out->resampler);

	free(out->cache);
	free(out->downmix_buf);
	free(out);
	*stream_out = NULL;
	adev->out = NULL;
	sco_stream_out = NULL;

	return ret;
}

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static int sco_open_output_stream(struct audio_hw_device *dev,
					audio_io_handle_t handle,
					audio_devices_t devices,
					audio_output_flags_t flags,
					struct audio_config *config,
					struct audio_stream_out **stream_out,
					const char *address)
{
	return  sco_open_output_stream_real(dev, handle, devices, flags,
						config, stream_out, address);
}
#else
static int sco_open_output_stream(struct audio_hw_device *dev,
					audio_io_handle_t handle,
					audio_devices_t devices,
					audio_output_flags_t flags,
					struct audio_config *config,
					struct audio_stream_out **stream_out)
{
	return sco_open_output_stream_real(dev, handle, devices, flags,
						config, stream_out, NULL);
}
#endif

static void sco_close_output_stream(struct audio_hw_device *dev,
					struct audio_stream_out *stream_out)
{
	struct sco_dev *sco_dev = (struct sco_dev *) dev;
	struct sco_stream_out *out = (struct sco_stream_out *) stream_out;

	DBG("dev %p stream %p fd %d", dev, out, sco_fd);

	if (out->resampler) {
		release_resampler(out->resampler);
		free(out->resample_buf);
	}

	free(out->cache);
	free(out->downmix_buf);
	free(out);
	sco_dev->out = NULL;

	pthread_mutex_lock(&sco_mutex);

	sco_stream_out = NULL;

	if (!sco_stream_in)
		sco_close_socket();

	pthread_mutex_unlock(&sco_mutex);
}

static int sco_set_parameters(struct audio_hw_device *dev,
							const char *kvpairs)
{
	DBG("%s", kvpairs);

	return 0;
}

static char *sco_get_parameters(const struct audio_hw_device *dev,
							const char *keys)
{
	DBG("");

	return strdup("");
}

static int sco_init_check(const struct audio_hw_device *dev)
{
	DBG("");

	return 0;
}

static int sco_set_voice_volume(struct audio_hw_device *dev, float volume)
{
	DBG("%f", volume);

	return 0;
}

static int sco_set_master_volume(struct audio_hw_device *dev, float volume)
{
	DBG("%f", volume);

	return 0;
}

static int sco_set_mode(struct audio_hw_device *dev, int mode)
{
	DBG("");

	return -ENOSYS;
}

static int sco_set_mic_mute(struct audio_hw_device *dev, bool state)
{
	DBG("");

	return -ENOSYS;
}

static int sco_get_mic_mute(const struct audio_hw_device *dev, bool *state)
{
	DBG("");

	return -ENOSYS;
}

static size_t sco_get_input_buffer_size(const struct audio_hw_device *dev,
					const struct audio_config *config)
{
	DBG("");

	return -ENOSYS;
}

static uint32_t in_get_sample_rate(const struct audio_stream *stream)
{
	struct sco_stream_in *in = (struct sco_stream_in *) stream;

	DBG("rate %u", in->cfg.rate);

	return in->cfg.rate;
}

static int in_set_sample_rate(struct audio_stream *stream, uint32_t rate)
{
	DBG("rate %u", rate);

	return 0;
}

static size_t in_get_buffer_size(const struct audio_stream *stream)
{
	struct sco_stream_in *in = (struct sco_stream_in *) stream;
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	size_t size = audio_stream_in_frame_size(&in->stream) *
							in->cfg.frame_num;
#else
	size_t size = audio_stream_frame_size(&in->stream.common) *
							in->cfg.frame_num;
#endif

	/* buffer size without resampling */
	if (in->cfg.rate == AUDIO_STREAM_SCO_RATE)
		size = 576;

	DBG("buf size %zd", size);

	return size;
}

static uint32_t in_get_channels(const struct audio_stream *stream)
{
	struct sco_stream_in *in = (struct sco_stream_in *) stream;

	DBG("channels num: %u", popcount(in->cfg.channels));

	return in->cfg.channels;
}

static audio_format_t in_get_format(const struct audio_stream *stream)
{
	struct sco_stream_in *in = (struct sco_stream_in *) stream;

	DBG("format: %u", in->cfg.format);

	return in->cfg.format;
}

static int in_set_format(struct audio_stream *stream, audio_format_t format)
{
	DBG("");

	return -ENOSYS;
}

static int in_standby(struct audio_stream *stream)
{
	DBG("");

	return 0;
}

static int in_dump(const struct audio_stream *stream, int fd)
{
	DBG("");

	return -ENOSYS;
}

static int in_set_parameters(struct audio_stream *stream, const char *kvpairs)
{
	DBG("%s", kvpairs);

	return 0;
}

static char *in_get_parameters(const struct audio_stream *stream,
							const char *keys)
{
	DBG("");

	return strdup("");
}

static int in_add_audio_effect(const struct audio_stream *stream,
							effect_handle_t effect)
{
	DBG("");

	return -ENOSYS;
}

static int in_remove_audio_effect(const struct audio_stream *stream,
							effect_handle_t effect)
{
	DBG("");

	return -ENOSYS;
}

static int in_set_gain(struct audio_stream_in *stream, float gain)
{
	DBG("");

	return -ENOSYS;
}

static bool read_data(struct sco_stream_in *in, char *buffer, size_t bytes)
{
	struct pollfd pfd;
	size_t len, read_bytes = 0;

	pfd.fd = sco_fd;
	pfd.events = POLLIN | POLLHUP | POLLNVAL;

	while (bytes > read_bytes) {
		int ret;

		/* poll for reading */
		if (poll(&pfd, 1, SOCKET_POLL_TIMEOUT_MS) == 0) {
			DBG("timeout fd %d", sco_fd);
			return false;
		}

		if (pfd.revents & (POLLHUP | POLLNVAL)) {
			error("error fd %d, events 0x%x", sco_fd, pfd.revents);
			return false;
		}

		len = bytes - read_bytes > sco_mtu ? sco_mtu :
							bytes - read_bytes;

		ret = read(sco_fd, buffer + read_bytes, len);
		if (ret > 0) {
			read_bytes += ret;
			DBG("read %d total %zd", ret, read_bytes);
			continue;
		}

		if (errno == EAGAIN) {
			ret = errno;
			warn("read failed (%d)", ret);
			continue;
		}

		if (errno != EINTR) {
			ret = errno;
			error("read failed (%d) fd %d bytes %zd", ret, sco_fd,
									bytes);
			return false;
		}
	}

	DBG("read %zd bytes", read_bytes);

	return true;
}

static ssize_t in_read(struct audio_stream_in *stream, void *buffer,
								size_t bytes)
{
	struct sco_stream_in *in = (struct sco_stream_in *) stream;
	size_t frame_size, frame_num, input_frame_num;
	void *read_buf = buffer;
	size_t total = bytes;
	int ret;

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	frame_size = audio_stream_in_frame_size(&in->stream);
#else
	frame_size = audio_stream_frame_size(&stream->common);
#endif

	if (!frame_size)
		return -1;

	frame_num = bytes / frame_size;
	input_frame_num = frame_num;

	DBG("Read from fd %d bytes %zu", sco_fd, bytes);

	if (ipc_get_sco_fd(&in->bd_addr) != SCO_STATUS_SUCCESS)
		return -1;

	if (!in->resampler && in->cfg.rate != AUDIO_STREAM_SCO_RATE) {
		error("Cannot find resampler");
		return -1;
	}

	if (in->resampler) {
		input_frame_num = get_resample_frame_num(AUDIO_STREAM_SCO_RATE,
							in->cfg.rate,
							frame_num, 0);
		if (input_frame_num > in->resample_frame_num) {
			DBG("resize input frames from %zd to %d",
				input_frame_num, in->resample_frame_num);
			input_frame_num = in->resample_frame_num;
		}

		read_buf = in->resample_buf;

		total = input_frame_num * sizeof(int16_t) * 1;
	}

	if(!read_data(in, read_buf, total))
		return -1;

	if (in->resampler) {
		ret = in->resampler->resample_from_input(in->resampler,
							in->resample_buf,
							&input_frame_num,
							(int16_t *) buffer,
							&frame_num);
		if (ret) {
			error("Failed to resample frames: %zd input %zd (%s)",
					frame_num, input_frame_num,
					strerror(ret));
			return -1;
		}

		DBG("resampler: remain %zd output %zd frames", input_frame_num,
								frame_num);
	}

	return bytes;
}

static uint32_t in_get_input_frames_lost(struct audio_stream_in *stream)
{
	DBG("");

	return -ENOSYS;
}

static int sco_open_input_stream_real(struct audio_hw_device *dev,
					audio_io_handle_t handle,
					audio_devices_t devices,
					struct audio_config *config,
					struct audio_stream_in **stream_in,
					audio_input_flags_t flags,
					const char *address,
					audio_source_t source)
{
	struct sco_dev *sco_dev = (struct sco_dev *) dev;
	struct sco_stream_in *in;
	int chan_num, ret;
	size_t resample_size;

	DBG("config %p device flags 0x%02x", config, devices);

	if (sco_stream_in) {
		DBG("stream_in already open");
		ret = -EIO;
		goto failed2;
	}

	in = calloc(1, sizeof(struct sco_stream_in));
	if (!in)
		return -ENOMEM;

	DBG("stream %p sco fd %d mtu %u", in, sco_fd, sco_mtu);

	in->stream.common.get_sample_rate = in_get_sample_rate;
	in->stream.common.set_sample_rate = in_set_sample_rate;
	in->stream.common.get_buffer_size = in_get_buffer_size;
	in->stream.common.get_channels = in_get_channels;
	in->stream.common.get_format = in_get_format;
	in->stream.common.set_format = in_set_format;
	in->stream.common.standby = in_standby;
	in->stream.common.dump = in_dump;
	in->stream.common.set_parameters = in_set_parameters;
	in->stream.common.get_parameters = in_get_parameters;
	in->stream.common.add_audio_effect = in_add_audio_effect;
	in->stream.common.remove_audio_effect = in_remove_audio_effect;
	in->stream.set_gain = in_set_gain;
	in->stream.read = in_read;
	in->stream.get_input_frames_lost = in_get_input_frames_lost;

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	if (address) {
		DBG("address %s", address);

		str2bt_bdaddr_t(address, &in->bd_addr);
	}
#endif

	if (config) {
		DBG("config: rate %u chan mask %x format %d offload %p",
				config->sample_rate, config->channel_mask,
				config->format, &config->offload_info);

		in->cfg.format = config->format;
		in->cfg.channels = config->channel_mask;
		in->cfg.rate = config->sample_rate;
	} else {
		in->cfg.format = AUDIO_STREAM_DEFAULT_FORMAT;
		in->cfg.channels = AUDIO_CHANNEL_OUT_MONO;
		in->cfg.rate = AUDIO_STREAM_DEFAULT_RATE;
	}

	in->cfg.frame_num = IN_STREAM_FRAMES;

	if (in->cfg.rate == AUDIO_STREAM_SCO_RATE)
		goto skip_resampler;

	/* Channel numbers for resampler */
	chan_num = 1;

	ret = create_resampler(AUDIO_STREAM_SCO_RATE, in->cfg.rate, chan_num,
						RESAMPLER_QUALITY_DEFAULT, NULL,
						&in->resampler);
	if (ret) {
		error("Failed to create resampler (%s)", strerror(-ret));
		goto failed;
	}

	in->resample_frame_num = get_resample_frame_num(AUDIO_STREAM_SCO_RATE,
							in->cfg.rate,
							in->cfg.frame_num, 0);

	resample_size = sizeof(int16_t) * chan_num * in->resample_frame_num;

	in->resample_buf = malloc(resample_size);
	if (!in->resample_buf) {
		error("failed to allocate resample buffer for %d frames",
							in->resample_frame_num);
		goto failed;
	}

	DBG("Resampler: input %d output %d chan %d frames %u size %zd",
				AUDIO_STREAM_SCO_RATE, in->cfg.rate, chan_num,
				in->resample_frame_num, resample_size);
skip_resampler:
	*stream_in = &in->stream;
	sco_dev->in = in;
	sco_stream_in = in;

	return 0;
failed:
	if (in->resampler)
		release_resampler(in->resampler);
	free(in);
failed2:
	*stream_in = NULL;
	sco_dev->in = NULL;
	sco_stream_in = NULL;

	return ret;
}

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static int sco_open_input_stream(struct audio_hw_device *dev,
					audio_io_handle_t handle,
					audio_devices_t devices,
					struct audio_config *config,
					struct audio_stream_in **stream_in,
					audio_input_flags_t flags,
					const char *address,
					audio_source_t source)
{
	return sco_open_input_stream_real(dev, handle, devices, config,
						stream_in, flags, address,
						source);
}
#else
static int sco_open_input_stream(struct audio_hw_device *dev,
					audio_io_handle_t handle,
					audio_devices_t devices,
					struct audio_config *config,
					struct audio_stream_in **stream_in)
{
	return sco_open_input_stream_real(dev, handle, devices, config,
						stream_in, 0, NULL, 0);
}
#endif

static void sco_close_input_stream(struct audio_hw_device *dev,
					struct audio_stream_in *stream_in)
{
	struct sco_dev *sco_dev = (struct sco_dev *) dev;
	struct sco_stream_in *in = (struct sco_stream_in *) stream_in;

	DBG("dev %p stream %p fd %d", dev, in, sco_fd);

	if (in->resampler) {
		release_resampler(in->resampler);
		free(in->resample_buf);
	}

	free(in);
	sco_dev->in = NULL;

	pthread_mutex_lock(&sco_mutex);

	sco_stream_in = NULL;

	if (!sco_stream_out)
		sco_close_socket();

	pthread_mutex_unlock(&sco_mutex);
}

static int sco_dump(const audio_hw_device_t *device, int fd)
{
	DBG("");

	return 0;
}

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static int set_master_mute(struct audio_hw_device *dev, bool mute)
{
	DBG("");
	return -ENOSYS;
}

static int get_master_mute(struct audio_hw_device *dev, bool *mute)
{
	DBG("");
	return -ENOSYS;
}

static int create_audio_patch(struct audio_hw_device *dev,
					unsigned int num_sources,
					const struct audio_port_config *sources,
					unsigned int num_sinks,
					const struct audio_port_config *sinks,
					audio_patch_handle_t *handle)
{
	DBG("");
	return -ENOSYS;
}

static int release_audio_patch(struct audio_hw_device *dev,
					audio_patch_handle_t handle)
{
	DBG("");
	return -ENOSYS;
}

static int get_audio_port(struct audio_hw_device *dev, struct audio_port *port)
{
	DBG("");
	return -ENOSYS;
}

static int set_audio_port_config(struct audio_hw_device *dev,
					const struct audio_port_config *config)
{
	DBG("");
	return -ENOSYS;
}
#endif

static int sco_close(hw_device_t *device)
{
	DBG("");

	free(device);

	return 0;
}

static void *ipc_handler(void *data)
{
	bool done = false;
	struct pollfd pfd;
	int sk;

	DBG("");

	while (!done) {
		DBG("Waiting for connection ...");

		sk = accept(listen_sk, NULL, NULL);
		if (sk < 0) {
			int err = errno;

			if (err == EINTR)
				continue;

			if (err != ECONNABORTED && err != EINVAL)
				error("sco: Failed to accept socket: %d (%s)",
							err, strerror(err));

			break;
		}

		pthread_mutex_lock(&sk_mutex);
		ipc_sk = sk;
		pthread_mutex_unlock(&sk_mutex);

		DBG("SCO IPC: Connected");

		memset(&pfd, 0, sizeof(pfd));
		pfd.fd = ipc_sk;
		pfd.events = POLLHUP | POLLERR | POLLNVAL;

		/* Check if socket is still alive. Empty while loop.*/
		while (poll(&pfd, 1, -1) < 0 && errno == EINTR);

		info("SCO HAL: Socket closed");

		pthread_mutex_lock(&sk_mutex);
		close(ipc_sk);
		ipc_sk = -1;
		pthread_mutex_unlock(&sk_mutex);
	}

	info("Closing SCO IPC thread");
	return NULL;
}

static int sco_ipc_init(void)
{
	struct sockaddr_un addr;
	int err;
	int sk;

	DBG("");

	sk = socket(PF_LOCAL, SOCK_SEQPACKET, 0);
	if (sk < 0) {
		err = -errno;
		error("sco: Failed to create socket: %d (%s)", -err,
								strerror(-err));
		return err;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;

	memcpy(addr.sun_path, BLUEZ_SCO_SK_PATH, sizeof(BLUEZ_SCO_SK_PATH));

	if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		err = -errno;
		error("sco: Failed to bind socket: %d (%s)", -err,
								strerror(-err));
		goto failed;
	}

	if (listen(sk, 1) < 0) {
		err = -errno;
		error("sco: Failed to listen on the socket: %d (%s)", -err,
								strerror(-err));
		goto failed;
	}

	listen_sk = sk;

	err = pthread_create(&ipc_th, NULL, ipc_handler, NULL);
	if (err) {
		err = -err;
		ipc_th = 0;
		error("sco: Failed to start IPC thread: %d (%s)",
							-err, strerror(-err));
		goto failed;
	}

	return 0;

failed:
	close(sk);
	return err;
}

static int sco_open(const hw_module_t *module, const char *name,
							hw_device_t **device)
{
	struct sco_dev *dev;
	int err;

	DBG("");

	if (strcmp(name, AUDIO_HARDWARE_INTERFACE)) {
		error("SCO: interface %s not matching [%s]", name,
						AUDIO_HARDWARE_INTERFACE);
		return -EINVAL;
	}

	err = sco_ipc_init();
	if (err < 0)
		return err;

	dev = calloc(1, sizeof(struct sco_dev));
	if (!dev)
		return -ENOMEM;

	dev->dev.common.tag = HARDWARE_DEVICE_TAG;
	dev->dev.common.version = AUDIO_DEVICE_API_VERSION_CURRENT;
	dev->dev.common.module = (struct hw_module_t *) module;
	dev->dev.common.close = sco_close;

	dev->dev.init_check = sco_init_check;
	dev->dev.set_voice_volume = sco_set_voice_volume;
	dev->dev.set_master_volume = sco_set_master_volume;
	dev->dev.set_mode = sco_set_mode;
	dev->dev.set_mic_mute = sco_set_mic_mute;
	dev->dev.get_mic_mute = sco_get_mic_mute;
	dev->dev.set_parameters = sco_set_parameters;
	dev->dev.get_parameters = sco_get_parameters;
	dev->dev.get_input_buffer_size = sco_get_input_buffer_size;
	dev->dev.open_output_stream = sco_open_output_stream;
	dev->dev.close_output_stream = sco_close_output_stream;
	dev->dev.open_input_stream = sco_open_input_stream;
	dev->dev.close_input_stream = sco_close_input_stream;
	dev->dev.dump = sco_dump;
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	dev->dev.set_master_mute = set_master_mute;
	dev->dev.get_master_mute = get_master_mute;
	dev->dev.create_audio_patch = create_audio_patch;
	dev->dev.release_audio_patch = release_audio_patch;
	dev->dev.get_audio_port = get_audio_port;
	dev->dev.set_audio_port_config = set_audio_port_config;
#endif

	*device = &dev->dev.common;

	return 0;
}

static struct hw_module_methods_t hal_module_methods = {
	.open = sco_open,
};

struct audio_module HAL_MODULE_INFO_SYM = {
	.common = {
		.tag = HARDWARE_MODULE_TAG,
		.version_major = 1,
		.version_minor = 0,
		.id = AUDIO_HARDWARE_MODULE_ID,
		.name = "SCO Audio HW HAL",
		.author = "Intel Corporation",
		.methods = &hal_module_methods,
	},
};
