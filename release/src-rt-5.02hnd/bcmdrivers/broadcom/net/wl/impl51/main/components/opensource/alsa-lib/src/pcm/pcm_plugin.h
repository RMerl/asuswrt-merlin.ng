/*
 *  PCM - Common plugin code
 *  Copyright (c) 2000 by Abramo Bagnara <abramo@alsa-project.org>
 *
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */
  
#include "iatomic.h"
#include "pcm_generic.h"

typedef snd_pcm_uframes_t (*snd_pcm_slave_xfer_areas_func_t)
     (snd_pcm_t *pcm, 
      const snd_pcm_channel_area_t *areas,
      snd_pcm_uframes_t offset, 
      snd_pcm_uframes_t size,
      const snd_pcm_channel_area_t *slave_areas,
      snd_pcm_uframes_t slave_offset, 
      snd_pcm_uframes_t *slave_sizep);

typedef snd_pcm_sframes_t (*snd_pcm_slave_xfer_areas_undo_func_t)
     (snd_pcm_t *pcm,
      const snd_pcm_channel_area_t *res_areas,	/* result areas */
      snd_pcm_uframes_t res_offset,		/* offset of result areas */
      snd_pcm_uframes_t res_size,		/* size of result areas */
      snd_pcm_uframes_t slave_undo_size);

typedef struct {
	snd_pcm_generic_t gen;
	snd_pcm_slave_xfer_areas_func_t read;
	snd_pcm_slave_xfer_areas_func_t write;
	snd_pcm_slave_xfer_areas_undo_func_t undo_read;
	snd_pcm_slave_xfer_areas_undo_func_t undo_write;
	snd_pcm_sframes_t (*client_frames)(snd_pcm_t *pcm, snd_pcm_sframes_t frames);
	snd_pcm_sframes_t (*slave_frames)(snd_pcm_t *pcm, snd_pcm_sframes_t frames);
	int (*init)(snd_pcm_t *pcm);
	snd_pcm_uframes_t appl_ptr, hw_ptr;
	snd_atomic_write_t watom;
} snd_pcm_plugin_t;	

/* make local functions really local */
#define snd_pcm_plugin_init \
	snd1_pcm_plugin_init
#define snd_pcm_plugin_fast_ops \
	snd1_pcm_plugin_fast_ops
#define snd_pcm_plugin_undo_read_generic \
	snd1_pcm_plugin_undo_read_generic
#define snd_pcm_plugin_undo_write_generic \
	snd1_pcm_plugin_undo_write_generic

void snd_pcm_plugin_init(snd_pcm_plugin_t *plugin);

extern const snd_pcm_fast_ops_t snd_pcm_plugin_fast_ops;

snd_pcm_sframes_t snd_pcm_plugin_undo_read_generic
     (snd_pcm_t *pcm,
      const snd_pcm_channel_area_t *res_areas,	/* result areas */
      snd_pcm_uframes_t res_offset,		/* offset of result areas */
      snd_pcm_uframes_t res_size,		/* size of result areas */
      snd_pcm_uframes_t slave_undo_size);

snd_pcm_sframes_t snd_pcm_plugin_undo_write_generic
     (snd_pcm_t *pcm,
      const snd_pcm_channel_area_t *res_areas,	/* result areas */
      snd_pcm_uframes_t res_offset,		/* offset of result areas */
      snd_pcm_uframes_t res_size,		/* size of result areas */
      snd_pcm_uframes_t slave_undo_size);

/* make local functions really local */
#define snd_pcm_linear_get_index	snd1_pcm_linear_get_index
#define snd_pcm_linear_put_index	snd1_pcm_linear_put_index
#define snd_pcm_linear_get32_index	snd1_pcm_linear_get32_index
#define snd_pcm_linear_put32_index	snd1_pcm_linear_put32_index
#define snd_pcm_linear_convert_index	snd1_pcm_linear_convert_index
#define snd_pcm_linear_convert	snd1_pcm_linear_convert
#define snd_pcm_linear_getput	snd1_pcm_linear_getput
#define snd_pcm_alaw_decode	snd1_pcm_alaw_decode
#define snd_pcm_alaw_encode	snd1_pcm_alaw_encode
#define snd_pcm_mulaw_decode	snd1_pcm_mulaw_decode
#define snd_pcm_mulaw_encode	snd1_pcm_mulaw_encode
#define snd_pcm_adpcm_decode	snd1_pcm_adpcm_decode
#define snd_pcm_adpcm_encode	snd1_pcm_adpcm_encode

int snd_pcm_linear_get_index(snd_pcm_format_t src_format, snd_pcm_format_t dst_format);
int snd_pcm_linear_put_index(snd_pcm_format_t src_format, snd_pcm_format_t dst_format);
int snd_pcm_linear_get32_index(snd_pcm_format_t src_format, snd_pcm_format_t dst_format);
int snd_pcm_linear_put32_index(snd_pcm_format_t src_format, snd_pcm_format_t dst_format);
int snd_pcm_linear_convert_index(snd_pcm_format_t src_format, snd_pcm_format_t dst_format);

void snd_pcm_linear_convert(const snd_pcm_channel_area_t *dst_areas, snd_pcm_uframes_t dst_offset,
			    const snd_pcm_channel_area_t *src_areas, snd_pcm_uframes_t src_offset,
			    unsigned int channels, snd_pcm_uframes_t frames,
			    unsigned int convidx);
void snd_pcm_linear_getput(const snd_pcm_channel_area_t *dst_areas, snd_pcm_uframes_t dst_offset,
			   const snd_pcm_channel_area_t *src_areas, snd_pcm_uframes_t src_offset,
			   unsigned int channels, snd_pcm_uframes_t frames,
			   unsigned int get_idx, unsigned int put_idx);
void snd_pcm_alaw_decode(const snd_pcm_channel_area_t *dst_areas,
			 snd_pcm_uframes_t dst_offset,
			 const snd_pcm_channel_area_t *src_areas,
			 snd_pcm_uframes_t src_offset,
			 unsigned int channels, snd_pcm_uframes_t frames,
			 unsigned int putidx);
void snd_pcm_alaw_encode(const snd_pcm_channel_area_t *dst_areas,
			 snd_pcm_uframes_t dst_offset,
			 const snd_pcm_channel_area_t *src_areas,
			 snd_pcm_uframes_t src_offset,
			 unsigned int channels, snd_pcm_uframes_t frames,
			 unsigned int getidx);
void snd_pcm_mulaw_decode(const snd_pcm_channel_area_t *dst_areas,
			  snd_pcm_uframes_t dst_offset,
			  const snd_pcm_channel_area_t *src_areas,
			  snd_pcm_uframes_t src_offset,
			  unsigned int channels, snd_pcm_uframes_t frames,
			  unsigned int putidx);
void snd_pcm_mulaw_encode(const snd_pcm_channel_area_t *dst_areas,
			  snd_pcm_uframes_t dst_offset,
			  const snd_pcm_channel_area_t *src_areas,
			  snd_pcm_uframes_t src_offset,
			  unsigned int channels, snd_pcm_uframes_t frames,
			  unsigned int getidx);

typedef struct _snd_pcm_adpcm_state {
	int pred_val;		/* Calculated predicted value */
	int step_idx;		/* Previous StepSize lookup index */
} snd_pcm_adpcm_state_t;

void snd_pcm_adpcm_decode(const snd_pcm_channel_area_t *dst_areas,
			  snd_pcm_uframes_t dst_offset,
			  const snd_pcm_channel_area_t *src_areas,
			  snd_pcm_uframes_t src_offset,
			  unsigned int channels, snd_pcm_uframes_t frames,
			  unsigned int putidx,
			  snd_pcm_adpcm_state_t *states);
void snd_pcm_adpcm_encode(const snd_pcm_channel_area_t *dst_areas,
			  snd_pcm_uframes_t dst_offset,
			  const snd_pcm_channel_area_t *src_areas,
			  snd_pcm_uframes_t src_offset,
			  unsigned int channels, snd_pcm_uframes_t frames,
			  unsigned int getidx,
			  snd_pcm_adpcm_state_t *states);
