/* GIO - GLib Input, Output and Streaming Library
 *
 * Copyright (C) 2006-2007 Red Hat, Inc.
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
 * Author: Alexander Larsson <alexl@redhat.com>
 */

#ifndef __G_WIN32_APP_INFO_H__
#define __G_WIN32_APP_INFO_H__

#include <gio/giotypes.h>

G_BEGIN_DECLS

#define G_TYPE_WIN32_APP_INFO         (g_win32_app_info_get_type ())
#define G_WIN32_APP_INFO(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), G_TYPE_WIN32_APP_INFO, GWin32AppInfo))
#define G_WIN32_APP_INFO_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_WIN32_APP_INFO, GWin32AppInfoClass))
#define G_IS_WIN32_APP_INFO(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), G_TYPE_WIN32_APP_INFO))
#define G_IS_WIN32_APP_INFO_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), G_TYPE_WIN32_APP_INFO))
#define G_WIN32_APP_INFO_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), G_TYPE_WIN32_APP_INFO, GWin32AppInfoClass))

typedef struct _GWin32AppInfo        GWin32AppInfo;
typedef struct _GWin32AppInfoClass   GWin32AppInfoClass;

struct _GWin32AppInfoClass
{
  GObjectClass parent_class;
};

GType g_win32_app_info_get_type (void) G_GNUC_CONST;

G_END_DECLS


#endif /* __G_WIN32_APP_INFO_H__ */
