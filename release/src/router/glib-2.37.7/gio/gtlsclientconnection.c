/* GIO - GLib Input, Output and Streaming Library
 *
 * Copyright © 2010 Red Hat, Inc
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
 */

#include "config.h"
#include "glib.h"

#include "gtlsclientconnection.h"
#include "ginitable.h"
#include "gioenumtypes.h"
#include "gsocket.h"
#include "gsocketconnectable.h"
#include "gtlsbackend.h"
#include "gtlscertificate.h"
#include "glibintl.h"

/**
 * SECTION:gtlsclientconnection
 * @short_description: TLS client-side connection
 * @include: gio/gio.h
 *
 * #GTlsClientConnection is the client-side subclass of
 * #GTlsConnection, representing a client-side TLS connection.
 */

/**
 * GTlsClientConnection:
 *
 * Abstract base class for the backend-specific client connection
 * type.
 *
 * Since: 2.28
 */

G_DEFINE_INTERFACE (GTlsClientConnection, g_tls_client_connection, G_TYPE_TLS_CONNECTION)

static void
g_tls_client_connection_default_init (GTlsClientConnectionInterface *iface)
{
  /**
   * GTlsClientConnection:validation-flags:
   *
   * What steps to perform when validating a certificate received from
   * a server. Server certificates that fail to validate in all of the
   * ways indicated here will be rejected unless the application
   * overrides the default via #GTlsConnection::accept-certificate.
   *
   * Since: 2.28
   */
  g_object_interface_install_property (iface,
				       g_param_spec_flags ("validation-flags",
							   P_("Validation flags"),
							   P_("What certificate validation to perform"),
							   G_TYPE_TLS_CERTIFICATE_FLAGS,
							   G_TLS_CERTIFICATE_VALIDATE_ALL,
							   G_PARAM_READWRITE |
							   G_PARAM_CONSTRUCT |
							   G_PARAM_STATIC_STRINGS));

  /**
   * GTlsClientConnection:server-identity:
   *
   * A #GSocketConnectable describing the identity of the server that
   * is expected on the other end of the connection.
   *
   * If the %G_TLS_CERTIFICATE_BAD_IDENTITY flag is set in
   * #GTlsClientConnection:validation-flags, this object will be used
   * to determine the expected identify of the remote end of the
   * connection; if #GTlsClientConnection:server-identity is not set,
   * or does not match the identity presented by the server, then the
   * %G_TLS_CERTIFICATE_BAD_IDENTITY validation will fail.
   *
   * In addition to its use in verifying the server certificate,
   * this is also used to give a hint to the server about what
   * certificate we expect, which is useful for servers that serve
   * virtual hosts.
   *
   * Since: 2.28
   */
  g_object_interface_install_property (iface,
				       g_param_spec_object ("server-identity",
							    P_("Server identity"),
							    P_("GSocketConnectable identifying the server"),
							    G_TYPE_SOCKET_CONNECTABLE,
							    G_PARAM_READWRITE |
							    G_PARAM_CONSTRUCT |
							    G_PARAM_STATIC_STRINGS));

  /**
   * GTlsClientConnection:use-ssl3:
   *
   * If %TRUE, tells the connection to use SSL 3.0 rather than trying
   * to negotiate the best version of TLS or SSL to use. This can be
   * used when talking to servers that don't implement version
   * negotiation correctly and therefore refuse to handshake at all with
   * a "modern" TLS handshake.
   *
   * Since: 2.28
   */
  g_object_interface_install_property (iface,
				       g_param_spec_boolean ("use-ssl3",
							     P_("Use SSL3"),
							     P_("Use SSL 3.0 rather than trying to use TLS 1.x"),
							     FALSE,
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT |
							     G_PARAM_STATIC_STRINGS));

  /**
   * GTlsClientConnection:accepted-cas:
   *
   * A list of the distinguished names of the Certificate Authorities
   * that the server will accept client certificates signed by. If the
   * server requests a client certificate during the handshake, then
   * this property will be set after the handshake completes.
   *
   * Each item in the list is a #GByteArray which contains the complete
   * subject DN of the certificate authority.
   *
   * Since: 2.28
   */
  g_object_interface_install_property (iface,
				       g_param_spec_pointer ("accepted-cas",
							     P_("Accepted CAs"),
							     P_("Distinguished names of the CAs the server accepts certificates from"),
							     G_PARAM_READABLE |
							     G_PARAM_STATIC_STRINGS));
}

/**
 * g_tls_client_connection_new:
 * @base_io_stream: the #GIOStream to wrap
 * @server_identity: (allow-none): the expected identity of the server
 * @error: #GError for error reporting, or %NULL to ignore.
 *
 * Creates a new #GTlsClientConnection wrapping @base_io_stream (which
 * must have pollable input and output streams) which is assumed to
 * communicate with the server identified by @server_identity.
 *
 * Return value: (transfer full) (type GTlsClientConnection): the new
 * #GTlsClientConnection, or %NULL on error
 *
 * Since: 2.28
 */
GIOStream *
g_tls_client_connection_new (GIOStream           *base_io_stream,
			     GSocketConnectable  *server_identity,
			     GError             **error)
{
  GObject *conn;
  GTlsBackend *backend;

  backend = g_tls_backend_get_default ();
  conn = g_initable_new (g_tls_backend_get_client_connection_type (backend),
			 NULL, error,
			 "base-io-stream", base_io_stream,
			 "server-identity", server_identity,
			 NULL);
  return G_IO_STREAM (conn);
}

/**
 * g_tls_client_connection_get_validation_flags:
 * @conn: the #GTlsClientConnection
 *
 * Gets @conn's validation flags
 *
 * Return value: the validation flags
 *
 * Since: 2.28
 */
GTlsCertificateFlags
g_tls_client_connection_get_validation_flags (GTlsClientConnection *conn)
{
  GTlsCertificateFlags flags = 0;

  g_return_val_if_fail (G_IS_TLS_CLIENT_CONNECTION (conn), 0);

  g_object_get (G_OBJECT (conn), "validation-flags", &flags, NULL);
  return flags;
}

/**
 * g_tls_client_connection_set_validation_flags:
 * @conn: the #GTlsClientConnection
 * @flags: the #GTlsCertificateFlags to use
 *
 * Sets @conn's validation flags, to override the default set of
 * checks performed when validating a server certificate. By default,
 * %G_TLS_CERTIFICATE_VALIDATE_ALL is used.
 *
 * Since: 2.28
 */
void
g_tls_client_connection_set_validation_flags (GTlsClientConnection  *conn,
					      GTlsCertificateFlags   flags)
{
  g_return_if_fail (G_IS_TLS_CLIENT_CONNECTION (conn));

  g_object_set (G_OBJECT (conn), "validation-flags", flags, NULL);
}

/**
 * g_tls_client_connection_get_server_identity:
 * @conn: the #GTlsClientConnection
 *
 * Gets @conn's expected server identity
 *
 * Return value: (transfer none): a #GSocketConnectable describing the
 * expected server identity, or %NULL if the expected identity is not
 * known.
 *
 * Since: 2.28
 */
GSocketConnectable *
g_tls_client_connection_get_server_identity (GTlsClientConnection *conn)
{
  GSocketConnectable *identity = NULL;

  g_return_val_if_fail (G_IS_TLS_CLIENT_CONNECTION (conn), 0);

  g_object_get (G_OBJECT (conn), "server-identity", &identity, NULL);
  if (identity)
    g_object_unref (identity);
  return identity;
}

/**
 * g_tls_client_connection_set_server_identity:
 * @conn: the #GTlsClientConnection
 * @identity: a #GSocketConnectable describing the expected server identity
 *
 * Sets @conn's expected server identity, which is used both to tell
 * servers on virtual hosts which certificate to present, and also
 * to let @conn know what name to look for in the certificate when
 * performing %G_TLS_CERTIFICATE_BAD_IDENTITY validation, if enabled.
 *
 * Since: 2.28
 */
void
g_tls_client_connection_set_server_identity (GTlsClientConnection *conn,
					     GSocketConnectable   *identity)
{
  g_return_if_fail (G_IS_TLS_CLIENT_CONNECTION (conn));

  g_object_set (G_OBJECT (conn), "server-identity", identity, NULL);
}

/**
 * g_tls_client_connection_get_use_ssl3:
 * @conn: the #GTlsClientConnection
 *
 * Gets whether @conn will use SSL 3.0 rather than the
 * highest-supported version of TLS; see
 * g_tls_client_connection_set_use_ssl3().
 *
 * Return value: whether @conn will use SSL 3.0
 *
 * Since: 2.28
 */
gboolean
g_tls_client_connection_get_use_ssl3 (GTlsClientConnection *conn)
{
  gboolean use_ssl3 = FALSE;

  g_return_val_if_fail (G_IS_TLS_CLIENT_CONNECTION (conn), 0);

  g_object_get (G_OBJECT (conn), "use-ssl3", &use_ssl3, NULL);
  return use_ssl3;
}

/**
 * g_tls_client_connection_set_use_ssl3:
 * @conn: the #GTlsClientConnection
 * @use_ssl3: whether to use SSL 3.0
 *
 * If @use_ssl3 is %TRUE, this forces @conn to use SSL 3.0 rather than
 * trying to properly negotiate the right version of TLS or SSL to use.
 * This can be used when talking to servers that do not implement the
 * fallbacks correctly and which will therefore fail to handshake with
 * a "modern" TLS handshake attempt.
 *
 * Since: 2.28
 */
void
g_tls_client_connection_set_use_ssl3 (GTlsClientConnection *conn,
				      gboolean              use_ssl3)
{
  g_return_if_fail (G_IS_TLS_CLIENT_CONNECTION (conn));

  g_object_set (G_OBJECT (conn), "use-ssl3", use_ssl3, NULL);
}

/**
 * g_tls_client_connection_get_accepted_cas:
 * @conn: the #GTlsClientConnection
 *
 * Gets the list of distinguished names of the Certificate Authorities
 * that the server will accept certificates from. This will be set
 * during the TLS handshake if the server requests a certificate.
 * Otherwise, it will be %NULL.
 *
 * Each item in the list is a #GByteArray which contains the complete
 * subject DN of the certificate authority.
 *
 * Return value: (element-type GByteArray) (transfer full): the list of
 * CA DNs. You should unref each element with g_byte_array_unref() and then
 * the free the list with g_list_free().
 *
 * Since: 2.28
 */
GList *
g_tls_client_connection_get_accepted_cas (GTlsClientConnection *conn)
{
  GList *accepted_cas = NULL;

  g_return_val_if_fail (G_IS_TLS_CLIENT_CONNECTION (conn), NULL);

  g_object_get (G_OBJECT (conn), "accepted-cas", &accepted_cas, NULL);
  return accepted_cas;
}
