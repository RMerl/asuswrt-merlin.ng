/* GIO - GLib Input, Output and Streaming Library
 * 
 * Copyright (C) 2006-2007 Red Hat, Inc.
 * Copyright (C) 2007 Sebastian Dröge.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors: Alexander Larsson <alexl@redhat.com>
 *          John McCutchan <john@johnmccutchan.com> 
 *          Sebastian Dröge <slomo@circular-chaos.org>
 */

#ifndef __G_INOTIFY_DIRECTORY_MONITOR_H__
#define __G_INOTIFY_DIRECTORY_MONITOR_H__

#include <glib-object.h>
#include <string.h>
#include <gio/glocaldirectorymonitor.h>
#include <gio/giomodule.h>

G_BEGIN_DECLS

#define G_TYPE_INOTIFY_DIRECTORY_MONITOR		(_g_inotify_directory_monitor_get_type ())
#define G_INOTIFY_DIRECTORY_MONITOR(o)			(G_TYPE_CHECK_INSTANCE_CAST ((o), G_TYPE_INOTIFY_DIRECTORY_MONITOR, GInotifyDirectoryMonitor))
#define G_INOTIFY_DIRECTORY_MONITOR_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST ((k), G_TYPE_INOTIFY_DIRECTORY_MONITOR, GInotifyDirectoryMonitorClass))
#define G_IS_INOTIFY_DIRECTORY_MONITOR(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), G_TYPE_INOTIFY_DIRECTORY_MONITOR))
#define G_IS_INOTIFY_DIRECTORY_MONITOR_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), G_TYPE_INOTIFY_DIRECTORY_MONITOR))

typedef struct _GInotifyDirectoryMonitor      GInotifyDirectoryMonitor;
typedef struct _GInotifyDirectoryMonitorClass GInotifyDirectoryMonitorClass;

struct _GInotifyDirectoryMonitorClass {
  GLocalDirectoryMonitorClass parent_class;
};

GType _g_inotify_directory_monitor_get_type (void);

G_END_DECLS

#endif /* __G_INOTIFY_DIRECTORY_MONITOR_H__ */
