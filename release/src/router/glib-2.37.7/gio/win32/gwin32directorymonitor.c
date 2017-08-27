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
 * Author: Vlad Grecescu <b100dian@gmail.com>
 * 
 */

#include "config.h"
#include "gwin32directorymonitor.h"
#include <windows.h>

G_DEFINE_TYPE_WITH_CODE (GWin32DirectoryMonitor,
			 g_win32_directory_monitor,
			 G_TYPE_LOCAL_DIRECTORY_MONITOR,
			 g_io_extension_point_implement (G_LOCAL_DIRECTORY_MONITOR_EXTENSION_POINT_NAME,
							 g_define_type_id,
							 "readdirectorychanges",
							 20))

struct _GWin32DirectoryMonitorPrivate {
  OVERLAPPED overlapped;
  DWORD buffer_allocated_bytes;
  gchar *file_notify_buffer;
  DWORD buffer_filled_bytes;
  HANDLE hDirectory;
  /* Needed in the APC where we only have this private struct */
  GFileMonitor *self;
};

static void g_win32_directory_monitor_finalize (GObject *base);
static gboolean g_win32_directory_monitor_cancel (GFileMonitor *base);

static GObject *g_win32_directory_monitor_constructor (GType                  type,
						       guint                  n_construct_properties,
						       GObjectConstructParam *construct_properties);

static gboolean
g_win32_directory_monitor_is_supported (void)
{
  return TRUE;
}

static void
g_win32_directory_monitor_finalize (GObject *base)
{
  GWin32DirectoryMonitor *self;
  self = G_WIN32_DIRECTORY_MONITOR (base);

  if (self->priv->hDirectory == INVALID_HANDLE_VALUE)
    {
      /* If we don't have a directory handle we can free
       * self->priv->file_notify_buffer and self->priv here. The
       * callback won't be called obviously any more (and presumably
       * never has been called).
       */
      g_free (self->priv->file_notify_buffer);
      self->priv->file_notify_buffer = NULL;
      g_free (self->priv);
    }
  else
    {
      /* If we have a directory handle, the OVERLAPPED struct is
       * passed once more to the callback as a result of the
       * CloseHandle() done in the cancel method, so self->priv has to
       * be kept around. The GWin32DirectoryMonitor object is
       * disappearing, so can't leave a pointer to it in
       * self->priv->self.
       */
      self->priv->self = NULL;
    }

  if (G_OBJECT_CLASS (g_win32_directory_monitor_parent_class)->finalize)
    (*G_OBJECT_CLASS (g_win32_directory_monitor_parent_class)->finalize) (base);
}

static gboolean
g_win32_directory_monitor_cancel (GFileMonitor *base)
{
  GWin32DirectoryMonitor *self;
  self = G_WIN32_DIRECTORY_MONITOR (base);

  /* This triggers a last callback() with nBytes==0. */

  /* Actually I am not so sure about that, it seems to trigger a last
   * callback allright, but the way to recognize that it is the final
   * one is not to check for nBytes==0, I think that was a
   * misunderstanding.
   */ 
  if (self->priv->hDirectory != INVALID_HANDLE_VALUE)
    CloseHandle (self->priv->hDirectory);

  if (G_FILE_MONITOR_CLASS (g_win32_directory_monitor_parent_class)->cancel)
    (*G_FILE_MONITOR_CLASS (g_win32_directory_monitor_parent_class)->cancel) (base);
  return TRUE;
}

static void CALLBACK 
g_win32_directory_monitor_callback (DWORD        error,
				    DWORD        nBytes,
				    LPOVERLAPPED lpOverlapped)
{
  gulong offset;
  PFILE_NOTIFY_INFORMATION pfile_notify_walker;
  glong file_name_len;
  gchar *file_name;
  gchar *path;
  GFile *file;
  GWin32DirectoryMonitorPrivate *priv = (GWin32DirectoryMonitorPrivate *) lpOverlapped;

  static GFileMonitorEvent events[] =
    {
      0, 
      G_FILE_MONITOR_EVENT_CREATED, /* FILE_ACTION_ADDED            */
      G_FILE_MONITOR_EVENT_DELETED, /* FILE_ACTION_REMOVED          */
      G_FILE_MONITOR_EVENT_CHANGED, /* FILE_ACTION_MODIFIED         */
      G_FILE_MONITOR_EVENT_DELETED, /* FILE_ACTION_RENAMED_OLD_NAME */
      G_FILE_MONITOR_EVENT_CREATED, /* FILE_ACTION_RENAMED_NEW_NAME */
    };

  /* If priv->self is NULL the GWin32DirectoryMonitor object has been destroyed. */
  if (priv->self == NULL ||
      g_file_monitor_is_cancelled (priv->self) ||
      priv->file_notify_buffer == NULL)
    {
      g_free (priv->file_notify_buffer);
      g_free (priv);
      return;
    }

  offset = 0;
  do {
    pfile_notify_walker = (PFILE_NOTIFY_INFORMATION)(priv->file_notify_buffer + offset);
    if (pfile_notify_walker->Action > 0)
      {
	file_name = g_utf16_to_utf8 (pfile_notify_walker->FileName, pfile_notify_walker->FileNameLength / sizeof(WCHAR), NULL, &file_name_len, NULL);
	path = g_build_filename(G_LOCAL_DIRECTORY_MONITOR (priv->self)->dirname, file_name, NULL);
	file = g_file_new_for_path (path);
	g_file_monitor_emit_event (priv->self, file, NULL, events [pfile_notify_walker->Action]);
	g_object_unref (file);
	g_free (path);
	g_free (file_name);
      }
    offset += pfile_notify_walker->NextEntryOffset;
  } while (pfile_notify_walker->NextEntryOffset);

  ReadDirectoryChangesW (priv->hDirectory,
			 (gpointer)priv->file_notify_buffer,
			 priv->buffer_allocated_bytes,
			 FALSE, 
			 FILE_NOTIFY_CHANGE_FILE_NAME |
			 FILE_NOTIFY_CHANGE_DIR_NAME |
			 FILE_NOTIFY_CHANGE_ATTRIBUTES |
			 FILE_NOTIFY_CHANGE_SIZE,
			 &priv->buffer_filled_bytes,
			 &priv->overlapped,
			 g_win32_directory_monitor_callback);
}

static GObject *
g_win32_directory_monitor_constructor (GType                  type,
				       guint                  n_construct_properties,
				       GObjectConstructParam *construct_properties) {
  GObject *obj;
  GWin32DirectoryMonitorClass *klass;
  GObjectClass *parent_class;
  GWin32DirectoryMonitor *self;
  wchar_t *wdirname;

  klass = G_WIN32_DIRECTORY_MONITOR_CLASS (g_type_class_peek (G_TYPE_WIN32_DIRECTORY_MONITOR));
  parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
  obj = parent_class->constructor (type, n_construct_properties, construct_properties);
  self = G_WIN32_DIRECTORY_MONITOR (obj);
  wdirname = g_utf8_to_utf16 (G_LOCAL_DIRECTORY_MONITOR (obj)->dirname, -1, NULL, NULL, NULL);

  self->priv->hDirectory = CreateFileW (wdirname,
					FILE_LIST_DIRECTORY,
					FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, 

					NULL,
					OPEN_EXISTING,
					FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
					NULL); 
  g_free (wdirname);
  if (self->priv->hDirectory == INVALID_HANDLE_VALUE)
    {
      /* Ignore errors */
      return obj;
    }

  ReadDirectoryChangesW (self->priv->hDirectory,
                         (gpointer)self->priv->file_notify_buffer,
                         self->priv->buffer_allocated_bytes,
                         FALSE,
                         FILE_NOTIFY_CHANGE_FILE_NAME |
                         FILE_NOTIFY_CHANGE_DIR_NAME |
                         FILE_NOTIFY_CHANGE_ATTRIBUTES |
                         FILE_NOTIFY_CHANGE_SIZE,
                         &self->priv->buffer_filled_bytes,
                         &self->priv->overlapped,
                         g_win32_directory_monitor_callback);
  /* Ignore errors */

  return obj;
}

static void 
g_win32_directory_monitor_class_init (GWin32DirectoryMonitorClass *klass)
{
  g_win32_directory_monitor_parent_class = g_type_class_peek_parent (klass);

  G_OBJECT_CLASS (klass)->constructor = g_win32_directory_monitor_constructor;
  G_OBJECT_CLASS (klass)->finalize = g_win32_directory_monitor_finalize;
  G_FILE_MONITOR_CLASS (klass)->cancel = g_win32_directory_monitor_cancel;

  G_LOCAL_DIRECTORY_MONITOR_CLASS (klass)->mount_notify = FALSE;
  G_LOCAL_DIRECTORY_MONITOR_CLASS (klass)->is_supported = g_win32_directory_monitor_is_supported;
}

static void
g_win32_directory_monitor_init (GWin32DirectoryMonitor *self) 
{
  self->priv = (GWin32DirectoryMonitorPrivate*)g_new0 (GWin32DirectoryMonitorPrivate, 1);
  g_assert (self->priv != 0);
	
  self->priv->buffer_allocated_bytes = 32768;
  self->priv->file_notify_buffer = g_new0 (gchar, self->priv->buffer_allocated_bytes);
  g_assert (self->priv->file_notify_buffer);
	
  self->priv->self = G_FILE_MONITOR (self);
}
