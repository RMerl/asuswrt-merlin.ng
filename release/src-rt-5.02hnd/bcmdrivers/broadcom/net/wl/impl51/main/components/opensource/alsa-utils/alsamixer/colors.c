/*
 * colors.c - color and attribute definitions
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
#include CURSESINC
#include "colors.h"

int attr_mixer_frame;
int attr_mixer_text;
int attr_mixer_active;
int attr_ctl_frame;
int attr_ctl_mute;
int attr_ctl_nomute;
int attr_ctl_capture;
int attr_ctl_nocapture;
int attr_ctl_label;
int attr_ctl_label_focus;
int attr_ctl_mark_focus;
int attr_ctl_bar_lo;
#ifdef TRICOLOR_VOLUME_BAR
int attr_ctl_bar_mi;
int attr_ctl_bar_hi;
#endif
int attr_ctl_inactive;
int attr_ctl_label_inactive;
int attr_errormsg;
int attr_infomsg;
int attr_textbox;
int attr_textfield;
int attr_menu;
int attr_menu_selected;

void init_colors(int use_color)
{
	if (!!has_colors() == !!use_color) {
		start_color();

		init_pair(1, COLOR_CYAN, COLOR_BLACK);
		init_pair(2, COLOR_YELLOW, COLOR_BLACK);
		init_pair(3, COLOR_WHITE, COLOR_GREEN);
		init_pair(4, COLOR_RED, COLOR_BLACK);
		init_pair(5, COLOR_WHITE, COLOR_BLACK);
		init_pair(6, COLOR_WHITE, COLOR_BLUE);
		init_pair(7, COLOR_RED, COLOR_BLUE);
		init_pair(8, COLOR_GREEN, COLOR_GREEN);
		init_pair(9, COLOR_WHITE, COLOR_RED);
#ifdef TRICOLOR_VOLUME_BAR
		init_pair(10, COLOR_WHITE, COLOR_WHITE);
		init_pair(11, COLOR_RED, COLOR_RED);
#endif

		attr_mixer_frame = COLOR_PAIR(1);
		attr_mixer_text = COLOR_PAIR(1);
		attr_mixer_active = A_BOLD | COLOR_PAIR(2);
		attr_ctl_frame = A_BOLD | COLOR_PAIR(1);
		attr_ctl_mute = COLOR_PAIR(1);
		attr_ctl_nomute = A_BOLD | COLOR_PAIR(3);
		attr_ctl_capture = A_BOLD | COLOR_PAIR(4);
		attr_ctl_nocapture = COLOR_PAIR(5);
		attr_ctl_label = A_BOLD | COLOR_PAIR(6);
		attr_ctl_label_focus = A_BOLD | COLOR_PAIR(7);
		attr_ctl_mark_focus = A_BOLD | COLOR_PAIR(4);
		attr_ctl_bar_lo = A_BOLD | COLOR_PAIR(8);
#ifdef TRICOLOR_VOLUME_BAR
		attr_ctl_bar_mi = A_BOLD | COLOR_PAIR(10);
		attr_ctl_bar_hi = A_BOLD | COLOR_PAIR(11);
#endif
		attr_ctl_inactive = COLOR_PAIR(5);
		attr_ctl_label_inactive = A_REVERSE | COLOR_PAIR(5);
		attr_errormsg = A_BOLD | COLOR_PAIR(9);
		attr_infomsg = A_BOLD | COLOR_PAIR(6);
		attr_textbox = A_BOLD | COLOR_PAIR(6);
		attr_textfield = A_REVERSE | COLOR_PAIR(5);
		attr_menu = A_BOLD | COLOR_PAIR(6);
		attr_menu_selected = A_REVERSE | COLOR_PAIR(6);
	} else {
		attr_mixer_frame = A_NORMAL;
		attr_mixer_text = A_NORMAL;
		attr_mixer_active = A_BOLD;
		attr_ctl_frame = A_BOLD;
		attr_ctl_mute = A_NORMAL;
		attr_ctl_nomute = A_BOLD;
		attr_ctl_capture = A_BOLD;
		attr_ctl_nocapture = A_NORMAL;
		attr_ctl_label = A_REVERSE;
		attr_ctl_label_focus = A_REVERSE | A_BOLD;
		attr_ctl_mark_focus = A_BOLD;
		attr_ctl_bar_lo = A_BOLD;
#ifdef TRICOLOR_VOLUME_BAR
		attr_ctl_bar_mi = A_BOLD;
		attr_ctl_bar_hi = A_BOLD;
#endif
		attr_ctl_inactive = A_NORMAL;
		attr_ctl_label_inactive = A_REVERSE;
		attr_errormsg = A_STANDOUT;
		attr_infomsg = A_NORMAL;
		attr_textbox = A_NORMAL;
		attr_textfield = A_REVERSE;
		attr_menu = A_NORMAL;
		attr_menu_selected = A_REVERSE;
	}
}
