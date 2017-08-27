/*
 * mainloop.c - main loop
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
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
#include <panel.h>
#include <alsa/asoundlib.h>
#include "mem.h"
#include "die.h"
#include "colors.h"
#include "widget.h"
#include "mixer_widget.h"
#include "mixer_display.h"
#include "mainloop.h"

static WINDOW *curses_initialized;

static void black_hole_error_handler(const char *file, int line,
				     const char *function, int err,
				     const char *fmt, ...)
{
}

void initialize_curses(bool use_color)
{
	curses_initialized = initscr();
	cbreak();
	noecho();
#ifdef HAVE_CURSES_ESCDELAY
	set_escdelay(100);
#endif
	window_size_changed(); /* update screen_lines/cols */
	init_colors(use_color);
	snd_lib_error_set_handler(black_hole_error_handler);
}

void shutdown(void)
{
	if (curses_initialized) {
		clear();
		refresh();
		curs_set(1);
		endwin();
	}
	mixer_shutdown();
}

void mainloop(void)
{
	struct pollfd *pollfds = NULL;
	int nfds = 0, n;
	struct widget *active_widget;
	unsigned short revents;
	int key;
	int err;

	for (;;) {
		update_panels();
		doupdate();

		active_widget = get_active_widget();
		if (!active_widget)
			break;

		n = 1 + snd_mixer_poll_descriptors_count(mixer);
		if (n != nfds) {
			free(pollfds);
			nfds = n;
			pollfds = ccalloc(nfds, sizeof *pollfds);
			pollfds[0].fd = fileno(stdin);
			pollfds[0].events = POLLIN;
		}
		err = snd_mixer_poll_descriptors(mixer, &pollfds[1], nfds - 1);
		if (err < 0)
			fatal_alsa_error("cannot get poll descriptors", err);
		n = poll(pollfds, nfds, -1);
		if (n < 0) {
			if (errno == EINTR) {
				pollfds[0].revents = 0;
				doupdate(); /* handle SIGWINCH */
			} else {
				fatal_error("poll error");
			}
		}
		if (pollfds[0].revents & (POLLERR | POLLHUP | POLLNVAL))
			break;
		if (pollfds[0].revents & POLLIN)
			--n;
		if (n > 0) {
			err = snd_mixer_poll_descriptors_revents(mixer, &pollfds[1], nfds - 1, &revents);
			if (err < 0)
				fatal_alsa_error("cannot get poll events", err);
			if (revents & (POLLERR | POLLNVAL))
				close_mixer_device();
			else if (revents & POLLIN)
				snd_mixer_handle_events(mixer);
		}
		key = wgetch(active_widget->window);
		while (key != ERR) {
#ifdef KEY_RESIZE
			if (key == KEY_RESIZE)
				window_size_changed();
			else
#endif
				active_widget->handle_key(key);
			active_widget = get_active_widget();
			if (!active_widget)
				break;
			key = wgetch(active_widget->window);
		}
		if (!active_widget)
			break;
		if (controls_changed)
			display_controls();
	}
	free(pollfds);
}
