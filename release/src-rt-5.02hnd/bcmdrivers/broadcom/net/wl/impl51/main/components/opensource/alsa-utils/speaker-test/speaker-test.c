/*
 * Copyright (C) 2000-2004 James Courtier-Dutton
 * Copyright (C) 2005 Nathan Hurst
 *
 * This file is part of the speaker-test tool.
 *
 * This small program sends a simple sinusoidal wave to your speakers.
 *
 * speaker-test is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * speaker-test is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 *
 *
 * Main program by James Courtier-Dutton (including some source code fragments from the alsa project.)
 * Some cleanup from Daniel Caujolle-Bert <segfault@club-internet.fr>
 * Pink noise option added Nathan Hurst, 
 *   based on generator by Phil Burk (pink.c)
 *
 * Changelog:
 *   0.0.8 Added support for pink noise output.
 * Changelog:
 *   0.0.7 Added support for more than 6 channels.
 * Changelog:
 *   0.0.6 Added support for different sample formats.
 *
 * $Id: speaker_test.c,v 1.00 2003/11/26 19:43:38 Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <ctype.h>
#include <byteswap.h>

#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API
#include <alsa/asoundlib.h>
#include <sys/time.h>
#include <math.h>
#include "pink.h"
#include "aconfig.h"
#include "gettext.h"
#include "version.h"

#ifdef ENABLE_NLS
#include <locale.h>
#endif

enum {
  TEST_PINK_NOISE = 1,
  TEST_SINE,
  TEST_WAV
};

#define MAX_CHANNELS	16

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define COMPOSE_ID(a,b,c,d)	((a) | ((b)<<8) | ((c)<<16) | ((d)<<24))
#define LE_SHORT(v)		(v)
#define LE_INT(v)		(v)
#define BE_SHORT(v)		bswap_16(v)
#define BE_INT(v)		bswap_32(v)
#else /* __BIG_ENDIAN */
#define COMPOSE_ID(a,b,c,d)	((d) | ((c)<<8) | ((b)<<16) | ((a)<<24))
#define LE_SHORT(v)		bswap_16(v)
#define LE_INT(v)		bswap_32(v)
#define BE_SHORT(v)		(v)
#define BE_INT(v)		(v)
#endif

static char              *device      = "default";       /* playback device */
static snd_pcm_format_t   format      = SND_PCM_FORMAT_S16; /* sample format */
static unsigned int       rate        = 48000;	            /* stream rate */
static unsigned int       channels    = 1;	            /* count of channels */
static unsigned int       speaker     = 0;	            /* count of channels */
static unsigned int       buffer_time = 0;	            /* ring buffer length in us */
static unsigned int       period_time = 0;	            /* period time in us */
static unsigned int       nperiods    = 4;                  /* number of periods */
static double             freq        = 440.0;              /* sinusoidal wave frequency in Hz */
static int                test_type   = TEST_PINK_NOISE;    /* Test type. 1 = noise, 2 = sine wave */
static pink_noise_t pink;
static snd_pcm_uframes_t  buffer_size;
static snd_pcm_uframes_t  period_size;
static const char *given_test_wav_file = NULL;
static char *wav_file_dir = SOUNDSDIR;
static int debug = 0;

static const char *const channel_name[MAX_CHANNELS] = {
  /*  0 */ N_("Front Left"),
  /*  1 */ N_("Front Right"),
  /*  2 */ N_("Rear Left"),
  /*  3 */ N_("Rear Right"),
  /*  4 */ N_("Center"),
  /*  5 */ N_("LFE"),
  /*  6 */ N_("Side Left"),
  /*  7 */ N_("Side Right"),
  /*  8 */ N_("Channel 9"),
  /*  9 */ N_("Channel 10"),
  /* 10 */ N_("Channel 11"),
  /* 11 */ N_("Channel 12"),
  /* 12 */ N_("Channel 13"),
  /* 13 */ N_("Channel 14"),
  /* 14 */ N_("Channel 15"),
  /* 15 */ N_("Channel 16")
};

static const int	channels4[] = {
  0, /* Front Left  */
  1, /* Front Right */
  3, /* Rear Right  */
  2, /* Rear Left   */
};
static const int	channels6[] = {
  0, /* Front Left  */
  4, /* Center      */
  1, /* Front Right */
  3, /* Rear Right  */
  2, /* Rear Left   */
  5, /* LFE         */
};
static const int	channels8[] = {
  0, /* Front Left  */
  4, /* Center      */
  1, /* Front Right */
  7, /* Side Right  */
  3, /* Rear Right  */
  2, /* Rear Left   */
  6, /* Side Left   */
  5, /* LFE         */
};
static const int	supported_formats[] = {
  SND_PCM_FORMAT_S8,
  SND_PCM_FORMAT_S16_LE,
  SND_PCM_FORMAT_S16_BE,
  SND_PCM_FORMAT_FLOAT_LE,
  SND_PCM_FORMAT_S32_LE,
  SND_PCM_FORMAT_S32_BE,
  -1
};

static void generate_sine(uint8_t *frames, int channel, int count, double *_phase) {
  double phase = *_phase;
  double max_phase = 1.0 / freq;
  double step = 1.0 / (double)rate;
  double res;
  float fres;
  int    chn;
  int32_t  ires;
  int8_t *samp8 = (int8_t*) frames;
  int16_t *samp16 = (int16_t*) frames;
  int32_t *samp32 = (int32_t*) frames;
  float   *samp_f = (float*) frames;

  while (count-- > 0) {
    for(chn=0;chn<channels;chn++) {
      switch (format) {
      case SND_PCM_FORMAT_S8:
        if (chn==channel) {
          res = (sin((phase * 2 * M_PI) / max_phase - M_PI)) * 0x03fffffff; /* Don't use MAX volume */
          ires = res;
          *samp8++ = ires >> 24;
        } else {
          *samp8++ = 0;
        }
        break;
      case SND_PCM_FORMAT_S16_LE:
        if (chn==channel) {
          res = (sin((phase * 2 * M_PI) / max_phase - M_PI)) * 0x03fffffff; /* Don't use MAX volume */
          ires = res;
          *samp16++ = LE_SHORT(ires >> 16);
        } else {
          *samp16++ = 0;
        }
        break;
      case SND_PCM_FORMAT_S16_BE:
        if (chn==channel) {
          res = (sin((phase * 2 * M_PI) / max_phase - M_PI)) * 0x03fffffff; /* Don't use MAX volume */
          ires = res;
          *samp16++ = BE_SHORT(ires >> 16);
        } else {
          *samp16++ = 0;
        }
        break;
      case SND_PCM_FORMAT_FLOAT_LE:
        if (chn==channel) {
          res = (sin((phase * 2 * M_PI) / max_phase - M_PI)) * 0.75 ; /* Don't use MAX volume */
          fres = res;
	  *samp_f++ = fres;
        } else {
	  *samp_f++ = 0.0;
        }
        break;
      case SND_PCM_FORMAT_S32_LE:
        if (chn==channel) {
          res = (sin((phase * 2 * M_PI) / max_phase - M_PI)) * 0x03fffffff; /* Don't use MAX volume */
          ires = res;
          *samp32++ = LE_INT(ires);
        } else {
          *samp32++ = 0;
        }
        break;
      case SND_PCM_FORMAT_S32_BE:
        if (chn==channel) {
          res = (sin((phase * 2 * M_PI) / max_phase - M_PI)) * 0x03fffffff; /* Don't use MAX volume */
          ires = res;
          *samp32++ = BE_INT(ires);
        } else {
          *samp32++ = 0;
        }
        break;
      default:
        ;
      }
    }

    phase += step;
    if (phase >= max_phase)
      phase -= max_phase;
  }

  *_phase = phase;
}

/* Pink noise is a better test than sine wave because we can tell
 * where pink noise is coming from more easily that a sine wave.
 */


static void generate_pink_noise( uint8_t *frames, int channel, int count) {
  double   res;
  int      chn;
  int32_t  ires;
  int8_t  *samp8 = (int8_t*) frames;
  int16_t *samp16 = (int16_t*) frames;
  int32_t *samp32 = (int32_t*) frames;

  while (count-- > 0) {
    for(chn=0;chn<channels;chn++) {
      switch (format) {
      case SND_PCM_FORMAT_S8:
        if (chn==channel) {
	  res = generate_pink_noise_sample(&pink) * 0x03fffffff; /* Don't use MAX volume */
	  ires = res;
	  *samp8++ = ires >> 24;
        } else {
	  *samp8++ = 0;
        }
        break;
      case SND_PCM_FORMAT_S16_LE:
        if (chn==channel) {
	  res = generate_pink_noise_sample(&pink) * 0x03fffffff; /* Don't use MAX volume */
	  ires = res;
          *samp16++ = LE_SHORT(ires >> 16);
        } else {
	  *samp16++ = 0;
        }
        break;
      case SND_PCM_FORMAT_S16_BE:
        if (chn==channel) {
          res = generate_pink_noise_sample(&pink) * 0x03fffffff; /* Don't use MAX volume */
          ires = res;
          *samp16++ = BE_SHORT(ires >> 16);
        } else {
          *samp16++ = 0;
        }
        break;
      case SND_PCM_FORMAT_S32_LE:
        if (chn==channel) {
          res = generate_pink_noise_sample(&pink) * 0x03fffffff; /* Don't use MAX volume */
          ires = res;
          *samp32++ = LE_INT(ires);
        } else {
          *samp32++ = 0;
        }
        break;
      case SND_PCM_FORMAT_S32_BE:
        if (chn==channel) {
	  res = generate_pink_noise_sample(&pink) * 0x03fffffff; /* Don't use MAX volume */
	  ires = res;
	  *samp32++ = BE_INT(ires);
        } else {
	  *samp32++ = 0;
        }
        break;
      default:
        ;
      }
    }
  }
}

static int set_hwparams(snd_pcm_t *handle, snd_pcm_hw_params_t *params, snd_pcm_access_t access) {
  unsigned int rrate;
  int          err;
  snd_pcm_uframes_t     period_size_min;
  snd_pcm_uframes_t     period_size_max;
  snd_pcm_uframes_t     buffer_size_min;
  snd_pcm_uframes_t     buffer_size_max;

  /* choose all parameters */
  err = snd_pcm_hw_params_any(handle, params);
  if (err < 0) {
    fprintf(stderr, _("Broken configuration for playback: no configurations available: %s\n"), snd_strerror(err));
    return err;
  }

  /* set the interleaved read/write format */
  err = snd_pcm_hw_params_set_access(handle, params, access);
  if (err < 0) {
    fprintf(stderr, _("Access type not available for playback: %s\n"), snd_strerror(err));
    return err;
  }

  /* set the sample format */
  err = snd_pcm_hw_params_set_format(handle, params, format);
  if (err < 0) {
    fprintf(stderr, _("Sample format not available for playback: %s\n"), snd_strerror(err));
    return err;
  }

  /* set the count of channels */
  err = snd_pcm_hw_params_set_channels(handle, params, channels);
  if (err < 0) {
    fprintf(stderr, _("Channels count (%i) not available for playbacks: %s\n"), channels, snd_strerror(err));
    return err;
  }

  /* set the stream rate */
  rrate = rate;
  err = snd_pcm_hw_params_set_rate(handle, params, rate, 0);
  if (err < 0) {
    fprintf(stderr, _("Rate %iHz not available for playback: %s\n"), rate, snd_strerror(err));
    return err;
  }

  if (rrate != rate) {
    fprintf(stderr, _("Rate doesn't match (requested %iHz, get %iHz, err %d)\n"), rate, rrate, err);
    return -EINVAL;
  }

  printf(_("Rate set to %iHz (requested %iHz)\n"), rrate, rate);
  /* set the buffer time */
  err = snd_pcm_hw_params_get_buffer_size_min(params, &buffer_size_min);
  err = snd_pcm_hw_params_get_buffer_size_max(params, &buffer_size_max);
  err = snd_pcm_hw_params_get_period_size_min(params, &period_size_min, NULL);
  err = snd_pcm_hw_params_get_period_size_max(params, &period_size_max, NULL);
  printf(_("Buffer size range from %lu to %lu\n"),buffer_size_min, buffer_size_max);
  printf(_("Period size range from %lu to %lu\n"),period_size_min, period_size_max);
  if (period_time > 0) {
    printf(_("Requested period time %u us\n"), period_time);
    err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, NULL);
    if (err < 0) {
      fprintf(stderr, _("Unable to set period time %u us for playback: %s\n"),
	     period_time, snd_strerror(err));
      return err;
    }
  }
  if (buffer_time > 0) {
    printf(_("Requested buffer time %u us\n"), buffer_time);
    err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, NULL);
    if (err < 0) {
      fprintf(stderr, _("Unable to set buffer time %u us for playback: %s\n"),
	     buffer_time, snd_strerror(err));
      return err;
    }
  }
  if (! buffer_time && ! period_time) {
    buffer_size = buffer_size_max;
    if (! period_time)
      buffer_size = (buffer_size / nperiods) * nperiods;
    printf(_("Using max buffer size %lu\n"), buffer_size);
    err = snd_pcm_hw_params_set_buffer_size_near(handle, params, &buffer_size);
    if (err < 0) {
      fprintf(stderr, _("Unable to set buffer size %lu for playback: %s\n"),
	     buffer_size, snd_strerror(err));
      return err;
    }
  }
  if (! buffer_time || ! period_time) {
    printf(_("Periods = %u\n"), nperiods);
    err = snd_pcm_hw_params_set_periods_near(handle, params, &nperiods, NULL);
    if (err < 0) {
      fprintf(stderr, _("Unable to set nperiods %u for playback: %s\n"),
	     nperiods, snd_strerror(err));
      return err;
    }
  }

  /* write the parameters to device */
  err = snd_pcm_hw_params(handle, params);
  if (err < 0) {
    fprintf(stderr, _("Unable to set hw params for playback: %s\n"), snd_strerror(err));
    return err;
  }

  snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
  snd_pcm_hw_params_get_period_size(params, &period_size, NULL);
  printf(_("was set period_size = %lu\n"),period_size);
  printf(_("was set buffer_size = %lu\n"),buffer_size);
  if (2*period_size > buffer_size) {
    fprintf(stderr, _("buffer to small, could not use\n"));
    return -EINVAL;
  }

  return 0;
}

static int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams) {
  int err;

  /* get the current swparams */
  err = snd_pcm_sw_params_current(handle, swparams);
  if (err < 0) {
    fprintf(stderr, _("Unable to determine current swparams for playback: %s\n"), snd_strerror(err));
    return err;
  }

  /* start the transfer when a buffer is full */
  err = snd_pcm_sw_params_set_start_threshold(handle, swparams, buffer_size);
  if (err < 0) {
    fprintf(stderr, _("Unable to set start threshold mode for playback: %s\n"), snd_strerror(err));
    return err;
  }

  /* allow the transfer when at least period_size frames can be processed */
  err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_size);
  if (err < 0) {
    fprintf(stderr, _("Unable to set avail min for playback: %s\n"), snd_strerror(err));
    return err;
  }

  /* write the parameters to the playback device */
  err = snd_pcm_sw_params(handle, swparams);
  if (err < 0) {
    fprintf(stderr, _("Unable to set sw params for playback: %s\n"), snd_strerror(err));
    return err;
  }

  return 0;
}

/*
 *   Underrun and suspend recovery
 */

static int xrun_recovery(snd_pcm_t *handle, int err) {
  if (err == -EPIPE) {	/* under-run */
    err = snd_pcm_prepare(handle);
    if (err < 0)
      fprintf(stderr, _("Can't recovery from underrun, prepare failed: %s\n"), snd_strerror(err));
    return 0;
  } 
  else if (err == -ESTRPIPE) {

    while ((err = snd_pcm_resume(handle)) == -EAGAIN)
      sleep(1);	/* wait until the suspend flag is released */

    if (err < 0) {
      err = snd_pcm_prepare(handle);
      if (err < 0)
        fprintf(stderr, _("Can't recovery from suspend, prepare failed: %s\n"), snd_strerror(err));
    }

    return 0;
  }

  return err;
}

/*
 * Handle WAV files
 */

static const char *wav_file[MAX_CHANNELS];
static int wav_file_size[MAX_CHANNELS];

struct wave_header {
  struct {
    uint32_t magic;
    uint32_t length;
    uint32_t type;
  } hdr;
  struct {
    uint32_t type;
    uint32_t length;
  } chunk1;
  struct {
    uint16_t format;
    uint16_t channels;
    uint32_t rate;
    uint32_t bytes_per_sec;
    uint16_t sample_size;
    uint16_t sample_bits;
  } body;
  struct {
    uint32_t type;
    uint32_t length;
  } chunk;
};

#define WAV_RIFF		COMPOSE_ID('R','I','F','F')
#define WAV_WAVE		COMPOSE_ID('W','A','V','E')
#define WAV_FMT			COMPOSE_ID('f','m','t',' ')
#define WAV_DATA		COMPOSE_ID('d','a','t','a')
#define WAV_PCM_CODE		1

static const char *search_for_file(const char *name)
{
  char *file;
  if (*name == '/')
    return strdup(name);
  file = malloc(strlen(wav_file_dir) + strlen(name) + 2);
  if (file)
    sprintf(file, "%s/%s", wav_file_dir, name);
  return file;
}

static int check_wav_file(int channel, const char *name)
{
  struct wave_header header;
  int fd;

  wav_file[channel] = search_for_file(name);
  if (! wav_file[channel]) {
    fprintf(stderr, _("No enough memory\n"));
    return -ENOMEM;
  }

  if ((fd = open(wav_file[channel], O_RDONLY)) < 0) {
    fprintf(stderr, _("Cannot open WAV file %s\n"), wav_file[channel]);
    return -EINVAL;
  }
  if (read(fd, &header, sizeof(header)) < (int)sizeof(header)) {
    fprintf(stderr, _("Invalid WAV file %s\n"), wav_file[channel]);
    goto error;
  }
  
  if (header.hdr.magic != WAV_RIFF || header.hdr.type != WAV_WAVE) {
    fprintf(stderr, _("Not a WAV file: %s\n"), wav_file[channel]);
    goto error;
  }
  if (header.body.format != LE_SHORT(WAV_PCM_CODE)) {
    fprintf(stderr, _("Unsupported WAV format %d for %s\n"),
	    LE_SHORT(header.body.format), wav_file[channel]);
    goto error;
  }
  if (header.body.channels != LE_SHORT(1)) {
    fprintf(stderr, _("%s is not a mono stream (%d channels)\n"),
	    wav_file[channel], LE_SHORT(header.body.channels)); 
    goto error;
  }
  if (header.body.rate != LE_INT(rate)) {
    fprintf(stderr, _("Sample rate doesn't match (%d) for %s\n"),
	    LE_INT(header.body.rate), wav_file[channel]);
    goto error;
  }
  if (header.body.sample_bits != LE_SHORT(16)) {
    fprintf(stderr, _("Unsupported sample format bits %d for %s\n"),
	    LE_SHORT(header.body.sample_bits), wav_file[channel]);
    goto error;
  }
  if (header.chunk.type != WAV_DATA) {
    fprintf(stderr, _("Invalid WAV file %s\n"), wav_file[channel]);
    goto error;
  }
  wav_file_size[channel] = LE_INT(header.chunk.length);
  close(fd);
  return 0;

 error:
  close(fd);
  return -EINVAL;
}

static int setup_wav_file(int chn)
{
  static const char *const wavs[MAX_CHANNELS] = {
    "Front_Left.wav",
    "Front_Right.wav",
    "Rear_Left.wav",
    "Rear_Right.wav",
    "Front_Center.wav",
    "Rear_Center.wav",
    "Side_Left.wav",
    "Side_Right.wav",
    "Channel_9.wav",
    "Channel_10.wav",
    "Channel_11.wav",
    "Channel_12.wav",
    "Channel_13.wav",
    "Channel_14.wav",
    "Channel_15.wav",
    "Channel_16.wav"
  };

  if (given_test_wav_file)
    return check_wav_file(chn, given_test_wav_file);
  else
    return check_wav_file(chn, wavs[chn]);
}

static int read_wav(uint16_t *buf, int channel, int offset, int bufsize)
{
  static FILE *wavfp = NULL;
  int size;

  if (! wav_file[channel]) {
    fprintf(stderr, _("Undefined channel %d\n"), channel);
    return -EINVAL;
  }

  if (offset >= wav_file_size[channel])
   return 0; /* finished */

  if (! offset) {
    if (wavfp)
      fclose(wavfp);
    wavfp = fopen(wav_file[channel], "r");
    if (! wavfp)
      return -errno;
    if (fseek(wavfp, sizeof(struct wave_header), SEEK_SET) < 0)
      return -errno;
  }
  if (offset + bufsize > wav_file_size[channel])
    bufsize = wav_file_size[channel] - offset;
  bufsize /= channels;
  for (size = 0; size < bufsize; size += 2) {
    int chn;
    for (chn = 0; chn < channels; chn++) {
      if (chn == channel) {
	if (fread(buf, 2, 1, wavfp) != 1)
	  return size;
      }
      else
	*buf = 0;
      buf++;
    }
  }
  return size;
}


/*
 *   Transfer method - write only
 */

static int write_buffer(snd_pcm_t *handle, uint8_t *ptr, int cptr)
{
  int err;

  while (cptr > 0) {

    err = snd_pcm_writei(handle, ptr, cptr);

    if (err == -EAGAIN)
      continue;

    if (err < 0) {
      fprintf(stderr, _("Write error: %d,%s\n"), err, snd_strerror(err));
      if (xrun_recovery(handle, err) < 0) {
	fprintf(stderr, _("xrun_recovery failed: %d,%s\n"), err, snd_strerror(err));
	return -1;
      }
      break;	/* skip one period */
    }

    ptr += snd_pcm_frames_to_bytes(handle, err);
    cptr -= err;
  }
  return 0;
}

static int write_loop(snd_pcm_t *handle, int channel, int periods, uint8_t *frames)
{
  double phase = 0;
  int    err, n;

  fflush(stdout);
  if (test_type == TEST_WAV) {
    int bufsize = snd_pcm_frames_to_bytes(handle, period_size);
    n = 0;
    while ((err = read_wav((uint16_t *)frames, channel, n, bufsize)) > 0) {
      n += err;
      if ((err = write_buffer(handle, frames,
			      snd_pcm_bytes_to_frames(handle, err * channels))) < 0)
	break;
    }
    if (buffer_size > n) {
      snd_pcm_drain(handle);
      snd_pcm_prepare(handle);
    }
    return err;
  }
    

  if (periods <= 0)
    periods = 1;

  for(n = 0; n < periods; n++) {
    if (test_type == TEST_PINK_NOISE)
      generate_pink_noise(frames, channel, period_size);
    else
      generate_sine(frames, channel, period_size, &phase);

    if ((err = write_buffer(handle, frames, period_size)) < 0)
      return err;
  }
  if (buffer_size > n * period_size) {
    snd_pcm_drain(handle);
    snd_pcm_prepare(handle);
  }
  return 0;
}

static void help(void)
{
  const int *fmt;

  printf(
	 _("Usage: speaker-test [OPTION]... \n"
	   "-h,--help	help\n"
	   "-D,--device	playback device\n"
	   "-r,--rate	stream rate in Hz\n"
	   "-c,--channels	count of channels in stream\n"
	   "-f,--frequency	sine wave frequency in Hz\n"
	   "-F,--format	sample format\n"
	   "-b,--buffer	ring buffer size in us\n"
	   "-p,--period	period size in us\n"
	   "-P,--nperiods	number of periods\n"
	   "-t,--test	pink=use pink noise, sine=use sine wave, wav=WAV file\n"
	   "-l,--nloops	specify number of loops to test, 0 = infinite\n"
	   "-s,--speaker	single speaker test. Values 1=Left, 2=right, etc\n"
	   "-w,--wavfile	Use the given WAV file as a test sound\n"
	   "-W,--wavdir	Specify the directory containing WAV files\n"
	   "\n"));
  printf(_("Recognized sample formats are:"));
  for (fmt = supported_formats; *fmt >= 0; fmt++) {
    const char *s = snd_pcm_format_name(*fmt);
    if (s)
      printf(" %s", s);
  }

  printf("\n\n");
}

int main(int argc, char *argv[]) {
  snd_pcm_t            *handle;
  int                   err, morehelp;
  snd_pcm_hw_params_t  *hwparams;
  snd_pcm_sw_params_t  *swparams;
  uint8_t              *frames;
  int                   chn;
  const int	       *fmt;
  double		time1,time2,time3;
  unsigned int		n, nloops;
  struct   timeval	tv1,tv2;

  static const struct option long_option[] = {
    {"help",      0, NULL, 'h'},
    {"device",    1, NULL, 'D'},
    {"rate",      1, NULL, 'r'},
    {"channels",  1, NULL, 'c'},
    {"frequency", 1, NULL, 'f'},
    {"format",    1, NULL, 'F'},
    {"buffer",    1, NULL, 'b'},
    {"period",    1, NULL, 'p'},
    {"nperiods",  1, NULL, 'P'},
    {"test",      1, NULL, 't'},
    {"nloops",    1, NULL, 'l'},
    {"speaker",   1, NULL, 's'},
    {"wavfile",   1, NULL, 'w'},
    {"wavdir",    1, NULL, 'W'},
    {"debug",	  0, NULL, 'd'},
    {NULL,        0, NULL, 0  },
  };

#ifdef ENABLE_NLS
  setlocale(LC_ALL, "");
  textdomain(PACKAGE);
#endif

  snd_pcm_hw_params_alloca(&hwparams);
  snd_pcm_sw_params_alloca(&swparams);
 
  nloops = 0;
  morehelp = 0;

  printf("\nspeaker-test %s\n\n", SND_UTIL_VERSION_STR);
  while (1) {
    int c;
    
    if ((c = getopt_long(argc, argv, "hD:r:c:f:F:b:p:P:t:l:s:w:W:d", long_option, NULL)) < 0)
      break;
    
    switch (c) {
    case 'h':
      morehelp++;
      break;
    case 'D':
      device = strdup(optarg);
      break;
    case 'F':
      format = snd_pcm_format_value(optarg);
      for (fmt = supported_formats; *fmt >= 0; fmt++)
        if (*fmt == format)
          break;
      if (*fmt < 0) {
        fprintf(stderr, "Format %s is not supported...\n", snd_pcm_format_name(format));
        exit(EXIT_FAILURE);
      }
      break;
    case 'r':
      rate = atoi(optarg);
      rate = rate < 4000 ? 4000 : rate;
      rate = rate > 196000 ? 196000 : rate;
      break;
    case 'c':
      channels = atoi(optarg);
      channels = channels < 1 ? 1 : channels;
      channels = channels > 1024 ? 1024 : channels;
      break;
    case 'f':
      freq = atof(optarg);
      freq = freq < 30.0 ? 30.0 : freq;
      freq = freq > 5000.0 ? 5000.0 : freq;
      break;
    case 'b':
      buffer_time = atoi(optarg);
      buffer_time = buffer_time > 1000000 ? 1000000 : buffer_time;
      break;
    case 'p':
      period_time = atoi(optarg);
      period_time = period_time > 1000000 ? 1000000 : period_time;
      break;
    case 'P':
      nperiods = atoi(optarg);
      if (nperiods < 2 || nperiods > 1024) {
	fprintf(stderr, _("Invalid number of periods %d\n"), nperiods);
	exit(1);
      }
      break;
    case 't':
      if (*optarg == 'p')
	test_type = TEST_PINK_NOISE;
      else if (*optarg == 's')
	test_type = TEST_SINE;
      else if (*optarg == 'w')
	test_type = TEST_WAV;
      else if (isdigit(*optarg)) {
	test_type = atoi(optarg);
	if (test_type < TEST_PINK_NOISE || test_type > TEST_WAV) {
	  fprintf(stderr, _("Invalid test type %s\n"), optarg);
	  exit(1);
	}
      } else {
	fprintf(stderr, _("Invalid test type %s\n"), optarg);
	exit(1);
      }
      break;
    case 'l':
      nloops = atoi(optarg);
      break;
    case 's':
      speaker = atoi(optarg);
      speaker = speaker < 1 ? 0 : speaker;
      speaker = speaker > channels ? 0 : speaker;
      if (speaker==0) {
        fprintf(stderr, _("Invalid parameter for -s option.\n"));
        exit(EXIT_FAILURE);
      }  
      break;
    case 'w':
      given_test_wav_file = optarg;
      break;
    case 'W':
      wav_file_dir = optarg;
      break;
    case 'd':
      debug = 1;
      break;
    default:
      fprintf(stderr, _("Unknown option '%c'\n"), c);
      exit(EXIT_FAILURE);
      break;
    }
  }

  if (morehelp) {
    help();
    exit(EXIT_SUCCESS);
  }

  if (test_type == TEST_WAV)
    format = SND_PCM_FORMAT_S16_LE; /* fixed format */

  printf(_("Playback device is %s\n"), device);
  printf(_("Stream parameters are %iHz, %s, %i channels\n"), rate, snd_pcm_format_name(format), channels);
  switch (test_type) {
  case TEST_PINK_NOISE:
    printf(_("Using 16 octaves of pink noise\n"));
    break;
  case TEST_SINE:
    printf(_("Sine wave rate is %.4fHz\n"), freq);
    break;
  case TEST_WAV:
    printf(_("WAV file(s)\n"));
    break;

  }

  while ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    printf(_("Playback open error: %d,%s\n"), err,snd_strerror(err));
    sleep(1);
  }

  if ((err = set_hwparams(handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    printf(_("Setting of hwparams failed: %s\n"), snd_strerror(err));
    snd_pcm_close(handle);
    exit(EXIT_FAILURE);
  }
  if ((err = set_swparams(handle, swparams)) < 0) {
    printf(_("Setting of swparams failed: %s\n"), snd_strerror(err));
    snd_pcm_close(handle);
    exit(EXIT_FAILURE);
  }
  if (debug) {
    snd_output_t *log;
    err = snd_output_stdio_attach(&log, stderr, 0);
    if (err >= 0) {
      snd_pcm_dump(handle, log);
      snd_output_close(log);
    }
  }

  frames = malloc(snd_pcm_frames_to_bytes(handle, period_size));
  if (test_type == TEST_PINK_NOISE)
    initialize_pink_noise(&pink, 16);
  
  if (frames == NULL) {
    fprintf(stderr, _("No enough memory\n"));
    exit(EXIT_FAILURE);
  }
  if (speaker==0) {

    if (test_type == TEST_WAV) {
      for (chn = 0; chn < channels; chn++) {
	if (setup_wav_file(chn) < 0)
	  exit(EXIT_FAILURE);
      }
    }

    for (n = 0; ! nloops || n < nloops; n++) {

      gettimeofday(&tv1, NULL);
      for(chn = 0; chn < channels; chn++) {
	int channel=chn;
	if (channels == 4) {
	    channel=channels4[chn];
	}
	if (channels == 6) {
	    channel=channels6[chn];
	}
	if (channels == 8) {
	    channel=channels8[chn];
	}
        printf(" %d - %s\n", channel, gettext(channel_name[channel]));

        err = write_loop(handle, channel, ((rate*3)/period_size), frames);

        if (err < 0) {
          fprintf(stderr, _("Transfer failed: %s\n"), snd_strerror(err));
          free(frames);
          snd_pcm_close(handle);
          exit(EXIT_SUCCESS);
        }
      }
      gettimeofday(&tv2, NULL);
      time1 = (double)tv1.tv_sec + ((double)tv1.tv_usec / 1000000.0);
      time2 = (double)tv2.tv_sec + ((double)tv2.tv_usec / 1000000.0);
      time3 = time2 - time1;
      printf(_("Time per period = %lf\n"), time3 );
    }
  } else {
    if (test_type == TEST_WAV) {
      if (setup_wav_file(speaker - 1) < 0)
	exit(EXIT_FAILURE);
    }

    printf("  - %s\n", gettext(channel_name[speaker-1]));
    err = write_loop(handle, speaker-1, ((rate*5)/period_size), frames);

    if (err < 0) {
      fprintf(stderr, _("Transfer failed: %s\n"), snd_strerror(err));
    }
  }


  free(frames);
  snd_pcm_close(handle);

  exit(EXIT_SUCCESS);
}
