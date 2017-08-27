/* GIO - GLib Input, Output and Streaming Library
 *
 * Copyright (C) 2010 Red Hat, Inc.
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

#ifndef __G_TLS_CLIENT_CONNECTION_H__
#define __G_TLS_CLIENT_CONNECTION_H__

#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
#error "Only <gio/gio.h> can be included directly."
#endif

#include <gio/gtlsconnection.h>

G_BEGIN_DECLS

#define G_TYPE_TLS_CLIENT_CONNECTION                (g_tls_client_connection_get_type ())
#define G_TLS_CLIENT_CONNECTION(inst)               (G_TYPE_CHECK_INSTANCE_CAST ((inst), G_TYPE_TLS_CLIENT_CONNECTION, GTlsClientConnection))
#define G_IS_TLS_CLIENT_CONNECTION(inst)            (G_TYPE_CHECK_INSTANCE_TYPE ((inst), G_TYPE_TLS_CLIENT_CONNECTION))
#define G_TLS_CLIENT_CONNECTION_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE ((inst), G_TYPE_TLS_CLIENT_CONNECTION, GTlsClientConnectionInterface))

typedef struct _GTlsClientConnectionInterface GTlsClientConnectionInterface;

struct _GTlsClientConnectionInterface
{
  GTypeInterface g_iface;

};

GLIB_AVAILABLE_IN_ALL
GType                 g_tls_client_connection_get_type             (void) G_GNUC_CONST;

GLIB_AVAILABLE_IN_ALL
GIOStream *           g_tls_client_connection_new                  (GIOStream               *base_io_stream,
								    GSocketConnectable      *server_identity,
								    GError                 **error);

GLIB_AVAILABLE_IN_ALL
GTlsCertificateFlags  g_tls_client_connection_get_validation_flags (GTlsClientConnection    *conn);
GLIB_AVAILABLE_IN_ALL
void                  g_tls_client_connection_set_validation_flags (GTlsClientConnection    *conn,
								    GTlsCertificateFlags     flags);
GLIB_AVAILABLE_IN_ALL
GSocketConnectable   *g_tls_client_connection_get_server_identity  (GTlsClientConnection    *conn);
GLIB_AVAILABLE_IN_ALL
void                  g_tls_client_connection_set_server_identity  (GTlsClientConnection    *conn,
								    GSocketConnectable      *identity);
GLIB_AVAILABLE_IN_ALL
gboolean              g_tls_client_connection_get_use_ssl3         (GTlsClientConnection    *conn);
GLIB_AVAILABLE_IN_ALL
void                  g_tls_client_connection_set_use_ssl3         (GTlsClientConnection    *conn,
								    gboolean                 use_ssl3);
GLIB_AVAILABLE_IN_ALL
GList *               g_tls_client_connection_get_accepted_cas     (GTlsClientConnection    *conn);

G_END_DECLS

#endif /* __G_TLS_CLIENT_CONNECTION_H__ */
