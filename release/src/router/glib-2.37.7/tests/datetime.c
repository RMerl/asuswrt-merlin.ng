/*
 * Copyright (C) 2011 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * licence, or (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA.
 */

#include <locale.h>
#include <glib/glib.h>

int
main (int argc, char *argv[])
{
  gchar *fmt;
  GDateTime *dt;
  gchar *str;

  setlocale (LC_ALL, "");

  if (argc > 1)
    fmt = argv[1];
  else
    fmt = "%x %X";

  dt = g_date_time_new_now_local ();
  str = g_date_time_format (dt, fmt);
  g_print ("%s\n", str);
  g_free (str);

  return 0;
}
