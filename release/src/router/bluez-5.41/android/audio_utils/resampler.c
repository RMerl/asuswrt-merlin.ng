/*
** Copyright 2011, The Android Open-Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

//#define LOG_NDEBUG 0

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <system/audio.h>
#include <audio_utils/resampler.h>
#include <speex/speex_resampler.h>

#include "hal-log.h"

struct resampler {
    struct resampler_itfe itfe;
    SpeexResamplerState *speex_resampler;       // handle on speex resampler
    struct resampler_buffer_provider *provider; // buffer provider installed by client
    uint32_t in_sample_rate;                    // input sampling rate in Hz
    uint32_t out_sample_rate;                   // output sampling rate in Hz
    uint32_t channel_count;                     // number of channels (interleaved)
    int16_t *in_buf;                            // input buffer
    size_t in_buf_size;                         // input buffer size
    size_t frames_in;                           // number of frames in input buffer
    size_t frames_rq;                           // cached number of output frames
    size_t frames_needed;                       // minimum number of input frames to produce
                                                // frames_rq output frames
    int32_t speex_delay_ns;                     // delay introduced by speex resampler in ns
};


//------------------------------------------------------------------------------
// speex based resampler
//------------------------------------------------------------------------------

static void resampler_reset(struct resampler_itfe *resampler)
{
    struct resampler *rsmp = (struct resampler *)resampler;

    rsmp->frames_in = 0;
    rsmp->frames_rq = 0;

    if (rsmp != NULL && rsmp->speex_resampler != NULL) {
        speex_resampler_reset_mem(rsmp->speex_resampler);
    }
}

static int32_t resampler_delay_ns(struct resampler_itfe *resampler)
{
    struct resampler *rsmp = (struct resampler *)resampler;

    int32_t delay = (int32_t)((1000000000 * (int64_t)rsmp->frames_in) / rsmp->in_sample_rate);
    delay += rsmp->speex_delay_ns;

    return delay;
}

// outputs a number of frames less or equal to *outFrameCount and updates *outFrameCount
// with the actual number of frames produced.
static int resampler_resample_from_provider(struct resampler_itfe *resampler,
                       int16_t *out,
                       size_t *outFrameCount)
{
    struct resampler *rsmp = (struct resampler *)resampler;
    size_t framesRq;
    size_t framesWr;
    size_t inFrames;

    if (rsmp == NULL || out == NULL || outFrameCount == NULL) {
        return -EINVAL;
    }
    if (rsmp->provider == NULL) {
        *outFrameCount = 0;
        return -ENOSYS;
    }

    framesRq = *outFrameCount;
    // update and cache the number of frames needed at the input sampling rate to produce
    // the number of frames requested at the output sampling rate
    if (framesRq != rsmp->frames_rq) {
        rsmp->frames_needed = (framesRq * rsmp->in_sample_rate) / rsmp->out_sample_rate + 1;
        rsmp->frames_rq = framesRq;
    }

    framesWr = 0;
    inFrames = 0;
    while (framesWr < framesRq) {
        size_t outFrames;
        if (rsmp->frames_in < rsmp->frames_needed) {
            struct resampler_buffer buf;
            // make sure that the number of frames present in rsmp->in_buf (rsmp->frames_in) is at
            // least the number of frames needed to produce the number of frames requested at
            // the output sampling rate
            if (rsmp->in_buf_size < rsmp->frames_needed) {
                rsmp->in_buf_size = rsmp->frames_needed;
                rsmp->in_buf = (int16_t *)realloc(rsmp->in_buf,
                                        rsmp->in_buf_size * rsmp->channel_count * sizeof(int16_t));
            }
            buf.frame_count = rsmp->frames_needed - rsmp->frames_in;
            rsmp->provider->get_next_buffer(rsmp->provider, &buf);
            if (buf.raw == NULL) {
                break;
            }
            memcpy(rsmp->in_buf + rsmp->frames_in * rsmp->channel_count,
                    buf.raw,
                    buf.frame_count * rsmp->channel_count * sizeof(int16_t));
            rsmp->frames_in += buf.frame_count;
            rsmp->provider->release_buffer(rsmp->provider, &buf);
        }

        outFrames = framesRq - framesWr;
        inFrames = rsmp->frames_in;
        if (rsmp->channel_count == 1) {
            speex_resampler_process_int(rsmp->speex_resampler,
                                        0,
                                        rsmp->in_buf,
                                        (void *) &inFrames,
                                        out + framesWr,
                                        (void *) &outFrames);
        } else {
            speex_resampler_process_interleaved_int(rsmp->speex_resampler,
                                        rsmp->in_buf,
                                        (void *) &inFrames,
                                        out + framesWr * rsmp->channel_count,
                                        (void *) &outFrames);
        }
        framesWr += outFrames;
        rsmp->frames_in -= inFrames;

        if ((framesWr != framesRq) && (rsmp->frames_in != 0))
            warn("ReSampler::resample() remaining %zd frames in and %zd out",
                rsmp->frames_in, (framesRq - framesWr));
    }
    if (rsmp->frames_in) {
        memmove(rsmp->in_buf,
                rsmp->in_buf + inFrames * rsmp->channel_count,
                rsmp->frames_in * rsmp->channel_count * sizeof(int16_t));
    }
    *outFrameCount = framesWr;

    return 0;
}

static int resampler_resample_from_input(struct resampler_itfe *resampler,
                                  int16_t *in,
                                  size_t *inFrameCount,
                                  int16_t *out,
                                  size_t *outFrameCount)
{
    struct resampler *rsmp = (struct resampler *)resampler;

    if (rsmp == NULL || in == NULL || inFrameCount == NULL ||
            out == NULL || outFrameCount == NULL) {
        return -EINVAL;
    }
    if (rsmp->provider != NULL) {
        *outFrameCount = 0;
        return -ENOSYS;
    }

    if (rsmp->channel_count == 1) {
        speex_resampler_process_int(rsmp->speex_resampler,
                                    0,
                                    in,
                                    (void *) inFrameCount,
                                    out,
                                    (void *) outFrameCount);
    } else {
        speex_resampler_process_interleaved_int(rsmp->speex_resampler,
                                                in,
                                                (void *) inFrameCount,
                                                out,
                                                (void *) outFrameCount);
    }

    DBG("resampler_resample_from_input() DONE in %zd out %zd", *inFrameCount, *outFrameCount);

    return 0;
}

int create_resampler(uint32_t inSampleRate,
                    uint32_t outSampleRate,
                    uint32_t channelCount,
                    uint32_t quality,
                    struct resampler_buffer_provider* provider,
                    struct resampler_itfe **resampler)
{
    int error;
    struct resampler *rsmp;
    int frames;

    DBG("create_resampler() In SR %d Out SR %d channels %d",
         inSampleRate, outSampleRate, channelCount);

    if (resampler == NULL) {
        return -EINVAL;
    }

    *resampler = NULL;

    if (quality <= RESAMPLER_QUALITY_MIN || quality >= RESAMPLER_QUALITY_MAX) {
        return -EINVAL;
    }

    rsmp = (struct resampler *)calloc(1, sizeof(struct resampler));

    rsmp->speex_resampler = speex_resampler_init(channelCount,
                                      inSampleRate,
                                      outSampleRate,
                                      quality,
                                      &error);
    if (rsmp->speex_resampler == NULL) {
        error("ReSampler: Cannot create speex resampler: %s", speex_resampler_strerror(error));
        free(rsmp);
        return -ENODEV;
    }

    rsmp->itfe.reset = resampler_reset;
    rsmp->itfe.resample_from_provider = resampler_resample_from_provider;
    rsmp->itfe.resample_from_input = resampler_resample_from_input;
    rsmp->itfe.delay_ns = resampler_delay_ns;

    rsmp->provider = provider;
    rsmp->in_sample_rate = inSampleRate;
    rsmp->out_sample_rate = outSampleRate;
    rsmp->channel_count = channelCount;
    rsmp->in_buf = NULL;
    rsmp->in_buf_size = 0;

    resampler_reset(&rsmp->itfe);

    frames = speex_resampler_get_input_latency(rsmp->speex_resampler);
    rsmp->speex_delay_ns = (int32_t)((1000000000 * (int64_t)frames) / rsmp->in_sample_rate);
    frames = speex_resampler_get_output_latency(rsmp->speex_resampler);
    rsmp->speex_delay_ns += (int32_t)((1000000000 * (int64_t)frames) / rsmp->out_sample_rate);

    *resampler = &rsmp->itfe;
    DBG("create_resampler() DONE rsmp %p &rsmp->itfe %p speex %p",
         rsmp, &rsmp->itfe, rsmp->speex_resampler);
    return 0;
}

void release_resampler(struct resampler_itfe *resampler)
{
    struct resampler *rsmp = (struct resampler *)resampler;

    if (rsmp == NULL) {
        return;
    }

    free(rsmp->in_buf);

    if (rsmp->speex_resampler != NULL) {
        speex_resampler_destroy(rsmp->speex_resampler);
    }
    free(rsmp);
}
