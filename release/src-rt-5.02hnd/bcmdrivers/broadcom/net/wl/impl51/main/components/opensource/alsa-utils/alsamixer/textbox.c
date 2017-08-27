/*
 * textbox.c - show a text box for messages, files or help
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include CURSESINC
#include <alsa/asoundlib.h>
#include "gettext_curses.h"
#include "utils.h"
#include "die.h"
#include "mem.h"
#include "colors.h"
#include "widget.h"
#include "textbox.h"

#define MAX_FILE_SIZE 1048576

static void create_text_box(const char *const *lines, unsigned int count,
			    const char *title, int attrs);

void show_error(const char *msg, int err)
{
	const char *lines[2];
	unsigned int count;

	lines[0] = msg;
	count = 1;
	if (err) {
		lines[1] = strerror(err);
		count = 2;
	}
	create_text_box(lines, count, _("Error"), attr_errormsg);
}

void show_alsa_error(const char *msg, int err)
{
	const char *lines[2];
	unsigned int count;

	lines[0] = msg;
	count = 1;
	if (err < 0) {
		lines[1] = snd_strerror(err);
		count = 2;
	}
	create_text_box(lines, count, _("Error"), attr_errormsg);
}

static char *read_file(const char *file_name, unsigned int *file_size)
{
	FILE *f;
	int err;
	char *buf;
	unsigned int allocated = 2048;
	unsigned int bytes_read;

	f = fopen(file_name, "r");
	if (!f) {
		err = errno;
		buf = casprintf(_("Cannot open file \"%s\"."), file_name);
		show_error(buf, err);
		free(buf);
		return NULL;
	}
	*file_size = 0;
	buf = NULL;
	do {
		allocated *= 2;
		buf = crealloc(buf, allocated);
		bytes_read = fread(buf + *file_size, 1, allocated - *file_size, f);
		*file_size += bytes_read;
	} while (*file_size == allocated && allocated < MAX_FILE_SIZE);
	fclose(f);
	if (*file_size > 0 && buf[*file_size - 1] != '\n' && *file_size < allocated) {
		buf[*file_size] = '\n';
		++*file_size;
	}
	return buf;
}

void show_textfile(const char *file_name)
{
	char *buf;
	unsigned int file_size;
	unsigned int line_count;
	unsigned int i;
	const char **lines;
	const char *start_line;

	buf = read_file(file_name, &file_size);
	if (!buf)
		return;
	line_count = 0;
	for (i = 0; i < file_size; ++i)
		line_count += buf[i] == '\n';
	lines = ccalloc(line_count, sizeof *lines);
	line_count = 0;
	start_line = buf;
	for (i = 0; i < file_size; ++i) {
		if (buf[i] == '\n') {
			lines[line_count++] = start_line;
			buf[i] = '\0';
			start_line = &buf[i + 1];
		}
		if (buf[i] == '\t')
			buf[i] = ' ';
	}
	create_text_box(lines, line_count, file_name, attr_textbox);
	free(lines);
	free(buf);
}

void show_text(const char *const *lines, unsigned int count, const char *title)
{
	create_text_box(lines, count, title, attr_textbox);
}

/**********************************************************************/

static struct widget text_widget;
static char *title;
static int widget_attrs;
static char **text_lines;
static unsigned int text_lines_count;
static int max_line_width;
static int text_box_y;
static int text_box_x;
static int max_scroll_y;
static int max_scroll_x;
static int current_top;
static int current_left;

static void update_text_lines(void)
{
	int i;
	int width;
	const char *line_begin;
	const char *line_end;
	int cur_y, cur_x;
	int rest_of_line;

	for (i = 0; i < text_box_y; ++i) {
		width = current_left;
		line_begin = mbs_at_width(text_lines[current_top + i], &width, 1);
		wmove(text_widget.window, i + 1, 1);
		if (width > current_left)
			waddch(text_widget.window, ' ');
		if (*line_begin != '\0') {
			width = text_box_x - (width > current_left);
			line_end = mbs_at_width(line_begin, &width, -1);
			if (width)
				waddnstr(text_widget.window, line_begin,
					 line_end - line_begin);
		}
		getyx(text_widget.window, cur_y, cur_x);
		if (cur_y == i + 1) {
			rest_of_line = text_box_x + 1 - cur_x;
			if (rest_of_line > 0)
				wprintw(text_widget.window, "%*s", rest_of_line, "");
		}
	}
}

static void update_y_scroll_bar(void)
{
	int length;
	int begin, end;
	int i;

	if (max_scroll_y <= 0 || text_lines_count == 0)
		return;
	length = text_box_y * text_box_y / text_lines_count;
	if (length >= text_box_y)
		return;
	begin = current_top * (text_box_y - length) / max_scroll_y;
	end = begin + length;
	for (i = 0; i < text_box_y; ++i)
		mvwaddch(text_widget.window, i + 1, text_box_x + 1,
			 i >= begin && i < end ? ACS_BOARD : ' ');
}

static void update_x_scroll_bar(void)
{
	int length;
	int begin, end;
	int i;

	if (max_scroll_x <= 0 || max_line_width <= 0)
		return;
	length = text_box_x * text_box_x / max_line_width;
	if (length >= text_box_x)
		return;
	begin = current_left * (text_box_x - length) / max_scroll_x;
	end = begin + length;
	wmove(text_widget.window, text_box_y + 1, 1);
	for (i = 0; i < text_box_x; ++i)
		waddch(text_widget.window, i >= begin && i < end ? ACS_BOARD : ' ');
}

static void move_x(int delta)
{
	int left;

	left = current_left + delta;
	if (left < 0)
		left = 0;
	else if (left > max_scroll_x)
		left = max_scroll_x;
	if (left != current_left) {
		current_left = left;
		update_text_lines();
		update_x_scroll_bar();
	}
}

static void move_y(int delta)
{
	int top;

	top = current_top + delta;
	if (top < 0)
		top = 0;
	else if (top > max_scroll_y)
		top = max_scroll_y;
	if (top != current_top) {
		current_top = top;
		update_text_lines();
		update_y_scroll_bar();
	}
}

static void on_handle_key(int key)
{
	switch (key) {
	case 10:
	case 13:
	case 27:
	case KEY_CANCEL:
	case KEY_ENTER:
	case KEY_CLOSE:
	case KEY_EXIT:
		text_widget.close();
		break;
	case KEY_DOWN:
	case KEY_SF:
	case 'J':
	case 'j':
	case 'X':
	case 'x':
		move_y(1);
		break;
	case KEY_UP:
	case KEY_SR:
	case 'K':
	case 'k':
	case 'W':
	case 'w':
		move_y(-1);
		break;
	case KEY_LEFT:
	case 'H':
	case 'h':
	case 'P':
	case 'p':
		move_x(-1);
		break;
	case KEY_RIGHT:
	case 'L':
	case 'l':
	case 'N':
	case 'n':
		move_x(1);
		break;
	case KEY_NPAGE:
	case ' ':
		move_y(text_box_y);
		break;
	case KEY_PPAGE:
	case KEY_BACKSPACE:
	case 'B':
	case 'b':
		move_y(-text_box_y);
		break;
	case KEY_HOME:
	case KEY_BEG:
		move_x(-max_scroll_x);
		break;
	case KEY_LL:
	case KEY_END:
		move_x(max_scroll_x);
		break;
	case '\t':
		move_x(8);
		break;
	case KEY_BTAB:
		move_x(-8);
		break;
	}
}

static bool create(void)
{
	int len, width;

	if (screen_lines < 3 || screen_cols < 8) {
		text_widget.close();
		beep();
		return FALSE;
	}

	width = max_line_width;
	len = get_mbs_width(title) + 2;
	if (width < len)
		width = len;

	text_box_y = text_lines_count;
	if (text_box_y > screen_lines - 2)
		text_box_y = screen_lines - 2;
	max_scroll_y = text_lines_count - text_box_y;
	text_box_x = width;
	if (text_box_x > screen_cols - 2)
		text_box_x = screen_cols - 2;
	max_scroll_x = max_line_width - text_box_x;

	widget_init(&text_widget, text_box_y + 2, text_box_x + 2,
		    SCREEN_CENTER, SCREEN_CENTER, widget_attrs, WIDGET_BORDER);
	mvwprintw(text_widget.window, 0, (text_box_x + 2 - get_mbs_width(title) - 2) / 2, " %s ", title);

	if (current_top > max_scroll_y)
		current_top = max_scroll_y;
	if (current_left > max_scroll_x)
		current_left = max_scroll_x;
	update_text_lines();
	update_y_scroll_bar();
	update_x_scroll_bar();
	return TRUE;
}

static void on_window_size_changed(void)
{
	create();
}

static void on_close(void)
{
	unsigned int i;

	for (i = 0; i < text_lines_count; ++i)
		free(text_lines[i]);
	free(text_lines);
	widget_free(&text_widget);
}

static struct widget text_widget = {
	.handle_key = on_handle_key,
	.window_size_changed = on_window_size_changed,
	.close = on_close,
};

static void create_text_box(const char *const *lines, unsigned int count,
			    const char *title_, int attrs)
{
	unsigned int i;

	text_lines = ccalloc(count, sizeof *text_lines);
	for (i = 0; i < count; ++i)
		text_lines[i] = cstrdup(lines[i]);
	text_lines_count = count;
	max_line_width = get_max_mbs_width(lines, count);
	title = cstrdup(title_);
	widget_attrs = attrs;

	current_top = 0;
	current_left = 0;

	create();
}
