/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2012 Collabora Ltd.
 */

#ifndef SOUP_MULTIPART_INPUT_STREAM_H
#define SOUP_MULTIPART_INPUT_STREAM_H 1

#include <libsoup/soup-types.h>
#include <libsoup/soup-message-headers.h>

G_BEGIN_DECLS

#define SOUP_TYPE_MULTIPART_INPUT_STREAM         (soup_multipart_input_stream_get_type ())
#define SOUP_MULTIPART_INPUT_STREAM(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), SOUP_TYPE_MULTIPART_INPUT_STREAM, SoupMultipartInputStream))
#define SOUP_MULTIPART_INPUT_STREAM_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), SOUP_TYPE_MULTIPART_INPUT_STREAM, SoupMultipartInputStreamClass))
#define SOUP_IS_MULTIPART_INPUT_STREAM(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), SOUP_TYPE_MULTIPART_INPUT_STREAM))
#define SOUP_IS_MULTIPART_INPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), SOUP_TYPE_MULTIPART_INPUT_STREAM))
#define SOUP_MULTIPART_INPUT_STREAM_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), SOUP_TYPE_MULTIPART_INPUT_STREAM, SoupMultipartInputStreamClass))

typedef struct _SoupMultipartInputStream        SoupMultipartInputStream;
typedef struct _SoupMultipartInputStreamPrivate SoupMultipartInputStreamPrivate;
typedef struct _SoupMultipartInputStreamClass   SoupMultipartInputStreamClass;

struct _SoupMultipartInputStream {
	GFilterInputStream parent_instance;

	/*< private >*/
	SoupMultipartInputStreamPrivate *priv;
};

struct _SoupMultipartInputStreamClass {
	GFilterInputStreamClass parent_class;
};

SOUP_AVAILABLE_IN_2_40
GType                     soup_multipart_input_stream_get_type         (void) G_GNUC_CONST;

SOUP_AVAILABLE_IN_2_40
SoupMultipartInputStream *soup_multipart_input_stream_new              (SoupMessage               *msg,
							                GInputStream              *base_stream);

SOUP_AVAILABLE_IN_2_40
GInputStream             *soup_multipart_input_stream_next_part        (SoupMultipartInputStream  *multipart,
									GCancellable	          *cancellable,
									GError                   **error);

SOUP_AVAILABLE_IN_2_40
void                      soup_multipart_input_stream_next_part_async  (SoupMultipartInputStream  *multipart,
									int                        io_priority,
								        GCancellable              *cancellable,
								        GAsyncReadyCallback        callback,
								        gpointer                   data);

SOUP_AVAILABLE_IN_2_40
GInputStream             *soup_multipart_input_stream_next_part_finish (SoupMultipartInputStream  *multipart,
									GAsyncResult              *res,
									GError                   **error);

SOUP_AVAILABLE_IN_2_40
SoupMessageHeaders       *soup_multipart_input_stream_get_headers      (SoupMultipartInputStream  *multipart);


G_END_DECLS

#endif /* SOUP_MULTIPART_INPUT_STREAM_H */
