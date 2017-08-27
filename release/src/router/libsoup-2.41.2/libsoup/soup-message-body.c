/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-message-body.c: SoupMessage request/response bodies
 *
 * Copyright (C) 2000-2003, Ximian, Inc.
 */

#include <string.h>

#include "soup-message-body.h"
#include "soup.h"

/**
 * SECTION:soup-message-body
 * @short_description: HTTP message body
 * @see_also: #SoupMessage
 *
 * #SoupMessageBody represents the request or response body of a
 * #SoupMessage.
 *
 * In addition to #SoupMessageBody, libsoup also defines a "smaller"
 * data buffer type, #SoupBuffer, which is primarily used as a
 * component of #SoupMessageBody. In particular, when using chunked
 * encoding to transmit or receive a message, each chunk is
 * represented as a #SoupBuffer.
 **/

/**
 * SoupMemoryUse:
 * @SOUP_MEMORY_STATIC: The memory is statically allocated and
 * constant; libsoup can use the passed-in buffer directly and not
 * need to worry about it being modified or freed.
 * @SOUP_MEMORY_TAKE: The caller has allocated the memory for the
 * #SoupBuffer's use; libsoup will assume ownership of it and free it
 * (with g_free()) when it is done with it.
 * @SOUP_MEMORY_COPY: The passed-in data belongs to the caller; the
 * #SoupBuffer will copy it into new memory, leaving the caller free
 * to reuse the original memory.
 * @SOUP_MEMORY_TEMPORARY: The passed-in data belongs to the caller,
 * but will remain valid for the lifetime of the #SoupBuffer. The
 * difference between this and @SOUP_MEMORY_STATIC is that if you copy
 * a @SOUP_MEMORY_TEMPORARY buffer, it will make a copy of the memory
 * as well, rather than reusing the original memory.
 *
 * Describes how #SoupBuffer should use the data passed in by the
 * caller.
 *
 * See also soup_buffer_new_with_owner(), which allows to you create a
 * buffer containing data which is owned by another object.
 **/

/* Internal SoupMemoryUse values */
enum {
	SOUP_MEMORY_SUBBUFFER = SOUP_MEMORY_TEMPORARY + 1,
	SOUP_MEMORY_OWNED
};

/**
 * SoupBuffer:
 * @data: (type gpointer): the data
 * @length: length of @data
 *
 * A data buffer, generally used to represent a chunk of a
 * #SoupMessageBody.
 *
 * @data is a #char because that's generally convenient; in some
 * situations you may need to cast it to #guchar or another type.
 **/

typedef struct {
	SoupBuffer     buffer;
	SoupMemoryUse  use;
	guint          refcount;

	gpointer       owner;
	GDestroyNotify owner_dnotify;
} SoupBufferPrivate;

/**
 * soup_buffer_new:
 * @use: how @data is to be used by the buffer
 * @data: data
 * @length: length of @data
 *
 * Creates a new #SoupBuffer containing @length bytes from @data.
 *
 * Return value: the new #SoupBuffer.
 **/
SoupBuffer *
soup_buffer_new (SoupMemoryUse use, gconstpointer data, gsize length)
{
	SoupBufferPrivate *priv = g_slice_new0 (SoupBufferPrivate);

	if (use == SOUP_MEMORY_COPY) {
		data = g_memdup (data, length);
		use = SOUP_MEMORY_TAKE;
	}

	priv->buffer.data = data;
	priv->buffer.length = length;
	priv->use = use;
	priv->refcount = 1;

	if (use == SOUP_MEMORY_TAKE) {
		priv->owner = (gpointer)data;
		priv->owner_dnotify = g_free;
	}

	return (SoupBuffer *)priv;
}

/**
 * soup_buffer_new_take:
 * @data: (array length=length) (transfer full): data
 * @length: length of @data
 *
 * Creates a new #SoupBuffer containing @length bytes from @data.
 *
 * This function is exactly equivalent to soup_buffer_new() with
 * %SOUP_MEMORY_TAKE as first argument; it exists mainly for
 * convenience and simplifying language bindings.
 *
 * Return value: the new #SoupBuffer.
 *
 * Rename to: soup_buffer_new
 * Since: 2.32
 **/
SoupBuffer *
soup_buffer_new_take (guchar *data, gsize length)
{
	return soup_buffer_new (SOUP_MEMORY_TAKE, data, length);
}

/**
 * soup_buffer_new_subbuffer:
 * @parent: the parent #SoupBuffer
 * @offset: offset within @parent to start at
 * @length: number of bytes to copy from @parent
 *
 * Creates a new #SoupBuffer containing @length bytes "copied" from
 * @parent starting at @offset. (Normally this will not actually copy
 * any data, but will instead simply reference the same data as
 * @parent does.)
 *
 * Return value: the new #SoupBuffer.
 **/
SoupBuffer *
soup_buffer_new_subbuffer (SoupBuffer *parent, gsize offset, gsize length)
{
	SoupBufferPrivate *priv;

	/* Normally this is just a ref, but if @parent is TEMPORARY,
	 * it will do an actual copy.
	 */
	parent = soup_buffer_copy (parent);

	priv = g_slice_new0 (SoupBufferPrivate);
	priv->buffer.data = parent->data + offset;
	priv->buffer.length = length;
	priv->use = SOUP_MEMORY_SUBBUFFER;
	priv->owner = parent;
	priv->owner_dnotify = (GDestroyNotify)soup_buffer_free;
	priv->refcount = 1;

	return (SoupBuffer *)priv;
}

/**
 * soup_buffer_new_with_owner:
 * @data: data
 * @length: length of @data
 * @owner: pointer to an object that owns @data
 * @owner_dnotify: (allow-none): a function to free/unref @owner when
 * the buffer is freed
 *
 * Creates a new #SoupBuffer containing @length bytes from @data. When
 * the #SoupBuffer is freed, it will call @owner_dnotify, passing
 * @owner to it. You must ensure that @data will remain valid until
 * @owner_dnotify is called.
 *
 * For example, you could use this to create a buffer containing data
 * returned from libxml without needing to do an extra copy:
 *
 * <informalexample><programlisting>
 *	xmlDocDumpMemory (doc, &xmlbody, &len);
 *	return soup_buffer_new_with_owner (xmlbody, len, xmlbody,
 *	                                   (GDestroyNotify)xmlFree);
 * </programlisting></informalexample>
 *
 * In this example, @data and @owner are the same, but in other cases
 * they would be different (eg, @owner would be a object, and @data
 * would be a pointer to one of the object's fields).
 *
 * Return value: the new #SoupBuffer.
 **/
SoupBuffer *
soup_buffer_new_with_owner (gconstpointer  data, gsize length,
			    gpointer owner, GDestroyNotify owner_dnotify)
{
	SoupBufferPrivate *priv = g_slice_new0 (SoupBufferPrivate);

	priv->buffer.data = data;
	priv->buffer.length = length;
	priv->use = SOUP_MEMORY_OWNED;
	priv->owner = owner;
	priv->owner_dnotify = owner_dnotify;
	priv->refcount = 1;

	return (SoupBuffer *)priv;
}

/**
 * soup_buffer_get_owner:
 * @buffer: a #SoupBuffer created with soup_buffer_new_with_owner()
 *
 * Gets the "owner" object for a buffer created with
 * soup_buffer_new_with_owner().
 *
 * Return value: (transfer none): the owner pointer
 **/
gpointer
soup_buffer_get_owner (SoupBuffer *buffer)
{
	SoupBufferPrivate *priv = (SoupBufferPrivate *)buffer;

	g_return_val_if_fail ((int)priv->use == (int)SOUP_MEMORY_OWNED, NULL);
	return priv->owner;
}

/**
 * soup_buffer_get_data:
 * @buffer: a #SoupBuffer
 * @data: (out) (array length=length) (transfer none): the pointer
 * to the buffer data is stored here
 * @length: (out): the length of the buffer data is stored here
 *
 * This function exists for use by language bindings, because it's not
 * currently possible to get the right effect by annotating the fields
 * of #SoupBuffer.
 *
 * Since: 2.32
 */
void
soup_buffer_get_data (SoupBuffer     *buffer,
		      const guint8  **data,
		      gsize          *length)
{
	*data = (const guint8 *)buffer->data;
	*length = buffer->length;
}

/**
 * soup_buffer_copy:
 * @buffer: a #SoupBuffer
 *
 * Makes a copy of @buffer. In reality, #SoupBuffer is a refcounted
 * type, and calling soup_buffer_copy() will normally just increment
 * the refcount on @buffer and return it. However, if @buffer was
 * created with #SOUP_MEMORY_TEMPORARY memory, then soup_buffer_copy()
 * will actually return a copy of it, so that the data in the copy
 * will remain valid after the temporary buffer is freed.
 *
 * Return value: the new (or newly-reffed) buffer
 **/
SoupBuffer *
soup_buffer_copy (SoupBuffer *buffer)
{
	SoupBufferPrivate *priv = (SoupBufferPrivate *)buffer;

	/* For non-TEMPORARY buffers, this is just a ref */
	if (priv->use != SOUP_MEMORY_TEMPORARY) {
		priv->refcount++;
		return buffer;
	}

	/* For TEMPORARY buffers, we need to do a real copy the first
	 * time, and then after that, we just keep returning the copy.
	 * We store the copy in priv->owner, which is technically
	 * backwards, but it saves us from having to keep an extra
	 * pointer in SoupBufferPrivate.
	 */

	if (!priv->owner) {
		priv->owner = soup_buffer_new (SOUP_MEMORY_COPY,
					       buffer->data,
					       buffer->length);
		priv->owner_dnotify = (GDestroyNotify)soup_buffer_free;
	}
	return soup_buffer_copy (priv->owner);
}

/**
 * soup_buffer_free:
 * @buffer: a #SoupBuffer
 *
 * Frees @buffer. (In reality, as described in the documentation for
 * soup_buffer_copy(), this is actually an "unref" operation, and may
 * or may not actually free @buffer.)
 **/
void
soup_buffer_free (SoupBuffer *buffer)
{
	SoupBufferPrivate *priv = (SoupBufferPrivate *)buffer;

	if (!--priv->refcount) {
		if (priv->owner_dnotify)
			priv->owner_dnotify (priv->owner);
		g_slice_free (SoupBufferPrivate, priv);
	}
}

/**
 * soup_buffer_get_as_bytes:
 * @buffer: a #SoupBuffer
 *
 * Returns: (transfer full): a new #GBytes which has the same content
 * as the #SoupBuffer.
 *
 * Since: 2.40
 */
GBytes *
soup_buffer_get_as_bytes (SoupBuffer *buffer)
{
	SoupBuffer *copy;

	copy = soup_buffer_copy (buffer);
	return g_bytes_new_with_free_func (copy->data, copy->length,
					   (GDestroyNotify)soup_buffer_free,
					   copy);
}

G_DEFINE_BOXED_TYPE (SoupBuffer, soup_buffer, soup_buffer_copy, soup_buffer_free)


/**
 * SoupMessageBody:
 * @data: the data
 * @length: length of @data
 *
 * A #SoupMessage request or response body.
 *
 * Note that while @length always reflects the full length of the
 * message body, @data is normally %NULL, and will only be filled in
 * after soup_message_body_flatten() is called. For client-side
 * messages, this automatically happens for the response body after it
 * has been fully read, unless you set the
 * %SOUP_MESSAGE_OVERWRITE_CHUNKS flags. Likewise, for server-side
 * messages, the request body is automatically filled in after being
 * read.
 *
 * As an added bonus, when @data is filled in, it is always terminated
 * with a '\0' byte (which is not reflected in @length).
 **/

typedef struct {
	SoupMessageBody body;
	GSList *chunks, *last;
	SoupBuffer *flattened;
	gboolean accumulate;
	goffset base_offset;
	int ref_count;
} SoupMessageBodyPrivate;

/**
 * soup_message_body_new:
 *
 * Creates a new #SoupMessageBody. #SoupMessage uses this internally; you
 * will not normally need to call it yourself.
 *
 * Return value: a new #SoupMessageBody.
 **/
SoupMessageBody *
soup_message_body_new (void)
{
	SoupMessageBodyPrivate *priv;

	priv = g_slice_new0 (SoupMessageBodyPrivate);
	priv->accumulate = TRUE;
	priv->ref_count = 1;

	return (SoupMessageBody *)priv;
}

/**
 * soup_message_body_set_accumulate:
 * @body: a #SoupMessageBody
 * @accumulate: whether or not to accumulate body chunks in @body
 *
 * Sets or clears the accumulate flag on @body. (The default value is
 * %TRUE.) If set to %FALSE, @body's %data field will not be filled in
 * after the body is fully sent/received, and the chunks that make up
 * @body may be discarded when they are no longer needed.
 *
 * In particular, if you set this flag to %FALSE on an "incoming"
 * message body (that is, the #SoupMessage:response_body of a
 * client-side message, or #SoupMessage:request_body of a server-side
 * message), this will cause each chunk of the body to be discarded
 * after its corresponding #SoupMessage::got_chunk signal is emitted.
 * (This is equivalent to setting the deprecated
 * %SOUP_MESSAGE_OVERWRITE_CHUNKS flag on the message.)
 *
 * If you set this flag to %FALSE on the #SoupMessage:response_body of
 * a server-side message, it will cause each chunk of the body to be
 * discarded after its corresponding #SoupMessage::wrote_chunk signal
 * is emitted.
 *
 * If you set the flag to %FALSE on the #SoupMessage:request_body of a
 * client-side message, it will block the accumulation of chunks into
 * @body's %data field, but it will not normally cause the chunks to
 * be discarded after being written like in the server-side
 * #SoupMessage:response_body case, because the request body needs to
 * be kept around in case the request needs to be sent a second time
 * due to redirection or authentication. However, if you set the
 * %SOUP_MESSAGE_CAN_REBUILD flag on the message, then the chunks will
 * be discarded, and you will be responsible for recreating the
 * request body after the #SoupMessage::restarted signal is emitted.
 *
 * Since: 2.24
 **/
void
soup_message_body_set_accumulate (SoupMessageBody *body,
				  gboolean         accumulate)
{
	SoupMessageBodyPrivate *priv = (SoupMessageBodyPrivate *)body;

	priv->accumulate = accumulate;
}

/**
 * soup_message_body_get_accumulate:
 * @body: a #SoupMessageBody
 *
 * Gets the accumulate flag on @body; see
 * soup_message_body_set_accumulate() for details.
 *
 * Return value: the accumulate flag for @body.
 *
 * Since: 2.24
 **/
gboolean
soup_message_body_get_accumulate (SoupMessageBody *body)
{
	SoupMessageBodyPrivate *priv = (SoupMessageBodyPrivate *)body;

	return priv->accumulate;
}

static void
append_buffer (SoupMessageBody *body, SoupBuffer *buffer)
{
	SoupMessageBodyPrivate *priv = (SoupMessageBodyPrivate *)body;

	if (priv->last) {
		priv->last = g_slist_append (priv->last, buffer);
		priv->last = priv->last->next;
	} else
		priv->chunks = priv->last = g_slist_append (NULL, buffer);

	if (priv->flattened) {
		soup_buffer_free (priv->flattened);
		priv->flattened = NULL;
		body->data = NULL;
	}
	body->length += buffer->length;
}

/**
 * soup_message_body_append:
 * @body: a #SoupMessageBody
 * @use: how to use @data
 * @data: (array length=length) (element-type guint8): data to append
 * @length: length of @data
 *
 * Appends @length bytes from @data to @body according to @use.
 **/
void
soup_message_body_append (SoupMessageBody *body, SoupMemoryUse use,
			  gconstpointer data, gsize length)
{
	if (length > 0)
		append_buffer (body, soup_buffer_new (use, data, length));
	else if (use == SOUP_MEMORY_TAKE)
		g_free ((gpointer)data);
}

/**
 * soup_message_body_append_take:
 * @body: a #SoupMessageBody
 * @data: (array length=length) (transfer full): data to append
 * @length: length of @data
 *
 * Appends @length bytes from @data to @body.
 *
 * This function is exactly equivalent to soup_message_body_append()
 * with %SOUP_MEMORY_TAKE as second argument; it exists mainly for
 * convenience and simplifying language bindings.
 *
 * Rename to: soup_message_body_append
 * Since: 2.32
 **/
void
soup_message_body_append_take (SoupMessageBody *body,
			       guchar *data, gsize length)
{
	soup_message_body_append(body, SOUP_MEMORY_TAKE, data, length);
}

/**
 * soup_message_body_append_buffer:
 * @body: a #SoupMessageBody
 * @buffer: a #SoupBuffer
 *
 * Appends the data from @buffer to @body. (#SoupMessageBody uses
 * #SoupBuffers internally, so this is normally a constant-time
 * operation that doesn't actually require copying the data in
 * @buffer.)
 **/
void
soup_message_body_append_buffer (SoupMessageBody *body, SoupBuffer *buffer)
{
	g_return_if_fail (buffer->length > 0);
	append_buffer (body, soup_buffer_copy (buffer));
}

/**
 * soup_message_body_truncate:
 * @body: a #SoupMessageBody
 *
 * Deletes all of the data in @body.
 **/
void
soup_message_body_truncate (SoupMessageBody *body)
{
	SoupMessageBodyPrivate *priv = (SoupMessageBodyPrivate *)body;

	g_slist_free_full (priv->chunks, (GDestroyNotify)soup_buffer_free);
	priv->chunks = priv->last = NULL;
	priv->base_offset = 0;

	if (priv->flattened) {
		soup_buffer_free (priv->flattened);
		priv->flattened = NULL;
		body->data = NULL;
	}
	body->length = 0;
}

/**
 * soup_message_body_complete:
 * @body: a #SoupMessageBody
 *
 * Tags @body as being complete; Call this when using chunked encoding
 * after you have appended the last chunk.
 **/
void
soup_message_body_complete (SoupMessageBody *body)
{
	append_buffer (body, soup_buffer_new (SOUP_MEMORY_STATIC, NULL, 0));
}

/**
 * soup_message_body_flatten:
 * @body: a #SoupMessageBody
 *
 * Fills in @body's data field with a buffer containing all of the
 * data in @body (plus an additional '\0' byte not counted by @body's
 * length field).
 *
 * Return value: a #SoupBuffer containing the same data as @body.
 * (You must free this buffer if you do not want it.)
 **/
SoupBuffer *
soup_message_body_flatten (SoupMessageBody *body)
{
	SoupMessageBodyPrivate *priv = (SoupMessageBodyPrivate *)body;
	char *buf, *ptr;
	GSList *iter;
	SoupBuffer *chunk;

	g_return_val_if_fail (priv->accumulate == TRUE, NULL);

	if (!priv->flattened) {
#if GLIB_SIZEOF_SIZE_T < 8
		g_return_val_if_fail (body->length < G_MAXSIZE, NULL);
#endif

		buf = ptr = g_malloc (body->length + 1);
		for (iter = priv->chunks; iter; iter = iter->next) {
			chunk = iter->data;
			memcpy (ptr, chunk->data, chunk->length);
			ptr += chunk->length;
		}
		*ptr = '\0';

		priv->flattened = soup_buffer_new (SOUP_MEMORY_TAKE,
						   buf, body->length);
		body->data = priv->flattened->data;
	}

	return soup_buffer_copy (priv->flattened);
}

/**
 * soup_message_body_get_chunk:
 * @body: a #SoupMessageBody
 * @offset: an offset
 *
 * Gets a #SoupBuffer containing data from @body starting at @offset.
 * The size of the returned chunk is unspecified. You can iterate
 * through the entire body by first calling
 * soup_message_body_get_chunk() with an offset of 0, and then on each
 * successive call, increment the offset by the length of the
 * previously-returned chunk.
 *
 * If @offset is greater than or equal to the total length of @body,
 * then the return value depends on whether or not
 * soup_message_body_complete() has been called or not; if it has,
 * then soup_message_body_get_chunk() will return a 0-length chunk
 * (indicating the end of @body). If it has not, then
 * soup_message_body_get_chunk() will return %NULL (indicating that
 * @body may still potentially have more data, but that data is not
 * currently available).
 *
 * Return value: a #SoupBuffer, or %NULL.
 **/
SoupBuffer *
soup_message_body_get_chunk (SoupMessageBody *body, goffset offset)
{
	SoupMessageBodyPrivate *priv = (SoupMessageBodyPrivate *)body;
	GSList *iter;
	SoupBuffer *chunk = NULL;

	offset -= priv->base_offset;
	for (iter = priv->chunks; iter; iter = iter->next) {
		chunk = iter->data;

		if (offset < chunk->length || offset == 0)
			break;

		offset -= chunk->length;
	}

	if (!iter)
		return NULL;

	if (offset == 0)
		return soup_buffer_copy (chunk);
	else {
		return soup_buffer_new_subbuffer (chunk, offset,
						  chunk->length - offset);
	}
}

/**
 * soup_message_body_got_chunk:
 * @body: a #SoupMessageBody
 * @chunk: a #SoupBuffer received from the network
 *
 * Handles the #SoupMessageBody part of receiving a chunk of data from
 * the network. Normally this means appending @chunk to @body, exactly
 * as with soup_message_body_append_buffer(), but if you have set
 * @body's accumulate flag to %FALSE, then that will not happen.
 *
 * This is a low-level method which you should not normally need to
 * use.
 *
 * Since: 2.24
 **/
void
soup_message_body_got_chunk (SoupMessageBody *body, SoupBuffer *chunk)
{
	SoupMessageBodyPrivate *priv = (SoupMessageBodyPrivate *)body;

	if (!priv->accumulate)
		return;

	soup_message_body_append_buffer (body, chunk);
}

/**
 * soup_message_body_wrote_chunk:
 * @body: a #SoupMessageBody
 * @chunk: a #SoupBuffer returned from soup_message_body_get_chunk()
 *
 * Handles the #SoupMessageBody part of writing a chunk of data to the
 * network. Normally this is a no-op, but if you have set @body's
 * accumulate flag to %FALSE, then this will cause @chunk to be
 * discarded to free up memory.
 *
 * This is a low-level method which you should not need to use, and
 * there are further restrictions on its proper use which are not
 * documented here.
 *
 * Since: 2.24
 **/
void
soup_message_body_wrote_chunk (SoupMessageBody *body, SoupBuffer *chunk)
{
	SoupMessageBodyPrivate *priv = (SoupMessageBodyPrivate *)body;
	SoupBuffer *chunk2;

	if (priv->accumulate)
		return;

	chunk2 = priv->chunks->data;
	g_return_if_fail (chunk->length == chunk2->length);
	g_return_if_fail (chunk == chunk2 || ((SoupBufferPrivate *)chunk2)->use == SOUP_MEMORY_TEMPORARY);

	priv->chunks = g_slist_remove (priv->chunks, chunk2);
	if (!priv->chunks)
		priv->last = NULL;

	priv->base_offset += chunk2->length;
	soup_buffer_free (chunk2);
}

static SoupMessageBody *
soup_message_body_copy (SoupMessageBody *body)
{
	SoupMessageBodyPrivate *priv = (SoupMessageBodyPrivate *)body;

	priv->ref_count++;
	return body;
}

/**
 * soup_message_body_free:
 * @body: a #SoupMessageBody
 *
 * Frees @body. You will not normally need to use this, as
 * #SoupMessage frees its associated message bodies automatically.
 **/
void
soup_message_body_free (SoupMessageBody *body)
{
	SoupMessageBodyPrivate *priv = (SoupMessageBodyPrivate *)body;

	if (--priv->ref_count == 0) {
		soup_message_body_truncate (body);
		g_slice_free (SoupMessageBodyPrivate, priv);
	}
}

G_DEFINE_BOXED_TYPE (SoupMessageBody, soup_message_body, soup_message_body_copy, soup_message_body_free)
