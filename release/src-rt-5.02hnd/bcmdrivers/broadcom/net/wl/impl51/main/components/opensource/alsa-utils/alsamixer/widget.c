/*
 * widget.c - handles widget objects and the widget stack
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
#include <term.h>
#include "die.h"
#include "widget.h"

int screen_lines;
int screen_cols;

static int cursor_visibility = -1;

static void widget_handle_key(int key)
{
}

static void update_cursor_visibility(void)
{
	struct widget *active_widget;

	active_widget = get_active_widget();
	if (active_widget &&
	    active_widget->cursor_visibility != cursor_visibility) {
		cursor_visibility = active_widget->cursor_visibility;
		curs_set(cursor_visibility);
	}
}

void widget_init(struct widget *widget, int lines_, int cols, int y, int x,
		 chtype bkgd, unsigned int flags)
{
	WINDOW *old_window;

	if (y == SCREEN_CENTER)
		y = (screen_lines - lines_) / 2;
	if (x == SCREEN_CENTER)
		x = (screen_cols - cols) / 2;

	old_window = widget->window;
	widget->window = newwin(lines_, cols, y, x);
	if (!widget->window)
		fatal_error("cannot create window");
	keypad(widget->window, TRUE);
	nodelay(widget->window, TRUE);
	leaveok(widget->window, !(flags & WIDGET_CURSOR_VISIBLE));
	wbkgdset(widget->window, bkgd);
	werase(widget->window);

	if (flags & WIDGET_BORDER)
		box(widget->window, 0, 0);
	if (flags & WIDGET_SUBWINDOW) {
		if (widget->subwindow)
			delwin(widget->subwindow);
		widget->subwindow = derwin(widget->window,
					   lines_ - 2, cols - 2, 1, 1);
		if (!widget->subwindow)
			fatal_error("cannot create subwindow");
		wbkgdset(widget->subwindow, bkgd);
	}
	widget->cursor_visibility = !!(flags & WIDGET_CURSOR_VISIBLE);

	if (widget->panel) {
		replace_panel(widget->panel, widget->window);
	} else {
		widget->panel = new_panel(widget->window);
		if (!widget->panel)
			fatal_error("cannot create panel");
		set_panel_userptr(widget->panel, widget);
	}

	if (!widget->handle_key)
		widget->handle_key = widget_handle_key;

	if (old_window)
		delwin(old_window);

	update_cursor_visibility();
}

void widget_free(struct widget *widget)
{
	if (widget->panel) {
		del_panel(widget->panel);
		widget->panel = NULL;
	}
	if (widget->subwindow) {
		delwin(widget->subwindow);
		widget->subwindow = NULL;
	}
	if (widget->window) {
		delwin(widget->window);
		widget->window = NULL;
	}

	update_cursor_visibility();
}

struct widget *get_active_widget(void)
{
	PANEL *active_panel;

	active_panel = panel_below(NULL);
	if (active_panel)
		return panel_userptr(active_panel);
	else
		return NULL;
}

void window_size_changed(void)
{
	PANEL *panel, *below;
	struct widget *widget;

	getmaxyx(stdscr, screen_lines, screen_cols);
	if (tigetflag("xenl") != 1 && tigetflag("am") != 1)
		--screen_lines;

	for (panel = panel_below(NULL); panel; panel = below) {
		below = panel_below(panel);
		widget = panel_userptr(panel);
		widget->window_size_changed();
	}
}
