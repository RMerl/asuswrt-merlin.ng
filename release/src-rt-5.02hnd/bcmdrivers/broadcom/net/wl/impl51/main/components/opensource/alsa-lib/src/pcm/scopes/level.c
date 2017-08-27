/*
 *  PCM - Meter level plugin (ncurses)
 *  Copyright (c) 2001 by Abramo Bagnara <abramo@alsa-project.org>
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

#include <curses.h>
#include <errno.h>
#include <alsa/asoundlib.h>

#define BAR_WIDTH 70
/* milliseconds to go from 32767 to 0 */
#define DECAY_MS 400
/* milliseconds for peak to disappear */
#define PEAK_MS 800

typedef struct _snd_pcm_scope_level_channel {
	int16_t level;
	int16_t peak;
	unsigned int peak_age;
} snd_pcm_scope_level_channel_t;

typedef struct _snd_pcm_scope_level {
	snd_pcm_t *pcm;
	snd_pcm_scope_t *s16;
	snd_pcm_scope_level_channel_t *channels;
	snd_pcm_uframes_t old;
	int top;
	WINDOW *win;
	unsigned int bar_width;
	unsigned int decay_ms;
	unsigned int peak_ms;
} snd_pcm_scope_level_t;

static int level_enable(snd_pcm_scope_t *scope)
{
	snd_pcm_scope_level_t *level = snd_pcm_scope_get_callback_private(scope);
	int y, x;
	level->channels = calloc(snd_pcm_meter_get_channels(level->pcm), sizeof(*level->channels));
	if (!level->channels) {
		free(level);
		return -ENOMEM;
	}
	snd_pcm_scope_set_callback_private(scope, level);
	level->win = initscr();
	winsdelln(level->win, snd_pcm_meter_get_channels(level->pcm));
        getyx(level->win, y, x);
	level->top = y;
	return 0;
}

static void level_disable(snd_pcm_scope_t *scope)
{
	snd_pcm_scope_level_t *level = snd_pcm_scope_get_callback_private(scope);
	endwin();
	free(level->channels);
}

static void level_close(snd_pcm_scope_t *scope)
{
	snd_pcm_scope_level_t *level = snd_pcm_scope_get_callback_private(scope);
	free(level);
}

static void level_start(snd_pcm_scope_t *scope ATTRIBUTE_UNUSED)
{
}

static void level_stop(snd_pcm_scope_t *scope)
{
	snd_pcm_scope_level_t *level = snd_pcm_scope_get_callback_private(scope);
	unsigned int c;
	for (c = 0; c < snd_pcm_meter_get_channels(level->pcm); c++) {
		move(level->top + c, 0);
		clrtoeol();
	}
	move(level->top, 0);
	refresh();
}

static void level_update(snd_pcm_scope_t *scope)
{
	snd_pcm_scope_level_t *level = snd_pcm_scope_get_callback_private(scope);
	snd_pcm_t *pcm = level->pcm;
	snd_pcm_sframes_t size;
	snd_pcm_uframes_t size1, size2;
	snd_pcm_uframes_t offset, cont;
	unsigned int c, channels;
	unsigned int ms;
	static char bar[256] = { [0 ... 255] = '#' };
	int max_decay;
	size = snd_pcm_meter_get_now(pcm) - level->old;
	if (size < 0)
		size += snd_pcm_meter_get_boundary(pcm);
	offset = level->old % snd_pcm_meter_get_bufsize(pcm);
	cont = snd_pcm_meter_get_bufsize(pcm) - offset;
	size1 = size;
	if (size1 > cont)
		size1 = cont;
	size2 = size - size1;
	ms = size * 1000 / snd_pcm_meter_get_rate(pcm);
	max_decay = 32768 * ms / level->decay_ms;
	channels = snd_pcm_meter_get_channels(pcm);
	for (c = 0; c < channels; c++) {
		int16_t *ptr;
		int s, lev = 0;
		snd_pcm_uframes_t n;
		snd_pcm_scope_level_channel_t *l;
		unsigned int lev_pos, peak_pos;
		l = &level->channels[c];
		ptr = snd_pcm_scope_s16_get_channel_buffer(level->s16, c) + offset;
		for (n = size1; n > 0; n--) {
			s = *ptr;
			if (s < 0)
				s = -s;
			if (s > lev)
				lev = s;
			ptr++;
		}
		ptr = snd_pcm_scope_s16_get_channel_buffer(level->s16, c);
		for (n = size2; n > 0; n--) {
			s = *ptr;
			if (s < 0)
				s = -s;
			if (s > lev)
				lev = s;
			ptr++;
		}
		l->level = lev;
		l->peak_age += ms;
		if (l->peak_age >= level->peak_ms ||
		    lev >= l->peak) {
			l->peak = lev;
			l->peak_age = 0;
		}
		if (lev < l->level - max_decay)
			lev = l->level - max_decay;
		move(level->top + c, 0);
		lev_pos = lev * level->bar_width / 32768;
		peak_pos = l->peak * level->bar_width / 32768;
		addnstr(bar, lev_pos);
		clrtoeol();
		mvaddch(level->top + c, peak_pos - 1, '#');
	}
	move(level->top, 0);
	refresh();
	level->old = snd_pcm_meter_get_now(pcm);
}

static void level_reset(snd_pcm_scope_t *scope)
{
	snd_pcm_scope_level_t *level = snd_pcm_scope_get_callback_private(scope);
	snd_pcm_t *pcm = level->pcm;
	memset(level->channels, 0, snd_pcm_meter_get_channels(pcm) * sizeof(*level->channels));
	level->old = snd_pcm_meter_get_now(pcm);
}

snd_pcm_scope_ops_t level_ops = {
	.enable = level_enable,
	.disable = level_disable,
	.close = level_close,
	.start = level_start,
	.stop = level_stop,
	.update = level_update,
	.reset = level_reset,
};

int snd_pcm_scope_level_open(snd_pcm_t *pcm, const char *name,
			     unsigned int bar_width, unsigned int decay_ms,
			     unsigned int peak_ms,
			     snd_pcm_scope_t **scopep)
{
	snd_pcm_scope_t *scope, *s16;
	snd_pcm_scope_level_t *level;
	int err = snd_pcm_scope_malloc(&scope);
	if (err < 0)
		return err;
	level = calloc(1, sizeof(*level));
	if (!level) {
		free(scope);
		return -ENOMEM;
	}
	level->pcm = pcm;
	level->bar_width = bar_width;
	level->decay_ms = decay_ms;
	level->peak_ms = peak_ms;
	s16 = snd_pcm_meter_search_scope(pcm, "s16");
	if (!s16) {
		err = snd_pcm_scope_s16_open(pcm, "s16", &s16);
		if (err < 0) {
			free(scope);
			free(level);
			return err;
		}
	}
	level->s16 = s16;
	snd_pcm_scope_set_ops(scope, &level_ops);
	snd_pcm_scope_set_callback_private(scope, level);
	if (name)
		snd_pcm_scope_set_name(scope, strdup(name));
	snd_pcm_meter_add_scope(pcm, scope);
	*scopep = scope;
	return 0;
}

int _snd_pcm_scope_level_open(snd_pcm_t *pcm, const char *name,
			      snd_config_t *root, snd_config_t *conf)
{
	snd_config_iterator_t i, next;
	snd_pcm_scope_t *scope;
	long bar_width = -1, decay_ms = -1, peak_ms = -1;
	int err;
	snd_config_for_each(i, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (strcmp(id, "comment") == 0)
			continue;
		if (strcmp(id, "type") == 0)
			continue;
		if (strcmp(id, "bar_width") == 0) {
			err = snd_config_get_integer(n, &bar_width);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				return -EINVAL;
			}
			continue;
		}
		if (strcmp(id, "decay_ms") == 0) {
			err = snd_config_get_integer(n, &decay_ms);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				return -EINVAL;
			}
			continue;
		}
		if (strcmp(id, "peak_ms") == 0) {
			err = snd_config_get_integer(n, &peak_ms);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				return -EINVAL;
			}
			continue;
		}
		SNDERR("Unknown field %s", id);
		return -EINVAL;
	}
	if (bar_width < 0)
		bar_width = BAR_WIDTH;
	if (decay_ms < 0)
		decay_ms = DECAY_MS;
	if (peak_ms < 0)
		peak_ms = PEAK_MS;
	return snd_pcm_scope_level_open(pcm, name, bar_width, decay_ms, peak_ms,
					&scope);
}
