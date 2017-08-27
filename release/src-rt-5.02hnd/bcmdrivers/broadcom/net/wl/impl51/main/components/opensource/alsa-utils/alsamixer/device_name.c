/*
 * device_name_form.c - ask for sound control device name
 * Copyright (c) Clemens Ladisch <clemens@ladisch.de>
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
#include CURSESINC
#include <form.h>
#include "gettext_curses.h"
#include "die.h"
#include "mem.h"
#include "utils.h"
#include "colors.h"
#include "widget.h"
#include "mixer_widget.h"
#include "card_select.h"
#include "device_name.h"

static struct widget form_widget;
static FIELD *fields[3];
static FORM *form;

static char *dup_current_name(void)
{
	int rows, cols, max, i;
	char *s;

	if (form_driver(form, REQ_VALIDATION) == E_OK) {
		dynamic_field_info(fields[1], &rows, &cols, &max);
		s = ccalloc(1, cols + 1);
		memcpy(s, field_buffer(fields[1], 0), cols);
		for (i = strlen(s) - 1; i >= 0 && s[i] == ' '; --i)
			s[i] = '\0';
		return s;
	} else {
		return cstrdup("");
	}
}

static void on_key_enter(void)
{
	char *s;
	bool ok;

	s = dup_current_name();
	ok = select_card_by_name(s);
	free(s);
	if (ok) {
		form_widget.close();
		close_card_select_list();
	}
}

static void on_form_key(int key)
{
	static const struct {
		int key;
		int request;
	} key_map[] = {
		{ KEY_LEFT, REQ_PREV_CHAR },
		{ KEY_RIGHT, REQ_NEXT_CHAR },
		{ KEY_HOME, REQ_BEG_FIELD },
		{ KEY_BACKSPACE, REQ_DEL_PREV },
		{ KEY_DC, REQ_DEL_CHAR },
		{ KEY_BEG, REQ_BEG_FIELD },
		{ KEY_END, REQ_END_FIELD },
	};
	unsigned int i;

	if (key >= 32 && key < 256) {
		form_driver(form, key);
		return;
	}
	for (i = 0; i < ARRAY_SIZE(key_map); ++i)
		if (key_map[i].key == key) {
			form_driver(form, key_map[i].request);
			break;
		}
}

static void on_handle_key(int key)
{
	switch (key) {
	case 27:
	case KEY_CANCEL:
		form_widget.close();
		break;
	case 10:
	case 13:
	case KEY_ENTER:
		on_key_enter();
		break;
	default:
		on_form_key(key);
		break;
	}
}

static bool create(void)
{
	const char *title;

	if (screen_lines < 6 || screen_cols < 36) {
		form_widget.close();
		beep();
		return FALSE;
	}
	widget_init(&form_widget,
		    6, 36, SCREEN_CENTER, SCREEN_CENTER,
		    attr_textbox, WIDGET_BORDER | WIDGET_SUBWINDOW | WIDGET_CURSOR_VISIBLE);
	title = _("Sound Card");
	mvwprintw(form_widget.window, 0, (36 - 2 - get_mbs_width(title)) / 2, " %s ", title);

	set_form_win(form, form_widget.window);
	set_form_sub(form, form_widget.subwindow);
	return TRUE;
}

static void on_window_size_changed(void)
{
	form_driver(form, REQ_VALIDATION); /* save field value */
	unpost_form(form);

	if (!create())
		return;

	/*
	 * This call fails because ncurses does not allow changing options of
	 * the current field, and we cannot change the current field because
	 * there is only one.  The only way to make this work would be to throw
	 * away and recreate all fields.
	 */
	field_opts_off(fields[1], O_BLANK);

	post_form(form);
}

static void on_close(void)
{
	unpost_form(form);
	free_form(form);
	free_field(fields[0]);
	free_field(fields[1]);
	widget_free(&form_widget);
}

static struct widget form_widget = {
	.handle_key = on_handle_key,
	.window_size_changed = on_window_size_changed,
	.close = on_close,
};

void create_device_name_form(void)
{
	fields[0] = new_field(1, 32, 1, 1, 0, 0);
	if (!fields[0])
		fatal_error("cannot create field");
	field_opts_off(fields[0], O_ACTIVE);
	field_opts_off(fields[0], O_EDIT);
	set_field_fore(fields[0], attr_textbox);
	set_field_back(fields[0], attr_textbox);
	set_field_buffer(fields[0], 0, _("Device name:"));

	fields[1] = new_field(1, 32, 2, 1, 0, 0);
	if (!fields[1])
		fatal_error("cannot create field");
	field_opts_off(fields[1], O_AUTOSKIP);
	field_opts_off(fields[1], O_NULLOK);
	field_opts_off(fields[1], O_STATIC);
	set_field_fore(fields[1], attr_textfield);
	set_field_back(fields[1], attr_textfield);
	set_field_buffer(fields[1], 0, mixer_device_name);

	form = new_form(fields);
	if (!form)
		fatal_error("cannot create form");

	if (!create())
		return;

	post_form(form);
}
