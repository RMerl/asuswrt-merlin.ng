/*
 * die.c - error handlers
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
#include <alsa/asoundlib.h>
#include "gettext_curses.h"
#include "mainloop.h"
#include "die.h"

void fatal_error(const char *msg)
{
	shutdown();
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

void fatal_alsa_error(const char *msg, int err)
{
	shutdown();
	fprintf(stderr, _("%s: %s\n"), msg, snd_strerror(err));
	exit(EXIT_FAILURE);
}
