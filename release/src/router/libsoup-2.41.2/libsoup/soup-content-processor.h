/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2012 Igalia, S.L.
 */


#ifndef SOUP_CONTENT_PROCESSOR_H
#define SOUP_CONTENT_PROCESSOR_H 1

#include <libsoup/soup-types.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define SOUP_TYPE_CONTENT_PROCESSOR                 (soup_content_processor_get_type ())
#define SOUP_CONTENT_PROCESSOR(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), SOUP_TYPE_CONTENT_PROCESSOR, SoupContentProcessor))
#define SOUP_IS_CONTENT_PROCESSOR(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SOUP_TYPE_CONTENT_PROCESSOR))
#define SOUP_CONTENT_PROCESSOR_GET_INTERFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), SOUP_TYPE_CONTENT_PROCESSOR, SoupContentProcessorInterface))

typedef enum {
	SOUP_STAGE_INVALID,

	SOUP_STAGE_MESSAGE_BODY,      /* Raw network data */
	SOUP_STAGE_TRANSFER_ENCODING, /* SoupBodyInputStream is here */
	SOUP_STAGE_ENTITY_BODY,       /* Has Transfer-Encoding removed */
	SOUP_STAGE_CONTENT_ENCODING,  /* SoupContentDecoder works here */
	SOUP_STAGE_BODY_DATA          /* Actual body data */
} SoupProcessingStage;

typedef struct _SoupContentProcessor             SoupContentProcessor;
typedef struct _SoupContentProcessorInterface    SoupContentProcessorInterface;

struct _SoupContentProcessorInterface {
	GTypeInterface parent;

	SoupProcessingStage processing_stage;

	/* methods */
	GInputStream*       (*wrap_input)             (SoupContentProcessor *processor,
						       GInputStream         *base_stream,
						       SoupMessage          *msg,
						       GError              **error);
};

GType soup_content_processor_get_type (void);

GInputStream       *soup_content_processor_wrap_input           (SoupContentProcessor *processor,
								 GInputStream         *base_stream,
								 SoupMessage          *msg,
								 GError              **error);

SoupProcessingStage soup_content_processor_get_processing_stage (SoupContentProcessor *processor);

G_END_DECLS

#endif /* SOUP_CONTENT_PROCESSOR_H */
