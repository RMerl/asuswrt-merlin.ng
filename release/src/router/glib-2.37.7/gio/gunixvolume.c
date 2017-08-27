/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

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
 *         David Zeuthen <davidz@redhat.com>
 */

#include "config.h"

#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <glib.h>
#include "gunixvolume.h"
#include "gunixmount.h"
#include "gunixmounts.h"
#include "gthemedicon.h"
#include "gvolume.h"
#include "gvolumemonitor.h"
#include "gtask.h"
#include "gioerror.h"
#include "glibintl.h"
/* for BUFSIZ */
#include <stdio.h>


struct _GUnixVolume {
  GObject parent;

  GVolumeMonitor *volume_monitor;
  GUnixMount     *mount; /* owned by volume monitor */
  
  char *device_path;
  char *mount_path;
  gboolean can_eject;

  char *identifier;
  char *identifier_type;
  
  char *name;
  GIcon *icon;
  GIcon *symbolic_icon;
};

static void g_unix_volume_volume_iface_init (GVolumeIface *iface);

#define g_unix_volume_get_type _g_unix_volume_get_type
G_DEFINE_TYPE_WITH_CODE (GUnixVolume, g_unix_volume, G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (G_TYPE_VOLUME,
						g_unix_volume_volume_iface_init))

static void
g_unix_volume_finalize (GObject *object)
{
  GUnixVolume *volume;
  
  volume = G_UNIX_VOLUME (object);

  if (volume->volume_monitor != NULL)
    g_object_unref (volume->volume_monitor);

  if (volume->mount)
    _g_unix_mount_unset_volume (volume->mount, volume);
  
  g_object_unref (volume->icon);
  g_object_unref (volume->symbolic_icon);
  g_free (volume->name);
  g_free (volume->mount_path);
  g_free (volume->device_path);
  g_free (volume->identifier);
  g_free (volume->identifier_type);

  G_OBJECT_CLASS (g_unix_volume_parent_class)->finalize (object);
}

static void
g_unix_volume_class_init (GUnixVolumeClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = g_unix_volume_finalize;
}

static void
g_unix_volume_init (GUnixVolume *unix_volume)
{
}

GUnixVolume *
_g_unix_volume_new (GVolumeMonitor  *volume_monitor,
                    GUnixMountPoint *mountpoint)
{
  GUnixVolume *volume;
  
  if (!(g_unix_mount_point_is_user_mountable (mountpoint) ||
	g_str_has_prefix (g_unix_mount_point_get_device_path (mountpoint), "/vol/")) ||
      g_unix_mount_point_is_loopback (mountpoint))
    return NULL;
  
  volume = g_object_new (G_TYPE_UNIX_VOLUME, NULL);
  volume->volume_monitor = volume_monitor != NULL ? g_object_ref (volume_monitor) : NULL;
  volume->mount_path = g_strdup (g_unix_mount_point_get_mount_path (mountpoint));
  volume->device_path = g_strdup (g_unix_mount_point_get_device_path (mountpoint));
  volume->can_eject = g_unix_mount_point_guess_can_eject (mountpoint);

  volume->name = g_unix_mount_point_guess_name (mountpoint);
  volume->icon = g_unix_mount_point_guess_icon (mountpoint);
  volume->symbolic_icon = g_unix_mount_point_guess_symbolic_icon (mountpoint);


  if (strcmp (g_unix_mount_point_get_fs_type (mountpoint), "nfs") == 0)
    {
      volume->identifier_type = g_strdup (G_VOLUME_IDENTIFIER_KIND_NFS_MOUNT);
      volume->identifier = g_strdup (volume->device_path);
    }
  else if (g_str_has_prefix (volume->device_path, "LABEL="))
    {
      volume->identifier_type = g_strdup (G_VOLUME_IDENTIFIER_KIND_LABEL);
      volume->identifier = g_strdup (volume->device_path + 6);
    }
  else if (g_str_has_prefix (volume->device_path, "UUID="))
    {
      volume->identifier_type = g_strdup (G_VOLUME_IDENTIFIER_KIND_UUID);
      volume->identifier = g_strdup (volume->device_path + 5);
    }
  else if (g_path_is_absolute (volume->device_path))
    {
      volume->identifier_type = g_strdup (G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
      volume->identifier = g_strdup (volume->device_path);
    }
  
  return volume;
}

void
_g_unix_volume_disconnected (GUnixVolume *volume)
{
  if (volume->mount)
    {
      _g_unix_mount_unset_volume (volume->mount, volume);
      volume->mount = NULL;
    }
}

void
_g_unix_volume_set_mount (GUnixVolume *volume,
                          GUnixMount  *mount)
{
  if (volume->mount == mount)
    return;
  
  if (volume->mount)
    _g_unix_mount_unset_volume (volume->mount, volume);
  
  volume->mount = mount;
  
  /* TODO: Emit changed in idle to avoid locking issues */
  g_signal_emit_by_name (volume, "changed");
  if (volume->volume_monitor != NULL)
    g_signal_emit_by_name (volume->volume_monitor, "volume-changed", volume);
}

void
_g_unix_volume_unset_mount (GUnixVolume  *volume,
                            GUnixMount *mount)
{
  if (volume->mount == mount)
    {
      volume->mount = NULL;
      /* TODO: Emit changed in idle to avoid locking issues */
      g_signal_emit_by_name (volume, "changed");
      if (volume->volume_monitor != NULL)
        g_signal_emit_by_name (volume->volume_monitor, "volume-changed", volume);
    }
}

static GIcon *
g_unix_volume_get_icon (GVolume *volume)
{
  GUnixVolume *unix_volume = G_UNIX_VOLUME (volume);
  return g_object_ref (unix_volume->icon);
}

static GIcon *
g_unix_volume_get_symbolic_icon (GVolume *volume)
{
  GUnixVolume *unix_volume = G_UNIX_VOLUME (volume);
  return g_object_ref (unix_volume->symbolic_icon);
}

static char *
g_unix_volume_get_name (GVolume *volume)
{
  GUnixVolume *unix_volume = G_UNIX_VOLUME (volume);
  return g_strdup (unix_volume->name);
}

static char *
g_unix_volume_get_uuid (GVolume *volume)
{
  return NULL;
}

static gboolean
g_unix_volume_can_mount (GVolume *volume)
{
  return TRUE;
}

static gboolean
g_unix_volume_can_eject (GVolume *volume)
{
  GUnixVolume *unix_volume = G_UNIX_VOLUME (volume);
  return unix_volume->can_eject;
}

static gboolean
g_unix_volume_should_automount (GVolume *volume)
{
  /* We automount all local volumes because we don't even
   * make the internal stuff visible
   */
  return TRUE;
}

static GDrive *
g_unix_volume_get_drive (GVolume *volume)
{
  return NULL;
}

static GMount *
g_unix_volume_get_mount (GVolume *volume)
{
  GUnixVolume *unix_volume = G_UNIX_VOLUME (volume);

  if (unix_volume->mount != NULL)
    return g_object_ref (unix_volume->mount);

  return NULL;
}


gboolean
_g_unix_volume_has_mount_path (GUnixVolume *volume,
                               const char  *mount_path)
{
  return strcmp (volume->mount_path, mount_path) == 0;
}


typedef struct {
  GUnixVolume *unix_volume;
  int error_fd;
  GIOChannel *error_channel;
  GSource *error_channel_source;
  GString *error_string;
} EjectMountOp;

static void
eject_mount_op_free (EjectMountOp *data)
{
  if (data->error_string != NULL)
    g_string_free (data->error_string, TRUE);

  if (data->error_channel != NULL)
    g_io_channel_unref (data->error_channel);

  if (data->error_channel_source)
    {
      g_source_destroy (data->error_channel_source);
      g_source_unref (data->error_channel_source);
    }

  if (data->error_fd != -1)
    close (data->error_fd);

  g_free (data);
}

static void
eject_mount_cb (GPid     pid,
                gint     status,
                gpointer user_data)
{
  GTask *task = user_data;
  EjectMountOp *data = g_task_get_task_data (task);
  
  if (WEXITSTATUS (status) != 0)
    {
      g_task_return_new_error (task,
                               G_IO_ERROR, 
                               G_IO_ERROR_FAILED,
                               "%s", data->error_string->str);
    }
  else
    g_task_return_boolean (task, TRUE);
  g_object_unref (task);
}

static gboolean
eject_mount_read_error (GIOChannel   *channel,
                        GIOCondition  condition,
                        gpointer      user_data)
{
  GTask *task = user_data;
  EjectMountOp *data = g_task_get_task_data (task);
  char buf[BUFSIZ];
  gsize bytes_read;
  GError *error;
  GIOStatus status;

  error = NULL;
read:
  status = g_io_channel_read_chars (channel, buf, sizeof (buf), &bytes_read, &error);
  if (status == G_IO_STATUS_NORMAL)
   {
     g_string_append_len (data->error_string, buf, bytes_read);
     if (bytes_read == sizeof (buf))
        goto read;
   }
  else if (status == G_IO_STATUS_EOF)
    g_string_append_len (data->error_string, buf, bytes_read);
  else if (status == G_IO_STATUS_ERROR)
    {
      if (data->error_string->len > 0)
        g_string_append (data->error_string, "\n");

      g_string_append (data->error_string, error->message);
      g_error_free (error);

      if (data->error_channel_source)
        {
          g_source_unref (data->error_channel_source);
          data->error_channel_source = NULL;
        }
      return FALSE;
    }

  return TRUE;
}

static void
eject_mount_do (GVolume             *volume,
                GCancellable        *cancellable,
                GAsyncReadyCallback  callback,
                gpointer             user_data,
                char               **argv)
{
  GUnixVolume *unix_volume = G_UNIX_VOLUME (volume);
  GTask *task;
  EjectMountOp *data;
  GPid child_pid;
  GSource *child_watch;
  GError *error;
  
  data = g_new0 (EjectMountOp, 1);
  data->unix_volume = unix_volume;
  data->error_fd = -1;
  
  task = g_task_new (unix_volume, cancellable, callback, user_data);
  g_task_set_task_data (task, data, (GDestroyNotify) eject_mount_op_free);

  error = NULL;
  if (!g_spawn_async_with_pipes (NULL,         /* working dir */
                                 argv,
                                 NULL,         /* envp */
                                 G_SPAWN_DO_NOT_REAP_CHILD|G_SPAWN_SEARCH_PATH,
                                 NULL,         /* child_setup */
                                 NULL,         /* user_data for child_setup */
                                 &child_pid,
                                 NULL,           /* standard_input */
                                 NULL,           /* standard_output */
                                 &(data->error_fd),
                                 &error))
    {
      g_assert (error != NULL);
      goto handle_error;
    }

  data->error_string = g_string_new ("");

  data->error_channel = g_io_channel_unix_new (data->error_fd);
  g_io_channel_set_flags (data->error_channel, G_IO_FLAG_NONBLOCK, &error);
  if (error != NULL)
    goto handle_error;

  data->error_channel_source = g_io_create_watch (data->error_channel, G_IO_IN);
  g_task_attach_source (task, data->error_channel_source,
                        (GSourceFunc) eject_mount_read_error);

  child_watch = g_child_watch_source_new (child_pid);
  g_task_attach_source (task, child_watch, (GSourceFunc) eject_mount_cb);
  g_source_unref (child_watch);

handle_error:
  if (error != NULL)
    {
      g_task_return_error (task, error);
      g_object_unref (task);
    }
}


static void
g_unix_volume_mount (GVolume            *volume,
                     GMountMountFlags    flags,
                     GMountOperation     *mount_operation,
                     GCancellable        *cancellable,
                     GAsyncReadyCallback  callback,
                     gpointer             user_data)
{
  GUnixVolume *unix_volume = G_UNIX_VOLUME (volume);
  char *argv[] = { "mount", NULL, NULL };

  if (unix_volume->mount_path != NULL)
    argv[1] = unix_volume->mount_path;
  else
    argv[1] = unix_volume->device_path;

  eject_mount_do (volume, cancellable, callback, user_data, argv);
}

static gboolean
g_unix_volume_mount_finish (GVolume        *volume,
                            GAsyncResult  *result,
                            GError       **error)
{
  return TRUE;
}

static void
g_unix_volume_eject (GVolume             *volume,
                     GMountUnmountFlags   flags,
                     GCancellable        *cancellable,
                     GAsyncReadyCallback  callback,
                     gpointer             user_data)
{
  GUnixVolume *unix_volume = G_UNIX_VOLUME (volume);
  char *argv[] = { "eject", NULL, NULL };

  argv[1] = unix_volume->device_path;

  eject_mount_do (volume, cancellable, callback, user_data, argv);
}

static gboolean
g_unix_volume_eject_finish (GVolume       *volume,
                            GAsyncResult  *result,
                            GError       **error)
{
  return TRUE;
}

static gchar *
g_unix_volume_get_identifier (GVolume     *volume,
                              const gchar *kind)
{
  GUnixVolume *unix_volume = G_UNIX_VOLUME (volume);

  if (unix_volume->identifier_type != NULL &&
      strcmp (kind, unix_volume->identifier_type) == 0)
    return g_strdup (unix_volume->identifier);

  return NULL;
}

static gchar **
g_unix_volume_enumerate_identifiers (GVolume *volume)
{
  GUnixVolume *unix_volume = G_UNIX_VOLUME (volume);
  gchar **res;

  if (unix_volume->identifier_type)
    {
      res = g_new (gchar *, 2);
      res[0] = g_strdup (unix_volume->identifier_type);
      res[1] = NULL;
    }
  else
    {
      res = g_new (gchar *, 1);
      res[0] = NULL;
    }

  return res;
}

static void
g_unix_volume_volume_iface_init (GVolumeIface *iface)
{
  iface->get_name = g_unix_volume_get_name;
  iface->get_icon = g_unix_volume_get_icon;
  iface->get_symbolic_icon = g_unix_volume_get_symbolic_icon;
  iface->get_uuid = g_unix_volume_get_uuid;
  iface->get_drive = g_unix_volume_get_drive;
  iface->get_mount = g_unix_volume_get_mount;
  iface->can_mount = g_unix_volume_can_mount;
  iface->can_eject = g_unix_volume_can_eject;
  iface->should_automount = g_unix_volume_should_automount;
  iface->mount_fn = g_unix_volume_mount;
  iface->mount_finish = g_unix_volume_mount_finish;
  iface->eject = g_unix_volume_eject;
  iface->eject_finish = g_unix_volume_eject_finish;
  iface->get_identifier = g_unix_volume_get_identifier;
  iface->enumerate_identifiers = g_unix_volume_enumerate_identifiers;
}
