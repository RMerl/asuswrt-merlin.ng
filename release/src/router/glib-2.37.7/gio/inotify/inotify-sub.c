/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 8 -*- */

/* inotify-sub.c - GMonitor based on inotify.

   Copyright (C) 2006 John McCutchan

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors: 
		 John McCutchan <john@johnmccutchan.com>
*/

#include "config.h"
#include <string.h>
#include <glib.h>

#include "inotify-sub.h"

static gboolean is_debug_enabled = FALSE;
#define IS_W if (is_debug_enabled) g_warning

static gchar*
dup_dirname (const gchar *dirname)
{
  gchar *d_dirname = g_strdup (dirname);
  size_t len = strlen (d_dirname);
  
  if (d_dirname[len - 1] == '/')
    d_dirname[len - 1] = '\0';
  
  return d_dirname;
}

inotify_sub*
_ih_sub_new (const gchar *dirname, 
             const gchar *filename,
             gboolean     pair_moves,
             gboolean     watch_hardlinks,
             gpointer     user_data)
{
  inotify_sub *sub = NULL;
  
  sub = g_new0 (inotify_sub, 1);
  sub->dirname = dup_dirname (dirname);
  sub->filename = g_strdup (filename);
  sub->pair_moves = pair_moves;
  sub->hardlinks = watch_hardlinks;
  sub->user_data = user_data;

  IS_W ("new subscription for %s being setup\n", sub->dirname);
  
  return sub;
}

void
_ih_sub_free (inotify_sub *sub)
{
  g_free (sub->dirname);
  g_free (sub->filename);
  g_free (sub);
}
