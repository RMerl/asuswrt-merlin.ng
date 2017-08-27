/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 8 -*- */

/* inotify-helper.c - GVFS Monitor based on inotify.

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

   Authors: 
		 John McCutchan <john@johnmccutchan.com>
*/

#include "config.h"
#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
/* Just include the local header to stop all the pain */
#include <sys/inotify.h>
#include <gio/glocalfile.h>
#include <gio/gfilemonitor.h>
#include <gio/gfile.h>
#include "inotify-helper.h"
#include "inotify-missing.h"
#include "inotify-path.h"

static gboolean ih_debug_enabled = FALSE;
#define IH_W if (ih_debug_enabled) g_warning 

static void ih_event_callback (ik_event_t  *event,
			       inotify_sub *sub,
			       gboolean     file_event);
static void ih_not_missing_callback (inotify_sub *sub);

/* We share this lock with inotify-kernel.c and inotify-missing.c
 *
 * inotify-kernel.c takes the lock when it reads events from
 * the kernel and when it processes those events
 *
 * inotify-missing.c takes the lock when it is scanning the missing
 * list.
 *
 * We take the lock in all public functions
 */
G_LOCK_DEFINE (inotify_lock);

static GFileMonitorEvent ih_mask_to_EventFlags (guint32 mask);

/**
 * _ih_startup:
 *
 * Initializes the inotify backend.  This must be called before
 * any other functions in this module.
 *
 * Return value: #TRUE if initialization succeeded, #FALSE otherwise
 */
gboolean
_ih_startup (void)
{
  static gboolean initialized = FALSE;
  static gboolean result = FALSE;
  
  G_LOCK (inotify_lock);
  
  if (initialized == TRUE)
    {
      G_UNLOCK (inotify_lock);
      return result;
    }

  result = _ip_startup (ih_event_callback);
  if (!result)
    {
      G_UNLOCK (inotify_lock);
      return FALSE;
    }
  _im_startup (ih_not_missing_callback);

  IH_W ("started gvfs inotify backend\n");
  
  initialized = TRUE;
  
  G_UNLOCK (inotify_lock);
  
  return TRUE;
}

/*
 * Adds a subscription to be monitored.
 */
gboolean
_ih_sub_add (inotify_sub *sub)
{
  G_LOCK (inotify_lock);
	
  if (!_ip_start_watching (sub))
    _im_add (sub);
  
  G_UNLOCK (inotify_lock);
  return TRUE;
}

/*
 * Cancels a subscription which was being monitored.
 */
gboolean
_ih_sub_cancel (inotify_sub *sub)
{
  G_LOCK (inotify_lock);

  if (!sub->cancelled)
    {
      IH_W ("cancelling %s\n", sub->dirname);
      sub->cancelled = TRUE;
      _im_rm (sub);
      _ip_stop_watching (sub);
    }
  
  G_UNLOCK (inotify_lock);
  
  return TRUE;
}

static char *
_ih_fullpath_from_event (ik_event_t *event,
			 const char *dirname,
			 const char *filename)
{
  char *fullpath;

  if (filename)
    fullpath = g_strdup_printf ("%s/%s", dirname, filename);
  else if (event->name)
    fullpath = g_strdup_printf ("%s/%s", dirname, event->name);
  else
    fullpath = g_strdup_printf ("%s/", dirname);

   return fullpath;
}


static gboolean
ih_event_is_paired_move (ik_event_t *event)
{
  if (event->pair)
    {
      ik_event_t *paired = event->pair;
      /* intofiy(7): IN_MOVE == IN_MOVED_FROM | IN_MOVED_TO */
      return (event->mask | paired->mask) & IN_MOVE;
    }

    return FALSE;
}

static void
ih_event_callback (ik_event_t  *event, 
                   inotify_sub *sub,
		   gboolean     file_event)
{
  gchar *fullpath;
  GFileMonitorEvent eflags;
  GFile* child;
  GFile* other;

  eflags = ih_mask_to_EventFlags (event->mask);
  fullpath = _ih_fullpath_from_event (event, sub->dirname,
				      file_event ? sub->filename : NULL);
  child = g_file_new_for_path (fullpath);
  g_free (fullpath);

  if (ih_event_is_paired_move (event) && sub->pair_moves)
    {
      const char *parent_dir = (char *) _ip_get_path_for_wd (event->pair->wd);
      fullpath = _ih_fullpath_from_event (event->pair, parent_dir, NULL);
      other = g_file_new_for_path (fullpath);
      g_free (fullpath);
      eflags = G_FILE_MONITOR_EVENT_MOVED;
      event->pair = NULL; /* prevents the paired event to be emitted as well */
    }
  else
    other = NULL;

  g_file_monitor_emit_event (G_FILE_MONITOR (sub->user_data),
			     child, other, eflags);

  /* For paired moves or moves whose mask has been changed from IN_MOVED_TO to
   * IN_CREATE, notify also that it's probably the last change to the file,
   * emitting CHANGES_DONE_HINT.
   * The first (first part of the if's guard below) is the case of a normal
   * move within the monitored tree and in the same mounted volume.
   * The latter (second part of the guard) is the case of a move within the
   * same mounted volume, but from a not monitored directory.
   *
   * It's not needed in cases like moves across mounted volumes as the IN_CREATE
   * will be followed by a IN_MODIFY and IN_CLOSE_WRITE events.
   * Also not needed if sub->pair_moves is set as EVENT_MOVED will be emitted
   * instead of EVENT_CREATED which implies no further modification will be
   * applied to the file
   * See: https://bugzilla.gnome.org/show_bug.cgi?id=640077
   */
  if ((!sub->pair_moves &&
        event->is_second_in_pair && (event->mask & IN_MOVED_TO)) ||
      (!ih_event_is_paired_move (event) &&
       (event->original_mask & IN_MOVED_TO) && (event->mask & IN_CREATE)))
    {
      g_file_monitor_emit_event (G_FILE_MONITOR (sub->user_data),
          child, NULL, G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT);
    }

  g_object_unref (child);
  if (other)
    g_object_unref (other);
}

static void
ih_not_missing_callback (inotify_sub *sub)
{
  gchar *fullpath;
  GFileMonitorEvent eflags;
  guint32 mask;
  GFile* child;

  if (sub->filename)
    {
      fullpath = g_strdup_printf ("%s/%s", sub->dirname, sub->filename);
      g_warning ("Missing callback called fullpath = %s\n", fullpath);
      if (!g_file_test (fullpath, G_FILE_TEST_EXISTS))
	{
	  g_free (fullpath);
	  return;
	}
      mask = IN_CREATE;
    }
  else
    {
      fullpath = g_strdup_printf ("%s", sub->dirname);
      mask = IN_CREATE|IN_ISDIR;
    }

  eflags = ih_mask_to_EventFlags (mask);
  child = g_file_new_for_path (fullpath);
  g_free (fullpath);

  g_file_monitor_emit_event (G_FILE_MONITOR (sub->user_data),
			     child, NULL, eflags);

  g_object_unref (child);
}

/* Transforms a inotify event to a GVFS event. */
static GFileMonitorEvent
ih_mask_to_EventFlags (guint32 mask)
{
  mask &= ~IN_ISDIR;
  switch (mask)
    {
    case IN_MODIFY:
      return G_FILE_MONITOR_EVENT_CHANGED;
    case IN_CLOSE_WRITE:
      return G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT;
    case IN_ATTRIB:
      return G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED;
    case IN_MOVE_SELF:
    case IN_MOVED_FROM:
    case IN_DELETE:
    case IN_DELETE_SELF:
      return G_FILE_MONITOR_EVENT_DELETED;
    case IN_CREATE:
    case IN_MOVED_TO:
      return G_FILE_MONITOR_EVENT_CREATED;
    case IN_UNMOUNT:
      return G_FILE_MONITOR_EVENT_UNMOUNTED;
    case IN_Q_OVERFLOW:
    case IN_OPEN:
    case IN_CLOSE_NOWRITE:
    case IN_ACCESS:
    case IN_IGNORED:
    default:
      return -1;
    }
}
