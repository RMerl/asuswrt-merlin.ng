/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2012 Igalia, S.L.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "soup-content-processor.h"
#include "soup.h"

static void soup_content_processor_default_init (SoupContentProcessorInterface *interface);

G_DEFINE_INTERFACE (SoupContentProcessor, soup_content_processor, G_TYPE_OBJECT)

static GInputStream *
soup_content_processor_real_wrap_input (SoupContentProcessor *processor,
					GInputStream *base_stream,
					SoupMessage *msg,
					GError **error)
{
	g_return_val_if_reached (NULL);
}

static void
soup_content_processor_default_init (SoupContentProcessorInterface *interface)
{
	interface->processing_stage = SOUP_STAGE_INVALID;
	interface->wrap_input = soup_content_processor_real_wrap_input;
}

GInputStream *
soup_content_processor_wrap_input (SoupContentProcessor *processor,
				   GInputStream *base_stream,
				   SoupMessage *msg,
				   GError **error)
{
	g_return_val_if_fail (SOUP_IS_CONTENT_PROCESSOR (processor), NULL);

	return SOUP_CONTENT_PROCESSOR_GET_INTERFACE (processor)->wrap_input (processor, base_stream, msg, error);
}

SoupProcessingStage
soup_content_processor_get_processing_stage (SoupContentProcessor *processor)
{
	g_return_val_if_fail (SOUP_IS_CONTENT_PROCESSOR (processor), SOUP_STAGE_INVALID);

	return SOUP_CONTENT_PROCESSOR_GET_INTERFACE (processor)->processing_stage;
}
