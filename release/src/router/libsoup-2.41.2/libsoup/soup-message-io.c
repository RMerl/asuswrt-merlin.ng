/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-message-io.c: HTTP message I/O
 *
 * Copyright (C) 2000-2003, Ximian, Inc.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib/gi18n-lib.h>

#include "soup.h"
#include "soup-body-input-stream.h"
#include "soup-body-output-stream.h"
#include "soup-client-input-stream.h"
#include "soup-connection.h"
#include "soup-content-processor.h"
#include "soup-content-sniffer-stream.h"
#include "soup-filter-input-stream.h"
#include "soup-message-private.h"
#include "soup-message-queue.h"
#include "soup-misc-private.h"

typedef enum {
	SOUP_MESSAGE_IO_CLIENT,
	SOUP_MESSAGE_IO_SERVER
} SoupMessageIOMode;

typedef enum {
	SOUP_MESSAGE_IO_STATE_NOT_STARTED,
	SOUP_MESSAGE_IO_STATE_ANY = SOUP_MESSAGE_IO_STATE_NOT_STARTED,
	SOUP_MESSAGE_IO_STATE_HEADERS,
	SOUP_MESSAGE_IO_STATE_BLOCKING,
	SOUP_MESSAGE_IO_STATE_BODY_START,
	SOUP_MESSAGE_IO_STATE_BODY,
	SOUP_MESSAGE_IO_STATE_BODY_DATA,
	SOUP_MESSAGE_IO_STATE_BODY_DONE,
	SOUP_MESSAGE_IO_STATE_FINISHING,
	SOUP_MESSAGE_IO_STATE_DONE
} SoupMessageIOState;

#define SOUP_MESSAGE_IO_STATE_ACTIVE(state) \
	(state != SOUP_MESSAGE_IO_STATE_NOT_STARTED && \
	 state != SOUP_MESSAGE_IO_STATE_BLOCKING && \
	 state != SOUP_MESSAGE_IO_STATE_DONE)
#define SOUP_MESSAGE_IO_STATE_POLLABLE(state) \
	(SOUP_MESSAGE_IO_STATE_ACTIVE (state) && \
	 state != SOUP_MESSAGE_IO_STATE_BODY_DONE)

typedef struct {
	SoupMessageQueueItem *item;
	SoupMessageIOMode     mode;
	GCancellable         *cancellable;

	GIOStream              *iostream;
	SoupFilterInputStream  *istream;
	GInputStream           *body_istream;
	GOutputStream          *ostream;
	GOutputStream          *body_ostream;
	GMainContext           *async_context;
	gboolean                blocking;

	SoupMessageIOState    read_state;
	SoupEncoding          read_encoding;
	GByteArray           *read_header_buf;
	SoupMessageBody      *read_body;
	goffset               read_length;

	SoupMessageIOState    write_state;
	SoupEncoding          write_encoding;
	GString              *write_buf;
	SoupMessageBody      *write_body;
	SoupBuffer           *write_chunk;
	goffset               write_body_offset;
	goffset               write_length;
	goffset               written;

	GSource *io_source;
	GSource *unpause_source;
	gboolean paused;

	SoupMessageGetHeadersFn   get_headers_cb;
	SoupMessageParseHeadersFn parse_headers_cb;
	gpointer                  header_data;
	SoupMessageCompletionFn   completion_cb;
	gpointer                  completion_data;
} SoupMessageIOData;
	

#define RESPONSE_BLOCK_SIZE 8192

void
soup_message_io_cleanup (SoupMessage *msg)
{
	SoupMessagePrivate *priv = SOUP_MESSAGE_GET_PRIVATE (msg);
	SoupMessageIOData *io;

	soup_message_io_stop (msg);

	io = priv->io_data;
	if (!io)
		return;
	priv->io_data = NULL;

	if (io->iostream)
		g_object_unref (io->iostream);
	if (io->body_istream)
		g_object_unref (io->body_istream);
	if (io->body_ostream)
		g_object_unref (io->body_ostream);
	if (io->async_context)
		g_main_context_unref (io->async_context);
	if (io->item)
		soup_message_queue_item_unref (io->item);

	g_byte_array_free (io->read_header_buf, TRUE);

	g_string_free (io->write_buf, TRUE);
	if (io->write_chunk)
		soup_buffer_free (io->write_chunk);

	g_slice_free (SoupMessageIOData, io);
}

void
soup_message_io_stop (SoupMessage *msg)
{
	SoupMessagePrivate *priv = SOUP_MESSAGE_GET_PRIVATE (msg);
	SoupMessageIOData *io = priv->io_data;

	if (!io)
		return;

	if (io->io_source) {
		g_source_destroy (io->io_source);
		g_source_unref (io->io_source);
		io->io_source = NULL;
	}

	if (io->unpause_source) {
		g_source_destroy (io->unpause_source);
		g_source_unref (io->unpause_source);
		io->unpause_source = NULL;
	}

	if (io->mode == SOUP_MESSAGE_IO_SERVER) {
		if (io->write_state < SOUP_MESSAGE_IO_STATE_FINISHING)
			g_io_stream_close (io->iostream, NULL, NULL);
	}
}

void
soup_message_io_finished (SoupMessage *msg)
{
	SoupMessagePrivate *priv = SOUP_MESSAGE_GET_PRIVATE (msg);
	SoupMessageIOData *io = priv->io_data;
	SoupMessageCompletionFn completion_cb = io->completion_cb;
	gpointer completion_data = io->completion_data;

	g_object_ref (msg);
	soup_message_io_cleanup (msg);
	if (completion_cb)
		completion_cb (msg, completion_data);
	g_object_unref (msg);
}

static gboolean
read_headers (SoupMessage *msg, GCancellable *cancellable, GError **error)
{
	SoupMessagePrivate *priv = SOUP_MESSAGE_GET_PRIVATE (msg);
	SoupMessageIOData *io = priv->io_data;
	gssize nread, old_len;
	gboolean got_lf;

	while (1) {
		old_len = io->read_header_buf->len;
		g_byte_array_set_size (io->read_header_buf, old_len + RESPONSE_BLOCK_SIZE);
		nread = soup_filter_input_stream_read_line (io->istream,
							    io->read_header_buf->data + old_len,
							    RESPONSE_BLOCK_SIZE,
							    io->blocking,
							    &got_lf,
							    cancellable, error);
		io->read_header_buf->len = old_len + MAX (nread, 0);
		if (nread == 0) {
			soup_message_set_status (msg, SOUP_STATUS_MALFORMED);
			g_set_error_literal (error, G_IO_ERROR,
					     G_IO_ERROR_PARTIAL_INPUT,
					     _("Connection terminated unexpectedly"));
		}
		if (nread <= 0)
			return FALSE;

		if (got_lf) {
			if (nread == 1 && old_len >= 2 &&
			    !strncmp ((char *)io->read_header_buf->data +
				      io->read_header_buf->len - 2,
				      "\n\n", 2))
				break;
			else if (nread == 2 && old_len >= 3 &&
				 !strncmp ((char *)io->read_header_buf->data +
					   io->read_header_buf->len - 3,
					   "\n\r\n", 3))
				break;
		}
	}

	/* We need to "rewind" io->read_header_buf back one line.
	 * That SHOULD be two characters (CR LF), but if the
	 * web server was stupid, it might only be one.
	 */
	if (io->read_header_buf->len < 3 ||
	    io->read_header_buf->data[io->read_header_buf->len - 2] == '\n')
		io->read_header_buf->len--;
	else
		io->read_header_buf->len -= 2;
	io->read_header_buf->data[io->read_header_buf->len] = '\0';

	return TRUE;
}

static gint
processing_stage_cmp (gconstpointer a,
		    gconstpointer b)
{
	SoupProcessingStage stage_a = soup_content_processor_get_processing_stage (SOUP_CONTENT_PROCESSOR (a));
	SoupProcessingStage stage_b = soup_content_processor_get_processing_stage (SOUP_CONTENT_PROCESSOR (b));

	if (stage_a > stage_b)
		return 1;
	if (stage_a == stage_b)
		return 0;
	return -1;
}

GInputStream *
soup_message_setup_body_istream (GInputStream *body_stream,
				 SoupMessage *msg,
				 SoupSession *session,
				 SoupProcessingStage start_at_stage)
{
	GInputStream *istream;
	GSList *p, *processors;

	istream = g_object_ref (body_stream);

	processors = soup_session_get_features (session, SOUP_TYPE_CONTENT_PROCESSOR);
	processors = g_slist_sort (processors, processing_stage_cmp);

	for (p = processors; p; p = p->next) {
		GInputStream *wrapper;
		SoupContentProcessor *processor;

		processor = SOUP_CONTENT_PROCESSOR (p->data);
		if (soup_message_disables_feature (msg, p->data) ||
		    soup_content_processor_get_processing_stage (processor) < start_at_stage)
			continue;

		wrapper = soup_content_processor_wrap_input (processor, istream, msg, NULL);
		if (wrapper) {
			g_object_unref (istream);
			istream = wrapper;
		}
	}

	g_slist_free (processors);

	return istream;
}

/*
 * There are two request/response formats: the basic request/response,
 * possibly with one or more unsolicited informational responses (such
 * as the WebDAV "102 Processing" response):
 *
 *     Client                            Server
 *      W:HEADERS  / R:NOT_STARTED    ->  R:HEADERS  / W:NOT_STARTED
 *      W:BODY     / R:NOT_STARTED    ->  R:BODY     / W:NOT_STARTED
 *     [W:DONE     / R:HEADERS (1xx)  <-  R:DONE     / W:HEADERS (1xx) ...]
 *      W:DONE     / R:HEADERS        <-  R:DONE     / W:HEADERS
 *      W:DONE     / R:BODY           <-  R:DONE     / W:BODY
 *      W:DONE     / R:DONE               R:DONE     / W:DONE
 *     
 * and the "Expect: 100-continue" request/response, with the client
 * blocking halfway through its request, and then either continuing or
 * aborting, depending on the server response:
 *
 *     Client                            Server
 *      W:HEADERS  / R:NOT_STARTED    ->  R:HEADERS  / W:NOT_STARTED
 *      W:BLOCKING / R:HEADERS        <-  R:BLOCKING / W:HEADERS
 *     [W:BODY     / R:BLOCKING       ->  R:BODY     / W:BLOCKING]
 *     [W:DONE     / R:HEADERS        <-  R:DONE     / W:HEADERS]
 *      W:DONE     / R:BODY           <-  R:DONE     / W:BODY
 *      W:DONE     / R:DONE               R:DONE     / W:DONE
 */

/* Attempts to push forward the writing side of @msg's I/O. Returns
 * %TRUE if it manages to make some progress, and it is likely that
 * further progress can be made. Returns %FALSE if it has reached a
 * stopping point of some sort (need input from the application,
 * socket not writable, write is complete, etc).
 */
static gboolean
io_write (SoupMessage *msg, GCancellable *cancellable, GError **error)
{
	SoupMessagePrivate *priv = SOUP_MESSAGE_GET_PRIVATE (msg);
	SoupMessageIOData *io = priv->io_data;
	SoupBuffer *chunk;
	gssize nwrote;

	switch (io->write_state) {
	case SOUP_MESSAGE_IO_STATE_HEADERS:
		if (!io->write_buf->len) {
			io->get_headers_cb (msg, io->write_buf,
					    &io->write_encoding,
					    io->header_data);
		}

		while (io->written < io->write_buf->len) {
			nwrote = g_pollable_stream_write (io->ostream,
							  io->write_buf->str + io->written,
							  io->write_buf->len - io->written,
							  io->blocking,
							  cancellable, error);
			if (nwrote == -1)
				return FALSE;
			io->written += nwrote;
		}

		io->written = 0;
		g_string_truncate (io->write_buf, 0);

		if (io->mode == SOUP_MESSAGE_IO_SERVER &&
		    SOUP_STATUS_IS_INFORMATIONAL (msg->status_code)) {
			if (msg->status_code == SOUP_STATUS_CONTINUE) {
				/* Stop and wait for the body now */
				io->write_state =
					SOUP_MESSAGE_IO_STATE_BLOCKING;
				io->read_state = SOUP_MESSAGE_IO_STATE_BODY_START;
			} else {
				/* We just wrote a 1xx response
				 * header, so stay in STATE_HEADERS.
				 * (The caller will pause us from the
				 * wrote_informational callback if he
				 * is not ready to send the final
				 * response.)
				 */
			}

			soup_message_wrote_informational (msg);
			soup_message_cleanup_response (msg);
			break;
		}

		if (io->write_encoding == SOUP_ENCODING_CONTENT_LENGTH) {
			SoupMessageHeaders *hdrs =
				(io->mode == SOUP_MESSAGE_IO_CLIENT) ?
				msg->request_headers : msg->response_headers;
			io->write_length = soup_message_headers_get_content_length (hdrs);
		}

		if (io->mode == SOUP_MESSAGE_IO_CLIENT &&
		    soup_message_headers_get_expectations (msg->request_headers) & SOUP_EXPECTATION_CONTINUE) {
			/* Need to wait for the Continue response */
			io->write_state = SOUP_MESSAGE_IO_STATE_BLOCKING;
			io->read_state = SOUP_MESSAGE_IO_STATE_HEADERS;
		} else {
			io->write_state = SOUP_MESSAGE_IO_STATE_BODY_START;

			/* If the client was waiting for a Continue
			 * but we sent something else, then they're
			 * now done writing.
			 */
			if (io->mode == SOUP_MESSAGE_IO_SERVER &&
			    io->read_state == SOUP_MESSAGE_IO_STATE_BLOCKING)
				io->read_state = SOUP_MESSAGE_IO_STATE_DONE;
		}

		soup_message_wrote_headers (msg);
		break;


	case SOUP_MESSAGE_IO_STATE_BODY_START:
		io->body_ostream = soup_body_output_stream_new (io->ostream,
								io->write_encoding,
								io->write_length);
		io->write_state = SOUP_MESSAGE_IO_STATE_BODY;
		break;


	case SOUP_MESSAGE_IO_STATE_BODY:
		if (!io->write_length &&
		    io->write_encoding != SOUP_ENCODING_EOF &&
		    io->write_encoding != SOUP_ENCODING_CHUNKED) {
			io->write_state = SOUP_MESSAGE_IO_STATE_BODY_DONE;
			break;
		}

		if (!io->write_chunk) {
			io->write_chunk = soup_message_body_get_chunk (io->write_body, io->write_body_offset);
			if (!io->write_chunk) {
				g_return_val_if_fail (!io->item || !io->item->new_api, FALSE);
				soup_message_io_pause (msg);
				return FALSE;
			}
			if (!io->write_chunk->length) {
				io->write_state = SOUP_MESSAGE_IO_STATE_BODY_DONE;
				break;
			}
		}

		nwrote = g_pollable_stream_write (io->body_ostream,
						  io->write_chunk->data + io->written,
						  io->write_chunk->length - io->written,
						  io->blocking,
						  cancellable, error);
		if (nwrote == -1)
			return FALSE;

		chunk = soup_buffer_new_subbuffer (io->write_chunk,
						   io->written, nwrote);
		io->written += nwrote;
		if (io->write_length)
			io->write_length -= nwrote;

		if (io->written == io->write_chunk->length)
			io->write_state = SOUP_MESSAGE_IO_STATE_BODY_DATA;

		soup_message_wrote_body_data (msg, chunk);
		soup_buffer_free (chunk);
		break;


	case SOUP_MESSAGE_IO_STATE_BODY_DATA:
		io->written = 0;
		if (io->write_chunk->length == 0) {
			io->write_state = SOUP_MESSAGE_IO_STATE_BODY_DONE;
			break;
		}

		if (io->mode == SOUP_MESSAGE_IO_SERVER ||
		    priv->msg_flags & SOUP_MESSAGE_CAN_REBUILD)
			soup_message_body_wrote_chunk (io->write_body, io->write_chunk);
		io->write_body_offset += io->write_chunk->length;
		soup_buffer_free (io->write_chunk);
		io->write_chunk = NULL;

		io->write_state = SOUP_MESSAGE_IO_STATE_BODY;
		soup_message_wrote_chunk (msg);
		break;


	case SOUP_MESSAGE_IO_STATE_BODY_DONE:
		if (io->body_ostream) {
			if (!g_output_stream_close (io->body_ostream, cancellable, error))
				return FALSE;
			g_clear_object (&io->body_ostream);
		}

		io->write_state = SOUP_MESSAGE_IO_STATE_FINISHING;
		soup_message_wrote_body (msg);
		break;


	case SOUP_MESSAGE_IO_STATE_FINISHING:
		io->write_state = SOUP_MESSAGE_IO_STATE_DONE;

		if (io->mode == SOUP_MESSAGE_IO_CLIENT)
			io->read_state = SOUP_MESSAGE_IO_STATE_HEADERS;
		break;


	default:
		g_return_val_if_reached (FALSE);
	}

	return TRUE;
}

/* Attempts to push forward the reading side of @msg's I/O. Returns
 * %TRUE if it manages to make some progress, and it is likely that
 * further progress can be made. Returns %FALSE if it has reached a
 * stopping point of some sort (need input from the application,
 * socket not readable, read is complete, etc).
 */
static gboolean
io_read (SoupMessage *msg, GCancellable *cancellable, GError **error)
{
	SoupMessagePrivate *priv = SOUP_MESSAGE_GET_PRIVATE (msg);
	SoupMessageIOData *io = priv->io_data;
	guchar *stack_buf = NULL;
	gssize nread;
	SoupBuffer *buffer;
	guint status;

	switch (io->read_state) {
	case SOUP_MESSAGE_IO_STATE_HEADERS:
		if (!read_headers (msg, cancellable, error))
			return FALSE;

		status = io->parse_headers_cb (msg, (char *)io->read_header_buf->data,
					       io->read_header_buf->len,
					       &io->read_encoding,
					       io->header_data);
		g_byte_array_set_size (io->read_header_buf, 0);

		if (status != SOUP_STATUS_OK) {
			/* Either we couldn't parse the headers, or they
			 * indicated something that would mean we wouldn't
			 * be able to parse the body. (Eg, unknown
			 * Transfer-Encoding.). Skip the rest of the
			 * reading, and make sure the connection gets
			 * closed when we're done.
			 */
			soup_message_set_status (msg, status);
			soup_message_headers_append (msg->request_headers,
						     "Connection", "close");
			io->read_state = SOUP_MESSAGE_IO_STATE_FINISHING;
			break;
		}

		if (io->mode == SOUP_MESSAGE_IO_CLIENT &&
		    SOUP_STATUS_IS_INFORMATIONAL (msg->status_code)) {
			if (msg->status_code == SOUP_STATUS_CONTINUE &&
			    io->write_state == SOUP_MESSAGE_IO_STATE_BLOCKING) {
				/* Pause the reader, unpause the writer */
				io->read_state =
					SOUP_MESSAGE_IO_STATE_BLOCKING;
				io->write_state =
					SOUP_MESSAGE_IO_STATE_BODY_START;
			} else {
				/* Just stay in HEADERS */
				io->read_state = SOUP_MESSAGE_IO_STATE_HEADERS;
			}

			/* Informational responses have no bodies, so
			 * bail out here rather than parsing encoding, etc
			 */
			soup_message_got_informational (msg);
			soup_message_cleanup_response (msg);
			break;
		} else if (io->mode == SOUP_MESSAGE_IO_SERVER &&
			   soup_message_headers_get_expectations (msg->request_headers) & SOUP_EXPECTATION_CONTINUE) {
			/* The client requested a Continue response. The
			 * got_headers handler may change this to something
			 * else though.
			 */
			soup_message_set_status (msg, SOUP_STATUS_CONTINUE);
			io->write_state = SOUP_MESSAGE_IO_STATE_HEADERS;
			io->read_state = SOUP_MESSAGE_IO_STATE_BLOCKING;
		} else {
			io->read_state = SOUP_MESSAGE_IO_STATE_BODY_START;

			/* If the client was waiting for a Continue
			 * but got something else, then it's done
			 * writing.
			 */
			if (io->mode == SOUP_MESSAGE_IO_CLIENT &&
			    io->write_state == SOUP_MESSAGE_IO_STATE_BLOCKING)
				io->write_state = SOUP_MESSAGE_IO_STATE_FINISHING;
		}

		if (io->read_encoding == SOUP_ENCODING_CONTENT_LENGTH) {
			SoupMessageHeaders *hdrs =
				(io->mode == SOUP_MESSAGE_IO_CLIENT) ?
				msg->response_headers : msg->request_headers;
			io->read_length = soup_message_headers_get_content_length (hdrs);

			if (io->mode == SOUP_MESSAGE_IO_CLIENT &&
			    !soup_message_is_keepalive (msg)) {
				/* Some servers suck and send
				 * incorrect Content-Length values, so
				 * allow EOF termination in this case
				 * (iff the message is too short) too.
				 */
				io->read_encoding = SOUP_ENCODING_EOF;
			}
		} else
			io->read_length = -1;

		soup_message_got_headers (msg);
		break;


	case SOUP_MESSAGE_IO_STATE_BODY_START:
		if (!io->body_istream) {
			GInputStream *body_istream = soup_body_input_stream_new (G_INPUT_STREAM (io->istream),
										 io->read_encoding,
										 io->read_length);

			/* TODO: server-side messages do not have a io->item. This means
			 * that we cannot use content processors for them right now.
			 */
			if (io->mode == SOUP_MESSAGE_IO_CLIENT) {
				io->body_istream = soup_message_setup_body_istream (body_istream, msg,
										    io->item->session,
										    SOUP_STAGE_MESSAGE_BODY);
				g_object_unref (body_istream);
			} else {
				io->body_istream = body_istream;
			}
		}

		if (priv->sniffer) {
			SoupContentSnifferStream *sniffer_stream = SOUP_CONTENT_SNIFFER_STREAM (io->body_istream);
			const char *content_type;
			GHashTable *params;

			if (!soup_content_sniffer_stream_is_ready (sniffer_stream, io->blocking, cancellable, error))
				return FALSE;

			content_type = soup_content_sniffer_stream_sniff (sniffer_stream, &params);
			soup_message_content_sniffed (msg, content_type, params);
		}

		io->read_state = SOUP_MESSAGE_IO_STATE_BODY;
		break;


	case SOUP_MESSAGE_IO_STATE_BODY:
		if (priv->chunk_allocator) {
			buffer = priv->chunk_allocator (msg, io->read_length, priv->chunk_allocator_data);
			if (!buffer) {
				g_return_val_if_fail (!io->item || !io->item->new_api, FALSE);
				soup_message_io_pause (msg);
				return FALSE;
			}
		} else {
			if (!stack_buf)
				stack_buf = alloca (RESPONSE_BLOCK_SIZE);
			buffer = soup_buffer_new (SOUP_MEMORY_TEMPORARY,
						  stack_buf,
						  RESPONSE_BLOCK_SIZE);
		}

		nread = g_pollable_stream_read (io->body_istream,
						(guchar *)buffer->data,
						buffer->length,
						io->blocking,
						cancellable, error);
		if (nread > 0) {
			buffer->length = nread;
			soup_message_body_got_chunk (io->read_body, buffer);
			soup_message_got_chunk (msg, buffer);
			soup_buffer_free (buffer);
			break;
		}

		soup_buffer_free (buffer);
		if (nread == -1)
			return FALSE;

		/* else nread == 0 */
		io->read_state = SOUP_MESSAGE_IO_STATE_BODY_DONE;
		break;


	case SOUP_MESSAGE_IO_STATE_BODY_DONE:
		io->read_state = SOUP_MESSAGE_IO_STATE_FINISHING;
		soup_message_got_body (msg);
		break;


	case SOUP_MESSAGE_IO_STATE_FINISHING:
		io->read_state = SOUP_MESSAGE_IO_STATE_DONE;

		if (io->mode == SOUP_MESSAGE_IO_SERVER)
			io->write_state = SOUP_MESSAGE_IO_STATE_HEADERS;
		break;


	default:
		g_return_val_if_reached (FALSE);
	}

	return TRUE;
}

typedef struct {
	GSource source;
	SoupMessage *msg;
	gboolean paused;
} SoupMessageSource;

static gboolean
message_source_check (GSource *source)
{
	SoupMessageSource *message_source = (SoupMessageSource *)source;

	if (message_source->paused) {
		SoupMessagePrivate *priv = SOUP_MESSAGE_GET_PRIVATE (message_source->msg);
		SoupMessageIOData *io = priv->io_data;

		if (!io || io->paused)
			return FALSE;
		else
			return TRUE;
	} else
		return FALSE;
}

static gboolean
message_source_prepare (GSource *source,
			gint    *timeout)
{
	*timeout = -1;
	return message_source_check (source);
}

static gboolean
message_source_dispatch (GSource     *source,
			 GSourceFunc  callback,
			 gpointer     user_data)
{
	SoupMessageSourceFunc func = (SoupMessageSourceFunc)callback;
	SoupMessageSource *message_source = (SoupMessageSource *)source;

	return (*func) (message_source->msg, user_data);
}

static void
message_source_finalize (GSource *source)
{
	SoupMessageSource *message_source = (SoupMessageSource *)source;

	g_object_unref (message_source->msg);
}

static gboolean
message_source_closure_callback (SoupMessage *msg,
				 gpointer     data)
{
	GClosure *closure = data;
	GValue param = G_VALUE_INIT;
	GValue result_value = G_VALUE_INIT;
	gboolean result;

	g_value_init (&result_value, G_TYPE_BOOLEAN);

	g_value_init (&param, SOUP_TYPE_MESSAGE);
	g_value_set_object (&param, msg);

	g_closure_invoke (closure, &result_value, 1, &param, NULL);

	result = g_value_get_boolean (&result_value);
	g_value_unset (&result_value);
	g_value_unset (&param);

	return result;
}

static GSourceFuncs message_source_funcs =
{
	message_source_prepare,
	message_source_check,
	message_source_dispatch,
	message_source_finalize,
	(GSourceFunc)message_source_closure_callback,
	(GSourceDummyMarshal)g_cclosure_marshal_generic,
};

GSource *
soup_message_io_get_source (SoupMessage *msg, GCancellable *cancellable,
			    SoupMessageSourceFunc callback, gpointer user_data)
{
	SoupMessagePrivate *priv = SOUP_MESSAGE_GET_PRIVATE (msg);
	SoupMessageIOData *io = priv->io_data;
	GSource *base_source, *source;
	SoupMessageSource *message_source;

	if (!io) {
		base_source = g_timeout_source_new (0);
	} else if (io->paused) {
		base_source = NULL;
	} else if (SOUP_MESSAGE_IO_STATE_POLLABLE (io->read_state)) {
		GPollableInputStream *istream;

		if (io->body_istream)
			istream = G_POLLABLE_INPUT_STREAM (io->body_istream);
		else
			istream = G_POLLABLE_INPUT_STREAM (io->istream);
		base_source = g_pollable_input_stream_create_source (istream, cancellable);
	} else if (SOUP_MESSAGE_IO_STATE_POLLABLE (io->write_state)) {
		GPollableOutputStream *ostream;

		if (io->body_ostream)
			ostream = G_POLLABLE_OUTPUT_STREAM (io->body_ostream);
		else
			ostream = G_POLLABLE_OUTPUT_STREAM (io->ostream);
		base_source = g_pollable_output_stream_create_source (ostream, cancellable);
	} else
		base_source = g_timeout_source_new (0);

	source = g_source_new (&message_source_funcs,
			       sizeof (SoupMessageSource));
	g_source_set_name (source, "SoupMessageSource");
	message_source = (SoupMessageSource *)source;
	message_source->msg = g_object_ref (msg);
	message_source->paused = io && io->paused;

	if (base_source) {
		g_source_set_dummy_callback (base_source);
		g_source_add_child_source (source, base_source);
		g_source_unref (base_source);
	}
	g_source_set_callback (source, (GSourceFunc) callback, user_data, NULL);
	return source;
}

static gboolean
request_is_restartable (SoupMessage *msg, GError *error)
{
	SoupMessagePrivate *priv = SOUP_MESSAGE_GET_PRIVATE (msg);
	SoupMessageIOData *io = priv->io_data;

	if (!io)
		return FALSE;

	return (io->mode == SOUP_MESSAGE_IO_CLIENT &&
		io->read_state <= SOUP_MESSAGE_IO_STATE_HEADERS &&
		io->read_header_buf->len == 0 &&
		soup_connection_get_ever_used (io->item->conn) &&
		!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_TIMED_OUT) &&
		!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK) &&
		error->domain != G_TLS_ERROR &&
		SOUP_METHOD_IS_IDEMPOTENT (msg->method));
}

static gboolean
io_run_until (SoupMessage *msg,
	      SoupMessageIOState read_state, SoupMessageIOState write_state,
	      GCancellable *cancellable, GError **error)
{
	SoupMessagePrivate *priv = SOUP_MESSAGE_GET_PRIVATE (msg);
	SoupMessageIOData *io = priv->io_data;
	gboolean progress = TRUE, done;
	GError *my_error = NULL;

	if (g_cancellable_set_error_if_cancelled (cancellable, error))
		return FALSE;
	else if (!io) {
		g_set_error_literal (error, G_IO_ERROR,
				     G_IO_ERROR_CANCELLED,
				     _("Operation was cancelled"));
		return FALSE;
	}

	g_object_ref (msg);

	while (progress && priv->io_data == io && !io->paused &&
	       (io->read_state < read_state || io->write_state < write_state)) {

		if (SOUP_MESSAGE_IO_STATE_ACTIVE (io->read_state))
			progress = io_read (msg, cancellable, &my_error);
		else if (SOUP_MESSAGE_IO_STATE_ACTIVE (io->write_state))
			progress = io_write (msg, cancellable, &my_error);
		else
			progress = FALSE;
	}

	if (my_error) {
		if (request_is_restartable (msg, my_error)) {
			/* Connection got closed, but we can safely try again */
			g_error_free (my_error);
			g_set_error_literal (error, SOUP_HTTP_ERROR,
					     SOUP_STATUS_TRY_AGAIN, "");
			g_object_unref (msg);
			return FALSE;
		}

		g_propagate_error (error, my_error);
		g_object_unref (msg);
		return FALSE;
	} else if (g_cancellable_set_error_if_cancelled (cancellable, error)) {
		g_object_unref (msg);
		return FALSE;
	} else if (priv->io_data != io) {
		g_set_error_literal (error, G_IO_ERROR,
				     G_IO_ERROR_CANCELLED,
				     _("Operation was cancelled"));
		g_object_unref (msg);
		return FALSE;
	}

	done = (io->read_state >= read_state &&
		io->write_state >= write_state);

	if (io->paused && !done) {
		g_set_error_literal (error, G_IO_ERROR,
				     G_IO_ERROR_WOULD_BLOCK,
				     _("Operation would block"));
		g_object_unref (msg);
		return FALSE;
	}

	g_object_unref (msg);
	return done;
}

static gboolean
io_run (SoupMessage *msg, gpointer user_data)
{
	SoupMessagePrivate *priv = SOUP_MESSAGE_GET_PRIVATE (msg);
	SoupMessageIOData *io = priv->io_data;
	GError *error = NULL;
	GCancellable *cancellable;

	if (io->io_source) {
		g_source_destroy (io->io_source);
		g_source_unref (io->io_source);
		io->io_source = NULL;
	}

	g_object_ref (msg);
	cancellable = io->cancellable ? g_object_ref (io->cancellable) : NULL;

	if (io_run_until (msg,
			  SOUP_MESSAGE_IO_STATE_DONE,
			  SOUP_MESSAGE_IO_STATE_DONE,
			  cancellable, &error)) {
		soup_message_io_finished (msg);
	} else if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK)) {
		g_clear_error (&error);
		io->io_source = soup_message_io_get_source (msg, NULL, io_run, msg);
		g_source_attach (io->io_source, io->async_context);
	} else if (error && priv->io_data == io) {
		if (g_error_matches (error, SOUP_HTTP_ERROR, SOUP_STATUS_TRY_AGAIN))
			io->item->state = SOUP_MESSAGE_RESTARTING;
		else if (error->domain == G_TLS_ERROR) {
			soup_message_set_status_full (msg,
						      SOUP_STATUS_SSL_FAILED,
						      error->message);
		} else if (!SOUP_STATUS_IS_TRANSPORT_ERROR (msg->status_code))
			soup_message_set_status (msg, SOUP_STATUS_IO_ERROR);

		g_error_free (error);
		soup_message_io_finished (msg);
	} else if (error)
		g_error_free (error);

	g_object_unref (msg);
	g_clear_object (&cancellable);

	return FALSE;
}

gboolean
soup_message_io_run_until_write (SoupMessage *msg,
				 GCancellable *cancellable, GError **error)
{
	return io_run_until (msg,
			     SOUP_MESSAGE_IO_STATE_ANY,
			     SOUP_MESSAGE_IO_STATE_BODY,
			     cancellable, error);
}

gboolean
soup_message_io_run_until_read (SoupMessage *msg,
				GCancellable *cancellable, GError **error)
{
	return io_run_until (msg,
			     SOUP_MESSAGE_IO_STATE_BODY,
			     SOUP_MESSAGE_IO_STATE_ANY,
			     cancellable, error);
}

gboolean
soup_message_io_run_until_finish (SoupMessage   *msg,
				  GCancellable  *cancellable,
				  GError       **error)
{
	g_object_ref (msg);

	if (!io_run_until (msg,
			   SOUP_MESSAGE_IO_STATE_DONE,
			   SOUP_MESSAGE_IO_STATE_DONE,
			   cancellable, error)) {
		g_object_unref (msg);
		return FALSE;
	}

	soup_message_io_finished (msg);
	g_object_unref (msg);
	return TRUE;
}

static void
client_stream_eof (SoupClientInputStream *stream, gpointer user_data)
{
	SoupMessage *msg = user_data;
	SoupMessagePrivate *priv = SOUP_MESSAGE_GET_PRIVATE (msg);
	SoupMessageIOData *io = priv->io_data;

	if (io && io->read_state == SOUP_MESSAGE_IO_STATE_BODY)
		io->read_state = SOUP_MESSAGE_IO_STATE_BODY_DONE;
}

GInputStream *
soup_message_io_get_response_istream (SoupMessage  *msg,
				      GError      **error)
{
	SoupMessagePrivate *priv = SOUP_MESSAGE_GET_PRIVATE (msg);
	SoupMessageIOData *io = priv->io_data;
	GInputStream *client_stream;

	g_return_val_if_fail (io->mode == SOUP_MESSAGE_IO_CLIENT, NULL);

	if (SOUP_STATUS_IS_TRANSPORT_ERROR (msg->status_code)) {
		g_set_error_literal (error, SOUP_HTTP_ERROR,
				     msg->status_code, msg->reason_phrase);
		return NULL;
	}

	client_stream = soup_client_input_stream_new (io->body_istream, msg);
	g_signal_connect (client_stream, "eof",
			  G_CALLBACK (client_stream_eof), msg);

	return client_stream;
}


static SoupMessageIOData *
new_iostate (SoupMessage *msg, GIOStream *iostream,
	     GMainContext *async_context, SoupMessageIOMode mode,
	     SoupMessageGetHeadersFn get_headers_cb,
	     SoupMessageParseHeadersFn parse_headers_cb,
	     gpointer header_data,
	     SoupMessageCompletionFn completion_cb,
	     gpointer completion_data)
{
	SoupMessagePrivate *priv = SOUP_MESSAGE_GET_PRIVATE (msg);
	SoupMessageIOData *io;

	io = g_slice_new0 (SoupMessageIOData);
	io->mode = mode;
	io->get_headers_cb   = get_headers_cb;
	io->parse_headers_cb = parse_headers_cb;
	io->header_data      = header_data;
	io->completion_cb    = completion_cb;
	io->completion_data  = completion_data;

	io->iostream = g_object_ref (iostream);
	io->istream = SOUP_FILTER_INPUT_STREAM (g_io_stream_get_input_stream (iostream));
	io->ostream = g_io_stream_get_output_stream (iostream);

	if (async_context) {
		io->async_context = g_main_context_ref (async_context);
		io->blocking = FALSE;
	} else
		io->blocking = TRUE;

	io->read_header_buf = g_byte_array_new ();
	io->write_buf       = g_string_new (NULL);

	io->read_state  = SOUP_MESSAGE_IO_STATE_NOT_STARTED;
	io->write_state = SOUP_MESSAGE_IO_STATE_NOT_STARTED;

	if (priv->io_data)
		soup_message_io_cleanup (msg);
	priv->io_data = io;
	return io;
}

void
soup_message_io_client (SoupMessageQueueItem *item,
			GIOStream *iostream,
			GMainContext *async_context,
			SoupMessageGetHeadersFn get_headers_cb,
			SoupMessageParseHeadersFn parse_headers_cb,
			gpointer header_data,
			SoupMessageCompletionFn completion_cb,
			gpointer completion_data)
{
	SoupMessageIOData *io;

	io = new_iostate (item->msg, iostream, async_context,
			  SOUP_MESSAGE_IO_CLIENT,
			  get_headers_cb, parse_headers_cb, header_data,
			  completion_cb, completion_data);

	io->item = item;
	soup_message_queue_item_ref (item);
	io->cancellable = item->cancellable;

	io->read_body       = item->msg->response_body;
	io->write_body      = item->msg->request_body;

	io->write_state     = SOUP_MESSAGE_IO_STATE_HEADERS;
	if (!item->new_api)
		io_run (item->msg, NULL);
}

void
soup_message_io_server (SoupMessage *msg,
			GIOStream *iostream, GMainContext *async_context,
			SoupMessageGetHeadersFn get_headers_cb,
			SoupMessageParseHeadersFn parse_headers_cb,
			gpointer header_data,
			SoupMessageCompletionFn completion_cb,
			gpointer completion_data)
{
	SoupMessageIOData *io;

	io = new_iostate (msg, iostream, async_context,
			  SOUP_MESSAGE_IO_SERVER,
			  get_headers_cb, parse_headers_cb, header_data,
			  completion_cb, completion_data);

	io->read_body       = msg->request_body;
	io->write_body      = msg->response_body;

	io->read_state      = SOUP_MESSAGE_IO_STATE_HEADERS;
	io_run (msg, NULL);
}

void  
soup_message_io_pause (SoupMessage *msg)
{
	SoupMessagePrivate *priv = SOUP_MESSAGE_GET_PRIVATE (msg);
	SoupMessageIOData *io = priv->io_data;

	g_return_if_fail (io != NULL);

	if (io->item && io->item->new_api)
		g_return_if_fail (io->read_state < SOUP_MESSAGE_IO_STATE_BODY);

	if (io->io_source) {
		g_source_destroy (io->io_source);
		g_source_unref (io->io_source);
		io->io_source = NULL;
	}

	if (io->unpause_source) {
		g_source_destroy (io->unpause_source);
		io->unpause_source = NULL;
	}

	io->paused = TRUE;
}

static gboolean
io_unpause_internal (gpointer msg)
{
	SoupMessagePrivate *priv = SOUP_MESSAGE_GET_PRIVATE (msg);
	SoupMessageIOData *io = priv->io_data;

	g_return_val_if_fail (io != NULL, FALSE);
	io->unpause_source = NULL;
	io->paused = FALSE;

	if (io->io_source)
		return FALSE;

	io_run (msg, NULL);
	return FALSE;
}

void
soup_message_io_unpause (SoupMessage *msg)
{
	SoupMessagePrivate *priv = SOUP_MESSAGE_GET_PRIVATE (msg);
	SoupMessageIOData *io = priv->io_data;

	g_return_if_fail (io != NULL);

	if (io->item && io->item->new_api) {
		g_return_if_fail (io->read_state < SOUP_MESSAGE_IO_STATE_BODY);
		io->paused = FALSE;
		return;
	}

	if (!io->blocking) {
		if (!io->unpause_source) {
			io->unpause_source = soup_add_completion (
				io->async_context, io_unpause_internal, msg);
		}
	} else
		io_unpause_internal (msg);
}

/**
 * soup_message_io_in_progress:
 * @msg: a #SoupMessage
 *
 * Tests whether or not I/O is currently in progress on @msg.
 *
 * Return value: whether or not I/O is currently in progress.
 **/
gboolean
soup_message_io_in_progress (SoupMessage *msg)
{
	SoupMessagePrivate *priv = SOUP_MESSAGE_GET_PRIVATE (msg);

	return priv->io_data != NULL;
}
