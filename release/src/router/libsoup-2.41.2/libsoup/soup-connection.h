/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2000-2003, Ximian, Inc.
 */

#ifndef SOUP_CONNECTION_H
#define SOUP_CONNECTION_H 1

#include "soup-types.h"
#include "soup-message-private.h"
#include "soup-misc.h"

G_BEGIN_DECLS

#define SOUP_TYPE_CONNECTION            (soup_connection_get_type ())
#define SOUP_CONNECTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SOUP_TYPE_CONNECTION, SoupConnection))
#define SOUP_CONNECTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_TYPE_CONNECTION, SoupConnectionClass))
#define SOUP_IS_CONNECTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SOUP_TYPE_CONNECTION))
#define SOUP_IS_CONNECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), SOUP_TYPE_CONNECTION))
#define SOUP_CONNECTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_TYPE_CONNECTION, SoupConnectionClass))

struct _SoupConnection {
	GObject parent;

};

typedef struct {
	GObjectClass parent_class;

	/* signals */
	void (*disconnected)    (SoupConnection *);

} SoupConnectionClass;

GType soup_connection_get_type (void);


typedef void  (*SoupConnectionCallback)        (SoupConnection   *conn,
						guint             status,
						gpointer          data);

#define SOUP_CONNECTION_REMOTE_URI      "remote-uri"
#define SOUP_CONNECTION_PROXY_RESOLVER  "proxy-resolver"
#define SOUP_CONNECTION_SSL             "ssl"
#define SOUP_CONNECTION_SSL_CREDENTIALS "ssl-creds"
#define SOUP_CONNECTION_SSL_STRICT      "ssl-strict"
#define SOUP_CONNECTION_SSL_FALLBACK    "ssl-fallback"
#define SOUP_CONNECTION_ASYNC_CONTEXT   "async-context"
#define SOUP_CONNECTION_USE_THREAD_CONTEXT "use-thread-context"
#define SOUP_CONNECTION_TIMEOUT         "timeout"
#define SOUP_CONNECTION_IDLE_TIMEOUT    "idle-timeout"
#define SOUP_CONNECTION_STATE           "state"
#define SOUP_CONNECTION_MESSAGE         "message"

void            soup_connection_connect_async  (SoupConnection   *conn,
						GCancellable     *cancellable,
						SoupConnectionCallback callback,
						gpointer          user_data);
guint           soup_connection_connect_sync   (SoupConnection   *conn,
						GCancellable     *cancellable);
guint           soup_connection_start_ssl_sync   (SoupConnection   *conn,
						  GCancellable     *cancellable);
void            soup_connection_start_ssl_async  (SoupConnection   *conn,
						  GCancellable     *cancellable,
						  SoupConnectionCallback callback,
						  gpointer          user_data);

void            soup_connection_disconnect     (SoupConnection   *conn);

SoupSocket     *soup_connection_get_socket     (SoupConnection   *conn);
SoupURI        *soup_connection_get_remote_uri (SoupConnection   *conn);
SoupURI        *soup_connection_get_proxy_uri  (SoupConnection   *conn);
gboolean        soup_connection_is_via_proxy   (SoupConnection   *conn);
gboolean        soup_connection_is_tunnelled   (SoupConnection   *conn);

SoupConnectionState soup_connection_get_state  (SoupConnection   *conn);
void                soup_connection_set_state  (SoupConnection   *conn,
						SoupConnectionState state);

gboolean        soup_connection_get_ever_used  (SoupConnection   *conn);

void            soup_connection_send_request   (SoupConnection          *conn,
						SoupMessageQueueItem    *item,
						SoupMessageCompletionFn  completion_cb,
						gpointer                 user_data);

gboolean        soup_connection_get_ssl_fallback (SoupConnection   *conn);

G_END_DECLS

#endif /* SOUP_CONNECTION_H */
