/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-connection.c: A single HTTP/HTTPS connection
 *
 * Copyright (C) 2000-2003, Ximian, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "soup-connection.h"
#include "soup.h"
#include "soup-marshal.h"
#include "soup-message-queue.h"
#include "soup-misc-private.h"

typedef struct {
	SoupSocket  *socket;

	SoupURI *remote_uri, *proxy_uri;
	SoupProxyURIResolver *proxy_resolver;
	gboolean use_gproxyresolver;
	GTlsDatabase *tlsdb;
	gboolean ssl, ssl_strict, ssl_fallback;

	GMainContext *async_context;
	gboolean      use_thread_context;

	SoupMessage *current_msg;
	SoupConnectionState state;
	time_t       unused_timeout;
	guint        io_timeout, idle_timeout;
	GSource     *idle_timeout_src;
	gboolean     reusable;
} SoupConnectionPrivate;
#define SOUP_CONNECTION_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), SOUP_TYPE_CONNECTION, SoupConnectionPrivate))

G_DEFINE_TYPE (SoupConnection, soup_connection, G_TYPE_OBJECT)

enum {
	EVENT,
	DISCONNECTED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

enum {
	PROP_0,

	PROP_REMOTE_URI,
	PROP_PROXY_RESOLVER,
	PROP_SSL,
	PROP_SSL_CREDS,
	PROP_SSL_STRICT,
	PROP_SSL_FALLBACK,
	PROP_ASYNC_CONTEXT,
	PROP_USE_THREAD_CONTEXT,
	PROP_TIMEOUT,
	PROP_IDLE_TIMEOUT,
	PROP_STATE,

	LAST_PROP
};

static void stop_idle_timer (SoupConnectionPrivate *priv);

/* Number of seconds after which we close a connection that hasn't yet
 * been used.
 */
#define SOUP_CONNECTION_UNUSED_TIMEOUT 3

static void
soup_connection_init (SoupConnection *conn)
{
}

static void
soup_connection_finalize (GObject *object)
{
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (object);

	g_clear_pointer (&priv->remote_uri, soup_uri_free);
	g_clear_pointer (&priv->proxy_uri, soup_uri_free);
	g_clear_object (&priv->tlsdb);
	g_clear_object (&priv->proxy_resolver);
	g_clear_pointer (&priv->async_context, g_main_context_unref);

	G_OBJECT_CLASS (soup_connection_parent_class)->finalize (object);
}

static void
soup_connection_dispose (GObject *object)
{
	SoupConnection *conn = SOUP_CONNECTION (object);
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (conn);

	stop_idle_timer (priv);

	if (priv->socket) {
		g_warning ("Disposing connection while connected");
		soup_connection_disconnect (conn);
	}

	G_OBJECT_CLASS (soup_connection_parent_class)->dispose (object);
}

static void
soup_connection_set_property (GObject *object, guint prop_id,
			      const GValue *value, GParamSpec *pspec)
{
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (object);
	SoupProxyURIResolver *proxy_resolver;

	switch (prop_id) {
	case PROP_REMOTE_URI:
		priv->remote_uri = g_value_dup_boxed (value);
		break;
	case PROP_PROXY_RESOLVER:
		proxy_resolver = g_value_get_object (value);
		if (proxy_resolver && SOUP_IS_PROXY_RESOLVER_DEFAULT (proxy_resolver))
			priv->use_gproxyresolver = TRUE;
		else if (proxy_resolver)
			priv->proxy_resolver = g_object_ref (proxy_resolver);
		break;
	case PROP_SSL:
		priv->ssl = g_value_get_boolean (value);
		break;
	case PROP_SSL_CREDS:
		if (priv->tlsdb)
			g_object_unref (priv->tlsdb);
		priv->tlsdb = g_value_dup_object (value);
		break;
	case PROP_SSL_STRICT:
		priv->ssl_strict = g_value_get_boolean (value);
		break;
	case PROP_SSL_FALLBACK:
		priv->ssl_fallback = g_value_get_boolean (value);
		break;
	case PROP_ASYNC_CONTEXT:
		priv->async_context = g_value_get_pointer (value);
		if (priv->async_context)
			g_main_context_ref (priv->async_context);
		break;
	case PROP_USE_THREAD_CONTEXT:
		priv->use_thread_context = g_value_get_boolean (value);
		break;
	case PROP_TIMEOUT:
		priv->io_timeout = g_value_get_uint (value);
		break;
	case PROP_IDLE_TIMEOUT:
		priv->idle_timeout = g_value_get_uint (value);
		break;
	case PROP_STATE:
		soup_connection_set_state (SOUP_CONNECTION (object), g_value_get_uint (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
soup_connection_get_property (GObject *object, guint prop_id,
			      GValue *value, GParamSpec *pspec)
{
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (object);

	switch (prop_id) {
	case PROP_REMOTE_URI:
		g_value_set_boxed (value, priv->remote_uri);
		break;
	case PROP_SSL:
		g_value_set_boolean (value, priv->ssl);
		break;
	case PROP_SSL_CREDS:
		g_value_set_object (value, priv->tlsdb);
		break;
	case PROP_SSL_STRICT:
		g_value_set_boolean (value, priv->ssl_strict);
		break;
	case PROP_SSL_FALLBACK:
		g_value_set_boolean (value, priv->ssl_fallback);
		break;
	case PROP_ASYNC_CONTEXT:
		g_value_set_pointer (value, priv->async_context ? g_main_context_ref (priv->async_context) : NULL);
		break;
	case PROP_USE_THREAD_CONTEXT:
		g_value_set_boolean (value, priv->use_thread_context);
		break;
	case PROP_TIMEOUT:
		g_value_set_uint (value, priv->io_timeout);
		break;
	case PROP_IDLE_TIMEOUT:
		g_value_set_uint (value, priv->idle_timeout);
		break;
	case PROP_STATE:
		g_value_set_enum (value, priv->state);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
soup_connection_class_init (SoupConnectionClass *connection_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (connection_class);

	g_type_class_add_private (connection_class, sizeof (SoupConnectionPrivate));

	/* virtual method override */
	object_class->dispose = soup_connection_dispose;
	object_class->finalize = soup_connection_finalize;
	object_class->set_property = soup_connection_set_property;
	object_class->get_property = soup_connection_get_property;

	/* signals */
	signals[EVENT] =
		g_signal_new ("event",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      0,
			      NULL, NULL,
			      NULL,
			      G_TYPE_NONE, 2,
			      G_TYPE_SOCKET_CLIENT_EVENT,
			      G_TYPE_IO_STREAM);
	signals[DISCONNECTED] =
		g_signal_new ("disconnected",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (SoupConnectionClass, disconnected),
			      NULL, NULL,
			      _soup_marshal_NONE__NONE,
			      G_TYPE_NONE, 0);

	/* properties */
	g_object_class_install_property (
		object_class, PROP_REMOTE_URI,
		g_param_spec_boxed (SOUP_CONNECTION_REMOTE_URI,
				    "Remote URI",
				    "The URI of the HTTP server",
				    SOUP_TYPE_URI,
				    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (
		object_class, PROP_PROXY_RESOLVER,
		g_param_spec_object (SOUP_CONNECTION_PROXY_RESOLVER,
				     "Proxy resolver",
				     "SoupProxyURIResolver to use",
				     SOUP_TYPE_PROXY_URI_RESOLVER,
				     G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (
		object_class, PROP_SSL,
		g_param_spec_boolean (SOUP_CONNECTION_SSL,
				      "SSL",
				      "Whether this is an SSL connection",
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (
		object_class, PROP_SSL_CREDS,
		g_param_spec_object (SOUP_CONNECTION_SSL_CREDENTIALS,
				     "SSL credentials",
				     "SSL credentials for this connection",
				     G_TYPE_TLS_DATABASE,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (
		object_class, PROP_SSL_STRICT,
		g_param_spec_boolean (SOUP_CONNECTION_SSL_STRICT,
				      "Strictly validate SSL certificates",
				      "Whether certificate errors should be considered a connection error",
				      TRUE,
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (
		object_class, PROP_SSL_FALLBACK,
		g_param_spec_boolean (SOUP_CONNECTION_SSL_FALLBACK,
				      "SSLv3 fallback",
				      "Use SSLv3 instead of TLS",
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (
		object_class, PROP_ASYNC_CONTEXT,
		g_param_spec_pointer (SOUP_CONNECTION_ASYNC_CONTEXT,
				      "Async GMainContext",
				      "GMainContext to dispatch this connection's async I/O in",
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (
		object_class, PROP_USE_THREAD_CONTEXT,
		g_param_spec_boolean (SOUP_CONNECTION_USE_THREAD_CONTEXT,
				      "Use thread context",
				      "Use g_main_context_get_thread_default",
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (
		object_class, PROP_TIMEOUT,
		g_param_spec_uint (SOUP_CONNECTION_TIMEOUT,
				   "Timeout value",
				   "Value in seconds to timeout a blocking I/O",
				   0, G_MAXUINT, 0,
				   G_PARAM_READWRITE));
	g_object_class_install_property (
		object_class, PROP_IDLE_TIMEOUT,
		g_param_spec_uint (SOUP_CONNECTION_IDLE_TIMEOUT,
				   "Idle Timeout",
				   "Connection lifetime when idle",
				   0, G_MAXUINT, 0,
				   G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (
		object_class, PROP_STATE,
		g_param_spec_enum (SOUP_CONNECTION_STATE,
				   "Connection state",
				   "Current state of connection",
				   SOUP_TYPE_CONNECTION_STATE, SOUP_CONNECTION_NEW,
				   G_PARAM_READWRITE));
}

static void
soup_connection_event (SoupConnection      *conn,
		       GSocketClientEvent   event,
		       GIOStream           *connection)
{
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (conn);

	if (!connection && priv->socket)
		connection = soup_socket_get_connection (priv->socket);

	g_signal_emit (conn, signals[EVENT], 0,
		       event, connection);
}

static gboolean
idle_timeout (gpointer conn)
{
	soup_connection_disconnect (conn);
	return FALSE;
}

static void
start_idle_timer (SoupConnection *conn)
{
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (conn);

	if (priv->idle_timeout > 0 && !priv->idle_timeout_src) {
		priv->idle_timeout_src =
			soup_add_timeout (priv->async_context,
					  priv->idle_timeout * 1000,
					  idle_timeout, conn);
	}
}

static void
stop_idle_timer (SoupConnectionPrivate *priv)
{
	if (priv->idle_timeout_src) {
		g_source_destroy (priv->idle_timeout_src);
		priv->idle_timeout_src = NULL;
	}
}

static void
current_msg_got_body (SoupMessage *msg, gpointer user_data)
{
	SoupConnection *conn = user_data;
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (conn);

	priv->unused_timeout = 0;

	if (priv->proxy_uri &&
	    msg->method == SOUP_METHOD_CONNECT &&
	    SOUP_STATUS_IS_SUCCESSFUL (msg->status_code)) {
		soup_connection_event (conn, G_SOCKET_CLIENT_PROXY_NEGOTIATED, NULL);

		/* We're now effectively no longer proxying */
		g_clear_pointer (&priv->proxy_uri, soup_uri_free);
	}

	priv->reusable = soup_message_is_keepalive (msg);
}

static void
clear_current_msg (SoupConnection *conn)
{
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (conn);
	SoupMessage *msg;

	msg = priv->current_msg;
	priv->current_msg = NULL;

	g_signal_handlers_disconnect_by_func (msg, G_CALLBACK (current_msg_got_body), conn);
	g_object_unref (msg);
}

static void
set_current_msg (SoupConnection *conn, SoupMessage *msg)
{
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (conn);

	g_return_if_fail (priv->state == SOUP_CONNECTION_IN_USE);

	g_object_freeze_notify (G_OBJECT (conn));

	if (priv->current_msg) {
		g_return_if_fail (priv->current_msg->method == SOUP_METHOD_CONNECT);
		clear_current_msg (conn);
	}

	stop_idle_timer (priv);

	priv->current_msg = g_object_ref (msg);
	priv->reusable = FALSE;

	g_signal_connect (msg, "got-body",
			  G_CALLBACK (current_msg_got_body), conn);

	if (priv->proxy_uri && msg->method == SOUP_METHOD_CONNECT)
		soup_connection_event (conn, G_SOCKET_CLIENT_PROXY_NEGOTIATING, NULL);

	g_object_thaw_notify (G_OBJECT (conn));
}

static void
proxy_socket_event (SoupSocket          *socket,
		    GSocketClientEvent   event,
		    GIOStream           *connection,
		    gpointer             user_data)
{
	SoupConnection *conn = user_data;

	/* We handle COMPLETE ourselves */
	if (event != G_SOCKET_CLIENT_COMPLETE)
		soup_connection_event (conn, event, connection);
}

typedef struct {
	SoupConnection *conn;
	SoupConnectionCallback callback;
	gpointer callback_data;
	GCancellable *cancellable;
	guint event_id;
} SoupConnectionAsyncConnectData;

static void
socket_connect_finished (SoupSocket *socket, guint status, gpointer user_data)
{
	SoupConnectionAsyncConnectData *data = user_data;
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (data->conn);

	g_signal_handler_disconnect (socket, data->event_id);

	if (SOUP_STATUS_IS_SUCCESSFUL (status)) {
		if (priv->ssl && !priv->proxy_uri) {
			soup_connection_event (data->conn,
					       G_SOCKET_CLIENT_TLS_HANDSHAKED,
					       NULL);
		}
		if (!priv->ssl || !priv->proxy_uri) {
			soup_connection_event (data->conn,
					       G_SOCKET_CLIENT_COMPLETE,
					       NULL);
		}

		soup_connection_set_state (data->conn, SOUP_CONNECTION_IN_USE);
		priv->unused_timeout = time (NULL) + SOUP_CONNECTION_UNUSED_TIMEOUT;
		start_idle_timer (data->conn);
	} else if (status == SOUP_STATUS_TLS_FAILED) {
		priv->ssl_fallback = TRUE;
		status = SOUP_STATUS_TRY_AGAIN;
	}

	if (data->callback) {
		if (priv->proxy_uri != NULL)
			status = soup_status_proxify (status);
		data->callback (data->conn, status, data->callback_data);
	}
	if (!SOUP_STATUS_IS_SUCCESSFUL (status) && status != SOUP_STATUS_TRY_AGAIN)
		soup_connection_disconnect (data->conn);
	g_object_unref (data->conn);
	if (data->cancellable)
		g_object_unref (data->cancellable);
	g_slice_free (SoupConnectionAsyncConnectData, data);
}

static void
socket_connect_result (SoupSocket *sock, guint status, gpointer user_data)
{
	SoupConnectionAsyncConnectData *data = user_data;
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (data->conn);

	if (!SOUP_STATUS_IS_SUCCESSFUL (status)) {
		socket_connect_finished (sock, status, data);
		return;
	}

	if (priv->use_gproxyresolver)
		priv->proxy_uri = soup_socket_get_http_proxy_uri (priv->socket);

	if (priv->ssl && !priv->proxy_uri) {
		if (soup_socket_start_ssl (sock, data->cancellable)) {
			soup_connection_event (data->conn,
					       G_SOCKET_CLIENT_TLS_HANDSHAKING,
					       NULL);
			soup_socket_handshake_async (sock, data->cancellable,
						     socket_connect_finished, data);
			return;
		}

		status = SOUP_STATUS_SSL_FAILED;
	}

	socket_connect_finished (sock, status, data);
}

static void
connect_async_to_uri (SoupConnectionAsyncConnectData *data, SoupURI *uri)
{
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (data->conn);
	SoupAddress *remote_addr;

	remote_addr = soup_address_new (uri->host, uri->port);
	priv->socket =
		soup_socket_new (SOUP_SOCKET_REMOTE_ADDRESS, remote_addr,
				 SOUP_SOCKET_SSL_CREDENTIALS, priv->tlsdb,
				 SOUP_SOCKET_SSL_STRICT, priv->ssl_strict,
				 SOUP_SOCKET_SSL_FALLBACK, priv->ssl_fallback,
				 SOUP_SOCKET_ASYNC_CONTEXT, priv->async_context,
				 SOUP_SOCKET_USE_THREAD_CONTEXT, priv->use_thread_context,
				 SOUP_SOCKET_USE_PROXY, priv->use_gproxyresolver,
				 SOUP_SOCKET_TIMEOUT, priv->io_timeout,
				 SOUP_SOCKET_CLEAN_DISPOSE, TRUE,
				 NULL);
	g_object_unref (remote_addr);

	data->event_id = g_signal_connect (priv->socket, "event",
					   G_CALLBACK (proxy_socket_event),
					   data->conn);

	soup_socket_connect_async (priv->socket, data->cancellable,
				   socket_connect_result, data);
}

static void
proxy_resolver_result (SoupProxyURIResolver *resolver,
		       guint status, SoupURI *proxy_uri,
		       gpointer user_data)
{
	SoupConnectionAsyncConnectData *data = user_data;
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (data->conn);

	if (status != SOUP_STATUS_OK) {
		socket_connect_finished (NULL, status, data);
		return;
	}

	if (proxy_uri) {
		priv->proxy_uri = soup_uri_copy (proxy_uri);
		connect_async_to_uri (data, proxy_uri);
	} else 
		connect_async_to_uri (data, priv->remote_uri);
}

void
soup_connection_connect_async (SoupConnection *conn,
			       GCancellable *cancellable,
			       SoupConnectionCallback callback,
			       gpointer user_data)
{
	SoupConnectionAsyncConnectData *data;
	SoupConnectionPrivate *priv;
	GMainContext *async_context;

	g_return_if_fail (SOUP_IS_CONNECTION (conn));
	priv = SOUP_CONNECTION_GET_PRIVATE (conn);
	g_return_if_fail (priv->socket == NULL);

	soup_connection_set_state (conn, SOUP_CONNECTION_CONNECTING);

	data = g_slice_new (SoupConnectionAsyncConnectData);
	data->conn = g_object_ref (conn);
	data->callback = callback;
	data->callback_data = user_data;
	data->cancellable = cancellable ? g_object_ref (cancellable) : NULL;

	if (!priv->proxy_resolver) {
		connect_async_to_uri (data, priv->remote_uri);
		return;
	}

	if (priv->use_thread_context)
		async_context = g_main_context_get_thread_default ();
	else
		async_context = priv->async_context;

	soup_proxy_uri_resolver_get_proxy_uri_async (priv->proxy_resolver,
						     priv->remote_uri,
						     async_context,
						     cancellable,
						     proxy_resolver_result,
						     data);
}

guint
soup_connection_connect_sync (SoupConnection *conn, GCancellable *cancellable)
{
	SoupConnectionPrivate *priv;
	guint status, event_id;
	SoupURI *connect_uri;
	SoupAddress *remote_addr;

	g_return_val_if_fail (SOUP_IS_CONNECTION (conn), SOUP_STATUS_MALFORMED);
	priv = SOUP_CONNECTION_GET_PRIVATE (conn);
	g_return_val_if_fail (priv->socket == NULL, SOUP_STATUS_MALFORMED);

	soup_connection_set_state (conn, SOUP_CONNECTION_CONNECTING);

	if (priv->proxy_resolver) {
		status = soup_proxy_uri_resolver_get_proxy_uri_sync (priv->proxy_resolver,
								     priv->remote_uri,
								     cancellable,
								     &priv->proxy_uri);
		if (status != SOUP_STATUS_OK)
			goto fail;

		if (priv->proxy_uri)
			connect_uri = priv->proxy_uri;
		else
			connect_uri = priv->remote_uri;
	} else
		connect_uri = priv->remote_uri;

	remote_addr = soup_address_new (connect_uri->host, connect_uri->port);
	priv->socket =
		soup_socket_new (SOUP_SOCKET_REMOTE_ADDRESS, remote_addr,
				 SOUP_SOCKET_USE_PROXY, priv->use_gproxyresolver,
				 SOUP_SOCKET_SSL_CREDENTIALS, priv->tlsdb,
				 SOUP_SOCKET_SSL_STRICT, priv->ssl_strict,
				 SOUP_SOCKET_SSL_FALLBACK, priv->ssl_fallback,
				 SOUP_SOCKET_FLAG_NONBLOCKING, FALSE,
				 SOUP_SOCKET_TIMEOUT, priv->io_timeout,
				 SOUP_SOCKET_CLEAN_DISPOSE, TRUE,
				 NULL);
	g_object_unref (remote_addr);
	event_id = g_signal_connect (priv->socket, "event",
				     G_CALLBACK (proxy_socket_event), conn);
	status = soup_socket_connect_sync (priv->socket, cancellable);

	if (!SOUP_STATUS_IS_SUCCESSFUL (status))
		goto fail;

	if (priv->use_gproxyresolver)
		priv->proxy_uri = soup_socket_get_http_proxy_uri (priv->socket);

	if (priv->ssl && !priv->proxy_uri) {
		if (!soup_socket_start_ssl (priv->socket, cancellable))
			status = SOUP_STATUS_SSL_FAILED;
		else {
			soup_connection_event (conn,
					       G_SOCKET_CLIENT_TLS_HANDSHAKING,
					       NULL);
			status = soup_socket_handshake_sync (priv->socket, cancellable);
			if (status == SOUP_STATUS_OK) {
				soup_connection_event (conn,
						       G_SOCKET_CLIENT_TLS_HANDSHAKED,
						       NULL);
			} else if (status == SOUP_STATUS_TLS_FAILED) {
				priv->ssl_fallback = TRUE;
				status = SOUP_STATUS_TRY_AGAIN;
			}
		}
	}

	if (SOUP_STATUS_IS_SUCCESSFUL (status)) {
		if (!priv->ssl || !priv->proxy_uri) {
			soup_connection_event (conn,
					       G_SOCKET_CLIENT_COMPLETE,
					       NULL);
		}
		soup_connection_set_state (conn, SOUP_CONNECTION_IN_USE);
		priv->unused_timeout = time (NULL) + SOUP_CONNECTION_UNUSED_TIMEOUT;
		start_idle_timer (conn);
	} else if (status != SOUP_STATUS_TRY_AGAIN) {
	fail:
		if (priv->socket) {
			soup_socket_disconnect (priv->socket);
			g_object_unref (priv->socket);
			priv->socket = NULL;
		}

		soup_connection_disconnect (conn);
	}

	if (priv->socket)
		g_signal_handler_disconnect (priv->socket, event_id);

	if (priv->proxy_uri != NULL)
		status = soup_status_proxify (status);
	return status;
}

gboolean
soup_connection_is_tunnelled (SoupConnection *conn)
{
	SoupConnectionPrivate *priv;

	g_return_val_if_fail (SOUP_IS_CONNECTION (conn), FALSE);
	priv = SOUP_CONNECTION_GET_PRIVATE (conn);

	return priv->ssl && priv->proxy_uri != NULL;
}

guint
soup_connection_start_ssl_sync (SoupConnection *conn,
				GCancellable   *cancellable)
{
	SoupConnectionPrivate *priv;
	guint status;

	g_return_val_if_fail (SOUP_IS_CONNECTION (conn), FALSE);
	priv = SOUP_CONNECTION_GET_PRIVATE (conn);

	if (!soup_socket_start_proxy_ssl (priv->socket,
					  priv->remote_uri->host,
					  cancellable)) {
		soup_connection_disconnect (conn);
		return SOUP_STATUS_SSL_FAILED;
	}

	soup_connection_event (conn, G_SOCKET_CLIENT_TLS_HANDSHAKING, NULL);
	status = soup_socket_handshake_sync (priv->socket, cancellable);
	if (status == SOUP_STATUS_OK) {
		soup_connection_event (conn, G_SOCKET_CLIENT_TLS_HANDSHAKED, NULL);
		soup_connection_event (conn, G_SOCKET_CLIENT_COMPLETE, NULL);
	} else if (status == SOUP_STATUS_TLS_FAILED) {
		priv->ssl_fallback = TRUE;
		status = SOUP_STATUS_TRY_AGAIN;
	}

	if (!SOUP_STATUS_IS_SUCCESSFUL (status) && status != SOUP_STATUS_TRY_AGAIN)
		soup_connection_disconnect (conn);

	return status;
}

static void
start_ssl_completed (SoupSocket *socket, guint status, gpointer user_data)
{
	SoupConnectionAsyncConnectData *data = user_data;
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (data->conn);

	if (status == SOUP_STATUS_OK) {
		soup_connection_event (data->conn, G_SOCKET_CLIENT_TLS_HANDSHAKED, NULL);
		soup_connection_event (data->conn, G_SOCKET_CLIENT_COMPLETE, NULL);
	} else if (status == SOUP_STATUS_TLS_FAILED) {
		priv->ssl_fallback = TRUE;
		status = SOUP_STATUS_TRY_AGAIN;
	}

	data->callback (data->conn, status, data->callback_data);
	if (!SOUP_STATUS_IS_SUCCESSFUL (status) && status != SOUP_STATUS_TRY_AGAIN)
		soup_connection_disconnect (data->conn);
	g_object_unref (data->conn);
	g_slice_free (SoupConnectionAsyncConnectData, data);
}

static gboolean
idle_start_ssl_completed (gpointer user_data)
{
	SoupConnectionAsyncConnectData *data = user_data;

	start_ssl_completed (NULL, SOUP_STATUS_SSL_FAILED, data);
	return FALSE;
}

void
soup_connection_start_ssl_async (SoupConnection   *conn,
				 GCancellable     *cancellable,
				 SoupConnectionCallback callback,
				 gpointer          user_data)
{
	SoupConnectionPrivate *priv;
	SoupConnectionAsyncConnectData *data;
	GMainContext *async_context;

	g_return_if_fail (SOUP_IS_CONNECTION (conn));
	priv = SOUP_CONNECTION_GET_PRIVATE (conn);

	data = g_slice_new (SoupConnectionAsyncConnectData);
	data->conn = g_object_ref (conn);
	data->callback = callback;
	data->callback_data = user_data;

	if (priv->use_thread_context)
		async_context = g_main_context_get_thread_default ();
	else
		async_context = priv->async_context;

	if (!soup_socket_start_proxy_ssl (priv->socket,
					  priv->remote_uri->host,
					  cancellable)) {
		soup_add_completion (async_context,
				     idle_start_ssl_completed, data);
		return;
	}

	soup_connection_event (conn, G_SOCKET_CLIENT_TLS_HANDSHAKING, NULL);
	soup_socket_handshake_async (priv->socket, cancellable,
				     start_ssl_completed, data);
}

/**
 * soup_connection_disconnect:
 * @conn: a connection
 *
 * Disconnects @conn's socket and emits a %disconnected signal.
 * After calling this, @conn will be essentially useless.
 **/
void
soup_connection_disconnect (SoupConnection *conn)
{
	SoupConnectionPrivate *priv;
	SoupConnectionState old_state;

	g_return_if_fail (SOUP_IS_CONNECTION (conn));
	priv = SOUP_CONNECTION_GET_PRIVATE (conn);

	old_state = priv->state;
	if (old_state != SOUP_CONNECTION_DISCONNECTED)
		soup_connection_set_state (conn, SOUP_CONNECTION_DISCONNECTED);

	if (priv->socket) {
		SoupSocket *socket = priv->socket;

		priv->socket = NULL;
		soup_socket_disconnect (socket);
		g_object_unref (socket);
	}

	if (old_state != SOUP_CONNECTION_DISCONNECTED)
		g_signal_emit (conn, signals[DISCONNECTED], 0);
}

SoupSocket *
soup_connection_get_socket (SoupConnection *conn)
{
	g_return_val_if_fail (SOUP_IS_CONNECTION (conn), NULL);

	return SOUP_CONNECTION_GET_PRIVATE (conn)->socket;
}

SoupURI *
soup_connection_get_remote_uri (SoupConnection *conn)
{
	g_return_val_if_fail (SOUP_IS_CONNECTION (conn), NULL);

	return SOUP_CONNECTION_GET_PRIVATE (conn)->remote_uri;
}

SoupURI *
soup_connection_get_proxy_uri (SoupConnection *conn)
{
	g_return_val_if_fail (SOUP_IS_CONNECTION (conn), NULL);

	return SOUP_CONNECTION_GET_PRIVATE (conn)->proxy_uri;
}

gboolean
soup_connection_is_via_proxy (SoupConnection *conn)
{
	g_return_val_if_fail (SOUP_IS_CONNECTION (conn), FALSE);

	return SOUP_CONNECTION_GET_PRIVATE (conn)->proxy_uri != NULL;
}

SoupConnectionState
soup_connection_get_state (SoupConnection *conn)
{
	SoupConnectionPrivate *priv;

	g_return_val_if_fail (SOUP_IS_CONNECTION (conn),
			      SOUP_CONNECTION_DISCONNECTED);
	priv = SOUP_CONNECTION_GET_PRIVATE (conn);

	if (priv->state == SOUP_CONNECTION_IDLE &&
	    g_socket_condition_check (soup_socket_get_gsocket (priv->socket), G_IO_IN))
		soup_connection_set_state (conn, SOUP_CONNECTION_REMOTE_DISCONNECTED);

	if (priv->state == SOUP_CONNECTION_IDLE &&
	    priv->unused_timeout && priv->unused_timeout < time (NULL))
		soup_connection_set_state (conn, SOUP_CONNECTION_REMOTE_DISCONNECTED);

	return priv->state;
}

void
soup_connection_set_state (SoupConnection *conn, SoupConnectionState state)
{
	SoupConnectionPrivate *priv;

	g_return_if_fail (SOUP_IS_CONNECTION (conn));
	g_return_if_fail (state >= SOUP_CONNECTION_NEW &&
			  state <= SOUP_CONNECTION_DISCONNECTED);

	g_object_freeze_notify (G_OBJECT (conn));

	priv = SOUP_CONNECTION_GET_PRIVATE (conn);

	if (priv->current_msg) {
		g_warn_if_fail (state == SOUP_CONNECTION_IDLE ||
				state == SOUP_CONNECTION_DISCONNECTED);
		clear_current_msg (conn);
	}

	if (state == SOUP_CONNECTION_IDLE && !priv->reusable) {
		/* This will recursively call set_state() */
		soup_connection_disconnect (conn);
	} else {
		priv->state = state;

		if (priv->state == SOUP_CONNECTION_IDLE)
			start_idle_timer (conn);

		g_object_notify (G_OBJECT (conn), "state");
	}

	g_object_thaw_notify (G_OBJECT (conn));
}

gboolean
soup_connection_get_ever_used (SoupConnection *conn)
{
	g_return_val_if_fail (SOUP_IS_CONNECTION (conn), FALSE);

	return SOUP_CONNECTION_GET_PRIVATE (conn)->unused_timeout == 0;
}

gboolean
soup_connection_get_ssl_fallback (SoupConnection *conn)
{
	return SOUP_CONNECTION_GET_PRIVATE (conn)->ssl_fallback;
}

void
soup_connection_send_request (SoupConnection          *conn,
			      SoupMessageQueueItem    *item,
			      SoupMessageCompletionFn  completion_cb,
			      gpointer                 user_data)
{
	SoupConnectionPrivate *priv;

	g_return_if_fail (SOUP_IS_CONNECTION (conn));
	g_return_if_fail (item != NULL);
	priv = SOUP_CONNECTION_GET_PRIVATE (conn);
	g_return_if_fail (priv->state != SOUP_CONNECTION_NEW &&
			  priv->state != SOUP_CONNECTION_DISCONNECTED);

	if (item->msg != priv->current_msg)
		set_current_msg (conn, item->msg);
	soup_message_send_request (item, completion_cb, user_data);
}
