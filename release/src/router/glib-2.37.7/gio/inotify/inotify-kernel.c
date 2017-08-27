/*
   Copyright (C) 2005 John McCutchan

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

   Authors:.
		John McCutchan <john@johnmccutchan.com>
*/

#include "config.h"

#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <glib.h>
#include "inotify-kernel.h"
#include <sys/inotify.h>

#include "glib-private.h"

/* Timings for pairing MOVED_TO / MOVED_FROM events */
#define PROCESS_EVENTS_TIME 1000 /* 1000 milliseconds (1 hz) */
#define DEFAULT_HOLD_UNTIL_TIME 0 /* 0 millisecond */
#define MOVE_HOLD_UNTIL_TIME 500 /* 500 microseconds or 0.5 milliseconds */

static int inotify_instance_fd = -1;
static GQueue *events_to_process = NULL;
static GQueue *event_queue = NULL;
static GHashTable * cookie_hash = NULL;
static GIOChannel *inotify_read_ioc;
static GPollFD ik_poll_fd;
static gboolean ik_poll_fd_enabled = TRUE;
static void (*user_cb)(ik_event_t *event);

static gboolean ik_read_callback (gpointer user_data);
static gboolean ik_process_eq_callback (gpointer user_data);

static guint32 ik_move_matches = 0;
static guint32 ik_move_misses = 0;

static gboolean process_eq_running = FALSE;

/* We use the lock from inotify-helper.c
 *
 * There are two places that we take this lock
 *
 * 1) In ik_read_callback
 *
 * 2) ik_process_eq_callback.
 *
 *
 * The rest of locking is taken care of in inotify-helper.c
 */
G_LOCK_EXTERN (inotify_lock);

typedef struct ik_event_internal {
  ik_event_t *event;
  gboolean seen;
  gboolean sent;
  GTimeVal hold_until;
  struct ik_event_internal *pair;
} ik_event_internal_t;

/* In order to perform non-sleeping inotify event chunking we need
 * a custom GSource
 */
static gboolean
ik_source_prepare (GSource *source,
		   gint    *timeout)
{
  return FALSE;
}

static gboolean
ik_source_timeout (gpointer data)
{
  GSource *source = (GSource *)data;
  
  /* Re-active the PollFD */
  g_source_add_poll (source, &ik_poll_fd);
  g_source_unref (source);
  ik_poll_fd_enabled = TRUE;
  
  return FALSE;
}

#define MAX_PENDING_COUNT 2
#define PENDING_THRESHOLD(qsize) ((qsize) >> 1)
#define PENDING_MARGINAL_COST(p) ((unsigned int)(1 << (p)))
#define MAX_QUEUED_EVENTS 2048
#define AVERAGE_EVENT_SIZE sizeof (struct inotify_event) + 16
#define TIMEOUT_MILLISECONDS 10

static gboolean
ik_source_check (GSource *source)
{
  static int prev_pending = 0, pending_count = 0;
  
  /* We already disabled the PollFD or
   * nothing to be read from inotify */
  if (!ik_poll_fd_enabled || !(ik_poll_fd.revents & G_IO_IN))
    return FALSE;

  if (pending_count < MAX_PENDING_COUNT)
    {
      GSource *timeout_source;
      unsigned int pending;
      
      if (ioctl (inotify_instance_fd, FIONREAD, &pending) == -1)
	goto do_read;
      
      pending /= AVERAGE_EVENT_SIZE;
      
      /* Don't wait if the number of pending events is too close
       * to the maximum queue size.
       */
      if (pending > PENDING_THRESHOLD (MAX_QUEUED_EVENTS))
	goto do_read;
      
      /* With each successive iteration, the minimum rate for
       * further sleep doubles. 
       */
      if (pending-prev_pending < PENDING_MARGINAL_COST (pending_count))
	goto do_read;
      
      prev_pending = pending;
      pending_count++;
      
      /* We are going to wait to read the events: */
      
      /* Remove the PollFD from the source */
      g_source_remove_poll (source, &ik_poll_fd);
      /* To avoid threading issues we need to flag that we've done that */
      ik_poll_fd_enabled = FALSE;
      /* Set a timeout to re-add the PollFD to the source */
      g_source_ref (source);

      timeout_source = g_timeout_source_new (TIMEOUT_MILLISECONDS);
      g_source_set_callback (timeout_source, ik_source_timeout, source, NULL);
      g_source_attach (timeout_source, GLIB_PRIVATE_CALL (g_get_worker_context) ());
      g_source_unref (timeout_source);

      return FALSE;
    }

do_read:
  /* We are ready to read events from inotify */

  prev_pending = 0;
  pending_count = 0;
  
  return TRUE;
}

static gboolean
ik_source_dispatch (GSource     *source,
		    GSourceFunc  callback,
		    gpointer     user_data)
{
  if (callback)
    return callback (user_data);
  return TRUE;
}

static GSourceFuncs ik_source_funcs =
{
  ik_source_prepare,
  ik_source_check,
  ik_source_dispatch,
  NULL
};

gboolean _ik_startup (void (*cb)(ik_event_t *event))
{
  static gboolean initialized = FALSE;
  GSource *source;
  
  user_cb = cb;
  /* Ignore multi-calls */
  if (initialized) 
    return inotify_instance_fd >= 0;

  initialized = TRUE;

#ifdef HAVE_INOTIFY_INIT1
  inotify_instance_fd = inotify_init1 (IN_CLOEXEC);
#else
  inotify_instance_fd = -1;
#endif
  if (inotify_instance_fd < 0)
    inotify_instance_fd = inotify_init ();

  if (inotify_instance_fd < 0)
    return FALSE;

  inotify_read_ioc = g_io_channel_unix_new (inotify_instance_fd);
  ik_poll_fd.fd = inotify_instance_fd;
  ik_poll_fd.events = G_IO_IN | G_IO_HUP | G_IO_ERR;
  g_io_channel_set_encoding (inotify_read_ioc, NULL, NULL);
  g_io_channel_set_flags (inotify_read_ioc, G_IO_FLAG_NONBLOCK, NULL);

  source = g_source_new (&ik_source_funcs, sizeof (GSource));
  g_source_set_name (source, "GIO Inotify");
  g_source_add_poll (source, &ik_poll_fd);
  g_source_set_callback (source, ik_read_callback, NULL, NULL);
  g_source_attach (source, GLIB_PRIVATE_CALL (g_get_worker_context) ());
  g_source_unref (source);

  cookie_hash = g_hash_table_new (g_direct_hash, g_direct_equal);
  event_queue = g_queue_new ();
  events_to_process = g_queue_new ();
  
  return TRUE;
}

static ik_event_internal_t *
ik_event_internal_new (ik_event_t *event)
{
  ik_event_internal_t *internal_event = g_new0 (ik_event_internal_t, 1);
  GTimeVal tv;
  
  g_assert (event);
  
  g_get_current_time (&tv);
  g_time_val_add (&tv, DEFAULT_HOLD_UNTIL_TIME);
  internal_event->event = event;
  internal_event->hold_until = tv;
  
  return internal_event;
}

static ik_event_t *
ik_event_new (char *buffer)
{
  struct inotify_event *kevent = (struct inotify_event *)buffer;
  ik_event_t *event = g_new0 (ik_event_t, 1);
  
  g_assert (buffer);
  
  event->wd = kevent->wd;
  event->mask = kevent->mask;
  event->cookie = kevent->cookie;
  event->len = kevent->len;
  if (event->len)
    event->name = g_strdup (kevent->name);
  else
    event->name = g_strdup ("");
  
  return event;
}

void
_ik_event_free (ik_event_t *event)
{
  if (event->pair)
    _ik_event_free (event->pair);
  g_free (event->name);
  g_free (event);
}

gint32
_ik_watch (const char *path, 
           guint32     mask, 
           int        *err)
{
  gint32 wd = -1;
  
  g_assert (path != NULL);
  g_assert (inotify_instance_fd >= 0);
  
  wd = inotify_add_watch (inotify_instance_fd, path, mask);
  
  if (wd < 0)
    {
      int e = errno;
      /* FIXME: debug msg failed to add watch */
      if (err)
	*err = e;
      return wd;
    }
  
  g_assert (wd >= 0);
  return wd;
}

int
_ik_ignore (const char *path, 
            gint32      wd)
{
  g_assert (wd >= 0);
  g_assert (inotify_instance_fd >= 0);
  
  if (inotify_rm_watch (inotify_instance_fd, wd) < 0)
    {
      /* int e = errno; */
      /* failed to rm watch */
      return -1;
    }
  
  return 0;
}

static void
ik_read_events (gsize  *buffer_size_out, 
                gchar **buffer_out)
{
  static gchar *buffer = NULL;
  static gsize buffer_size;
  
  /* Initialize the buffer on our first call */
  if (buffer == NULL)
    {
      buffer_size = AVERAGE_EVENT_SIZE;
      buffer_size *= MAX_QUEUED_EVENTS;
      buffer = g_malloc (buffer_size);
    }

  *buffer_size_out = 0;
  *buffer_out = NULL;
  
  memset (buffer, 0, buffer_size);

  if (g_io_channel_read_chars (inotify_read_ioc, (char *)buffer, buffer_size, buffer_size_out, NULL) != G_IO_STATUS_NORMAL) {
    /* error reading */
  }
  *buffer_out = buffer;
}

static gboolean
ik_read_callback (gpointer user_data)
{
  gchar *buffer;
  gsize buffer_size, buffer_i, events;
  
  G_LOCK (inotify_lock);
  ik_read_events (&buffer_size, &buffer);
  
  buffer_i = 0;
  events = 0;
  while (buffer_i < buffer_size)
    {
      struct inotify_event *event;
      gsize event_size;
      event = (struct inotify_event *)&buffer[buffer_i];
      event_size = sizeof(struct inotify_event) + event->len;
      g_queue_push_tail (events_to_process, ik_event_internal_new (ik_event_new (&buffer[buffer_i])));
      buffer_i += event_size;
      events++;
    }
  
  /* If the event process callback is off, turn it back on */
  if (!process_eq_running && events)
    {
      GSource *timeout_source;

      process_eq_running = TRUE;
      timeout_source = g_timeout_source_new (PROCESS_EVENTS_TIME);
      g_source_set_callback (timeout_source, ik_process_eq_callback, NULL, NULL);
      g_source_attach (timeout_source, GLIB_PRIVATE_CALL (g_get_worker_context ()));
      g_source_unref (timeout_source);
    }
  
  G_UNLOCK (inotify_lock);
  
  return TRUE;
}

static gboolean
g_timeval_lt (GTimeVal *val1,
              GTimeVal *val2)
{
  if (val1->tv_sec < val2->tv_sec)
    return TRUE;

  if (val1->tv_sec > val2->tv_sec)
    return FALSE;

  /* val1->tv_sec == val2->tv_sec */
  if (val1->tv_usec < val2->tv_usec)
    return TRUE;

  return FALSE;
}

static gboolean
g_timeval_le (GTimeVal *val1,
              GTimeVal *val2)
{
  if (val1->tv_sec < val2->tv_sec)
    return TRUE;

  if (val1->tv_sec > val2->tv_sec)
    return FALSE;

  /* val1->tv_sec == val2->tv_sec */
  if (val1->tv_usec <= val2->tv_usec)
    return TRUE;

  return FALSE;
}

static void
ik_pair_events (ik_event_internal_t *event1, 
                ik_event_internal_t *event2)
{
  g_assert (event1 && event2);
  /* We should only be pairing events that have the same cookie */
  g_assert (event1->event->cookie == event2->event->cookie);
  /* We shouldn't pair an event that already is paired */
  g_assert (event1->pair == NULL && event2->pair == NULL);

  /* Pair the internal structures and the ik_event_t structures */
  event1->pair = event2;
  event1->event->pair = event2->event;
  event2->event->is_second_in_pair = TRUE;

  if (g_timeval_lt (&event1->hold_until, &event2->hold_until))
    event1->hold_until = event2->hold_until;

  event2->hold_until = event1->hold_until;
}

static void
ik_event_add_microseconds (ik_event_internal_t *event, 
                           glong                ms)
{
  g_assert (event);
  g_time_val_add (&event->hold_until, ms);
}

static gboolean
ik_event_ready (ik_event_internal_t *event)
{
  GTimeVal tv;
  g_assert (event);
  
  g_get_current_time (&tv);
  
  /* An event is ready if,
   *
   * it has no cookie -- there is nothing to be gained by holding it
   * or, it is already paired -- we don't need to hold it anymore
   * or, we have held it long enough
   */
  return
    event->event->cookie == 0 ||
    event->pair != NULL ||
    g_timeval_le (&event->hold_until, &tv);
}

static void
ik_pair_moves (gpointer data, 
               gpointer user_data)
{
  ik_event_internal_t *event = (ik_event_internal_t *)data;
  
  if (event->seen == TRUE || event->sent == TRUE)
    return;
  
  if (event->event->cookie != 0)
    {
      /* When we get a MOVED_FROM event we delay sending the event by
       * MOVE_HOLD_UNTIL_TIME microseconds. We need to do this because a
       * MOVED_TO pair _might_ be coming in the near future */
      if (event->event->mask & IN_MOVED_FROM)
	{
	  g_hash_table_insert (cookie_hash, GINT_TO_POINTER (event->event->cookie), event);
	  /* because we don't deliver move events there is no point in waiting for the match right now. */
	  ik_event_add_microseconds (event, MOVE_HOLD_UNTIL_TIME);
	}
      else if (event->event->mask & IN_MOVED_TO)
	{
	  /* We need to check if we are waiting for this MOVED_TO events cookie to pair it with
	   * a MOVED_FROM */
	  ik_event_internal_t *match = NULL;
	  match = g_hash_table_lookup (cookie_hash, GINT_TO_POINTER (event->event->cookie));
	  if (match)
	    {
	      g_hash_table_remove (cookie_hash, GINT_TO_POINTER (event->event->cookie));
	      ik_pair_events (match, event);
	    }
	}
    }
  event->seen = TRUE;
}

static void
ik_process_events (void)
{
  g_queue_foreach (events_to_process, ik_pair_moves, NULL);

  while (!g_queue_is_empty (events_to_process))
    {
      ik_event_internal_t *event = g_queue_peek_head (events_to_process);
      
      /* This must have been sent as part of a MOVED_TO/MOVED_FROM */
      if (event->sent)
	{
	  /* Pop event */
	  g_queue_pop_head (events_to_process);
	  /* Free the internal event structure */
	  g_free (event);
	  continue;
	}
      
      /* The event isn't ready yet */
      if (!ik_event_ready (event))
	break;
      
      /* Pop it */
      event = g_queue_pop_head (events_to_process);
      
      /* Check if this is a MOVED_FROM that is also sitting in the cookie_hash */
      if (event->event->cookie && event->pair == NULL &&
	  g_hash_table_lookup (cookie_hash, GINT_TO_POINTER (event->event->cookie)))
	g_hash_table_remove (cookie_hash, GINT_TO_POINTER (event->event->cookie));
      
      if (event->pair)
	{
	  /* We send out paired MOVED_FROM/MOVED_TO events in the same event buffer */
	  /* g_assert (event->event->mask == IN_MOVED_FROM && event->pair->event->mask == IN_MOVED_TO); */
	  /* Copy the paired data */
	  event->pair->sent = TRUE;
	  event->sent = TRUE;
	  ik_move_matches++;
	}
      else if (event->event->cookie)
	{
	  /* If we couldn't pair a MOVED_FROM and MOVED_TO together, we change
	   * the event masks */
	  /* Changeing MOVED_FROM to DELETE and MOVED_TO to create lets us make
	   * the gaurantee that you will never see a non-matched MOVE event */
	  event->event->original_mask = event->event->mask;

	  if (event->event->mask & IN_MOVED_FROM)
	    {
	      event->event->mask = IN_DELETE|(event->event->mask & IN_ISDIR);
	      ik_move_misses++; /* not super accurate, if we aren't watching the destination it still counts as a miss */
	    }
	  if (event->event->mask & IN_MOVED_TO)
	    event->event->mask = IN_CREATE|(event->event->mask & IN_ISDIR);
	}
      
      /* Push the ik_event_t onto the event queue */
      g_queue_push_tail (event_queue, event->event);
      /* Free the internal event structure */
      g_free (event);
    }
}

static gboolean
ik_process_eq_callback (gpointer user_data)
{
  gboolean res;
  
  /* Try and move as many events to the event queue */
  G_LOCK (inotify_lock);
  ik_process_events ();
  
  while (!g_queue_is_empty (event_queue))
    {
      ik_event_t *event = g_queue_pop_head (event_queue);
      
      user_cb (event);
    }

  res = TRUE;
  
  if (g_queue_get_length (events_to_process) == 0)
    {
      process_eq_running = FALSE;
      res = FALSE;
    }
  
  G_UNLOCK (inotify_lock);
  
  return res;
}
