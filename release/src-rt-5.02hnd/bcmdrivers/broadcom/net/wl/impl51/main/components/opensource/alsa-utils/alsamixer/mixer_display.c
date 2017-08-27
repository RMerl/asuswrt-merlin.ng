/*
 * mixer_display.c - handles displaying of mixer widget and controls
 * Copyright (c) 1874 Lewis Carroll
 * Copyright (c) 2009 Clemens Ladisch <clemens@ladisch.de>
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
#include <strings.h>
#include CURSESINC
#include <alsa/asoundlib.h>
#include "gettext_curses.h"
#include "utils.h"
#include "mem.h"
#include "colors.h"
#include "widget.h"
#include "mixer_widget.h"
#include "mixer_controls.h"
#include "mixer_display.h"

enum align {
	ALIGN_LEFT,
	ALIGN_RIGHT,
	ALIGN_CENTER,
};

static bool screen_too_small;
static bool has_info_items;

static int info_items_left;
static int info_items_width;

static int visible_controls;
static int first_visible_control_index;
static int first_control_x;
static int control_width;
static int control_name_width;

static int base_y;
static int volume_height;
static int cswitch_y;
static int values_y;
static int name_y;
static int channel_name_y;

static void display_string_in_field(int y, int x, const char *s, int width, enum align align)
{
	int string_width;
	const char *s_end;
	int spaces;
	int cur_y, cur_x;

	wmove(mixer_widget.window, y, x);
	string_width = width;
	s_end = mbs_at_width(s, &string_width, -1);
	if (string_width >= width) {
		waddnstr(mixer_widget.window, s, s_end - s);
	} else {
		if (align != ALIGN_LEFT) {
			spaces = width - string_width;
			if (align == ALIGN_CENTER)
				spaces /= 2;
			if (spaces > 0)
				wprintw(mixer_widget.window, "%*s", spaces, "");
		}
		waddstr(mixer_widget.window, s);
		if (align != ALIGN_RIGHT) {
			getyx(mixer_widget.window, cur_y, cur_x);
			if (cur_y == y) {
				spaces = x + width - cur_x;
				if (spaces > 0)
					wprintw(mixer_widget.window, "%*s", spaces, "");
			}
		}
	}
}

void init_mixer_layout(void)
{
	const char *labels_left[4] = {
		_("Card:"),
		_("Chip:"),
		_("View:"),
		_("Item:"),
	};
	const char *labels_right[4] = {
		_("F1:  Help"),
		_("F2:  System information"),
		_("F6:  Select sound card"),
		_("Esc: Exit"),
	};
	unsigned int label_width_left, label_width_right;
	unsigned int right_x, i;

	screen_too_small = screen_lines < 14 || screen_cols < 12;
	has_info_items = screen_lines >= 6;
	if (!has_info_items)
		return;

	label_width_left = get_max_mbs_width(labels_left, 4);
	label_width_right = get_max_mbs_width(labels_right, 4);
	if (2 + label_width_left + 1 + 28 + label_width_right + 2 > screen_cols)
		label_width_right = 0;
	if (2 + label_width_left + 1 + 28 + label_width_right + 2 > screen_cols)
		label_width_left = 0;

	info_items_left = label_width_left ? 3 + label_width_left : 2;
	right_x = screen_cols - label_width_right - 2;
	info_items_width = right_x - info_items_left;
	if (info_items_width < 1) {
		has_info_items = FALSE;
		return;
	}

	wattrset(mixer_widget.window, attr_mixer_text);
	if (label_width_left)
		for (i = 0; i < 4; ++i)
			display_string_in_field(1 + i, 2, labels_left[i],
						label_width_left, ALIGN_RIGHT);
	if (label_width_right)
		for (i = 0; i < 4; ++i)
			display_string_in_field(1 + i, right_x, labels_right[i],
						label_width_right, ALIGN_LEFT);
}

void display_card_info(void)
{
	snd_hctl_t *hctl;
	snd_ctl_t *ctl;
	snd_ctl_card_info_t *card_info;
	const char *card_name = NULL;
	const char *mixer_name = NULL;
	int err;

	if (!has_info_items)
		return;

	snd_ctl_card_info_alloca(&card_info);
	if (mixer_device_name)
		err = snd_mixer_get_hctl(mixer, mixer_device_name, &hctl);
	else
		err = -1;
	if (err >= 0) {
		ctl = snd_hctl_ctl(hctl);
		err = snd_ctl_card_info(ctl, card_info);
		if (err >= 0) {
			card_name = snd_ctl_card_info_get_name(card_info);
			mixer_name = snd_ctl_card_info_get_mixername(card_info);
		}
	}

	if (card_name)
		wattrset(mixer_widget.window, attr_mixer_active);
	else {
		wattrset(mixer_widget.window, attr_mixer_text);
		if (unplugged)
			card_name = _("(unplugged)");
		else
			card_name = "-";
	}
	display_string_in_field(1, info_items_left, card_name, info_items_width, ALIGN_LEFT);

	if (mixer_name)
		wattrset(mixer_widget.window, attr_mixer_active);
	else {
		wattrset(mixer_widget.window, attr_mixer_text);
		mixer_name = "-";
	}
	display_string_in_field(2, info_items_left, mixer_name, info_items_width, ALIGN_LEFT);
}

void display_view_mode(void)
{
	const char *modes[3] = {
		_("Playback"),
		_("Capture"),
		_("All"),
	};
	unsigned int widths[3];
	bool has_view_mode;
	int i;

	if (!has_info_items)
		return;

	has_view_mode = controls_count > 0 || are_there_any_controls();
	for (i = 0; i < 3; ++i)
		widths[i] = get_mbs_width(modes[i]);
	if (4 + widths[0] + 6 + widths[1] + 6 + widths[2] + 1 <= info_items_width) {
		wmove(mixer_widget.window, 3, info_items_left);
		wattrset(mixer_widget.window, attr_mixer_text);
		for (i = 0; i < 3; ++i) {
			wprintw(mixer_widget.window, "F%c:", '3' + i);
			if (has_view_mode && (int)view_mode == i) {
				wattrset(mixer_widget.window, attr_mixer_active);
				wprintw(mixer_widget.window, "[%s]", modes[i]);
				wattrset(mixer_widget.window, attr_mixer_text);
			} else {
				wprintw(mixer_widget.window, " %s ", modes[i]);
			}
			if (i < 2)
				waddch(mixer_widget.window, ' ');
		}
	} else {
		wattrset(mixer_widget.window, attr_mixer_active);
		display_string_in_field(3, info_items_left,
					has_view_mode ? modes[view_mode] : "",
					info_items_width, ALIGN_LEFT);
	}
}

static char *format_gain(long db)
{
	if (db != SND_CTL_TLV_DB_GAIN_MUTE)
		return casprintf("%.2f", db / 100.0);
	else
		return cstrdup(_("mute"));
}

static void display_focus_item_info(void)
{
	struct control *control;
	unsigned int index;
	char buf[64];
	long db, db2;
	int sw, sw2;
	char *dbs, *dbs2;
	char *value_info;
	char *item_info;
	int err;

	if (!has_info_items)
		return;
	wattrset(mixer_widget.window, attr_mixer_active);
	if (!controls_count || screen_too_small) {
		display_string_in_field(4, info_items_left, "", info_items_width, ALIGN_LEFT);
		return;
	}
	control = &controls[focus_control_index];
	value_info = NULL;
	if (control->flags & TYPE_ENUM) {
		err = snd_mixer_selem_get_enum_item(control->elem, ffs(control->enum_channel_bits) - 1, &index);
		if (err >= 0)
			err = snd_mixer_selem_get_enum_item_name(control->elem, index, sizeof buf - 1, buf);
		if (err >= 0)
			value_info = casprintf(" [%s]", buf);
	} else if (control->flags & (TYPE_PVOLUME | TYPE_CVOLUME)) {
		int (*get_vol_func)(snd_mixer_elem_t *, snd_mixer_selem_channel_id_t, long *);

		if (control->flags & TYPE_PVOLUME)
			get_vol_func = snd_mixer_selem_get_playback_dB;
		else
			get_vol_func = snd_mixer_selem_get_capture_dB;
		if (!(control->flags & HAS_VOLUME_1)) {
			err = get_vol_func(control->elem, control->volume_channels[0], &db);
			if (err >= 0) {
				dbs = format_gain(db);
				value_info = casprintf(" [%s %s]", _("dB gain:"), dbs);
				free(dbs);
			}
		} else {
			err = get_vol_func(control->elem, control->volume_channels[0], &db);
			if (err >= 0)
				err = get_vol_func(control->elem, control->volume_channels[1], &db2);
			if (err >= 0) {
				dbs = format_gain(db);
				dbs2 = format_gain(db2);
				value_info = casprintf(_(" [%s %s, %s]"), _("dB gain:"), dbs, dbs2);
				free(dbs);
				free(dbs2);
			}
		}
	} else if (control->flags & TYPE_PSWITCH) {
		if (!(control->flags & HAS_PSWITCH_1)) {
			err = snd_mixer_selem_get_playback_switch(control->elem, control->pswitch_channels[0], &sw);
			if (err >= 0 && !sw)
				value_info = casprintf(" [%s]", _("Off"));
		} else {
			err = snd_mixer_selem_get_playback_switch(control->elem, control->pswitch_channels[0], &sw);
			if (err >= 0)
				err = snd_mixer_selem_get_playback_switch(control->elem, control->pswitch_channels[1], &sw2);
			if (err >= 0 && (!sw || !sw2))
				value_info = casprintf(" [%s, %s]", sw ? _("On") : _("Off"), sw2 ? _("On") : _("Off"));
		}
	} else if (control->flags & TYPE_CSWITCH) {
		if (!(control->flags & HAS_CSWITCH_1)) {
			err = snd_mixer_selem_get_capture_switch(control->elem, control->cswitch_channels[0], &sw);
			if (err >= 0 && !sw)
				value_info = casprintf(" [%s]", _("Off"));
		} else {
			err = snd_mixer_selem_get_capture_switch(control->elem, control->cswitch_channels[0], &sw);
			if (err >= 0)
				err = snd_mixer_selem_get_capture_switch(control->elem, control->cswitch_channels[1], &sw2);
			if (err >= 0 && (!sw || !sw2))
				value_info = casprintf(" [%s, %s]", sw ? _("On") : _("Off"), sw2 ? _("On") : _("Off"));
		}
	}
	item_info = casprintf("%s%s", control->name, value_info ? value_info : "");
	free(value_info);
	display_string_in_field(4, info_items_left, item_info, info_items_width, ALIGN_LEFT);
	free(item_info);
}

static void clear_controls_display(void)
{
	int i;

	wattrset(mixer_widget.window, attr_mixer_frame);
	for (i = 5; i < screen_lines - 1; ++i)
		mvwprintw(mixer_widget.window, i, 1, "%*s", screen_cols - 2, "");
}

static void center_string(int line, const char *s)
{
	int width = get_mbs_width(s);
	if (width <= screen_cols - 2)
		mvwaddstr(mixer_widget.window, line, (screen_cols - width) / 2, s);
}

static void display_unplugged(void)
{
	int lines, top, left;
	bool boojum;

	lines = screen_lines - 6;
	if (lines < 2)
		return;
	top = lines / 2;
	boojum = lines >= 10 && screen_cols >= 48;
	top -= boojum ? 5 : 1;
	if (top < 5)
		top = 5;
	if (boojum) {
		left = (screen_cols - 46) / 2;
		wattrset(mixer_widget.window, attr_mixer_text);
		mvwaddstr(mixer_widget.window, top + 0, left,    "In the midst of the word he was trying to say,");
		mvwaddstr(mixer_widget.window, top + 1, left + 2,  "In the midst of his laughter and glee,");
		mvwaddstr(mixer_widget.window, top + 2, left,    "He had softly and suddenly vanished away---");
		mvwaddstr(mixer_widget.window, top + 3, left + 2,  "For the Snark was a Boojum, you see.");
		mvwchgat(mixer_widget.window,  top + 3, left + 16, 3,          /* ^^^ */
			 attr_mixer_text | A_BOLD, PAIR_NUMBER(attr_mixer_text), NULL);
		mvwaddstr(mixer_widget.window, top + 5, left,    "(Lewis Carroll, \"The Hunting of the Snark\")");
		top += 8;
	}
	wattrset(mixer_widget.window, attr_errormsg);
	center_string(top, _("The sound device was unplugged."));
	center_string(top + 1, _("Press F6 to select another sound card."));
}

static void display_no_controls(void)
{
	int y;
	const char *msg;

	y = (screen_lines - 6) / 2 - 1;
	if (y < 5)
		y = 5;
	if (y >= screen_lines - 1)
		return;
	wattrset(mixer_widget.window, attr_infomsg);
	if (view_mode == VIEW_MODE_PLAYBACK && are_there_any_controls())
		msg = _("This sound device does not have any playback controls.");
	else if (view_mode == VIEW_MODE_CAPTURE && are_there_any_controls())
		msg = _("This sound device does not have any capture controls.");
	else
		msg = _("This sound device does not have any controls.");
	center_string(y, msg);
}

static void display_string_centered_in_control(int y, int col, const char *s, int width)
{
	int left, x;

	left = first_control_x + col * (control_width + 1);
	x = left + (control_width - width) / 2;
	display_string_in_field(y, x, s, width, ALIGN_CENTER);
}

static long clamp(long value, long min, long max)
{
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}

static void display_control(unsigned int control_index)
{
	struct control *control;
	int col;
	int i, c;
	int left, frame_left;
	int bar_height, value;
	long volumes[2];
	long min, max;
	int switches[2];
	unsigned int index;
	const char *s;
	char buf[64];
	int err;

	control = &controls[control_index];
	col = control_index - first_visible_control_index;
	left = first_control_x + col * (control_width + 1);
	frame_left = left + (control_width - 4) / 2;
	if (control->flags & IS_ACTIVE)
		wattrset(mixer_widget.window, attr_ctl_frame);
	else
		wattrset(mixer_widget.window, attr_ctl_inactive);
	if (control->flags & (TYPE_PVOLUME | TYPE_CVOLUME)) {
		mvwaddch(mixer_widget.window, base_y - volume_height - 1, frame_left, ACS_ULCORNER);
		waddch(mixer_widget.window, ACS_HLINE);
		waddch(mixer_widget.window, ACS_HLINE);
		waddch(mixer_widget.window, ACS_URCORNER);
		for (i = 0; i < volume_height; ++i) {
			mvwaddch(mixer_widget.window, base_y - i - 1, frame_left, ACS_VLINE);
			mvwaddch(mixer_widget.window, base_y - i - 1, frame_left + 3, ACS_VLINE);
		}
		mvwaddch(mixer_widget.window, base_y, frame_left,
			 control->flags & TYPE_PSWITCH ? ACS_LTEE : ACS_LLCORNER);
		waddch(mixer_widget.window, ACS_HLINE);
		waddch(mixer_widget.window, ACS_HLINE);
		waddch(mixer_widget.window,
		       control->flags & TYPE_PSWITCH ? ACS_RTEE : ACS_LRCORNER);
	} else if (control->flags & TYPE_PSWITCH) {
		mvwaddch(mixer_widget.window, base_y, frame_left, ACS_ULCORNER);
		waddch(mixer_widget.window, ACS_HLINE);
		waddch(mixer_widget.window, ACS_HLINE);
		waddch(mixer_widget.window, ACS_URCORNER);
	}
	if (control->flags & TYPE_PSWITCH) {
		mvwaddch(mixer_widget.window, base_y + 1, frame_left, ACS_VLINE);
		mvwaddch(mixer_widget.window, base_y + 1, frame_left + 3, ACS_VLINE);
		mvwaddch(mixer_widget.window, base_y + 2, frame_left, ACS_LLCORNER);
		waddch(mixer_widget.window, ACS_HLINE);
		waddch(mixer_widget.window, ACS_HLINE);
		waddch(mixer_widget.window, ACS_LRCORNER);
	}
	if (control->flags & (TYPE_PVOLUME | TYPE_CVOLUME)) {
		int (*get_vol_func)(snd_mixer_elem_t *, snd_mixer_selem_channel_id_t, long *);

		if (control->flags & TYPE_PVOLUME)
			get_vol_func = snd_mixer_selem_get_playback_volume;
		else
			get_vol_func = snd_mixer_selem_get_capture_volume;
		err = get_vol_func(control->elem, control->volume_channels[0], &volumes[0]);
		if (err >= 0 && (control->flags & HAS_VOLUME_1))
			err = get_vol_func(control->elem, control->volume_channels[1], &volumes[1]);
		else
			volumes[1] = volumes[0];
		if (err < 0)
			return;
		if (control->flags & TYPE_PVOLUME)
			err = snd_mixer_selem_get_playback_volume_range(control->elem, &min, &max);
		else
			err = snd_mixer_selem_get_capture_volume_range(control->elem, &min, &max);
		if (err < 0)
			return;
		if (min >= max)
			max = min + 1;
		volumes[0] = clamp(volumes[0], min, max);
		volumes[1] = clamp(volumes[1], min, max);

		if (control->flags & IS_ACTIVE)
			wattrset(mixer_widget.window, 0);
		for (c = 0; c < 2; c++) {
			bar_height = ((volumes[c] - min) * volume_height +
				      max - min - 1) / (max - min);
			for (i = 0; i < volume_height; ++i) {
				chtype ch;
				if (i + 1 > bar_height)
					ch = ' ' | (control->flags & IS_ACTIVE ?
						    attr_ctl_frame : 0);
				else {
					ch = ACS_CKBOARD;
					if (!(control->flags & IS_ACTIVE))
						;
#ifdef TRICOLOR_VOLUME_BAR
					else if (i > volume_height * 8 / 10)
						ch |= attr_ctl_bar_hi;
					else if (i > volume_height * 4 / 10)
						ch |= attr_ctl_bar_mi;
#endif
					else
						ch |= attr_ctl_bar_lo;
				}
				mvwaddch(mixer_widget.window, base_y - i - 1,
					 frame_left + c + 1, ch);
			}
		}
		if (control->flags & IS_ACTIVE)
			wattrset(mixer_widget.window, attr_mixer_active);
		value = ((volumes[0] - min) * 100 + (max - min) / 2) / (max - min);
		if (!(control->flags & HAS_VOLUME_1)) {
			sprintf(buf, "%d", value);
			display_string_in_field(values_y, frame_left - 2, buf, 8, ALIGN_CENTER);
		} else {
			mvwprintw(mixer_widget.window, values_y, frame_left - 2, "%3d", value);
			if (control->flags & IS_ACTIVE)
				wattrset(mixer_widget.window, attr_ctl_frame);
			waddstr(mixer_widget.window, "<>");
			if (control->flags & IS_ACTIVE)
				wattrset(mixer_widget.window, attr_mixer_active);
			value = ((volumes[1] - min) * 100 + (max - min) / 2) / (max - min);
			wprintw(mixer_widget.window, "%-3d", value);
		}
	}

	if (control->flags & TYPE_PSWITCH) {
		err = snd_mixer_selem_get_playback_switch(control->elem, control->pswitch_channels[0], &switches[0]);
		if (err >= 0 && (control->flags & HAS_PSWITCH_1))
			err = snd_mixer_selem_get_playback_switch(control->elem, control->pswitch_channels[1], &switches[1]);
		else
			switches[1] = switches[0];
		if (err < 0)
			return;
		if (control->flags & IS_ACTIVE)
			wattrset(mixer_widget.window, 0);
		mvwaddch(mixer_widget.window, base_y + 1, frame_left + 1,
			 switches[0]
			 /* TRANSLATORS: playback on; one character */
			 ? _("O")[0] | (control->flags & IS_ACTIVE ? attr_ctl_nomute : 0)
			 /* TRANSLATORS: playback muted; one character */
			 : _("M")[0] | (control->flags & IS_ACTIVE ? attr_ctl_mute : 0));
		waddch(mixer_widget.window,
		       switches[1]
		       ? _("O")[0] | (control->flags & IS_ACTIVE ? attr_ctl_nomute : 0)
		       : _("M")[0] | (control->flags & IS_ACTIVE ? attr_ctl_mute : 0));
	}

	if (control->flags & TYPE_CSWITCH) {
		err = snd_mixer_selem_get_capture_switch(control->elem, control->cswitch_channels[0], &switches[0]);
		if (err >= 0 && (control->flags & HAS_CSWITCH_1))
			err = snd_mixer_selem_get_capture_switch(control->elem, control->cswitch_channels[1], &switches[1]);
		else
			switches[1] = switches[0];
		if (err < 0)
			return;
		if (control->flags & IS_ACTIVE)
			wattrset(mixer_widget.window, switches[0] ? attr_ctl_capture : attr_ctl_nocapture);
		/* TRANSLATORS: "left"; no more than two characters */
		display_string_in_field(cswitch_y - 1, frame_left - 2, switches[0] ? _("L") : "", 2, ALIGN_RIGHT);
		if (control->flags & IS_ACTIVE)
			wattrset(mixer_widget.window, switches[1] ? attr_ctl_capture : attr_ctl_nocapture);
		/* TRANSLATORS: "right"; no more than two characters */
		display_string_in_field(cswitch_y - 1, frame_left + 4, switches[1] ? _("R") : "", 2, ALIGN_LEFT);
		/* TRANSLATORS: no more than eight characters */
		s = _("CAPTURE");
		if (switches[0] || switches[1]) {
			if (control->flags & IS_ACTIVE)
				wattrset(mixer_widget.window, attr_ctl_capture);
			display_string_in_field(cswitch_y, frame_left - 2, s, 8, ALIGN_CENTER);
		} else {
			i = get_mbs_width(s);
			if (i > 8)
				i = 8;
			memset(buf, '-', i);
			buf[i] = '\0';
			if (control->flags & IS_ACTIVE)
				wattrset(mixer_widget.window, attr_ctl_nocapture);
			display_string_in_field(cswitch_y, frame_left - 2, buf, 8, ALIGN_CENTER);
		}
	}

	if (control->flags & TYPE_ENUM) {
		err = snd_mixer_selem_get_enum_item(control->elem, ffs(control->enum_channel_bits) - 1, &index);
		if (err < 0)
			return;
		err = snd_mixer_selem_get_enum_item_name(control->elem, index, sizeof buf - 1, buf);
		if (err < 0)
			return;
		if (control->flags & IS_ACTIVE)
			wattrset(mixer_widget.window, attr_mixer_active);
		display_string_centered_in_control(base_y, col, buf, control_width);
	}

	if (control_index == focus_control_index) {
		i = first_control_x + col * (control_width + 1) + (control_width - control_name_width) / 2;
		wattrset(mixer_widget.window, attr_ctl_mark_focus);
		mvwaddch(mixer_widget.window, name_y, i - 1, '<');
		mvwaddch(mixer_widget.window, name_y, i + control_name_width, '>');
		if (control->flags & IS_ACTIVE)
			wattrset(mixer_widget.window, attr_ctl_label_focus);
		else
			wattrset(mixer_widget.window, attr_ctl_label_inactive);
	} else {
		if (control->flags & IS_ACTIVE)
			wattrset(mixer_widget.window, attr_ctl_label);
		else
			wattrset(mixer_widget.window, attr_ctl_label_inactive);
	}
	display_string_centered_in_control(name_y, col, control->name, control_name_width);
	if (channel_name_y > name_y) {
		if (control->flags & IS_MULTICH) {
			switch (control->flags & MULTICH_MASK) {
			case 0:
			default:
				s = _("Front");
				break;
			case 1:
				s = _("Rear");
				break;
			case 2:
				s = _("Center");
				break;
			case 3:
				s = _("Woofer");
				break;
			case 4:
				s = _("Side");
				break;
			}
		} else {
			s = "";
			wattrset(mixer_widget.window, attr_mixer_frame);
		}
		display_string_centered_in_control(channel_name_y, col, s,
						   control_name_width);
	}
}

static void display_scroll_indicators(void)
{
	int y0, y1, y;
	chtype left, right;

	if (screen_too_small)
		return;
	y0 = screen_lines * 3 / 8;
	y1 = screen_lines * 5 / 8;
	left = first_visible_control_index > 0 ? ACS_LARROW : ACS_VLINE;
	right = first_visible_control_index + visible_controls < controls_count
		? ACS_RARROW : ACS_VLINE;
	wattrset(mixer_widget.window, attr_mixer_frame);
	for (y = y0; y <= y1; ++y) {
		mvwaddch(mixer_widget.window, y, 0, left);
		mvwaddch(mixer_widget.window, y, screen_cols - 1, right);
	}
}

void display_controls(void)
{
	unsigned int i;

	if (first_visible_control_index > controls_count - visible_controls)
		first_visible_control_index = controls_count - visible_controls;
	if (first_visible_control_index > focus_control_index)
		first_visible_control_index = focus_control_index;
	else if (first_visible_control_index < focus_control_index - visible_controls + 1 && visible_controls)
		first_visible_control_index = focus_control_index - visible_controls + 1;

	clear_controls_display();

	display_focus_item_info();

	if (controls_count > 0) {
		if (!screen_too_small)
			for (i = 0; i < visible_controls; ++i)
				display_control(first_visible_control_index + i);
	} else if (unplugged) {
		display_unplugged();
	} else if (mixer_device_name) {
		display_no_controls();
	}
	display_scroll_indicators();
	controls_changed = FALSE;
}

void compute_controls_layout(void)
{
	bool any_volume, any_pswitch, any_cswitch, any_multich;
	int max_width, name_len;
	int height, space;
	unsigned int i;

	if (controls_count == 0 || screen_too_small) {
		visible_controls = 0;
		return;
	}

	any_volume = FALSE;
	any_pswitch = FALSE;
	any_cswitch = FALSE;
	any_multich = FALSE;
	for (i = 0; i < controls_count; ++i) {
		if (controls[i].flags & (TYPE_PVOLUME | TYPE_CVOLUME))
			any_volume = 1;
		if (controls[i].flags & TYPE_PSWITCH)
			any_pswitch = 1;
		if (controls[i].flags & TYPE_CSWITCH)
			any_cswitch = 1;
		if (controls[i].flags & IS_MULTICH)
			any_multich = 1;
	}

	max_width = 8;
	for (i = 0; i < controls_count; ++i) {
		name_len = strlen(controls[i].name);
		if (name_len > max_width)
			max_width = name_len;
	}
	max_width = (max_width + 1) & ~1;

	control_width = (screen_cols - 3 - (int)controls_count) / controls_count;
	if (control_width < 8)
		control_width = 8;
	if (control_width > max_width)
		control_width = max_width;
	if (control_width > screen_cols - 4)
		control_width = screen_cols - 4;

	visible_controls = (screen_cols - 3) / (control_width + 1);
	if (visible_controls > controls_count)
		visible_controls = controls_count;

	first_control_x = 2 + (screen_cols - 3 - visible_controls * (control_width + 1)) / 2;

	if (control_width < max_width)
		control_name_width = control_width;
	else
		control_name_width = max_width;

	height = 2;
	if (any_volume)
		height += 2;
	if (any_pswitch)
		height += 2;
	if (any_cswitch)
		height += 1;
	if (any_multich)
		height += 1;
	if (any_volume) {
		space = screen_lines - 6 - height;
		if (space <= 1)
			volume_height = 1;
		else if (space <= 10)
			volume_height = space;
		else
			volume_height = 10 + (space - 10) / 2;
		height += volume_height;
	}

	space = screen_lines - 6 - height;
	channel_name_y = screen_lines - 2 - space / 2;
	name_y = channel_name_y - any_multich;
	values_y = name_y - any_volume;
	cswitch_y = values_y - any_cswitch;
	base_y = cswitch_y - 1 - 2 * any_pswitch;
}
