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

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include <glib-object.h>
#include <glib/gfileutils.h>
#include <gio/gfilemonitor.h>
#include <gio/glocalfilemonitor.h>
#include <gio/giomodule.h>
#include <gio/gpollfilemonitor.h>
#include <gio/gfile.h>
#include <glib-unix.h>
#include "glib-private.h"

#include "kqueue-helper.h"
#include "dep-list.h"

G_LOCK_DEFINE_STATIC (kq_lock);
static GSource       *kq_source;
static int	      kq_queue = -1;

#define G_TYPE_KQUEUE_FILE_MONITOR	(g_kqueue_file_monitor_get_type ())
#define G_KQUEUE_FILE_MONITOR(inst)	(G_TYPE_CHECK_INSTANCE_CAST ((inst), \
					G_TYPE_KQUEUE_FILE_MONITOR, GKqueueFileMonitor))

/* C11 allows type redefinition, but GLib is configured to use C89, which causes
 * clang to show warnings when we use a C11 feature. Since the C89 requirement
 * is mostly used to support MSVC, we simply ignore the warning here because
 * this file is never going to be useful on Windows. */
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtypedef-redefinition"
#endif

typedef GLocalFileMonitorClass GKqueueFileMonitorClass;

/* When the file we are monitoring is a directory, sub_dir is subscribed to the
 * directory itself and sub_file is NULL.
 *
 * When the file we are monitoring is a regular file, sub_dir is subscribed to
 * the directory containing the file and sub_file is subscribed to the file
 * being monitored. We have to monitor both because it is possible that the
 * file chosen for monitoring doesn't exist when the file monitor is started.
 * We monitor on its parent in order to get notification when it is created.
 *
 * To distinguish between a directory monitor and a regular file monitor, check
 * whether sub_file is NULL. */
struct _GKqueueFileMonitor
{
  GLocalFileMonitor parent_instance;

  kqueue_sub *sub_dir;
  kqueue_sub *sub_file;
#ifndef O_EVTONLY
  GFileMonitor *fallback;
  GFile *fbfile;
#endif
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

GType g_kqueue_file_monitor_get_type (void);
G_DEFINE_TYPE_WITH_CODE (GKqueueFileMonitor, g_kqueue_file_monitor, G_TYPE_LOCAL_FILE_MONITOR,
	g_io_extension_point_implement (G_LOCAL_FILE_MONITOR_EXTENSION_POINT_NAME,
		g_define_type_id,
                "kqueue",
		20))

#ifndef O_EVTONLY
#define O_KQFLAG O_RDONLY
#else
#define O_KQFLAG O_EVTONLY
#endif

static inline unsigned int
note_all (void)
{
  unsigned int notes = NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_RENAME;
#ifdef NOTE_TRUNCATE
  notes |= NOTE_TRUNCATE;
#endif
#ifdef NOTE_CLOSE_WRITE
  notes |= NOTE_CLOSE_WRITE;
#endif
  return notes;
}

static gboolean g_kqueue_file_monitor_cancel (GFileMonitor* monitor);
static gboolean g_kqueue_file_monitor_is_supported (void);

static kqueue_sub	*_kqsub_new (gchar *, gchar *, GKqueueFileMonitor *, GFileMonitorSource *);
static void		 _kqsub_free (kqueue_sub *);
static gboolean		 _kqsub_cancel (kqueue_sub *);


#ifndef O_EVTONLY
static void
_fallback_callback (GFileMonitor      *unused,
                    GFile             *first,
                    GFile             *second,
                    GFileMonitorEvent  event,
                    gpointer           udata)
{
  GKqueueFileMonitor *kq_mon = G_KQUEUE_FILE_MONITOR (udata);

  g_file_monitor_emit_event (G_FILE_MONITOR (kq_mon), first, second, event);
}

/*
 * _ke_is_excluded:
 * @full_path - a path to file to check.
 *
 * Returns: TRUE if the file should be excluded from the kqueue-powered
 *      monitoring, FALSE otherwise.
 **/
static gboolean
_ke_is_excluded (const char *full_path)
{
  GFile *f = NULL;
  GMount *mount = NULL;

  f = g_file_new_for_path (full_path);

  if (f != NULL) {
    mount = g_file_find_enclosing_mount (f, NULL, NULL);
    g_object_unref (f);
  }

  if (mount != NULL && (g_str_has_prefix (full_path, "/media/") || g_str_has_prefix (full_path, "/run/media/")))
  {
    g_warning ("Excluding %s from kernel notification, falling back to poll", full_path);
    if (mount)
      g_object_unref (mount);
    return TRUE;
  }

  return FALSE;
}
#endif /* !O_EVTONLY */

static void
g_kqueue_file_monitor_finalize (GObject *object)
{
  GKqueueFileMonitor *kqueue_monitor = G_KQUEUE_FILE_MONITOR (object);

  if (kqueue_monitor->sub_dir)
    {
      _kqsub_cancel (kqueue_monitor->sub_dir);
      _kqsub_free (kqueue_monitor->sub_dir);
      kqueue_monitor->sub_dir = NULL;
    }

  if (kqueue_monitor->sub_file)
    {
      _kqsub_cancel (kqueue_monitor->sub_file);
      _kqsub_free (kqueue_monitor->sub_file);
      kqueue_monitor->sub_file = NULL;
    }

#ifndef O_EVTONLY
  if (kqueue_monitor->fallback)
    g_object_unref (kqueue_monitor->fallback);

  if (kqueue_monitor->fbfile)
    g_object_unref (kqueue_monitor->fbfile);
#endif

  if (G_OBJECT_CLASS (g_kqueue_file_monitor_parent_class)->finalize)
    (*G_OBJECT_CLASS (g_kqueue_file_monitor_parent_class)->finalize) (object);
}

static void
g_kqueue_file_monitor_start (GLocalFileMonitor *local_monitor,
                             const gchar *dirname,
                             const gchar *basename,
                             const gchar *filename,
                             GFileMonitorSource *source)
{
  GKqueueFileMonitor *kqueue_monitor = G_KQUEUE_FILE_MONITOR (local_monitor);
  kqueue_sub *sub_dir = NULL, *sub_file = NULL;
  gchar *path_dir, *path_file, *file_basename;

  /* There are three possible cases here:
   *
   *  1. Directory: dirname != NULL, basename == NULL, filename == NULL
   *  2. Regular file: dirname != NULL, basename != NULL, filename == NULL
   *  3. Hard links: dirname == NULL, basename == NULL, filename != NULL
   *
   *  Note that we don't distinguish between case 2 and 3. Kqueue monitors
   *  files based on file descriptors, so we always receive events come from
   *  hard links.
   */
  if (filename != NULL)
    {
      path_dir = g_path_get_dirname (filename);
      path_file = g_strdup (filename);
      file_basename = g_path_get_basename (filename);
    }
  else
    {
      path_dir = g_strdup (dirname);
      if (basename != NULL)
        {
          path_file = g_build_filename (dirname, basename, NULL);
          file_basename = g_strdup (basename);
        }
      else
        {
          path_file = NULL;
          file_basename = NULL;
        }
    }

#ifndef O_EVTONLY
  if (_ke_is_excluded (path_dir))
    {
      GFile *file;
      if (path_file != NULL)
        file = g_file_new_for_path (path_file);
      else
        file = g_file_new_for_path (path_dir);
      g_free (path_dir);
      g_free (path_file);
      g_free (file_basename);
      kqueue_monitor->fbfile = file;
      kqueue_monitor->fallback = _g_poll_file_monitor_new (file);
      g_signal_connect (kqueue_monitor->fallback, "changed",
			G_CALLBACK (_fallback_callback), kqueue_monitor);
      return;
    }
#endif

  /* For a directory monitor, create a subscription object anyway.
   * It will be used for directory diff calculation routines. 
   * Wait, directory diff in a GKqueueFileMonitor?
   * Yes, it is. When a file monitor is started on an non-existent
   * file, GIO uses a GKqueueFileMonitor object for that. If a directory
   * will be created under that path, GKqueueFileMonitor will have to
   * handle the directory notifications. */
  sub_dir = _kqsub_new (g_steal_pointer (&path_dir), NULL,
                        kqueue_monitor, source);
  if (!_kqsub_start_watching (sub_dir))
    _km_add_missing (sub_dir);

  /* Unlike GInotifyFileMonitor, which always uses a directory monitor
   * regardless of the type of the file being monitored, kqueue doesn't
   * give us events generated by files under it when we are monitoring
   * a directory. We have to monitor the file itself to know changes which
   * was made to the file itself. */
  if (path_file != NULL)
    {
      sub_file = _kqsub_new (g_steal_pointer (&path_file),
                             g_steal_pointer (&file_basename),
                             kqueue_monitor, source);
      if (!_kqsub_start_watching (sub_file))
        _km_add_missing (sub_file);
    }

  kqueue_monitor->sub_dir = sub_dir;
  kqueue_monitor->sub_file = sub_file;
  g_clear_pointer (&path_dir, g_free);
  g_clear_pointer (&path_file, g_free);
  g_clear_pointer (&file_basename, g_free);
}

static void
g_kqueue_file_monitor_class_init (GKqueueFileMonitorClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GFileMonitorClass *file_monitor_class = G_FILE_MONITOR_CLASS (klass);
  GLocalFileMonitorClass *local_file_monitor_class = G_LOCAL_FILE_MONITOR_CLASS (klass);

  gobject_class->finalize = g_kqueue_file_monitor_finalize;
  file_monitor_class->cancel = g_kqueue_file_monitor_cancel;

  local_file_monitor_class->is_supported = g_kqueue_file_monitor_is_supported;
  local_file_monitor_class->start = g_kqueue_file_monitor_start;
  local_file_monitor_class->mount_notify = TRUE; /* TODO: ??? */
}

static void
g_kqueue_file_monitor_init (GKqueueFileMonitor *monitor)
{
}

static gboolean
g_kqueue_file_monitor_callback (gint fd, GIOCondition condition, gpointer user_data)
{
  gint64 now = g_source_get_time (kq_source);
  kqueue_sub *sub;
  GFileMonitorSource *source;
  struct kevent ev;
  struct timespec ts;

  memset (&ts, 0, sizeof(ts));

  /* We must hold the global lock before accessing any kqueue_sub because it is
   * possible for other threads to call g_kqueue_file_monitor_cancel, which may
   * free the kqueue_sub struct we are accessing. */
  G_LOCK (kq_lock);

  while (kevent(fd, NULL, 0, &ev, 1, &ts) > 0)
    {
        if (ev.filter != EVFILT_VNODE || ev.udata == NULL)
          continue;

        sub = ev.udata;
        source = sub->source;

        /* When we are monitoring a regular file which already exists, ignore
         * events generated by its parent directory. This has to be the first
         * check to prevent the following code to emit useless events */
        if (sub->is_dir && sub->mon->sub_file != NULL && sub->mon->sub_file->fd != -1)
          continue;

        if (ev.flags & EV_ERROR)
          ev.fflags = NOTE_REVOKE;

        if (sub->is_dir && ev.fflags & (NOTE_WRITE | NOTE_EXTEND))
          {
            /* If we are monitoring on a non-existent regular file, trigger the
             * rescan of missing files immediately so we don't have to wait for
             * 4 seconds for discovering missing files. We pass the sub_file
             * corresponding to the GKqueueFileMonitor to 'check_this_sub_only'
             * argument to prevent _km_scan_missing from emiting 'CREATED'
             * events because _kh_dir_diff will do it for us. */
            if (sub->mon->sub_file != NULL && sub->mon->sub_file->fd == -1)
              _km_scan_missing (sub->mon->sub_file);

            /* If we are monitoring a regular file, don't emit 'DELETED' events
             * from the directory monitor because it will be emitted from the
             * file itself when a NOTE_DELETE is reported on sub_file. */
            _kh_dir_diff (sub, sub->mon->sub_file == NULL);

#ifdef NOTE_TRUNCATE
            ev.fflags &= ~(NOTE_WRITE | NOTE_EXTEND | NOTE_TRUNCATE);
#else
            ev.fflags &= ~(NOTE_WRITE | NOTE_EXTEND);
#endif
          }

        /* Here starts the long section of mapping kqueue events to
         * GFileMonitorEvent. Since kqueue can return multiple events in a
         * single kevent struct, we must use 'if' instead of 'else if'. */
        if (ev.fflags & NOTE_DELETE)
          {
            struct stat st;
            if (fstat (sub->fd, &st) < 0)
              st.st_nlink = 0;

            g_file_monitor_source_handle_event (source,
                                                G_FILE_MONITOR_EVENT_DELETED,
                                                sub->basename, NULL, NULL, now);

            /* If the last reference to the file was removed, delete the
             * subscription from kqueue and add it to the missing list.
             * If you are monitoring a file which has hard link count higher
             * than 1, it is possible for the same file to emit 'DELETED'
             * events multiple times. */
            if (st.st_nlink == 0)
              {
                _kqsub_cancel (sub);
                _km_add_missing (sub);
              }
          }
         if (ev.fflags & NOTE_REVOKE)
           {
             g_file_monitor_source_handle_event (source,
                                                 G_FILE_MONITOR_EVENT_UNMOUNTED,
                                                 sub->basename, NULL, NULL, now);
             _kqsub_cancel (sub);
             _km_add_missing (sub);
           }
        if (ev.fflags & NOTE_ATTRIB)
          {
            g_file_monitor_source_handle_event (source,
                                                G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED,
                                                sub->basename, NULL, NULL, now);
          }
#ifdef NOTE_TRUNCATE
        if (ev.fflags & (NOTE_WRITE | NOTE_EXTEND | NOTE_TRUNCATE))
#else
        if (ev.fflags & (NOTE_WRITE | NOTE_EXTEND))
#endif
          {
            g_file_monitor_source_handle_event (source,
                                                G_FILE_MONITOR_EVENT_CHANGED,
                                                sub->basename, NULL, NULL, now);
          }
        if (ev.fflags & NOTE_RENAME)
          {
            /* Since there’s apparently no way to get the new name of the
             * file out of kqueue(), all we can do is say that this one has
             * been deleted. */
            g_file_monitor_source_handle_event (source,
                                                G_FILE_MONITOR_EVENT_DELETED,
                                                sub->basename, NULL, NULL, now);
          }
#ifdef NOTE_CLOSE_WRITE
        if (ev.fflags & NOTE_CLOSE_WRITE)
          {
            g_file_monitor_source_handle_event (source,
                                                G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT,
                                                sub->basename, NULL, NULL, now);
          }
#endif

        /* Handle the case when a file is created again shortly after it was
         * deleted. It has to be the last check because 'DELETED' must happen
         * before 'CREATED'. */
        if (ev.fflags & (NOTE_DELETE | NOTE_REVOKE))
          _km_scan_missing (NULL);
    }

  G_UNLOCK (kq_lock);

  return TRUE;
}

static gboolean
g_kqueue_file_monitor_is_supported (void)
{
  int errsv;

  G_LOCK (kq_lock);

  if (kq_queue == -1)
    {
      kq_queue = kqueue ();
      errsv = errno;

      if (kq_queue == -1)
        {
          g_warning ("Unable to create a kqueue: %s", g_strerror (errsv));
          G_UNLOCK (kq_lock);
          return FALSE;
        }

      kq_source = g_unix_fd_source_new (kq_queue, G_IO_IN);
      g_source_set_callback (kq_source, (GSourceFunc) g_kqueue_file_monitor_callback, NULL, NULL);
      g_source_attach (kq_source, GLIB_PRIVATE_CALL (g_get_worker_context) ());
    }

  G_UNLOCK (kq_lock);

  return TRUE;
}

static gboolean
g_kqueue_file_monitor_cancel (GFileMonitor *monitor)
{
  GKqueueFileMonitor *kqueue_monitor = G_KQUEUE_FILE_MONITOR (monitor);

  /* We must hold the global lock before calling _kqsub_cancel. However, we
   * cannot call G_LOCK in _kqsub_cancel because it is also used by
   * g_kqueue_file_monitor_callback, which already holds the lock itself. */
  G_LOCK (kq_lock);

  if (kqueue_monitor->sub_dir)
    {
      _kqsub_cancel (kqueue_monitor->sub_dir);
      _kqsub_free (kqueue_monitor->sub_dir);
      kqueue_monitor->sub_dir = NULL;
    }
  if (kqueue_monitor->sub_file)
    {
      _kqsub_cancel (kqueue_monitor->sub_file);
      _kqsub_free (kqueue_monitor->sub_file);
      kqueue_monitor->sub_file = NULL;
    }

  G_UNLOCK (kq_lock);

#ifndef O_EVTONLY
  if (kqueue_monitor->fallback)
    {
      g_signal_handlers_disconnect_by_func (kqueue_monitor->fallback, _fallback_callback, kqueue_monitor);
      g_file_monitor_cancel (kqueue_monitor->fallback);
    }
#endif

  if (G_FILE_MONITOR_CLASS (g_kqueue_file_monitor_parent_class)->cancel)
    (*G_FILE_MONITOR_CLASS (g_kqueue_file_monitor_parent_class)->cancel) (monitor);

  return TRUE;
}

static kqueue_sub *
_kqsub_new (gchar *filename, gchar *basename, GKqueueFileMonitor *mon, GFileMonitorSource *source)
{
  kqueue_sub *sub;

  sub = g_slice_new (kqueue_sub);
  sub->filename = filename;
  sub->basename = basename;
  sub->mon = mon;
  g_source_ref ((GSource *) source);
  sub->source = source;
  sub->fd = -1;
  sub->deps = NULL;
  sub->is_dir = 0;

  return sub;
}

static void
_kqsub_free (kqueue_sub *sub)
{
  g_assert (sub->deps == NULL);
  g_assert (sub->fd == -1);

  g_source_unref ((GSource *) sub->source);
  g_free (sub->filename);
  g_free (sub->basename);
  g_slice_free (kqueue_sub, sub);
}

static gboolean
_kqsub_cancel (kqueue_sub *sub)
{
  /* WARNING: Before calling this function, you must hold a lock on kq_lock
   * or you will cause use-after-free in g_kqueue_file_monitor_callback. */

  struct kevent ev;

  /* Remove the event and close the file descriptor to automatically
   * delete pending events. */
  if (sub->fd != -1)
    {
      EV_SET (&ev, sub->fd, EVFILT_VNODE, EV_DELETE, note_all (), 0, sub);
      if (kevent (kq_queue, &ev, 1, NULL, 0, NULL) == -1)
        {
          g_warning ("Unable to remove event for %s: %s", sub->filename, g_strerror (errno));
          return FALSE;
        }
      close (sub->fd);
      sub->fd = -1;
    }

  _km_remove (sub);

  if (sub->deps)
    {
      dl_free (sub->deps);
      sub->deps = NULL;
    }

  return TRUE;
}

gboolean
_kqsub_start_watching (kqueue_sub *sub)
{
  struct stat st;
  struct kevent ev;

  sub->fd = open (sub->filename, O_KQFLAG);
  if (sub->fd == -1)
      return FALSE;

  if (fstat (sub->fd, &st) == -1)
    {
      g_warning ("fstat failed for %s: %s", sub->filename, g_strerror (errno));
      close (sub->fd);
      sub->fd = -1;
      return FALSE;
    }

  sub->is_dir = (st.st_mode & S_IFDIR) ? 1 : 0;
  if (sub->is_dir)
    {
      if (sub->deps)
        dl_free (sub->deps);

      sub->deps = dl_listing (sub->filename);
    }

  EV_SET (&ev, sub->fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, note_all (), 0, sub);
  if (kevent (kq_queue, &ev, 1, NULL, 0, NULL) == -1)
    {
      g_warning ("Unable to add event for %s: %s", sub->filename, g_strerror (errno));
      close (sub->fd);
      sub->fd = -1;
      return FALSE;
    }

  return TRUE;
}
