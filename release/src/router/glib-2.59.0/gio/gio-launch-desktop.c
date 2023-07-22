/* GIO - GLib Input, Output and Streaming Library
 *
 * Copyright (C) 2018 Endless Mobile, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Daniel Drake <drake@endlessm.com>
 */

/*
 * gio-launch-desktop: GDesktopAppInfo helper
 * Executable wrapper to set GIO_LAUNCHED_DESKTOP_FILE_PID
 * There are complications when doing this in a fork()/exec() codepath,
 * and it cannot otherwise be done with posix_spawn().
 * This wrapper is designed to be minimal and lightweight.
 * It does not even link against glib.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int
main (int argc, char *argv[])
{
  pid_t pid = getpid ();
  char buf[50];
  int r;

  if (argc < 2)
    return -1;

  r = snprintf (buf, sizeof (buf), "GIO_LAUNCHED_DESKTOP_FILE_PID=%ld", (long) pid);
  if (r >= sizeof (buf))
    return -1;

  putenv (buf);

  return execvp (argv[1], argv + 1);
}
