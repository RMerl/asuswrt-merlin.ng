/*
 * mixer_controls.c - handles mixer controls and mapping from selems
 * Copyright (c) 1998,1999 Tim Janik <timj@gtk.org>
 *                         Jaroslav Kysela <perex@perex.cz>
 * Copyright (c) 2009      Clemens Ladisch <clemens@ladisch.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "aconfig.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include CURSESINC
#include <alsa/asoundlib.h>
#include "utils.h"
#include "mem.h"
#include "mixer_display.h"
#include "mixer_widget.h"
#include "mixer_controls.h"

struct control *controls;
unsigned int controls_count;

static const snd_mixer_selem_channel_id_t supported_channels[] = {
	SND_MIXER_SCHN_FRONT_LEFT,
	SND_MIXER_SCHN_FRONT_RIGHT,
	SND_MIXER_SCHN_REAR_LEFT,
	SND_MIXER_SCHN_REAR_RIGHT,
	SND_MIXER_SCHN_FRONT_CENTER,
	SND_MIXER_SCHN_WOOFER,
	SND_MIXER_SCHN_SIDE_LEFT,
	SND_MIXER_SCHN_SIDE_RIGHT,
};
#define LAST_SUPPORTED_CHANNEL SND_MIXER_SCHN_SIDE_RIGHT

static const snd_mixer_selem_channel_id_t control_channels[][2] = {
	{ SND_MIXER_SCHN_FRONT_LEFT, SND_MIXER_SCHN_FRONT_RIGHT },
	{ SND_MIXER_SCHN_REAR_LEFT, SND_MIXER_SCHN_REAR_RIGHT },
	{ SND_MIXER_SCHN_FRONT_CENTER, SND_MIXER_SCHN_UNKNOWN },
	{ SND_MIXER_SCHN_WOOFER, SND_MIXER_SCHN_UNKNOWN },
	{ SND_MIXER_SCHN_SIDE_LEFT, SND_MIXER_SCHN_SIDE_RIGHT },
};

bool are_there_any_controls(void)
{
	snd_mixer_elem_t *elem;
	unsigned int i;

	for (elem = snd_mixer_first_elem(mixer);
	     elem;
	     elem = snd_mixer_elem_next(elem)) {
		if (snd_mixer_elem_get_type(elem) != SND_MIXER_ELEM_SIMPLE)
			continue;
		if (snd_mixer_selem_is_enumerated(elem))
			return TRUE;
		if (snd_mixer_selem_has_playback_volume_joined(elem) ||
		    snd_mixer_selem_has_capture_volume_joined(elem) ||
		    snd_mixer_selem_has_playback_switch_joined(elem) ||
		    snd_mixer_selem_has_capture_switch_joined(elem))
			return TRUE;
		for (i = 0; i < ARRAY_SIZE(supported_channels); ++i)
			if (snd_mixer_selem_has_playback_channel(elem, supported_channels[i]) ||
			    snd_mixer_selem_has_capture_channel(elem, supported_channels[i]))
				return TRUE;
	}
	return FALSE;
}

static bool has_more_than_front_capture_channels(snd_mixer_elem_t *elem)
{
	unsigned int i;

	for (i = 2; i < ARRAY_SIZE(supported_channels); ++i)
		if (snd_mixer_selem_has_capture_channel(elem, supported_channels[i]))
			return TRUE;
	return FALSE;
}

static bool has_any_control_channel(snd_mixer_elem_t *elem,
				    const snd_mixer_selem_channel_id_t channels[2],
				    int (*has_channel)(snd_mixer_elem_t *, snd_mixer_selem_channel_id_t))
{
	return has_channel(elem, channels[0]) ||
	       (channels[1] != SND_MIXER_SCHN_UNKNOWN && has_channel(elem, channels[1]));
}

static bool has_merged_cswitch(snd_mixer_elem_t *elem)
{
	bool pvol, psw;
	unsigned int i;

	pvol = snd_mixer_selem_has_playback_volume(elem);
	psw = snd_mixer_selem_has_playback_switch(elem);
	if ((pvol || psw) &&
	    snd_mixer_selem_has_capture_switch(elem) &&
	    !snd_mixer_selem_has_capture_volume(elem)) {
		if (snd_mixer_selem_has_capture_switch_joined(elem))
			return TRUE;
		else if (((pvol && snd_mixer_selem_has_playback_volume_joined(elem)) ||
			  (psw && snd_mixer_selem_has_playback_switch_joined(elem))) &&
			 has_more_than_front_capture_channels(elem))
			return FALSE;
		for (i = 0; i < ARRAY_SIZE(control_channels); ++i) {
			if (has_any_control_channel(elem, control_channels[i], snd_mixer_selem_has_capture_channel) &&
			    !has_any_control_channel(elem, control_channels[i], snd_mixer_selem_has_playback_channel))
				return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

static unsigned int get_playback_controls_count(snd_mixer_elem_t *elem)
{
	unsigned int count = 0;
	unsigned int i;
	int has_vol, has_sw;

	has_vol = snd_mixer_selem_has_playback_volume(elem);
	has_sw = snd_mixer_selem_has_playback_switch(elem);
	if (!has_vol && !has_sw)
		return 0;
	if ((!has_vol || snd_mixer_selem_has_playback_volume_joined(elem)) &&
	    (!has_sw || snd_mixer_selem_has_playback_switch_joined(elem)))
		return 1;
	for (i = 0; i < ARRAY_SIZE(control_channels); ++i) {
		if (snd_mixer_selem_has_playback_channel(elem, control_channels[i][0]) ||
		    (control_channels[i][1] != SND_MIXER_SCHN_UNKNOWN &&
		     snd_mixer_selem_has_playback_channel(elem, control_channels[i][1])))
			++count;
	}
	return count;
}

static unsigned int get_capture_controls_count(snd_mixer_elem_t *elem)
{
	unsigned int count = 0;
	unsigned int i;
	int has_vol, has_sw;

	has_vol = snd_mixer_selem_has_capture_volume(elem);
	has_sw = snd_mixer_selem_has_capture_switch(elem);
	if ((!has_vol && !has_sw) ||
	    (view_mode == VIEW_MODE_ALL && has_merged_cswitch(elem)))
		return 0;
	if ((!has_vol || snd_mixer_selem_has_capture_volume_joined(elem)) &&
	    (!has_sw || snd_mixer_selem_has_capture_switch_joined(elem)))
		return 1;
	for (i = 0; i < ARRAY_SIZE(control_channels); ++i) {
		if (snd_mixer_selem_has_capture_channel(elem, control_channels[i][0]) ||
		    (control_channels[i][1] != SND_MIXER_SCHN_UNKNOWN &&
		     snd_mixer_selem_has_capture_channel(elem, control_channels[i][1])))
			++count;
	}
	return count;
}

static unsigned int get_controls_count_for_elem(snd_mixer_elem_t *elem)
{
	unsigned int p, c;

	if (snd_mixer_elem_get_type(elem) != SND_MIXER_ELEM_SIMPLE)
		return 0;
	if (snd_mixer_selem_is_enumerated(elem)) {
		switch (view_mode) {
		case VIEW_MODE_PLAYBACK:
			return snd_mixer_selem_is_enum_capture(elem) ? 0 : 1;
		case VIEW_MODE_CAPTURE:
			return snd_mixer_selem_is_enum_capture(elem) ? 1 : 0;
		case VIEW_MODE_ALL:
		default:
			return 1;
		}
	}
	switch (view_mode) {
	case VIEW_MODE_PLAYBACK:
		return get_playback_controls_count(elem);
	case VIEW_MODE_CAPTURE:
		return get_capture_controls_count(elem);
	case VIEW_MODE_ALL:
	default:
		p = get_playback_controls_count(elem);
		c = get_capture_controls_count(elem);
		return has_merged_cswitch(elem) ? p : p + c;
	}
}

static void create_name(struct control *control)
{
	unsigned int index;
	char *s;

	index = snd_mixer_selem_get_index(control->elem);
	if (index > 0)
		control->name = casprintf("%s %u", snd_mixer_selem_get_name(control->elem), index);
	else
		control->name = cstrdup(snd_mixer_selem_get_name(control->elem));

	while ((s = strstr(control->name, "IEC958")) != NULL)
		memcpy(s, "S/PDIF", 6);
}

static unsigned int create_controls_for_elem(snd_mixer_elem_t *elem, struct control *control)
{
	unsigned int count = 0;
	unsigned int i;
	unsigned int multich_flag;
	unsigned int enum_index;
	struct control *front_control = NULL;
	bool has_pvol, has_psw;
	bool has_cvol, has_csw;
	bool has_channel[LAST_SUPPORTED_CHANNEL + 1];
	bool merged_cswitch;
	bool has_ch0, has_ch1;

	if (snd_mixer_elem_get_type(elem) != SND_MIXER_ELEM_SIMPLE)
		return 0;
	if (snd_mixer_selem_is_enumerated(elem)) {
		if ((view_mode == VIEW_MODE_PLAYBACK && snd_mixer_selem_is_enum_capture(elem)) ||
		    (view_mode == VIEW_MODE_CAPTURE && !snd_mixer_selem_is_enum_capture(elem)))
			return 0;
		control->elem = elem;
		control->flags = TYPE_ENUM;
		control->enum_channel_bits = 0;
		for (i = 0; i <= SND_MIXER_SCHN_LAST; ++i)
			if (snd_mixer_selem_get_enum_item(control->elem, (snd_mixer_selem_channel_id_t)i, &enum_index) >= 0)
				control->enum_channel_bits |= 1 << i;
		if (snd_mixer_selem_is_active(control->elem))
			control->flags |= IS_ACTIVE;
		create_name(control);
		return 1;
	}
	has_pvol = snd_mixer_selem_has_playback_volume(elem);
	has_psw = snd_mixer_selem_has_playback_switch(elem);
	has_cvol = snd_mixer_selem_has_capture_volume(elem);
	has_csw = snd_mixer_selem_has_capture_switch(elem);
	merged_cswitch = view_mode == VIEW_MODE_ALL && has_merged_cswitch(elem);
	if (view_mode != VIEW_MODE_CAPTURE && (has_pvol || has_psw)) {
		if ((!has_pvol || snd_mixer_selem_has_playback_volume_joined(elem)) &&
		    (!has_psw || snd_mixer_selem_has_playback_switch_joined(elem))) {
			control->elem = elem;
			if (has_pvol) {
				control->flags |= TYPE_PVOLUME | HAS_VOLUME_0;
				control->volume_channels[0] = 0;
			}
			if (has_psw) {
				control->flags |= TYPE_PSWITCH | HAS_PSWITCH_0;
				control->pswitch_channels[0] = 0;
			}
			if (merged_cswitch) {
				control->flags |= TYPE_CSWITCH;
				if (snd_mixer_selem_has_capture_switch_joined(elem)) {
					control->flags |= HAS_CSWITCH_0;
					control->cswitch_channels[0] = 0;
				} else {
					if (snd_mixer_selem_has_capture_channel(elem, control_channels[0][0])) {
						control->flags |= HAS_CSWITCH_0;
						control->cswitch_channels[0] = control_channels[0][0];
					}
					if (control_channels[0][1] != SND_MIXER_SCHN_UNKNOWN &&
					    snd_mixer_selem_has_capture_channel(elem, control_channels[0][1])) {
						control->flags |= HAS_CSWITCH_1;
						control->cswitch_channels[1] = control_channels[0][1];
					}
				}
				if ((control->flags & (HAS_CSWITCH_0 | HAS_CSWITCH_1)) == HAS_CSWITCH_1) {
					control->flags ^= HAS_CSWITCH_0 | HAS_CSWITCH_1;
					control->cswitch_channels[0] = control->cswitch_channels[1];
				}
			}
			if (snd_mixer_selem_is_active(control->elem))
				control->flags |= IS_ACTIVE;
			create_name(control);
			++control;
			++count;
		} else {
			multich_flag = 0;
			for (i = 0; i < ARRAY_SIZE(supported_channels); ++i)
				has_channel[supported_channels[i]] =
					snd_mixer_selem_has_playback_channel(elem, supported_channels[i]);
			for (i = 0; i < ARRAY_SIZE(control_channels); ++i) {
				has_ch0 = has_channel[control_channels[i][0]];
				has_ch1 = control_channels[i][1] != SND_MIXER_SCHN_UNKNOWN &&
					has_channel[control_channels[i][1]];
				if (!has_ch0 && !has_ch1)
					continue;
				control->elem = elem;
				if (has_pvol) {
					control->flags |= TYPE_PVOLUME;
					if (snd_mixer_selem_has_playback_volume_joined(elem)) {
						control->flags |= HAS_VOLUME_0;
						control->volume_channels[0] = 0;
					} else {
						if (has_ch0) {
							control->flags |= HAS_VOLUME_0;
							control->volume_channels[0] = control_channels[i][0];
						}
						if (has_ch1) {
							control->flags |= HAS_VOLUME_1;
							control->volume_channels[1] = control_channels[i][1];
						}
					}
				}
				if (has_psw) {
					control->flags |= TYPE_PSWITCH;
					if (snd_mixer_selem_has_playback_switch_joined(elem)) {
						control->flags |= HAS_PSWITCH_0;
						control->pswitch_channels[0] = 0;
					} else {
						if (has_ch0) {
							control->flags |= HAS_PSWITCH_0;
							control->pswitch_channels[0] = control_channels[i][0];
						}
						if (has_ch1) {
							control->flags |= HAS_PSWITCH_1;
							control->pswitch_channels[1] = control_channels[i][1];
						}
					}
				}
				if (merged_cswitch) {
					control->flags |= TYPE_CSWITCH;
					if (snd_mixer_selem_has_capture_switch_joined(elem)) {
						control->flags |= HAS_CSWITCH_0;
						control->cswitch_channels[0] = 0;
					} else {
						if (snd_mixer_selem_has_capture_channel(elem, control_channels[i][0])) {
							control->flags |= HAS_CSWITCH_0;
							control->cswitch_channels[0] = control_channels[i][0];
						}
						if (control_channels[i][1] != SND_MIXER_SCHN_UNKNOWN &&
						    snd_mixer_selem_has_capture_channel(elem, control_channels[i][1])) {
							control->flags |= HAS_CSWITCH_1;
							control->cswitch_channels[1] = control_channels[i][1];
						}
					}
				}
				if ((control->flags & (HAS_VOLUME_0 | HAS_VOLUME_1)) == HAS_VOLUME_1) {
					control->flags ^= HAS_VOLUME_0 | HAS_VOLUME_1;
					control->volume_channels[0] = control->volume_channels[1];
				}
				if ((control->flags & (HAS_PSWITCH_0 | HAS_PSWITCH_1)) == HAS_PSWITCH_1) {
					control->flags ^= HAS_PSWITCH_0 | HAS_PSWITCH_1;
					control->pswitch_channels[0] = control->pswitch_channels[1];
				}
				if ((control->flags & (HAS_CSWITCH_0 | HAS_CSWITCH_1)) == HAS_CSWITCH_1) {
					control->flags ^= HAS_CSWITCH_0 | HAS_CSWITCH_1;
					control->cswitch_channels[0] = control->cswitch_channels[1];
				}
				if (snd_mixer_selem_is_active(control->elem))
					control->flags |= IS_ACTIVE;
				create_name(control);
				if (i == 0)
					front_control = control;
				else {
					front_control->flags |= IS_MULTICH | 0;
					control->flags |= IS_MULTICH | i;
				}
				++control;
				++count;
			}
		}
	}
	if (view_mode != VIEW_MODE_PLAYBACK && (has_cvol || has_csw) && !merged_cswitch) {
		if ((!has_cvol || snd_mixer_selem_has_capture_volume_joined(elem)) &&
		    (!has_csw || snd_mixer_selem_has_capture_switch_joined(elem))) {
			control->elem = elem;
			if (has_cvol) {
				control->flags |= TYPE_CVOLUME | HAS_VOLUME_0;
				control->volume_channels[0] = 0;
			}
			if (has_csw) {
				control->flags |= TYPE_CSWITCH | HAS_CSWITCH_0;
				control->cswitch_channels[0] = 0;
			}
			if (snd_mixer_selem_is_active(control->elem))
				control->flags |= IS_ACTIVE;
			create_name(control);
			++control;
			++count;
		} else {
			for (i = 0; i < ARRAY_SIZE(supported_channels); ++i)
				has_channel[supported_channels[i]] =
					snd_mixer_selem_has_capture_channel(elem, supported_channels[i]);
			for (i = 0; i < ARRAY_SIZE(control_channels); ++i) {
				has_ch0 = has_channel[control_channels[i][0]];
				has_ch1 = control_channels[i][1] != SND_MIXER_SCHN_UNKNOWN &&
					has_channel[control_channels[i][1]];
				if (!has_ch0 && !has_ch1)
					continue;
				control->elem = elem;
				if (has_cvol) {
					control->flags |= TYPE_CVOLUME;
					if (snd_mixer_selem_has_capture_volume_joined(elem)) {
						control->flags |= HAS_VOLUME_0;
						control->volume_channels[0] = 0;
					} else {
						if (has_ch0) {
							control->flags |= HAS_VOLUME_0;
							control->volume_channels[0] = control_channels[i][0];
						}
						if (has_ch1) {
							control->flags |= HAS_VOLUME_1;
							control->volume_channels[1] = control_channels[i][1];
						}
					}
				}
				if (has_csw) {
					control->flags |= TYPE_CSWITCH;
					if (snd_mixer_selem_has_capture_switch_joined(elem)) {
						control->flags |= HAS_CSWITCH_0;
						control->cswitch_channels[0] = 0;
					} else {
						if (has_ch0) {
							control->flags |= HAS_CSWITCH_0;
							control->cswitch_channels[0] = control_channels[i][0];
						}
						if (has_ch1) {
							control->flags |= HAS_CSWITCH_1;
							control->cswitch_channels[1] = control_channels[i][1];
						}
					}
				}
				if ((control->flags & (HAS_VOLUME_0 | HAS_VOLUME_1)) == HAS_VOLUME_1) {
					control->flags ^= HAS_VOLUME_0 | HAS_VOLUME_1;
					control->volume_channels[0] = control->volume_channels[1];
				}
				if ((control->flags & (HAS_CSWITCH_0 | HAS_CSWITCH_1)) == HAS_CSWITCH_1) {
					control->flags ^= HAS_CSWITCH_0 | HAS_CSWITCH_1;
					control->cswitch_channels[0] = control->cswitch_channels[1];
				}
				if (snd_mixer_selem_is_active(control->elem))
					control->flags |= IS_ACTIVE;
				create_name(control);
				if (i == 0)
					front_control = control;
				else {
					front_control->flags |= IS_MULTICH | 0;
					control->flags |= IS_MULTICH | i;
				}
				++control;
				++count;
			}
		}
	}
	return count;
}

static void search_for_focus_control(void)
{
	snd_mixer_elem_t *elem;
	unsigned int i;

	elem = snd_mixer_find_selem(mixer, current_selem_id);
	if (elem)
		for (i = 0; i < controls_count; ++i)
			if (controls[i].elem == elem) {
				focus_control_index = i;
				for (;;) {
					++i;
					if (i >= controls_count || controls[i].elem != elem)
						return;
					if (controls[i].flags == current_control_flags) {
						focus_control_index = i;
						return;
					}
				}
			}
	focus_control_index = 0;
}

void free_controls(void)
{
	unsigned int i;

	for (i = 0; i < controls_count; ++i)
		free(controls[i].name);
	free(controls);
	controls = NULL;
	controls_count = 0;
}

void create_controls(void)
{
	snd_mixer_elem_t *elem;
	struct control *control;

	free_controls();

	for (elem = snd_mixer_first_elem(mixer);
	     elem;
	     elem = snd_mixer_elem_next(elem))
		controls_count += get_controls_count_for_elem(elem);

	if (controls_count > 0) {
		controls = ccalloc(controls_count, sizeof *controls);
		control = controls;
		for (elem = snd_mixer_first_elem(mixer);
		     elem;
		     elem = snd_mixer_elem_next(elem))
			control += create_controls_for_elem(elem, control);
		assert(control == controls + controls_count);
	}

	compute_controls_layout();
	display_view_mode();

	search_for_focus_control();
	refocus_control();
}
