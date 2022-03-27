/*
 * Copyright (C) 2011 The Android Open Source Project
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
 */


#ifndef ANDROID_AUDIO_CORE_H
#define ANDROID_AUDIO_CORE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

#define popcount __builtin_popcount

/* The enums were moved here mostly from
 * frameworks/base/include/media/AudioSystem.h
 */

/* device address used to refer to the standard remote submix */
#define AUDIO_REMOTE_SUBMIX_DEVICE_ADDRESS "0"

/* AudioFlinger and AudioPolicy services use I/O handles to identify audio sources and sinks */
typedef int audio_io_handle_t;
#define AUDIO_IO_HANDLE_NONE    0

/* Audio stream types */
typedef enum {
    /* These values must kept in sync with
     * frameworks/base/media/java/android/media/AudioSystem.java
     */
    AUDIO_STREAM_DEFAULT          = -1,
    AUDIO_STREAM_MIN              = 0,
    AUDIO_STREAM_VOICE_CALL       = 0,
    AUDIO_STREAM_SYSTEM           = 1,
    AUDIO_STREAM_RING             = 2,
    AUDIO_STREAM_MUSIC            = 3,
    AUDIO_STREAM_ALARM            = 4,
    AUDIO_STREAM_NOTIFICATION     = 5,
    AUDIO_STREAM_BLUETOOTH_SCO    = 6,
    AUDIO_STREAM_ENFORCED_AUDIBLE = 7, /* Sounds that cannot be muted by user
                                        * and must be routed to speaker
                                        */
    AUDIO_STREAM_DTMF             = 8,
    AUDIO_STREAM_TTS              = 9,

    AUDIO_STREAM_CNT,
    AUDIO_STREAM_MAX              = AUDIO_STREAM_CNT - 1,
} audio_stream_type_t;

/* Do not change these values without updating their counterparts
 * in frameworks/base/media/java/android/media/AudioAttributes.java
 */
typedef enum {
    AUDIO_CONTENT_TYPE_UNKNOWN      = 0,
    AUDIO_CONTENT_TYPE_SPEECH       = 1,
    AUDIO_CONTENT_TYPE_MUSIC        = 2,
    AUDIO_CONTENT_TYPE_MOVIE        = 3,
    AUDIO_CONTENT_TYPE_SONIFICATION = 4,

    AUDIO_CONTENT_TYPE_CNT,
    AUDIO_CONTENT_TYPE_MAX          = AUDIO_CONTENT_TYPE_CNT - 1,
} audio_content_type_t;

/* Do not change these values without updating their counterparts
 * in frameworks/base/media/java/android/media/AudioAttributes.java
 */
typedef enum {
    AUDIO_USAGE_UNKNOWN                            = 0,
    AUDIO_USAGE_MEDIA                              = 1,
    AUDIO_USAGE_VOICE_COMMUNICATION                = 2,
    AUDIO_USAGE_VOICE_COMMUNICATION_SIGNALLING     = 3,
    AUDIO_USAGE_ALARM                              = 4,
    AUDIO_USAGE_NOTIFICATION                       = 5,
    AUDIO_USAGE_NOTIFICATION_TELEPHONY_RINGTONE    = 6,
    AUDIO_USAGE_NOTIFICATION_COMMUNICATION_REQUEST = 7,
    AUDIO_USAGE_NOTIFICATION_COMMUNICATION_INSTANT = 8,
    AUDIO_USAGE_NOTIFICATION_COMMUNICATION_DELAYED = 9,
    AUDIO_USAGE_NOTIFICATION_EVENT                 = 10,
    AUDIO_USAGE_ASSISTANCE_ACCESSIBILITY           = 11,
    AUDIO_USAGE_ASSISTANCE_NAVIGATION_GUIDANCE     = 12,
    AUDIO_USAGE_ASSISTANCE_SONIFICATION            = 13,
    AUDIO_USAGE_GAME                               = 14,

    AUDIO_USAGE_CNT,
    AUDIO_USAGE_MAX                                = AUDIO_USAGE_CNT - 1,
} audio_usage_t;

typedef uint32_t audio_flags_mask_t;

/* Do not change these values without updating their counterparts
 * in frameworks/base/media/java/android/media/AudioAttributes.java
 */
enum {
    AUDIO_FLAG_AUDIBILITY_ENFORCED = 0x1,
    AUDIO_FLAG_SECURE              = 0x2,
    AUDIO_FLAG_SCO                 = 0x4,
    AUDIO_FLAG_BEACON              = 0x8,
    AUDIO_FLAG_HW_AV_SYNC          = 0x10,
    AUDIO_FLAG_HW_HOTWORD          = 0x20,
};

/* Do not change these values without updating their counterparts
 * in frameworks/base/media/java/android/media/MediaRecorder.java,
 * frameworks/av/services/audiopolicy/AudioPolicyService.cpp,
 * and system/media/audio_effects/include/audio_effects/audio_effects_conf.h!
 */
typedef enum {
    AUDIO_SOURCE_DEFAULT             = 0,
    AUDIO_SOURCE_MIC                 = 1,
    AUDIO_SOURCE_VOICE_UPLINK        = 2,
    AUDIO_SOURCE_VOICE_DOWNLINK      = 3,
    AUDIO_SOURCE_VOICE_CALL          = 4,
    AUDIO_SOURCE_CAMCORDER           = 5,
    AUDIO_SOURCE_VOICE_RECOGNITION   = 6,
    AUDIO_SOURCE_VOICE_COMMUNICATION = 7,
    AUDIO_SOURCE_REMOTE_SUBMIX       = 8, /* Source for the mix to be presented remotely.      */
                                          /* An example of remote presentation is Wifi Display */
                                          /*  where a dongle attached to a TV can be used to   */
                                          /*  play the mix captured by this audio source.      */
    AUDIO_SOURCE_CNT,
    AUDIO_SOURCE_MAX                 = AUDIO_SOURCE_CNT - 1,
    AUDIO_SOURCE_HOTWORD             = 1999, /* A low-priority, preemptible audio source for
                                                for background software hotword detection.
                                                Same tuning as AUDIO_SOURCE_VOICE_RECOGNITION.
                                                Used only internally to the framework. Not exposed
                                                at the audio HAL. */
} audio_source_t;

/* Audio attributes */
#define AUDIO_ATTRIBUTES_TAGS_MAX_SIZE 256
typedef struct {
    audio_content_type_t content_type;
    audio_usage_t        usage;
    audio_source_t       source;
    audio_flags_mask_t   flags;
    char                 tags[AUDIO_ATTRIBUTES_TAGS_MAX_SIZE]; /* UTF8 */
} audio_attributes_t;

/* special audio session values
 * (XXX: should this be living in the audio effects land?)
 */
typedef enum {
    /* session for effects attached to a particular output stream
     * (value must be less than 0)
     */
    AUDIO_SESSION_OUTPUT_STAGE = -1,

    /* session for effects applied to output mix. These effects can
     * be moved by audio policy manager to another output stream
     * (value must be 0)
     */
    AUDIO_SESSION_OUTPUT_MIX = 0,

    /* application does not specify an explicit session ID to be used,
     * and requests a new session ID to be allocated
     * TODO use unique values for AUDIO_SESSION_OUTPUT_MIX and AUDIO_SESSION_ALLOCATE,
     * after all uses have been updated from 0 to the appropriate symbol, and have been tested.
     */
    AUDIO_SESSION_ALLOCATE = 0,
} audio_session_t;

/* a unique ID allocated by AudioFlinger for use as a audio_io_handle_t or audio_session_t */
typedef int audio_unique_id_t;

#define AUDIO_UNIQUE_ID_ALLOCATE AUDIO_SESSION_ALLOCATE

/* Audio sub formats (see enum audio_format). */

/* PCM sub formats */
typedef enum {
    /* All of these are in native byte order */
    AUDIO_FORMAT_PCM_SUB_16_BIT          = 0x1, /* DO NOT CHANGE - PCM signed 16 bits */
    AUDIO_FORMAT_PCM_SUB_8_BIT           = 0x2, /* DO NOT CHANGE - PCM unsigned 8 bits */
    AUDIO_FORMAT_PCM_SUB_32_BIT          = 0x3, /* PCM signed .31 fixed point */
    AUDIO_FORMAT_PCM_SUB_8_24_BIT        = 0x4, /* PCM signed 7.24 fixed point */
    AUDIO_FORMAT_PCM_SUB_FLOAT           = 0x5, /* PCM single-precision floating point */
    AUDIO_FORMAT_PCM_SUB_24_BIT_PACKED   = 0x6, /* PCM signed .23 fixed point packed in 3 bytes */
} audio_format_pcm_sub_fmt_t;

/* The audio_format_*_sub_fmt_t declarations are not currently used */

/* MP3 sub format field definition : can use 11 LSBs in the same way as MP3
 * frame header to specify bit rate, stereo mode, version...
 */
typedef enum {
    AUDIO_FORMAT_MP3_SUB_NONE            = 0x0,
} audio_format_mp3_sub_fmt_t;

/* AMR NB/WB sub format field definition: specify frame block interleaving,
 * bandwidth efficient or octet aligned, encoding mode for recording...
 */
typedef enum {
    AUDIO_FORMAT_AMR_SUB_NONE            = 0x0,
} audio_format_amr_sub_fmt_t;

/* AAC sub format field definition: specify profile or bitrate for recording... */
typedef enum {
    AUDIO_FORMAT_AAC_SUB_MAIN            = 0x1,
    AUDIO_FORMAT_AAC_SUB_LC              = 0x2,
    AUDIO_FORMAT_AAC_SUB_SSR             = 0x4,
    AUDIO_FORMAT_AAC_SUB_LTP             = 0x8,
    AUDIO_FORMAT_AAC_SUB_HE_V1           = 0x10,
    AUDIO_FORMAT_AAC_SUB_SCALABLE        = 0x20,
    AUDIO_FORMAT_AAC_SUB_ERLC            = 0x40,
    AUDIO_FORMAT_AAC_SUB_LD              = 0x80,
    AUDIO_FORMAT_AAC_SUB_HE_V2           = 0x100,
    AUDIO_FORMAT_AAC_SUB_ELD             = 0x200,
} audio_format_aac_sub_fmt_t;

/* VORBIS sub format field definition: specify quality for recording... */
typedef enum {
    AUDIO_FORMAT_VORBIS_SUB_NONE         = 0x0,
} audio_format_vorbis_sub_fmt_t;

/* Audio format consists of a main format field (upper 8 bits) and a sub format
 * field (lower 24 bits).
 *
 * The main format indicates the main codec type. The sub format field
 * indicates options and parameters for each format. The sub format is mainly
 * used for record to indicate for instance the requested bitrate or profile.
 * It can also be used for certain formats to give informations not present in
 * the encoded audio stream (e.g. octet alignement for AMR).
 */
typedef enum {
    AUDIO_FORMAT_INVALID             = 0xFFFFFFFFUL,
    AUDIO_FORMAT_DEFAULT             = 0,
    AUDIO_FORMAT_PCM                 = 0x00000000UL, /* DO NOT CHANGE */
    AUDIO_FORMAT_MP3                 = 0x01000000UL,
    AUDIO_FORMAT_AMR_NB              = 0x02000000UL,
    AUDIO_FORMAT_AMR_WB              = 0x03000000UL,
    AUDIO_FORMAT_AAC                 = 0x04000000UL,
    AUDIO_FORMAT_HE_AAC_V1           = 0x05000000UL, /* Deprecated, Use AUDIO_FORMAT_AAC_HE_V1*/
    AUDIO_FORMAT_HE_AAC_V2           = 0x06000000UL, /* Deprecated, Use AUDIO_FORMAT_AAC_HE_V2*/
    AUDIO_FORMAT_VORBIS              = 0x07000000UL,
    AUDIO_FORMAT_OPUS                = 0x08000000UL,
    AUDIO_FORMAT_AC3                 = 0x09000000UL,
    AUDIO_FORMAT_E_AC3               = 0x0A000000UL,
    AUDIO_FORMAT_MAIN_MASK           = 0xFF000000UL,
    AUDIO_FORMAT_SUB_MASK            = 0x00FFFFFFUL,

    /* Aliases */
    /* note != AudioFormat.ENCODING_PCM_16BIT */
    AUDIO_FORMAT_PCM_16_BIT          = (AUDIO_FORMAT_PCM |
                                        AUDIO_FORMAT_PCM_SUB_16_BIT),
    /* note != AudioFormat.ENCODING_PCM_8BIT */
    AUDIO_FORMAT_PCM_8_BIT           = (AUDIO_FORMAT_PCM |
                                        AUDIO_FORMAT_PCM_SUB_8_BIT),
    AUDIO_FORMAT_PCM_32_BIT          = (AUDIO_FORMAT_PCM |
                                        AUDIO_FORMAT_PCM_SUB_32_BIT),
    AUDIO_FORMAT_PCM_8_24_BIT        = (AUDIO_FORMAT_PCM |
                                        AUDIO_FORMAT_PCM_SUB_8_24_BIT),
    AUDIO_FORMAT_PCM_FLOAT           = (AUDIO_FORMAT_PCM |
                                        AUDIO_FORMAT_PCM_SUB_FLOAT),
    AUDIO_FORMAT_PCM_24_BIT_PACKED   = (AUDIO_FORMAT_PCM |
                                        AUDIO_FORMAT_PCM_SUB_24_BIT_PACKED),
    AUDIO_FORMAT_AAC_MAIN            = (AUDIO_FORMAT_AAC |
                                        AUDIO_FORMAT_AAC_SUB_MAIN),
    AUDIO_FORMAT_AAC_LC              = (AUDIO_FORMAT_AAC |
                                        AUDIO_FORMAT_AAC_SUB_LC),
    AUDIO_FORMAT_AAC_SSR             = (AUDIO_FORMAT_AAC |
                                        AUDIO_FORMAT_AAC_SUB_SSR),
    AUDIO_FORMAT_AAC_LTP             = (AUDIO_FORMAT_AAC |
                                        AUDIO_FORMAT_AAC_SUB_LTP),
    AUDIO_FORMAT_AAC_HE_V1           = (AUDIO_FORMAT_AAC |
                                        AUDIO_FORMAT_AAC_SUB_HE_V1),
    AUDIO_FORMAT_AAC_SCALABLE        = (AUDIO_FORMAT_AAC |
                                        AUDIO_FORMAT_AAC_SUB_SCALABLE),
    AUDIO_FORMAT_AAC_ERLC            = (AUDIO_FORMAT_AAC |
                                        AUDIO_FORMAT_AAC_SUB_ERLC),
    AUDIO_FORMAT_AAC_LD              = (AUDIO_FORMAT_AAC |
                                        AUDIO_FORMAT_AAC_SUB_LD),
    AUDIO_FORMAT_AAC_HE_V2           = (AUDIO_FORMAT_AAC |
                                        AUDIO_FORMAT_AAC_SUB_HE_V2),
    AUDIO_FORMAT_AAC_ELD             = (AUDIO_FORMAT_AAC |
                                        AUDIO_FORMAT_AAC_SUB_ELD),
} audio_format_t;

/* For the channel mask for position assignment representation */
enum {

/* These can be a complete audio_channel_mask_t. */

    AUDIO_CHANNEL_NONE                      = 0x0,
    AUDIO_CHANNEL_INVALID                   = 0xC0000000,

/* These can be the bits portion of an audio_channel_mask_t
 * with representation AUDIO_CHANNEL_REPRESENTATION_POSITION.
 * Using these bits as a complete audio_channel_mask_t is deprecated.
 */

    /* output channels */
    AUDIO_CHANNEL_OUT_FRONT_LEFT            = 0x1,
    AUDIO_CHANNEL_OUT_FRONT_RIGHT           = 0x2,
    AUDIO_CHANNEL_OUT_FRONT_CENTER          = 0x4,
    AUDIO_CHANNEL_OUT_LOW_FREQUENCY         = 0x8,
    AUDIO_CHANNEL_OUT_BACK_LEFT             = 0x10,
    AUDIO_CHANNEL_OUT_BACK_RIGHT            = 0x20,
    AUDIO_CHANNEL_OUT_FRONT_LEFT_OF_CENTER  = 0x40,
    AUDIO_CHANNEL_OUT_FRONT_RIGHT_OF_CENTER = 0x80,
    AUDIO_CHANNEL_OUT_BACK_CENTER           = 0x100,
    AUDIO_CHANNEL_OUT_SIDE_LEFT             = 0x200,
    AUDIO_CHANNEL_OUT_SIDE_RIGHT            = 0x400,
    AUDIO_CHANNEL_OUT_TOP_CENTER            = 0x800,
    AUDIO_CHANNEL_OUT_TOP_FRONT_LEFT        = 0x1000,
    AUDIO_CHANNEL_OUT_TOP_FRONT_CENTER      = 0x2000,
    AUDIO_CHANNEL_OUT_TOP_FRONT_RIGHT       = 0x4000,
    AUDIO_CHANNEL_OUT_TOP_BACK_LEFT         = 0x8000,
    AUDIO_CHANNEL_OUT_TOP_BACK_CENTER       = 0x10000,
    AUDIO_CHANNEL_OUT_TOP_BACK_RIGHT        = 0x20000,

/* TODO: should these be considered complete channel masks, or only bits? */

    AUDIO_CHANNEL_OUT_MONO     = AUDIO_CHANNEL_OUT_FRONT_LEFT,
    AUDIO_CHANNEL_OUT_STEREO   = (AUDIO_CHANNEL_OUT_FRONT_LEFT |
                                  AUDIO_CHANNEL_OUT_FRONT_RIGHT),
    AUDIO_CHANNEL_OUT_QUAD     = (AUDIO_CHANNEL_OUT_FRONT_LEFT |
                                  AUDIO_CHANNEL_OUT_FRONT_RIGHT |
                                  AUDIO_CHANNEL_OUT_BACK_LEFT |
                                  AUDIO_CHANNEL_OUT_BACK_RIGHT),
    AUDIO_CHANNEL_OUT_QUAD_BACK = AUDIO_CHANNEL_OUT_QUAD,
    /* like AUDIO_CHANNEL_OUT_QUAD_BACK with *_SIDE_* instead of *_BACK_* */
    AUDIO_CHANNEL_OUT_QUAD_SIDE = (AUDIO_CHANNEL_OUT_FRONT_LEFT |
                                  AUDIO_CHANNEL_OUT_FRONT_RIGHT |
                                  AUDIO_CHANNEL_OUT_SIDE_LEFT |
                                  AUDIO_CHANNEL_OUT_SIDE_RIGHT),
    AUDIO_CHANNEL_OUT_5POINT1  = (AUDIO_CHANNEL_OUT_FRONT_LEFT |
                                  AUDIO_CHANNEL_OUT_FRONT_RIGHT |
                                  AUDIO_CHANNEL_OUT_FRONT_CENTER |
                                  AUDIO_CHANNEL_OUT_LOW_FREQUENCY |
                                  AUDIO_CHANNEL_OUT_BACK_LEFT |
                                  AUDIO_CHANNEL_OUT_BACK_RIGHT),
    AUDIO_CHANNEL_OUT_5POINT1_BACK = AUDIO_CHANNEL_OUT_5POINT1,
    /* like AUDIO_CHANNEL_OUT_5POINT1_BACK with *_SIDE_* instead of *_BACK_* */
    AUDIO_CHANNEL_OUT_5POINT1_SIDE = (AUDIO_CHANNEL_OUT_FRONT_LEFT |
                                  AUDIO_CHANNEL_OUT_FRONT_RIGHT |
                                  AUDIO_CHANNEL_OUT_FRONT_CENTER |
                                  AUDIO_CHANNEL_OUT_LOW_FREQUENCY |
                                  AUDIO_CHANNEL_OUT_SIDE_LEFT |
                                  AUDIO_CHANNEL_OUT_SIDE_RIGHT),
    // matches the correct AudioFormat.CHANNEL_OUT_7POINT1_SURROUND definition for 7.1
    AUDIO_CHANNEL_OUT_7POINT1  = (AUDIO_CHANNEL_OUT_FRONT_LEFT |
                                  AUDIO_CHANNEL_OUT_FRONT_RIGHT |
                                  AUDIO_CHANNEL_OUT_FRONT_CENTER |
                                  AUDIO_CHANNEL_OUT_LOW_FREQUENCY |
                                  AUDIO_CHANNEL_OUT_BACK_LEFT |
                                  AUDIO_CHANNEL_OUT_BACK_RIGHT |
                                  AUDIO_CHANNEL_OUT_SIDE_LEFT |
                                  AUDIO_CHANNEL_OUT_SIDE_RIGHT),
    AUDIO_CHANNEL_OUT_ALL      = (AUDIO_CHANNEL_OUT_FRONT_LEFT |
                                  AUDIO_CHANNEL_OUT_FRONT_RIGHT |
                                  AUDIO_CHANNEL_OUT_FRONT_CENTER |
                                  AUDIO_CHANNEL_OUT_LOW_FREQUENCY |
                                  AUDIO_CHANNEL_OUT_BACK_LEFT |
                                  AUDIO_CHANNEL_OUT_BACK_RIGHT |
                                  AUDIO_CHANNEL_OUT_FRONT_LEFT_OF_CENTER |
                                  AUDIO_CHANNEL_OUT_FRONT_RIGHT_OF_CENTER |
                                  AUDIO_CHANNEL_OUT_BACK_CENTER|
                                  AUDIO_CHANNEL_OUT_SIDE_LEFT|
                                  AUDIO_CHANNEL_OUT_SIDE_RIGHT|
                                  AUDIO_CHANNEL_OUT_TOP_CENTER|
                                  AUDIO_CHANNEL_OUT_TOP_FRONT_LEFT|
                                  AUDIO_CHANNEL_OUT_TOP_FRONT_CENTER|
                                  AUDIO_CHANNEL_OUT_TOP_FRONT_RIGHT|
                                  AUDIO_CHANNEL_OUT_TOP_BACK_LEFT|
                                  AUDIO_CHANNEL_OUT_TOP_BACK_CENTER|
                                  AUDIO_CHANNEL_OUT_TOP_BACK_RIGHT),

/* These are bits only, not complete values */

    /* input channels */
    AUDIO_CHANNEL_IN_LEFT            = 0x4,
    AUDIO_CHANNEL_IN_RIGHT           = 0x8,
    AUDIO_CHANNEL_IN_FRONT           = 0x10,
    AUDIO_CHANNEL_IN_BACK            = 0x20,
    AUDIO_CHANNEL_IN_LEFT_PROCESSED  = 0x40,
    AUDIO_CHANNEL_IN_RIGHT_PROCESSED = 0x80,
    AUDIO_CHANNEL_IN_FRONT_PROCESSED = 0x100,
    AUDIO_CHANNEL_IN_BACK_PROCESSED  = 0x200,
    AUDIO_CHANNEL_IN_PRESSURE        = 0x400,
    AUDIO_CHANNEL_IN_X_AXIS          = 0x800,
    AUDIO_CHANNEL_IN_Y_AXIS          = 0x1000,
    AUDIO_CHANNEL_IN_Z_AXIS          = 0x2000,
    AUDIO_CHANNEL_IN_VOICE_UPLINK    = 0x4000,
    AUDIO_CHANNEL_IN_VOICE_DNLINK    = 0x8000,

/* TODO: should these be considered complete channel masks, or only bits, or deprecated? */

    AUDIO_CHANNEL_IN_MONO   = AUDIO_CHANNEL_IN_FRONT,
    AUDIO_CHANNEL_IN_STEREO = (AUDIO_CHANNEL_IN_LEFT | AUDIO_CHANNEL_IN_RIGHT),
    AUDIO_CHANNEL_IN_FRONT_BACK = (AUDIO_CHANNEL_IN_FRONT | AUDIO_CHANNEL_IN_BACK),
    AUDIO_CHANNEL_IN_ALL    = (AUDIO_CHANNEL_IN_LEFT |
                               AUDIO_CHANNEL_IN_RIGHT |
                               AUDIO_CHANNEL_IN_FRONT |
                               AUDIO_CHANNEL_IN_BACK|
                               AUDIO_CHANNEL_IN_LEFT_PROCESSED |
                               AUDIO_CHANNEL_IN_RIGHT_PROCESSED |
                               AUDIO_CHANNEL_IN_FRONT_PROCESSED |
                               AUDIO_CHANNEL_IN_BACK_PROCESSED|
                               AUDIO_CHANNEL_IN_PRESSURE |
                               AUDIO_CHANNEL_IN_X_AXIS |
                               AUDIO_CHANNEL_IN_Y_AXIS |
                               AUDIO_CHANNEL_IN_Z_AXIS |
                               AUDIO_CHANNEL_IN_VOICE_UPLINK |
                               AUDIO_CHANNEL_IN_VOICE_DNLINK),
};

/* A channel mask per se only defines the presence or absence of a channel, not the order.
 * But see AUDIO_INTERLEAVE_* below for the platform convention of order.
 *
 * audio_channel_mask_t is an opaque type and its internal layout should not
 * be assumed as it may change in the future.
 * Instead, always use the functions declared in this header to examine.
 *
 * These are the current representations:
 *
 *   AUDIO_CHANNEL_REPRESENTATION_POSITION
 *     is a channel mask representation for position assignment.
 *     Each low-order bit corresponds to the spatial position of a transducer (output),
 *     or interpretation of channel (input).
 *     The user of a channel mask needs to know the context of whether it is for output or input.
 *     The constants AUDIO_CHANNEL_OUT_* or AUDIO_CHANNEL_IN_* apply to the bits portion.
 *     It is not permitted for no bits to be set.
 *
 *   AUDIO_CHANNEL_REPRESENTATION_INDEX
 *     is a channel mask representation for index assignment.
 *     Each low-order bit corresponds to a selected channel.
 *     There is no platform interpretation of the various bits.
 *     There is no concept of output or input.
 *     It is not permitted for no bits to be set.
 *
 * All other representations are reserved for future use.
 *
 * Warning: current representation distinguishes between input and output, but this will not the be
 * case in future revisions of the platform. Wherever there is an ambiguity between input and output
 * that is currently resolved by checking the channel mask, the implementer should look for ways to
 * fix it with additional information outside of the mask.
 */
typedef uint32_t audio_channel_mask_t;

/* Maximum number of channels for all representations */
#define AUDIO_CHANNEL_COUNT_MAX             30

/* log(2) of maximum number of representations, not part of public API */
#define AUDIO_CHANNEL_REPRESENTATION_LOG2   2

/* Representations */
typedef enum {
    AUDIO_CHANNEL_REPRESENTATION_POSITION    = 0,    // must be zero for compatibility
    // 1 is reserved for future use
    AUDIO_CHANNEL_REPRESENTATION_INDEX       = 2,
    // 3 is reserved for future use
} audio_channel_representation_t;

/* The return value is undefined if the channel mask is invalid. */
static inline uint32_t audio_channel_mask_get_bits(audio_channel_mask_t channel)
{
    return channel & ((1 << AUDIO_CHANNEL_COUNT_MAX) - 1);
}

/* The return value is undefined if the channel mask is invalid. */
static inline audio_channel_representation_t audio_channel_mask_get_representation(
        audio_channel_mask_t channel)
{
    // The right shift should be sufficient, but also "and" for safety in case mask is not 32 bits
    return (audio_channel_representation_t)
            ((channel >> AUDIO_CHANNEL_COUNT_MAX) & ((1 << AUDIO_CHANNEL_REPRESENTATION_LOG2) - 1));
}

/* Returns true if the channel mask is valid,
 * or returns false for AUDIO_CHANNEL_NONE, AUDIO_CHANNEL_INVALID, and other invalid values.
 * This function is unable to determine whether a channel mask for position assignment
 * is invalid because an output mask has an invalid output bit set,
 * or because an input mask has an invalid input bit set.
 * All other APIs that take a channel mask assume that it is valid.
 */
static inline bool audio_channel_mask_is_valid(audio_channel_mask_t channel)
{
    uint32_t bits = audio_channel_mask_get_bits(channel);
    audio_channel_representation_t representation = audio_channel_mask_get_representation(channel);
    switch (representation) {
    case AUDIO_CHANNEL_REPRESENTATION_POSITION:
    case AUDIO_CHANNEL_REPRESENTATION_INDEX:
        break;
    default:
        bits = 0;
        break;
    }
    return bits != 0;
}

/* Not part of public API */
static inline audio_channel_mask_t audio_channel_mask_from_representation_and_bits(
        audio_channel_representation_t representation, uint32_t bits)
{
    return (audio_channel_mask_t) ((representation << AUDIO_CHANNEL_COUNT_MAX) | bits);
}

/* Expresses the convention when stereo audio samples are stored interleaved
 * in an array.  This should improve readability by allowing code to use
 * symbolic indices instead of hard-coded [0] and [1].
 *
 * For multi-channel beyond stereo, the platform convention is that channels
 * are interleaved in order from least significant channel mask bit
 * to most significant channel mask bit, with unused bits skipped.
 * Any exceptions to this convention will be noted at the appropriate API.
 */
enum {
    AUDIO_INTERLEAVE_LEFT   = 0,
    AUDIO_INTERLEAVE_RIGHT  = 1,
};

typedef enum {
    AUDIO_MODE_INVALID          = -2,
    AUDIO_MODE_CURRENT          = -1,
    AUDIO_MODE_NORMAL           = 0,
    AUDIO_MODE_RINGTONE         = 1,
    AUDIO_MODE_IN_CALL          = 2,
    AUDIO_MODE_IN_COMMUNICATION = 3,

    AUDIO_MODE_CNT,
    AUDIO_MODE_MAX              = AUDIO_MODE_CNT - 1,
} audio_mode_t;

/* This enum is deprecated */
typedef enum {
    AUDIO_IN_ACOUSTICS_NONE          = 0,
    AUDIO_IN_ACOUSTICS_AGC_ENABLE    = 0x0001,
    AUDIO_IN_ACOUSTICS_AGC_DISABLE   = 0,
    AUDIO_IN_ACOUSTICS_NS_ENABLE     = 0x0002,
    AUDIO_IN_ACOUSTICS_NS_DISABLE    = 0,
    AUDIO_IN_ACOUSTICS_TX_IIR_ENABLE = 0x0004,
    AUDIO_IN_ACOUSTICS_TX_DISABLE    = 0,
} audio_in_acoustics_t;

enum {
    AUDIO_DEVICE_NONE                          = 0x0,
    /* reserved bits */
    AUDIO_DEVICE_BIT_IN                        = 0x80000000,
    AUDIO_DEVICE_BIT_DEFAULT                   = 0x40000000,
    /* output devices */
    AUDIO_DEVICE_OUT_EARPIECE                  = 0x1,
    AUDIO_DEVICE_OUT_SPEAKER                   = 0x2,
    AUDIO_DEVICE_OUT_WIRED_HEADSET             = 0x4,
    AUDIO_DEVICE_OUT_WIRED_HEADPHONE           = 0x8,
    AUDIO_DEVICE_OUT_BLUETOOTH_SCO             = 0x10,
    AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET     = 0x20,
    AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT      = 0x40,
    AUDIO_DEVICE_OUT_BLUETOOTH_A2DP            = 0x80,
    AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES = 0x100,
    AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER    = 0x200,
    AUDIO_DEVICE_OUT_AUX_DIGITAL               = 0x400,
    AUDIO_DEVICE_OUT_HDMI                      = AUDIO_DEVICE_OUT_AUX_DIGITAL,
    /* uses an analog connection (multiplexed over the USB connector pins for instance) */
    AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET         = 0x800,
    AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET         = 0x1000,
    /* USB accessory mode: your Android device is a USB device and the dock is a USB host */
    AUDIO_DEVICE_OUT_USB_ACCESSORY             = 0x2000,
    /* USB host mode: your Android device is a USB host and the dock is a USB device */
    AUDIO_DEVICE_OUT_USB_DEVICE                = 0x4000,
    AUDIO_DEVICE_OUT_REMOTE_SUBMIX             = 0x8000,
    /* Telephony voice TX path */
    AUDIO_DEVICE_OUT_TELEPHONY_TX              = 0x10000,
    /* Analog jack with line impedance detected */
    AUDIO_DEVICE_OUT_LINE                      = 0x20000,
    /* HDMI Audio Return Channel */
    AUDIO_DEVICE_OUT_HDMI_ARC                  = 0x40000,
    /* S/PDIF out */
    AUDIO_DEVICE_OUT_SPDIF                     = 0x80000,
    /* FM transmitter out */
    AUDIO_DEVICE_OUT_FM                        = 0x100000,
    /* Line out for av devices */
    AUDIO_DEVICE_OUT_AUX_LINE                  = 0x200000,
    /* limited-output speaker device for acoustic safety */
    AUDIO_DEVICE_OUT_SPEAKER_SAFE              = 0x400000,
    AUDIO_DEVICE_OUT_DEFAULT                   = AUDIO_DEVICE_BIT_DEFAULT,
    AUDIO_DEVICE_OUT_ALL      = (AUDIO_DEVICE_OUT_EARPIECE |
                                 AUDIO_DEVICE_OUT_SPEAKER |
                                 AUDIO_DEVICE_OUT_WIRED_HEADSET |
                                 AUDIO_DEVICE_OUT_WIRED_HEADPHONE |
                                 AUDIO_DEVICE_OUT_BLUETOOTH_SCO |
                                 AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET |
                                 AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT |
                                 AUDIO_DEVICE_OUT_BLUETOOTH_A2DP |
                                 AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES |
                                 AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER |
                                 AUDIO_DEVICE_OUT_HDMI |
                                 AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET |
                                 AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET |
                                 AUDIO_DEVICE_OUT_USB_ACCESSORY |
                                 AUDIO_DEVICE_OUT_USB_DEVICE |
                                 AUDIO_DEVICE_OUT_REMOTE_SUBMIX |
                                 AUDIO_DEVICE_OUT_TELEPHONY_TX |
                                 AUDIO_DEVICE_OUT_LINE |
                                 AUDIO_DEVICE_OUT_HDMI_ARC |
                                 AUDIO_DEVICE_OUT_SPDIF |
                                 AUDIO_DEVICE_OUT_FM |
                                 AUDIO_DEVICE_OUT_AUX_LINE |
                                 AUDIO_DEVICE_OUT_SPEAKER_SAFE |
                                 AUDIO_DEVICE_OUT_DEFAULT),
    AUDIO_DEVICE_OUT_ALL_A2DP = (AUDIO_DEVICE_OUT_BLUETOOTH_A2DP |
                                 AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES |
                                 AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER),
    AUDIO_DEVICE_OUT_ALL_SCO  = (AUDIO_DEVICE_OUT_BLUETOOTH_SCO |
                                 AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET |
                                 AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT),
    AUDIO_DEVICE_OUT_ALL_USB  = (AUDIO_DEVICE_OUT_USB_ACCESSORY |
                                 AUDIO_DEVICE_OUT_USB_DEVICE),

    /* input devices */
    AUDIO_DEVICE_IN_COMMUNICATION         = AUDIO_DEVICE_BIT_IN | 0x1,
    AUDIO_DEVICE_IN_AMBIENT               = AUDIO_DEVICE_BIT_IN | 0x2,
    AUDIO_DEVICE_IN_BUILTIN_MIC           = AUDIO_DEVICE_BIT_IN | 0x4,
    AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET = AUDIO_DEVICE_BIT_IN | 0x8,
    AUDIO_DEVICE_IN_WIRED_HEADSET         = AUDIO_DEVICE_BIT_IN | 0x10,
    AUDIO_DEVICE_IN_AUX_DIGITAL           = AUDIO_DEVICE_BIT_IN | 0x20,
    AUDIO_DEVICE_IN_HDMI                  = AUDIO_DEVICE_IN_AUX_DIGITAL,
    /* Telephony voice RX path */
    AUDIO_DEVICE_IN_VOICE_CALL            = AUDIO_DEVICE_BIT_IN | 0x40,
    AUDIO_DEVICE_IN_TELEPHONY_RX          = AUDIO_DEVICE_IN_VOICE_CALL,
    AUDIO_DEVICE_IN_BACK_MIC              = AUDIO_DEVICE_BIT_IN | 0x80,
    AUDIO_DEVICE_IN_REMOTE_SUBMIX         = AUDIO_DEVICE_BIT_IN | 0x100,
    AUDIO_DEVICE_IN_ANLG_DOCK_HEADSET     = AUDIO_DEVICE_BIT_IN | 0x200,
    AUDIO_DEVICE_IN_DGTL_DOCK_HEADSET     = AUDIO_DEVICE_BIT_IN | 0x400,
    AUDIO_DEVICE_IN_USB_ACCESSORY         = AUDIO_DEVICE_BIT_IN | 0x800,
    AUDIO_DEVICE_IN_USB_DEVICE            = AUDIO_DEVICE_BIT_IN | 0x1000,
    /* FM tuner input */
    AUDIO_DEVICE_IN_FM_TUNER              = AUDIO_DEVICE_BIT_IN | 0x2000,
    /* TV tuner input */
    AUDIO_DEVICE_IN_TV_TUNER              = AUDIO_DEVICE_BIT_IN | 0x4000,
    /* Analog jack with line impedance detected */
    AUDIO_DEVICE_IN_LINE                  = AUDIO_DEVICE_BIT_IN | 0x8000,
    /* S/PDIF in */
    AUDIO_DEVICE_IN_SPDIF                 = AUDIO_DEVICE_BIT_IN | 0x10000,
    AUDIO_DEVICE_IN_BLUETOOTH_A2DP        = AUDIO_DEVICE_BIT_IN | 0x20000,
    AUDIO_DEVICE_IN_LOOPBACK              = AUDIO_DEVICE_BIT_IN | 0x40000,
    AUDIO_DEVICE_IN_DEFAULT               = AUDIO_DEVICE_BIT_IN | AUDIO_DEVICE_BIT_DEFAULT,

    AUDIO_DEVICE_IN_ALL     = (AUDIO_DEVICE_IN_COMMUNICATION |
                               AUDIO_DEVICE_IN_AMBIENT |
                               AUDIO_DEVICE_IN_BUILTIN_MIC |
                               AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET |
                               AUDIO_DEVICE_IN_WIRED_HEADSET |
                               AUDIO_DEVICE_IN_HDMI |
                               AUDIO_DEVICE_IN_TELEPHONY_RX |
                               AUDIO_DEVICE_IN_BACK_MIC |
                               AUDIO_DEVICE_IN_REMOTE_SUBMIX |
                               AUDIO_DEVICE_IN_ANLG_DOCK_HEADSET |
                               AUDIO_DEVICE_IN_DGTL_DOCK_HEADSET |
                               AUDIO_DEVICE_IN_USB_ACCESSORY |
                               AUDIO_DEVICE_IN_USB_DEVICE |
                               AUDIO_DEVICE_IN_FM_TUNER |
                               AUDIO_DEVICE_IN_TV_TUNER |
                               AUDIO_DEVICE_IN_LINE |
                               AUDIO_DEVICE_IN_SPDIF |
                               AUDIO_DEVICE_IN_BLUETOOTH_A2DP |
                               AUDIO_DEVICE_IN_LOOPBACK |
                               AUDIO_DEVICE_IN_DEFAULT),
    AUDIO_DEVICE_IN_ALL_SCO = AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET,
    AUDIO_DEVICE_IN_ALL_USB  = (AUDIO_DEVICE_IN_USB_ACCESSORY |
                                AUDIO_DEVICE_IN_USB_DEVICE),
};

typedef uint32_t audio_devices_t;

/* the audio output flags serve two purposes:
 * - when an AudioTrack is created they indicate a "wish" to be connected to an
 * output stream with attributes corresponding to the specified flags
 * - when present in an output profile descriptor listed for a particular audio
 * hardware module, they indicate that an output stream can be opened that
 * supports the attributes indicated by the flags.
 * the audio policy manager will try to match the flags in the request
 * (when getOuput() is called) to an available output stream.
 */
typedef enum {
    AUDIO_OUTPUT_FLAG_NONE = 0x0,       // no attributes
    AUDIO_OUTPUT_FLAG_DIRECT = 0x1,     // this output directly connects a track
                                        // to one output stream: no software mixer
    AUDIO_OUTPUT_FLAG_PRIMARY = 0x2,    // this output is the primary output of
                                        // the device. It is unique and must be
                                        // present. It is opened by default and
                                        // receives routing, audio mode and volume
                                        // controls related to voice calls.
    AUDIO_OUTPUT_FLAG_FAST = 0x4,       // output supports "fast tracks",
                                        // defined elsewhere
    AUDIO_OUTPUT_FLAG_DEEP_BUFFER = 0x8, // use deep audio buffers
    AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD = 0x10,  // offload playback of compressed
                                                // streams to hardware codec
    AUDIO_OUTPUT_FLAG_NON_BLOCKING = 0x20, // use non-blocking write
    AUDIO_OUTPUT_FLAG_HW_AV_SYNC = 0x40 // output uses a hardware A/V synchronization source
} audio_output_flags_t;

/* The audio input flags are analogous to audio output flags.
 * Currently they are used only when an AudioRecord is created,
 * to indicate a preference to be connected to an input stream with
 * attributes corresponding to the specified flags.
 */
typedef enum {
    AUDIO_INPUT_FLAG_NONE       = 0x0,  // no attributes
    AUDIO_INPUT_FLAG_FAST       = 0x1,  // prefer an input that supports "fast tracks"
    AUDIO_INPUT_FLAG_HW_HOTWORD = 0x2,  // prefer an input that captures from hw hotword source
} audio_input_flags_t;

/* Additional information about compressed streams offloaded to
 * hardware playback
 * The version and size fields must be initialized by the caller by using
 * one of the constants defined here.
 */
typedef struct {
    uint16_t version;                   // version of the info structure
    uint16_t size;                      // total size of the structure including version and size
    uint32_t sample_rate;               // sample rate in Hz
    audio_channel_mask_t channel_mask;  // channel mask
    audio_format_t format;              // audio format
    audio_stream_type_t stream_type;    // stream type
    uint32_t bit_rate;                  // bit rate in bits per second
    int64_t duration_us;                // duration in microseconds, -1 if unknown
    bool has_video;                     // true if stream is tied to a video stream
    bool is_streaming;                  // true if streaming, false if local playback
} audio_offload_info_t;

#define AUDIO_MAKE_OFFLOAD_INFO_VERSION(maj,min) \
            ((((maj) & 0xff) << 8) | ((min) & 0xff))

#define AUDIO_OFFLOAD_INFO_VERSION_0_1 AUDIO_MAKE_OFFLOAD_INFO_VERSION(0, 1)
#define AUDIO_OFFLOAD_INFO_VERSION_CURRENT AUDIO_OFFLOAD_INFO_VERSION_0_1

static const audio_offload_info_t AUDIO_INFO_INITIALIZER = {
    version: AUDIO_OFFLOAD_INFO_VERSION_CURRENT,
    size: sizeof(audio_offload_info_t),
    sample_rate: 0,
    channel_mask: 0,
    format: AUDIO_FORMAT_DEFAULT,
    stream_type: AUDIO_STREAM_VOICE_CALL,
    bit_rate: 0,
    duration_us: 0,
    has_video: false,
    is_streaming: false
};

/* common audio stream configuration parameters
 * You should memset() the entire structure to zero before use to
 * ensure forward compatibility
 */
struct audio_config {
    uint32_t sample_rate;
    audio_channel_mask_t channel_mask;
    audio_format_t  format;
    audio_offload_info_t offload_info;
    size_t frame_count;
};
typedef struct audio_config audio_config_t;

static const audio_config_t AUDIO_CONFIG_INITIALIZER = {
    sample_rate: 0,
    channel_mask: AUDIO_CHANNEL_NONE,
    format: AUDIO_FORMAT_DEFAULT,
    offload_info: {
        version: AUDIO_OFFLOAD_INFO_VERSION_CURRENT,
        size: sizeof(audio_offload_info_t),
        sample_rate: 0,
        channel_mask: 0,
        format: AUDIO_FORMAT_DEFAULT,
        stream_type: AUDIO_STREAM_VOICE_CALL,
        bit_rate: 0,
        duration_us: 0,
        has_video: false,
        is_streaming: false
    },
    frame_count: 0,
};


/* audio hw module handle functions or structures referencing a module */
typedef int audio_module_handle_t;

/******************************
 *  Volume control
 *****************************/

/* If the audio hardware supports gain control on some audio paths,
 * the platform can expose them in the audio_policy.conf file. The audio HAL
 * will then implement gain control functions that will use the following data
 * structures. */

/* Type of gain control exposed by an audio port */
#define AUDIO_GAIN_MODE_JOINT     0x1 /* supports joint channel gain control */
#define AUDIO_GAIN_MODE_CHANNELS  0x2 /* supports separate channel gain control */
#define AUDIO_GAIN_MODE_RAMP      0x4 /* supports gain ramps */

typedef uint32_t audio_gain_mode_t;


/* An audio_gain struct is a representation of a gain stage.
 * A gain stage is always attached to an audio port. */
struct audio_gain  {
    audio_gain_mode_t    mode;          /* e.g. AUDIO_GAIN_MODE_JOINT */
    audio_channel_mask_t channel_mask;  /* channels which gain an be controlled.
                                           N/A if AUDIO_GAIN_MODE_CHANNELS is not supported */
    int                  min_value;     /* minimum gain value in millibels */
    int                  max_value;     /* maximum gain value in millibels */
    int                  default_value; /* default gain value in millibels */
    unsigned int         step_value;    /* gain step in millibels */
    unsigned int         min_ramp_ms;   /* minimum ramp duration in ms */
    unsigned int         max_ramp_ms;   /* maximum ramp duration in ms */
};

/* The gain configuration structure is used to get or set the gain values of a
 * given port */
struct audio_gain_config  {
    int                  index;             /* index of the corresponding audio_gain in the
                                               audio_port gains[] table */
    audio_gain_mode_t    mode;              /* mode requested for this command */
    audio_channel_mask_t channel_mask;      /* channels which gain value follows.
                                               N/A in joint mode */
    int                  values[sizeof(audio_channel_mask_t) * 8]; /* gain values in millibels
                                               for each channel ordered from LSb to MSb in
                                               channel mask. The number of values is 1 in joint
                                               mode or popcount(channel_mask) */
    unsigned int         ramp_duration_ms; /* ramp duration in ms */
};

/******************************
 *  Routing control
 *****************************/

/* Types defined here are used to describe an audio source or sink at internal
 * framework interfaces (audio policy, patch panel) or at the audio HAL.
 * Sink and sources are grouped in a concept of “audio port” representing an
 * audio end point at the edge of the system managed by the module exposing
 * the interface. */

/* Audio port role: either source or sink */
typedef enum {
    AUDIO_PORT_ROLE_NONE,
    AUDIO_PORT_ROLE_SOURCE,
    AUDIO_PORT_ROLE_SINK,
} audio_port_role_t;

/* Audio port type indicates if it is a session (e.g AudioTrack),
 * a mix (e.g PlaybackThread output) or a physical device
 * (e.g AUDIO_DEVICE_OUT_SPEAKER) */
typedef enum {
    AUDIO_PORT_TYPE_NONE,
    AUDIO_PORT_TYPE_DEVICE,
    AUDIO_PORT_TYPE_MIX,
    AUDIO_PORT_TYPE_SESSION,
} audio_port_type_t;

/* Each port has a unique ID or handle allocated by policy manager */
typedef int audio_port_handle_t;
#define AUDIO_PORT_HANDLE_NONE 0


/* maximum audio device address length */
#define AUDIO_DEVICE_MAX_ADDRESS_LEN 32

/* extension for audio port configuration structure when the audio port is a
 * hardware device */
struct audio_port_config_device_ext {
    audio_module_handle_t hw_module;                /* module the device is attached to */
    audio_devices_t       type;                     /* device type (e.g AUDIO_DEVICE_OUT_SPEAKER) */
    char                  address[AUDIO_DEVICE_MAX_ADDRESS_LEN]; /* device address. "" if N/A */
};

/* extension for audio port configuration structure when the audio port is a
 * sub mix */
struct audio_port_config_mix_ext {
    audio_module_handle_t hw_module;    /* module the stream is attached to */
    audio_io_handle_t handle;           /* I/O handle of the input/output stream */
    union {
        //TODO: change use case for output streams: use strategy and mixer attributes
        audio_stream_type_t stream;
        audio_source_t      source;
    } usecase;
};

/* extension for audio port configuration structure when the audio port is an
 * audio session */
struct audio_port_config_session_ext {
    audio_session_t   session; /* audio session */
};

/* Flags indicating which fields are to be considered in struct audio_port_config */
#define AUDIO_PORT_CONFIG_SAMPLE_RATE  0x1
#define AUDIO_PORT_CONFIG_CHANNEL_MASK 0x2
#define AUDIO_PORT_CONFIG_FORMAT       0x4
#define AUDIO_PORT_CONFIG_GAIN         0x8
#define AUDIO_PORT_CONFIG_ALL (AUDIO_PORT_CONFIG_SAMPLE_RATE | \
                               AUDIO_PORT_CONFIG_CHANNEL_MASK | \
                               AUDIO_PORT_CONFIG_FORMAT | \
                               AUDIO_PORT_CONFIG_GAIN)

/* audio port configuration structure used to specify a particular configuration of
 * an audio port */
struct audio_port_config {
    audio_port_handle_t      id;           /* port unique ID */
    audio_port_role_t        role;         /* sink or source */
    audio_port_type_t        type;         /* device, mix ... */
    unsigned int             config_mask;  /* e.g AUDIO_PORT_CONFIG_ALL */
    unsigned int             sample_rate;  /* sampling rate in Hz */
    audio_channel_mask_t     channel_mask; /* channel mask if applicable */
    audio_format_t           format;       /* format if applicable */
    struct audio_gain_config gain;         /* gain to apply if applicable */
    union {
        struct audio_port_config_device_ext  device;  /* device specific info */
        struct audio_port_config_mix_ext     mix;     /* mix specific info */
        struct audio_port_config_session_ext session; /* session specific info */
    } ext;
};


/* max number of sampling rates in audio port */
#define AUDIO_PORT_MAX_SAMPLING_RATES 16
/* max number of channel masks in audio port */
#define AUDIO_PORT_MAX_CHANNEL_MASKS 16
/* max number of audio formats in audio port */
#define AUDIO_PORT_MAX_FORMATS 16
/* max number of gain controls in audio port */
#define AUDIO_PORT_MAX_GAINS 16

/* extension for audio port structure when the audio port is a hardware device */
struct audio_port_device_ext {
    audio_module_handle_t hw_module;    /* module the device is attached to */
    audio_devices_t       type;         /* device type (e.g AUDIO_DEVICE_OUT_SPEAKER) */
    char                  address[AUDIO_DEVICE_MAX_ADDRESS_LEN];
};

/* Latency class of the audio mix */
typedef enum {
    AUDIO_LATENCY_LOW,
    AUDIO_LATENCY_NORMAL,
} audio_mix_latency_class_t;

/* extension for audio port structure when the audio port is a sub mix */
struct audio_port_mix_ext {
    audio_module_handle_t     hw_module;     /* module the stream is attached to */
    audio_io_handle_t         handle;        /* I/O handle of the input.output stream */
    audio_mix_latency_class_t latency_class; /* latency class */
    // other attributes: routing strategies
};

/* extension for audio port structure when the audio port is an audio session */
struct audio_port_session_ext {
    audio_session_t   session; /* audio session */
};


struct audio_port {
    audio_port_handle_t      id;                /* port unique ID */
    audio_port_role_t        role;              /* sink or source */
    audio_port_type_t        type;              /* device, mix ... */
    unsigned int             num_sample_rates;  /* number of sampling rates in following array */
    unsigned int             sample_rates[AUDIO_PORT_MAX_SAMPLING_RATES];
    unsigned int             num_channel_masks; /* number of channel masks in following array */
    audio_channel_mask_t     channel_masks[AUDIO_PORT_MAX_CHANNEL_MASKS];
    unsigned int             num_formats;       /* number of formats in following array */
    audio_format_t           formats[AUDIO_PORT_MAX_FORMATS];
    unsigned int             num_gains;         /* number of gains in following array */
    struct audio_gain        gains[AUDIO_PORT_MAX_GAINS];
    struct audio_port_config active_config;     /* current audio port configuration */
    union {
        struct audio_port_device_ext  device;
        struct audio_port_mix_ext     mix;
        struct audio_port_session_ext session;
    } ext;
};

/* An audio patch represents a connection between one or more source ports and
 * one or more sink ports. Patches are connected and disconnected by audio policy manager or by
 * applications via framework APIs.
 * Each patch is identified by a handle at the interface used to create that patch. For instance,
 * when a patch is created by the audio HAL, the HAL allocates and returns a handle.
 * This handle is unique to a given audio HAL hardware module.
 * But the same patch receives another system wide unique handle allocated by the framework.
 * This unique handle is used for all transactions inside the framework.
 */
typedef int audio_patch_handle_t;
#define AUDIO_PATCH_HANDLE_NONE 0

#define AUDIO_PATCH_PORTS_MAX   16

struct audio_patch {
    audio_patch_handle_t id;            /* patch unique ID */
    unsigned int      num_sources;      /* number of sources in following array */
    struct audio_port_config sources[AUDIO_PATCH_PORTS_MAX];
    unsigned int      num_sinks;        /* number of sinks in following array */
    struct audio_port_config sinks[AUDIO_PATCH_PORTS_MAX];
};



/* a HW synchronization source returned by the audio HAL */
typedef uint32_t audio_hw_sync_t;

/* an invalid HW synchronization source indicating an error */
#define AUDIO_HW_SYNC_INVALID 0

static inline bool audio_is_output_device(audio_devices_t device)
{
    if (((device & AUDIO_DEVICE_BIT_IN) == 0) &&
            (popcount(device) == 1) && ((device & ~AUDIO_DEVICE_OUT_ALL) == 0))
        return true;
    else
        return false;
}

static inline bool audio_is_input_device(audio_devices_t device)
{
    if ((device & AUDIO_DEVICE_BIT_IN) != 0) {
        device &= ~AUDIO_DEVICE_BIT_IN;
        if ((popcount(device) == 1) && ((device & ~AUDIO_DEVICE_IN_ALL) == 0))
            return true;
    }
    return false;
}

static inline bool audio_is_output_devices(audio_devices_t device)
{
    return (device & AUDIO_DEVICE_BIT_IN) == 0;
}

static inline bool audio_is_a2dp_in_device(audio_devices_t device)
{
    if ((device & AUDIO_DEVICE_BIT_IN) != 0) {
        device &= ~AUDIO_DEVICE_BIT_IN;
        if ((popcount(device) == 1) && (device & AUDIO_DEVICE_IN_BLUETOOTH_A2DP))
            return true;
    }
    return false;
}

static inline bool audio_is_a2dp_out_device(audio_devices_t device)
{
    if ((popcount(device) == 1) && (device & AUDIO_DEVICE_OUT_ALL_A2DP))
        return true;
    else
        return false;
}

// Deprecated - use audio_is_a2dp_out_device() instead
static inline bool audio_is_a2dp_device(audio_devices_t device)
{
    return audio_is_a2dp_out_device(device);
}

static inline bool audio_is_bluetooth_sco_device(audio_devices_t device)
{
    if ((device & AUDIO_DEVICE_BIT_IN) == 0) {
        if ((popcount(device) == 1) && ((device & ~AUDIO_DEVICE_OUT_ALL_SCO) == 0))
            return true;
    } else {
        device &= ~AUDIO_DEVICE_BIT_IN;
        if ((popcount(device) == 1) && ((device & ~AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET) == 0))
            return true;
    }

    return false;
}

static inline bool audio_is_usb_out_device(audio_devices_t device)
{
    return ((popcount(device) == 1) && (device & AUDIO_DEVICE_OUT_ALL_USB));
}

static inline bool audio_is_usb_in_device(audio_devices_t device)
{
    if ((device & AUDIO_DEVICE_BIT_IN) != 0) {
        device &= ~AUDIO_DEVICE_BIT_IN;
        if (popcount(device) == 1 && (device & AUDIO_DEVICE_IN_ALL_USB) != 0)
            return true;
    }
    return false;
}

/* OBSOLETE - use audio_is_usb_out_device() instead. */
static inline bool audio_is_usb_device(audio_devices_t device)
{
    return audio_is_usb_out_device(device);
}

static inline bool audio_is_remote_submix_device(audio_devices_t device)
{
    if ((device & AUDIO_DEVICE_OUT_REMOTE_SUBMIX) == AUDIO_DEVICE_OUT_REMOTE_SUBMIX
            || (device & AUDIO_DEVICE_IN_REMOTE_SUBMIX) == AUDIO_DEVICE_IN_REMOTE_SUBMIX)
        return true;
    else
        return false;
}

/* Returns true if:
 *  representation is valid, and
 *  there is at least one channel bit set which _could_ correspond to an input channel, and
 *  there are no channel bits set which could _not_ correspond to an input channel.
 * Otherwise returns false.
 */
static inline bool audio_is_input_channel(audio_channel_mask_t channel)
{
    uint32_t bits = audio_channel_mask_get_bits(channel);
    switch (audio_channel_mask_get_representation(channel)) {
    case AUDIO_CHANNEL_REPRESENTATION_POSITION:
        if (bits & ~AUDIO_CHANNEL_IN_ALL) {
            bits = 0;
        }
        // fall through
    case AUDIO_CHANNEL_REPRESENTATION_INDEX:
        return bits != 0;
    default:
        return false;
    }
}

/* Returns true if:
 *  representation is valid, and
 *  there is at least one channel bit set which _could_ correspond to an output channel, and
 *  there are no channel bits set which could _not_ correspond to an output channel.
 * Otherwise returns false.
 */
static inline bool audio_is_output_channel(audio_channel_mask_t channel)
{
    uint32_t bits = audio_channel_mask_get_bits(channel);
    switch (audio_channel_mask_get_representation(channel)) {
    case AUDIO_CHANNEL_REPRESENTATION_POSITION:
        if (bits & ~AUDIO_CHANNEL_OUT_ALL) {
            bits = 0;
        }
        // fall through
    case AUDIO_CHANNEL_REPRESENTATION_INDEX:
        return bits != 0;
    default:
        return false;
    }
}

/* Returns the number of channels from an input channel mask,
 * used in the context of audio input or recording.
 * If a channel bit is set which could _not_ correspond to an input channel,
 * it is excluded from the count.
 * Returns zero if the representation is invalid.
 */
static inline uint32_t audio_channel_count_from_in_mask(audio_channel_mask_t channel)
{
    uint32_t bits = audio_channel_mask_get_bits(channel);
    switch (audio_channel_mask_get_representation(channel)) {
    case AUDIO_CHANNEL_REPRESENTATION_POSITION:
        // TODO: We can now merge with from_out_mask and remove anding
        bits &= AUDIO_CHANNEL_IN_ALL;
        // fall through
    case AUDIO_CHANNEL_REPRESENTATION_INDEX:
        return popcount(bits);
    default:
        return 0;
    }
}

/* Returns the number of channels from an output channel mask,
 * used in the context of audio output or playback.
 * If a channel bit is set which could _not_ correspond to an output channel,
 * it is excluded from the count.
 * Returns zero if the representation is invalid.
 */
static inline uint32_t audio_channel_count_from_out_mask(audio_channel_mask_t channel)
{
    uint32_t bits = audio_channel_mask_get_bits(channel);
    switch (audio_channel_mask_get_representation(channel)) {
    case AUDIO_CHANNEL_REPRESENTATION_POSITION:
        // TODO: We can now merge with from_in_mask and remove anding
        bits &= AUDIO_CHANNEL_OUT_ALL;
        // fall through
    case AUDIO_CHANNEL_REPRESENTATION_INDEX:
        return popcount(bits);
    default:
        return 0;
    }
}

/* Derive an output channel mask for position assignment from a channel count.
 * This is to be used when the content channel mask is unknown. The 1, 2, 4, 5, 6, 7 and 8 channel
 * cases are mapped to the standard game/home-theater layouts, but note that 4 is mapped to quad,
 * and not stereo + FC + mono surround. A channel count of 3 is arbitrarily mapped to stereo + FC
 * for continuity with stereo.
 * Returns the matching channel mask,
 * or AUDIO_CHANNEL_NONE if the channel count is zero,
 * or AUDIO_CHANNEL_INVALID if the channel count exceeds that of the
 * configurations for which a default output channel mask is defined.
 */
static inline audio_channel_mask_t audio_channel_out_mask_from_count(uint32_t channel_count)
{
    uint32_t bits;
    switch (channel_count) {
    case 0:
        return AUDIO_CHANNEL_NONE;
    case 1:
        bits = AUDIO_CHANNEL_OUT_MONO;
        break;
    case 2:
        bits = AUDIO_CHANNEL_OUT_STEREO;
        break;
    case 3:
        bits = AUDIO_CHANNEL_OUT_STEREO | AUDIO_CHANNEL_OUT_FRONT_CENTER;
        break;
    case 4: // 4.0
        bits = AUDIO_CHANNEL_OUT_QUAD;
        break;
    case 5: // 5.0
        bits = AUDIO_CHANNEL_OUT_QUAD | AUDIO_CHANNEL_OUT_FRONT_CENTER;
        break;
    case 6: // 5.1
        bits = AUDIO_CHANNEL_OUT_5POINT1;
        break;
    case 7: // 6.1
        bits = AUDIO_CHANNEL_OUT_5POINT1 | AUDIO_CHANNEL_OUT_BACK_CENTER;
        break;
    case 8:
        bits = AUDIO_CHANNEL_OUT_7POINT1;
        break;
    default:
        return AUDIO_CHANNEL_INVALID;
    }
    return audio_channel_mask_from_representation_and_bits(
            AUDIO_CHANNEL_REPRESENTATION_POSITION, bits);
}

/* Derive an input channel mask for position assignment from a channel count.
 * Currently handles only mono and stereo.
 * Returns the matching channel mask,
 * or AUDIO_CHANNEL_NONE if the channel count is zero,
 * or AUDIO_CHANNEL_INVALID if the channel count exceeds that of the
 * configurations for which a default input channel mask is defined.
 */
static inline audio_channel_mask_t audio_channel_in_mask_from_count(uint32_t channel_count)
{
    uint32_t bits;
    switch (channel_count) {
    case 0:
        return AUDIO_CHANNEL_NONE;
    case 1:
        bits = AUDIO_CHANNEL_IN_MONO;
        break;
    case 2:
        bits = AUDIO_CHANNEL_IN_STEREO;
        break;
    default:
        return AUDIO_CHANNEL_INVALID;
    }
    return audio_channel_mask_from_representation_and_bits(
            AUDIO_CHANNEL_REPRESENTATION_POSITION, bits);
}

/* Derive a channel mask for index assignment from a channel count.
 * Returns the matching channel mask,
 * or AUDIO_CHANNEL_NONE if the channel count is zero,
 * or AUDIO_CHANNEL_INVALID if the channel count exceeds AUDIO_CHANNEL_COUNT_MAX.
 */
static inline audio_channel_mask_t audio_channel_mask_for_index_assignment_from_count(
        uint32_t channel_count)
{
    uint32_t bits;

    if (channel_count == 0) {
        return AUDIO_CHANNEL_NONE;
    }
    if (channel_count > AUDIO_CHANNEL_COUNT_MAX) {
        return AUDIO_CHANNEL_INVALID;
    }
    bits = (1 << channel_count) - 1;
    return audio_channel_mask_from_representation_and_bits(
            AUDIO_CHANNEL_REPRESENTATION_INDEX, bits);
}

static inline bool audio_is_valid_format(audio_format_t format)
{
    switch (format & AUDIO_FORMAT_MAIN_MASK) {
    case AUDIO_FORMAT_PCM:
        switch (format) {
        case AUDIO_FORMAT_PCM_16_BIT:
        case AUDIO_FORMAT_PCM_8_BIT:
        case AUDIO_FORMAT_PCM_32_BIT:
        case AUDIO_FORMAT_PCM_8_24_BIT:
        case AUDIO_FORMAT_PCM_FLOAT:
        case AUDIO_FORMAT_PCM_24_BIT_PACKED:
            return true;
        case AUDIO_FORMAT_INVALID:
        case AUDIO_FORMAT_DEFAULT:
        case AUDIO_FORMAT_MP3:
        case AUDIO_FORMAT_AMR_NB:
        case AUDIO_FORMAT_AMR_WB:
        case AUDIO_FORMAT_AAC:
        case AUDIO_FORMAT_HE_AAC_V1:
        case AUDIO_FORMAT_HE_AAC_V2:
        case AUDIO_FORMAT_VORBIS:
        case AUDIO_FORMAT_OPUS:
        case AUDIO_FORMAT_AC3:
        case AUDIO_FORMAT_E_AC3:
        case AUDIO_FORMAT_MAIN_MASK:
        case AUDIO_FORMAT_SUB_MASK:
        case AUDIO_FORMAT_AAC_MAIN:
        case AUDIO_FORMAT_AAC_LC:
        case AUDIO_FORMAT_AAC_SSR:
        case AUDIO_FORMAT_AAC_LTP:
        case AUDIO_FORMAT_AAC_HE_V1:
        case AUDIO_FORMAT_AAC_SCALABLE:
        case AUDIO_FORMAT_AAC_ERLC:
        case AUDIO_FORMAT_AAC_LD:
        case AUDIO_FORMAT_AAC_HE_V2:
        case AUDIO_FORMAT_AAC_ELD:
        default:
            return false;
        }
        /* not reached */
    case AUDIO_FORMAT_MP3:
    case AUDIO_FORMAT_AMR_NB:
    case AUDIO_FORMAT_AMR_WB:
    case AUDIO_FORMAT_AAC:
    case AUDIO_FORMAT_HE_AAC_V1:
    case AUDIO_FORMAT_HE_AAC_V2:
    case AUDIO_FORMAT_VORBIS:
    case AUDIO_FORMAT_OPUS:
    case AUDIO_FORMAT_AC3:
    case AUDIO_FORMAT_E_AC3:
        return true;
    default:
        return false;
    }
}

static inline bool audio_is_linear_pcm(audio_format_t format)
{
    return ((format & AUDIO_FORMAT_MAIN_MASK) == AUDIO_FORMAT_PCM);
}

static inline size_t audio_bytes_per_sample(audio_format_t format)
{
    size_t size = 0;

    switch (format) {
    case AUDIO_FORMAT_PCM_32_BIT:
    case AUDIO_FORMAT_PCM_8_24_BIT:
        size = sizeof(int32_t);
        break;
    case AUDIO_FORMAT_PCM_24_BIT_PACKED:
        size = sizeof(uint8_t) * 3;
        break;
    case AUDIO_FORMAT_PCM_16_BIT:
        size = sizeof(int16_t);
        break;
    case AUDIO_FORMAT_PCM_8_BIT:
        size = sizeof(uint8_t);
        break;
    case AUDIO_FORMAT_PCM_FLOAT:
        size = sizeof(float);
        break;
    case AUDIO_FORMAT_INVALID:
    case AUDIO_FORMAT_DEFAULT:
    case AUDIO_FORMAT_MP3:
    case AUDIO_FORMAT_AMR_NB:
    case AUDIO_FORMAT_AMR_WB:
    case AUDIO_FORMAT_AAC:
    case AUDIO_FORMAT_HE_AAC_V1:
    case AUDIO_FORMAT_HE_AAC_V2:
    case AUDIO_FORMAT_VORBIS:
    case AUDIO_FORMAT_OPUS:
    case AUDIO_FORMAT_AC3:
    case AUDIO_FORMAT_E_AC3:
    case AUDIO_FORMAT_MAIN_MASK:
    case AUDIO_FORMAT_SUB_MASK:
    case AUDIO_FORMAT_AAC_MAIN:
    case AUDIO_FORMAT_AAC_LC:
    case AUDIO_FORMAT_AAC_SSR:
    case AUDIO_FORMAT_AAC_LTP:
    case AUDIO_FORMAT_AAC_HE_V1:
    case AUDIO_FORMAT_AAC_SCALABLE:
    case AUDIO_FORMAT_AAC_ERLC:
    case AUDIO_FORMAT_AAC_LD:
    case AUDIO_FORMAT_AAC_HE_V2:
    case AUDIO_FORMAT_AAC_ELD:
    default:
        break;
    }
    return size;
}

/* converts device address to string sent to audio HAL via set_parameters */
#if 0 /* never used error */
static char *audio_device_address_to_parameter(audio_devices_t device, const char *address)
{
    const size_t kSize = AUDIO_DEVICE_MAX_ADDRESS_LEN + sizeof("a2dp_sink_address=");
    char param[kSize];

    if (device & AUDIO_DEVICE_OUT_ALL_A2DP)
        snprintf(param, kSize, "%s=%s", "a2dp_sink_address", address);
    else if (device & AUDIO_DEVICE_OUT_REMOTE_SUBMIX)
        snprintf(param, kSize, "%s=%s", "mix", address);
    else
        snprintf(param, kSize, "%s", address);

    return strdup(param);
}
#endif

__END_DECLS

#endif  // ANDROID_AUDIO_CORE_H
