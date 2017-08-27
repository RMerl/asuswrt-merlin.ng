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
#include <string.h>

#include "gfilemonitor.h"
#include "gioenumtypes.h"
#include "gfile.h"
#include "gvfs.h"
#include "glibintl.h"


struct _FileChange;
typedef struct _FileChange FileChange;
static void file_change_free (FileChange *change);

/**
 * SECTION:gfilemonitor
 * @short_description: File Monitor
 * @include: gio/gio.h
 *
 * Monitors a file or directory for changes.
 *
 * To obtain a #GFileMonitor for a file or directory, use
 * g_file_monitor(), g_file_monitor_file(), or
 * g_file_monitor_directory().
 *
 * To get informed about changes to the file or directory you are
 * monitoring, connect to the #GFileMonitor::changed signal. The
 * signal will be emitted in the <link
 * linkend="g-main-context-push-thread-default">thread-default main
 * context</link> of the thread that the monitor was created in
 * (though if the global default main context is blocked, this may
 * cause notifications to be blocked even if the thread-default
 * context is still running).
 **/

G_LOCK_DEFINE_STATIC(cancelled);

enum {
  CHANGED,
  LAST_SIGNAL
};

typedef struct {
  GFile *file;
  guint32 last_sent_change_time; /* 0 == not sent */
  guint32 send_delayed_change_at; /* 0 == never */
  guint32 send_virtual_changes_done_at; /* 0 == never */
} RateLimiter;

struct _GFileMonitorPrivate {
  gboolean cancelled;
  int rate_limit_msec;

  /* Rate limiting change events */
  GHashTable *rate_limiter;

  GMutex mutex;
  GSource *pending_file_change_source;
  GSList *pending_file_changes; /* FileChange */

  GSource *timeout;
  guint32 timeout_fires_at;

  GMainContext *context;
};

enum {
  PROP_0,
  PROP_RATE_LIMIT,
  PROP_CANCELLED
};

/* work around a limitation of the aliasing foo */
#undef g_file_monitor

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (GFileMonitor, g_file_monitor, G_TYPE_OBJECT)

static void
g_file_monitor_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  GFileMonitor *monitor;

  monitor = G_FILE_MONITOR (object);

  switch (prop_id)
    {
    case PROP_RATE_LIMIT:
      g_file_monitor_set_rate_limit (monitor, g_value_get_int (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
g_file_monitor_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  GFileMonitor *monitor;
  GFileMonitorPrivate *priv;

  monitor = G_FILE_MONITOR (object);
  priv = monitor->priv;

  switch (prop_id)
    {
    case PROP_RATE_LIMIT:
      g_value_set_int (value, priv->rate_limit_msec);
      break;

    case PROP_CANCELLED:
      G_LOCK (cancelled);
      g_value_set_boolean (value, priv->cancelled);
      G_UNLOCK (cancelled);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

#define DEFAULT_RATE_LIMIT_MSECS 800
#define DEFAULT_VIRTUAL_CHANGES_DONE_DELAY_SECS 2

static guint signals[LAST_SIGNAL] = { 0 };

static void
rate_limiter_free (RateLimiter *limiter)
{
  g_object_unref (limiter->file);
  g_slice_free (RateLimiter, limiter);
}

static void
g_file_monitor_finalize (GObject *object)
{
  GFileMonitor *monitor;

  monitor = G_FILE_MONITOR (object);

  if (monitor->priv->timeout)
    {
      g_source_destroy (monitor->priv->timeout);
      g_source_unref (monitor->priv->timeout);
    }

  g_hash_table_destroy (monitor->priv->rate_limiter);

  g_main_context_unref (monitor->priv->context);
  g_mutex_clear (&monitor->priv->mutex);

  G_OBJECT_CLASS (g_file_monitor_parent_class)->finalize (object);
}

static void
g_file_monitor_dispose (GObject *object)
{
  GFileMonitor *monitor;
  GFileMonitorPrivate *priv;
  
  monitor = G_FILE_MONITOR (object);
  priv = monitor->priv;

  if (priv->pending_file_change_source)
    {
      g_source_destroy (priv->pending_file_change_source);
      g_source_unref (priv->pending_file_change_source);
      priv->pending_file_change_source = NULL;
    }
  g_slist_free_full (priv->pending_file_changes, (GDestroyNotify) file_change_free);
  priv->pending_file_changes = NULL;

  /* Make sure we cancel on last unref */
  g_file_monitor_cancel (monitor);

  G_OBJECT_CLASS (g_file_monitor_parent_class)->dispose (object);
}

static void
g_file_monitor_class_init (GFileMonitorClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = g_file_monitor_finalize;
  object_class->dispose = g_file_monitor_dispose;
  object_class->get_property = g_file_monitor_get_property;
  object_class->set_property = g_file_monitor_set_property;

  /**
   * GFileMonitor::changed:
   * @monitor: a #GFileMonitor.
   * @file: a #GFile.
   * @other_file: (allow-none): a #GFile or #NULL.
   * @event_type: a #GFileMonitorEvent.
   *
   * Emitted when @file has been changed.
   *
   * If using #G_FILE_MONITOR_SEND_MOVED flag and @event_type is
   * #G_FILE_MONITOR_EVENT_MOVED, @file will be set to a #GFile containing the
   * old path, and @other_file will be set to a #GFile containing the new path.
   *
   * In all the other cases, @other_file will be set to #NULL.
   **/
  signals[CHANGED] =
    g_signal_new (I_("changed"),
		  G_TYPE_FILE_MONITOR,
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (GFileMonitorClass, changed),
		  NULL, NULL,
		  NULL,
		  G_TYPE_NONE, 3,
		  G_TYPE_FILE, G_TYPE_FILE, G_TYPE_FILE_MONITOR_EVENT);

  g_object_class_install_property (object_class,
                                   PROP_RATE_LIMIT,
                                   g_param_spec_int ("rate-limit",
                                                     P_("Rate limit"),
                                                     P_("The limit of the monitor to watch for changes, in milliseconds"),
                                                     0, G_MAXINT,
                                                     DEFAULT_RATE_LIMIT_MSECS,
                                                     G_PARAM_READWRITE|
                                                     G_PARAM_STATIC_NAME|G_PARAM_STATIC_NICK|G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_CANCELLED,
                                   g_param_spec_boolean ("cancelled",
                                                         P_("Cancelled"),
                                                         P_("Whether the monitor has been cancelled"),
                                                         FALSE,
                                                         G_PARAM_READABLE|
                                                         G_PARAM_STATIC_NAME|G_PARAM_STATIC_NICK|G_PARAM_STATIC_BLURB));
}

static void
g_file_monitor_init (GFileMonitor *monitor)
{
  monitor->priv = g_file_monitor_get_instance_private (monitor);
  monitor->priv->rate_limit_msec = DEFAULT_RATE_LIMIT_MSECS;
  monitor->priv->rate_limiter = g_hash_table_new_full (g_file_hash, (GEqualFunc)g_file_equal,
						       NULL, (GDestroyNotify) rate_limiter_free);
  monitor->priv->context = g_main_context_ref_thread_default ();
  g_mutex_init (&monitor->priv->mutex);
}

/**
 * g_file_monitor_is_cancelled:
 * @monitor: a #GFileMonitor
 * 
 * Returns whether the monitor is canceled.
 *
 * Returns: %TRUE if monitor is canceled. %FALSE otherwise.
 **/
gboolean
g_file_monitor_is_cancelled (GFileMonitor *monitor)
{
  gboolean res;

  g_return_val_if_fail (G_IS_FILE_MONITOR (monitor), FALSE);

  G_LOCK (cancelled);
  res = monitor->priv->cancelled;
  G_UNLOCK (cancelled);
  
  return res;
}

/**
 * g_file_monitor_cancel:
 * @monitor: a #GFileMonitor.
 * 
 * Cancels a file monitor.
 * 
 * Returns: %TRUE if monitor was cancelled.
 **/
gboolean
g_file_monitor_cancel (GFileMonitor* monitor)
{
  GFileMonitorClass *klass;
  
  g_return_val_if_fail (G_IS_FILE_MONITOR (monitor), FALSE);
  
  G_LOCK (cancelled);
  if (monitor->priv->cancelled)
    {
      G_UNLOCK (cancelled);
      return TRUE;
    }
  
  monitor->priv->cancelled = TRUE;
  G_UNLOCK (cancelled);
  
  g_object_notify (G_OBJECT (monitor), "cancelled");

  klass = G_FILE_MONITOR_GET_CLASS (monitor);
  return (* klass->cancel) (monitor);
}

/**
 * g_file_monitor_set_rate_limit:
 * @monitor: a #GFileMonitor.
 * @limit_msecs: a non-negative integer with the limit in milliseconds
 *     to poll for changes
 *
 * Sets the rate limit to which the @monitor will report
 * consecutive change events to the same file.
 */
void
g_file_monitor_set_rate_limit (GFileMonitor *monitor,
                               gint          limit_msecs)
{
  GFileMonitorPrivate *priv;

  g_return_if_fail (G_IS_FILE_MONITOR (monitor));
  g_return_if_fail (limit_msecs >= 0);

  priv = monitor->priv;
  if (priv->rate_limit_msec != limit_msecs)
    {
      monitor->priv->rate_limit_msec = limit_msecs;
      g_object_notify (G_OBJECT (monitor), "rate-limit");
    }
}

struct _FileChange {
  GFile             *child;
  GFile             *other_file;
  GFileMonitorEvent  event_type;
};

static void
file_change_free (FileChange *change)
{
  g_object_unref (change->child);
  if (change->other_file)
    g_object_unref (change->other_file);
  
  g_slice_free (FileChange, change);
}

static gboolean
emit_cb (gpointer data)
{
  GFileMonitor *monitor = G_FILE_MONITOR (data);
  GSList *pending, *iter;

  g_mutex_lock (&monitor->priv->mutex);
  pending = g_slist_reverse (monitor->priv->pending_file_changes);
  monitor->priv->pending_file_changes = NULL;
  if (monitor->priv->pending_file_change_source)
    {
      g_source_unref (monitor->priv->pending_file_change_source);
      monitor->priv->pending_file_change_source = NULL;
    }
  g_mutex_unlock (&monitor->priv->mutex);

  g_object_ref (monitor);
  for (iter = pending; iter; iter = iter->next)
    {
       FileChange *change = iter->data;

       g_signal_emit (monitor, signals[CHANGED], 0,
	  	      change->child, change->other_file, change->event_type);
       file_change_free (change);
    }
  g_slist_free (pending);
  g_object_unref (monitor);

  return FALSE;
}

static void
emit_in_idle (GFileMonitor      *monitor,
	      GFile             *child,
	      GFile             *other_file,
	      GFileMonitorEvent  event_type)
{
  GSource *source;
  FileChange *change;
  GFileMonitorPrivate *priv;

  priv = monitor->priv;

  change = g_slice_new (FileChange);

  change->child = g_object_ref (child);
  if (other_file)
    change->other_file = g_object_ref (other_file);
  else
    change->other_file = NULL;
  change->event_type = event_type;

  g_mutex_lock (&monitor->priv->mutex);
  if (!priv->pending_file_change_source)
    {
      source = g_idle_source_new ();
      priv->pending_file_change_source = source;
      g_source_set_priority (source, 0);

      /* We don't ref monitor here - instead dispose will free any
       * pending idles.
       */
      g_source_set_callback (source, emit_cb, monitor, NULL);
      g_source_attach (source, monitor->priv->context);
    }
  /* We reverse this in the processor */
  priv->pending_file_changes = g_slist_prepend (priv->pending_file_changes, change);
  g_mutex_unlock (&monitor->priv->mutex);
}

static guint32
get_time_msecs (void)
{
  return g_get_monotonic_time () / G_TIME_SPAN_MILLISECOND;
}

static guint32
time_difference (guint32 from, guint32 to)
{
  if (from > to)
    return 0;
  return to - from;
}

/* Change event rate limiting support: */

static RateLimiter *
new_limiter (GFileMonitor *monitor,
	     GFile             *file)
{
  RateLimiter *limiter;

  limiter = g_slice_new0 (RateLimiter);
  limiter->file = g_object_ref (file);
  g_hash_table_insert (monitor->priv->rate_limiter, file, limiter);
  
  return limiter;
}

static void
rate_limiter_send_virtual_changes_done_now (GFileMonitor *monitor, 
                                            RateLimiter  *limiter)
{
  if (limiter->send_virtual_changes_done_at != 0)
    {
      emit_in_idle (monitor, limiter->file, NULL,
		    G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT);
      limiter->send_virtual_changes_done_at = 0;
    }
}

static void
rate_limiter_send_delayed_change_now (GFileMonitor *monitor, 
                                      RateLimiter *limiter, 
                                      guint32 time_now)
{
  if (limiter->send_delayed_change_at != 0)
    {
      emit_in_idle (monitor, 
		    limiter->file, NULL,
		    G_FILE_MONITOR_EVENT_CHANGED);
      limiter->send_delayed_change_at = 0;
      limiter->last_sent_change_time = time_now;
    }
}

typedef struct {
  guint32 min_time;
  guint32 time_now;
  GFileMonitor *monitor;
} ForEachData;

static gboolean
calc_min_time (GFileMonitor *monitor, 
               RateLimiter *limiter, 
               guint32 time_now, 
               guint32 *min_time)
{
  gboolean delete_me;
  guint32 expire_at;

  delete_me = TRUE;

  if (limiter->last_sent_change_time != 0)
    {
      /* Set a timeout at 2*rate limit so that we can clear out the change from the hash eventually */
      expire_at = limiter->last_sent_change_time + 2 * monitor->priv->rate_limit_msec;

      if (time_difference (time_now, expire_at) > 0)
	{
	  delete_me = FALSE;
	  *min_time = MIN (*min_time,
			   time_difference (time_now, expire_at));
	}
    }

  if (limiter->send_delayed_change_at != 0)
    {
      delete_me = FALSE;
      *min_time = MIN (*min_time,
		       time_difference (time_now, limiter->send_delayed_change_at));
    }

  if (limiter->send_virtual_changes_done_at != 0)
    {
      delete_me = FALSE;
      *min_time = MIN (*min_time,
		       time_difference (time_now, limiter->send_virtual_changes_done_at));
    }

  return delete_me;
}

static gboolean
foreach_rate_limiter_fire (gpointer key,
			   gpointer value,
			   gpointer user_data)
{
  RateLimiter *limiter = value;
  ForEachData *data = user_data;

  if (limiter->send_delayed_change_at != 0 &&
      time_difference (data->time_now, limiter->send_delayed_change_at) == 0)
    rate_limiter_send_delayed_change_now (data->monitor, limiter, data->time_now);
  
  if (limiter->send_virtual_changes_done_at != 0 &&
      time_difference (data->time_now, limiter->send_virtual_changes_done_at) == 0)
    rate_limiter_send_virtual_changes_done_now (data->monitor, limiter);
  
  return calc_min_time (data->monitor, limiter, data->time_now, &data->min_time);
}

static gboolean 
rate_limiter_timeout (gpointer timeout_data)
{
  GFileMonitor *monitor = timeout_data;
  ForEachData data;
  GSource *source;
  
  data.min_time = G_MAXUINT32;
  data.monitor = monitor;
  data.time_now = get_time_msecs ();
  g_hash_table_foreach_remove (monitor->priv->rate_limiter,
			       foreach_rate_limiter_fire,
			       &data);
  
  /* Remove old timeout */
  if (monitor->priv->timeout)
    {
      g_source_destroy (monitor->priv->timeout);
      g_source_unref (monitor->priv->timeout);
      monitor->priv->timeout = NULL;
      monitor->priv->timeout_fires_at = 0;
    }
  
  /* Set up new timeout */
  if (data.min_time != G_MAXUINT32)
    {
      source = g_timeout_source_new (data.min_time + 1); /* + 1 to make sure we've really passed the time */
      g_source_set_callback (source, rate_limiter_timeout, monitor, NULL);
      g_source_attach (source, monitor->priv->context);
      
      monitor->priv->timeout = source;
      monitor->priv->timeout_fires_at = data.time_now + data.min_time; 
    }
  
  return FALSE;
}

static gboolean
foreach_rate_limiter_update (gpointer key,
			     gpointer value,
			     gpointer user_data)
{
  RateLimiter *limiter = value;
  ForEachData *data = user_data;

  return calc_min_time (data->monitor, limiter, data->time_now, &data->min_time);
}

static void
update_rate_limiter_timeout (GFileMonitor *monitor, 
                             guint new_time)
{
  ForEachData data;
  GSource *source;
  
  if (monitor->priv->timeout_fires_at != 0 && new_time != 0 &&
      time_difference (new_time, monitor->priv->timeout_fires_at) == 0)
    return; /* Nothing to do, we already fire earlier than that */

  data.min_time = G_MAXUINT32;
  data.monitor = monitor;
  data.time_now = get_time_msecs ();
  g_hash_table_foreach_remove (monitor->priv->rate_limiter,
			       foreach_rate_limiter_update,
			       &data);

  /* Remove old timeout */
  if (monitor->priv->timeout)
    {
      g_source_destroy (monitor->priv->timeout);
      g_source_unref (monitor->priv->timeout);
      monitor->priv->timeout_fires_at = 0;
      monitor->priv->timeout = NULL;
    }

  /* Set up new timeout */
  if (data.min_time != G_MAXUINT32)
    {
      source = g_timeout_source_new (data.min_time + 1);  /* + 1 to make sure we've really passed the time */
      g_source_set_callback (source, rate_limiter_timeout, monitor, NULL);
      g_source_attach (source, monitor->priv->context);
      
      monitor->priv->timeout = source;
      monitor->priv->timeout_fires_at = data.time_now + data.min_time; 
    }
}

/**
 * g_file_monitor_emit_event:
 * @monitor: a #GFileMonitor.
 * @child: a #GFile.
 * @other_file: a #GFile.
 * @event_type: a set of #GFileMonitorEvent flags.
 * 
 * Emits the #GFileMonitor::changed signal if a change
 * has taken place. Should be called from file monitor 
 * implementations only.
 *
 * The signal will be emitted from an idle handler (in the <link
 * linkend="g-main-context-push-thread-default">thread-default main
 * context</link>).
 **/
void
g_file_monitor_emit_event (GFileMonitor      *monitor,
			   GFile             *child,
			   GFile             *other_file,
			   GFileMonitorEvent  event_type)
{
  guint32 time_now, since_last;
  gboolean emit_now;
  RateLimiter *limiter;

  g_return_if_fail (G_IS_FILE_MONITOR (monitor));
  g_return_if_fail (G_IS_FILE (child));

  limiter = g_hash_table_lookup (monitor->priv->rate_limiter, child);

  if (event_type != G_FILE_MONITOR_EVENT_CHANGED)
    {
      if (limiter)
	{
	  rate_limiter_send_delayed_change_now (monitor, limiter, get_time_msecs ());
	  if (event_type == G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT)
	    limiter->send_virtual_changes_done_at = 0;
	  else
	    rate_limiter_send_virtual_changes_done_now (monitor, limiter);
	  update_rate_limiter_timeout (monitor, 0);
	}
      emit_in_idle (monitor, child, other_file, event_type);
    }
  else
    {
      /* Changed event, rate limit */
      time_now = get_time_msecs ();
      emit_now = TRUE;
      
      if (limiter)
	{
	  since_last = time_difference (limiter->last_sent_change_time, time_now);
	  if (since_last < monitor->priv->rate_limit_msec)
	    {
	      /* We ignore this change, but arm a timer so that we can fire it later if we
		 don't get any other events (that kill this timeout) */
	      emit_now = FALSE;
	      if (limiter->send_delayed_change_at == 0)
		{
		  limiter->send_delayed_change_at = time_now + monitor->priv->rate_limit_msec;
		  update_rate_limiter_timeout (monitor, limiter->send_delayed_change_at);
		}
	    }
	}
      
      if (limiter == NULL)
	limiter = new_limiter (monitor, child);
      
      if (emit_now)
	{
	  emit_in_idle (monitor, child, other_file, event_type);
	  
	  limiter->last_sent_change_time = time_now;
	  limiter->send_delayed_change_at = 0;
	  /* Set a timeout of 2*rate limit so that we can clear out the change from the hash eventually */
	  update_rate_limiter_timeout (monitor, time_now + 2 * monitor->priv->rate_limit_msec);
	}
      
      /* Schedule a virtual change done. This is removed if we get a real one, and
	 postponed if we get more change events. */
      
      limiter->send_virtual_changes_done_at = time_now + DEFAULT_VIRTUAL_CHANGES_DONE_DELAY_SECS * 1000;
      update_rate_limiter_timeout (monitor, limiter->send_virtual_changes_done_at);
    }
}
