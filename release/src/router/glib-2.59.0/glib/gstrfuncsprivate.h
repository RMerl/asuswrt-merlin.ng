/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
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
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <string.h>

/*
 * g_memdup2:
 * @mem: (nullable): the memory to copy.
 * @byte_size: the number of bytes to copy.
 *
 * Allocates @byte_size bytes of memory, and copies @byte_size bytes into it
 * from @mem. If @mem is %NULL it returns %NULL.
 *
 * This replaces g_memdup(), which was prone to integer overflows when
 * converting the argument from a #gsize to a #guint.
 *
 * This static inline version is a backport of the new public API from
 * GLib 2.68, kept internal to GLib for backport to older stable releases.
 * See https://gitlab.gnome.org/GNOME/glib/-/issues/2319.
 *
 * Returns: (nullable): a pointer to the newly-allocated copy of the memory,
 *    or %NULL if @mem is %NULL.
 * Since: 2.68
 */
static inline gpointer
g_memdup2 (gconstpointer mem,
           gsize         byte_size)
{
  gpointer new_mem;

  if (mem && byte_size != 0)
    {
      new_mem = g_malloc (byte_size);
      memcpy (new_mem, mem, byte_size);
    }
  else
    new_mem = NULL;

  return new_mem;
}
