/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-session-sync.c
 *
 * Copyright (C) 2000-2003, Ximian, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define LIBSOUP_I_HAVE_READ_BUG_594377_AND_KNOW_SOUP_PASSWORD_MANAGER_MIGHT_GO_AWAY

#include "soup-session-sync.h"
#include "soup.h"
#include "soup-session-private.h"
#include "soup-message-private.h"
#include "soup-message-queue.h"

/**
 * SECTION:soup-session-sync
 * @short_description: Soup session for blocking I/O in multithreaded
 * programs.
 *
 * #SoupSessionSync is an implementation of #SoupSession that uses
 * synchronous I/O, intended for use in multi-threaded programs.
 *
 * You can use #SoupSessionSync from multiple threads concurrently.
 * Eg, you can send a #SoupMessage in one thread, and then while
 * waiting for the response, send another #SoupMessage from another
 * thread. You can also send a message from one thread and then call
 * soup_session_cancel_message() on it from any other thread (although
 * you need to be careful to avoid race conditions, where the message
 * finishes and is then unreffed by the sending thread just before you
 * cancel it).
 *
 * However, the majority of other types and methods in libsoup are not
 * MT-safe. In particular, you <emphasis>cannot</emphasis> modify or
 * examine a #SoupMessage while it is being transmitted by
 * #SoupSessionSync in another thread. Once a message has been handed
 * off to #SoupSessionSync, it can only be manipulated from its signal
 * handler callbacks, until I/O is complete.
 **/

typedef struct {
	GMutex lock;
	GCond cond;
} SoupSessionSyncPrivate;
#define SOUP_SESSION_SYNC_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), SOUP_TYPE_SESSION_SYNC, SoupSessionSyncPrivate))

G_DEFINE_TYPE (SoupSessionSync, soup_session_sync, SOUP_TYPE_SESSION)

static void
soup_session_sync_init (SoupSessionSync *ss)
{
	SoupSessionSyncPrivate *priv = SOUP_SESSION_SYNC_GET_PRIVATE (ss);

	g_mutex_init (&priv->lock);
	g_cond_init (&priv->cond);
}

static void
soup_session_sync_finalize (GObject *object)
{
	SoupSessionSyncPrivate *priv = SOUP_SESSION_SYNC_GET_PRIVATE (object);

	g_mutex_clear (&priv->lock);
	g_cond_clear (&priv->cond);

	G_OBJECT_CLASS (soup_session_sync_parent_class)->finalize (object);
}


/**
 * soup_session_sync_new:
 *
 * Creates an synchronous #SoupSession with the default options.
 *
 * Return value: the new session.
 **/
SoupSession *
soup_session_sync_new (void)
{
	return g_object_new (SOUP_TYPE_SESSION_SYNC, NULL);
}

/**
 * soup_session_sync_new_with_options:
 * @optname1: name of first property to set
 * @...: value of @optname1, followed by additional property/value pairs
 *
 * Creates an synchronous #SoupSession with the specified options.
 *
 * Return value: the new session.
 **/
SoupSession *
soup_session_sync_new_with_options (const char *optname1, ...)
{
	SoupSession *session;
	va_list ap;

	va_start (ap, optname1);
	session = (SoupSession *)g_object_new_valist (SOUP_TYPE_SESSION_SYNC,
						      optname1, ap);
	va_end (ap);

	return session;
}

static guint
tunnel_connect (SoupSession *session, SoupMessageQueueItem *related)
{
	SoupConnection *conn = related->conn;
	SoupMessageQueueItem *item;
	guint status;

	g_object_ref (conn);

	item = soup_session_make_connect_message (session, conn);
	do {
		soup_session_send_queue_item (session, item, NULL);
		status = item->msg->status_code;
		if (item->state == SOUP_MESSAGE_RESTARTING &&
		    soup_message_io_in_progress (item->msg)) {
			soup_message_restarted (item->msg);
			item->state = SOUP_MESSAGE_RUNNING;
		} else {
			if (item->state == SOUP_MESSAGE_RESTARTING)
				status = SOUP_STATUS_TRY_AGAIN;
			item->state = SOUP_MESSAGE_FINISHED;
			soup_message_finished (item->msg);
		}
	} while (item->state == SOUP_MESSAGE_STARTING);
	soup_session_unqueue_item (session, item);
	soup_message_queue_item_unref (item);

	if (SOUP_STATUS_IS_SUCCESSFUL (status)) {
		if (!soup_connection_start_ssl_sync (conn, related->cancellable))
			status = SOUP_STATUS_SSL_FAILED;
		soup_message_set_https_status (related->msg, conn);
	}

	g_object_unref (conn);
	return status;
}

static void
get_connection (SoupMessageQueueItem *item)
{
	SoupSession *session = item->session;
	SoupMessage *msg = item->msg;
	gboolean try_pruning = FALSE;
	guint status;

try_again:
	soup_session_cleanup_connections (session, FALSE);

	if (!soup_session_get_connection (session, item, &try_pruning)) {
		if (!try_pruning)
			return;
		soup_session_cleanup_connections (session, TRUE);
		if (!soup_session_get_connection (session, item, &try_pruning))
			return;
		try_pruning = FALSE;
	}

	if (soup_connection_get_state (item->conn) == SOUP_CONNECTION_IDLE) {
		item->state = SOUP_MESSAGE_READY;
		return;
	}

	if (soup_connection_get_state (item->conn) == SOUP_CONNECTION_NEW) {
		status = soup_connection_connect_sync (item->conn, item->cancellable);
		if (status == SOUP_STATUS_TRY_AGAIN) {
			soup_session_set_item_connection (session, item, NULL);
			goto try_again;
		}

		soup_message_set_https_status (msg, item->conn);

		if (!SOUP_STATUS_IS_SUCCESSFUL (status)) {
			if (!msg->status_code)
				soup_session_set_item_status (session, item, status);
			item->state = SOUP_MESSAGE_FINISHING;
			soup_session_set_item_connection (session, item, NULL);
			return;
		}
	}

	if (soup_connection_is_tunnelled (item->conn)) {
		status = tunnel_connect (session, item);
		if (!SOUP_STATUS_IS_SUCCESSFUL (status)) {
			soup_session_set_item_connection (session, item, NULL);
			if (status == SOUP_STATUS_TRY_AGAIN)
				goto try_again;
			soup_session_set_item_status (session, item, status);
			item->state = SOUP_MESSAGE_FINISHING;
			return;
		}
	}

	item->state = SOUP_MESSAGE_READY;
}

static void process_queue_item (SoupMessageQueueItem *item);

static void
new_api_message_completed (SoupMessage *msg, gpointer user_data)
{
	SoupMessageQueueItem *item = user_data;

	if (item->state != SOUP_MESSAGE_RESTARTING) {
		item->state = SOUP_MESSAGE_FINISHING;
		process_queue_item (item);
	}
}

static void
process_queue_item (SoupMessageQueueItem *item)
{
	SoupSession *session = item->session;
	SoupSessionSyncPrivate *priv = SOUP_SESSION_SYNC_GET_PRIVATE (session);

	soup_message_queue_item_ref (item);

	do {
		if (item->paused) {
			g_mutex_lock (&priv->lock);
			while (item->paused)
				g_cond_wait (&priv->cond, &priv->lock);
			g_mutex_unlock (&priv->lock);
		}

		switch (item->state) {
		case SOUP_MESSAGE_STARTING:
			g_mutex_lock (&priv->lock);
			do {
				get_connection (item);
				if (item->state == SOUP_MESSAGE_STARTING)
					g_cond_wait (&priv->cond, &priv->lock);
			} while (item->state == SOUP_MESSAGE_STARTING);
			g_mutex_unlock (&priv->lock);
			break;

		case SOUP_MESSAGE_READY:
			item->state = SOUP_MESSAGE_RUNNING;

			if (item->new_api) {
				soup_session_send_queue_item (item->session, item, new_api_message_completed);
				goto out;
			}

			soup_session_send_queue_item (item->session, item, NULL);
			if (item->state != SOUP_MESSAGE_RESTARTING)
				item->state = SOUP_MESSAGE_FINISHING;
			break;

		case SOUP_MESSAGE_RUNNING:
			g_warn_if_fail (item->new_api);
			item->state = SOUP_MESSAGE_FINISHING;
			break;

		case SOUP_MESSAGE_RESTARTING:
			item->state = SOUP_MESSAGE_STARTING;
			soup_message_restarted (item->msg);
			break;

		case SOUP_MESSAGE_FINISHING:
			item->state = SOUP_MESSAGE_FINISHED;
			soup_message_finished (item->msg);
			soup_session_unqueue_item (session, item);
			break;

		default:
			g_warn_if_reached ();
			item->state = SOUP_MESSAGE_FINISHING;
			break;
		}
	} while (item->state != SOUP_MESSAGE_FINISHED);

 out:
	soup_message_queue_item_unref (item);
}

static gboolean
queue_message_callback (gpointer data)
{
	SoupMessageQueueItem *item = data;

	item->callback (item->session, item->msg, item->callback_data);
	soup_message_queue_item_unref (item);
	return FALSE;
}

static gpointer
queue_message_thread (gpointer data)
{
	SoupMessageQueueItem *item = data;

	process_queue_item (item);
	if (item->callback) {
		soup_add_completion (soup_session_get_async_context (item->session),
				     queue_message_callback, item);
	} else
		soup_message_queue_item_unref (item);

	return NULL;
}

static void
soup_session_sync_queue_message (SoupSession *session, SoupMessage *msg,
				 SoupSessionCallback callback, gpointer user_data)
{
	SoupMessageQueueItem *item;
	GThread *thread;

	item = soup_session_append_queue_item (session, msg, callback, user_data);
	thread = g_thread_new ("SoupSessionSync:queue_message",
			       queue_message_thread, item);
	g_thread_unref (thread);
}

static guint
soup_session_sync_send_message (SoupSession *session, SoupMessage *msg)
{
	SoupMessageQueueItem *item;
	guint status;

	item = soup_session_append_queue_item (session, msg, NULL, NULL);
	process_queue_item (item);
	status = msg->status_code;
	soup_message_queue_item_unref (item);
	return status;
}

static void
soup_session_sync_cancel_message (SoupSession *session, SoupMessage *msg, guint status_code)
{
	SoupSessionSyncPrivate *priv = SOUP_SESSION_SYNC_GET_PRIVATE (session);

	g_mutex_lock (&priv->lock);
	SOUP_SESSION_CLASS (soup_session_sync_parent_class)->cancel_message (session, msg, status_code);
	g_cond_broadcast (&priv->cond);
	g_mutex_unlock (&priv->lock);
}

static void
soup_session_sync_auth_required (SoupSession *session, SoupMessage *msg,
				 SoupAuth *auth, gboolean retrying)
{
	SoupSessionFeature *password_manager;

	password_manager = soup_session_get_feature_for_message (
		session, SOUP_TYPE_PASSWORD_MANAGER, msg);
	if (password_manager) {
		soup_password_manager_get_passwords_sync (
			SOUP_PASSWORD_MANAGER (password_manager),
			msg, auth, NULL); /* FIXME cancellable */
	}

	SOUP_SESSION_CLASS (soup_session_sync_parent_class)->
		auth_required (session, msg, auth, retrying);
}

static void
soup_session_sync_flush_queue (SoupSession *session)
{
	SoupSessionSyncPrivate *priv = SOUP_SESSION_SYNC_GET_PRIVATE (session);
	SoupMessageQueue *queue;
	SoupMessageQueueItem *item;
	GHashTable *current;
	gboolean done = FALSE;

	/* Record the current contents of the queue */
	current = g_hash_table_new (NULL, NULL);
	queue = soup_session_get_queue (session);
	for (item = soup_message_queue_first (queue);
	     item;
	     item = soup_message_queue_next (queue, item))
		g_hash_table_insert (current, item, item);

	/* Cancel everything */
	SOUP_SESSION_CLASS (soup_session_sync_parent_class)->flush_queue (session);

	/* Wait until all of the items in @current have been removed
	 * from the queue. (This is not the same as "wait for the
	 * queue to be empty", because the app may queue new requests
	 * in response to the cancellation of the old ones. We don't
	 * try to cancel those requests as well, since we'd likely
	 * just end up looping forever.)
	 */
	g_mutex_lock (&priv->lock);
	do {
		done = TRUE;
		for (item = soup_message_queue_first (queue);
		     item;
		     item = soup_message_queue_next (queue, item)) {
			if (g_hash_table_lookup (current, item))
				done = FALSE;
		}

		if (!done)
			g_cond_wait (&priv->cond, &priv->lock);
	} while (!done);
	g_mutex_unlock (&priv->lock);

	g_hash_table_destroy (current);
}

static void
soup_session_sync_kick (SoupSession *session)
{
	SoupSessionSyncPrivate *priv = SOUP_SESSION_SYNC_GET_PRIVATE (session);

	g_cond_broadcast (&priv->cond);
}

static void
soup_session_sync_class_init (SoupSessionSyncClass *session_sync_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (session_sync_class);
	SoupSessionClass *session_class = SOUP_SESSION_CLASS (session_sync_class);

	g_type_class_add_private (session_sync_class, sizeof (SoupSessionSyncPrivate));

	/* virtual method override */
	session_class->queue_message = soup_session_sync_queue_message;
	session_class->send_message = soup_session_sync_send_message;
	session_class->cancel_message = soup_session_sync_cancel_message;
	session_class->auth_required = soup_session_sync_auth_required;
	session_class->flush_queue = soup_session_sync_flush_queue;
	session_class->kick = soup_session_sync_kick;

	object_class->finalize = soup_session_sync_finalize;
}


GInputStream *
soup_session_send_request (SoupSession   *session,
			   SoupMessage   *msg,
			   GCancellable  *cancellable,
			   GError       **error)
{
	SoupMessageQueueItem *item;
	GInputStream *stream = NULL;
	GOutputStream *ostream;
	GMemoryOutputStream *mostream;
	gssize size;
	GError *my_error = NULL;

	g_return_val_if_fail (SOUP_IS_SESSION_SYNC (session), NULL);

	item = soup_session_append_queue_item (session, msg, NULL, NULL);

	item->new_api = TRUE;
	if (cancellable) {
		g_object_unref (item->cancellable);
		item->cancellable = g_object_ref (cancellable);
	}

	while (!stream) {
		/* Get a connection, etc */
		process_queue_item (item);
		if (item->state != SOUP_MESSAGE_RUNNING)
			break;

		/* Send request, read headers */
		if (!soup_message_io_run_until_read (msg, item->cancellable, &my_error)) {
			if (g_error_matches (my_error, SOUP_HTTP_ERROR, SOUP_STATUS_TRY_AGAIN)) {
				item->state = SOUP_MESSAGE_RESTARTING;
				soup_message_io_finished (item->msg);
				g_clear_error (&my_error);
				continue;
			} else
				break;
		}

		stream = soup_message_io_get_response_istream (msg, &my_error);
		if (!stream)
			break;

		/* Break if the message doesn't look likely-to-be-requeued */
		if (msg->status_code != SOUP_STATUS_UNAUTHORIZED &&
		    msg->status_code != SOUP_STATUS_PROXY_UNAUTHORIZED &&
		    !soup_session_would_redirect (session, msg))
			break;

		/* Gather the current message body... */
		ostream = g_memory_output_stream_new (NULL, 0, g_realloc, g_free);
		if (g_output_stream_splice (ostream, stream,
					    G_OUTPUT_STREAM_SPLICE_CLOSE_SOURCE |
					    G_OUTPUT_STREAM_SPLICE_CLOSE_TARGET,
					    item->cancellable, &my_error) == -1) {
			g_object_unref (stream);
			g_object_unref (ostream);
			stream = NULL;
			break;
		}
		g_object_unref (stream);
		stream = NULL;

		/* If the message was requeued, loop */
		if (item->state == SOUP_MESSAGE_RESTARTING) {
			g_object_unref (ostream);
			continue;
		}

		/* Not requeued, so return the original body */
		mostream = G_MEMORY_OUTPUT_STREAM (ostream);
		size = g_memory_output_stream_get_data_size (mostream);
		stream = g_memory_input_stream_new ();
		if (size) {
			g_memory_input_stream_add_data (G_MEMORY_INPUT_STREAM (stream),
							g_memory_output_stream_steal_data (mostream),
							size, g_free);
		}
		g_object_unref (ostream);
	}

	if (my_error)
		g_propagate_error (error, my_error);
	else if (SOUP_STATUS_IS_TRANSPORT_ERROR (msg->status_code)) {
		if (stream) {
			g_object_unref (stream);
			stream = NULL;
		}
		g_set_error_literal (error, SOUP_HTTP_ERROR, msg->status_code,
				     msg->reason_phrase);
	} else if (!stream)
		stream = g_memory_input_stream_new ();

	if (!stream) {
		if (soup_message_io_in_progress (msg))
			soup_message_io_finished (msg);
		else if (item->state != SOUP_MESSAGE_FINISHED)
			item->state = SOUP_MESSAGE_FINISHING;

		if (item->state != SOUP_MESSAGE_FINISHED)
			process_queue_item (item);
	}

	soup_message_queue_item_unref (item);
	return stream;
}
