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

#include "config.h"

#include "glocaldirectorymonitor.h"
#include "gunixmounts.h"
#include "giomodule-priv.h"
#include "gfile.h"
#include "gioerror.h"
#include "glibintl.h"

#include <string.h>


enum
{
  PROP_0,
  PROP_DIRNAME,
  PROP_FLAGS
};

static gboolean g_local_directory_monitor_cancel (GFileMonitor      *monitor);
static void     mounts_changed                   (GUnixMountMonitor *mount_monitor, 
                                                  gpointer           user_data);

G_DEFINE_ABSTRACT_TYPE (GLocalDirectoryMonitor, g_local_directory_monitor, G_TYPE_FILE_MONITOR)

static void
g_local_directory_monitor_finalize (GObject *object)
{
  GLocalDirectoryMonitor *local_monitor;
  local_monitor = G_LOCAL_DIRECTORY_MONITOR (object);

  g_free (local_monitor->dirname);

  if (local_monitor->mount_monitor)
    {
      g_signal_handlers_disconnect_by_func (local_monitor->mount_monitor, mounts_changed, local_monitor);
      g_object_unref (local_monitor->mount_monitor);
      local_monitor->mount_monitor = NULL;
    }

  G_OBJECT_CLASS (g_local_directory_monitor_parent_class)->finalize (object);
}

static void
g_local_directory_monitor_set_property (GObject      *object,
                                        guint         property_id,
                                        const GValue *value,
                                        GParamSpec   *pspec)
{
  switch (property_id)
  {
    case PROP_DIRNAME:
      /* Do nothing */
      break;
    case PROP_FLAGS:
      /* Do nothing */
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static GObject *
g_local_directory_monitor_constructor (GType                  type,
                                       guint                  n_construct_properties,
                                       GObjectConstructParam *construct_properties)
{
  GObject *obj;
  GLocalDirectoryMonitorClass *klass;
  GObjectClass *parent_class;
  GLocalDirectoryMonitor *local_monitor;
  GFileMonitorFlags  flags = 0;
  const gchar *dirname = NULL;
  gint i;
  
  klass = G_LOCAL_DIRECTORY_MONITOR_CLASS (g_type_class_peek (G_TYPE_LOCAL_DIRECTORY_MONITOR));
  parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
  obj = parent_class->constructor (type,
                                   n_construct_properties,
                                   construct_properties);

  local_monitor = G_LOCAL_DIRECTORY_MONITOR (obj);

  for (i = 0; i < n_construct_properties; i++)
    {
      if (strcmp ("dirname", g_param_spec_get_name (construct_properties[i].pspec)) == 0)
        {
          g_warn_if_fail (G_VALUE_HOLDS_STRING (construct_properties[i].value));
          dirname = g_value_get_string (construct_properties[i].value);
        }
      if (strcmp ("flags", g_param_spec_get_name (construct_properties[i].pspec)) == 0)
        {
          g_warn_if_fail (G_VALUE_HOLDS_FLAGS (construct_properties[i].value));
          flags = g_value_get_flags (construct_properties[i].value);
        }
    }

  local_monitor->dirname = g_strdup (dirname);
  local_monitor->flags = flags;

  if (!klass->mount_notify &&
      (flags & G_FILE_MONITOR_WATCH_MOUNTS))
    {
#ifdef G_OS_WIN32
      /*claim everything was mounted */
      local_monitor->was_mounted = TRUE;
#else
      GUnixMountEntry *mount;
      
      /* Emulate unmount detection */
      
      mount = g_unix_mount_at (local_monitor->dirname, NULL);
      
      local_monitor->was_mounted = mount != NULL;
      
      if (mount)
        g_unix_mount_free (mount);

      local_monitor->mount_monitor = g_unix_mount_monitor_new ();
      g_signal_connect_object (local_monitor->mount_monitor, "mounts-changed",
			       G_CALLBACK (mounts_changed), local_monitor, 0);
#endif
    }

  return obj;
}

static void
g_local_directory_monitor_class_init (GLocalDirectoryMonitorClass* klass)
{
  GObjectClass* gobject_class = G_OBJECT_CLASS (klass);
  GFileMonitorClass *file_monitor_class = G_FILE_MONITOR_CLASS (klass);
  
  gobject_class->finalize = g_local_directory_monitor_finalize;
  gobject_class->set_property = g_local_directory_monitor_set_property;
  gobject_class->constructor = g_local_directory_monitor_constructor;

  file_monitor_class->cancel = g_local_directory_monitor_cancel;

  g_object_class_install_property (gobject_class, 
                                   PROP_DIRNAME,
                                   g_param_spec_string ("dirname", 
                                                        P_("Directory name"), 
                                                        P_("Directory to monitor"),
                                                        NULL, 
                                                        G_PARAM_CONSTRUCT_ONLY|
                                                        G_PARAM_WRITABLE|
                                                        G_PARAM_STATIC_NAME|G_PARAM_STATIC_NICK|G_PARAM_STATIC_BLURB));
  g_object_class_install_property (gobject_class,
                                   PROP_FLAGS,
                                   g_param_spec_flags ("flags",
						       P_("Monitor flags"),
						       P_("Monitor flags"),
						       G_TYPE_FILE_MONITOR_FLAGS,
						       0,
						       G_PARAM_CONSTRUCT_ONLY|
						       G_PARAM_WRITABLE|
						       G_PARAM_STATIC_NAME|G_PARAM_STATIC_NICK|G_PARAM_STATIC_BLURB));

  klass->mount_notify = FALSE;
}

static void
g_local_directory_monitor_init (GLocalDirectoryMonitor *local_monitor)
{
}

static void
mounts_changed (GUnixMountMonitor *mount_monitor,
                gpointer           user_data)
{
  GLocalDirectoryMonitor *local_monitor = user_data;
#ifdef G_OS_UNIX
  GUnixMountEntry *mount;
#endif
  gboolean is_mounted;
  GFile *file;
  
  /* Emulate unmount detection */
#ifdef G_OS_UNIX
  mount = g_unix_mount_at (local_monitor->dirname, NULL);
  
  is_mounted = mount != NULL;
  
  if (mount)
    g_unix_mount_free (mount);
#else
  /*claim everything was mounted */
  is_mounted = TRUE;
#endif

  if (local_monitor->was_mounted != is_mounted)
    {
      if (local_monitor->was_mounted && !is_mounted)
        {
          file = g_file_new_for_path (local_monitor->dirname);
          g_file_monitor_emit_event (G_FILE_MONITOR (local_monitor),
				     file, NULL,
				     G_FILE_MONITOR_EVENT_UNMOUNTED);
          g_object_unref (file);
        }
      local_monitor->was_mounted = is_mounted;
    }
}

GFileMonitor*
_g_local_directory_monitor_new (const char         *dirname,
				GFileMonitorFlags   flags,
                                gboolean            is_remote_fs,
				GError            **error)
{
  GFileMonitor *monitor = NULL;
  GType type = G_TYPE_INVALID;

  if (is_remote_fs)
    type = _g_io_module_get_default_type (G_NFS_DIRECTORY_MONITOR_EXTENSION_POINT_NAME,
                                          "GIO_USE_FILE_MONITOR",
                                          G_STRUCT_OFFSET (GLocalDirectoryMonitorClass, is_supported));

  if (type == G_TYPE_INVALID)
    type = _g_io_module_get_default_type (G_LOCAL_DIRECTORY_MONITOR_EXTENSION_POINT_NAME,
                                          "GIO_USE_FILE_MONITOR",
                                          G_STRUCT_OFFSET (GLocalDirectoryMonitorClass, is_supported));

  if (type != G_TYPE_INVALID)
    monitor = G_FILE_MONITOR (g_object_new (type, "dirname", dirname, "flags", flags, NULL));
  else
    g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                         _("Unable to find default local directory monitor type"));

  return monitor;
}

static gboolean
g_local_directory_monitor_cancel (GFileMonitor *monitor)
{
  GLocalDirectoryMonitor *local_monitor = G_LOCAL_DIRECTORY_MONITOR (monitor);

  if (local_monitor->mount_monitor)
    {
      g_signal_handlers_disconnect_by_func (local_monitor->mount_monitor, mounts_changed, local_monitor);
      g_object_unref (local_monitor->mount_monitor);
      local_monitor->mount_monitor = NULL;
    }

  return TRUE;
}
