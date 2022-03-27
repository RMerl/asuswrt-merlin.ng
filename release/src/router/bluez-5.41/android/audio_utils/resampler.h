/*
** Copyright 2008, The Android Open-Source Project
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

#ifndef ANDROID_RESAMPLER_H
#define ANDROID_RESAMPLER_H

#include <stdint.h>
#include <sys/time.h>

__BEGIN_DECLS


#define RESAMPLER_QUALITY_MAX 10
#define RESAMPLER_QUALITY_MIN 0
#define RESAMPLER_QUALITY_DEFAULT 4
#define RESAMPLER_QUALITY_VOIP 3
#define RESAMPLER_QUALITY_DESKTOP 5

struct resampler_buffer {
    union {
        void*       raw;
        short*      i16;
        int8_t*     i8;
    };
    size_t frame_count;
};

/* call back interface used by the resampler to get new data */
struct resampler_buffer_provider
{
    /**
     *  get a new buffer of data:
     *   as input: buffer->frame_count is the number of frames requested
     *   as output: buffer->frame_count is the number of frames returned
     *              buffer->raw points to data returned
     */
    int (*get_next_buffer)(struct resampler_buffer_provider *provider,
            struct resampler_buffer *buffer);
    /**
     *  release a consumed buffer of data:
     *   as input: buffer->frame_count is the number of frames released
     *             buffer->raw points to data released
     */
    void (*release_buffer)(struct resampler_buffer_provider *provider,
            struct resampler_buffer *buffer);
};

/* resampler interface */
struct resampler_itfe {
    /**
     * reset resampler state
     */
    void (*reset)(struct resampler_itfe *resampler);
    /**
     * resample input from buffer provider and output at most *outFrameCount to out buffer.
     * *outFrameCount is updated with the actual number of frames produced.
     */
    int (*resample_from_provider)(struct resampler_itfe *resampler,
                    int16_t *out,
                    size_t *outFrameCount);
    /**
     * resample at most *inFrameCount frames from in buffer and output at most
     * *outFrameCount to out buffer. *inFrameCount and *outFrameCount are updated respectively
     * with the number of frames remaining in input and written to output.
     */
    int (*resample_from_input)(struct resampler_itfe *resampler,
                    int16_t *in,
                    size_t *inFrameCount,
                    int16_t *out,
                    size_t *outFrameCount);
    /**
     * return the latency introduced by the resampler in ns.
     */
    int32_t (*delay_ns)(struct resampler_itfe *resampler);
};

/**
 * create a resampler according to input parameters passed.
 * If resampler_buffer_provider is not NULL only resample_from_provider() can be called.
 * If resampler_buffer_provider is NULL only resample_from_input() can be called.
 */
int create_resampler(uint32_t inSampleRate,
          uint32_t outSampleRate,
          uint32_t channelCount,
          uint32_t quality,
          struct resampler_buffer_provider *provider,
          struct resampler_itfe **);

/**
 * release resampler resources.
 */
void release_resampler(struct resampler_itfe *);

__END_DECLS

#endif // ANDROID_RESAMPLER_H
