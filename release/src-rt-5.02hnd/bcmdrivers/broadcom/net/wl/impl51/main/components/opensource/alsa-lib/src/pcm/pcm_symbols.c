/*
 *  PCM Symbols
 *  Copyright (c) 2001 by Jaroslav Kysela <perex@perex.cz>
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

#ifndef PIC

#include "config.h"

extern const char *_snd_module_pcm_adpcm;
extern const char *_snd_module_pcm_alaw;
extern const char *_snd_module_pcm_copy;
extern const char *_snd_module_pcm_file;
extern const char *_snd_module_pcm_hooks;
extern const char *_snd_module_pcm_hw;
extern const char *_snd_module_pcm_linear;
extern const char *_snd_module_pcm_meter;
extern const char *_snd_module_pcm_mulaw;
extern const char *_snd_module_pcm_multi;
extern const char *_snd_module_pcm_null;
extern const char *_snd_module_pcm_empty;
extern const char *_snd_module_pcm_plug;
extern const char *_snd_module_pcm_rate;
extern const char *_snd_module_pcm_route;
extern const char *_snd_module_pcm_share;
extern const char *_snd_module_pcm_shm;
extern const char *_snd_module_pcm_lfloat;
extern const char *_snd_module_pcm_ladspa;
extern const char *_snd_module_pcm_dmix;
extern const char *_snd_module_pcm_dsnoop;
extern const char *_snd_module_pcm_dshare;
extern const char *_snd_module_pcm_asym;
extern const char *_snd_module_pcm_iec958;
extern const char *_snd_module_pcm_softvol;
extern const char *_snd_module_pcm_extplug;
extern const char *_snd_module_pcm_ioplug;
extern const char *_snd_module_pcm_mmap_emul;

static const char **snd_pcm_open_objects[] = {
	&_snd_module_pcm_hw,
#include "pcm_symbols_list.c"
};
	
void *snd_pcm_open_symbols(void)
{
	return snd_pcm_open_objects;
}

#endif /* !PIC */
