/*******************************************************************************
  Copyright (c) 2011, 2012 Dmitry Matveev <me@dmitrymatveev.co.uk>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*******************************************************************************/

#include "config.h"

#include "gkqueuedirectorymonitor.h"
#include "kqueue-helper.h"
#include "kqueue-exclusions.h"
#include <gio/gpollfilemonitor.h>
#include <gio/gfile.h>
#include <gio/giomodule.h>


struct _GKqueueDirectoryMonitor
{
  GLocalDirectoryMonitor parent_instance;
  kqueue_sub *sub;

  GFileMonitor *fallback;
  GFile *fbfile;
 
  gboolean pair_moves;
};

static gboolean g_kqueue_directory_monitor_cancel (GFileMonitor *monitor);

#define g_kqueue_directory_monitor_get_type _g_kqueue_directory_monitor_get_type
G_DEFINE_TYPE_WITH_CODE (GKqueueDirectoryMonitor, g_kqueue_directory_monitor, G_TYPE_LOCAL_DIRECTORY_MONITOR,
       g_io_extension_point_implement (G_LOCAL_DIRECTORY_MONITOR_EXTENSION_POINT_NAME,
               g_define_type_id,
               "kqueue",
               20))


static void
_fallback_callback (GFileMonitor      *unused,
                    GFile             *first,
                    GFile             *second,
                    GFileMonitorEvent  event,
                    gpointer           udata)
{
  GKqueueDirectoryMonitor *kq_mon = G_KQUEUE_DIRECTORY_MONITOR (udata); 
  GFileMonitor *mon = G_FILE_MONITOR (kq_mon);
  g_assert (kq_mon != NULL);
  g_assert (mon != NULL);
  (void) unused;

  if (event == G_FILE_MONITOR_EVENT_CHANGED)
  {
    _kh_dir_diff (kq_mon->sub, mon);
  }
  else
    g_file_monitor_emit_event (mon, first, second, event);
}


static void
g_kqueue_directory_monitor_finalize (GObject *object)
{
  GKqueueDirectoryMonitor *kqueue_monitor = G_KQUEUE_DIRECTORY_MONITOR (object);
  
  if (kqueue_monitor->sub)
    {
      _kh_cancel_sub (kqueue_monitor->sub);
      _kh_sub_free (kqueue_monitor->sub);
      kqueue_monitor->sub = NULL;
    }

  if (kqueue_monitor->fallback)
    g_object_unref (kqueue_monitor->fallback);

  if (kqueue_monitor->fbfile)
    g_object_unref (kqueue_monitor->fbfile);

  if (G_OBJECT_CLASS (g_kqueue_directory_monitor_parent_class)->finalize)
    (*G_OBJECT_CLASS (g_kqueue_directory_monitor_parent_class)->finalize) (object);
}

static GObject*
g_kqueue_directory_monitor_constructor (GType                 type,
                                        guint                 n_construct_properties,
                                        GObjectConstructParam *construct_properties)
{
  GObject *obj;
  GKqueueDirectoryMonitorClass *klass;
  GObjectClass *parent_class;
  GKqueueDirectoryMonitor *kqueue_monitor;
  kqueue_sub *sub = NULL;
  gboolean ret_kh_startup;
  const gchar *path = NULL;

  klass = G_KQUEUE_DIRECTORY_MONITOR_CLASS (g_type_class_peek (G_TYPE_KQUEUE_DIRECTORY_MONITOR));
  parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
  obj = parent_class->constructor (type,
                                   n_construct_properties,
                                   construct_properties);

  kqueue_monitor = G_KQUEUE_DIRECTORY_MONITOR (obj);

  ret_kh_startup = _kh_startup ();
  g_assert (ret_kh_startup);

  kqueue_monitor->pair_moves = (G_LOCAL_DIRECTORY_MONITOR (obj)->flags & G_FILE_MONITOR_SEND_MOVED)
                               ? TRUE : FALSE;

  kqueue_monitor->sub = NULL;
  kqueue_monitor->fallback = NULL;
  kqueue_monitor->fbfile = NULL;

  path = G_LOCAL_DIRECTORY_MONITOR (obj)->dirname;

  /* For a directory monitor, create a subscription object anyway.
   * It will be used for directory diff calculation routines. */

  sub = _kh_sub_new (path,
                     kqueue_monitor->pair_moves,
                     kqueue_monitor);

  /* FIXME: what to do about errors here? we can't return NULL or another
   * kind of error and an assertion is probably too hard (same issue as in
   * the inotify backend) */
  g_assert (sub != NULL);
  kqueue_monitor->sub = sub;

  if (!_ke_is_excluded (path))
    _kh_add_sub (sub);
  else
    {
      GFile *file = g_file_new_for_path (path);
      kqueue_monitor->fbfile = file;
      kqueue_monitor->fallback = _g_poll_file_monitor_new (file);
      g_signal_connect (kqueue_monitor->fallback,
                        "changed",
                        G_CALLBACK (_fallback_callback),
                        kqueue_monitor);
    }

  return obj;
}

static gboolean
g_kqueue_directory_monitor_is_supported (void)
{
  return _kh_startup ();
}

static void
g_kqueue_directory_monitor_class_init (GKqueueDirectoryMonitorClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GFileMonitorClass *directory_monitor_class = G_FILE_MONITOR_CLASS (klass);
  GLocalDirectoryMonitorClass *local_directory_monitor_class = G_LOCAL_DIRECTORY_MONITOR_CLASS (klass);

  gobject_class->finalize = g_kqueue_directory_monitor_finalize;
  gobject_class->constructor = g_kqueue_directory_monitor_constructor;
  directory_monitor_class->cancel = g_kqueue_directory_monitor_cancel;

  local_directory_monitor_class->mount_notify = TRUE; /* TODO: ??? */
  local_directory_monitor_class->is_supported = g_kqueue_directory_monitor_is_supported;
}

static void
g_kqueue_directory_monitor_init (GKqueueDirectoryMonitor *monitor)
{
}

static gboolean
g_kqueue_directory_monitor_cancel (GFileMonitor *monitor)
{
  GKqueueDirectoryMonitor *kqueue_monitor = G_KQUEUE_DIRECTORY_MONITOR (monitor);

  if (kqueue_monitor->sub)
    {
      _kh_cancel_sub (kqueue_monitor->sub);
      _kh_sub_free (kqueue_monitor->sub);
      kqueue_monitor->sub = NULL;
    }
  else if (kqueue_monitor->fallback)
    g_file_monitor_cancel (kqueue_monitor->fallback);


  if (G_FILE_MONITOR_CLASS (g_kqueue_directory_monitor_parent_class)->cancel)
    (*G_FILE_MONITOR_CLASS (g_kqueue_directory_monitor_parent_class)->cancel) (monitor);

  return TRUE;
}
