/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-content-decoder.c
 *
 * Copyright (C) 2009 Red Hat, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "soup-content-decoder.h"
#include "soup-converter-wrapper.h"
#include "soup.h"
#include "soup-message-private.h"

/**
 * SECTION:soup-content-decoder
 * @short_description: Content-Encoding handler
 *
 * #SoupContentDecoder handles the "Accept-Encoding" header on
 * outgoing messages, and the "Content-Encoding" header on incoming
 * ones. If you add it to a session with soup_session_add_feature() or
 * soup_session_add_feature_by_type(), the session will automatically
 * use Content-Encoding as appropriate.
 *
 * (Note that currently there is no way to (automatically) use
 * Content-Encoding when sending a request body, or to pick specific
 * encoding types to support.)
 *
 * If #SoupContentDecoder successfully decodes the Content-Encoding,
 * it will set the %SOUP_MESSAGE_CONTENT_DECODED flag on the message,
 * and the message body and the chunks in the #SoupMessage::got_chunk
 * signals will contain the decoded data; however, the message headers
 * will be unchanged (and so "Content-Encoding" will still be present,
 * "Content-Length" will describe the original encoded length, etc).
 *
 * If "Content-Encoding" contains any encoding types that
 * #SoupContentDecoder doesn't recognize, then none of the encodings
 * will be decoded (and the %SOUP_MESSAGE_CONTENT_DECODED flag will
 * not be set).
 *
 * Since: 2.30
 **/

struct _SoupContentDecoderPrivate {
	GHashTable *decoders;
};

typedef GConverter * (*SoupContentDecoderCreator) (void);

static void soup_content_decoder_session_feature_init (SoupSessionFeatureInterface *feature_interface, gpointer interface_data);

static SoupContentProcessorInterface *soup_content_decoder_default_content_processor_interface;
static void soup_content_decoder_content_processor_init (SoupContentProcessorInterface *interface, gpointer interface_data);


G_DEFINE_TYPE_WITH_CODE (SoupContentDecoder, soup_content_decoder, G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (SOUP_TYPE_SESSION_FEATURE,
						soup_content_decoder_session_feature_init)
			 G_IMPLEMENT_INTERFACE (SOUP_TYPE_CONTENT_PROCESSOR,
						soup_content_decoder_content_processor_init))

static GSList *
soup_content_decoder_get_decoders_for_msg (SoupContentDecoder *decoder, SoupMessage *msg)
{
	const char *header;
	GSList *encodings, *e, *decoders = NULL;
	SoupContentDecoderCreator converter_creator;
	GConverter *converter;

	header = soup_message_headers_get_list (msg->response_headers,
						"Content-Encoding");
	if (!header)
		return NULL;

	/* Workaround for an apache bug (bgo 613361) */
	if (!g_ascii_strcasecmp (header, "gzip") ||
	    !g_ascii_strcasecmp (header, "x-gzip")) {
		const char *content_type = soup_message_headers_get_content_type (msg->response_headers, NULL);

		if (content_type &&
		    (!g_ascii_strcasecmp (content_type, "application/gzip") ||
		     !g_ascii_strcasecmp (content_type, "application/x-gzip")))
			return NULL;
	}

	/* OK, really, no one is ever going to use more than one
	 * encoding, but we'll be robust.
	 */
	encodings = soup_header_parse_list (header);
	if (!encodings)
		return NULL;

	for (e = encodings; e; e = e->next) {
		if (!g_hash_table_lookup (decoder->priv->decoders, e->data)) {
			soup_header_free_list (encodings);
			return NULL;
		}
	}

	for (e = encodings; e; e = e->next) {
		converter_creator = g_hash_table_lookup (decoder->priv->decoders, e->data);
		converter = converter_creator ();

		/* Content-Encoding lists the codings in the order
		 * they were applied in, so we put decoders in reverse
		 * order so the last-applied will be the first
		 * decoded.
		 */
		decoders = g_slist_prepend (decoders, converter);
	}
	soup_header_free_list (encodings);

	return decoders;
}

static GInputStream*
soup_content_decoder_content_processor_wrap_input (SoupContentProcessor *processor,
						   GInputStream *base_stream,
						   SoupMessage *msg,
						   GError **error)
{
	GSList *decoders, *d;
	GInputStream *istream;

	decoders = soup_content_decoder_get_decoders_for_msg (SOUP_CONTENT_DECODER (processor), msg);
	if (!decoders)
		return NULL;

	istream = g_object_ref (base_stream);
	for (d = decoders; d; d = d->next) {
		GConverter *decoder, *wrapper;
		GInputStream *filter;

		decoder = d->data;
		wrapper = soup_converter_wrapper_new (decoder, msg);
		filter = g_object_new (G_TYPE_CONVERTER_INPUT_STREAM,
				       "base-stream", istream,
				       "converter", wrapper,
				       NULL);
		g_object_unref (istream);
		g_object_unref (wrapper);
		istream = filter;
	}

	g_slist_free_full (decoders, g_object_unref);

	return istream;
}

static void
soup_content_decoder_content_processor_init (SoupContentProcessorInterface *processor_interface,
					     gpointer interface_data)
{
	soup_content_decoder_default_content_processor_interface =
		g_type_default_interface_peek (SOUP_TYPE_CONTENT_PROCESSOR);

	processor_interface->processing_stage = SOUP_STAGE_CONTENT_ENCODING;
	processor_interface->wrap_input = soup_content_decoder_content_processor_wrap_input;
}

/* This is constant for now */
#define ACCEPT_ENCODING_HEADER "gzip, deflate"

static GConverter *
gzip_decoder_creator (void)
{
	return (GConverter *)g_zlib_decompressor_new (G_ZLIB_COMPRESSOR_FORMAT_GZIP);
}

static GConverter *
zlib_decoder_creator (void)
{
	return (GConverter *)g_zlib_decompressor_new (G_ZLIB_COMPRESSOR_FORMAT_ZLIB);
}

static void
soup_content_decoder_init (SoupContentDecoder *decoder)
{
	decoder->priv = G_TYPE_INSTANCE_GET_PRIVATE (decoder,
						     SOUP_TYPE_CONTENT_DECODER,
						     SoupContentDecoderPrivate);

	decoder->priv->decoders = g_hash_table_new (g_str_hash, g_str_equal);
	/* Hardcoded for now */
	g_hash_table_insert (decoder->priv->decoders, "gzip",
			     gzip_decoder_creator);
	g_hash_table_insert (decoder->priv->decoders, "x-gzip",
			     gzip_decoder_creator);
	g_hash_table_insert (decoder->priv->decoders, "deflate",
			     zlib_decoder_creator);
}

static void
soup_content_decoder_finalize (GObject *object)
{
	SoupContentDecoder *decoder = SOUP_CONTENT_DECODER (object);

	g_hash_table_destroy (decoder->priv->decoders);

	G_OBJECT_CLASS (soup_content_decoder_parent_class)->finalize (object);
}

static void
soup_content_decoder_class_init (SoupContentDecoderClass *decoder_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (decoder_class);

	g_type_class_add_private (decoder_class, sizeof (SoupContentDecoderPrivate));

	object_class->finalize = soup_content_decoder_finalize;
}

static void
soup_content_decoder_request_queued (SoupSessionFeature *feature,
				     SoupSession *session,
				     SoupMessage *msg)
{
	if (!soup_message_headers_get_one (msg->request_headers,
					   "Accept-Encoding")) {
		soup_message_headers_append (msg->request_headers,
					     "Accept-Encoding",
					     ACCEPT_ENCODING_HEADER);
	}
}

static void
soup_content_decoder_session_feature_init (SoupSessionFeatureInterface *feature_interface,
					   gpointer interface_data)
{
	feature_interface->request_queued = soup_content_decoder_request_queued;
}
