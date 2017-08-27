/*
 * proc_files.c - shows ALSA system information files
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
#include <assert.h>
#include <menu.h>
#include <unistd.h>
#include "gettext_curses.h"
#include "utils.h"
#include "die.h"
#include "mem.h"
#include "colors.h"
#include "widget.h"
#include "textbox.h"
#include "proc_files.h"

static struct widget proc_widget;
static ITEM *items[7];
static unsigned int items_count;
static MENU *menu;

static void on_menu_key(int key)
{
	static const struct {
		int key;
		int request;
	} key_map[] = {
		{ KEY_DOWN, REQ_DOWN_ITEM },
		{ KEY_UP, REQ_UP_ITEM },
		{ KEY_HOME, REQ_FIRST_ITEM },
		{ KEY_NPAGE, REQ_SCR_DPAGE },
		{ KEY_PPAGE, REQ_SCR_UPAGE },
		{ KEY_BEG, REQ_FIRST_ITEM },
		{ KEY_END, REQ_LAST_ITEM },
	};
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(key_map); ++i)
		if (key_map[i].key == key) {
			menu_driver(menu, key_map[i].request);
			break;
		}
}

static void on_handle_key(int key)
{
	ITEM *item;

	switch (key) {
	case 27:
	case KEY_CANCEL:
		proc_widget.close();
		break;
	case 10:
	case 13:
	case KEY_ENTER:
		item = current_item(menu);
		if (item)
			show_textfile(item_name(item));
		break;
	default:
		on_menu_key(key);
		break;
	}
}

static bool create(void)
{
	int rows, columns;
	const char *title;

	if (screen_lines < 3 || screen_cols < 20) {
		proc_widget.close();
		beep();
		return FALSE;
	}
	scale_menu(menu, &rows, &columns);
	rows += 2;
	columns += 2;
	if (rows > screen_lines)
		rows = screen_lines;
	if (columns > screen_cols)
		columns = screen_cols;

	widget_init(&proc_widget, rows, columns, SCREEN_CENTER, SCREEN_CENTER,
		    attr_menu, WIDGET_BORDER | WIDGET_SUBWINDOW);

	title = _("Select File");
	mvwprintw(proc_widget.window, 0, (columns - 2 - get_mbs_width(title)) / 2, " %s ", title);
	set_menu_win(menu, proc_widget.window);
	set_menu_sub(menu, proc_widget.subwindow);
	return TRUE;
}

static void on_window_size_changed(void)
{
	unpost_menu(menu);
	if (!create())
		return;
	post_menu(menu);
}

static void on_close(void)
{
	unsigned int i;

	unpost_menu(menu);
	free_menu(menu);
	for (i = 0; i < items_count; ++i)
		free_item(items[i]);
	widget_free(&proc_widget);
}

static void add_item(const char *file_name)
{
	if (access(file_name, F_OK) == 0) {
		items[items_count] = new_item(file_name, NULL);
		if (!items[items_count])
			fatal_error("cannot create menu item");
		++items_count;
		assert(items_count < ARRAY_SIZE(items));
	}
}

static struct widget proc_widget = {
	.handle_key = on_handle_key,
	.window_size_changed = on_window_size_changed,
	.close = on_close,
};

void create_proc_files_list(void)
{
	items_count = 0;
	add_item("/proc/asound/version");
	add_item("/proc/asound/cards");
	add_item("/proc/asound/devices");
	add_item("/proc/asound/oss/devices");
	add_item("/proc/asound/timers");
	add_item("/proc/asound/pcm");
	items[items_count] = NULL;

	menu = new_menu(items);
	if (!menu)
		fatal_error("cannot create menu");
	set_menu_fore(menu, attr_menu_selected);
	set_menu_back(menu, attr_menu);
	set_menu_mark(menu, NULL);
	menu_opts_off(menu, O_SHOWDESC);

	if (!create())
		return;

	post_menu(menu);
}
