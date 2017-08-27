/*
   Copyright (C) 2005 John McCutchan

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

   Authors:.
		John McCutchan <john@johnmccutchan.com>
*/

#ifndef __INOTIFY_PATH_H
#define __INOTIFY_PATH_H

#include "inotify-kernel.h"
#include "inotify-sub.h"

gboolean     _ip_startup (void (*event_cb)(ik_event_t *event, inotify_sub *sub, gboolean file_event));
gboolean     _ip_start_watching (inotify_sub *sub);
gboolean     _ip_stop_watching  (inotify_sub *sub);
const char * _ip_get_path_for_wd (gint32 wd);
#endif
