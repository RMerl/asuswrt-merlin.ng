/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright 2011 Igalia, S.L.
 * Copyright 2011 Red Hat, Inc.
 */

#ifndef SOUP_MISC_PRIVATE_H
#define SOUP_MISC_PRIVATE_H 1

#include "soup-socket.h"

char *uri_decoded_copy (const char *str, int length);

guint soup_socket_handshake_sync  (SoupSocket         *sock,
				   GCancellable       *cancellable);
void  soup_socket_handshake_async (SoupSocket         *sock,
				   GCancellable       *cancellable,
				   SoupSocketCallback  callback,
				   gpointer            user_data);

GSocket   *soup_socket_get_gsocket    (SoupSocket *sock);
GIOStream *soup_socket_get_connection (SoupSocket *sock);
GIOStream *soup_socket_get_iostream   (SoupSocket *sock);

#define SOUP_SOCKET_CLEAN_DISPOSE "clean-dispose"
#define SOUP_SOCKET_USE_PROXY     "use-proxy"
SoupURI *soup_socket_get_http_proxy_uri (SoupSocket *sock);

/* At some point it might be possible to mark additional methods
 * safe or idempotent...
 */
#define SOUP_METHOD_IS_SAFE(method) (method == SOUP_METHOD_GET || \
				     method == SOUP_METHOD_HEAD || \
				     method == SOUP_METHOD_OPTIONS || \
				     method == SOUP_METHOD_PROPFIND)

#define SOUP_METHOD_IS_IDEMPOTENT(method) (method == SOUP_METHOD_GET || \
					   method == SOUP_METHOD_HEAD || \
					   method == SOUP_METHOD_OPTIONS || \
					   method == SOUP_METHOD_PROPFIND || \
					   method == SOUP_METHOD_PUT || \
					   method == SOUP_METHOD_DELETE)

GSource *soup_add_completion_reffed (GMainContext *async_context,
				     GSourceFunc   function,
				     gpointer      data);

#endif /* SOUP_MISC_PRIVATE_H */
