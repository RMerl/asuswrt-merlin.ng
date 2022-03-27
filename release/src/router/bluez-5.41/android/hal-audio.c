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
#include <arpa/inet.h>
#include <fcntl.h>

#include <hardware/audio.h>
#include <hardware/hardware.h>

#include "audio-msg.h"
#include "ipc-common.h"
#include "hal-log.h"
#include "hal-msg.h"
#include "hal-audio.h"
#include "hal-utils.h"
#include "hal.h"

#define FIXED_A2DP_PLAYBACK_LATENCY_MS 25

#define FIXED_BUFFER_SIZE (20 * 512)

#define MAX_DELAY	100000 /* 100ms */

static const uint8_t a2dp_src_uuid[] = {
		0x00, 0x00, 0x11, 0x0a, 0x00, 0x00, 0x10, 0x00,
		0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb };

static int listen_sk = -1;
static int audio_sk = -1;

static pthread_t ipc_th = 0;
static pthread_mutex_t sk_mutex = PTHREAD_MUTEX_INITIALIZER;

static void timespec_add(struct timespec *base, uint64_t time_us,
							struct timespec *res)
{
	res->tv_sec = base->tv_sec + time_us / 1000000;
	res->tv_nsec = base->tv_nsec + (time_us % 1000000) * 1000;

	if (res->tv_nsec >= 1000000000) {
		res->tv_sec++;
		res->tv_nsec -= 1000000000;
	}
}

static void timespec_diff(struct timespec *a, struct timespec *b,
							struct timespec *res)
{
	res->tv_sec = a->tv_sec - b->tv_sec;
	res->tv_nsec = a->tv_nsec - b->tv_nsec;

	if (res->tv_nsec < 0) {
		res->tv_sec--;
		res->tv_nsec += 1000000000; /* 1sec */
	}
}

static uint64_t timespec_diff_us(struct timespec *a, struct timespec *b)
{
	struct timespec res;

	timespec_diff(a, b, &res);

	return res.tv_sec * 1000000ll + res.tv_nsec / 1000ll;
}

#if defined(ANDROID)
/*
 * Bionic does not have clock_nanosleep() prototype in time.h even though
 * it provides its implementation.
 */
extern int clock_nanosleep(clockid_t clock_id, int flags,
					const struct timespec *request,
					struct timespec *remain);
#endif

static struct {
	const audio_codec_get_t get_codec;
	bool loaded;
} audio_codecs[] = {
		{ .get_codec = codec_aptx, .loaded = false },
		{ .get_codec = codec_sbc, .loaded = false },
};

#define NUM_CODECS (sizeof(audio_codecs) / sizeof(audio_codecs[0]))

#define MAX_AUDIO_ENDPOINTS NUM_CODECS

struct audio_endpoint {
	uint8_t id;
	const struct audio_codec *codec;
	void *codec_data;
	int fd;

	struct media_packet *mp;
	size_t mp_data_len;

	uint16_t seq;
	uint32_t samples;
	struct timespec start;

	bool resync;
};

static struct audio_endpoint audio_endpoints[MAX_AUDIO_ENDPOINTS];

enum a2dp_state_t {
	AUDIO_A2DP_STATE_NONE,
	AUDIO_A2DP_STATE_STANDBY,
	AUDIO_A2DP_STATE_SUSPENDED,
	AUDIO_A2DP_STATE_STARTED
};

struct a2dp_stream_out {
	struct audio_stream_out stream;

	struct audio_endpoint *ep;
	enum a2dp_state_t audio_state;
	struct audio_input_config cfg;

	uint8_t *downmix_buf;
};

struct a2dp_audio_dev {
	struct audio_hw_device dev;
	struct a2dp_stream_out *out;
};

static int audio_ipc_cmd(uint8_t service_id, uint8_t opcode, uint16_t len,
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

	if (audio_sk < 0) {
		error("audio: Invalid cmd socket passed to audio_ipc_cmd");
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

	ret = sendmsg(audio_sk, &msg, 0);
	if (ret < 0) {
		error("audio: Sending command failed:%s", strerror(errno));
		goto failed;
	}

	/* socket was shutdown */
	if (ret == 0) {
		error("audio: Command socket closed");
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

	ret = recvmsg(audio_sk, &msg, 0);
	if (ret < 0) {
		error("audio: Receiving command response failed:%s",
							strerror(errno));
		goto failed;
	}

	if (ret < (ssize_t) sizeof(cmd)) {
		error("audio: Too small response received(%zd bytes)", ret);
		goto failed;
	}

	if (cmd.service_id != service_id) {
		error("audio: Invalid service id (%u vs %u)", cmd.service_id,
								service_id);
		goto failed;
	}

	if (ret != (ssize_t) (sizeof(cmd) + cmd.len)) {
		error("audio: Malformed response received(%zd bytes)", ret);
		goto failed;
	}

	if (cmd.opcode != opcode && cmd.opcode != AUDIO_OP_STATUS) {
		error("audio: Invalid opcode received (%u vs %u)",
						cmd.opcode, opcode);
		goto failed;
	}

	if (cmd.opcode == AUDIO_OP_STATUS) {
		struct ipc_status *s = rsp;

		if (sizeof(*s) != cmd.len) {
			error("audio: Invalid status length");
			goto failed;
		}

		if (s->code == AUDIO_STATUS_SUCCESS) {
			error("audio: Invalid success status response");
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

	return AUDIO_STATUS_SUCCESS;

failed:
	/* Some serious issue happen on IPC - recover */
	shutdown(audio_sk, SHUT_RDWR);
	pthread_mutex_unlock(&sk_mutex);

	return AUDIO_STATUS_FAILED;
}

static int ipc_open_cmd(const struct audio_codec *codec)
{
	uint8_t buf[BLUEZ_AUDIO_MTU];
	struct audio_cmd_open *cmd = (struct audio_cmd_open *) buf;
	struct audio_rsp_open rsp;
	size_t cmd_len = sizeof(buf) - sizeof(*cmd);
	size_t rsp_len = sizeof(rsp);
	int result;

	DBG("");

	memcpy(cmd->uuid, a2dp_src_uuid, sizeof(a2dp_src_uuid));

	cmd->codec = codec->type;
	cmd->presets = codec->get_presets(cmd->preset, &cmd_len);

	cmd_len += sizeof(*cmd);

	result = audio_ipc_cmd(AUDIO_SERVICE_ID, AUDIO_OP_OPEN, cmd_len, cmd,
				&rsp_len, &rsp, NULL);

	if (result != AUDIO_STATUS_SUCCESS)
		return 0;

	return rsp.id;
}

static int ipc_close_cmd(uint8_t endpoint_id)
{
	struct audio_cmd_close cmd;
	int result;

	DBG("");

	cmd.id = endpoint_id;

	result = audio_ipc_cmd(AUDIO_SERVICE_ID, AUDIO_OP_CLOSE,
				sizeof(cmd), &cmd, NULL, NULL, NULL);

	return result;
}

static int ipc_open_stream_cmd(uint8_t *endpoint_id, uint16_t *mtu, int *fd,
						struct audio_preset **caps)
{
	char buf[BLUEZ_AUDIO_MTU];
	struct audio_cmd_open_stream cmd;
	struct audio_rsp_open_stream *rsp =
					(struct audio_rsp_open_stream *) &buf;
	size_t rsp_len = sizeof(buf);
	int result;

	DBG("");

	if (!caps)
		return AUDIO_STATUS_FAILED;

	cmd.id = *endpoint_id;

	result = audio_ipc_cmd(AUDIO_SERVICE_ID, AUDIO_OP_OPEN_STREAM,
				sizeof(cmd), &cmd, &rsp_len, rsp, fd);
	if (result == AUDIO_STATUS_SUCCESS) {
		size_t buf_len = sizeof(struct audio_preset) +
					rsp->preset[0].len;
		*endpoint_id = rsp->id;
		*mtu = rsp->mtu;
		*caps = malloc(buf_len);
		memcpy(*caps, &rsp->preset, buf_len);
	} else {
		*caps = NULL;
	}

	return result;
}

static int ipc_close_stream_cmd(uint8_t endpoint_id)
{
	struct audio_cmd_close_stream cmd;
	int result;

	DBG("");

	cmd.id = endpoint_id;

	result = audio_ipc_cmd(AUDIO_SERVICE_ID, AUDIO_OP_CLOSE_STREAM,
					sizeof(cmd), &cmd, NULL, NULL, NULL);

	return result;
}

static int ipc_resume_stream_cmd(uint8_t endpoint_id)
{
	struct audio_cmd_resume_stream cmd;
	int result;

	DBG("");

	cmd.id = endpoint_id;

	result = audio_ipc_cmd(AUDIO_SERVICE_ID, AUDIO_OP_RESUME_STREAM,
					sizeof(cmd), &cmd, NULL, NULL, NULL);

	return result;
}

static int ipc_suspend_stream_cmd(uint8_t endpoint_id)
{
	struct audio_cmd_suspend_stream cmd;
	int result;

	DBG("");

	cmd.id = endpoint_id;

	result = audio_ipc_cmd(AUDIO_SERVICE_ID, AUDIO_OP_SUSPEND_STREAM,
					sizeof(cmd), &cmd, NULL, NULL, NULL);

	return result;
}

struct register_state {
	struct audio_endpoint *ep;
	bool error;
};

static void register_endpoint(const struct audio_codec *codec,
						struct register_state *state)
{
	struct audio_endpoint *ep = state->ep;

	/* don't even try to register more endpoints if one failed */
	if (state->error)
		return;

	ep->id = ipc_open_cmd(codec);

	if (!ep->id) {
		state->error = true;
		error("Failed to register endpoint");
		return;
	}

	ep->codec = codec;
	ep->codec_data = NULL;
	ep->fd = -1;

	state->ep++;
}

static int register_endpoints(void)
{
	struct register_state state;
	unsigned int i;

	state.ep = &audio_endpoints[0];
	state.error = false;

	for (i = 0; i < NUM_CODECS; i++) {
		const struct audio_codec *codec = audio_codecs[i].get_codec();

		if (!audio_codecs[i].loaded)
			continue;

		register_endpoint(codec, &state);
	}

	return state.error ? AUDIO_STATUS_FAILED : AUDIO_STATUS_SUCCESS;
}

static void unregister_endpoints(void)
{
	size_t i;

	for (i = 0; i < MAX_AUDIO_ENDPOINTS; i++) {
		struct audio_endpoint *ep = &audio_endpoints[i];

		if (ep->id) {
			ipc_close_cmd(ep->id);
			memset(ep, 0, sizeof(*ep));
		}
	}
}

static bool open_endpoint(struct audio_endpoint **epp,
						struct audio_input_config *cfg)
{
	struct audio_preset *preset;
	struct audio_endpoint *ep = *epp;
	const struct audio_codec *codec;
	uint16_t mtu;
	uint16_t payload_len;
	int fd;
	size_t i;
	uint8_t ep_id = 0;

	if (ep)
		ep_id = ep->id;

	if (ipc_open_stream_cmd(&ep_id, &mtu, &fd, &preset) !=
							AUDIO_STATUS_SUCCESS)
		return false;

	DBG("ep_id=%d mtu=%u", ep_id, mtu);

	for (i = 0; i < MAX_AUDIO_ENDPOINTS; i++)
		if (audio_endpoints[i].id == ep_id) {
			ep = &audio_endpoints[i];
			break;
		}

	if (!ep) {
		error("Cound not find opened endpoint");
		goto failed;
	}

	*epp = ep;

	payload_len = mtu;
	if (ep->codec->use_rtp)
		payload_len -= sizeof(struct rtp_header);

	ep->fd = fd;

	codec = ep->codec;
	codec->init(preset, payload_len, &ep->codec_data);
	codec->get_config(ep->codec_data, cfg);

	ep->mp = calloc(mtu, 1);
	if (!ep->mp)
		goto failed;

	if (ep->codec->use_rtp) {
		struct media_packet_rtp *mp_rtp =
					(struct media_packet_rtp *) ep->mp;
		mp_rtp->hdr.v = 2;
		mp_rtp->hdr.pt = 0x60;
		mp_rtp->hdr.ssrc = htonl(1);
	}

	ep->mp_data_len = payload_len;

	free(preset);

	return true;

failed:
	close(fd);
	free(preset);

	return false;
}

static void close_endpoint(struct audio_endpoint *ep)
{
	ipc_close_stream_cmd(ep->id);
	if (ep->fd >= 0) {
		close(ep->fd);
		ep->fd = -1;
	}

	free(ep->mp);

	ep->codec->cleanup(ep->codec_data);
	ep->codec_data = NULL;
}

static bool resume_endpoint(struct audio_endpoint *ep)
{
	if (ipc_resume_stream_cmd(ep->id) != AUDIO_STATUS_SUCCESS)
		return false;

	ep->samples = 0;
	ep->resync = false;

	ep->codec->update_qos(ep->codec_data, QOS_POLICY_DEFAULT);

	return true;
}

static void downmix_to_mono(struct a2dp_stream_out *out, const uint8_t *buffer,
								size_t bytes)
{
	const int16_t *input = (const void *) buffer;
	int16_t *output = (void *) out->downmix_buf;
	size_t i, frames;

	/* PCM 16bit stereo */
	frames = bytes / (2 * sizeof(int16_t));

	for (i = 0; i < frames; i++) {
		int16_t l = get_le16(&input[i * 2]);
		int16_t r = get_le16(&input[i * 2 + 1]);

		put_le16((l + r) / 2, &output[i]);
	}
}

static bool wait_for_endpoint(struct audio_endpoint *ep, bool *writable)
{
	int ret;

	while (true) {
		struct pollfd pollfd;

		pollfd.fd = ep->fd;
		pollfd.events = POLLOUT;
		pollfd.revents = 0;

		ret = poll(&pollfd, 1, 500);

		if (ret >= 0) {
			*writable = !!(pollfd.revents & POLLOUT);
			break;
		}

		if (errno != EINTR) {
			ret = errno;
			error("poll failed (%d)", ret);
			return false;
		}
	}

	return true;
}

static bool write_to_endpoint(struct audio_endpoint *ep, size_t bytes)
{
	struct media_packet *mp = (struct media_packet *) ep->mp;
	int ret;

	while (true) {
		ret = write(ep->fd, mp, bytes);

		if (ret >= 0)
			break;

		/*
		 * this should not happen so let's issue warning, but do not
		 * fail, we can try to write next packet
		 */
		if (errno == EAGAIN) {
			ret = errno;
			warn("write failed (%d)", ret);
			break;
		}

		if (errno != EINTR) {
			ret = errno;
			error("write failed (%d)", ret);
			return false;
		}
	}

	return true;
}

static bool write_data(struct a2dp_stream_out *out, const void *buffer,
								size_t bytes)
{
	struct audio_endpoint *ep = out->ep;
	struct media_packet *mp = (struct media_packet *) ep->mp;
	struct media_packet_rtp *mp_rtp = (struct media_packet_rtp *) ep->mp;
	size_t free_space = ep->mp_data_len;
	size_t consumed = 0;

	while (consumed < bytes) {
		size_t written = 0;
		ssize_t read;
		uint32_t samples;
		int ret;
		struct timespec current;
		uint64_t audio_sent, audio_passed;
		bool do_write = false;

		/*
		 * prepare media packet in advance so we don't waste time after
		 * wakeup
		 */
		if (ep->codec->use_rtp) {
			mp_rtp->hdr.sequence_number = htons(ep->seq++);
			mp_rtp->hdr.timestamp = htonl(ep->samples);
		}
		read = ep->codec->encode_mediapacket(ep->codec_data,
						buffer + consumed,
						bytes - consumed, mp,
						free_space, &written);

		/*
		 * not much we can do here, let's just ignore remaining
		 * data and continue
		 */
		if (read <= 0)
			return true;

		/* calculate where are we and where we should be */
		clock_gettime(CLOCK_MONOTONIC, &current);
		if (!ep->samples)
			memcpy(&ep->start, &current, sizeof(ep->start));
		audio_sent = ep->samples * 1000000ll / out->cfg.rate;
		audio_passed = timespec_diff_us(&current, &ep->start);

		/*
		 * if we're ahead of stream then wait for next write point,
		 * if we're lagging more than 100ms then stop writing and just
		 * skip data until we're back in sync
		 */
		if (audio_sent > audio_passed) {
			struct timespec anchor;

			ep->resync = false;

			timespec_add(&ep->start, audio_sent, &anchor);

			while (true) {
				ret = clock_nanosleep(CLOCK_MONOTONIC,
							TIMER_ABSTIME, &anchor,
							NULL);

				if (!ret)
					break;

				if (ret != EINTR) {
					error("clock_nanosleep failed (%d)",
									ret);
					return false;
				}
			}
		} else if (!ep->resync) {
			uint64_t diff = audio_passed - audio_sent;

			if (diff > MAX_DELAY) {
				warn("lag is %jums, resyncing", diff / 1000);

				ep->codec->update_qos(ep->codec_data,
							QOS_POLICY_DECREASE);
				ep->resync = true;
			}
		}

		/* we send data only in case codec encoded some data, i.e. some
		 * codecs do internal buffering and output data only if full
		 * frame can be encoded
		 * in resync mode we'll just drop mediapackets
		 */
		if (written > 0 && !ep->resync) {
			/* wait some time for socket to be ready for write,
			 * but we'll just skip writing data if timeout occurs
			 */
			if (!wait_for_endpoint(ep, &do_write))
				return false;

			if (do_write) {
				if (ep->codec->use_rtp)
					written += sizeof(struct rtp_header);

				if (!write_to_endpoint(ep, written))
					return false;
			}
		}

		/*
		 * AudioFlinger provides 16bit PCM, so sample size is 2 bytes
		 * multiplied by number of channels. Number of channels is
		 * simply number of bits set in channels mask.
		 */
		samples = read / (2 * popcount(out->cfg.channels));
		ep->samples += samples;
		consumed += read;
	}

	return true;
}

static ssize_t out_write(struct audio_stream_out *stream, const void *buffer,
								size_t bytes)
{
	struct a2dp_stream_out *out = (struct a2dp_stream_out *) stream;
	const void *in_buf = buffer;
	size_t in_len = bytes;

	/* just return in case we're closing */
	if (out->audio_state == AUDIO_A2DP_STATE_NONE)
		return -1;

	/* We can auto-start only from standby */
	if (out->audio_state == AUDIO_A2DP_STATE_STANDBY) {
		DBG("stream in standby, auto-start");

		if (!resume_endpoint(out->ep))
			return -1;

		out->audio_state = AUDIO_A2DP_STATE_STARTED;
	}

	if (out->audio_state != AUDIO_A2DP_STATE_STARTED) {
		error("audio: stream not started");
		return -1;
	}

	if (out->ep->fd < 0) {
		error("audio: no transport socket");
		return -1;
	}

	/*
	 * currently Android audioflinger is not able to provide mono stream on
	 * A2DP output so down mixing needs to be done in hal-audio plugin.
	 *
	 * for reference see
	 * AudioFlinger::PlaybackThread::readOutputParameters()
	 * frameworks/av/services/audioflinger/Threads.cpp:1631
	 */
	if (out->cfg.channels == AUDIO_CHANNEL_OUT_MONO) {
		if (!out->downmix_buf) {
			error("audio: downmix buffer not initialized");
			return -1;
		}

		downmix_to_mono(out, buffer, bytes);

		in_buf = out->downmix_buf;
		in_len = bytes / 2;
	}

	if (!write_data(out, in_buf, in_len))
		return -1;

	return bytes;
}

static uint32_t out_get_sample_rate(const struct audio_stream *stream)
{
	struct a2dp_stream_out *out = (struct a2dp_stream_out *) stream;

	DBG("");

	return out->cfg.rate;
}

static int out_set_sample_rate(struct audio_stream *stream, uint32_t rate)
{
	struct a2dp_stream_out *out = (struct a2dp_stream_out *) stream;

	DBG("");

	if (rate != out->cfg.rate) {
		warn("audio: cannot set sample rate to %d", rate);
		return -1;
	}

	return 0;
}

static size_t out_get_buffer_size(const struct audio_stream *stream)
{
	DBG("");

	/*
	 * We should return proper buffer size calculated by codec (so each
	 * input buffer is encoded into single media packed) but this does not
	 * work well with AudioFlinger and causes problems. For this reason we
	 * use magic value here and out_write code takes care of splitting
	 * input buffer into multiple media packets.
	 */
	return FIXED_BUFFER_SIZE;
}

static uint32_t out_get_channels(const struct audio_stream *stream)
{
	DBG("");

	/*
	 * AudioFlinger can only provide stereo stream, so we return it here and
	 * later we'll downmix this to mono in case codec requires it
	 */

	return AUDIO_CHANNEL_OUT_STEREO;
}

static audio_format_t out_get_format(const struct audio_stream *stream)
{
	struct a2dp_stream_out *out = (struct a2dp_stream_out *) stream;

	DBG("");

	return out->cfg.format;
}

static int out_set_format(struct audio_stream *stream, audio_format_t format)
{
	DBG("");
	return -ENOSYS;
}

static int out_standby(struct audio_stream *stream)
{
	struct a2dp_stream_out *out = (struct a2dp_stream_out *) stream;

	DBG("");

	if (out->audio_state == AUDIO_A2DP_STATE_STARTED) {
		if (ipc_suspend_stream_cmd(out->ep->id) != AUDIO_STATUS_SUCCESS)
			return -1;
		out->audio_state = AUDIO_A2DP_STATE_STANDBY;
	}

	return 0;
}

static int out_dump(const struct audio_stream *stream, int fd)
{
	DBG("");
	return -ENOSYS;
}

static int out_set_parameters(struct audio_stream *stream, const char *kvpairs)
{
	struct a2dp_stream_out *out = (struct a2dp_stream_out *) stream;
	char *kvpair;
	char *str;
	char *saveptr;
	bool enter_suspend = false;
	bool exit_suspend = false;

	DBG("%s", kvpairs);

	str = strdup(kvpairs);
	if (!str)
		return -ENOMEM;

	kvpair = strtok_r(str, ";", &saveptr);

	for (; kvpair && *kvpair; kvpair = strtok_r(NULL, ";", &saveptr)) {
		char *keyval;

		keyval = strchr(kvpair, '=');
		if (!keyval)
			continue;

		*keyval = '\0';
		keyval++;

		if (!strcmp(kvpair, "closing")) {
			if (!strcmp(keyval, "true"))
				out->audio_state = AUDIO_A2DP_STATE_NONE;
		} else if (!strcmp(kvpair, "A2dpSuspended")) {
			if (!strcmp(keyval, "true"))
				enter_suspend = true;
			else
				exit_suspend = true;
		}
	}

	free(str);

	if (enter_suspend && out->audio_state == AUDIO_A2DP_STATE_STARTED) {
		if (ipc_suspend_stream_cmd(out->ep->id) != AUDIO_STATUS_SUCCESS)
			return -1;
		out->audio_state = AUDIO_A2DP_STATE_SUSPENDED;
	}

	if (exit_suspend && out->audio_state == AUDIO_A2DP_STATE_SUSPENDED)
		out->audio_state = AUDIO_A2DP_STATE_STANDBY;

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
	struct a2dp_stream_out *out = (struct a2dp_stream_out *) stream;
	struct audio_endpoint *ep = out->ep;
	size_t pkt_duration;

	DBG("");

	pkt_duration = ep->codec->get_mediapacket_duration(ep->codec_data);

	return FIXED_A2DP_PLAYBACK_LATENCY_MS + pkt_duration / 1000;
}

static int out_set_volume(struct audio_stream_out *stream, float left,
								float right)
{
	DBG("");
	/* volume controlled in audioflinger mixer (digital) */
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

static uint32_t in_get_sample_rate(const struct audio_stream *stream)
{
	DBG("");
	return -ENOSYS;
}

static int in_set_sample_rate(struct audio_stream *stream, uint32_t rate)
{
	DBG("");
	return -ENOSYS;
}

static size_t in_get_buffer_size(const struct audio_stream *stream)
{
	DBG("");
	return -ENOSYS;
}

static uint32_t in_get_channels(const struct audio_stream *stream)
{
	DBG("");
	return -ENOSYS;
}

static audio_format_t in_get_format(const struct audio_stream *stream)
{
	DBG("");
	return -ENOSYS;
}

static int in_set_format(struct audio_stream *stream, audio_format_t format)
{
	DBG("");
	return -ENOSYS;
}

static int in_standby(struct audio_stream *stream)
{
	DBG("");
	return -ENOSYS;
}

static int in_dump(const struct audio_stream *stream, int fd)
{
	DBG("");
	return -ENOSYS;
}

static int in_set_parameters(struct audio_stream *stream, const char *kvpairs)
{
	DBG("");
	return -ENOSYS;
}

static char *in_get_parameters(const struct audio_stream *stream,
							const char *keys)
{
	DBG("");
	return strdup("");
}

static int in_set_gain(struct audio_stream_in *stream, float gain)
{
	DBG("");
	return -ENOSYS;
}

static ssize_t in_read(struct audio_stream_in *stream, void *buffer,
								size_t bytes)
{
	DBG("");
	return -ENOSYS;
}

static uint32_t in_get_input_frames_lost(struct audio_stream_in *stream)
{
	DBG("");
	return -ENOSYS;
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

static int audio_open_output_stream_real(struct audio_hw_device *dev,
					audio_io_handle_t handle,
					audio_devices_t devices,
					audio_output_flags_t flags,
					struct audio_config *config,
					struct audio_stream_out **stream_out,
					const char *address)
{
	struct a2dp_audio_dev *a2dp_dev = (struct a2dp_audio_dev *) dev;
	struct a2dp_stream_out *out;

	out = calloc(1, sizeof(struct a2dp_stream_out));
	if (!out)
		return -ENOMEM;

	DBG("");

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

	/* We want to autoselect opened endpoint */
	out->ep = NULL;

	if (!open_endpoint(&out->ep, &out->cfg))
		goto fail;

	DBG("rate=%d channels=%d format=%d", out->cfg.rate,
					out->cfg.channels, out->cfg.format);

	if (out->cfg.channels == AUDIO_CHANNEL_OUT_MONO) {
		out->downmix_buf = malloc(FIXED_BUFFER_SIZE / 2);
		if (!out->downmix_buf)
			goto fail;
	}

	*stream_out = &out->stream;
	a2dp_dev->out = out;

	out->audio_state = AUDIO_A2DP_STATE_STANDBY;

	return 0;

fail:
	error("audio: cannot open output stream");
	free(out);
	*stream_out = NULL;
	return -EIO;
}

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static int audio_open_output_stream(struct audio_hw_device *dev,
					audio_io_handle_t handle,
					audio_devices_t devices,
					audio_output_flags_t flags,
					struct audio_config *config,
					struct audio_stream_out **stream_out,
					const char *address)
{
	return audio_open_output_stream_real(dev, handle, devices, flags,
						config, stream_out, address);
}
#else
static int audio_open_output_stream(struct audio_hw_device *dev,
					audio_io_handle_t handle,
					audio_devices_t devices,
					audio_output_flags_t flags,
					struct audio_config *config,
					struct audio_stream_out **stream_out)
{
	return audio_open_output_stream_real(dev, handle, devices, flags,
						config, stream_out, NULL);
}
#endif

static void audio_close_output_stream(struct audio_hw_device *dev,
					struct audio_stream_out *stream)
{
	struct a2dp_audio_dev *a2dp_dev = (struct a2dp_audio_dev *) dev;
	struct a2dp_stream_out *out = (struct a2dp_stream_out *) stream;

	DBG("");

	close_endpoint(a2dp_dev->out->ep);

	free(out->downmix_buf);

	free(stream);
	a2dp_dev->out = NULL;
}

static int audio_set_parameters(struct audio_hw_device *dev,
							const char *kvpairs)
{
	struct a2dp_audio_dev *a2dp_dev = (struct a2dp_audio_dev *) dev;
	struct a2dp_stream_out *out = a2dp_dev->out;

	DBG("");

	if (!out)
		return 0;

	return out->stream.common.set_parameters((struct audio_stream *) out,
								kvpairs);
}

static char *audio_get_parameters(const struct audio_hw_device *dev,
							const char *keys)
{
	DBG("");
	return strdup("");
}

static int audio_init_check(const struct audio_hw_device *dev)
{
	DBG("");
	return 0;
}

static int audio_set_voice_volume(struct audio_hw_device *dev, float volume)
{
	DBG("");
	return -ENOSYS;
}

static int audio_set_master_volume(struct audio_hw_device *dev, float volume)
{
	DBG("");
	return -ENOSYS;
}

static int audio_set_mode(struct audio_hw_device *dev, int mode)
{
	DBG("");
	return -ENOSYS;
}

static int audio_set_mic_mute(struct audio_hw_device *dev, bool state)
{
	DBG("");
	return -ENOSYS;
}

static int audio_get_mic_mute(const struct audio_hw_device *dev, bool *state)
{
	DBG("");
	return -ENOSYS;
}

static size_t audio_get_input_buffer_size(const struct audio_hw_device *dev,
					const struct audio_config *config)
{
	DBG("");
	return -ENOSYS;
}

static int audio_open_input_stream_real(struct audio_hw_device *dev,
					audio_io_handle_t handle,
					audio_devices_t devices,
					struct audio_config *config,
					struct audio_stream_in **stream_in,
					audio_input_flags_t flags,
					const char *address,
					audio_source_t source)
{
	struct audio_stream_in *in;

	DBG("");

	in = calloc(1, sizeof(struct audio_stream_in));
	if (!in)
		return -ENOMEM;

	in->common.get_sample_rate = in_get_sample_rate;
	in->common.set_sample_rate = in_set_sample_rate;
	in->common.get_buffer_size = in_get_buffer_size;
	in->common.get_channels = in_get_channels;
	in->common.get_format = in_get_format;
	in->common.set_format = in_set_format;
	in->common.standby = in_standby;
	in->common.dump = in_dump;
	in->common.set_parameters = in_set_parameters;
	in->common.get_parameters = in_get_parameters;
	in->common.add_audio_effect = in_add_audio_effect;
	in->common.remove_audio_effect = in_remove_audio_effect;
	in->set_gain = in_set_gain;
	in->read = in_read;
	in->get_input_frames_lost = in_get_input_frames_lost;

	*stream_in = in;

	return 0;
}

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static int audio_open_input_stream(struct audio_hw_device *dev,
					audio_io_handle_t handle,
					audio_devices_t devices,
					struct audio_config *config,
					struct audio_stream_in **stream_in,
					audio_input_flags_t flags,
					const char *address,
					audio_source_t source)
{
	return audio_open_input_stream_real(dev, handle, devices, config,
						stream_in, flags, address,
						source);
}
#else
static int audio_open_input_stream(struct audio_hw_device *dev,
					audio_io_handle_t handle,
					audio_devices_t devices,
					struct audio_config *config,
					struct audio_stream_in **stream_in)
{
	return audio_open_input_stream_real(dev, handle, devices, config,
						stream_in, 0, NULL, 0);
}
#endif

static void audio_close_input_stream(struct audio_hw_device *dev,
					struct audio_stream_in *stream_in)
{
	DBG("");
	free(stream_in);
}

static int audio_dump(const audio_hw_device_t *device, int fd)
{
	DBG("");
	return -ENOSYS;
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

static int audio_close(hw_device_t *device)
{
	struct a2dp_audio_dev *a2dp_dev = (struct a2dp_audio_dev *)device;
	unsigned int i;

	DBG("");

	unregister_endpoints();

	for (i = 0; i < NUM_CODECS; i++) {
		const struct audio_codec *codec = audio_codecs[i].get_codec();

		if (!audio_codecs[i].loaded)
			continue;

		if (codec->unload)
			codec->unload();

		audio_codecs[i].loaded = false;
	}

	shutdown(listen_sk, SHUT_RDWR);
	shutdown(audio_sk, SHUT_RDWR);

	pthread_join(ipc_th, NULL);

	close(listen_sk);
	listen_sk = -1;

	free(a2dp_dev);
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
				error("audio: Failed to accept socket: %d (%s)",
							err, strerror(err));

			break;
		}

		pthread_mutex_lock(&sk_mutex);
		audio_sk = sk;
		pthread_mutex_unlock(&sk_mutex);

		DBG("Audio IPC: Connected");

		if (register_endpoints() != AUDIO_STATUS_SUCCESS) {
			error("audio: Failed to register endpoints");

			unregister_endpoints();

			pthread_mutex_lock(&sk_mutex);
			shutdown(audio_sk, SHUT_RDWR);
			close(audio_sk);
			audio_sk = -1;
			pthread_mutex_unlock(&sk_mutex);

			continue;
		}

		memset(&pfd, 0, sizeof(pfd));
		pfd.fd = audio_sk;
		pfd.events = POLLHUP | POLLERR | POLLNVAL;

		/* Check if socket is still alive. Empty while loop.*/
		while (poll(&pfd, 1, -1) < 0 && errno == EINTR);

		info("Audio HAL: Socket closed");

		pthread_mutex_lock(&sk_mutex);
		close(audio_sk);
		audio_sk = -1;
		pthread_mutex_unlock(&sk_mutex);
	}

	/* audio_sk is closed at this point, just cleanup endpoints states */
	memset(audio_endpoints, 0, sizeof(audio_endpoints));

	info("Closing Audio IPC thread");
	return NULL;
}

static int audio_ipc_init(void)
{
	struct sockaddr_un addr;
	int err;
	int sk;

	DBG("");

	sk = socket(PF_LOCAL, SOCK_SEQPACKET, 0);
	if (sk < 0) {
		err = -errno;
		error("audio: Failed to create socket: %d (%s)", -err,
								strerror(-err));
		return err;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;

	memcpy(addr.sun_path, BLUEZ_AUDIO_SK_PATH,
					sizeof(BLUEZ_AUDIO_SK_PATH));

	if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		err = -errno;
		error("audio: Failed to bind socket: %d (%s)", -err,
								strerror(-err));
		goto failed;
	}

	if (listen(sk, 1) < 0) {
		err = -errno;
		error("audio: Failed to listen on the socket: %d (%s)", -err,
								strerror(-err));
		goto failed;
	}

	listen_sk = sk;

	err = pthread_create(&ipc_th, NULL, ipc_handler, NULL);
	if (err) {
		err = -err;
		ipc_th = 0;
		error("audio: Failed to start Audio IPC thread: %d (%s)",
							-err, strerror(-err));
		goto failed;
	}

	return 0;

failed:
	close(sk);
	return err;
}

static int audio_open(const hw_module_t *module, const char *name,
							hw_device_t **device)
{
	struct a2dp_audio_dev *a2dp_dev;
	size_t i;
	int err;

	DBG("");

	if (strcmp(name, AUDIO_HARDWARE_INTERFACE)) {
		error("audio: interface %s not matching [%s]", name,
						AUDIO_HARDWARE_INTERFACE);
		return -EINVAL;
	}

	err = audio_ipc_init();
	if (err < 0)
		return err;

	a2dp_dev = calloc(1, sizeof(struct a2dp_audio_dev));
	if (!a2dp_dev)
		return -ENOMEM;

	a2dp_dev->dev.common.tag = HARDWARE_DEVICE_TAG;
	a2dp_dev->dev.common.version = AUDIO_DEVICE_API_VERSION_CURRENT;
	a2dp_dev->dev.common.module = (struct hw_module_t *) module;
	a2dp_dev->dev.common.close = audio_close;

	a2dp_dev->dev.init_check = audio_init_check;
	a2dp_dev->dev.set_voice_volume = audio_set_voice_volume;
	a2dp_dev->dev.set_master_volume = audio_set_master_volume;
	a2dp_dev->dev.set_mode = audio_set_mode;
	a2dp_dev->dev.set_mic_mute = audio_set_mic_mute;
	a2dp_dev->dev.get_mic_mute = audio_get_mic_mute;
	a2dp_dev->dev.set_parameters = audio_set_parameters;
	a2dp_dev->dev.get_parameters = audio_get_parameters;
	a2dp_dev->dev.get_input_buffer_size = audio_get_input_buffer_size;
	a2dp_dev->dev.open_output_stream = audio_open_output_stream;
	a2dp_dev->dev.close_output_stream = audio_close_output_stream;
	a2dp_dev->dev.open_input_stream = audio_open_input_stream;
	a2dp_dev->dev.close_input_stream = audio_close_input_stream;
	a2dp_dev->dev.dump = audio_dump;
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	a2dp_dev->dev.set_master_mute = set_master_mute;
	a2dp_dev->dev.get_master_mute = get_master_mute;
	a2dp_dev->dev.create_audio_patch = create_audio_patch;
	a2dp_dev->dev.release_audio_patch = release_audio_patch;
	a2dp_dev->dev.get_audio_port = get_audio_port;
	a2dp_dev->dev.set_audio_port_config = set_audio_port_config;
#endif

	for (i = 0; i < NUM_CODECS; i++) {
		const struct audio_codec *codec = audio_codecs[i].get_codec();

		if (codec->load && !codec->load())
			continue;

		audio_codecs[i].loaded = true;
	}

	/*
	 * Note that &a2dp_dev->dev.common is the same pointer as a2dp_dev.
	 * This results from the structure of following structs:a2dp_audio_dev,
	 * audio_hw_device. We will rely on this later in the code.
	 */
	*device = &a2dp_dev->dev.common;

	return 0;
}

static struct hw_module_methods_t hal_module_methods = {
	.open = audio_open,
};

struct audio_module HAL_MODULE_INFO_SYM = {
	.common = {
		.tag = HARDWARE_MODULE_TAG,
		.version_major = 1,
		.version_minor = 0,
		.id = AUDIO_HARDWARE_MODULE_ID,
		.name = "A2DP Bluez HW HAL",
		.author = "Intel Corporation",
		.methods = &hal_module_methods,
	},
};
