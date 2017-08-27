/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-server.c: Asynchronous HTTP server
 *
 * Copyright (C) 2001-2003, Ximian, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "soup-server.h"
#include "soup.h"
#include "soup-message-private.h"
#include "soup-marshal.h"
#include "soup-path-map.h" 

/**
 * SECTION:soup-server
 * @short_description: HTTP server
 * @see_also: #SoupAuthDomain
 *
 * #SoupServer implements a simple HTTP server.
 * 
 * To begin, create a server using soup_server_new(). Add at least one
 * handler by calling soup_server_add_handler(); the handler will be
 * called to process any requests underneath the path passed to
 * soup_server_add_handler(). (If you want all requests to go to the
 * same handler, just pass "/" (or %NULL) for the path.) Any request
 * that does not match any handler will automatically be returned to
 * the client with a 404 (Not Found) status.
 *
 * If you want to handle the special "*" URI (eg, "OPTIONS *"), you
 * must explicitly register a handler for "*"; the default handler
 * will not be used for that case.
 * 
 * To add authentication to some or all paths, create an appropriate
 * #SoupAuthDomain (qv), and add it to the server via
 * soup_server_add_auth_domain(). (As with handlers, you must
 * explicitly add "*" to an auth domain if you want it to be covered.)
 * 
 * Additional processing options are available via #SoupServer's
 * signals; Connect to #SoupServer::request-started to be notified
 * every time a new request is being processed. (This gives you a
 * chance to connect to the #SoupMessage "got-" signals in case you
 * want to do processing before the body has been fully read.)
 * 
 * Once the server is set up, start it processing connections by
 * calling soup_server_run_async() or soup_server_run(). #SoupServer
 * runs via the glib main loop; if you need to have a server that runs
 * in another thread (or merely isn't bound to the default main loop),
 * create a #GMainContext for it to use, and set that via the
 * #SOUP_SERVER_ASYNC_CONTEXT property.
 **/

G_DEFINE_TYPE (SoupServer, soup_server, G_TYPE_OBJECT)

enum {
	REQUEST_STARTED,
	REQUEST_READ,
	REQUEST_FINISHED,
	REQUEST_ABORTED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

struct SoupClientContext {
	SoupServer     *server;
	SoupSocket     *sock;
	SoupMessage    *msg;
	SoupAuthDomain *auth_domain;
	char           *auth_user;

	int             ref_count;
};

typedef struct {
	char                   *path;

	SoupServerCallback      callback;
	GDestroyNotify          destroy;
	gpointer                user_data;
} SoupServerHandler;

typedef struct {
	SoupAddress       *iface;
	guint              port;

	char              *ssl_cert_file, *ssl_key_file;
	GTlsCertificate   *ssl_cert;

	char              *server_header;

	GMainLoop         *loop;

	SoupSocket        *listen_sock;
	GSList            *clients;

	gboolean           raw_paths;
	SoupPathMap       *handlers;
	SoupServerHandler *default_handler;
	
	GSList            *auth_domains;

	GMainContext      *async_context;
} SoupServerPrivate;
#define SOUP_SERVER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), SOUP_TYPE_SERVER, SoupServerPrivate))

#define SOUP_SERVER_SERVER_HEADER_BASE "libsoup/" PACKAGE_VERSION

enum {
	PROP_0,

	PROP_PORT,
	PROP_INTERFACE,
	PROP_SSL_CERT_FILE,
	PROP_SSL_KEY_FILE,
	PROP_TLS_CERTIFICATE,
	PROP_ASYNC_CONTEXT,
	PROP_RAW_PATHS,
	PROP_SERVER_HEADER,

	LAST_PROP
};

static SoupClientContext *soup_client_context_ref (SoupClientContext *client);
static void soup_client_context_unref (SoupClientContext *client);

static void
free_handler (SoupServerHandler *hand)
{
	g_free (hand->path);
	g_slice_free (SoupServerHandler, hand);
}

static void
soup_server_init (SoupServer *server)
{
	SoupServerPrivate *priv = SOUP_SERVER_GET_PRIVATE (server);

	priv->handlers = soup_path_map_new ((GDestroyNotify)free_handler);
}

static void
soup_server_finalize (GObject *object)
{
	SoupServer *server = SOUP_SERVER (object);
	SoupServerPrivate *priv = SOUP_SERVER_GET_PRIVATE (server);

	g_clear_object (&priv->iface);

	g_free (priv->ssl_cert_file);
	g_free (priv->ssl_key_file);
	g_clear_object (&priv->ssl_cert);

	g_free (priv->server_header);

	g_clear_object (&priv->listen_sock);

	while (priv->clients) {
		SoupClientContext *client = priv->clients->data;
		SoupSocket *sock = g_object_ref (client->sock);

		priv->clients = g_slist_remove (priv->clients, client);

		/* keep a ref on the client context so it doesn't get destroyed
		 * when we finish the message; the SoupSocket::disconnect
		 * handler will refer to client->server later when the socket is
		 * disconnected.
		 */
		soup_client_context_ref (client);

		if (client->msg) {
			soup_message_set_status (client->msg, SOUP_STATUS_IO_ERROR);
			soup_message_io_finished (client->msg);
		}

		soup_socket_disconnect (sock);
		g_object_unref (sock);

		soup_client_context_unref (client);
	}

	g_clear_pointer (&priv->default_handler, free_handler);
	soup_path_map_free (priv->handlers);

	g_slist_free_full (priv->auth_domains, g_object_unref);

	g_clear_pointer (&priv->loop, g_main_loop_unref);
	g_clear_pointer (&priv->async_context, g_main_context_unref);

	G_OBJECT_CLASS (soup_server_parent_class)->finalize (object);
}

static GObject *
soup_server_constructor (GType                  type,
			 guint                  n_construct_properties,
			 GObjectConstructParam *construct_properties)
{
	GObject *server;
	SoupServerPrivate *priv;

	server = G_OBJECT_CLASS (soup_server_parent_class)->constructor (
		type, n_construct_properties, construct_properties);
	if (!server)
		return NULL;
	priv = SOUP_SERVER_GET_PRIVATE (server);

	if (!priv->iface) {
		priv->iface =
			soup_address_new_any (SOUP_ADDRESS_FAMILY_IPV4,
					      priv->port);
	}

	if (priv->ssl_cert_file && priv->ssl_key_file) {
		GError *error = NULL;

		if (priv->ssl_cert)
			g_object_unref (priv->ssl_cert);
		priv->ssl_cert = g_tls_certificate_new_from_files (priv->ssl_cert_file, priv->ssl_key_file, &error);
		if (!priv->ssl_cert) {
			g_warning ("Could not read SSL certificate from '%s': %s",
				   priv->ssl_cert_file, error->message);
			g_error_free (error);
			g_object_unref (server);
			return NULL;
		}
	}

	priv->listen_sock =
		soup_socket_new (SOUP_SOCKET_LOCAL_ADDRESS, priv->iface,
				 SOUP_SOCKET_SSL_CREDENTIALS, priv->ssl_cert,
				 SOUP_SOCKET_ASYNC_CONTEXT, priv->async_context,
				 NULL);
	if (!soup_socket_listen (priv->listen_sock)) {
		g_object_unref (server);
		return NULL;
	}

	/* Re-resolve the interface address, in particular in case
	 * the passed-in address had SOUP_ADDRESS_ANY_PORT.
	 */
	g_object_unref (priv->iface);
	priv->iface = soup_socket_get_local_address (priv->listen_sock);
	g_object_ref (priv->iface);
	priv->port = soup_address_get_port (priv->iface);

	return server;
}

static void
soup_server_set_property (GObject *object, guint prop_id,
			  const GValue *value, GParamSpec *pspec)
{
	SoupServerPrivate *priv = SOUP_SERVER_GET_PRIVATE (object);
	const char *header;

	switch (prop_id) {
	case PROP_PORT:
		priv->port = g_value_get_uint (value);
		break;
	case PROP_INTERFACE:
		if (priv->iface)
			g_object_unref (priv->iface);
		priv->iface = g_value_get_object (value);
		if (priv->iface)
			g_object_ref (priv->iface);
		break;
	case PROP_SSL_CERT_FILE:
		priv->ssl_cert_file =
			g_strdup (g_value_get_string (value));
		break;
	case PROP_SSL_KEY_FILE:
		priv->ssl_key_file =
			g_strdup (g_value_get_string (value));
		break;
	case PROP_TLS_CERTIFICATE:
		if (priv->ssl_cert)
			g_object_unref (priv->ssl_cert);
		priv->ssl_cert = g_value_dup_object (value);
		break;
	case PROP_ASYNC_CONTEXT:
		priv->async_context = g_value_get_pointer (value);
		if (priv->async_context)
			g_main_context_ref (priv->async_context);
		break;
	case PROP_RAW_PATHS:
		priv->raw_paths = g_value_get_boolean (value);
		break;
	case PROP_SERVER_HEADER:
		g_free (priv->server_header);
		header = g_value_get_string (value);
		if (!header)
			priv->server_header = NULL;
		else if (!*header) {
			priv->server_header =
				g_strdup (SOUP_SERVER_SERVER_HEADER_BASE);
		} else if (g_str_has_suffix (header, " ")) {
			priv->server_header =
				g_strdup_printf ("%s%s", header,
						 SOUP_SERVER_SERVER_HEADER_BASE);
		} else
			priv->server_header = g_strdup (header);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
soup_server_get_property (GObject *object, guint prop_id,
			  GValue *value, GParamSpec *pspec)
{
	SoupServerPrivate *priv = SOUP_SERVER_GET_PRIVATE (object);

	switch (prop_id) {
	case PROP_PORT:
		g_value_set_uint (value, priv->port);
		break;
	case PROP_INTERFACE:
		g_value_set_object (value, priv->iface);
		break;
	case PROP_SSL_CERT_FILE:
		g_value_set_string (value, priv->ssl_cert_file);
		break;
	case PROP_SSL_KEY_FILE:
		g_value_set_string (value, priv->ssl_key_file);
		break;
	case PROP_TLS_CERTIFICATE:
		g_value_set_object (value, priv->ssl_cert);
		break;
	case PROP_ASYNC_CONTEXT:
		g_value_set_pointer (value, priv->async_context ? g_main_context_ref (priv->async_context) : NULL);
		break;
	case PROP_RAW_PATHS:
		g_value_set_boolean (value, priv->raw_paths);
		break;
	case PROP_SERVER_HEADER:
		g_value_set_string (value, priv->server_header);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
soup_server_class_init (SoupServerClass *server_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (server_class);

	g_type_class_add_private (server_class, sizeof (SoupServerPrivate));

	/* virtual method override */
	object_class->constructor = soup_server_constructor;
	object_class->finalize = soup_server_finalize;
	object_class->set_property = soup_server_set_property;
	object_class->get_property = soup_server_get_property;

	/* signals */

	/**
	 * SoupServer::request-started:
	 * @server: the server
	 * @message: the new message
	 * @client: the client context
	 *
	 * Emitted when the server has started reading a new request.
	 * @message will be completely blank; not even the
	 * Request-Line will have been read yet. About the only thing
	 * you can usefully do with it is connect to its signals.
	 *
	 * If the request is read successfully, this will eventually
	 * be followed by a #SoupServer::request_read signal. If a
	 * response is then sent, the request processing will end with
	 * a #SoupServer::request_finished signal. If a network error
	 * occurs, the processing will instead end with
	 * #SoupServer::request_aborted.
	 **/
	signals[REQUEST_STARTED] =
		g_signal_new ("request-started",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (SoupServerClass, request_started),
			      NULL, NULL,
			      _soup_marshal_NONE__OBJECT_POINTER,
			      G_TYPE_NONE, 2, 
			      SOUP_TYPE_MESSAGE,
			      SOUP_TYPE_CLIENT_CONTEXT);

	/**
	 * SoupServer::request-read:
	 * @server: the server
	 * @message: the message
	 * @client: the client context
	 *
	 * Emitted when the server has successfully read a request.
	 * @message will have all of its request-side information
	 * filled in, and if the message was authenticated, @client
	 * will have information about that. This signal is emitted
	 * before any handlers are called for the message, and if it
	 * sets the message's #status_code, then normal handler
	 * processing will be skipped.
	 **/
	signals[REQUEST_READ] =
		g_signal_new ("request-read",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (SoupServerClass, request_read),
			      NULL, NULL,
			      _soup_marshal_NONE__OBJECT_POINTER,
			      G_TYPE_NONE, 2,
			      SOUP_TYPE_MESSAGE,
			      SOUP_TYPE_CLIENT_CONTEXT);

	/**
	 * SoupServer::request-finished:
	 * @server: the server
	 * @message: the message
	 * @client: the client context
	 *
	 * Emitted when the server has finished writing a response to
	 * a request.
	 **/
	signals[REQUEST_FINISHED] =
		g_signal_new ("request-finished",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (SoupServerClass, request_finished),
			      NULL, NULL,
			      _soup_marshal_NONE__OBJECT_POINTER,
			      G_TYPE_NONE, 2,
			      SOUP_TYPE_MESSAGE,
			      SOUP_TYPE_CLIENT_CONTEXT);

	/**
	 * SoupServer::request-aborted:
	 * @server: the server
	 * @message: the message
	 * @client: the client context
	 *
	 * Emitted when processing has failed for a message; this
	 * could mean either that it could not be read (if
	 * #SoupServer::request_read has not been emitted for it yet),
	 * or that the response could not be written back (if
	 * #SoupServer::request_read has been emitted but
	 * #SoupServer::request_finished has not been).
	 *
	 * @message is in an undefined state when this signal is
	 * emitted; the signal exists primarily to allow the server to
	 * free any state that it may have allocated in
	 * #SoupServer::request_started.
	 **/
	signals[REQUEST_ABORTED] =
		g_signal_new ("request-aborted",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (SoupServerClass, request_aborted),
			      NULL, NULL,
			      _soup_marshal_NONE__OBJECT_POINTER,
			      G_TYPE_NONE, 2,
			      SOUP_TYPE_MESSAGE,
			      SOUP_TYPE_CLIENT_CONTEXT);

	/* properties */
	/**
	 * SOUP_SERVER_PORT:
	 *
	 * Alias for the #SoupServer:port property. (The port the
	 * server listens on.)
	 **/
	g_object_class_install_property (
		object_class, PROP_PORT,
		g_param_spec_uint (SOUP_SERVER_PORT,
				   "Port",
				   "Port to listen on",
				   0, 65536, 0,
				   G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * SOUP_SERVER_INTERFACE:
	 *
	 * Alias for the #SoupServer:interface property. (The address
	 * of the network interface the server listens on.)
	 **/
	g_object_class_install_property (
		object_class, PROP_INTERFACE,
		g_param_spec_object (SOUP_SERVER_INTERFACE,
				     "Interface",
				     "Address of interface to listen on",
				     SOUP_TYPE_ADDRESS,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * SOUP_SERVER_SSL_CERT_FILE:
	 *
	 * Alias for the #SoupServer:ssl-cert-file property, qv.
	 */
	/**
	 * SoupServer:ssl-cert-file:
	 *
	 * Path to a file containing a PEM-encoded certificate. If
	 * this and #SoupServer:ssl-key-file are both set, then the
	 * server will speak https rather than plain http.
	 *
	 * Alternatively, you can use #SoupServer:tls-certificate
	 * to provide an arbitrary #GTlsCertificate.
	 */
	g_object_class_install_property (
		object_class, PROP_SSL_CERT_FILE,
		g_param_spec_string (SOUP_SERVER_SSL_CERT_FILE,
				     "SSL certificate file",
				     "File containing server SSL certificate",
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * SOUP_SERVER_SSL_KEY_FILE:
	 *
	 * Alias for the #SoupServer:ssl-key-file property, qv.
	 */
	/**
	 * SoupServer:ssl-key-file:
	 *
	 * Path to a file containing a PEM-encoded private key. If
	 * this and #SoupServer:ssl-key-file are both set, then the
	 * server will speak https rather than plain http. Note that
	 * you are allowed to set them to the same value, if you have
	 * a single file containing both the certificate and the key.
	 *
	 * Alternatively, you can use #SoupServer:tls-certificate
	 * to provide an arbitrary #GTlsCertificate.
	 */
	g_object_class_install_property (
		object_class, PROP_SSL_KEY_FILE,
		g_param_spec_string (SOUP_SERVER_SSL_KEY_FILE,
				     "SSL key file",
				     "File containing server SSL key",
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * SOUP_SERVER_TLS_CERTIFICATE:
	 *
	 * Alias for the #SoupServer:tls-certificate property, qv.
	 */
	/**
	 * SoupServer:tls-certificate:
	 *
	 * A #GTlsCertificate that has a #GTlsCertificate:private-key
	 * set. If this is set, then the server will speak https
	 * rather than plain http.
	 *
	 * Alternatively, you can use #SoupServer:ssl-cert-file and
	 * #SoupServer:ssl-key-file properties, to have #SoupServer
	 * read in a a certificate from a file.
	 */
	g_object_class_install_property (
		object_class, PROP_TLS_CERTIFICATE,
		g_param_spec_object (SOUP_SERVER_TLS_CERTIFICATE,
				     "TLS certificate",
				     "GTlsCertificate to use for https",
				     G_TYPE_TLS_CERTIFICATE,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * SOUP_SERVER_ASYNC_CONTEXT:
	 *
	 * Alias for the #SoupServer:async-context property. (The
	 * server's #GMainContext.)
	 **/
	g_object_class_install_property (
		object_class, PROP_ASYNC_CONTEXT,
		g_param_spec_pointer (SOUP_SERVER_ASYNC_CONTEXT,
				      "Async GMainContext",
				      "The GMainContext to dispatch async I/O in",
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * SOUP_SERVER_RAW_PATHS:
	 *
	 * Alias for the #SoupServer:raw-paths property. (If %TRUE,
	 * percent-encoding in the Request-URI path will not be
	 * automatically decoded.)
	 **/
	g_object_class_install_property (
		object_class, PROP_RAW_PATHS,
		g_param_spec_boolean (SOUP_SERVER_RAW_PATHS,
				      "Raw paths",
				      "If %TRUE, percent-encoding in the Request-URI path will not be automatically decoded.",
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	/**
	 * SoupServer:server-header:
	 *
	 * If non-%NULL, the value to use for the "Server" header on
	 * #SoupMessage<!-- -->s processed by this server.
	 *
	 * The Server header is the server equivalent of the
	 * User-Agent header, and provides information about the
	 * server and its components. It contains a list of one or
	 * more product tokens, separated by whitespace, with the most
	 * significant product token coming first. The tokens must be
	 * brief, ASCII, and mostly alphanumeric (although "-", "_",
	 * and "." are also allowed), and may optionally include a "/"
	 * followed by a version string. You may also put comments,
	 * enclosed in parentheses, between or after the tokens.
	 *
	 * Some HTTP server implementations intentionally do not use
	 * version numbers in their Server header, so that
	 * installations running older versions of the server don't
	 * end up advertising their vulnerability to specific security
	 * holes.
	 *
	 * As with #SoupSession:user_agent, if you set a
	 * #SoupServer:server_header property that has trailing whitespace,
	 * #SoupServer will append its own product token (eg,
	 * "<literal>libsoup/2.3.2</literal>") to the end of the
	 * header for you.
	 **/
	/**
	 * SOUP_SERVER_SERVER_HEADER:
	 *
	 * Alias for the #SoupServer:server-header property, qv.
	 **/
	g_object_class_install_property (
		object_class, PROP_SERVER_HEADER,
		g_param_spec_string (SOUP_SERVER_SERVER_HEADER,
				     "Server header",
				     "Server header",
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}

/**
 * soup_server_new:
 * @optname1: name of first property to set
 * @...: value of @optname1, followed by additional property/value pairs
 *
 * Creates a new #SoupServer.
 *
 * Return value: a new #SoupServer
 **/
SoupServer *
soup_server_new (const char *optname1, ...)
{
	SoupServer *server;
	va_list ap;

	va_start (ap, optname1);
	server = (SoupServer *)g_object_new_valist (SOUP_TYPE_SERVER,
						    optname1, ap);
	va_end (ap);

	return server;
}

/**
 * soup_server_get_port:
 * @server: a #SoupServer
 *
 * Gets the TCP port that @server is listening on. This is most useful
 * when you did not request a specific port (or explicitly requested
 * %SOUP_ADDRESS_ANY_PORT).
 *
 * Return value: the port @server is listening on.
 **/
guint
soup_server_get_port (SoupServer *server)
{
	g_return_val_if_fail (SOUP_IS_SERVER (server), 0);

	return SOUP_SERVER_GET_PRIVATE (server)->port;
}

/**
 * soup_server_is_https:
 * @server: a #SoupServer
 *
 * Checks whether @server is running plain http or https.
 *
 * In order for a server to run https, you must set the
 * %SOUP_SERVER_SSL_CERT_FILE and %SOUP_SERVER_SSL_KEY_FILE properties
 * to provide it with an SSL certificate to use.
 *
 * Return value: %TRUE if @server is serving https.
 **/
gboolean
soup_server_is_https (SoupServer *server)
{
	SoupServerPrivate *priv;

	g_return_val_if_fail (SOUP_IS_SERVER (server), 0);
	priv = SOUP_SERVER_GET_PRIVATE (server);

	return (priv->ssl_cert_file && priv->ssl_key_file);
}

/**
 * soup_server_get_listener:
 * @server: a #SoupServer
 *
 * Gets @server's listening socket. You should treat this as
 * read-only; writing to it or modifiying it may cause @server to
 * malfunction.
 *
 * Return value: (transfer none): the listening socket.
 **/
SoupSocket *
soup_server_get_listener (SoupServer *server)
{
	SoupServerPrivate *priv;

	g_return_val_if_fail (SOUP_IS_SERVER (server), NULL);
	priv = SOUP_SERVER_GET_PRIVATE (server);

	return priv->listen_sock;
}

static void start_request (SoupServer *, SoupClientContext *);

static SoupClientContext *
soup_client_context_new (SoupServer *server, SoupSocket *sock)
{
	SoupClientContext *client = g_slice_new0 (SoupClientContext);

	client->server = server;
	client->sock = sock;
	client->ref_count = 1;

	return client;
}

static void
soup_client_context_cleanup (SoupClientContext *client)
{
	if (client->auth_domain) {
		g_object_unref (client->auth_domain);
		client->auth_domain = NULL;
	}
	if (client->auth_user) {
		g_free (client->auth_user);
		client->auth_user = NULL;
	}
	client->msg = NULL;
}

static SoupClientContext *
soup_client_context_ref (SoupClientContext *client)
{
	client->ref_count++;
	return client;
}

static void
soup_client_context_unref (SoupClientContext *client)
{
	if (--client->ref_count == 0) {
		soup_client_context_cleanup (client);
		g_slice_free (SoupClientContext, client);
	}
}

static void
request_finished (SoupMessage *msg, gpointer user_data)
{
	SoupClientContext *client = user_data;
	SoupServer *server = client->server;
	SoupSocket *sock = client->sock;

	soup_message_finished (msg);
	g_signal_emit (server,
		       msg->status_code == SOUP_STATUS_IO_ERROR ?
		       signals[REQUEST_ABORTED] : signals[REQUEST_FINISHED],
		       0, msg, client);

	soup_client_context_cleanup (client);
	if (soup_socket_is_connected (sock) && soup_message_is_keepalive (msg)) {
		/* Start a new request */
		start_request (server, client);
	} else {
		soup_socket_disconnect (sock);
		soup_client_context_unref (client);
	}
	g_object_unref (msg);
	g_object_unref (sock);
}

static SoupServerHandler *
soup_server_get_handler (SoupServer *server, const char *path)
{
	SoupServerPrivate *priv;
	SoupServerHandler *hand;

	g_return_val_if_fail (SOUP_IS_SERVER (server), NULL);
	priv = SOUP_SERVER_GET_PRIVATE (server);

	if (path) {
		hand = soup_path_map_lookup (priv->handlers, path);
		if (hand)
			return hand;
		if (!strcmp (path, "*"))
			return NULL;
	}
	return priv->default_handler;
}

static void
got_headers (SoupMessage *req, SoupClientContext *client)
{
	SoupServer *server = client->server;
	SoupServerPrivate *priv = SOUP_SERVER_GET_PRIVATE (server);
	SoupURI *uri;
	SoupDate *date;
	char *date_string;
	SoupAuthDomain *domain;
	GSList *iter;
	gboolean rejected = FALSE;
	char *auth_user;

	if (!priv->raw_paths) {
		char *decoded_path;

		uri = soup_message_get_uri (req);
		decoded_path = soup_uri_decode (uri->path);

		if (strstr (decoded_path, "/../") ||
		    g_str_has_suffix (decoded_path, "/..")) {
			/* Introducing new ".." segments is not allowed */
			g_free (decoded_path);
			soup_message_set_status (req, SOUP_STATUS_BAD_REQUEST);
			return;
		}

		soup_uri_set_path (uri, decoded_path);
		g_free (decoded_path);
	}

	/* Add required response headers */
	date = soup_date_new_from_now (0);
	date_string = soup_date_to_string (date, SOUP_DATE_HTTP);
	soup_message_headers_replace (req->response_headers, "Date",
				      date_string);
	g_free (date_string);
	soup_date_free (date);
	
	/* Now handle authentication. (We do this here so that if
	 * the request uses "Expect: 100-continue", we can reject it
	 * immediately rather than waiting for the request body to
	 * be sent.
	 */
	for (iter = priv->auth_domains; iter; iter = iter->next) {
		domain = iter->data;

		if (soup_auth_domain_covers (domain, req)) {
			auth_user = soup_auth_domain_accepts (domain, req);
			if (auth_user) {
				client->auth_domain = g_object_ref (domain);
				client->auth_user = auth_user;
				return;
			}

			rejected = TRUE;
		}
	}

	/* If no auth domain rejected it, then it's ok. */
	if (!rejected)
		return;

	for (iter = priv->auth_domains; iter; iter = iter->next) {
		domain = iter->data;

		if (soup_auth_domain_covers (domain, req))
			soup_auth_domain_challenge (domain, req);
	}
}

static void
call_handler (SoupMessage *req, SoupClientContext *client)
{
	SoupServer *server = client->server;
	SoupServerHandler *hand;
	SoupURI *uri;

	g_signal_emit (server, signals[REQUEST_READ], 0, req, client);

	if (req->status_code != 0)
		return;

	uri = soup_message_get_uri (req);
	hand = soup_server_get_handler (server, uri->path);
	if (!hand) {
		soup_message_set_status (req, SOUP_STATUS_NOT_FOUND);
		return;
	}

	if (hand->callback) {
		GHashTable *form_data_set;

		if (uri->query)
			form_data_set = soup_form_decode (uri->query);
		else
			form_data_set = NULL;

		/* Call method handler */
		(*hand->callback) (server, req,
				   uri->path, form_data_set,
				   client, hand->user_data);

		if (form_data_set)
			g_hash_table_destroy (form_data_set);
	}
}

static void
start_request (SoupServer *server, SoupClientContext *client)
{
	SoupServerPrivate *priv = SOUP_SERVER_GET_PRIVATE (server);
	SoupMessage *msg;

	soup_client_context_cleanup (client);

	/* Listen for another request on this connection */
	msg = g_object_new (SOUP_TYPE_MESSAGE,
			    SOUP_MESSAGE_SERVER_SIDE, TRUE,
			    NULL);
	client->msg = msg;

	if (priv->server_header) {
		soup_message_headers_append (msg->response_headers, "Server",
					     priv->server_header);
	}

	g_signal_connect (msg, "got_headers", G_CALLBACK (got_headers), client);
	g_signal_connect (msg, "got_body", G_CALLBACK (call_handler), client);

	g_signal_emit (server, signals[REQUEST_STARTED], 0,
		       msg, client);

	g_object_ref (client->sock);

	if (priv->async_context)
		g_main_context_push_thread_default (priv->async_context);
	soup_message_read_request (msg, client->sock,
				   request_finished, client);
	if (priv->async_context)
		g_main_context_pop_thread_default (priv->async_context);
}

static void
socket_disconnected (SoupSocket *sock, SoupClientContext *client)
{
	SoupServerPrivate *priv = SOUP_SERVER_GET_PRIVATE (client->server);

	priv->clients = g_slist_remove (priv->clients, client);
	g_signal_handlers_disconnect_by_func (sock, socket_disconnected, client);
	g_object_unref (sock);
}

static void
new_connection (SoupSocket *listner, SoupSocket *sock, gpointer user_data)
{
	SoupServer *server = user_data;
	SoupServerPrivate *priv = SOUP_SERVER_GET_PRIVATE (server);
	SoupClientContext *client;

	client = soup_client_context_new (server, g_object_ref (sock));
	priv->clients = g_slist_prepend (priv->clients, client);
	g_signal_connect (sock, "disconnected",
			  G_CALLBACK (socket_disconnected), client);
	start_request (server, client);
}

/**
 * soup_server_run_async:
 * @server: a #SoupServer
 *
 * Starts @server, causing it to listen for and process incoming
 * connections.
 *
 * The server actually runs in @server's #GMainContext. It will not
 * actually perform any processing unless the appropriate main loop is
 * running. In the simple case where you did not set the server's
 * %SOUP_SERVER_ASYNC_CONTEXT property, this means the server will run
 * whenever the glib main loop is running.
 **/
void
soup_server_run_async (SoupServer *server)
{
	SoupServerPrivate *priv;

	g_return_if_fail (SOUP_IS_SERVER (server));
	priv = SOUP_SERVER_GET_PRIVATE (server);

	if (!priv->listen_sock) {
		if (priv->loop) {
			g_main_loop_unref (priv->loop);
			priv->loop = NULL;
		}
		return;
	}

	g_signal_connect (priv->listen_sock, "new_connection",
			  G_CALLBACK (new_connection), server);

	return;

}

/**
 * soup_server_run:
 * @server: a #SoupServer
 *
 * Starts @server, causing it to listen for and process incoming
 * connections. Unlike soup_server_run_async(), this creates a
 * #GMainLoop and runs it, and it will not return until someone calls
 * soup_server_quit() to stop the server.
 **/
void
soup_server_run (SoupServer *server)
{
	SoupServerPrivate *priv;

	g_return_if_fail (SOUP_IS_SERVER (server));
	priv = SOUP_SERVER_GET_PRIVATE (server);

	if (!priv->loop) {
		priv->loop = g_main_loop_new (priv->async_context, TRUE);
		soup_server_run_async (server);
	}

	if (priv->loop)
		g_main_loop_run (priv->loop);
}

/**
 * soup_server_quit:
 * @server: a #SoupServer
 *
 * Stops processing for @server. Call this to clean up after
 * soup_server_run_async(), or to terminate a call to soup_server_run().
 *
 * @server is still in a working state after this call; you can start
 * and stop a server as many times as you want.
 **/
void
soup_server_quit (SoupServer *server)
{
	SoupServerPrivate *priv;

	g_return_if_fail (SOUP_IS_SERVER (server));
	priv = SOUP_SERVER_GET_PRIVATE (server);

	g_signal_handlers_disconnect_by_func (priv->listen_sock,
					      G_CALLBACK (new_connection),
					      server);
	if (priv->loop)
		g_main_loop_quit (priv->loop);
}

/**
 * soup_server_disconnect:
 * @server: a #SoupServer
 *
 * Stops processing for @server and closes its socket. This implies
 * the effects of soup_server_quit(), but additionally closes the
 * listening socket.  Note that messages currently in progress will
 * continue to be handled, if the main loop associated with the
 * server is resumed or kept running.
 *
 * After calling this function, @server is no longer functional, so it
 * has nearly the same effect as destroying @server entirely. The
 * function is thus useful mainly for language bindings without
 * explicit control over object lifetime.
 **/
void
soup_server_disconnect (SoupServer *server)
{
	SoupServerPrivate *priv;

	g_return_if_fail (SOUP_IS_SERVER (server));
	priv = SOUP_SERVER_GET_PRIVATE (server);

	soup_server_quit (server);

	if (priv->listen_sock) {
		soup_socket_disconnect (priv->listen_sock);
		g_object_unref (priv->listen_sock);
		priv->listen_sock = NULL;
	}
}

/**
 * soup_server_get_async_context:
 * @server: a #SoupServer
 *
 * Gets @server's async_context. This does not add a ref to the
 * context, so you will need to ref it yourself if you want it to
 * outlive its server.
 *
 * Return value: (transfer none): @server's #GMainContext, which may be %NULL
 **/
GMainContext *
soup_server_get_async_context (SoupServer *server)
{
	SoupServerPrivate *priv;

	g_return_val_if_fail (SOUP_IS_SERVER (server), NULL);
	priv = SOUP_SERVER_GET_PRIVATE (server);

	return priv->async_context;
}

/**
 * SoupClientContext:
 *
 * A #SoupClientContext provides additional information about the
 * client making a particular request. In particular, you can use
 * soup_client_context_get_auth_domain() and
 * soup_client_context_get_auth_user() to determine if HTTP
 * authentication was used successfully.
 *
 * soup_client_context_get_address() and/or
 * soup_client_context_get_host() can be used to get information for
 * logging or debugging purposes. soup_client_context_get_socket() may
 * also be of use in some situations (eg, tracking when multiple
 * requests are made on the same connection).
 **/
G_DEFINE_BOXED_TYPE (SoupClientContext, soup_client_context, soup_client_context_ref, soup_client_context_unref)

/**
 * soup_client_context_get_socket:
 * @client: a #SoupClientContext
 *
 * Retrieves the #SoupSocket that @client is associated with.
 *
 * If you are using this method to observe when multiple requests are
 * made on the same persistent HTTP connection (eg, as the ntlm-test
 * test program does), you will need to pay attention to socket
 * destruction as well (either by using weak references, or by
 * connecting to the #SoupSocket::disconnected signal), so that you do
 * not get fooled when the allocator reuses the memory address of a
 * previously-destroyed socket to represent a new socket.
 *
 * Return value: (transfer none): the #SoupSocket that @client is
 * associated with.
 **/
SoupSocket *
soup_client_context_get_socket (SoupClientContext *client)
{
	g_return_val_if_fail (client != NULL, NULL);

	return client->sock;
}

/**
 * soup_client_context_get_address:
 * @client: a #SoupClientContext
 *
 * Retrieves the #SoupAddress associated with the remote end
 * of a connection.
 *
 * Return value: (transfer none): the #SoupAddress associated with the
 * remote end of a connection.
 **/
SoupAddress *
soup_client_context_get_address (SoupClientContext *client)
{
	g_return_val_if_fail (client != NULL, NULL);

	return soup_socket_get_remote_address (client->sock);
}

/**
 * soup_client_context_get_host:
 * @client: a #SoupClientContext
 *
 * Retrieves the IP address associated with the remote end of a
 * connection. (If you want the actual hostname, you'll have to call
 * soup_client_context_get_address() and then call the appropriate
 * #SoupAddress method to resolve it.)
 *
 * Return value: the IP address associated with the remote end of a
 * connection.
 **/
const char *
soup_client_context_get_host (SoupClientContext *client)
{
	SoupAddress *address;

	address = soup_client_context_get_address (client);
	return soup_address_get_physical (address);
}

/**
 * soup_client_context_get_auth_domain:
 * @client: a #SoupClientContext
 *
 * Checks whether the request associated with @client has been
 * authenticated, and if so returns the #SoupAuthDomain that
 * authenticated it.
 *
 * Return value: (transfer none) (allow-none): a #SoupAuthDomain, or
 * %NULL if the request was not authenticated.
 **/
SoupAuthDomain *
soup_client_context_get_auth_domain (SoupClientContext *client)
{
	g_return_val_if_fail (client != NULL, NULL);

	return client->auth_domain;
}

/**
 * soup_client_context_get_auth_user:
 * @client: a #SoupClientContext
 *
 * Checks whether the request associated with @client has been
 * authenticated, and if so returns the username that the client
 * authenticated as.
 *
 * Return value: the authenticated-as user, or %NULL if the request
 * was not authenticated.
 **/
const char *
soup_client_context_get_auth_user (SoupClientContext *client)
{
	g_return_val_if_fail (client != NULL, NULL);

	return client->auth_user;
}

/**
 * SoupServerCallback:
 * @server: the #SoupServer
 * @msg: the message being processed
 * @path: the path component of @msg's Request-URI
 * @query: (element-type utf8 utf8) (allow-none): the parsed query
 *         component of @msg's Request-URI
 * @client: additional contextual information about the client
 * @user_data: the data passed to @soup_server_add_handler
 *
 * A callback used to handle requests to a #SoupServer. The callback
 * will be invoked after receiving the request body; @msg's
 * #SoupMessage:method, #SoupMessage:request_headers, and
 * #SoupMessage:request_body fields will be filled in.
 *
 * @path and @query contain the likewise-named components of the
 * Request-URI, subject to certain assumptions. By default,
 * #SoupServer decodes all percent-encoding in the URI path, such that
 * "/foo%<!-- -->2Fbar" is treated the same as "/foo/bar". If your
 * server is serving resources in some non-POSIX-filesystem namespace,
 * you may want to distinguish those as two distinct paths. In that
 * case, you can set the %SOUP_SERVER_RAW_PATHS property when creating
 * the #SoupServer, and it will leave those characters undecoded. (You
 * may want to call soup_uri_normalize() to decode any percent-encoded
 * characters that you aren't handling specially.)
 *
 * @query contains the query component of the Request-URI parsed
 * according to the rules for HTML form handling. Although this is the
 * only commonly-used query string format in HTTP, there is nothing
 * that actually requires that HTTP URIs use that format; if your
 * server needs to use some other format, you can just ignore @query,
 * and call soup_message_get_uri() and parse the URI's query field
 * yourself.
 *
 * After determining what to do with the request, the callback must at
 * a minimum call soup_message_set_status() (or
 * soup_message_set_status_full()) on @msg to set the response status
 * code. Additionally, it may set response headers and/or fill in the
 * response body.
 *
 * If the callback cannot fully fill in the response before returning
 * (eg, if it needs to wait for information from a database, or
 * another network server), it should call soup_server_pause_message()
 * to tell #SoupServer to not send the response right away. When the
 * response is ready, call soup_server_unpause_message() to cause it
 * to be sent.
 *
 * To send the response body a bit at a time using "chunked" encoding,
 * first call soup_message_headers_set_encoding() to set
 * %SOUP_ENCODING_CHUNKED on the #SoupMessage:response_headers. Then call
 * soup_message_body_append() (or soup_message_body_append_buffer())
 * to append each chunk as it becomes ready, and
 * soup_server_unpause_message() to make sure it's running. (The
 * server will automatically pause the message if it is using chunked
 * encoding but no more chunks are available.) When you are done, call
 * soup_message_body_complete() to indicate that no more chunks are
 * coming.
 **/

/**
 * soup_server_add_handler:
 * @server: a #SoupServer
 * @path: (allow-none): the toplevel path for the handler
 * @callback: callback to invoke for requests under @path
 * @user_data: data for @callback
 * @destroy: destroy notifier to free @user_data
 *
 * Adds a handler to @server for requests under @path. See the
 * documentation for #SoupServerCallback for information about
 * how callbacks should behave.
 *
 * If @path is %NULL or "/", then this will be the default handler for
 * all requests that don't have a more specific handler. Note though
 * that if you want to handle requests to the special "*" URI, you
 * must explicitly register a handler for "*"; the default handler
 * will not be used for that case.
 **/
void
soup_server_add_handler (SoupServer            *server,
			 const char            *path,
			 SoupServerCallback     callback,
			 gpointer               user_data,
			 GDestroyNotify         destroy)
{
	SoupServerPrivate *priv;
	SoupServerHandler *hand;

	g_return_if_fail (SOUP_IS_SERVER (server));
	g_return_if_fail (callback != NULL);
	priv = SOUP_SERVER_GET_PRIVATE (server);

	/* "" was never documented as meaning the same this as "/",
	 * but it effectively was. We have to special case it now or
	 * otherwise it would match "*" too.
	 */
	if (path && (!*path || !strcmp (path, "/")))
		path = NULL;

	hand = g_slice_new0 (SoupServerHandler);
	hand->path       = g_strdup (path);
	hand->callback   = callback;
	hand->destroy    = destroy;
	hand->user_data  = user_data;

	soup_server_remove_handler (server, path);
	if (path)
		soup_path_map_add (priv->handlers, path, hand);
	else
		priv->default_handler = hand;
}

static void
unregister_handler (SoupServerHandler *handler)
{
	if (handler->destroy)
		handler->destroy (handler->user_data);
}

/**
 * soup_server_remove_handler:
 * @server: a #SoupServer
 * @path: the toplevel path for the handler
 *
 * Removes the handler registered at @path.
 **/
void
soup_server_remove_handler (SoupServer *server, const char *path)
{
	SoupServerPrivate *priv;
	SoupServerHandler *hand;

	g_return_if_fail (SOUP_IS_SERVER (server));
	priv = SOUP_SERVER_GET_PRIVATE (server);

	if (!path || !*path || !strcmp (path, "/")) {
		if (priv->default_handler) {
			unregister_handler (priv->default_handler);
			free_handler (priv->default_handler);
			priv->default_handler = NULL;
		}
		return;
	}

	hand = soup_path_map_lookup (priv->handlers, path);
	if (hand && !strcmp (path, hand->path)) {
		unregister_handler (hand);
		soup_path_map_remove (priv->handlers, path);
	}
}

/**
 * soup_server_add_auth_domain:
 * @server: a #SoupServer
 * @auth_domain: a #SoupAuthDomain
 *
 * Adds an authentication domain to @server. Each auth domain will
 * have the chance to require authentication for each request that
 * comes in; normally auth domains will require authentication for
 * requests on certain paths that they have been set up to watch, or
 * that meet other criteria set by the caller. If an auth domain
 * determines that a request requires authentication (and the request
 * doesn't contain authentication), @server will automatically reject
 * the request with an appropriate status (401 Unauthorized or 407
 * Proxy Authentication Required). If the request used the
 * "100-continue" Expectation, @server will reject it before the
 * request body is sent.
 **/
void
soup_server_add_auth_domain (SoupServer *server, SoupAuthDomain *auth_domain)
{
	SoupServerPrivate *priv;

	g_return_if_fail (SOUP_IS_SERVER (server));
	priv = SOUP_SERVER_GET_PRIVATE (server);

	priv->auth_domains = g_slist_append (priv->auth_domains, auth_domain);
	g_object_ref (auth_domain);
}

/**
 * soup_server_remove_auth_domain:
 * @server: a #SoupServer
 * @auth_domain: a #SoupAuthDomain
 *
 * Removes @auth_domain from @server.
 **/
void
soup_server_remove_auth_domain (SoupServer *server, SoupAuthDomain *auth_domain)
{
	SoupServerPrivate *priv;

	g_return_if_fail (SOUP_IS_SERVER (server));
	priv = SOUP_SERVER_GET_PRIVATE (server);

	priv->auth_domains = g_slist_remove (priv->auth_domains, auth_domain);
	g_object_unref (auth_domain);
}

/**
 * soup_server_pause_message:
 * @server: a #SoupServer
 * @msg: a #SoupMessage associated with @server.
 *
 * Pauses I/O on @msg. This can be used when you need to return from
 * the server handler without having the full response ready yet. Use
 * soup_server_unpause_message() to resume I/O.
 **/
void
soup_server_pause_message (SoupServer *server,
			   SoupMessage *msg)
{
	g_return_if_fail (SOUP_IS_SERVER (server));
	g_return_if_fail (SOUP_IS_MESSAGE (msg));

	soup_message_io_pause (msg);
}

/**
 * soup_server_unpause_message:
 * @server: a #SoupServer
 * @msg: a #SoupMessage associated with @server.
 *
 * Resumes I/O on @msg. Use this to resume after calling
 * soup_server_pause_message(), or after adding a new chunk to a
 * chunked response.
 *
 * I/O won't actually resume until you return to the main loop.
 **/
void
soup_server_unpause_message (SoupServer *server,
			     SoupMessage *msg)
{
	g_return_if_fail (SOUP_IS_SERVER (server));
	g_return_if_fail (SOUP_IS_MESSAGE (msg));

	soup_message_io_unpause (msg);
}

