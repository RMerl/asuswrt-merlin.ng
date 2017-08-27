/* inotify-helper.h - GVFS Directory Monitor using inotify

   Copyright (C) 2007 John McCutchan

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

   Author: John McCutchan <john@johnmccutchan.com>
*/


#ifndef __INOTIFY_HELPER_H
#define __INOTIFY_HELPER_H

#include "inotify-sub.h"

gboolean _ih_startup    (void);
gboolean _ih_sub_add    (inotify_sub *sub);
gboolean _ih_sub_cancel (inotify_sub *sub);

#endif /* __INOTIFY_HELPER_H */
