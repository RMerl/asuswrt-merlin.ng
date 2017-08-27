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

#include "config.h"

#include "gfamdirectorymonitor.h"
#include <gio/giomodule.h>

#include "fam-helper.h"

struct _GFamDirectoryMonitor
{
  GLocalDirectoryMonitor parent_instance;
  fam_sub *sub;
};

static gboolean g_fam_directory_monitor_cancel (GFileMonitor* monitor);

G_DEFINE_DYNAMIC_TYPE (GFamDirectoryMonitor, g_fam_directory_monitor, G_TYPE_LOCAL_DIRECTORY_MONITOR)

static void
g_fam_directory_monitor_finalize (GObject *object)
{
  GFamDirectoryMonitor *fam_monitor = G_FAM_DIRECTORY_MONITOR (object);
  fam_sub *sub = fam_monitor->sub;

  if (sub) {
    if (!_fam_sub_cancel (sub))
      g_warning ("Unexpected error cancelling fam monitor");

    fam_monitor->sub = NULL;
  }

  if (G_OBJECT_CLASS (g_fam_directory_monitor_parent_class)->finalize)
    (*G_OBJECT_CLASS (g_fam_directory_monitor_parent_class)->finalize) (object);
}

static GObject *
g_fam_directory_monitor_constructor (GType type,
                                    guint n_construct_properties,
                                    GObjectConstructParam *construct_properties)
{
  GObject *obj;
  GFamDirectoryMonitorClass *klass;
  GObjectClass *parent_class;
  GFamDirectoryMonitor *fam_monitor;
  const gchar *dirname = NULL;
  fam_sub *sub = NULL;
  
  klass = G_FAM_DIRECTORY_MONITOR_CLASS (g_type_class_peek (G_TYPE_FAM_DIRECTORY_MONITOR));
  parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
  obj = parent_class->constructor (type,
                                   n_construct_properties,
                                   construct_properties);

  fam_monitor = G_FAM_DIRECTORY_MONITOR (obj);

  dirname = G_LOCAL_DIRECTORY_MONITOR (obj)->dirname;
  g_assert (dirname != NULL);

  sub = _fam_sub_add (dirname, TRUE, fam_monitor);
  /* FIXME: what to do about errors here? we can't return NULL or another
   * kind of error and an assertion is probably too hard */
  g_assert (sub != NULL);

  fam_monitor->sub = sub;

  return obj;
}

static void
g_fam_directory_monitor_class_finalize (GFamDirectoryMonitorClass *klass)
{
}

static gboolean
g_fam_directory_monitor_is_supported (void)
{
  return _fam_sub_startup ();
}

static void
g_fam_directory_monitor_class_init (GFamDirectoryMonitorClass* klass)
{
  GObjectClass* gobject_class = G_OBJECT_CLASS (klass);
  GFileMonitorClass *file_monitor_class = G_FILE_MONITOR_CLASS (klass);
  GLocalDirectoryMonitorClass *local_directory_monitor_class = G_LOCAL_DIRECTORY_MONITOR_CLASS (klass);
  
  gobject_class->finalize = g_fam_directory_monitor_finalize;
  gobject_class->constructor = g_fam_directory_monitor_constructor;
  file_monitor_class->cancel = g_fam_directory_monitor_cancel;

  local_directory_monitor_class->mount_notify = FALSE;
  local_directory_monitor_class->is_supported = g_fam_directory_monitor_is_supported;
}

static void
g_fam_directory_monitor_init (GFamDirectoryMonitor* monitor)
{

}

static gboolean
g_fam_directory_monitor_cancel (GFileMonitor* monitor)
{
  GFamDirectoryMonitor *fam_monitor = G_FAM_DIRECTORY_MONITOR (monitor);
  fam_sub *sub = fam_monitor->sub;

  if (sub) {
    if (!_fam_sub_cancel (sub))
      g_warning ("Unexpected error cancelling fam monitor");

    fam_monitor->sub = NULL;
  }

  if (G_FILE_MONITOR_CLASS (g_fam_directory_monitor_parent_class)->cancel)
    (*G_FILE_MONITOR_CLASS (g_fam_directory_monitor_parent_class)->cancel) (monitor);

  return TRUE;
}

void
g_fam_directory_monitor_register (GIOModule *module)
{
  g_fam_directory_monitor_register_type (G_TYPE_MODULE (module));
  g_io_extension_point_implement (G_LOCAL_DIRECTORY_MONITOR_EXTENSION_POINT_NAME,
				  G_TYPE_FAM_DIRECTORY_MONITOR,
				  "fam",
				  10);
  g_io_extension_point_implement (G_NFS_DIRECTORY_MONITOR_EXTENSION_POINT_NAME,
				  G_TYPE_FAM_DIRECTORY_MONITOR,
				  "fam",
				  10);
}

