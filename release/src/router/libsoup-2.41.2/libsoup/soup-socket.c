/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-socket.c: Socket networking code.
 *
 * Copyright (C) 2000-2003, Ximian, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "soup-socket.h"
#include "soup.h"
#include "soup-filter-input-stream.h"
#include "soup-io-stream.h"
#include "soup-marshal.h"
#include "soup-misc-private.h"

/**
 * SECTION:soup-socket
 * @short_description: A network socket
 *
 * #SoupSocket is libsoup's TCP socket type. While it is primarily
 * intended for internal use, #SoupSocket<!-- -->s are exposed in the
 * API in various places, and some of their methods (eg,
 * soup_socket_get_remote_address()) may be useful to applications.
 **/

G_DEFINE_TYPE (SoupSocket, soup_socket, G_TYPE_OBJECT)

enum {
	READABLE,
	WRITABLE,
	DISCONNECTED,
	NEW_CONNECTION,
	EVENT,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

enum {
	PROP_0,

	PROP_LOCAL_ADDRESS,
	PROP_REMOTE_ADDRESS,
	PROP_NON_BLOCKING,
	PROP_IS_SERVER,
	PROP_SSL_CREDENTIALS,
	PROP_SSL_STRICT,
	PROP_SSL_FALLBACK,
	PROP_ASYNC_CONTEXT,
	PROP_USE_THREAD_CONTEXT,
	PROP_TIMEOUT,
	PROP_TRUSTED_CERTIFICATE,
	PROP_CLEAN_DISPOSE,
	PROP_TLS_CERTIFICATE,
	PROP_TLS_ERRORS,
	PROP_USE_PROXY,

	LAST_PROP
};

typedef struct {
	SoupAddress *local_addr, *remote_addr;
	GIOStream *conn, *iostream;
	GSocket *gsock;
	GInputStream *istream;
	GOutputStream *ostream;
	GTlsCertificateFlags tls_errors;

	guint non_blocking:1;
	guint is_server:1;
	guint ssl:1;
	guint ssl_strict:1;
	guint ssl_fallback:1;
	guint clean_dispose:1;
	guint use_thread_context:1;
	guint use_proxy:1;
	gpointer ssl_creds;

	GMainContext   *async_context;
	GSource        *watch_src;
	GSource        *read_src, *write_src;

	GMutex iolock, addrlock;
	guint timeout;

	GCancellable *connect_cancel;
} SoupSocketPrivate;
#define SOUP_SOCKET_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), SOUP_TYPE_SOCKET, SoupSocketPrivate))

static void soup_socket_peer_certificate_changed (GObject *conn,
						  GParamSpec *pspec,
						  gpointer user_data);

static void
soup_socket_init (SoupSocket *sock)
{
	SoupSocketPrivate *priv = SOUP_SOCKET_GET_PRIVATE (sock);

	priv->non_blocking = TRUE;
	g_mutex_init (&priv->addrlock);
	g_mutex_init (&priv->iolock);
}

static void
disconnect_internal (SoupSocket *sock, gboolean close)
{
	SoupSocketPrivate *priv = SOUP_SOCKET_GET_PRIVATE (sock);

	g_clear_object (&priv->gsock);
	if (priv->conn && close)
		g_io_stream_close (priv->conn, NULL, NULL);

	if (priv->read_src) {
		g_source_destroy (priv->read_src);
		priv->read_src = NULL;
	}
	if (priv->write_src) {
		g_source_destroy (priv->write_src);
		priv->write_src = NULL;
	}
}

static void
soup_socket_finalize (GObject *object)
{
	SoupSocketPrivate *priv = SOUP_SOCKET_GET_PRIVATE (object);

	if (priv->connect_cancel) {
		if (priv->clean_dispose)
			g_warning ("Disposing socket %p during connect", object);
		g_object_unref (priv->connect_cancel);
	}
	if (priv->gsock) {
		if (priv->clean_dispose)
			g_warning ("Disposing socket %p while still connected", object);
		disconnect_internal (SOUP_SOCKET (object), TRUE);
	}

	g_clear_object (&priv->conn);
	g_clear_object (&priv->iostream);
	g_clear_object (&priv->istream);
	g_clear_object (&priv->ostream);

	g_clear_object (&priv->local_addr);
	g_clear_object (&priv->remote_addr);

	if (priv->watch_src) {
		if (priv->clean_dispose && !priv->is_server)
			g_warning ("Disposing socket %p during async op", object);
		g_source_destroy (priv->watch_src);
	}
	g_clear_pointer (&priv->async_context, g_main_context_unref);

	g_mutex_clear (&priv->addrlock);
	g_mutex_clear (&priv->iolock);

	G_OBJECT_CLASS (soup_socket_parent_class)->finalize (object);
}


static void
finish_socket_setup (SoupSocketPrivate *priv)
{
	if (!priv->gsock)
		return;

	if (!priv->conn)
		priv->conn = (GIOStream *)g_socket_connection_factory_create_connection (priv->gsock);
	if (!priv->iostream)
		priv->iostream = soup_io_stream_new (priv->conn, FALSE);
	if (!priv->istream)
		priv->istream = g_object_ref (g_io_stream_get_input_stream (priv->iostream));
	if (!priv->ostream)
		priv->ostream = g_object_ref (g_io_stream_get_output_stream (priv->iostream));

	g_socket_set_timeout (priv->gsock, priv->timeout);
}

static void
soup_socket_set_property (GObject *object, guint prop_id,
			  const GValue *value, GParamSpec *pspec)
{
	SoupSocketPrivate *priv = SOUP_SOCKET_GET_PRIVATE (object);

	switch (prop_id) {
	case PROP_LOCAL_ADDRESS:
		priv->local_addr = (SoupAddress *)g_value_dup_object (value);
		break;
	case PROP_REMOTE_ADDRESS:
		priv->remote_addr = (SoupAddress *)g_value_dup_object (value);
		break;
	case PROP_NON_BLOCKING:
		priv->non_blocking = g_value_get_boolean (value);
		break;
	case PROP_SSL_CREDENTIALS:
		priv->ssl_creds = g_value_get_pointer (value);
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
		priv->timeout = g_value_get_uint (value);
		if (priv->conn)
			g_socket_set_timeout (priv->gsock, priv->timeout);
		break;
	case PROP_USE_PROXY:
		priv->use_proxy = g_value_get_boolean (value);
		break;
	case PROP_CLEAN_DISPOSE:
		priv->clean_dispose = g_value_get_boolean (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
soup_socket_get_property (GObject *object, guint prop_id,
			  GValue *value, GParamSpec *pspec)
{
	SoupSocketPrivate *priv = SOUP_SOCKET_GET_PRIVATE (object);

	switch (prop_id) {
	case PROP_LOCAL_ADDRESS:
		g_value_set_object (value, soup_socket_get_local_address (SOUP_SOCKET (object)));
		break;
	case PROP_REMOTE_ADDRESS:
		g_value_set_object (value, soup_socket_get_remote_address (SOUP_SOCKET (object)));
		break;
	case PROP_NON_BLOCKING:
		g_value_set_boolean (value, priv->non_blocking);
		break;
	case PROP_IS_SERVER:
		g_value_set_boolean (value, priv->is_server);
		break;
	case PROP_SSL_CREDENTIALS:
		g_value_set_pointer (value, priv->ssl_creds);
		break;
	case PROP_SSL_STRICT:
		g_value_set_boolean (value, priv->ssl_strict);
		break;
	case PROP_SSL_FALLBACK:
		g_value_set_boolean (value, priv->ssl_fallback);
		break;
	case PROP_TRUSTED_CERTIFICATE:
		g_value_set_boolean (value, priv->tls_errors == 0);
		break;
	case PROP_ASYNC_CONTEXT:
		g_value_set_pointer (value, priv->async_context ? g_main_context_ref (priv->async_context) : NULL);
		break;
	case PROP_USE_THREAD_CONTEXT:
		g_value_set_boolean (value, priv->use_thread_context);
		break;
	case PROP_TIMEOUT:
		g_value_set_uint (value, priv->timeout);
		break;
	case PROP_TLS_CERTIFICATE:
		if (G_IS_TLS_CONNECTION (priv->conn))
			g_value_set_object (value, g_tls_connection_get_peer_certificate (G_TLS_CONNECTION (priv->conn)));
		else
			g_value_set_object (value, NULL);
		break;
	case PROP_TLS_ERRORS:
		g_value_set_flags (value, priv->tls_errors);
		break;
	case PROP_USE_PROXY:
		g_value_set_boolean (value, priv->use_proxy);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
soup_socket_class_init (SoupSocketClass *socket_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (socket_class);

	g_type_class_add_private (socket_class, sizeof (SoupSocketPrivate));

	/* virtual method override */
	object_class->finalize = soup_socket_finalize;
	object_class->set_property = soup_socket_set_property;
	object_class->get_property = soup_socket_get_property;

	/* signals */

	/**
	 * SoupSocket::readable:
	 * @sock: the socket
	 *
	 * Emitted when an async socket is readable. See
	 * soup_socket_read(), soup_socket_read_until() and
	 * #SoupSocket:non-blocking.
	 **/
	signals[READABLE] =
		g_signal_new ("readable",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (SoupSocketClass, readable),
			      NULL, NULL,
			      _soup_marshal_NONE__NONE,
			      G_TYPE_NONE, 0);

	/**
	 * SoupSocket::writable:
	 * @sock: the socket
	 *
	 * Emitted when an async socket is writable. See
	 * soup_socket_write() and #SoupSocket:non-blocking.
	 **/
	signals[WRITABLE] =
		g_signal_new ("writable",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (SoupSocketClass, writable),
			      NULL, NULL,
			      _soup_marshal_NONE__NONE,
			      G_TYPE_NONE, 0);

	/**
	 * SoupSocket::disconnected:
	 * @sock: the socket
	 *
	 * Emitted when the socket is disconnected, for whatever
	 * reason.
	 **/
	signals[DISCONNECTED] =
		g_signal_new ("disconnected",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (SoupSocketClass, disconnected),
			      NULL, NULL,
			      _soup_marshal_NONE__NONE,
			      G_TYPE_NONE, 0);

	/**
	 * SoupSocket::new-connection:
	 * @sock: the socket
	 * @new: the new socket
	 *
	 * Emitted when a listening socket (set up with
	 * soup_socket_listen()) receives a new connection.
	 *
	 * You must ref the @new if you want to keep it; otherwise it
	 * will be destroyed after the signal is emitted.
	 **/
	signals[NEW_CONNECTION] =
		g_signal_new ("new_connection",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (SoupSocketClass, new_connection),
			      NULL, NULL,
			      _soup_marshal_NONE__OBJECT,
			      G_TYPE_NONE, 1,
			      SOUP_TYPE_SOCKET);
	/**
	 * SoupSocket::event:
	 * @sock: the socket
	 * @event: the event that occurred
	 * @connection: the current connection state
	 *
	 * Emitted when a network-related event occurs. See
	 * #GSocketClient::event for more details.
	 *
	 * Since: 2.38
	 **/
	signals[EVENT] =
		g_signal_new ("event",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
			      NULL,
			      G_TYPE_NONE, 2,
			      G_TYPE_SOCKET_CLIENT_EVENT,
			      G_TYPE_IO_STREAM);


	/* properties */
	/**
	 * SOUP_SOCKET_LOCAL_ADDRESS:
	 *
	 * Alias for the #SoupSocket:local-address property. (Address
	 * of local end of socket.)
	 **/
	g_object_class_install_property (
		object_class, PROP_LOCAL_ADDRESS,
		g_param_spec_object (SOUP_SOCKET_LOCAL_ADDRESS,
				     "Local address",
				     "Address of local end of socket",
				     SOUP_TYPE_ADDRESS,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * SOUP_SOCKET_REMOTE_ADDRESS:
	 *
	 * Alias for the #SoupSocket:remote-address property. (Address
	 * of remote end of socket.)
	 **/
	g_object_class_install_property (
		object_class, PROP_REMOTE_ADDRESS,
		g_param_spec_object (SOUP_SOCKET_REMOTE_ADDRESS,
				     "Remote address",
				     "Address of remote end of socket",
				     SOUP_TYPE_ADDRESS,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * SoupSocket:non-blocking:
	 *
	 * Whether or not the socket uses non-blocking I/O.
	 *
	 * #SoupSocket's I/O methods are designed around the idea of
	 * using a single codepath for both synchronous and
	 * asynchronous I/O. If you want to read off a #SoupSocket,
	 * the "correct" way to do it is to call soup_socket_read() or
	 * soup_socket_read_until() repeatedly until you have read
	 * everything you want. If it returns %SOUP_SOCKET_WOULD_BLOCK
	 * at any point, stop reading and wait for it to emit the
	 * #SoupSocket::readable signal. Then go back to the
	 * reading-as-much-as-you-can loop. Likewise, for writing to a
	 * #SoupSocket, you should call soup_socket_write() either
	 * until you have written everything, or it returns
	 * %SOUP_SOCKET_WOULD_BLOCK (in which case you wait for
	 * #SoupSocket::writable and then go back into the loop).
	 *
	 * Code written this way will work correctly with both
	 * blocking and non-blocking sockets; blocking sockets will
	 * simply never return %SOUP_SOCKET_WOULD_BLOCK, and so the
	 * code that handles that case just won't get used for them.
	 **/
	/**
	 * SOUP_SOCKET_FLAG_NONBLOCKING:
	 *
	 * Alias for the #SoupSocket:non-blocking property. (Whether
	 * or not the socket uses non-blocking I/O.)
	 **/
	g_object_class_install_property (
		object_class, PROP_NON_BLOCKING,
		g_param_spec_boolean (SOUP_SOCKET_FLAG_NONBLOCKING,
				      "Non-blocking",
				      "Whether or not the socket uses non-blocking I/O",
				      TRUE,
				      G_PARAM_READWRITE));
	/**
	 * SOUP_SOCKET_IS_SERVER:
	 *
	 * Alias for the #SoupSocket:is-server property. (Whether or
	 * not the socket is a server socket.)
	 **/
	g_object_class_install_property (
		object_class, PROP_IS_SERVER,
		g_param_spec_boolean (SOUP_SOCKET_IS_SERVER,
				      "Server",
				      "Whether or not the socket is a server socket",
				      FALSE,
				      G_PARAM_READABLE));
	/**
	 * SOUP_SOCKET_SSL_CREDENTIALS:
	 *
	 * Alias for the #SoupSocket:ssl-creds property.
	 * (SSL credential information.)
	 **/
	/* For historical reasons, there's only a single property
	 * here, which is a GTlsDatabase for client sockets, and
	 * a GTlsCertificate for server sockets. Whee!
	 */
	g_object_class_install_property (
		object_class, PROP_SSL_CREDENTIALS,
		g_param_spec_pointer (SOUP_SOCKET_SSL_CREDENTIALS,
				      "SSL credentials",
				      "SSL credential information, passed from the session to the SSL implementation",
				      G_PARAM_READWRITE));
	/**
	 * SOUP_SOCKET_SSL_STRICT:
	 *
	 * Alias for the #SoupSocket:ssl-strict property.
	 **/
	g_object_class_install_property (
		object_class, PROP_SSL_STRICT,
		g_param_spec_boolean (SOUP_SOCKET_SSL_STRICT,
				      "Strictly validate SSL certificates",
				      "Whether certificate errors should be considered a connection error",
				      TRUE,
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * SOUP_SOCKET_SSL_FALLBACK:
	 *
	 * Alias for the #SoupSocket:ssl-fallback property.
	 **/
	g_object_class_install_property (
		object_class, PROP_SSL_FALLBACK,
		g_param_spec_boolean (SOUP_SOCKET_SSL_FALLBACK,
				      "SSLv3 fallback",
				      "Use SSLv3 instead of TLS (client-side only)",
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * SOUP_SOCKET_TRUSTED_CERTIFICATE:
	 *
	 * Alias for the #SoupSocket:trusted-certificate
	 * property.
	 **/
	g_object_class_install_property (
		object_class, PROP_TRUSTED_CERTIFICATE,
		g_param_spec_boolean (SOUP_SOCKET_TRUSTED_CERTIFICATE,
				     "Trusted Certificate",
				     "Whether the server certificate is trusted, if this is an SSL socket",
				     FALSE,
				     G_PARAM_READABLE));
	/**
	 * SOUP_SOCKET_ASYNC_CONTEXT:
	 *
	 * Alias for the #SoupSocket:async-context property. (The
	 * socket's #GMainContext.)
	 **/
	g_object_class_install_property (
		object_class, PROP_ASYNC_CONTEXT,
		g_param_spec_pointer (SOUP_SOCKET_ASYNC_CONTEXT,
				      "Async GMainContext",
				      "The GMainContext to dispatch this socket's async I/O in",
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	/**
	 * SOUP_SOCKET_USE_THREAD_CONTEXT:
	 *
	 * Alias for the #SoupSocket:use-thread-context property. (Use
	 * g_main_context_get_thread_default())
	 *
	 * Since: 2.38
	 */
	/**
	 * SoupSocket:use-thread-context:
	 *
	 * Use g_main_context_get_thread_default().
	 *
	 * Since: 2.38
	 */
	g_object_class_install_property (
		object_class, PROP_USE_THREAD_CONTEXT,
		g_param_spec_boolean (SOUP_SOCKET_USE_THREAD_CONTEXT,
				      "Use thread context",
				      "Use g_main_context_get_thread_default",
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	/**
	 * SOUP_SOCKET_TIMEOUT:
	 *
	 * Alias for the #SoupSocket:timeout property. (The timeout
	 * in seconds for blocking socket I/O operations.)
	 **/
	g_object_class_install_property (
		object_class, PROP_TIMEOUT,
		g_param_spec_uint (SOUP_SOCKET_TIMEOUT,
				   "Timeout value",
				   "Value in seconds to timeout a blocking I/O",
				   0, G_MAXUINT, 0,
				   G_PARAM_READWRITE));

	g_object_class_install_property (
		object_class, PROP_CLEAN_DISPOSE,
		g_param_spec_boolean ("clean-dispose",
				      "Clean dispose",
				      "Warn on unclean dispose",
				      FALSE,
				      G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * SOUP_SOCKET_TLS_CERTIFICATE:
	 *
	 * Alias for the #SoupSocket:tls-certificate
	 * property. Note that this property's value is only useful
	 * if the socket is for a TLS connection, and only reliable
	 * after some data has been transferred to or from it.
	 *
	 * Since: 2.34
	 **/
	g_object_class_install_property (
		object_class, PROP_TLS_CERTIFICATE,
		g_param_spec_object (SOUP_SOCKET_TLS_CERTIFICATE,
				     "TLS certificate",
				     "The peer's TLS certificate",
				     G_TYPE_TLS_CERTIFICATE,
				     G_PARAM_READABLE));
	/**
	 * SOUP_SOCKET_TLS_ERRORS:
	 *
	 * Alias for the #SoupSocket:tls-errors
	 * property. Note that this property's value is only useful
	 * if the socket is for a TLS connection, and only reliable
	 * after some data has been transferred to or from it.
	 *
	 * Since: 2.34
	 **/
	g_object_class_install_property (
		object_class, PROP_TLS_ERRORS,
		g_param_spec_flags (SOUP_SOCKET_TLS_ERRORS,
				    "TLS errors",
				    "Errors with the peer's TLS certificate",
				    G_TYPE_TLS_CERTIFICATE_FLAGS, 0,
				    G_PARAM_READABLE));

	g_object_class_install_property (
		object_class, PROP_USE_PROXY,
		g_param_spec_boolean (SOUP_SOCKET_USE_PROXY,
				      "Use proxy",
				      "Use #GProxyResolver",
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}


/**
 * soup_socket_new:
 * @optname1: name of first property to set (or %NULL)
 * @...: value of @optname1, followed by additional property/value pairs
 *
 * Creates a new (disconnected) socket
 *
 * Return value: the new socket
 **/
SoupSocket *
soup_socket_new (const char *optname1, ...)
{
	SoupSocket *sock;
	va_list ap;

	va_start (ap, optname1);
	sock = (SoupSocket *)g_object_new_valist (SOUP_TYPE_SOCKET,
						  optname1, ap);
	va_end (ap);

	return sock;
}

static void
proxy_socket_client_event (GSocketClient       *client,
			   GSocketClientEvent   event,
			   GSocketConnectable  *connectable,
			   GIOStream           *connection,
			   gpointer             user_data)
{
	SoupSocket *sock = user_data;

	g_signal_emit (sock, signals[EVENT], 0,
		       event, connection);
}

static guint
socket_connected (SoupSocket *sock, GSocketConnection *conn, GError *error)
{
	SoupSocketPrivate *priv = SOUP_SOCKET_GET_PRIVATE (sock);

	if (priv->connect_cancel) {
		GCancellable *cancellable = priv->connect_cancel;
		gboolean cancelled = g_cancellable_is_cancelled (cancellable);

		g_object_unref (priv->connect_cancel);
		priv->connect_cancel = NULL;
		if (cancelled) {
			g_clear_error (&error);
			return SOUP_STATUS_CANCELLED;
		}
	}

	if (error) {
		if (error->domain == G_RESOLVER_ERROR) {
			g_error_free (error);
			return SOUP_STATUS_CANT_RESOLVE;
		} else {
			g_error_free (error);
			return SOUP_STATUS_CANT_CONNECT;
		}
	}

	priv->conn = (GIOStream *)conn;
	priv->gsock = g_object_ref (g_socket_connection_get_socket (conn));
	finish_socket_setup (priv);

	return SOUP_STATUS_OK;
}

/**
 * SoupSocketCallback:
 * @sock: the #SoupSocket
 * @status: an HTTP status code indicating success or failure
 * @user_data: the data passed to soup_socket_connect_async()
 *
 * The callback function passed to soup_socket_connect_async().
 **/

typedef struct {
	SoupSocket *sock;
	SoupSocketCallback callback;
	gpointer user_data;
} SoupSocketAsyncConnectData;

static void
async_connected (GObject *client, GAsyncResult *result, gpointer data)
{
	SoupSocketAsyncConnectData *sacd = data;
	SoupSocketPrivate *priv = SOUP_SOCKET_GET_PRIVATE (sacd->sock);
	GError *error = NULL;
	GSocketConnection *conn;
	guint status;

	if (priv->async_context && !priv->use_thread_context)
		g_main_context_pop_thread_default (priv->async_context);

	conn = g_socket_client_connect_finish (G_SOCKET_CLIENT (client),
					       result, &error);
	status = socket_connected (sacd->sock, conn, error);

	sacd->callback (sacd->sock, status, sacd->user_data);
	g_object_unref (sacd->sock);
	g_slice_free (SoupSocketAsyncConnectData, sacd);
}

/**
 * soup_socket_connect_async:
 * @sock: a client #SoupSocket (which must not already be connected)
 * @cancellable: a #GCancellable, or %NULL
 * @callback: (scope async): callback to call after connecting
 * @user_data: data to pass to @callback
 *
 * Begins asynchronously connecting to @sock's remote address. The
 * socket will call @callback when it succeeds or fails (but not
 * before returning from this function).
 *
 * If @cancellable is non-%NULL, it can be used to cancel the
 * connection. @callback will still be invoked in this case, with a
 * status of %SOUP_STATUS_CANCELLED.
 **/
void
soup_socket_connect_async (SoupSocket *sock, GCancellable *cancellable,
			   SoupSocketCallback callback, gpointer user_data)
{
	SoupSocketPrivate *priv;
	SoupSocketAsyncConnectData *sacd;
	GSocketClient *client;

	g_return_if_fail (SOUP_IS_SOCKET (sock));
	priv = SOUP_SOCKET_GET_PRIVATE (sock);
	g_return_if_fail (priv->remote_addr != NULL);

	sacd = g_slice_new0 (SoupSocketAsyncConnectData);
	sacd->sock = g_object_ref (sock);
	sacd->callback = callback;
	sacd->user_data = user_data;

	priv->connect_cancel = cancellable ? g_object_ref (cancellable) : g_cancellable_new ();

	if (priv->async_context && !priv->use_thread_context)
		g_main_context_push_thread_default (priv->async_context);

	client = g_socket_client_new ();
	g_signal_connect (client, "event",
			  G_CALLBACK (proxy_socket_client_event), sock);
	if (priv->use_proxy)
		g_socket_client_add_application_proxy (client, "http");
	else
		g_socket_client_set_enable_proxy (client, FALSE);
	if (priv->timeout)
		g_socket_client_set_timeout (client, priv->timeout);
	g_socket_client_connect_async (client,
				       G_SOCKET_CONNECTABLE (priv->remote_addr),
				       priv->connect_cancel,
				       async_connected, sacd);
	g_object_unref (client);
}

/**
 * soup_socket_connect_sync:
 * @sock: a client #SoupSocket (which must not already be connected)
 * @cancellable: a #GCancellable, or %NULL
 *
 * Attempt to synchronously connect @sock to its remote address.
 *
 * If @cancellable is non-%NULL, it can be used to cancel the
 * connection, in which case soup_socket_connect_sync() will return
 * %SOUP_STATUS_CANCELLED.
 *
 * Return value: a success or failure code.
 **/
guint
soup_socket_connect_sync (SoupSocket *sock, GCancellable *cancellable)
{
	SoupSocketPrivate *priv;
	GSocketClient *client;
	GSocketConnection *conn;
	GError *error = NULL;

	g_return_val_if_fail (SOUP_IS_SOCKET (sock), SOUP_STATUS_MALFORMED);
	priv = SOUP_SOCKET_GET_PRIVATE (sock);
	g_return_val_if_fail (!priv->is_server, SOUP_STATUS_MALFORMED);
	g_return_val_if_fail (priv->gsock == NULL, SOUP_STATUS_MALFORMED);
	g_return_val_if_fail (priv->remote_addr != NULL, SOUP_STATUS_MALFORMED);

	if (cancellable)
		g_object_ref (cancellable);
	else
		cancellable = g_cancellable_new ();
	priv->connect_cancel = cancellable;

	client = g_socket_client_new ();
	g_signal_connect (client, "event",
			  G_CALLBACK (proxy_socket_client_event), sock);
	if (priv->use_proxy)
		g_socket_client_add_application_proxy (client, "http");
	else
		g_socket_client_set_enable_proxy (client, FALSE);
	if (priv->timeout)
		g_socket_client_set_timeout (client, priv->timeout);
	conn = g_socket_client_connect (client,
					G_SOCKET_CONNECTABLE (priv->remote_addr),
					priv->connect_cancel, &error);
	g_object_unref (client);

	return socket_connected (sock, conn, error);
}

int
soup_socket_get_fd (SoupSocket *sock)
{
	g_return_val_if_fail (SOUP_IS_SOCKET (sock), -1);

	return g_socket_get_fd (SOUP_SOCKET_GET_PRIVATE (sock)->gsock);
}

GSocket *
soup_socket_get_gsocket (SoupSocket *sock)
{
	g_return_val_if_fail (SOUP_IS_SOCKET (sock), NULL);

	return SOUP_SOCKET_GET_PRIVATE (sock)->gsock;
}

GIOStream *
soup_socket_get_connection (SoupSocket *sock)
{
	g_return_val_if_fail (SOUP_IS_SOCKET (sock), NULL);

	return SOUP_SOCKET_GET_PRIVATE (sock)->conn;
}

GIOStream *
soup_socket_get_iostream (SoupSocket *sock)
{
	g_return_val_if_fail (SOUP_IS_SOCKET (sock), NULL);

	return SOUP_SOCKET_GET_PRIVATE (sock)->iostream;
}

static GSource *
soup_socket_create_watch (SoupSocketPrivate *priv, GIOCondition cond,
			  GPollableSourceFunc callback, gpointer user_data,
			  GCancellable *cancellable)
{
	GSource *watch;
	GMainContext *async_context;

	if (cond == G_IO_IN)
		watch = g_pollable_input_stream_create_source (G_POLLABLE_INPUT_STREAM (priv->istream), cancellable);
	else
		watch = g_pollable_output_stream_create_source (G_POLLABLE_OUTPUT_STREAM (priv->ostream), cancellable);
	g_source_set_callback (watch, (GSourceFunc)callback, user_data, NULL);

	if (priv->use_thread_context)
		async_context = g_main_context_get_thread_default ();
	else
		async_context = priv->async_context;

	g_source_attach (watch, async_context);
	g_source_unref (watch);

	return watch;
}

static gboolean
listen_watch (GObject *pollable, gpointer data)
{
	SoupSocket *sock = data, *new;
	SoupSocketPrivate *priv = SOUP_SOCKET_GET_PRIVATE (sock), *new_priv;
	GSocket *new_gsock;

	new_gsock = g_socket_accept (priv->gsock, NULL, NULL);
	if (!new_gsock)
		return FALSE;

	new = g_object_new (SOUP_TYPE_SOCKET, NULL);
	new_priv = SOUP_SOCKET_GET_PRIVATE (new);
	new_priv->gsock = new_gsock;
	if (priv->async_context)
		new_priv->async_context = g_main_context_ref (priv->async_context);
	new_priv->use_thread_context = priv->use_thread_context;
	new_priv->non_blocking = priv->non_blocking;
	new_priv->is_server = TRUE;
	new_priv->ssl = priv->ssl;
	if (priv->ssl_creds)
		new_priv->ssl_creds = priv->ssl_creds;
	finish_socket_setup (new_priv);

	if (new_priv->ssl_creds) {
		if (!soup_socket_start_proxy_ssl (new, NULL, NULL)) {
			g_object_unref (new);
			return TRUE;
		}
	}

	g_signal_emit (sock, signals[NEW_CONNECTION], 0, new);
	g_object_unref (new);

	return TRUE;
}

/**
 * soup_socket_listen:
 * @sock: a server #SoupSocket (which must not already be connected or
 * listening)
 *
 * Makes @sock start listening on its local address. When connections
 * come in, @sock will emit #SoupSocket::new_connection.
 *
 * Return value: whether or not @sock is now listening.
 **/
gboolean
soup_socket_listen (SoupSocket *sock)

{
	SoupSocketPrivate *priv;
	GSocketAddress *addr;

	g_return_val_if_fail (SOUP_IS_SOCKET (sock), FALSE);
	priv = SOUP_SOCKET_GET_PRIVATE (sock);
	g_return_val_if_fail (priv->gsock == NULL, FALSE);
	g_return_val_if_fail (priv->local_addr != NULL, FALSE);

	priv->is_server = TRUE;

	/* @local_addr may have its port set to 0. So we intentionally
	 * don't store it in priv->local_addr, so that if the
	 * caller calls soup_socket_get_local_address() later, we'll
	 * have to make a new addr by calling getsockname(), which
	 * will have the right port number.
	 */
	addr = soup_address_get_gsockaddr (priv->local_addr);
	g_return_val_if_fail (addr != NULL, FALSE);

	priv->gsock = g_socket_new (g_socket_address_get_family (addr),
				    G_SOCKET_TYPE_STREAM,
				    G_SOCKET_PROTOCOL_DEFAULT,
				    NULL);
	if (!priv->gsock)
		goto cant_listen;
	finish_socket_setup (priv);

	/* Bind */
	if (!g_socket_bind (priv->gsock, addr, TRUE, NULL))
		goto cant_listen;
	/* Force local_addr to be re-resolved now */
	g_object_unref (priv->local_addr);
	priv->local_addr = NULL;

	/* Listen */
	if (!g_socket_listen (priv->gsock, NULL))
		goto cant_listen;

	priv->watch_src = soup_socket_create_watch (priv, G_IO_IN,
						    listen_watch, sock,
						    NULL);
	g_object_unref (addr);
	return TRUE;

 cant_listen:
	if (priv->conn)
		disconnect_internal (sock, TRUE);
	g_object_unref (addr);

	return FALSE;
}

static void
soup_socket_peer_certificate_changed (GObject *conn, GParamSpec *pspec,
				      gpointer sock)
{
	SoupSocketPrivate *priv = SOUP_SOCKET_GET_PRIVATE (sock);

	priv->tls_errors = g_tls_connection_get_peer_certificate_errors (G_TLS_CONNECTION (priv->conn));

	g_object_notify (sock, "tls-certificate");
	g_object_notify (sock, "tls-errors");
}

static gboolean
soup_socket_accept_certificate (GTlsConnection *conn, GTlsCertificate *cert,
				GTlsCertificateFlags errors, gpointer sock)
{
	return TRUE;
}

/**
 * soup_socket_start_ssl:
 * @sock: the socket
 * @cancellable: a #GCancellable
 *
 * Starts using SSL on @socket.
 *
 * Return value: success or failure
 **/
gboolean
soup_socket_start_ssl (SoupSocket *sock, GCancellable *cancellable)
{
	SoupSocketPrivate *priv = SOUP_SOCKET_GET_PRIVATE (sock);

	return soup_socket_start_proxy_ssl (sock, soup_address_get_name (priv->remote_addr), cancellable);
}
	
/**
 * soup_socket_start_proxy_ssl:
 * @sock: the socket
 * @ssl_host: hostname of the SSL server
 * @cancellable: a #GCancellable
 *
 * Starts using SSL on @socket, expecting to find a host named
 * @ssl_host.
 *
 * Return value: success or failure
 **/
gboolean
soup_socket_start_proxy_ssl (SoupSocket *sock, const char *ssl_host,
			     GCancellable *cancellable)
{
	SoupSocketPrivate *priv = SOUP_SOCKET_GET_PRIVATE (sock);
	GTlsBackend *backend = g_tls_backend_get_default ();

	if (G_IS_TLS_CONNECTION (priv->conn))
		return TRUE;

	if (g_cancellable_is_cancelled (cancellable))
		return FALSE;

	priv->ssl = TRUE;

	if (!priv->is_server) {
		GTlsClientConnection *conn;
		GSocketConnectable *identity;

		identity = g_network_address_new (ssl_host, 0);
		conn = g_initable_new (g_tls_backend_get_client_connection_type (backend),
				       NULL, NULL,
				       "base-io-stream", priv->conn,
				       "server-identity", identity,
				       "database", priv->ssl_creds,
				       "require-close-notify", FALSE,
				       "use-ssl3", priv->ssl_fallback,
				       NULL);
		g_object_unref (identity);

		if (!conn)
			return FALSE;

		g_object_unref (priv->conn);
		priv->conn = G_IO_STREAM (conn);

		if (!priv->ssl_strict) {
			g_signal_connect (conn, "accept-certificate",
					  G_CALLBACK (soup_socket_accept_certificate),
					  sock);
		}
	} else {
		GTlsServerConnection *conn;

		conn = g_initable_new (g_tls_backend_get_server_connection_type (backend),
				       NULL, NULL,
				       "base-io-stream", priv->conn,
				       "certificate", priv->ssl_creds,
				       "use-system-certdb", FALSE,
				       "require-close-notify", FALSE,
				       NULL);
		if (!conn)
			return FALSE;

		g_object_unref (priv->conn);
		priv->conn = G_IO_STREAM (conn);
	}

	g_signal_connect (priv->conn, "notify::peer-certificate",
			  G_CALLBACK (soup_socket_peer_certificate_changed), sock);

	g_clear_object (&priv->istream);
	g_clear_object (&priv->ostream);
	g_clear_object (&priv->iostream);
	priv->iostream = soup_io_stream_new (priv->conn, FALSE);
	priv->istream = g_object_ref (g_io_stream_get_input_stream (priv->iostream));
	priv->ostream = g_object_ref (g_io_stream_get_output_stream (priv->iostream));

	return TRUE;
}
	
guint
soup_socket_handshake_sync (SoupSocket    *sock,
			    GCancellable  *cancellable)
{
	SoupSocketPrivate *priv = SOUP_SOCKET_GET_PRIVATE (sock);
	GError *error = NULL;

	priv->ssl = TRUE;
	if (g_tls_connection_handshake (G_TLS_CONNECTION (priv->conn),
					cancellable, &error))
		return SOUP_STATUS_OK;
	else if (!priv->ssl_fallback &&
		 g_error_matches (error, G_TLS_ERROR, G_TLS_ERROR_NOT_TLS)) {
		g_error_free (error);
		return SOUP_STATUS_TLS_FAILED;
	} else {
		g_error_free (error);
		return SOUP_STATUS_SSL_FAILED;
	}
}

static void
handshake_async_ready (GObject *source, GAsyncResult *result, gpointer user_data)
{
	SoupSocketAsyncConnectData *data = user_data;
	SoupSocketPrivate *priv = SOUP_SOCKET_GET_PRIVATE (data->sock);
	GError *error = NULL;
	guint status;

	if (priv->async_context && !priv->use_thread_context)
		g_main_context_pop_thread_default (priv->async_context);

	if (g_tls_connection_handshake_finish (G_TLS_CONNECTION (source),
					       result, &error))
		status = SOUP_STATUS_OK;
	else if (!priv->ssl_fallback &&
		 g_error_matches (error, G_TLS_ERROR, G_TLS_ERROR_NOT_TLS))
		status = SOUP_STATUS_TLS_FAILED;
	else
		status = SOUP_STATUS_SSL_FAILED;
	g_clear_error (&error);

	data->callback (data->sock, status, data->user_data);
	g_object_unref (data->sock);
	g_slice_free (SoupSocketAsyncConnectData, data);
}

void
soup_socket_handshake_async (SoupSocket         *sock,
			     GCancellable       *cancellable,
			     SoupSocketCallback  callback,
			     gpointer            user_data)
{
	SoupSocketPrivate *priv = SOUP_SOCKET_GET_PRIVATE (sock);
	SoupSocketAsyncConnectData *data;

	priv->ssl = TRUE;

	data = g_slice_new (SoupSocketAsyncConnectData);
	data->sock = g_object_ref (sock);
	data->callback = callback;
	data->user_data = user_data;

	if (priv->async_context && !priv->use_thread_context)
		g_main_context_push_thread_default (priv->async_context);
	g_tls_connection_handshake_async (G_TLS_CONNECTION (priv->conn),
					  G_PRIORITY_DEFAULT,
					  cancellable, handshake_async_ready,
					  data);
}

/**
 * soup_socket_is_ssl:
 * @sock: a #SoupSocket
 *
 * Tests if @sock is doing (or has attempted to do) SSL.
 *
 * Return value: %TRUE if @sock has SSL credentials set
 **/
gboolean
soup_socket_is_ssl (SoupSocket *sock)
{
	SoupSocketPrivate *priv = SOUP_SOCKET_GET_PRIVATE (sock);

	return priv->ssl;
}

/**
 * soup_socket_disconnect:
 * @sock: a #SoupSocket
 *
 * Disconnects @sock. Any further read or write attempts on it will
 * fail.
 **/
void
soup_socket_disconnect (SoupSocket *sock)
{
	SoupSocketPrivate *priv;
	gboolean already_disconnected = FALSE;

	g_return_if_fail (SOUP_IS_SOCKET (sock));
	priv = SOUP_SOCKET_GET_PRIVATE (sock);

	if (priv->connect_cancel) {
		disconnect_internal (sock, FALSE);
		g_cancellable_cancel (priv->connect_cancel);
		return;
	} else if (g_mutex_trylock (&priv->iolock)) {
		if (priv->gsock)
			disconnect_internal (sock, TRUE);
		else
			already_disconnected = TRUE;
		g_mutex_unlock (&priv->iolock);
	} else {
		/* Another thread is currently doing IO, so
		 * we can't close the socket. So just shutdown
		 * the file descriptor to force the I/O to fail.
		 * (It will actually be closed when the socket
		 * is destroyed.)
		 */
		g_socket_shutdown (priv->gsock, TRUE, TRUE, NULL);
	}

	if (already_disconnected)
		return;

	/* Keep ref around signals in case the object is unreferenced
	 * in a handler
	 */
	g_object_ref (sock);

	if (priv->non_blocking) {
		/* Give all readers a chance to notice the connection close */
		g_signal_emit (sock, signals[READABLE], 0);
	}

	/* FIXME: can't disconnect until all data is read */

	/* Then let everyone know we're disconnected */
	g_signal_emit (sock, signals[DISCONNECTED], 0);

	g_object_unref (sock);
}

/**
 * soup_socket_is_connected:
 * @sock: a #SoupSocket
 *
 * Tests if @sock is connected to another host
 *
 * Return value: %TRUE or %FALSE.
 **/
gboolean
soup_socket_is_connected (SoupSocket *sock)
{
	SoupSocketPrivate *priv;

	g_return_val_if_fail (SOUP_IS_SOCKET (sock), FALSE);
	priv = SOUP_SOCKET_GET_PRIVATE (sock);

	return priv->conn && !g_io_stream_is_closed (priv->conn);
}

/**
 * soup_socket_get_local_address:
 * @sock: a #SoupSocket
 *
 * Returns the #SoupAddress corresponding to the local end of @sock.
 *
 * Calling this method on an unconnected socket is considered to be
 * an error, and produces undefined results.
 *
 * Return value: (transfer none): the #SoupAddress
 **/
SoupAddress *
soup_socket_get_local_address (SoupSocket *sock)
{
	SoupSocketPrivate *priv;

	g_return_val_if_fail (SOUP_IS_SOCKET (sock), NULL);
	priv = SOUP_SOCKET_GET_PRIVATE (sock);

	g_mutex_lock (&priv->addrlock);
	if (!priv->local_addr) {
		GSocketAddress *addr;
		struct sockaddr_storage sa;
		gssize sa_len;
		GError *error = NULL;

		if (priv->gsock == NULL) {
			g_warning ("%s: socket not connected", G_STRLOC);
			goto unlock;
		}

		addr = g_socket_get_local_address (priv->gsock, &error);
		if (addr == NULL) {
			g_warning ("%s: %s", G_STRLOC, error->message);
			g_error_free (error);
			goto unlock;
		}
		sa_len = g_socket_address_get_native_size (addr);
		g_socket_address_to_native (addr, &sa, sa_len, NULL);
		priv->local_addr = soup_address_new_from_sockaddr ((struct sockaddr *)&sa, sa_len);
		g_object_unref (addr);
	}
unlock:
	g_mutex_unlock (&priv->addrlock);

	return priv->local_addr;
}

/**
 * soup_socket_get_remote_address:
 * @sock: a #SoupSocket
 *
 * Returns the #SoupAddress corresponding to the remote end of @sock.
 *
 * Calling this method on an unconnected socket is considered to be
 * an error, and produces undefined results.
 *
 * Return value: (transfer none): the #SoupAddress
 **/
SoupAddress *
soup_socket_get_remote_address (SoupSocket *sock)
{
	SoupSocketPrivate *priv;

	g_return_val_if_fail (SOUP_IS_SOCKET (sock), NULL);
	priv = SOUP_SOCKET_GET_PRIVATE (sock);

	g_mutex_lock (&priv->addrlock);
	if (!priv->remote_addr) {
		GSocketAddress *addr;
		struct sockaddr_storage sa;
		gssize sa_len;
		GError *error = NULL;

		if (priv->gsock == NULL) {
			g_warning ("%s: socket not connected", G_STRLOC);
			goto unlock;
		}

		addr = g_socket_get_remote_address (priv->gsock, &error);
		if (addr == NULL) {
			g_warning ("%s: %s", G_STRLOC, error->message);
			g_error_free (error);
			goto unlock;
		}
		sa_len = g_socket_address_get_native_size (addr);
		g_socket_address_to_native (addr, &sa, sa_len, NULL);
		priv->remote_addr = soup_address_new_from_sockaddr ((struct sockaddr *)&sa, sa_len);
		g_object_unref (addr);
	}
unlock:
	g_mutex_unlock (&priv->addrlock);

	return priv->remote_addr;
}

SoupURI *
soup_socket_get_http_proxy_uri (SoupSocket *sock)
{
	SoupSocketPrivate *priv = SOUP_SOCKET_GET_PRIVATE (sock);
	GSocketAddress *addr;
	GProxyAddress *paddr;

	if (!priv->gsock)
		return NULL;
	addr = g_socket_get_remote_address (priv->gsock, NULL);
	if (!addr || !G_IS_PROXY_ADDRESS (addr))
		return NULL;

	paddr = G_PROXY_ADDRESS (addr);
	if (strcmp (g_proxy_address_get_protocol (paddr), "http") != 0)
		return NULL;

	return soup_uri_new (g_proxy_address_get_uri (paddr));
}

static gboolean
socket_read_watch (GObject *pollable, gpointer user_data)
{
	SoupSocket *sock = user_data;
	SoupSocketPrivate *priv = SOUP_SOCKET_GET_PRIVATE (sock);

	priv->read_src = NULL;
	g_signal_emit (sock, signals[READABLE], 0);
	return FALSE;
}

static SoupSocketIOStatus
translate_read_status (SoupSocket *sock, GCancellable *cancellable,
		       gssize my_nread, gsize *nread,
		       GError *my_err, GError **error)
{
	SoupSocketPrivate *priv = SOUP_SOCKET_GET_PRIVATE (sock);

	if (my_nread > 0) {
		g_assert_no_error (my_err);
		*nread = my_nread;
		return SOUP_SOCKET_OK;
	} else if (my_nread == 0) {
		g_assert_no_error (my_err);
		*nread = my_nread;
		return SOUP_SOCKET_EOF;
	} else if (g_error_matches (my_err, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK)) {
		g_clear_error (&my_err);
		if (!priv->read_src) {
			priv->read_src =
				soup_socket_create_watch (priv, G_IO_IN,
							  socket_read_watch, sock,
							  cancellable);
		}
		return SOUP_SOCKET_WOULD_BLOCK;
	}

	g_propagate_error (error, my_err);
	return SOUP_SOCKET_ERROR;
}

/**
 * SoupSocketIOStatus:
 * @SOUP_SOCKET_OK: Success
 * @SOUP_SOCKET_WOULD_BLOCK: Cannot read/write any more at this time
 * @SOUP_SOCKET_EOF: End of file
 * @SOUP_SOCKET_ERROR: Other error
 *
 * Return value from the #SoupSocket IO methods.
 **/

/**
 * soup_socket_read:
 * @sock: the socket
 * @buffer: buffer to read into
 * @len: size of @buffer in bytes
 * @nread: (out): on return, the number of bytes read into @buffer
 * @cancellable: a #GCancellable, or %NULL
 * @error: error pointer
 *
 * Attempts to read up to @len bytes from @sock into @buffer. If some
 * data is successfully read, soup_socket_read() will return
 * %SOUP_SOCKET_OK, and *@nread will contain the number of bytes
 * actually read (which may be less than @len).
 *
 * If @sock is non-blocking, and no data is available, the return
 * value will be %SOUP_SOCKET_WOULD_BLOCK. In this case, the caller
 * can connect to the #SoupSocket::readable signal to know when there
 * is more data to read. (NB: You MUST read all available data off the
 * socket first. #SoupSocket::readable is only emitted after
 * soup_socket_read() returns %SOUP_SOCKET_WOULD_BLOCK, and it is only
 * emitted once. See the documentation for #SoupSocket:non-blocking.)
 *
 * Return value: a #SoupSocketIOStatus, as described above (or
 * %SOUP_SOCKET_EOF if the socket is no longer connected, or
 * %SOUP_SOCKET_ERROR on any other error, in which case @error will
 * also be set).
 **/
SoupSocketIOStatus
soup_socket_read (SoupSocket *sock, gpointer buffer, gsize len,
		  gsize *nread, GCancellable *cancellable, GError **error)
{
	SoupSocketPrivate *priv;
	SoupSocketIOStatus status;
	gssize my_nread;
	GError *my_err = NULL;

	g_return_val_if_fail (SOUP_IS_SOCKET (sock), SOUP_SOCKET_ERROR);
	g_return_val_if_fail (nread != NULL, SOUP_SOCKET_ERROR);

	priv = SOUP_SOCKET_GET_PRIVATE (sock);

	g_mutex_lock (&priv->iolock);

	if (!priv->istream) {
		status = SOUP_SOCKET_EOF;
		goto out;
	}

	if (!priv->non_blocking) {
		my_nread = g_input_stream_read (priv->istream, buffer, len,
						cancellable, &my_err);
	} else {
		my_nread = g_pollable_input_stream_read_nonblocking (G_POLLABLE_INPUT_STREAM (priv->istream),
								     buffer, len,
								     cancellable, &my_err);
	}
	status = translate_read_status (sock, cancellable,
					my_nread, nread, my_err, error);

out:
	g_mutex_unlock (&priv->iolock);

	return status;
}

/**
 * soup_socket_read_until:
 * @sock: the socket
 * @buffer: buffer to read into
 * @len: size of @buffer in bytes
 * @boundary: boundary to read until
 * @boundary_len: length of @boundary in bytes
 * @nread: (out): on return, the number of bytes read into @buffer
 * @got_boundary: on return, whether or not the data in @buffer
 * ends with the boundary string
 * @cancellable: a #GCancellable, or %NULL
 * @error: error pointer
 *
 * Like soup_socket_read(), but reads no further than the first
 * occurrence of @boundary. (If the boundary is found, it will be
 * included in the returned data, and *@got_boundary will be set to
 * %TRUE.) Any data after the boundary will returned in future reads.
 *
 * soup_socket_read_until() will almost always return fewer than @len
 * bytes: if the boundary is found, then it will only return the bytes
 * up until the end of the boundary, and if the boundary is not found,
 * then it will leave the last <literal>(boundary_len - 1)</literal>
 * bytes in its internal buffer, in case they form the start of the
 * boundary string. Thus, @len normally needs to be at least 1 byte
 * longer than @boundary_len if you want to make any progress at all.
 *
 * Return value: as for soup_socket_read()
 **/
SoupSocketIOStatus
soup_socket_read_until (SoupSocket *sock, gpointer buffer, gsize len,
			gconstpointer boundary, gsize boundary_len,
			gsize *nread, gboolean *got_boundary,
			GCancellable *cancellable, GError **error)
{
	SoupSocketPrivate *priv;
	SoupSocketIOStatus status;
	gssize my_nread;
	GError *my_err = NULL;

	g_return_val_if_fail (SOUP_IS_SOCKET (sock), SOUP_SOCKET_ERROR);
	g_return_val_if_fail (nread != NULL, SOUP_SOCKET_ERROR);
	g_return_val_if_fail (len >= boundary_len, SOUP_SOCKET_ERROR);

	priv = SOUP_SOCKET_GET_PRIVATE (sock);

	g_mutex_lock (&priv->iolock);

	*got_boundary = FALSE;

	if (!priv->istream)
		status = SOUP_SOCKET_EOF;
	else {
		my_nread = soup_filter_input_stream_read_until (
			SOUP_FILTER_INPUT_STREAM (priv->istream),
			buffer, len, boundary, boundary_len,
			!priv->non_blocking,
			TRUE, got_boundary, cancellable, &my_err);
		status = translate_read_status (sock, cancellable,
						my_nread, nread, my_err, error);
	}

	g_mutex_unlock (&priv->iolock);
	return status;
}

static gboolean
socket_write_watch (GObject *pollable, gpointer user_data)
{
	SoupSocket *sock = user_data;
	SoupSocketPrivate *priv = SOUP_SOCKET_GET_PRIVATE (sock);

	priv->write_src = NULL;
	g_signal_emit (sock, signals[WRITABLE], 0);
	return FALSE;
}

/**
 * soup_socket_write:
 * @sock: the socket
 * @buffer: data to write
 * @len: size of @buffer, in bytes
 * @nwrote: (out): on return, number of bytes written
 * @cancellable: a #GCancellable, or %NULL
 * @error: error pointer
 *
 * Attempts to write @len bytes from @buffer to @sock. If some data is
 * successfully written, the return status will be %SOUP_SOCKET_OK,
 * and *@nwrote will contain the number of bytes actually written
 * (which may be less than @len).
 *
 * If @sock is non-blocking, and no data could be written right away,
 * the return value will be %SOUP_SOCKET_WOULD_BLOCK. In this case,
 * the caller can connect to the #SoupSocket::writable signal to know
 * when more data can be written. (NB: #SoupSocket::writable is only
 * emitted after soup_socket_write() returns %SOUP_SOCKET_WOULD_BLOCK,
 * and it is only emitted once. See the documentation for
 * #SoupSocket:non-blocking.)
 *
 * Return value: a #SoupSocketIOStatus, as described above (or
 * %SOUP_SOCKET_EOF or %SOUP_SOCKET_ERROR. @error will be set if the
 * return value is %SOUP_SOCKET_ERROR.)
 **/
SoupSocketIOStatus
soup_socket_write (SoupSocket *sock, gconstpointer buffer,
		   gsize len, gsize *nwrote,
		   GCancellable *cancellable, GError **error)
{
	SoupSocketPrivate *priv;
	GError *my_err = NULL;
	gssize my_nwrote;

	g_return_val_if_fail (SOUP_IS_SOCKET (sock), SOUP_SOCKET_ERROR);
	g_return_val_if_fail (nwrote != NULL, SOUP_SOCKET_ERROR);

	priv = SOUP_SOCKET_GET_PRIVATE (sock);

	g_mutex_lock (&priv->iolock);

	if (!priv->conn) {
		g_mutex_unlock (&priv->iolock);
		return SOUP_SOCKET_EOF;
	}
	if (priv->write_src) {
		g_mutex_unlock (&priv->iolock);
		return SOUP_SOCKET_WOULD_BLOCK;
	}

	if (!priv->non_blocking) {
		my_nwrote = g_output_stream_write (priv->ostream,
						   buffer, len,
						   cancellable, &my_err);
	} else {
		my_nwrote = g_pollable_output_stream_write_nonblocking (
			G_POLLABLE_OUTPUT_STREAM (priv->ostream),
			buffer, len, cancellable, &my_err);
	}

	if (my_nwrote > 0) {
		g_mutex_unlock (&priv->iolock);
		g_clear_error (&my_err);
		*nwrote = my_nwrote;
		return SOUP_SOCKET_OK;
	}

	if (g_error_matches (my_err, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK)) {
		g_mutex_unlock (&priv->iolock);
		g_clear_error (&my_err);

		priv->write_src =
			soup_socket_create_watch (priv,
						  G_IO_OUT,
						  socket_write_watch, sock, cancellable);
		return SOUP_SOCKET_WOULD_BLOCK;
	}

	g_mutex_unlock (&priv->iolock);
	g_propagate_error (error, my_err);
	return SOUP_SOCKET_ERROR;
}
