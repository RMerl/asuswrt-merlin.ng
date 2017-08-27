/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-session-async.c
 *
 * Copyright (C) 2000-2003, Ximian, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define LIBSOUP_I_HAVE_READ_BUG_594377_AND_KNOW_SOUP_PASSWORD_MANAGER_MIGHT_GO_AWAY

#include "soup-session-async.h"
#include "soup.h"
#include "soup-session-private.h"
#include "soup-message-private.h"
#include "soup-message-queue.h"
#include "soup-misc-private.h"

/**
 * SECTION:soup-session-async
 * @short_description: Soup session for asynchronous (main-loop-based) I/O.
 *
 * #SoupSessionAsync is an implementation of #SoupSession that uses
 * non-blocking I/O via the glib main loop. It is intended for use in
 * single-threaded programs.
 **/

static void run_queue (SoupSessionAsync *sa);
static void do_idle_run_queue (SoupSession *session);

static void send_request_running   (SoupSession *session, SoupMessageQueueItem *item);
static void send_request_restarted (SoupSession *session, SoupMessageQueueItem *item);
static void send_request_finished  (SoupSession *session, SoupMessageQueueItem *item);

G_DEFINE_TYPE (SoupSessionAsync, soup_session_async, SOUP_TYPE_SESSION)

typedef struct {
	SoupSessionAsync *sa;
	GSList *sources;
	gboolean disposed;

} SoupSessionAsyncPrivate;
#define SOUP_SESSION_ASYNC_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), SOUP_TYPE_SESSION_ASYNC, SoupSessionAsyncPrivate))

static void
soup_session_async_init (SoupSessionAsync *sa)
{
	SoupSessionAsyncPrivate *priv = SOUP_SESSION_ASYNC_GET_PRIVATE (sa);

	priv->sa = sa;
}

static void
soup_session_async_dispose (GObject *object)
{
	SoupSessionAsyncPrivate *priv = SOUP_SESSION_ASYNC_GET_PRIVATE (object);
	GSList *iter;

	priv->disposed = TRUE;
	for (iter = priv->sources; iter; iter = iter->next) {
		g_source_destroy (iter->data);
		g_source_unref (iter->data);
	}
	g_clear_pointer (&priv->sources, g_slist_free);

	G_OBJECT_CLASS (soup_session_async_parent_class)->dispose (object);
}


/**
 * soup_session_async_new:
 *
 * Creates an asynchronous #SoupSession with the default options.
 *
 * Return value: the new session.
 **/
SoupSession *
soup_session_async_new (void)
{
	return g_object_new (SOUP_TYPE_SESSION_ASYNC, NULL);
}

/**
 * soup_session_async_new_with_options:
 * @optname1: name of first property to set
 * @...: value of @optname1, followed by additional property/value pairs
 *
 * Creates an asynchronous #SoupSession with the specified options.
 *
 * Return value: the new session.
 **/
SoupSession *
soup_session_async_new_with_options (const char *optname1, ...)
{
	SoupSession *session;
	va_list ap;

	va_start (ap, optname1);
	session = (SoupSession *)g_object_new_valist (SOUP_TYPE_SESSION_ASYNC,
						      optname1, ap);
	va_end (ap);

	return session;
}

static void
message_completed (SoupMessage *msg, gpointer user_data)
{
	SoupMessageQueueItem *item = user_data;

	do_idle_run_queue (item->session);

	if (item->state != SOUP_MESSAGE_RESTARTING)
		item->state = SOUP_MESSAGE_FINISHING;
}

static void
ssl_tunnel_completed (SoupConnection *conn, guint status, gpointer user_data)
{
	SoupMessageQueueItem *tunnel_item = user_data;
	SoupMessageQueueItem *item = tunnel_item->related;
	SoupSession *session = item->session;

	soup_message_finished (tunnel_item->msg);
	soup_message_queue_item_unref (tunnel_item);

	if (!SOUP_STATUS_IS_SUCCESSFUL (status)) {
		soup_session_set_item_connection (session, item, NULL);
		soup_message_set_status (item->msg, status);
	}

	item->state = SOUP_MESSAGE_READY;
	do_idle_run_queue (session);
	soup_message_queue_item_unref (item);
}

static void
tunnel_message_completed (SoupMessage *tunnel_msg, gpointer user_data)
{
	SoupMessageQueueItem *tunnel_item = user_data;
	SoupSession *session = tunnel_item->session;
	SoupMessageQueueItem *item = tunnel_item->related;

	if (tunnel_item->state == SOUP_MESSAGE_RESTARTING) {
		soup_message_restarted (tunnel_msg);
		if (tunnel_item->conn) {
			tunnel_item->state = SOUP_MESSAGE_RUNNING;
			soup_session_send_queue_item (session, tunnel_item,
						      tunnel_message_completed);
			return;
		}

		soup_message_set_status (tunnel_msg, SOUP_STATUS_TRY_AGAIN);
	}

	tunnel_item->state = SOUP_MESSAGE_FINISHED;
	soup_session_unqueue_item (session, tunnel_item);

	if (SOUP_STATUS_IS_SUCCESSFUL (tunnel_msg->status_code)) {
		soup_connection_start_ssl_async (item->conn, item->cancellable,
						 ssl_tunnel_completed, tunnel_item);
	} else {
		ssl_tunnel_completed (item->conn, tunnel_msg->status_code,
				      tunnel_item);
	}
}

static void
got_connection (SoupConnection *conn, guint status, gpointer user_data)
{
	SoupMessageQueueItem *item = user_data;
	SoupSession *session = item->session;

	if (status != SOUP_STATUS_OK) {
		if (item->state == SOUP_MESSAGE_CONNECTING) {
			soup_session_set_item_status (session, item, status);
			soup_session_set_item_connection (session, item, NULL);
			item->state = SOUP_MESSAGE_READY;
		}
	} else
		item->state = SOUP_MESSAGE_CONNECTED;

	run_queue ((SoupSessionAsync *)session);
	soup_message_queue_item_unref (item);
}

static void
process_queue_item (SoupMessageQueueItem *item,
		    gboolean             *should_prune,
		    gboolean              loop)
{
	SoupSession *session = item->session;

	if (item->async_context != soup_session_get_async_context (session))
		return;

	do {
		if (item->paused)
			return;

		switch (item->state) {
		case SOUP_MESSAGE_STARTING:
			if (!soup_session_get_connection (session, item, should_prune))
				return;

			if (soup_connection_get_state (item->conn) != SOUP_CONNECTION_NEW) {
				item->state = SOUP_MESSAGE_READY;
				break;
			}

			item->state = SOUP_MESSAGE_CONNECTING;
			soup_message_queue_item_ref (item);
			soup_connection_connect_async (item->conn, item->cancellable,
						       got_connection, item);
			return;

		case SOUP_MESSAGE_CONNECTED:
			if (soup_connection_is_tunnelled (item->conn)) {
				SoupMessageQueueItem *tunnel_item;

				soup_message_queue_item_ref (item);

				item->state = SOUP_MESSAGE_TUNNELING;

				tunnel_item = soup_session_make_connect_message (session, item->conn);
				tunnel_item->related = item;
				soup_session_send_queue_item (session, tunnel_item, tunnel_message_completed);
				return;
			}

			item->state = SOUP_MESSAGE_READY;
			break;

		case SOUP_MESSAGE_READY:
			soup_message_set_https_status (item->msg, item->conn);
			if (item->msg->status_code) {
				if (item->msg->status_code == SOUP_STATUS_TRY_AGAIN) {
					soup_message_cleanup_response (item->msg);
					item->state = SOUP_MESSAGE_STARTING;
				} else
					item->state = SOUP_MESSAGE_FINISHING;
				break;
			}

			item->state = SOUP_MESSAGE_RUNNING;
			soup_session_send_queue_item (session, item, message_completed);
			if (item->new_api)
				send_request_running (session, item);
			break;

		case SOUP_MESSAGE_RESTARTING:
			item->state = SOUP_MESSAGE_STARTING;
			soup_message_restarted (item->msg);
			if (item->new_api)
				send_request_restarted (session, item);
			break;

		case SOUP_MESSAGE_FINISHING:
			item->state = SOUP_MESSAGE_FINISHED;
			soup_message_finished (item->msg);
			if (item->state != SOUP_MESSAGE_FINISHED)
				break;

			soup_message_queue_item_ref (item);
			soup_session_unqueue_item (session, item);
			if (item->callback)
				item->callback (session, item->msg, item->callback_data);
			else if (item->new_api)
				send_request_finished (session, item);

			soup_message_queue_item_unref (item);
			return;

		default:
			/* Nothing to do with this message in any
			 * other state.
			 */
			return;
		}
	} while (loop && item->state != SOUP_MESSAGE_FINISHED);
}

static void
run_queue (SoupSessionAsync *sa)
{
	SoupSession *session = SOUP_SESSION (sa);
	SoupMessageQueue *queue = soup_session_get_queue (session);
	SoupMessageQueueItem *item;
	SoupMessage *msg;
	gboolean try_pruning = TRUE, should_prune = FALSE;

	g_object_ref (session);
	soup_session_cleanup_connections (session, FALSE);

 try_again:
	for (item = soup_message_queue_first (queue);
	     item;
	     item = soup_message_queue_next (queue, item)) {
		msg = item->msg;

		/* CONNECT messages are handled specially */
		if (msg->method != SOUP_METHOD_CONNECT)
			process_queue_item (item, &should_prune, TRUE);
	}

	if (try_pruning && should_prune) {
		/* There is at least one message in the queue that
		 * could be sent if we pruned an idle connection from
		 * some other server.
		 */
		if (soup_session_cleanup_connections (session, TRUE)) {
			try_pruning = should_prune = FALSE;
			goto try_again;
		}
	}

	g_object_unref (session);
}

static gboolean
idle_run_queue (gpointer user_data)
{
	SoupSessionAsyncPrivate *priv = user_data;
	GSource *source;

	if (priv->disposed)
		return FALSE;

	source = g_main_current_source ();
	priv->sources = g_slist_remove (priv->sources, source);

	/* Ensure that the source is destroyed before running the queue */
	g_source_destroy (source);
	g_source_unref (source);

	run_queue (priv->sa);
	return FALSE;
}

static void
do_idle_run_queue (SoupSession *session)
{
	SoupSessionAsyncPrivate *priv = SOUP_SESSION_ASYNC_GET_PRIVATE (session);
	GMainContext *async_context = soup_session_get_async_context (session);
	GSource *source;

	if (priv->disposed)
		return;

	/* We use priv rather than session as the source data, because
	 * other parts of libsoup (or the calling app) may have sources
	 * using the session as the source data.
	 */

	source = g_main_context_find_source_by_user_data (async_context, priv);
	if (source)
		return;

	source = soup_add_completion_reffed (async_context, idle_run_queue, priv);
	priv->sources = g_slist_prepend (priv->sources, source);
}

static void
soup_session_async_queue_message (SoupSession *session, SoupMessage *req,
				  SoupSessionCallback callback, gpointer user_data)
{
	SoupMessageQueueItem *item;

	item = soup_session_append_queue_item (session, req, callback, user_data);
	soup_message_queue_item_unref (item);

	do_idle_run_queue (session);
}

static guint
soup_session_async_send_message (SoupSession *session, SoupMessage *req)
{
	SoupMessageQueueItem *item;
	GMainContext *async_context =
		soup_session_get_async_context (session);

	soup_session_async_queue_message (session, req, NULL, NULL);

	item = soup_message_queue_lookup (soup_session_get_queue (session), req);
	g_return_val_if_fail (item != NULL, SOUP_STATUS_MALFORMED);

	while (item->state != SOUP_MESSAGE_FINISHED)
		g_main_context_iteration (async_context, TRUE);

	soup_message_queue_item_unref (item);

	return req->status_code;
}

static void
soup_session_async_cancel_message (SoupSession *session, SoupMessage *msg,
				   guint status_code)
{
	SoupMessageQueue *queue;
	SoupMessageQueueItem *item;
	gboolean dummy;

	SOUP_SESSION_CLASS (soup_session_async_parent_class)->
		cancel_message (session, msg, status_code);

	queue = soup_session_get_queue (session);
	item = soup_message_queue_lookup (queue, msg);
	if (!item)
		return;

	/* Force it to finish immediately, so that
	 * soup_session_abort (session); g_object_unref (session);
	 * will work. (The soup_session_cancel_message() docs
	 * point out that the callback will be invoked from
	 * within the cancel call.)
	 */
	if (soup_message_io_in_progress (msg))
		soup_message_io_finished (msg);
	else if (item->state != SOUP_MESSAGE_FINISHED)
		item->state = SOUP_MESSAGE_FINISHING;

	if (item->state != SOUP_MESSAGE_FINISHED)
		process_queue_item (item, &dummy, FALSE);

	soup_message_queue_item_unref (item);
}

static void
got_passwords (SoupPasswordManager *password_manager, SoupMessage *msg,
	       SoupAuth *auth, gboolean retrying, gpointer session)
{
	soup_session_unpause_message (session, msg);
	SOUP_SESSION_CLASS (soup_session_async_parent_class)->
		auth_required (session, msg, auth, retrying);
	g_object_unref (auth);
}

static void
soup_session_async_auth_required (SoupSession *session, SoupMessage *msg,
				  SoupAuth *auth, gboolean retrying)
{
	SoupSessionFeature *password_manager;

	password_manager = soup_session_get_feature_for_message (
		session, SOUP_TYPE_PASSWORD_MANAGER, msg);
	if (password_manager) {
		soup_session_pause_message (session, msg);
		g_object_ref (auth);
		soup_password_manager_get_passwords_async (
			SOUP_PASSWORD_MANAGER (password_manager),
			msg, auth, retrying,
			soup_session_get_async_context (session),
			NULL, /* FIXME cancellable */
			got_passwords, session);
	} else {
		SOUP_SESSION_CLASS (soup_session_async_parent_class)->
			auth_required (session, msg, auth, retrying);
	}
}

static void
soup_session_async_kick (SoupSession *session)
{
	do_idle_run_queue (session);
}


static void
send_request_return_result (SoupMessageQueueItem *item,
			    gpointer stream, GError *error)
{
	GTask *task;

	task = item->task;
	item->task = NULL;

	if (item->io_source) {
		g_source_destroy (item->io_source);
		g_clear_pointer (&item->io_source, g_source_unref);
	}

	if (error)
		g_task_return_error (task, error);
	else if (SOUP_STATUS_IS_TRANSPORT_ERROR (item->msg->status_code)) {
		if (stream)
			g_object_unref (stream);
		g_task_return_new_error (task, SOUP_HTTP_ERROR,
					 item->msg->status_code,
					 "%s",
					 item->msg->reason_phrase);
	} else
		g_task_return_pointer (task, stream, g_object_unref);
	g_object_unref (task);
}

static void
send_request_restarted (SoupSession *session, SoupMessageQueueItem *item)
{
	/* We won't be needing this, then. */
	g_object_set_data (G_OBJECT (item->msg), "SoupSessionAsync:ostream", NULL);
	item->io_started = FALSE;
}

static void
send_request_finished (SoupSession *session, SoupMessageQueueItem *item)
{
	GMemoryOutputStream *mostream;
	GInputStream *istream = NULL;
	GError *error = NULL;

	if (!item->task) {
		/* Something else already took care of it. */
		return;
	}

	mostream = g_object_get_data (G_OBJECT (item->task), "SoupSessionAsync:ostream");
	if (mostream) {
		gpointer data;
		gssize size;

		/* We thought it would be requeued, but it wasn't, so
		 * return the original body.
		 */
		size = g_memory_output_stream_get_data_size (mostream);
		data = size ? g_memory_output_stream_steal_data (mostream) : g_strdup ("");
		istream = g_memory_input_stream_new_from_data (data, size, g_free);
	} else if (item->io_started) {
		/* The message finished before becoming readable. This
		 * will happen, eg, if it's cancelled from got-headers.
		 * Do nothing; the op will complete via read_ready_cb()
		 * after we return;
		 */
		return;
	} else {
		/* The message finished before even being started;
		 * probably a tunnel connect failure.
		 */
		istream = g_memory_input_stream_new ();
	}

	send_request_return_result (item, istream, error);
}

static void
send_async_spliced (GObject *source, GAsyncResult *result, gpointer user_data)
{
	SoupMessageQueueItem *item = user_data;
	GInputStream *istream = g_object_get_data (source, "istream");
	GError *error = NULL;

	/* It should be safe to call the sync close() method here since
	 * the message body has already been written.
	 */
	g_input_stream_close (istream, NULL, NULL);
	g_object_unref (istream);

	/* If the message was cancelled, it will be completed via other means */
	if (g_cancellable_is_cancelled (item->cancellable) ||
	    !item->task) {
		soup_message_queue_item_unref (item);
		return;
	}

	if (g_output_stream_splice_finish (G_OUTPUT_STREAM (source),
					   result, &error) == -1) {
		send_request_return_result (item, NULL, error);
		soup_message_queue_item_unref (item);
		return;
	}

	/* Otherwise either restarted or finished will eventually be called. */
	do_idle_run_queue (item->session);
	soup_message_queue_item_unref (item);
}

static void
send_async_maybe_complete (SoupMessageQueueItem *item,
			   GInputStream         *stream)
{
	if (item->msg->status_code == SOUP_STATUS_UNAUTHORIZED ||
	    item->msg->status_code == SOUP_STATUS_PROXY_UNAUTHORIZED ||
	    soup_session_would_redirect (item->session, item->msg)) {
		GOutputStream *ostream;

		/* Message may be requeued, so gather the current message body... */
		ostream = g_memory_output_stream_new (NULL, 0, g_realloc, g_free);
		g_object_set_data_full (G_OBJECT (item->task), "SoupSessionAsync:ostream",
					ostream, g_object_unref);

		g_object_set_data (G_OBJECT (ostream), "istream", stream);

		/* Give the splice op its own ref on item */
		soup_message_queue_item_ref (item);
		g_output_stream_splice_async (ostream, stream,
					      /* We can't use CLOSE_SOURCE because it
					       * might get closed in the wrong thread.
					       */
					      G_OUTPUT_STREAM_SPLICE_CLOSE_TARGET,
					      G_PRIORITY_DEFAULT,
					      item->cancellable,
					      send_async_spliced, item);
		return;
	}

	send_request_return_result (item, stream, NULL);
}

static void try_run_until_read (SoupMessageQueueItem *item);

static gboolean
read_ready_cb (SoupMessage *msg, gpointer user_data)
{
	SoupMessageQueueItem *item = user_data;

	g_clear_pointer (&item->io_source, g_source_unref);
	try_run_until_read (item);
	return FALSE;
}

static void
try_run_until_read (SoupMessageQueueItem *item)
{
	GError *error = NULL;
	GInputStream *stream = NULL;

	if (soup_message_io_run_until_read (item->msg, item->cancellable, &error))
		stream = soup_message_io_get_response_istream (item->msg, &error);
	if (stream) {
		send_async_maybe_complete (item, stream);
		return;
	}

	if (g_error_matches (error, SOUP_HTTP_ERROR, SOUP_STATUS_TRY_AGAIN)) {
		item->state = SOUP_MESSAGE_RESTARTING;
		soup_message_io_finished (item->msg);
		g_error_free (error);
		return;
	}

	if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK)) {
		if (item->state != SOUP_MESSAGE_FINISHED) {
			gboolean dummy;

			if (soup_message_io_in_progress (item->msg))
				soup_message_io_finished (item->msg);
			item->state = SOUP_MESSAGE_FINISHING;
			process_queue_item (item, &dummy, FALSE);
		}
		send_request_return_result (item, NULL, error);
		return;
	}

	g_clear_error (&error);
	item->io_source = soup_message_io_get_source (item->msg, item->cancellable,
						      read_ready_cb, item);
	g_source_attach (item->io_source, soup_session_get_async_context (item->session));
}

static void
send_request_running (SoupSession *session, SoupMessageQueueItem *item)
{
	item->io_started = TRUE;
	try_run_until_read (item);
}

void
soup_session_send_request_async (SoupSession         *session,
				 SoupMessage         *msg,
				 GCancellable        *cancellable,
				 GAsyncReadyCallback  callback,
				 gpointer             user_data)
{
	SoupMessageQueueItem *item;
	gboolean use_thread_context;

	g_return_if_fail (SOUP_IS_SESSION_ASYNC (session));

	g_object_get (G_OBJECT (session),
		      SOUP_SESSION_USE_THREAD_CONTEXT, &use_thread_context,
		      NULL);
	g_return_if_fail (use_thread_context);

	soup_session_async_queue_message (session, msg, NULL, NULL);

	item = soup_message_queue_lookup (soup_session_get_queue (session), msg);
	g_return_if_fail (item != NULL);

	item->new_api = TRUE;
	item->task = g_task_new (session, cancellable, callback, user_data);
	g_task_set_task_data (item->task, item, (GDestroyNotify) soup_message_queue_item_unref);

	if (cancellable) {
		g_object_unref (item->cancellable);
		item->cancellable = g_object_ref (cancellable);
	}
}

GInputStream *
soup_session_send_request_finish (SoupSession   *session,
				  GAsyncResult  *result,
				  GError       **error)
{
	g_return_val_if_fail (SOUP_IS_SESSION_ASYNC (session), NULL);
	g_return_val_if_fail (g_task_is_valid (result, session), NULL);

	return g_task_propagate_pointer (G_TASK (result), error);
}

static void
soup_session_async_class_init (SoupSessionAsyncClass *soup_session_async_class)
{
	SoupSessionClass *session_class = SOUP_SESSION_CLASS (soup_session_async_class);
	GObjectClass *object_class = G_OBJECT_CLASS (session_class);

	g_type_class_add_private (soup_session_async_class,
				  sizeof (SoupSessionAsyncPrivate));

	/* virtual method override */
	session_class->queue_message = soup_session_async_queue_message;
	session_class->send_message = soup_session_async_send_message;
	session_class->cancel_message = soup_session_async_cancel_message;
	session_class->auth_required = soup_session_async_auth_required;
	session_class->kick = soup_session_async_kick;

	object_class->dispose = soup_session_async_dispose;
}
