/**
 * \file mixer/simple_none.c
 * \brief Mixer Simple Element Class Interface
 * \author Jaroslav Kysela <perex@perex.cz>
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2001-2004
 *
 * Mixer simple element class interface.
 */
/*
 *  Mixer Interface - simple controls
 *  Copyright (c) 2000,2004 by Jaroslav Kysela <perex@perex.cz>
 *  Copyright (c) 2001 by Abramo Bagnara <abramo@alsa-project.org>
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <alsa/asoundlib.h>
#include "mixer_simple.h"
#include "config.h"

#ifndef DOC_HIDDEN

#define MIXER_COMPARE_WEIGHT_SIMPLE_BASE        0
#define MIXER_COMPARE_WEIGHT_NEXT_BASE          10000000
#define MIXER_COMPARE_WEIGHT_NOT_FOUND          1000000000

typedef enum _selem_ctl_type {
	CTL_SINGLE,
	CTL_GLOBAL_ENUM,
	CTL_GLOBAL_SWITCH,
	CTL_GLOBAL_VOLUME,
	CTL_GLOBAL_ROUTE,
	CTL_PLAYBACK_ENUM,
	CTL_PLAYBACK_SWITCH,
	CTL_PLAYBACK_VOLUME,
	CTL_PLAYBACK_ROUTE,
	CTL_CAPTURE_ENUM,
	CTL_CAPTURE_SWITCH,
	CTL_CAPTURE_VOLUME,
	CTL_CAPTURE_ROUTE,
	CTL_CAPTURE_SOURCE,
	CTL_LAST = CTL_CAPTURE_SOURCE,
} selem_ctl_type_t;

typedef struct _selem_ctl {
	snd_hctl_elem_t *elem;
	snd_ctl_elem_type_t type;
	unsigned int inactive: 1;
	unsigned int values;
	long min, max;
} selem_ctl_t;

typedef struct _selem_none {
	sm_selem_t selem;
	selem_ctl_t ctls[CTL_LAST + 1];
	unsigned int capture_item;
	struct selem_str {
		unsigned int range: 1;	/* Forced range */
		unsigned int db_initialized: 1;
		unsigned int db_init_error: 1;
		long min, max;
		unsigned int channels;
		long vol[32];
		unsigned int sw;
		unsigned int *db_info;
	} str[2];
} selem_none_t;

static const struct mixer_name_table {
	const char *longname;
	const char *shortname;
} name_table[] = {
	{"Tone Control - Switch", "Tone"},
	{"Tone Control - Bass", "Bass"},
	{"Tone Control - Treble", "Treble"},
	{"Synth Tone Control - Switch", "Synth Tone"},
	{"Synth Tone Control - Bass", "Synth Bass"},
	{"Synth Tone Control - Treble", "Synth Treble"},
	{0, 0},
};

#endif /* !DOC_HIDDEN */

static const char *get_short_name(const char *lname)
{
	const struct mixer_name_table *p;
	for (p = name_table; p->longname; p++) {
		if (!strcmp(lname, p->longname))
			return p->shortname;
	}
	return lname;
}

static int compare_mixer_priority_lookup(const char **name, const char * const *names, int coef)
{
	int res;

	for (res = 0; *names; names++, res += coef) {
		if (!strncmp(*name, *names, strlen(*names))) {
			*name += strlen(*names);
			if (**name == ' ')
				(*name)++;
			return res+1;
		}
	}
	return MIXER_COMPARE_WEIGHT_NOT_FOUND;
}

static int get_compare_weight(const char *name, unsigned int idx)
{
	static const char *const names[] = {
		"Master",
		"Headphone",
		"Speaker",
		"Tone",
		"Bass",
		"Treble",
		"3D Control",
		"PCM",
		"Front",
		"Surround",
		"Center",
		"LFE",
		"Side",
		"Synth",
		"FM",
		"Wave",
		"Music",
		"DSP",
		"Line",
		"CD",
		"Mic",
		"Video",
		"Zoom Video",
		"Phone",
		"I2S",
		"IEC958",
		"PC Speaker",
		"Beep",
		"Aux",
		"Mono",
		"Playback",
		"Capture",
		"Mix",
		NULL
	};
	static const char *const names1[] = {
		"-",
		NULL,
	};
	static const char *const names2[] = {
		"Mono",
		"Digital",
		"Switch",
		"Depth",
		"Wide",
		"Space",
		"Level",
		"Center",
		"Output",
		"Boost",
		"Tone",
		"Bass",
		"Treble",
		NULL,
	};
	const char *name1;
	int res, res1;

	if ((res = compare_mixer_priority_lookup((const char **)&name, names, 1000)) == MIXER_COMPARE_WEIGHT_NOT_FOUND)
		return MIXER_COMPARE_WEIGHT_NOT_FOUND;
	if (*name == '\0')
		goto __res;
	for (name1 = name; *name1 != '\0'; name1++);
	for (name1--; name1 != name && *name1 != ' '; name1--);
	while (name1 != name && *name1 == ' ')
		name1--;
	if (name1 != name) {
		for (; name1 != name && *name1 != ' '; name1--);
		name = name1;
		if ((res1 = compare_mixer_priority_lookup((const char **)&name, names1, 200)) == MIXER_COMPARE_WEIGHT_NOT_FOUND)
			return res;
		res += res1;
	} else {
		name = name1;
	}
	if ((res1 = compare_mixer_priority_lookup((const char **)&name, names2, 20)) == MIXER_COMPARE_WEIGHT_NOT_FOUND)
		return res;
      __res:
	return MIXER_COMPARE_WEIGHT_SIMPLE_BASE + res + idx;
}

static long to_user(selem_none_t *s, int dir, selem_ctl_t *c, long value)
{
	int64_t n;
	if (c->max == c->min)
		return s->str[dir].min;
	n = (int64_t) (value - c->min) * (s->str[dir].max - s->str[dir].min);
	return s->str[dir].min + (n + (c->max - c->min) / 2) / (c->max - c->min);
}

static long from_user(selem_none_t *s, int dir, selem_ctl_t *c, long value)
{
	int64_t n;
	if (s->str[dir].max == s->str[dir].min)
		return c->min;
	n = (int64_t) (value - s->str[dir].min) * (c->max - c->min);
	return c->min + (n + (s->str[dir].max - s->str[dir].min) / 2) / (s->str[dir].max - s->str[dir].min);
}

static int elem_read_volume(selem_none_t *s, int dir, selem_ctl_type_t type)
{
	snd_ctl_elem_value_t *ctl;
	unsigned int idx;
	int err;
	selem_ctl_t *c = &s->ctls[type];
	snd_ctl_elem_value_alloca(&ctl);
	if ((err = snd_hctl_elem_read(c->elem, ctl)) < 0)
		return err;
	for (idx = 0; idx < s->str[dir].channels; idx++) {
		unsigned int idx1 = idx;
		if (idx >= c->values)
			idx1 = 0;
		s->str[dir].vol[idx] = to_user(s, dir, c, snd_ctl_elem_value_get_integer(ctl, idx1));
	}
	return 0;
}

static int elem_read_switch(selem_none_t *s, int dir, selem_ctl_type_t type)
{
	snd_ctl_elem_value_t *ctl;
	unsigned int idx;
	int err;
	selem_ctl_t *c = &s->ctls[type];
	snd_ctl_elem_value_alloca(&ctl);
	if ((err = snd_hctl_elem_read(c->elem, ctl)) < 0)
		return err;
	for (idx = 0; idx < s->str[dir].channels; idx++) {
		unsigned int idx1 = idx;
		if (idx >= c->values)
			idx1 = 0;
		if (!snd_ctl_elem_value_get_integer(ctl, idx1))
			s->str[dir].sw &= ~(1 << idx);
	}
	return 0;
}

static int elem_read_route(selem_none_t *s, int dir, selem_ctl_type_t type)
{
	snd_ctl_elem_value_t *ctl;
	unsigned int idx;
	int err;
	selem_ctl_t *c = &s->ctls[type];
	snd_ctl_elem_value_alloca(&ctl);
	if ((err = snd_hctl_elem_read(c->elem, ctl)) < 0)
		return err;
	for (idx = 0; idx < s->str[dir].channels; idx++) {
		unsigned int idx1 = idx;
		if (idx >= c->values)
			idx1 = 0;
		if (!snd_ctl_elem_value_get_integer(ctl, idx1 * c->values + idx1))
			s->str[dir].sw &= ~(1 << idx);
	}
	return 0;
}

static int elem_read_enum(selem_none_t *s)
{
	snd_ctl_elem_value_t *ctl;
	unsigned int idx;
	int err;
	int type;
	selem_ctl_t *c;
	type = CTL_GLOBAL_ENUM;
	if ( (s->selem.caps & (SM_CAP_CENUM | SM_CAP_PENUM)) == (SM_CAP_CENUM | SM_CAP_PENUM) )
		type = CTL_GLOBAL_ENUM;
	else if (s->selem.caps & SM_CAP_PENUM)
		type = CTL_PLAYBACK_ENUM;
	else if (s->selem.caps & SM_CAP_CENUM)
		type = CTL_CAPTURE_ENUM;
	c = &s->ctls[type];
	snd_ctl_elem_value_alloca(&ctl);
	if ((err = snd_hctl_elem_read(c->elem, ctl)) < 0)
		return err;
	for (idx = 0; idx < s->str[0].channels; idx++) {
		unsigned int idx1 = idx;
		if (idx >= c->values)
			idx1 = 0;
		s->str[0].vol[idx] = snd_ctl_elem_value_get_enumerated(ctl, idx1);
	}
	return 0;
}

static int selem_read(snd_mixer_elem_t *elem)
{
	selem_none_t *s;
	unsigned int idx;
	int err = 0;
	long pvol[32], cvol[32];
	unsigned int psw, csw;

	assert(snd_mixer_elem_get_type(elem) == SND_MIXER_ELEM_SIMPLE);
	s = snd_mixer_elem_get_private(elem);

	memcpy(pvol, s->str[SM_PLAY].vol, sizeof(pvol));
	memset(&s->str[SM_PLAY].vol, 0, sizeof(s->str[SM_PLAY].vol));
	psw = s->str[SM_PLAY].sw;
	s->str[SM_PLAY].sw = ~0U;
	memcpy(cvol, s->str[SM_CAPT].vol, sizeof(cvol));
	memset(&s->str[SM_CAPT].vol, 0, sizeof(s->str[SM_CAPT].vol));
	csw = s->str[SM_CAPT].sw;
	s->str[SM_CAPT].sw = ~0U;

	if (s->ctls[CTL_GLOBAL_ENUM].elem) {
		err = elem_read_enum(s);
		if (err < 0)
			return err;
		goto __skip_cswitch;
	}

	if (s->ctls[CTL_CAPTURE_ENUM].elem) {
		err = elem_read_enum(s);
		if (err < 0)
			return err;
		goto __skip_cswitch;
	}

	if (s->ctls[CTL_PLAYBACK_ENUM].elem) {
		err = elem_read_enum(s);
		if (err < 0)
			return err;
		goto __skip_cswitch;
	}


	if (s->ctls[CTL_PLAYBACK_VOLUME].elem)
		err = elem_read_volume(s, SM_PLAY, CTL_PLAYBACK_VOLUME);
	else if (s->ctls[CTL_GLOBAL_VOLUME].elem)
		err = elem_read_volume(s, SM_PLAY, CTL_GLOBAL_VOLUME);
	else if (s->ctls[CTL_SINGLE].elem &&
		 s->ctls[CTL_SINGLE].type == SND_CTL_ELEM_TYPE_INTEGER)
		err = elem_read_volume(s, SM_PLAY, CTL_SINGLE);
	if (err < 0)
		return err;

	if ((s->selem.caps & (SM_CAP_GSWITCH|SM_CAP_PSWITCH)) == 0) {
		s->str[SM_PLAY].sw = 0;
		goto __skip_pswitch;
	}
	if (s->ctls[CTL_PLAYBACK_SWITCH].elem) {
		err = elem_read_switch(s, SM_PLAY, CTL_PLAYBACK_SWITCH);
		if (err < 0)
			return err;
	}
	if (s->ctls[CTL_GLOBAL_SWITCH].elem) {
		err = elem_read_switch(s, SM_PLAY, CTL_GLOBAL_SWITCH);
		if (err < 0)
			return err;
	}
	if (s->ctls[CTL_SINGLE].elem &&
	    s->ctls[CTL_SINGLE].type == SND_CTL_ELEM_TYPE_BOOLEAN) {
		err = elem_read_switch(s, SM_PLAY, CTL_SINGLE);
		if (err < 0)
			return err;
	}
	if (s->ctls[CTL_PLAYBACK_ROUTE].elem) {
		err = elem_read_route(s, SM_PLAY, CTL_PLAYBACK_ROUTE);
		if (err < 0)
			return err;
	}
	if (s->ctls[CTL_GLOBAL_ROUTE].elem) {
		err = elem_read_route(s, SM_PLAY, CTL_GLOBAL_ROUTE);
		if (err < 0)
			return err;
	}
      __skip_pswitch:

	if (s->ctls[CTL_CAPTURE_VOLUME].elem)
		err = elem_read_volume(s, SM_CAPT, CTL_CAPTURE_VOLUME);
	else if (s->ctls[CTL_GLOBAL_VOLUME].elem)
		err = elem_read_volume(s, SM_CAPT, CTL_GLOBAL_VOLUME);
	else if (s->ctls[CTL_SINGLE].elem &&
		 s->ctls[CTL_SINGLE].type == SND_CTL_ELEM_TYPE_INTEGER)
		err = elem_read_volume(s, SM_CAPT, CTL_SINGLE);
	if (err < 0)
		return err;

	if ((s->selem.caps & (SM_CAP_GSWITCH|SM_CAP_CSWITCH)) == 0) {
		s->str[SM_CAPT].sw = 0;
		goto __skip_cswitch;
	}
	if (s->ctls[CTL_CAPTURE_SWITCH].elem) {
		err = elem_read_switch(s, SM_CAPT, CTL_CAPTURE_SWITCH);
		if (err < 0)
			return err;
	}
	if (s->ctls[CTL_GLOBAL_SWITCH].elem) {
		err = elem_read_switch(s, SM_CAPT, CTL_GLOBAL_SWITCH);
		if (err < 0)
			return err;
	}
	if (s->ctls[CTL_SINGLE].elem &&
	    s->ctls[CTL_SINGLE].type == SND_CTL_ELEM_TYPE_BOOLEAN) {
		err = elem_read_switch(s, SM_CAPT, CTL_SINGLE);
		if (err < 0)
			return err;
	}
	if (s->ctls[CTL_CAPTURE_ROUTE].elem) {
		err = elem_read_route(s, SM_CAPT, CTL_CAPTURE_ROUTE);
		if (err < 0)
			return err;
	}
	if (s->ctls[CTL_GLOBAL_ROUTE].elem) {
		err = elem_read_route(s, SM_CAPT, CTL_GLOBAL_ROUTE);
		if (err < 0)
			return err;
	}
	if (s->ctls[CTL_CAPTURE_SOURCE].elem) {
		snd_ctl_elem_value_t *ctl;
		selem_ctl_t *c = &s->ctls[CTL_CAPTURE_SOURCE];
		snd_ctl_elem_value_alloca(&ctl);
		err = snd_hctl_elem_read(c->elem, ctl);
		if (err < 0)
			return err;
		for (idx = 0; idx < s->str[SM_CAPT].channels; idx++) {
			unsigned int idx1 = idx;
			if (idx >= c->values)
				idx1 = 0;
			if (snd_ctl_elem_value_get_enumerated(ctl, idx1) != s->capture_item)
				s->str[SM_CAPT].sw &= ~(1 << idx);
		}
	}
      __skip_cswitch:

	if (memcmp(pvol, s->str[SM_PLAY].vol, sizeof(pvol)) ||
	    psw != s->str[SM_PLAY].sw ||
	    memcmp(cvol, s->str[SM_CAPT].vol, sizeof(cvol)) ||
	    csw != s->str[SM_CAPT].sw)
		return 1;
	return 0;
}

static int elem_write_volume(selem_none_t *s, int dir, selem_ctl_type_t type)
{
	snd_ctl_elem_value_t *ctl;
	unsigned int idx;
	int err;
	selem_ctl_t *c = &s->ctls[type];
	snd_ctl_elem_value_alloca(&ctl);
	if ((err = snd_hctl_elem_read(c->elem, ctl)) < 0)
		return err;
	for (idx = 0; idx < c->values; idx++)
		snd_ctl_elem_value_set_integer(ctl, idx, from_user(s, dir, c, s->str[dir].vol[idx]));
	if ((err = snd_hctl_elem_write(c->elem, ctl)) < 0)
		return err;
	return 0;
}

static int elem_write_switch(selem_none_t *s, int dir, selem_ctl_type_t type)
{
	snd_ctl_elem_value_t *ctl;
	unsigned int idx;
	int err;
	selem_ctl_t *c = &s->ctls[type];
	snd_ctl_elem_value_alloca(&ctl);
	if ((err = snd_hctl_elem_read(c->elem, ctl)) < 0)
		return err;
	for (idx = 0; idx < c->values; idx++)
		snd_ctl_elem_value_set_integer(ctl, idx, !!(s->str[dir].sw & (1 << idx)));
	if ((err = snd_hctl_elem_write(c->elem, ctl)) < 0)
		return err;
	return 0;
}

static int elem_write_switch_constant(selem_none_t *s, selem_ctl_type_t type, int val)
{
	snd_ctl_elem_value_t *ctl;
	unsigned int idx;
	int err;
	selem_ctl_t *c = &s->ctls[type];
	snd_ctl_elem_value_alloca(&ctl);
	if ((err = snd_hctl_elem_read(c->elem, ctl)) < 0)
		return err;
	for (idx = 0; idx < c->values; idx++)
		snd_ctl_elem_value_set_integer(ctl, idx, !!val);
	if ((err = snd_hctl_elem_write(c->elem, ctl)) < 0)
		return err;
	return 0;
}

static int elem_write_route(selem_none_t *s, int dir, selem_ctl_type_t type)
{
	snd_ctl_elem_value_t *ctl;
	unsigned int idx;
	int err;
	selem_ctl_t *c = &s->ctls[type];
	snd_ctl_elem_value_alloca(&ctl);
	if ((err = snd_hctl_elem_read(c->elem, ctl)) < 0)
		return err;
	for (idx = 0; idx < c->values * c->values; idx++)
		snd_ctl_elem_value_set_integer(ctl, idx, 0);
	for (idx = 0; idx < c->values; idx++)
		snd_ctl_elem_value_set_integer(ctl, idx * c->values + idx, !!(s->str[dir].sw & (1 << idx)));
	if ((err = snd_hctl_elem_write(c->elem, ctl)) < 0)
		return err;
	return 0;
}

static int elem_write_enum(selem_none_t *s)
{
	snd_ctl_elem_value_t *ctl;
	unsigned int idx;
	int err;
	int type;
	selem_ctl_t *c;
	type = CTL_GLOBAL_ENUM;
	if ( (s->selem.caps & (SM_CAP_CENUM | SM_CAP_PENUM) ) == (SM_CAP_CENUM | SM_CAP_PENUM) )
		type = CTL_GLOBAL_ENUM;
	else if (s->selem.caps & SM_CAP_PENUM)
		type = CTL_PLAYBACK_ENUM;
	else if (s->selem.caps & SM_CAP_CENUM)
		type = CTL_CAPTURE_ENUM;
	c = &s->ctls[type];
	snd_ctl_elem_value_alloca(&ctl);
	if ((err = snd_hctl_elem_read(c->elem, ctl)) < 0)
		return err;
	for (idx = 0; idx < c->values; idx++)
		snd_ctl_elem_value_set_enumerated(ctl, idx, (unsigned int)s->str[0].vol[idx]);
	if ((err = snd_hctl_elem_write(c->elem, ctl)) < 0)
		return err;
	return 0;
}

static int selem_write_main(snd_mixer_elem_t *elem)
{
	selem_none_t *s;
	unsigned int idx;
	int err;

	assert(snd_mixer_elem_get_type(elem) == SND_MIXER_ELEM_SIMPLE);
	s = snd_mixer_elem_get_private(elem);

	if (s->ctls[CTL_GLOBAL_ENUM].elem)
		return elem_write_enum(s);

	if (s->ctls[CTL_PLAYBACK_ENUM].elem)
		return elem_write_enum(s);

	if (s->ctls[CTL_CAPTURE_ENUM].elem)
		return elem_write_enum(s);

	if (s->ctls[CTL_SINGLE].elem) {
		if (s->ctls[CTL_SINGLE].type == SND_CTL_ELEM_TYPE_INTEGER)
			err = elem_write_volume(s, SM_PLAY, CTL_SINGLE);
		else
			err = elem_write_switch(s, SM_PLAY, CTL_SINGLE);
		if (err < 0)
			return err;
	}
	if (s->ctls[CTL_GLOBAL_VOLUME].elem) {
		err = elem_write_volume(s, SM_PLAY, CTL_GLOBAL_VOLUME);
		if (err < 0)
			return err;
	}
	if (s->ctls[CTL_GLOBAL_SWITCH].elem) {
		if (s->ctls[CTL_PLAYBACK_SWITCH].elem && s->ctls[CTL_CAPTURE_SWITCH].elem)
			err = elem_write_switch_constant(s, CTL_GLOBAL_SWITCH, 1);
		else
			err = elem_write_switch(s, SM_PLAY, CTL_GLOBAL_SWITCH);
		if (err < 0)
			return err;
	}
	if (s->ctls[CTL_PLAYBACK_VOLUME].elem) {
		err = elem_write_volume(s, SM_PLAY, CTL_PLAYBACK_VOLUME);
		if (err < 0)
			return err;
	}
	if (s->ctls[CTL_PLAYBACK_SWITCH].elem) {
		err = elem_write_switch(s, SM_PLAY, CTL_PLAYBACK_SWITCH);
		if (err < 0)
			return err;
	}
	if (s->ctls[CTL_PLAYBACK_ROUTE].elem) {
		err = elem_write_route(s, SM_PLAY, CTL_PLAYBACK_ROUTE);
		if (err < 0)
			return err;
	}
	if (s->ctls[CTL_CAPTURE_VOLUME].elem) {
		err = elem_write_volume(s, SM_CAPT, CTL_CAPTURE_VOLUME);
		if (err < 0)
			return err;
	}
	if (s->ctls[CTL_CAPTURE_SWITCH].elem) {
		err = elem_write_switch(s, SM_CAPT, CTL_CAPTURE_SWITCH);
		if (err < 0)
			return err;
	}
	if (s->ctls[CTL_CAPTURE_ROUTE].elem) {
		err = elem_write_route(s, SM_CAPT, CTL_CAPTURE_ROUTE);
		if (err < 0)
			return err;
	}
	if (s->ctls[CTL_CAPTURE_SOURCE].elem) {
		snd_ctl_elem_value_t *ctl;
		selem_ctl_t *c = &s->ctls[CTL_CAPTURE_SOURCE];
		snd_ctl_elem_value_alloca(&ctl);
		if ((err = snd_hctl_elem_read(c->elem, ctl)) < 0)
			return err;
		for (idx = 0; idx < c->values; idx++) {
			if (s->str[SM_CAPT].sw & (1 << idx))
				snd_ctl_elem_value_set_enumerated(ctl, idx, s->capture_item);
		}
		if ((err = snd_hctl_elem_write(c->elem, ctl)) < 0)
			return err;
		/* update the element, don't remove */
		err = selem_read(elem);
		if (err < 0)
			return err;
	}
	return 0;
}

static int selem_write(snd_mixer_elem_t *elem)
{
	int err;
	
	err = selem_write_main(elem);
	if (err < 0)
		selem_read(elem);
	return err;
}

static void selem_free(snd_mixer_elem_t *elem)
{
	selem_none_t *simple = snd_mixer_elem_get_private(elem);
	assert(snd_mixer_elem_get_type(elem) == SND_MIXER_ELEM_SIMPLE);
	if (simple->selem.id)
		snd_mixer_selem_id_free(simple->selem.id);
	/* free db range information */
	free(simple->str[0].db_info);
	free(simple->str[1].db_info);
	free(simple);
}

static int simple_update(snd_mixer_elem_t *melem)
{
	selem_none_t *simple;
	unsigned int caps, pchannels, cchannels;
	long pmin, pmax, cmin, cmax;
	selem_ctl_t *ctl;
	const char *name;

	caps = 0;
	pchannels = 0;
	pmin = LONG_MAX;
	pmax = LONG_MIN;
	cchannels = 0;
	cmin = LONG_MAX;
	cmax = LONG_MIN;
	assert(snd_mixer_elem_get_type(melem) == SND_MIXER_ELEM_SIMPLE);
	simple = snd_mixer_elem_get_private(melem);
	name = snd_mixer_selem_get_name(melem);
	ctl = &simple->ctls[CTL_SINGLE];
	if (ctl->elem) {
		pchannels = cchannels = ctl->values;
		if (ctl->type == SND_CTL_ELEM_TYPE_INTEGER) {
			caps |= SM_CAP_GVOLUME;
			pmin = cmin = ctl->min;
			pmax = cmax = ctl->max;
		} else
			caps |= SM_CAP_GSWITCH;
	}
	ctl = &simple->ctls[CTL_GLOBAL_SWITCH];
	if (ctl->elem) {
		if (pchannels < ctl->values)
			pchannels = ctl->values;
		if (cchannels < ctl->values)
			cchannels = ctl->values;
		caps |= SM_CAP_GSWITCH;
	}
	ctl = &simple->ctls[CTL_GLOBAL_ROUTE];
	if (ctl->elem) {
		if (pchannels < ctl->values)
			pchannels = ctl->values;
		if (cchannels < ctl->values)
			cchannels = ctl->values;
		caps |= SM_CAP_GSWITCH;
	}
	ctl = &simple->ctls[CTL_GLOBAL_VOLUME];
	if (ctl->elem) {
		if (pchannels < ctl->values)
			pchannels = ctl->values;
		if (pmin > ctl->min)
			pmin = ctl->min;
		if (pmax < ctl->max)
			pmax = ctl->max;
		if (cchannels < ctl->values)
			cchannels = ctl->values;
		if (cmin > ctl->min)
			cmin = ctl->min;
		if (cmax < ctl->max)
			cmax = ctl->max;
		caps |= SM_CAP_GVOLUME;
	}
	ctl = &simple->ctls[CTL_PLAYBACK_SWITCH];
	if (ctl->elem) {
		if (pchannels < ctl->values)
			pchannels = ctl->values;
		caps |= SM_CAP_PSWITCH;
		caps &= ~SM_CAP_GSWITCH;
	}
	ctl = &simple->ctls[CTL_PLAYBACK_ROUTE];
	if (ctl->elem) {
		if (pchannels < ctl->values)
			pchannels = ctl->values;
		caps |= SM_CAP_PSWITCH;
		caps &= ~SM_CAP_GSWITCH;
	}
	ctl = &simple->ctls[CTL_CAPTURE_SWITCH];
	if (ctl->elem) {
		if (cchannels < ctl->values)
			cchannels = ctl->values;
		caps |= SM_CAP_CSWITCH;
		caps &= ~SM_CAP_GSWITCH;
	}
	ctl = &simple->ctls[CTL_CAPTURE_ROUTE];
	if (ctl->elem) {
		if (cchannels < ctl->values)
			cchannels = ctl->values;
		caps |= SM_CAP_CSWITCH;
		caps &= ~SM_CAP_GSWITCH;
	}
	ctl = &simple->ctls[CTL_PLAYBACK_VOLUME];
	if (ctl->elem) {
		if (pchannels < ctl->values)
			pchannels = ctl->values;
		if (pmin > ctl->min)
			pmin = ctl->min;
		if (pmax < ctl->max)
			pmax = ctl->max;
		caps |= SM_CAP_PVOLUME;
		caps &= ~SM_CAP_GVOLUME;
	}
	ctl = &simple->ctls[CTL_CAPTURE_VOLUME];
	if (ctl->elem) {
		if (cchannels < ctl->values)
			cchannels = ctl->values;
		if (cmin > ctl->min)
			cmin = ctl->min;
		if (cmax < ctl->max)
			cmax = ctl->max;
		caps |= SM_CAP_CVOLUME;
		caps &= ~SM_CAP_GVOLUME;
	}
	ctl = &simple->ctls[CTL_CAPTURE_SOURCE];
	if (ctl->elem) {
		if (cchannels < ctl->values)
			cchannels = ctl->values;
		caps |= SM_CAP_CSWITCH | SM_CAP_CSWITCH_EXCL;
		caps &= ~SM_CAP_GSWITCH;
	}
	ctl = &simple->ctls[CTL_GLOBAL_ENUM];
	if (ctl->elem) {
		if (pchannels < ctl->values)
			pchannels = ctl->values;
		caps |= SM_CAP_PENUM | SM_CAP_CENUM;
	}
	ctl = &simple->ctls[CTL_PLAYBACK_ENUM];
	if (ctl->elem) {
		if (pchannels < ctl->values)
			pchannels = ctl->values;
		caps |= SM_CAP_PENUM;
	}
	ctl = &simple->ctls[CTL_CAPTURE_ENUM];
	if (ctl->elem) {
		if (pchannels < ctl->values)
			pchannels = ctl->values;
		caps |= SM_CAP_CENUM;
	}
	if (pchannels > 32)
		pchannels = 32;
	if (cchannels > 32)
		cchannels = 32;
	if (caps & (SM_CAP_GSWITCH|SM_CAP_PSWITCH))
		caps |= SM_CAP_PSWITCH_JOIN;
	if (caps & (SM_CAP_GVOLUME|SM_CAP_PVOLUME))
		caps |= SM_CAP_PVOLUME_JOIN;
	if (caps & (SM_CAP_GSWITCH|SM_CAP_CSWITCH))
		caps |= SM_CAP_CSWITCH_JOIN;
	if (caps & (SM_CAP_GVOLUME|SM_CAP_CVOLUME))
		caps |= SM_CAP_CVOLUME_JOIN;
	if (pchannels > 1 || cchannels > 1) {
		if (simple->ctls[CTL_SINGLE].elem &&
		    simple->ctls[CTL_SINGLE].values > 1) {
			if (caps & SM_CAP_GSWITCH)
				caps &= ~(SM_CAP_PSWITCH_JOIN|SM_CAP_CSWITCH_JOIN);
			else
				caps &= ~(SM_CAP_PVOLUME_JOIN|SM_CAP_CVOLUME_JOIN);
		}
		if (simple->ctls[CTL_GLOBAL_ROUTE].elem ||
		    (simple->ctls[CTL_GLOBAL_SWITCH].elem &&
		     simple->ctls[CTL_GLOBAL_SWITCH].values > 1)) {
			caps &= ~(SM_CAP_PSWITCH_JOIN|SM_CAP_CSWITCH_JOIN);
		}
		if (simple->ctls[CTL_GLOBAL_VOLUME].elem &&
		    simple->ctls[CTL_GLOBAL_VOLUME].values > 1) {
			caps &= ~(SM_CAP_PVOLUME_JOIN|SM_CAP_CVOLUME_JOIN);
		}
	}
	if (pchannels > 1) {
		if (simple->ctls[CTL_PLAYBACK_ROUTE].elem ||
		    (simple->ctls[CTL_PLAYBACK_SWITCH].elem &&
		     simple->ctls[CTL_PLAYBACK_SWITCH].values > 1)) {
			caps &= ~SM_CAP_PSWITCH_JOIN;
		}
		if (simple->ctls[CTL_PLAYBACK_VOLUME].elem &&
		    simple->ctls[CTL_PLAYBACK_VOLUME].values > 1) {
			caps &= ~SM_CAP_PVOLUME_JOIN;
		}
	}
	if (cchannels > 1) {
		if (simple->ctls[CTL_CAPTURE_ROUTE].elem ||
		    (simple->ctls[CTL_CAPTURE_SWITCH].elem &&
		     simple->ctls[CTL_CAPTURE_SWITCH].values > 1) ||
		    (simple->ctls[CTL_CAPTURE_SOURCE].elem &&
		     simple->ctls[CTL_CAPTURE_SOURCE].values > 1)) {
			caps &= ~SM_CAP_CSWITCH_JOIN;
		}
		if (simple->ctls[CTL_CAPTURE_VOLUME].elem &&
		    simple->ctls[CTL_CAPTURE_VOLUME].values > 1) {
			caps &= ~SM_CAP_CVOLUME_JOIN;
		}
	}

	/* exceptions */
	if ((caps & (SM_CAP_GSWITCH|SM_CAP_PSWITCH|SM_CAP_CSWITCH)) &&
	    (caps & (SM_CAP_GSWITCH|SM_CAP_PSWITCH|SM_CAP_CSWITCH)) == (caps & SM_CAP_GSWITCH)) {
		caps &= ~(SM_CAP_GSWITCH|SM_CAP_CSWITCH_JOIN|SM_CAP_CSWITCH_EXCL);
		caps |= SM_CAP_PSWITCH;
	}

	if ((caps & SM_CAP_GSWITCH) &&
	    (caps & (SM_CAP_PSWITCH|SM_CAP_CSWITCH)) == 0)
		caps |= SM_CAP_PSWITCH|SM_CAP_CSWITCH;

	if ((caps & SM_CAP_GVOLUME) &&
	    (caps & (SM_CAP_PVOLUME|SM_CAP_CVOLUME)) == 0)
		caps |= SM_CAP_PVOLUME|SM_CAP_CVOLUME;

	simple->selem.caps = caps;
	simple->str[SM_PLAY].channels = pchannels;
	if (!simple->str[SM_PLAY].range) {
		simple->str[SM_PLAY].min = pmin != LONG_MAX ? pmin : 0;
		simple->str[SM_PLAY].max = pmax != LONG_MIN ? pmax : 0;
	}
	simple->str[SM_CAPT].channels = cchannels;
	if (!simple->str[SM_CAPT].range) {
		simple->str[SM_CAPT].min = cmin != LONG_MAX ? cmin : 0;
		simple->str[SM_CAPT].max = cmax != LONG_MIN ? cmax : 0;
	}
	return 0;
}	   

#ifndef DOC_HIDDEN
static const struct suf {
	const char *suffix;
	selem_ctl_type_t type;
} suffixes[] = {
	{" Playback Enum", CTL_PLAYBACK_ENUM},
	{" Playback Switch", CTL_PLAYBACK_SWITCH},
	{" Playback Route", CTL_PLAYBACK_ROUTE},
	{" Playback Volume", CTL_PLAYBACK_VOLUME},
	{" Capture Enum", CTL_CAPTURE_ENUM},
	{" Capture Switch", CTL_CAPTURE_SWITCH},
	{" Capture Route", CTL_CAPTURE_ROUTE},
	{" Capture Volume", CTL_CAPTURE_VOLUME},
	{" Enum", CTL_GLOBAL_ENUM},
	{" Switch", CTL_GLOBAL_SWITCH},
	{" Route", CTL_GLOBAL_ROUTE},
	{" Volume", CTL_GLOBAL_VOLUME},
	{NULL, 0}
};
#endif

/* Return base length or 0 on failure */
static int base_len(const char *name, selem_ctl_type_t *type)
{
	const struct suf *p;
	size_t nlen = strlen(name);
	p = suffixes;
	while (p->suffix) {
		size_t slen = strlen(p->suffix);
		size_t l;
		if (nlen > slen) {
			l = nlen - slen;
			if (strncmp(name + l, p->suffix, slen) == 0 &&
			    (l < 1 || name[l-1] != '-')) {	/* 3D Control - Switch */
				*type = p->type;
				return l;
			}
		}
		p++;
	}

	if (!strcmp(name, "Input Source")) {
		*type = CTL_CAPTURE_ROUTE;
		return strlen(name);
	}

	return 0;
}


/*
 * Simple Mixer Operations
 */
	
static int _snd_mixer_selem_set_volume(snd_mixer_elem_t *elem, int dir, snd_mixer_selem_channel_id_t channel, long value)
{
	selem_none_t *s = snd_mixer_elem_get_private(elem);
	if (s->selem.caps & SM_CAP_GVOLUME)
		dir = SM_PLAY;
	if ((unsigned int) channel >= s->str[dir].channels)
		return 0;
	if (value < s->str[dir].min || value > s->str[dir].max)
		return 0;
	if (s->selem.caps & 
	    (dir == SM_PLAY ? SM_CAP_PVOLUME_JOIN : SM_CAP_CVOLUME_JOIN))
		channel = 0;
	if (value != s->str[dir].vol[channel]) {
		s->str[dir].vol[channel] = value;
		return 1;
	}
	return 0;
}

static int _snd_mixer_selem_set_switch(snd_mixer_elem_t *elem, int dir, snd_mixer_selem_channel_id_t channel, int value)
{
	selem_none_t *s = snd_mixer_elem_get_private(elem);
	if ((unsigned int) channel >= s->str[dir].channels)
		return 0;
	if (s->selem.caps & 
	    (dir == SM_PLAY ? SM_CAP_PSWITCH_JOIN : SM_CAP_CSWITCH_JOIN))
		channel = 0;
	if (value) {
		if (!(s->str[dir].sw & (1 << channel))) {
			s->str[dir].sw |= 1 << channel;
			return 1;
		}
	} else {
		if (s->str[dir].sw & (1 << channel)) {
			s->str[dir].sw &= ~(1 << channel);
			return 1;
		}
	}
	return 0;
}

static int is_ops(snd_mixer_elem_t *elem, int dir, int cmd, int val)
{
	selem_none_t *s = snd_mixer_elem_get_private(elem);
	
	switch (cmd) {

	case SM_OPS_IS_ACTIVE: {
		selem_ctl_type_t ctl;
		for (ctl = CTL_SINGLE; ctl <= CTL_LAST; ctl++)
			if (s->ctls[ctl].elem != NULL && s->ctls[ctl].inactive)
				return 0;
		return 1;
	}

	case SM_OPS_IS_MONO:
		return s->str[dir].channels == 1;

	case SM_OPS_IS_CHANNEL:
		return (unsigned int) val < s->str[dir].channels;

	case SM_OPS_IS_ENUMERATED:
		if (val == 1) {
			if (dir == SM_PLAY && (s->selem.caps & SM_CAP_PENUM) && !(s->selem.caps & SM_CAP_CENUM) )
				return 1;
			if (dir == SM_CAPT && (s->selem.caps & SM_CAP_CENUM) && !(s->selem.caps & SM_CAP_PENUM) )
				return 1;
			return 0;
		}
		if (s->selem.caps & (SM_CAP_CENUM | SM_CAP_PENUM) )
			return 1;
		return 0;
	
	case SM_OPS_IS_ENUMCNT:
		/* Both */
		if ( (s->selem.caps & (SM_CAP_CENUM | SM_CAP_PENUM)) == (SM_CAP_CENUM | SM_CAP_PENUM) ) {
			if (! s->ctls[CTL_GLOBAL_ENUM].elem)
				return -EINVAL;
			return s->ctls[CTL_GLOBAL_ENUM].max;
		/* Only Playback */
		} else if (s->selem.caps & SM_CAP_PENUM ) {
			if (! s->ctls[CTL_PLAYBACK_ENUM].elem)
				return -EINVAL;
			return s->ctls[CTL_PLAYBACK_ENUM].max;
		/* Only Capture */
		} else if (s->selem.caps & SM_CAP_CENUM ) {
			if (! s->ctls[CTL_CAPTURE_ENUM].elem)
				return -EINVAL;
			return s->ctls[CTL_CAPTURE_ENUM].max;
		}

	}
	
	return 1;
}

static int get_range_ops(snd_mixer_elem_t *elem, int dir,
			 long *min, long *max)
{
	selem_none_t *s = snd_mixer_elem_get_private(elem);
	*min = s->str[dir].min;
	*max = s->str[dir].max;
	return 0;
}

static int set_range_ops(snd_mixer_elem_t *elem, int dir,
			 long min, long max)
{
	selem_none_t *s = snd_mixer_elem_get_private(elem);
	int err;

	s->str[dir].range = 1;
	s->str[dir].min = min;
	s->str[dir].max = max;
	if ((err = selem_read(elem)) < 0)
		return err;
	return 0;
}

static int get_volume_ops(snd_mixer_elem_t *elem, int dir,
			  snd_mixer_selem_channel_id_t channel, long *value)
{
	selem_none_t *s = snd_mixer_elem_get_private(elem);
	if (s->selem.caps & SM_CAP_GVOLUME)
		dir = SM_PLAY;
	if ((unsigned int) channel >= s->str[dir].channels)
		return -EINVAL;
	*value = s->str[dir].vol[channel];
	return 0;
}

static int init_db_range(snd_hctl_elem_t *ctl, struct selem_str *rec);

static int convert_to_dB(snd_hctl_elem_t *ctl, struct selem_str *rec,
			 long volume, long *db_gain)
{
	if (init_db_range(ctl, rec) < 0)
		return -EINVAL;
	return snd_tlv_convert_to_dB(rec->db_info, rec->min, rec->max,
				     volume, db_gain);
}

/* initialize dB range information, reading TLV via hcontrol
 */
static int init_db_range(snd_hctl_elem_t *ctl, struct selem_str *rec)
{
	snd_ctl_elem_info_t *info;
	unsigned int *tlv = NULL;
	const unsigned int tlv_size = 4096;
	unsigned int *dbrec;
	int db_size;

	if (rec->db_init_error)
		return -EINVAL;
	if (rec->db_initialized)
		return 0;

	snd_ctl_elem_info_alloca(&info);
	if (snd_hctl_elem_info(ctl, info) < 0)
		goto error;
	if (! snd_ctl_elem_info_is_tlv_readable(info))
		goto error;
	tlv = malloc(tlv_size);
	if (! tlv)
		return -ENOMEM;
	if (snd_hctl_elem_tlv_read(ctl, tlv, tlv_size) < 0)
		goto error;
	db_size = snd_tlv_parse_dB_info(tlv, tlv_size, &dbrec);
	if (db_size < 0)
		goto error;
	rec->db_info = malloc(db_size);
	if (!rec->db_info)
		goto error;
	memcpy(rec->db_info, dbrec, db_size);
	free(tlv);
	rec->db_initialized = 1;
	return 0;

 error:
	free(tlv);
	rec->db_init_error = 1;
	return -EINVAL;
}

/* get selem_ctl for TLV access */
static selem_ctl_t *get_selem_ctl(selem_none_t *s, int dir)
{
	selem_ctl_t *c;
	if (dir == SM_PLAY)
		c = &s->ctls[CTL_PLAYBACK_VOLUME];
	else if (dir == SM_CAPT)
		c = &s->ctls[CTL_CAPTURE_VOLUME];
	else
		return NULL;
	if (! c->elem) {
		c = &s->ctls[CTL_GLOBAL_VOLUME];
		if (! c->elem)
			return NULL;
	}
	if (c->type != SND_CTL_ELEM_TYPE_INTEGER)
		return NULL;
	return c;
}

static int get_dB_range(snd_hctl_elem_t *ctl, struct selem_str *rec,
			long *min, long *max)
{
	if (init_db_range(ctl, rec) < 0)
		return -EINVAL;

	return snd_tlv_get_dB_range(rec->db_info, rec->min, rec->max, min, max);
}
	
static int get_dB_range_ops(snd_mixer_elem_t *elem, int dir,
			    long *min, long *max)
{
	selem_none_t *s = snd_mixer_elem_get_private(elem);
	selem_ctl_t *c;

	if (s->selem.caps & SM_CAP_GVOLUME)
		dir = SM_PLAY;
	c = get_selem_ctl(s, dir);
	if (! c)
		return -EINVAL;
	return get_dB_range(c->elem, &s->str[dir], min, max);
}

static int convert_from_dB(snd_hctl_elem_t *ctl, struct selem_str *rec,
			   long db_gain, long *value, int xdir)
{
	if (init_db_range(ctl, rec) < 0)
		return -EINVAL;

	return snd_tlv_convert_from_dB(rec->db_info, rec->min, rec->max,
				       db_gain, value, xdir);
}

static int ask_vol_dB_ops(snd_mixer_elem_t *elem,
			  int dir,
			  long value,
			  long *dBvalue)
{
	selem_none_t *s = snd_mixer_elem_get_private(elem);
	selem_ctl_t *c;

	c = get_selem_ctl(s, dir);
	if (! c)
		return -EINVAL;
	int res = convert_to_dB(c->elem, &s->str[dir], value, dBvalue);
	return res;
}

static int get_dB_ops(snd_mixer_elem_t *elem,
                      int dir,
                      snd_mixer_selem_channel_id_t channel,
                      long *value)
{
	selem_none_t *s = snd_mixer_elem_get_private(elem);
	selem_ctl_t *c;
	int err;
	long volume, db_gain;

	if (s->selem.caps & SM_CAP_GVOLUME)
		dir = SM_PLAY;
	c = get_selem_ctl(s, dir);
	if (! c)
		return -EINVAL;
	if ((err = get_volume_ops(elem, dir, channel, &volume)) < 0)
		goto _err;
	if ((err = convert_to_dB(c->elem, &s->str[dir], volume, &db_gain)) < 0)
		goto _err;
	err = 0;
	*value = db_gain;
 _err:
	return err;
}

static int get_switch_ops(snd_mixer_elem_t *elem, int dir,
			  snd_mixer_selem_channel_id_t channel, int *value)
{
	selem_none_t *s = snd_mixer_elem_get_private(elem);
	if (s->selem.caps & SM_CAP_GSWITCH)
		dir = SM_PLAY;
	if ((unsigned int) channel >= s->str[dir].channels)
		return -EINVAL;
	*value = !!(s->str[dir].sw & (1 << channel));
	return 0;
}

static int set_volume_ops(snd_mixer_elem_t *elem, int dir,
			  snd_mixer_selem_channel_id_t channel, long value)
{
	int changed;
	changed = _snd_mixer_selem_set_volume(elem, dir, channel, value);
	if (changed < 0)
		return changed;
	if (changed)
		return selem_write(elem);
	return 0;
}

static int ask_dB_vol_ops(snd_mixer_elem_t *elem, int dir,
		          long dbValue, long *value, int xdir)
{
	selem_none_t *s = snd_mixer_elem_get_private(elem);
	selem_ctl_t *c;

	if (s->selem.caps & SM_CAP_GVOLUME)
		dir = SM_PLAY;
	c = get_selem_ctl(s, dir);
	if (! c)
		return -EINVAL;
	return convert_from_dB(c->elem, &s->str[dir], dbValue, value, xdir);
}

static int set_dB_ops(snd_mixer_elem_t *elem, int dir,
		      snd_mixer_selem_channel_id_t channel,
		      long db_gain, int xdir)
{
	selem_none_t *s = snd_mixer_elem_get_private(elem);
	selem_ctl_t *c;
	long value;
	int err;

	if (s->selem.caps & SM_CAP_GVOLUME)
		dir = SM_PLAY;
	c = get_selem_ctl(s, dir);
	if (! c)
		return -EINVAL;
	err = convert_from_dB(c->elem, &s->str[dir], db_gain, &value, xdir);
	if (err < 0)
		return err;
	return set_volume_ops(elem, dir, channel, value);
}

static int set_switch_ops(snd_mixer_elem_t *elem, int dir,
			  snd_mixer_selem_channel_id_t channel, int value)
{
	int changed;
	selem_none_t *s = snd_mixer_elem_get_private(elem);
	if (s->selem.caps & SM_CAP_GSWITCH)
		dir = SM_PLAY;
	if (dir == SM_PLAY) {
		if (! (s->selem.caps & (SM_CAP_GSWITCH|SM_CAP_PSWITCH)))
			return -EINVAL;
	} else {
		if (! (s->selem.caps & (SM_CAP_GSWITCH|SM_CAP_CSWITCH)))
			return -EINVAL;
	}
	changed = _snd_mixer_selem_set_switch(elem, dir, channel, value);
	if (changed < 0)
		return changed;
	if (changed)
		return selem_write(elem);
	return 0;
}

static int enum_item_name_ops(snd_mixer_elem_t *elem,
			      unsigned int item,
			      size_t maxlen, char *buf)
{
	selem_none_t *s = snd_mixer_elem_get_private(elem);
	snd_ctl_elem_info_t *info;
	snd_hctl_elem_t *helem;
	int type;

	type = CTL_GLOBAL_ENUM;
	helem = s->ctls[type].elem;
	if (!helem) {
		type = CTL_PLAYBACK_ENUM;
		helem = s->ctls[type].elem;
	}
	if (!helem) {
		type = CTL_CAPTURE_ENUM;
		helem = s->ctls[type].elem;
	}
	assert(helem);
	if (item >= (unsigned int)s->ctls[type].max)
		return -EINVAL;
	snd_ctl_elem_info_alloca(&info);
	snd_hctl_elem_info(helem, info);
	snd_ctl_elem_info_set_item(info, item);
	snd_hctl_elem_info(helem, info);
	strncpy(buf, snd_ctl_elem_info_get_item_name(info), maxlen);
	return 0;
}

static int get_enum_item_ops(snd_mixer_elem_t *elem,
			     snd_mixer_selem_channel_id_t channel,
			     unsigned int *itemp)
{
	selem_none_t *s = snd_mixer_elem_get_private(elem);
	snd_ctl_elem_value_t *ctl;
	snd_hctl_elem_t *helem;
	int err;

	if ((unsigned int) channel >= s->str[0].channels)
		return -EINVAL;
	helem = s->ctls[CTL_GLOBAL_ENUM].elem;
	if (!helem) helem = s->ctls[CTL_PLAYBACK_ENUM].elem;
	if (!helem) helem = s->ctls[CTL_CAPTURE_ENUM].elem;
	assert(helem);
	snd_ctl_elem_value_alloca(&ctl);
	err = snd_hctl_elem_read(helem, ctl);
	if (! err)
		*itemp = snd_ctl_elem_value_get_enumerated(ctl, channel);
	return err;
}

static int set_enum_item_ops(snd_mixer_elem_t *elem,
			     snd_mixer_selem_channel_id_t channel,
			     unsigned int item)
{
	selem_none_t *s = snd_mixer_elem_get_private(elem);
	snd_ctl_elem_value_t *ctl;
	snd_hctl_elem_t *helem;
	int err;
	int type;

	if ((unsigned int) channel >= s->str[0].channels) {
		return -EINVAL;
	}
	type = CTL_GLOBAL_ENUM;
	helem = s->ctls[type].elem;
	if (!helem) {
		type = CTL_PLAYBACK_ENUM;
		helem = s->ctls[type].elem;
	}
	if (!helem) {
		type = CTL_CAPTURE_ENUM;
		helem = s->ctls[type].elem;
	}
	assert(helem);
	if (item >= (unsigned int)s->ctls[type].max) {
		return -EINVAL;
	}
	snd_ctl_elem_value_alloca(&ctl);
	err = snd_hctl_elem_read(helem, ctl);
	if (err < 0) {
		return err;
	}
	snd_ctl_elem_value_set_enumerated(ctl, channel, item);
	return snd_hctl_elem_write(helem, ctl);
}

static struct sm_elem_ops simple_none_ops = {
	.is		= is_ops,
	.get_range	= get_range_ops,
	.get_dB_range	= get_dB_range_ops,
	.set_range	= set_range_ops,
	.ask_vol_dB	= ask_vol_dB_ops,
	.ask_dB_vol	= ask_dB_vol_ops,
	.get_volume	= get_volume_ops,
	.get_dB		= get_dB_ops,
	.set_volume	= set_volume_ops,
	.set_dB		= set_dB_ops,
	.get_switch	= get_switch_ops,
	.set_switch	= set_switch_ops,
	.enum_item_name	= enum_item_name_ops,
	.get_enum_item	= get_enum_item_ops,
	.set_enum_item	= set_enum_item_ops
};

static int simple_add1(snd_mixer_class_t *class, const char *name,
		       snd_hctl_elem_t *helem, selem_ctl_type_t type,
		       unsigned int value)
{
	snd_mixer_elem_t *melem;
	snd_mixer_selem_id_t *id;
	int new = 0;
	int err;
	snd_ctl_elem_info_t *info;
	selem_none_t *simple;
	const char *name1;
	snd_ctl_elem_type_t ctype;
	unsigned long values;

	snd_ctl_elem_info_alloca(&info);
	err = snd_hctl_elem_info(helem, info);
	if (err < 0)
		return err;
	ctype = snd_ctl_elem_info_get_type(info);
	values = snd_ctl_elem_info_get_count(info);
	switch (type) {
	case CTL_SINGLE:
		if (ctype == SND_CTL_ELEM_TYPE_ENUMERATED)
			type = CTL_GLOBAL_ENUM;
		else if (ctype != SND_CTL_ELEM_TYPE_BOOLEAN &&
		         ctype != SND_CTL_ELEM_TYPE_INTEGER)
			return 0;
		break;
	case CTL_GLOBAL_ROUTE:
	case CTL_PLAYBACK_ROUTE:
	case CTL_CAPTURE_ROUTE:
	{
		unsigned int n;
		if (ctype == SND_CTL_ELEM_TYPE_ENUMERATED) {
			if (type == CTL_PLAYBACK_ROUTE)
				type = CTL_PLAYBACK_ENUM;
			else if (type == CTL_CAPTURE_ROUTE)
				type = CTL_CAPTURE_ENUM;
			else
				type = CTL_GLOBAL_ENUM;
			break;
		}
		if (ctype != SND_CTL_ELEM_TYPE_BOOLEAN)
			return 0;
#ifdef HAVE_SOFT_FLOAT
		/* up to 256 channels */
		for (n = 1; n < 256; n++)
			if (n * n == values)
				break;
#else
		n = sqrt((double)values);
#endif
		if (n * n != values)
			return 0;
		values = n;
		break;
	}
	case CTL_GLOBAL_SWITCH:
	case CTL_PLAYBACK_SWITCH:
	case CTL_CAPTURE_SWITCH:
		if (ctype == SND_CTL_ELEM_TYPE_ENUMERATED) {
			if (type == CTL_PLAYBACK_SWITCH)
				type = CTL_PLAYBACK_ENUM;
			else if (type == CTL_CAPTURE_SWITCH)
				type = CTL_CAPTURE_ENUM;
			else
				type = CTL_GLOBAL_ENUM;
			break;
		}
		if (ctype != SND_CTL_ELEM_TYPE_BOOLEAN)
			return 0;
		break;
	case CTL_GLOBAL_VOLUME:
	case CTL_PLAYBACK_VOLUME:
	case CTL_CAPTURE_VOLUME:
		if (ctype == SND_CTL_ELEM_TYPE_ENUMERATED) {
			if (type == CTL_PLAYBACK_VOLUME)
				type = CTL_PLAYBACK_ENUM;
			else if (type == CTL_CAPTURE_VOLUME)
				type = CTL_CAPTURE_ENUM;
			else
				type = CTL_GLOBAL_ENUM;
			break;
		}
		if (ctype != SND_CTL_ELEM_TYPE_INTEGER)
			return 0;
		break;
	case CTL_CAPTURE_SOURCE:
		if (ctype != SND_CTL_ELEM_TYPE_ENUMERATED)
			return 0;
		break;
	case CTL_GLOBAL_ENUM:
	case CTL_PLAYBACK_ENUM:
	case CTL_CAPTURE_ENUM:
		if (ctype != SND_CTL_ELEM_TYPE_ENUMERATED)
			return 0;
		break;
	default:
		assert(0);
		break;
	}
	name1 = get_short_name(name);
	if (snd_mixer_selem_id_malloc(&id))
		return -ENOMEM;
	snd_mixer_selem_id_set_name(id, name1);
	snd_mixer_selem_id_set_index(id, snd_hctl_elem_get_index(helem));
	melem = snd_mixer_find_selem(snd_mixer_class_get_mixer(class), id);
	if (!melem) {
		simple = calloc(1, sizeof(*simple));
		if (!simple) {
			snd_mixer_selem_id_free(id);
			return -ENOMEM;
		}
		simple->selem.id = id;
		simple->selem.ops = &simple_none_ops;
		err = snd_mixer_elem_new(&melem, SND_MIXER_ELEM_SIMPLE,
					 get_compare_weight(snd_mixer_selem_id_get_name(simple->selem.id), snd_mixer_selem_id_get_index(simple->selem.id)),
					 simple, selem_free);
		if (err < 0) {
			snd_mixer_selem_id_free(id);
			free(simple);
			return err;
		}
		new = 1;
	} else {
		simple = snd_mixer_elem_get_private(melem);
		snd_mixer_selem_id_free(id);
	}
	if (simple->ctls[type].elem) {
		SNDERR("helem (%s,'%s',%u,%u,%u) appears twice or more",
				snd_ctl_elem_iface_name(snd_hctl_elem_get_interface(helem)),
				snd_hctl_elem_get_name(helem),
				snd_hctl_elem_get_index(helem),
				snd_hctl_elem_get_device(helem),
				snd_hctl_elem_get_subdevice(helem));
		err = -EINVAL;
		goto __error;
	}
	simple->ctls[type].elem = helem;
	simple->ctls[type].type = snd_ctl_elem_info_get_type(info);
	simple->ctls[type].inactive = snd_ctl_elem_info_is_inactive(info);
	simple->ctls[type].values = values;
	if ( (type == CTL_GLOBAL_ENUM) ||
	     (type == CTL_PLAYBACK_ENUM) ||
	     (type == CTL_CAPTURE_ENUM) ) {
		simple->ctls[type].min = 0;
		simple->ctls[type].max = snd_ctl_elem_info_get_items(info);
	} else {
		if (ctype == SND_CTL_ELEM_TYPE_INTEGER) {
			simple->ctls[type].min = snd_ctl_elem_info_get_min(info);
			simple->ctls[type].max = snd_ctl_elem_info_get_max(info);
		}
	}
	switch (type) {
	case CTL_CAPTURE_SOURCE:
		simple->capture_item = value;
		break;
	default:
		break;
	}
	err = snd_mixer_elem_attach(melem, helem);
	if (err < 0)
		goto __error;
	err = simple_update(melem);
	if (err < 0) {
		if (new)
			goto __error;
		return err;
	}
	if (new)
		err = snd_mixer_elem_add(melem, class);
	else
		err = snd_mixer_elem_info(melem);
	if (err < 0)
		return err;
	err = selem_read(melem);
	if (err < 0)
		return err;
	if (err)
		err = snd_mixer_elem_value(melem);
	return err;
      __error:
	if (new)
		snd_mixer_elem_free(melem);
	return -EINVAL;
}

static int simple_event_add(snd_mixer_class_t *class, snd_hctl_elem_t *helem)
{
	const char *name = snd_hctl_elem_get_name(helem);
	size_t len;
	selem_ctl_type_t type = CTL_SINGLE; /* to shut up warning */
	if (snd_hctl_elem_get_interface(helem) != SND_CTL_ELEM_IFACE_MIXER)
		return 0;
	if (strcmp(name, "Capture Source") == 0) {
		snd_ctl_elem_info_t *info;
		unsigned int k, items;
		int err;
		snd_ctl_elem_info_alloca(&info);
		err = snd_hctl_elem_info(helem, info);
		assert(err >= 0);
		if (snd_ctl_elem_info_get_type(info) != SND_CTL_ELEM_TYPE_ENUMERATED)
			return 0;
		items = snd_ctl_elem_info_get_items(info);
		for (k = 0; k < items; ++k) {
			const char *n;
			snd_ctl_elem_info_set_item(info, k);
			err = snd_hctl_elem_info(helem, info);
			if (err < 0)
				return err;
			n = snd_ctl_elem_info_get_item_name(info);
			err = simple_add1(class, n, helem, CTL_CAPTURE_SOURCE, k);
			if (err < 0)
				return err;
		}
		return 0;
	}
	len = base_len(name, &type);
	if (len == 0) {
		return simple_add1(class, name, helem, CTL_SINGLE, 0);
	} else {
		char ename[128];
		if (len >= sizeof(ename))
			len = sizeof(ename) - 1;
		memcpy(ename, name, len);
		ename[len] = 0;
		/* exception: Capture Volume and Capture Switch */
		if (type == CTL_GLOBAL_VOLUME && !strcmp(ename, "Capture"))
			type = CTL_CAPTURE_VOLUME;
		else if (type == CTL_GLOBAL_SWITCH && !strcmp(ename, "Capture"))
			type = CTL_CAPTURE_SWITCH;
		return simple_add1(class, ename, helem, type, 0);
	}
}

static int simple_event_remove(snd_hctl_elem_t *helem,
			       snd_mixer_elem_t *melem)
{
	selem_none_t *simple = snd_mixer_elem_get_private(melem);
	int err;
	int k;
	for (k = 0; k <= CTL_LAST; k++) {
		if (simple->ctls[k].elem == helem)
			break;
	}
	assert(k <= CTL_LAST);
	simple->ctls[k].elem = NULL;
	err = snd_mixer_elem_detach(melem, helem);
	if (err < 0)
		return err;
	if (snd_mixer_elem_empty(melem))
		return snd_mixer_elem_remove(melem);
	err = simple_update(melem);
	return snd_mixer_elem_info(melem);
}

static int simple_event(snd_mixer_class_t *class, unsigned int mask,
			snd_hctl_elem_t *helem, snd_mixer_elem_t *melem)
{
	int err;
	if (mask == SND_CTL_EVENT_MASK_REMOVE)
		return simple_event_remove(helem, melem);
	if (mask & SND_CTL_EVENT_MASK_ADD) {
		err = simple_event_add(class, helem);
		if (err < 0)
			return err;
	}
	if (mask & SND_CTL_EVENT_MASK_INFO) {
		err = simple_event_remove(helem, melem);
		if (err < 0)
			return err;
		err = simple_event_add(class, helem);
		if (err < 0)
			return err;
		return 0;
	}
	if (mask & SND_CTL_EVENT_MASK_VALUE) {
		err = selem_read(melem);
		if (err < 0)
			return err;
		if (err) {
			err = snd_mixer_elem_value(melem);
			if (err < 0)
				return err;
		}
	}
	return 0;
}

/**
 * \brief Register mixer simple element class - none abstraction
 * \param mixer Mixer handle
 * \param options Options container
 * \param classp Pointer to returned mixer simple element class handle (or NULL)
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_simple_none_register(snd_mixer_t *mixer,
				   struct snd_mixer_selem_regopt *options ATTRIBUTE_UNUSED,
				   snd_mixer_class_t **classp)
{
	snd_mixer_class_t *class;
	int err;

	if (snd_mixer_class_malloc(&class))
		return -ENOMEM;
	snd_mixer_class_set_event(class, simple_event);
	snd_mixer_class_set_compare(class, snd_mixer_selem_compare);
	err = snd_mixer_class_register(class, mixer);
	if (err < 0) {
		free(class);
		return err;
	}
	if (classp)
		*classp = class;
	return 0;
}
