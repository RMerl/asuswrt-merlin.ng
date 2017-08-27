/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2000-2003, Ximian, Inc.
 */

#ifndef SOUP_SESSION_PRIVATE_H
#define SOUP_SESSION_PRIVATE_H 1

#include "soup-session.h"
#include "soup-message-private.h"
#include "soup-proxy-uri-resolver.h"

G_BEGIN_DECLS

/* "protected" methods for subclasses */
SoupMessageQueue     *soup_session_get_queue            (SoupSession          *session);

SoupMessageQueueItem *soup_session_append_queue_item    (SoupSession          *session,
							 SoupMessage          *msg,
							 SoupSessionCallback   callback,
							 gpointer              user_data);
SoupMessageQueueItem *soup_session_make_connect_message (SoupSession          *session,
							 SoupConnection       *conn);
gboolean              soup_session_get_connection       (SoupSession          *session,
							 SoupMessageQueueItem *item,
							 gboolean             *try_pruning);
gboolean              soup_session_cleanup_connections  (SoupSession          *session,
							 gboolean              prune_idle);
void                  soup_session_send_queue_item      (SoupSession          *session,
							 SoupMessageQueueItem *item,
							 SoupMessageCompletionFn completion_cb);
void                  soup_session_unqueue_item         (SoupSession          *session,
							 SoupMessageQueueItem *item);
void                  soup_session_set_item_connection  (SoupSession          *session,
							 SoupMessageQueueItem *item,
							 SoupConnection       *conn);
void                  soup_session_set_item_status      (SoupSession          *session,
							 SoupMessageQueueItem *item,
							 guint                 status_code);

GInputStream         *soup_session_send_request         (SoupSession          *session,
							 SoupMessage          *msg,
							 GCancellable         *cancellable,
							 GError              **error);

void                  soup_session_send_request_async   (SoupSession          *session,
							 SoupMessage          *msg,
							 GCancellable         *cancellable,
							 GAsyncReadyCallback   callback,
							 gpointer              user_data);
GInputStream         *soup_session_send_request_finish  (SoupSession          *session,
							 GAsyncResult         *result,
							 GError              **error);

G_END_DECLS

#endif /* SOUP_SESSION_PRIVATE_H */
