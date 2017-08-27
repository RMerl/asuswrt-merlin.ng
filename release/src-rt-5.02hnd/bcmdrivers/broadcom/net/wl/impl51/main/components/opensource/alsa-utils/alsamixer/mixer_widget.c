/*
 * mixer_widget.c - mixer widget and keys handling
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
#include <errno.h>
#include <alsa/asoundlib.h>
#include "gettext_curses.h"
#include "version.h"
#include "utils.h"
#include "die.h"
#include "mem.h"
#include "colors.h"
#include "widget.h"
#include "textbox.h"
#include "proc_files.h"
#include "card_select.h"
#include "mixer_controls.h"
#include "mixer_display.h"
#include "mixer_widget.h"

snd_mixer_t *mixer;
char *mixer_device_name;
bool unplugged;

struct widget mixer_widget;

enum view_mode view_mode;

int focus_control_index;
snd_mixer_selem_id_t *current_selem_id;
unsigned int current_control_flags;

bool controls_changed;

enum channel_mask {
	LEFT = 1,
	RIGHT = 2,
};

static int elem_callback(snd_mixer_elem_t *elem, unsigned int mask)
{
	if (mask & (SND_CTL_EVENT_MASK_REMOVE |
		    SND_CTL_EVENT_MASK_INFO |
		    SND_CTL_EVENT_MASK_VALUE))
		controls_changed = TRUE;
	return 0;
}

static int mixer_callback(snd_mixer_t *mixer, unsigned int mask, snd_mixer_elem_t *elem)
{
	if (mask & SND_CTL_EVENT_MASK_ADD) {
		snd_mixer_elem_set_callback(elem, elem_callback);
		controls_changed = TRUE;
	}
	return 0;
}

void create_mixer_object(struct snd_mixer_selem_regopt *selem_regopt)
{
	int err;

	err = snd_mixer_open(&mixer, 0);
	if (err < 0)
		fatal_alsa_error(_("cannot open mixer"), err);

	mixer_device_name = cstrdup(selem_regopt->device);
	err = snd_mixer_selem_register(mixer, selem_regopt, NULL);
	if (err < 0)
		fatal_alsa_error(_("cannot open mixer"), err);

	snd_mixer_set_callback(mixer, mixer_callback);

	err = snd_mixer_load(mixer);
	if (err < 0)
		fatal_alsa_error(_("cannot load mixer controls"), err);

	err = snd_mixer_selem_id_malloc(&current_selem_id);
	if (err < 0)
		fatal_error("out of memory");
}

static void set_view_mode(enum view_mode m)
{
	view_mode = m;
	create_controls();
}

static void close_hctl(void)
{
	free_controls();
	if (mixer_device_name) {
		snd_mixer_detach(mixer, mixer_device_name);
		free(mixer_device_name);
		mixer_device_name = NULL;
	}
}

static void check_unplugged(void)
{
	snd_hctl_t *hctl;
	snd_ctl_t *ctl;
	unsigned int state;
	int err;

	unplugged = FALSE;
	if (mixer_device_name) {
		err = snd_mixer_get_hctl(mixer, mixer_device_name, &hctl);
		if (err >= 0) {
			ctl = snd_hctl_ctl(hctl);
			/* just any random function that does an ioctl() */
			err = snd_ctl_get_power_state(ctl, &state);
			if (err == -ENODEV)
				unplugged = TRUE;
		}
	}
}

void close_mixer_device(void)
{
	check_unplugged();
	close_hctl();

	display_card_info();
	set_view_mode(view_mode);
}

bool select_card_by_name(const char *device_name)
{
	int err;
	bool opened;
	char *msg;

	close_hctl();
	unplugged = FALSE;

	opened = FALSE;
	if (device_name) {
		err = snd_mixer_attach(mixer, device_name);
		if (err >= 0)
			opened = TRUE;
		else {
			msg = casprintf(_("Cannot open mixer device '%s'."), device_name);
			show_alsa_error(msg, err);
			free(msg);
		}
	}
	if (opened) {
		mixer_device_name = cstrdup(device_name);

		err = snd_mixer_load(mixer);
		if (err < 0)
			fatal_alsa_error(_("cannot load mixer controls"), err);
	}

	display_card_info();
	set_view_mode(view_mode);
	return opened;
}

static void show_help(void)
{
	const char *help[] = {
		_("Esc     Exit"),
		_("F1 ? H  Help"),
		_("F2 /    System information"),
		_("F3      Show playback controls"),
		_("F4      Show capture controls"),
		_("F5      Show all controls"),
		_("Tab     Toggle view mode (F3/F4/F5)"),
		_("F6 S    Select sound card"),
		_("L       Redraw screen"),
		"",
		_("Left    Move to the previous control"),
		_("Right   Move to the next control"),
		"",
		_("Up/Down    Change volume"),
		_("+ -        Change volume"),
		_("Page Up/Dn Change volume in big steps"),
		_("End        Set volume to 0%"),
		_("0-9        Set volume to 0%-90%"),
		_("Q W E      Increase left/both/right volumes"),
		/* TRANSLATORS: or Y instead of Z */
		_("Z X C      Decrease left/both/right volumes"),
		_("B          Balance left and right volumes"),
		"",
		_("M          Toggle mute"),
		/* TRANSLATORS: or , . */
		_("< >        Toggle left/right mute"),
		"",
		_("Space      Toggle capture"),
		/* TRANSLATORS: or Insert Delete */
		_("; '        Toggle left/right capture"),
		"",
		_("Authors:"),
		_("  Tim Janik <timj@gtk.org>"),
		_("  Jaroslav Kysela <perex@perex.cz>"),
		_("  Clemens Ladisch <clemens@ladisch.de>"),
	};
	show_text(help, ARRAY_SIZE(help), _("Help"));
}

void refocus_control(void)
{
	if (focus_control_index < controls_count) {
		snd_mixer_selem_get_id(controls[focus_control_index].elem, current_selem_id);
		current_control_flags = controls[focus_control_index].flags;
	}

	display_controls();
}

static struct control *get_focus_control(unsigned int type)
{
	if (focus_control_index >= 0 &&
	    focus_control_index < controls_count &&
	    (controls[focus_control_index].flags & IS_ACTIVE) &&
	    (controls[focus_control_index].flags & type))
		return &controls[focus_control_index];
	else
		return NULL;
}

static void change_enum_to_percent(struct control *control, int value)
{
	unsigned int i;
	unsigned int index;
	unsigned int new_index;
	int items;
	int err;

	i = ffs(control->enum_channel_bits) - 1;
	err = snd_mixer_selem_get_enum_item(control->elem, i, &index);
	if (err < 0)
		return;
	new_index = index;
	if (value == 0) {
		new_index = 0;
	} else if (value == 100) {
		items = snd_mixer_selem_get_enum_items(control->elem);
		if (items < 1)
			return;
		new_index = items - 1;
	}
	if (new_index == index)
		return;
	for (i = 0; i <= SND_MIXER_SCHN_LAST; ++i)
		if (control->enum_channel_bits & (1 << i))
			snd_mixer_selem_set_enum_item(control->elem, i, new_index);
}

static void change_enum_relative(struct control *control, int delta)
{
	int items;
	unsigned int i;
	unsigned int index;
	int new_index;
	int err;

	items = snd_mixer_selem_get_enum_items(control->elem);
	if (items < 1)
		return;
	err = snd_mixer_selem_get_enum_item(control->elem, 0, &index);
	if (err < 0)
		return;
	new_index = (int)index + delta;
	if (new_index < 0)
		new_index = 0;
	else if (new_index >= items)
		new_index = items - 1;
	if (new_index == index)
		return;
	for (i = 0; i <= SND_MIXER_SCHN_LAST; ++i)
		if (control->enum_channel_bits & (1 << i))
			snd_mixer_selem_set_enum_item(control->elem, i, new_index);
}

static void change_volume_to_percent(struct control *control, int value, unsigned int channels)
{
	int (*get_range_func)(snd_mixer_elem_t *, long *, long *);
	int (*set_func)(snd_mixer_elem_t *, snd_mixer_selem_channel_id_t, long);
	long min, max;
	int err;

	if (!(control->flags & HAS_VOLUME_1))
		channels = LEFT;
	if (control->flags & TYPE_PVOLUME) {
		get_range_func = snd_mixer_selem_get_playback_volume_range;
		set_func = snd_mixer_selem_set_playback_volume;
	} else {
		get_range_func = snd_mixer_selem_get_capture_volume_range;
		set_func = snd_mixer_selem_set_capture_volume;
	}
	err = get_range_func(control->elem, &min, &max);
	if (err < 0)
		return;
	if (channels & LEFT)
		set_func(control->elem, control->volume_channels[0], min + (max - min) * value / 100);
	if (channels & RIGHT)
		set_func(control->elem, control->volume_channels[1], min + (max - min) * value / 100);
}

static void change_volume_relative(struct control *control, int delta, unsigned int channels)
{
	int (*get_range_func)(snd_mixer_elem_t *, long *, long *);
	int (*get_func)(snd_mixer_elem_t *, snd_mixer_selem_channel_id_t, long *);
	int (*set_func)(snd_mixer_elem_t *, snd_mixer_selem_channel_id_t, long);
	long min, max;
	long left, right;
	long value;
	int err;

	if (!(control->flags & HAS_VOLUME_1))
		channels = LEFT;
	if (control->flags & TYPE_PVOLUME) {
		get_range_func = snd_mixer_selem_get_playback_volume_range;
		get_func = snd_mixer_selem_get_playback_volume;
		set_func = snd_mixer_selem_set_playback_volume;
	} else {
		get_range_func = snd_mixer_selem_get_capture_volume_range;
		get_func = snd_mixer_selem_get_capture_volume;
		set_func = snd_mixer_selem_set_capture_volume;
	}
	err = get_range_func(control->elem, &min, &max);
	if (err < 0)
		return;
	if (channels & LEFT) {
		err = get_func(control->elem, control->volume_channels[0], &left);
		if (err < 0)
			return;
	}
	if (channels & RIGHT) {
		err = get_func(control->elem, control->volume_channels[1], &right);
		if (err < 0)
			return;
	}
	if (channels & LEFT) {
		value = left + delta;
		if (value < min)
			value = min;
		else if (value > max)
			value = max;
		if (value != left)
			set_func(control->elem, control->volume_channels[0], value);
	}
	if (channels & RIGHT) {
		value = right + delta;
		if (value < min)
			value = min;
		else if (value > max)
			value = max;
		if (value != right)
			set_func(control->elem, control->volume_channels[1], value);
	}
}

static void change_control_to_percent(int value, unsigned int channels)
{
	struct control *control;

	control = get_focus_control(TYPE_PVOLUME | TYPE_CVOLUME | TYPE_ENUM);
	if (!control)
		return;
	if (control->flags & TYPE_ENUM)
		change_enum_to_percent(control, value);
	else
		change_volume_to_percent(control, value, channels);
	display_controls();
}

static void change_control_relative(int delta, unsigned int channels)
{
	struct control *control;

	control = get_focus_control(TYPE_PVOLUME | TYPE_CVOLUME | TYPE_ENUM);
	if (!control)
		return;
	if (control->flags & TYPE_ENUM)
		change_enum_relative(control, delta);
	else
		change_volume_relative(control, delta, channels);
	display_controls();
}

static void toggle_switches(unsigned int type, unsigned int channels)
{
	struct control *control;
	unsigned int switch_1_mask;
	int (*get_func)(snd_mixer_elem_t *, snd_mixer_selem_channel_id_t, int *);
	int (*set_func)(snd_mixer_elem_t *, snd_mixer_selem_channel_id_t, int);
	snd_mixer_selem_channel_id_t channel_ids[2];
	int left, right;
	int err;

	control = get_focus_control(type);
	if (!control)
		return;
	if (type == TYPE_PSWITCH) {
		switch_1_mask = HAS_PSWITCH_1;
		get_func = snd_mixer_selem_get_playback_switch;
		set_func = snd_mixer_selem_set_playback_switch;
		channel_ids[0] = control->pswitch_channels[0];
		channel_ids[1] = control->pswitch_channels[1];
	} else {
		switch_1_mask = HAS_CSWITCH_1;
		get_func = snd_mixer_selem_get_capture_switch;
		set_func = snd_mixer_selem_set_capture_switch;
		channel_ids[0] = control->cswitch_channels[0];
		channel_ids[1] = control->cswitch_channels[1];
	}
	if (!(control->flags & switch_1_mask))
		channels = LEFT;
	if (channels & LEFT) {
		err = get_func(control->elem, channel_ids[0], &left);
		if (err < 0)
			return;
	}
	if (channels & RIGHT) {
		err = get_func(control->elem, channel_ids[1], &right);
		if (err < 0)
			return;
	}
	if (channels & LEFT)
		set_func(control->elem, channel_ids[0], !left);
	if (channels & RIGHT)
		set_func(control->elem, channel_ids[1], !right);
	display_controls();
}

static void toggle_mute(unsigned int channels)
{
	toggle_switches(TYPE_PSWITCH, channels);
}

static void toggle_capture(unsigned int channels)
{
	toggle_switches(TYPE_CSWITCH, channels);
}

static void balance_volumes(void)
{
	struct control *control;
	long left, right;
	int err;

	control = get_focus_control(TYPE_PVOLUME | TYPE_CVOLUME);
	if (!control || !(control->flags & HAS_VOLUME_1))
		return;
	if (control->flags & TYPE_PVOLUME) {
		err = snd_mixer_selem_get_playback_volume(control->elem, control->volume_channels[0], &left);
		if (err < 0)
			return;
		err = snd_mixer_selem_get_playback_volume(control->elem, control->volume_channels[1], &right);
		if (err < 0)
			return;
	} else {
		err = snd_mixer_selem_get_capture_volume(control->elem, control->volume_channels[0], &left);
		if (err < 0)
			return;
		err = snd_mixer_selem_get_capture_volume(control->elem, control->volume_channels[1], &right);
		if (err < 0)
			return;
	}
	left = (left + right) / 2;
	if (control->flags & TYPE_PVOLUME) {
		snd_mixer_selem_set_playback_volume(control->elem, control->volume_channels[0], left);
		snd_mixer_selem_set_playback_volume(control->elem, control->volume_channels[1], left);
	} else {
		snd_mixer_selem_set_capture_volume(control->elem, control->volume_channels[0], left);
		snd_mixer_selem_set_capture_volume(control->elem, control->volume_channels[1], left);
	}
	display_controls();
}

static void on_handle_key(int key)
{
	switch (key) {
	case 27:
	case KEY_CANCEL:
	case KEY_F(10):
		mixer_widget.close();
		break;
	case KEY_F(1):
	case KEY_HELP:
	case 'H':
	case 'h':
	case '?':
		show_help();
		break;
	case KEY_F(2):
	case '/':
		create_proc_files_list();
		break;
	case KEY_F(3):
		set_view_mode(VIEW_MODE_PLAYBACK);
		break;
	case KEY_F(4):
		set_view_mode(VIEW_MODE_CAPTURE);
		break;
	case KEY_F(5):
		set_view_mode(VIEW_MODE_ALL);
		break;
	case '\t':
		set_view_mode((enum view_mode)((view_mode + 1) % VIEW_MODE_COUNT));
		break;
	case KEY_F(6):
	case 'S':
	case 's':
		create_card_select_list();
		break;
	case KEY_REFRESH:
	case 12:
	case 'L':
	case 'l':
		clearok(mixer_widget.window, TRUE);
		display_controls();
		break;
	case KEY_LEFT:
	case 'P':
	case 'p':
		if (focus_control_index > 0) {
			--focus_control_index;
			refocus_control();
		}
		break;
	case KEY_RIGHT:
	case 'N':
	case 'n':
		if (focus_control_index < controls_count - 1) {
			++focus_control_index;
			refocus_control();
		}
		break;
	case KEY_PPAGE:
		change_control_relative(5, LEFT | RIGHT);
		break;
	case KEY_NPAGE:
		change_control_relative(-5, LEFT | RIGHT);
		break;
	case KEY_LL:
	case KEY_END:
		change_control_to_percent(0, LEFT | RIGHT);
		break;
	case KEY_UP:
	case '+':
	case 'K':
	case 'k':
	case 'W':
	case 'w':
		change_control_relative(1, LEFT | RIGHT);
		break;
	case KEY_DOWN:
	case '-':
	case 'J':
	case 'j':
	case 'X':
	case 'x':
		change_control_relative(-1, LEFT | RIGHT);
		break;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		change_control_to_percent((key - '0') * 10, LEFT | RIGHT);
		break;
	case 'Q':
	case 'q':
		change_control_relative(1, LEFT);
		break;
	case 'Y':
	case 'y':
	case 'Z':
	case 'z':
		change_control_relative(-1, LEFT);
		break;
	case 'E':
	case 'e':
		change_control_relative(1, RIGHT);
		break;
	case 'C':
	case 'c':
		change_control_relative(-1, RIGHT);
		break;
	case 'M':
	case 'm':
		toggle_mute(LEFT | RIGHT);
		break;
	case 'B':
	case 'b':
	case '=':
		balance_volumes();
		break;
	case '<':
	case ',':
		toggle_mute(LEFT);
		break;
	case '>':
	case '.':
		toggle_mute(RIGHT);
		break;
	case ' ':
		toggle_capture(LEFT | RIGHT);
		break;
	case KEY_IC:
	case ';':
		toggle_capture(LEFT);
		break;
	case KEY_DC:
	case '\'':
		toggle_capture(RIGHT);
		break;
	}
}

static void create(void)
{
	static const char title[] = " AlsaMixer v" SND_UTIL_VERSION_STR " ";

	widget_init(&mixer_widget, screen_lines, screen_cols, 0, 0,
		    attr_mixer_frame, WIDGET_BORDER);
	if (screen_cols >= (sizeof(title) - 1) + 2) {
		wattrset(mixer_widget.window, attr_mixer_active);
		mvwaddstr(mixer_widget.window, 0, (screen_cols - (sizeof(title) - 1)) / 2, title);
	}
	init_mixer_layout();
	display_card_info();
	set_view_mode(view_mode);
}

static void on_window_size_changed(void)
{
	create();
}

static void on_close(void)
{
	widget_free(&mixer_widget);
}

void mixer_shutdown(void)
{
	free_controls();
	if (mixer)
		snd_mixer_close(mixer);
	if (current_selem_id)
		snd_mixer_selem_id_free(current_selem_id);
}

struct widget mixer_widget = {
	.handle_key = on_handle_key,
	.window_size_changed = on_window_size_changed,
	.close = on_close,
};

void create_mixer_widget(void)
{
	create();
}
