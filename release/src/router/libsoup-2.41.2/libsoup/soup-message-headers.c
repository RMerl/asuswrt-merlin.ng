/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-message-headers.c: HTTP message header arrays
 *
 * Copyright (C) 2007, 2008 Red Hat, Inc.
 */

#include <string.h>

#include "soup-message-headers.h"
#include "soup.h"

/**
 * SECTION:soup-message-headers
 * @short_description: HTTP message headers
 * @see_also: #SoupMessage
 *
 * #SoupMessageHeaders represents the HTTP message headers associated
 * with a request or response.
 **/

/**
 * SoupMessageHeaders:
 *
 * The HTTP message headers associated with a request or response.
 */

/**
 * SoupMessageHeadersType:
 * @SOUP_MESSAGE_HEADERS_REQUEST: request headers
 * @SOUP_MESSAGE_HEADERS_RESPONSE: response headers
 * @SOUP_MESSAGE_HEADERS_MULTIPART: multipart body part headers
 *
 * Value passed to soup_message_headers_new() to set certain default
 * behaviors.
 **/

typedef void (*SoupHeaderSetter) (SoupMessageHeaders *, const char *);
static const char *intern_header_name (const char *name, SoupHeaderSetter *setter);
static void clear_special_headers (SoupMessageHeaders *hdrs);

typedef struct {
	const char *name;
	char *value;
} SoupHeader;

struct SoupMessageHeaders {
	GArray *array;
	GHashTable *concat;
	SoupMessageHeadersType type;

	SoupEncoding encoding;
	goffset content_length;
	SoupExpectation expectations;
	char *content_type;

	int ref_count;
};

/**
 * soup_message_headers_new:
 * @type: the type of headers
 *
 * Creates a #SoupMessageHeaders. (#SoupMessage does this
 * automatically for its own headers. You would only need to use this
 * method if you are manually parsing or generating message headers.)
 *
 * Return value: a new #SoupMessageHeaders
 **/
SoupMessageHeaders *
soup_message_headers_new (SoupMessageHeadersType type)
{
	SoupMessageHeaders *hdrs;

	hdrs = g_slice_new0 (SoupMessageHeaders);
	/* FIXME: is "5" a good default? */
	hdrs->array = g_array_sized_new (TRUE, FALSE, sizeof (SoupHeader), 5);
	hdrs->type = type;
	hdrs->encoding = -1;
	hdrs->ref_count = 1;

	return hdrs;
}

static SoupMessageHeaders *
soup_message_headers_copy (SoupMessageHeaders *hdrs)
{
	hdrs->ref_count++;
	return hdrs;
}

/**
 * soup_message_headers_free:
 * @hdrs: a #SoupMessageHeaders
 *
 * Frees @hdrs.
 **/
void
soup_message_headers_free (SoupMessageHeaders *hdrs)
{
	if (--hdrs->ref_count == 0) {
		soup_message_headers_clear (hdrs);
		g_array_free (hdrs->array, TRUE);
		g_clear_pointer (&hdrs->concat, g_hash_table_destroy);
		g_slice_free (SoupMessageHeaders, hdrs);
	}
}

G_DEFINE_BOXED_TYPE (SoupMessageHeaders, soup_message_headers, soup_message_headers_copy, soup_message_headers_free)

/**
 * soup_message_headers_clear:
 * @hdrs: a #SoupMessageHeaders
 *
 * Clears @hdrs.
 **/
void
soup_message_headers_clear (SoupMessageHeaders *hdrs)
{
	SoupHeader *hdr_array = (SoupHeader *)hdrs->array->data;
	int i;

	for (i = 0; i < hdrs->array->len; i++)
		g_free (hdr_array[i].value);
	g_array_set_size (hdrs->array, 0);

	if (hdrs->concat)
		g_hash_table_remove_all (hdrs->concat);

	clear_special_headers (hdrs);
}

/**
 * soup_message_headers_clean_connection_headers:
 * @hdrs: a #SoupMessageHeaders
 *
 * Removes all the headers listed in the Connection header.
 *
 * Since: 2.36
 */
void
soup_message_headers_clean_connection_headers (SoupMessageHeaders *hdrs)
{
	/* RFC 2616 14.10 */
	const char *connection;
	GSList *tokens, *t;

	connection = soup_message_headers_get_list (hdrs, "Connection");
	if (!connection)
		return;

	tokens = soup_header_parse_list (connection);
	for (t = tokens; t; t = t->next)
		soup_message_headers_remove (hdrs, t->data);
	soup_header_free_list (tokens);
}

/**
 * soup_message_headers_append:
 * @hdrs: a #SoupMessageHeaders
 * @name: the header name to add
 * @value: the new value of @name
 *
 * Appends a new header with name @name and value @value to @hdrs. (If
 * there is an existing header with name @name, then this creates a
 * second one, which is only allowed for list-valued headers; see also
 * soup_message_headers_replace().)
 *
 * The caller is expected to make sure that @name and @value are
 * syntactically correct.
 **/
void
soup_message_headers_append (SoupMessageHeaders *hdrs,
			     const char *name, const char *value)
{
	SoupHeader header;
	SoupHeaderSetter setter;

	g_return_if_fail (name != NULL);
	g_return_if_fail (value != NULL);

	/* Setting a syntactically invalid header name or value is
	 * considered to be a programming error. However, it can also
	 * be a security hole, so we want to fail here even if
	 * compiled with G_DISABLE_CHECKS.
	 */
#ifndef G_DISABLE_CHECKS
	g_return_if_fail (*name && strpbrk (name, " \t\r\n:") == NULL);
	g_return_if_fail (strpbrk (value, "\r\n") == NULL);
#else
	if (*name && strpbrk (name, " \t\r\n:")) {
		g_warning ("soup_message_headers_append: Ignoring bad name '%s'", name);
		return;
	}
	if (strpbrk (value, "\r\n")) {
		g_warning ("soup_message_headers_append: Ignoring bad value '%s'", value);
		return;
	}
#endif

	header.name = intern_header_name (name, &setter);
	header.value = g_strdup (value);
	g_array_append_val (hdrs->array, header);
	if (hdrs->concat)
		g_hash_table_remove (hdrs->concat, header.name);
	if (setter)
		setter (hdrs, header.value);
}

/**
 * soup_message_headers_replace:
 * @hdrs: a #SoupMessageHeaders
 * @name: the header name to replace
 * @value: the new value of @name
 *
 * Replaces the value of the header @name in @hdrs with @value. (See
 * also soup_message_headers_append().)
 *
 * The caller is expected to make sure that @name and @value are
 * syntactically correct.
 **/
void
soup_message_headers_replace (SoupMessageHeaders *hdrs,
			      const char *name, const char *value)
{
	soup_message_headers_remove (hdrs, name);
	soup_message_headers_append (hdrs, name, value);
}

static int
find_header (SoupHeader *hdr_array, const char *interned_name, int nth)
{
	int i;

	for (i = 0; hdr_array[i].name; i++) {
		if (hdr_array[i].name == interned_name) {
			if (nth-- == 0)
				return i;
		}
	}
	return -1;
}

static int
find_last_header (SoupHeader *hdr_array, guint length, const char *interned_name, int nth)
{
	int i;

	for (i = length; i >= 0; i--) {
		if (hdr_array[i].name == interned_name) {
			if (nth-- == 0)
				return i;
		}
	}
	return -1;
}

/**
 * soup_message_headers_remove:
 * @hdrs: a #SoupMessageHeaders
 * @name: the header name to remove
 *
 * Removes @name from @hdrs. If there are multiple values for @name,
 * they are all removed.
 **/
void
soup_message_headers_remove (SoupMessageHeaders *hdrs, const char *name)
{
	SoupHeader *hdr_array = (SoupHeader *)(hdrs->array->data);
	SoupHeaderSetter setter;
	int index;

	g_return_if_fail (name != NULL);

	name = intern_header_name (name, &setter);
	while ((index = find_header (hdr_array, name, 0)) != -1) {
		g_free (hdr_array[index].value);
		g_array_remove_index (hdrs->array, index);
	}
	if (hdrs->concat)
		g_hash_table_remove (hdrs->concat, name);
	if (setter)
		setter (hdrs, NULL);
}

/**
 * soup_message_headers_get_one:
 * @hdrs: a #SoupMessageHeaders
 * @name: header name
 * 
 * Gets the value of header @name in @hdrs. Use this for headers whose
 * values are <emphasis>not</emphasis> comma-delimited lists, and
 * which therefore can only appear at most once in the headers. For
 * list-valued headers, use soup_message_headers_get_list().
 *
 * If @hdrs does erroneously contain multiple copies of the header, it
 * is not defined which one will be returned. (Ideally, it will return
 * whichever one makes libsoup most compatible with other HTTP
 * implementations.)
 *
 * Return value: the header's value or %NULL if not found.
 *
 * Since: 2.28
 **/
const char *
soup_message_headers_get_one (SoupMessageHeaders *hdrs, const char *name)
{
	SoupHeader *hdr_array = (SoupHeader *)(hdrs->array->data);
	guint hdr_length = hdrs->array->len;
	int index;

	g_return_val_if_fail (name != NULL, NULL);

	name = intern_header_name (name, NULL);

	index = find_last_header (hdr_array, hdr_length, name, 0);

	return (index == -1) ? NULL : hdr_array[index].value;
}

/**
 * soup_message_headers_get_list:
 * @hdrs: a #SoupMessageHeaders
 * @name: header name
 * 
 * Gets the value of header @name in @hdrs. Use this for headers whose
 * values are comma-delimited lists, and which are therefore allowed
 * to appear multiple times in the headers. For non-list-valued
 * headers, use soup_message_headers_get_one().
 *
 * If @name appears multiple times in @hdrs,
 * soup_message_headers_get_list() will concatenate all of the values
 * together, separated by commas. This is sometimes awkward to parse
 * (eg, WWW-Authenticate, Set-Cookie), but you have to be able to deal
 * with it anyway, because the HTTP spec explicitly states that this
 * transformation is allowed, and so an upstream proxy could do the
 * same thing.
 * 
 * Return value: the header's value or %NULL if not found.
 *
 * Since: 2.28
 **/
const char *
soup_message_headers_get_list (SoupMessageHeaders *hdrs, const char *name)
{
	SoupHeader *hdr_array = (SoupHeader *)(hdrs->array->data);
	GString *concat;
	char *value;
	int index, i;

	g_return_val_if_fail (name != NULL, NULL);

	name = intern_header_name (name, NULL);
	if (hdrs->concat) {
		value = g_hash_table_lookup (hdrs->concat, name);
		if (value)
			return value;
	}

	index = find_header (hdr_array, name, 0);
	if (index == -1)
		return NULL;
	else if (find_header (hdr_array, name, 1) == -1)
		return hdr_array[index].value;

	concat = g_string_new (NULL);
	for (i = 0; (index = find_header (hdr_array, name, i)) != -1; i++) {
		if (i != 0)
			g_string_append (concat, ", ");
		g_string_append (concat, hdr_array[index].value);
	}
	value = g_string_free (concat, FALSE);

	if (!hdrs->concat)
		hdrs->concat = g_hash_table_new_full (NULL, NULL, NULL, g_free);
	g_hash_table_insert (hdrs->concat, (gpointer)name, value);
	return value;
}

/**
 * soup_message_headers_get:
 * @hdrs: a #SoupMessageHeaders
 * @name: header name
 * 
 * Gets the value of header @name in @hdrs.
 *
 * This method was supposed to work correctly for both single-valued
 * and list-valued headers, but because some HTTP clients/servers
 * mistakenly send multiple copies of headers that are supposed to be
 * single-valued, it sometimes returns incorrect results. To fix this,
 * the methods soup_message_headers_get_one() and
 * soup_message_headers_get_list() were introduced, so callers can
 * explicitly state which behavior they are expecting.
 *
 * Return value: as with soup_message_headers_get_list().
 * 
 * Deprecated: Use soup_message_headers_get_one() or
 * soup_message_headers_get_list() instead.
 **/
const char *
soup_message_headers_get (SoupMessageHeaders *hdrs, const char *name)
{
	return soup_message_headers_get_list (hdrs, name);
}

/**
 * SoupMessageHeadersIter:
 *
 * An opaque type used to iterate over a %SoupMessageHeaders
 * structure.
 *
 * After intializing the iterator with
 * soup_message_headers_iter_init(), call
 * soup_message_headers_iter_next() to fetch data from it.
 *
 * You may not modify the headers while iterating over them.
 **/

typedef struct {
	SoupMessageHeaders *hdrs;
	int index;
} SoupMessageHeadersIterReal;

/**
 * soup_message_headers_iter_init:
 * @iter: (out) (transfer none): a pointer to a %SoupMessageHeadersIter
 * structure
 * @hdrs: a %SoupMessageHeaders
 *
 * Initializes @iter for iterating @hdrs.
 **/
void
soup_message_headers_iter_init (SoupMessageHeadersIter *iter,
				SoupMessageHeaders *hdrs)
{
	SoupMessageHeadersIterReal *real = (SoupMessageHeadersIterReal *)iter;

	real->hdrs = hdrs;
	real->index = 0;
}

/**
 * soup_message_headers_iter_next:
 * @iter: (inout) (transfer none): a %SoupMessageHeadersIter
 * @name: (out) (transfer none): pointer to a variable to return
 * the header name in
 * @value: (out) (transfer none): pointer to a variable to return
 * the header value in
 *
 * Yields the next name/value pair in the %SoupMessageHeaders being
 * iterated by @iter. If @iter has already yielded the last header,
 * then soup_message_headers_iter_next() will return %FALSE and @name
 * and @value will be unchanged.
 *
 * Return value: %TRUE if another name and value were returned, %FALSE
 * if the end of the headers has been reached.
 **/
gboolean
soup_message_headers_iter_next (SoupMessageHeadersIter *iter,
				const char **name, const char **value)
{
	SoupMessageHeadersIterReal *real = (SoupMessageHeadersIterReal *)iter;
	SoupHeader *hdr_array = (SoupHeader *)real->hdrs->array->data;

	if (real->index >= real->hdrs->array->len)
		return FALSE;

	*name = hdr_array[real->index].name;
	*value = hdr_array[real->index].value;
	real->index++;
	return TRUE;
}

/**
 * SoupMessageHeadersForeachFunc:
 * @name: the header name
 * @value: the header value
 * @user_data: the data passed to soup_message_headers_foreach()
 *
 * The callback passed to soup_message_headers_foreach().
 **/

/**
 * soup_message_headers_foreach:
 * @hdrs: a #SoupMessageHeaders
 * @func: (scope call): callback function to run for each header
 * @user_data: data to pass to @func
 * 
 * Calls @func once for each header value in @hdrs.
 *
 * Beware that unlike soup_message_headers_get(), this processes the
 * headers in exactly the way they were added, rather than
 * concatenating multiple same-named headers into a single value.
 * (This is intentional; it ensures that if you call
 * soup_message_headers_append() multiple times with the same name,
 * then the I/O code will output multiple copies of the header when
 * sending the message to the remote implementation, which may be
 * required for interoperability in some cases.)
 *
 * You may not modify the headers from @func.
 **/
void
soup_message_headers_foreach (SoupMessageHeaders *hdrs,
			      SoupMessageHeadersForeachFunc func,
			      gpointer            user_data)
{
	SoupHeader *hdr_array = (SoupHeader *)hdrs->array->data;
	int i;

	for (i = 0; i < hdrs->array->len; i++)
		func (hdr_array[i].name, hdr_array[i].value, user_data);
}


G_LOCK_DEFINE_STATIC (header_pool);
static GHashTable *header_pool, *header_setters;

static void transfer_encoding_setter (SoupMessageHeaders *, const char *);
static void content_length_setter (SoupMessageHeaders *, const char *);
static void expectation_setter (SoupMessageHeaders *, const char *);
static void content_type_setter (SoupMessageHeaders *, const char *);

static char *
intern_header_locked (const char *name)
{
	char *interned;

	interned = g_hash_table_lookup (header_pool, name);
	if (!interned) {
		char *dup = g_strdup (name);
		g_hash_table_insert (header_pool, dup, dup);
		interned = dup;
	}
	return interned;
}

static const char *
intern_header_name (const char *name, SoupHeaderSetter *setter)
{
	const char *interned;

	G_LOCK (header_pool);

	if (!header_pool) {
		header_pool = g_hash_table_new (soup_str_case_hash, soup_str_case_equal);
		header_setters = g_hash_table_new (NULL, NULL);
		g_hash_table_insert (header_setters,
				     intern_header_locked ("Transfer-Encoding"),
				     transfer_encoding_setter);
		g_hash_table_insert (header_setters,
				     intern_header_locked ("Content-Length"),
				     content_length_setter);
		g_hash_table_insert (header_setters,
				     intern_header_locked ("Expect"),
				     expectation_setter);
		g_hash_table_insert (header_setters,
				     intern_header_locked ("Content-Type"),
				     content_type_setter);
	}

	interned = intern_header_locked (name);
	if (setter)
		*setter = g_hash_table_lookup (header_setters, interned);

	G_UNLOCK (header_pool);
	return interned;
}

static void
clear_special_headers (SoupMessageHeaders *hdrs)
{
	SoupHeaderSetter setter;
	GHashTableIter iter;
	gpointer key, value;

	/* Make sure header_setters has been initialized */
	intern_header_name ("", NULL);

	g_hash_table_iter_init (&iter, header_setters);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		setter = value;
		setter (hdrs, NULL);
	}
}

/* Specific headers */

static void
transfer_encoding_setter (SoupMessageHeaders *hdrs, const char *value)
{
	if (value) {
		if (g_ascii_strcasecmp (value, "chunked") == 0)
			hdrs->encoding = SOUP_ENCODING_CHUNKED;
		else
			hdrs->encoding = SOUP_ENCODING_UNRECOGNIZED;
	} else
		hdrs->encoding = -1;
}

static void
content_length_setter (SoupMessageHeaders *hdrs, const char *value)
{
	/* Transfer-Encoding trumps Content-Length */
	if (hdrs->encoding == SOUP_ENCODING_CHUNKED)
		return;

	if (value) {
		char *end;

		hdrs->content_length = g_ascii_strtoull (value, &end, 10);
		if (*end)
			hdrs->encoding = SOUP_ENCODING_UNRECOGNIZED;
		else
			hdrs->encoding = SOUP_ENCODING_CONTENT_LENGTH;
	} else
		hdrs->encoding = -1;
}

/**
 * SoupEncoding:
 * @SOUP_ENCODING_UNRECOGNIZED: unknown / error
 * @SOUP_ENCODING_NONE: no body is present (which is not the same as a
 * 0-length body, and only occurs in certain places)
 * @SOUP_ENCODING_CONTENT_LENGTH: Content-Length encoding
 * @SOUP_ENCODING_EOF: Response body ends when the connection is closed
 * @SOUP_ENCODING_CHUNKED: chunked encoding (currently only supported
 * for response)
 * @SOUP_ENCODING_BYTERANGES: multipart/byteranges (Reserved for future
 * use: NOT CURRENTLY IMPLEMENTED)
 *
 * How a message body is encoded for transport
 **/

/**
 * soup_message_headers_get_encoding:
 * @hdrs: a #SoupMessageHeaders
 *
 * Gets the message body encoding that @hdrs declare. This may not
 * always correspond to the encoding used on the wire; eg, a HEAD
 * response may declare a Content-Length or Transfer-Encoding, but
 * it will never actually include a body.
 *
 * Return value: the encoding declared by @hdrs.
 **/
SoupEncoding
soup_message_headers_get_encoding (SoupMessageHeaders *hdrs)
{
	const char *header;

	if (hdrs->encoding != -1)
		return hdrs->encoding;

	/* If Transfer-Encoding was set, hdrs->encoding would already
	 * be set. So we don't need to check that possibility.
	 */
	header = soup_message_headers_get_one (hdrs, "Content-Length");
	if (header) {
		content_length_setter (hdrs, header);
		if (hdrs->encoding != -1)
			return hdrs->encoding;
	}

	/* Per RFC 2616 4.4, a response body that doesn't indicate its
	 * encoding otherwise is terminated by connection close, and a
	 * request that doesn't indicate otherwise has no body. Note
	 * that SoupMessage calls soup_message_headers_set_encoding()
	 * to override the response body default for our own
	 * server-side messages.
	 */
	hdrs->encoding = (hdrs->type == SOUP_MESSAGE_HEADERS_RESPONSE) ?
		SOUP_ENCODING_EOF : SOUP_ENCODING_NONE;
	return hdrs->encoding;
}

/**
 * soup_message_headers_set_encoding:
 * @hdrs: a #SoupMessageHeaders
 * @encoding: a #SoupEncoding
 *
 * Sets the message body encoding that @hdrs will declare. In particular,
 * you should use this if you are going to send a request or response in
 * chunked encoding.
 **/
void
soup_message_headers_set_encoding (SoupMessageHeaders *hdrs,
				   SoupEncoding        encoding)
{
	if (encoding == hdrs->encoding)
		return;

	switch (encoding) {
	case SOUP_ENCODING_NONE:
	case SOUP_ENCODING_EOF:
		soup_message_headers_remove (hdrs, "Transfer-Encoding");
		soup_message_headers_remove (hdrs, "Content-Length");
		break;

	case SOUP_ENCODING_CONTENT_LENGTH:
		soup_message_headers_remove (hdrs, "Transfer-Encoding");
		break;

	case SOUP_ENCODING_CHUNKED:
		soup_message_headers_remove (hdrs, "Content-Length");
		soup_message_headers_replace (hdrs, "Transfer-Encoding", "chunked");
		break;

	default:
		g_return_if_reached ();
	}

	hdrs->encoding = encoding;
}

/**
 * soup_message_headers_get_content_length:
 * @hdrs: a #SoupMessageHeaders
 *
 * Gets the message body length that @hdrs declare. This will only
 * be non-0 if soup_message_headers_get_encoding() returns
 * %SOUP_ENCODING_CONTENT_LENGTH.
 *
 * Return value: the message body length declared by @hdrs.
 **/
goffset
soup_message_headers_get_content_length (SoupMessageHeaders *hdrs)
{
	SoupEncoding encoding;

	encoding = soup_message_headers_get_encoding (hdrs);
	if (encoding == SOUP_ENCODING_CONTENT_LENGTH)
		return hdrs->content_length;
	else
		return 0;
}

/**
 * soup_message_headers_set_content_length:
 * @hdrs: a #SoupMessageHeaders
 * @content_length: the message body length
 *
 * Sets the message body length that @hdrs will declare, and sets
 * @hdrs's encoding to %SOUP_ENCODING_CONTENT_LENGTH.
 *
 * You do not normally need to call this; if @hdrs is set to use
 * Content-Length encoding, libsoup will automatically set its
 * Content-Length header for you immediately before sending the
 * headers. One situation in which this method is useful is when
 * generating the response to a HEAD request; Calling
 * soup_message_headers_set_content_length() allows you to put the
 * correct content length into the response without needing to waste
 * memory by filling in a response body which won't actually be sent.
 **/
void
soup_message_headers_set_content_length (SoupMessageHeaders *hdrs,
					 goffset             content_length)
{
	char length[128];

	g_snprintf (length, sizeof (length), "%" G_GUINT64_FORMAT,
		    content_length);
	soup_message_headers_remove (hdrs, "Transfer-Encoding");
	soup_message_headers_replace (hdrs, "Content-Length", length);
}

static void
expectation_setter (SoupMessageHeaders *hdrs, const char *value)
{
	if (value) {
		if (!g_ascii_strcasecmp (value, "100-continue"))
			hdrs->expectations = SOUP_EXPECTATION_CONTINUE;
		else
			hdrs->expectations = SOUP_EXPECTATION_UNRECOGNIZED;
	} else
		hdrs->expectations = 0;
}

/**
 * SoupExpectation:
 * @SOUP_EXPECTATION_CONTINUE: "100-continue"
 * @SOUP_EXPECTATION_UNRECOGNIZED: any unrecognized expectation
 *
 * Represents the parsed value of the "Expect" header.
 **/

/**
 * soup_message_headers_get_expectations:
 * @hdrs: a #SoupMessageHeaders
 *
 * Gets the expectations declared by @hdrs's "Expect" header.
 * Currently this will either be %SOUP_EXPECTATION_CONTINUE or
 * %SOUP_EXPECTATION_UNRECOGNIZED.
 *
 * Return value: the contents of @hdrs's "Expect" header
 **/
SoupExpectation
soup_message_headers_get_expectations (SoupMessageHeaders *hdrs)
{
	return hdrs->expectations;
}

/**
 * soup_message_headers_set_expectations:
 * @hdrs: a #SoupMessageHeaders
 * @expectations: the expectations to set
 *
 * Sets @hdrs's "Expect" header according to @expectations.
 *
 * Currently %SOUP_EXPECTATION_CONTINUE is the only known expectation
 * value. You should set this value on a request if you are sending a
 * large message body (eg, via POST or PUT), and want to give the
 * server a chance to reject the request after seeing just the headers
 * (eg, because it will require authentication before allowing you to
 * post, or because you're POSTing to a URL that doesn't exist). This
 * saves you from having to transmit the large request body when the
 * server is just going to ignore it anyway.
 **/
void
soup_message_headers_set_expectations (SoupMessageHeaders *hdrs,
				       SoupExpectation     expectations)
{
	g_return_if_fail ((expectations & ~SOUP_EXPECTATION_CONTINUE) == 0);

	if (expectations & SOUP_EXPECTATION_CONTINUE)
		soup_message_headers_replace (hdrs, "Expect", "100-continue");
	else
		soup_message_headers_remove (hdrs, "Expect");
}

/**
 * SoupRange:
 * @start: the start of the range
 * @end: the end of the range
 *
 * Represents a byte range as used in the Range header.
 *
 * If @end is non-negative, then @start and @end represent the bounds
 * of of the range, counting from 0. (Eg, the first 500 bytes would be
 * represented as @start = 0 and @end = 499.)
 *
 * If @end is -1 and @start is non-negative, then this represents a
 * range starting at @start and ending with the last byte of the
 * requested resource body. (Eg, all but the first 500 bytes would be
 * @start = 500, and @end = -1.)
 *
 * If @end is -1 and @start is negative, then it represents a "suffix
 * range", referring to the last -@start bytes of the resource body.
 * (Eg, the last 500 bytes would be @start = -500 and @end = -1.)
 *
 * Since: 2.26
 **/

static int
sort_ranges (gconstpointer a, gconstpointer b)
{
	SoupRange *ra = (SoupRange *)a;
	SoupRange *rb = (SoupRange *)b;

	return ra->start - rb->start;
}

/**
 * soup_message_headers_get_ranges:
 * @hdrs: a #SoupMessageHeaders
 * @total_length: the total_length of the response body
 * @ranges: (out): return location for an array of #SoupRange
 * @length: the length of the returned array
 *
 * Parses @hdrs's Range header and returns an array of the requested
 * byte ranges. The returned array must be freed with
 * soup_message_headers_free_ranges().
 *
 * If @total_length is non-0, its value will be used to adjust the
 * returned ranges to have explicit start and end values, and the
 * returned ranges will be sorted and non-overlapping. If
 * @total_length is 0, then some ranges may have an end value of -1,
 * as described under #SoupRange, and some of the ranges may be
 * redundant.
 *
 * Return value: %TRUE if @hdrs contained a "Range" header containing
 * byte ranges which could be parsed, %FALSE otherwise (in which case
 * @range and @length will not be set).
 *
 * Since: 2.26
 **/
gboolean
soup_message_headers_get_ranges (SoupMessageHeaders  *hdrs,
				 goffset              total_length,
				 SoupRange          **ranges,
				 int                 *length)
{
	const char *range = soup_message_headers_get_one (hdrs, "Range");
	GSList *range_list, *r;
	GArray *array;
	char *spec, *end;
	int i;

	if (!range || strncmp (range, "bytes", 5) != 0)
		return FALSE;

	range += 5;
	while (g_ascii_isspace (*range))
		range++;
	if (*range++ != '=')
		return FALSE;
	while (g_ascii_isspace (*range))
		range++;

	range_list = soup_header_parse_list (range);
	if (!range_list)
		return FALSE;

	array = g_array_new (FALSE, FALSE, sizeof (SoupRange));
	for (r = range_list; r; r = r->next) {
		SoupRange cur;

		spec = r->data;
		if (*spec == '-') {
			cur.start = g_ascii_strtoll (spec, &end, 10) + total_length;
			cur.end = total_length - 1;
		} else {
			cur.start = g_ascii_strtoull (spec, &end, 10);
			if (*end == '-')
				end++;
			if (*end)
				cur.end = g_ascii_strtoull (end, &end, 10);
			else
				cur.end = total_length - 1;
		}
		if (*end) {
			g_array_free (array, TRUE);
			soup_header_free_list (range_list);
			return FALSE;
		}

		g_array_append_val (array, cur);
	}

	soup_header_free_list (range_list);

	if (total_length) {
		g_array_sort (array, sort_ranges);
		for (i = 1; i < array->len; i++) {
			SoupRange *cur = &((SoupRange *)array->data)[i];
			SoupRange *prev = &((SoupRange *)array->data)[i - 1];

			if (cur->start <= prev->end) {
				prev->end = MAX (prev->end, cur->end);
				g_array_remove_index (array, i);
			}
		}
	}

	*ranges = (SoupRange *)array->data;
	*length = array->len;

	g_array_free (array, FALSE);
	return TRUE;
}

/**
 * soup_message_headers_free_ranges:
 * @hdrs: a #SoupMessageHeaders
 * @ranges: an array of #SoupRange
 *
 * Frees the array of ranges returned from soup_message_headers_get_ranges().
 *
 * Since: 2.26
 **/
void
soup_message_headers_free_ranges (SoupMessageHeaders  *hdrs,
				  SoupRange           *ranges)
{
	g_free (ranges);
}

/**
 * soup_message_headers_set_ranges:
 * @hdrs: a #SoupMessageHeaders
 * @ranges: an array of #SoupRange
 * @length: the length of @range
 *
 * Sets @hdrs's Range header to request the indicated ranges. (If you
 * only want to request a single range, you can use
 * soup_message_headers_set_range().)
 *
 * Since: 2.26
 **/
void
soup_message_headers_set_ranges (SoupMessageHeaders  *hdrs,
				 SoupRange           *ranges,
				 int                  length)
{
	GString *header;
	int i;

	header = g_string_new ("bytes=");
	for (i = 0; i < length; i++) {
		if (i > 0)
			g_string_append_c (header, ',');
		if (ranges[i].end >= 0) {
			g_string_append_printf (header, "%" G_GINT64_FORMAT "-%" G_GINT64_FORMAT,
						ranges[i].start, ranges[i].end);
		} else if (ranges[i].start >= 0) {
			g_string_append_printf (header,"%" G_GINT64_FORMAT "-",
						ranges[i].start);
		} else {
			g_string_append_printf (header, "%" G_GINT64_FORMAT,
						ranges[i].start);
		}
	}

	soup_message_headers_replace (hdrs, "Range", header->str);
	g_string_free (header, TRUE);
}

/**
 * soup_message_headers_set_range:
 * @hdrs: a #SoupMessageHeaders
 * @start: the start of the range to request
 * @end: the end of the range to request
 *
 * Sets @hdrs's Range header to request the indicated range.
 * @start and @end are interpreted as in a #SoupRange.
 *
 * If you need to request multiple ranges, use
 * soup_message_headers_set_ranges().
 *
 * Since: 2.26
 **/
void
soup_message_headers_set_range (SoupMessageHeaders  *hdrs,
				goffset              start,
				goffset              end)
{
	SoupRange range;

	range.start = start;
	range.end = end;
	soup_message_headers_set_ranges (hdrs, &range, 1);
}

/**
 * soup_message_headers_get_content_range:
 * @hdrs: a #SoupMessageHeaders
 * @start: return value for the start of the range
 * @end: return value for the end of the range
 * @total_length: return value for the total length of the resource,
 * or %NULL if you don't care.
 *
 * Parses @hdrs's Content-Range header and returns it in @start,
 * @end, and @total_length. If the total length field in the header
 * was specified as "*", then @total_length will be set to -1.
 *
 * Return value: %TRUE if @hdrs contained a "Content-Range" header
 * containing a byte range which could be parsed, %FALSE otherwise.
 *
 * Since: 2.26
 **/
gboolean
soup_message_headers_get_content_range (SoupMessageHeaders  *hdrs,
					goffset             *start,
					goffset             *end,
					goffset             *total_length)
{
	const char *header = soup_message_headers_get_one (hdrs, "Content-Range");
	goffset length;
	char *p;

	if (!header || strncmp (header, "bytes ", 6) != 0)
		return FALSE;

	header += 6;
	while (g_ascii_isspace (*header))
		header++;
	if (!g_ascii_isdigit (*header))
		return FALSE;

	*start = g_ascii_strtoull (header, &p, 10);
	if (*p != '-')
		return FALSE;
	*end = g_ascii_strtoull (p + 1, &p, 10);
	if (*p != '/')
		return FALSE;
	p++;
	if (*p == '*') {
		length = -1;
		p++;
	} else
		length = g_ascii_strtoull (p, &p, 10);

	if (total_length)
		*total_length = length;
	return *p == '\0';
}

/**
 * soup_message_headers_set_content_range:
 * @hdrs: a #SoupMessageHeaders
 * @start: the start of the range
 * @end: the end of the range
 * @total_length: the total length of the resource, or -1 if unknown
 *
 * Sets @hdrs's Content-Range header according to the given values.
 * (Note that @total_length is the total length of the entire resource
 * that this is a range of, not simply @end - @start + 1.)
 *
 * Since: 2.26
 **/
void
soup_message_headers_set_content_range (SoupMessageHeaders  *hdrs,
					goffset              start,
					goffset              end,
					goffset              total_length)
{
	char *header;

	if (total_length >= 0) {
		header = g_strdup_printf ("bytes %" G_GINT64_FORMAT "-%"
					  G_GINT64_FORMAT "/%" G_GINT64_FORMAT,
					  start, end, total_length);
	} else {
		header = g_strdup_printf ("bytes %" G_GINT64_FORMAT "-%"
					  G_GINT64_FORMAT "/*", start, end);
	}
	soup_message_headers_replace (hdrs, "Content-Range", header);
	g_free (header);
}

static gboolean
parse_content_foo (SoupMessageHeaders *hdrs, const char *header_name,
		   char **foo, GHashTable **params)
{
	const char *header;
	char *semi;

	header = soup_message_headers_get_one (hdrs, header_name);
	if (!header)
		return FALSE;

	if (foo) {
		*foo = g_strdup (header);
		semi = strchr (*foo, ';');
		if (semi) {
			char *p = semi;

			*semi++ = '\0';
			while (p - 1 > *foo && g_ascii_isspace(p[-1]))
				*(--p) = '\0';
		}
	} else {
		semi = strchr (header, ';');
		if (semi)
			semi++;
	}

	if (!params)
		return TRUE;

	if (!semi) {
		*params = soup_header_parse_semi_param_list ("");
		return TRUE;
	}

	*params = soup_header_parse_semi_param_list (semi);
	return TRUE;
}

static void
set_content_foo (SoupMessageHeaders *hdrs, const char *header_name,
		 const char *foo, GHashTable *params)
{
	GString *str;
	GHashTableIter iter;
	gpointer key, value;

	str = g_string_new (foo);
	if (params) {
		g_hash_table_iter_init (&iter, params);
		while (g_hash_table_iter_next (&iter, &key, &value)) {
			g_string_append (str, "; ");
			soup_header_g_string_append_param (str, key, value);
		}
	}

	soup_message_headers_replace (hdrs, header_name, str->str);
	g_string_free (str, TRUE);
}

static void
content_type_setter (SoupMessageHeaders *hdrs, const char *value)
{
	g_free (hdrs->content_type);
	if (value) {
		char *content_type, *p;

		parse_content_foo (hdrs, "Content-Type", &content_type, NULL);
		p = strpbrk (content_type, " /");
		if (!p || *p != '/' || strpbrk (p + 1, " /")) {
			g_free (content_type);
			hdrs->content_type = NULL;
		} else
			hdrs->content_type = content_type;
	} else
		hdrs->content_type = NULL;
}

/**
 * soup_message_headers_get_content_type:
 * @hdrs: a #SoupMessageHeaders
 * @params: (out) (element-type utf8 utf8) (allow-none) (transfer full):
 *   return location for the Content-Type parameters (eg, "charset"), or
 *   %NULL
 *
 * Looks up the "Content-Type" header in @hdrs, parses it, and returns
 * its value in *@content_type and *@params. @params can be %NULL if you
 * are only interested in the content type itself.
 *
 * Return value: a string with the value of the "Content-Type" header
 * or NULL if @hdrs does not contain that header or it cannot be
 * parsed (in which case *@params will be unchanged).
 *
 * Since: 2.26
 **/
const char *
soup_message_headers_get_content_type (SoupMessageHeaders  *hdrs,
				       GHashTable         **params)
{
	if (!hdrs->content_type)
		return NULL;

	if (params)
		parse_content_foo (hdrs, "Content-Type", NULL, params);
	return hdrs->content_type;
}

/**
 * soup_message_headers_set_content_type:
 * @hdrs: a #SoupMessageHeaders
 * @content_type: the MIME type
 * @params: (allow-none) (element-type utf8 utf8): additional
 * parameters, or %NULL
 *
 * Sets the "Content-Type" header in @hdrs to @content_type,
 * optionally with additional parameters specified in @params.
 *
 * Since: 2.26
 **/
void
soup_message_headers_set_content_type (SoupMessageHeaders  *hdrs,
				       const char          *content_type,
				       GHashTable          *params)
{
	set_content_foo (hdrs, "Content-Type", content_type, params);
}

/**
 * soup_message_headers_get_content_disposition:
 * @hdrs: a #SoupMessageHeaders
 * @disposition: (out) (transfer full): return location for the
 * disposition-type, or %NULL
 * @params: (out) (transfer full) (element-type utf8 utf8): return
 * location for the Content-Disposition parameters, or %NULL
 *
 * Looks up the "Content-Disposition" header in @hdrs, parses it, and
 * returns its value in *@disposition and *@params. @params can be
 * %NULL if you are only interested in the disposition-type.
 *
 * In HTTP, the most common use of this header is to set a
 * disposition-type of "attachment", to suggest to the browser that a
 * response should be saved to disk rather than displayed in the
 * browser. If @params contains a "filename" parameter, this is a
 * suggestion of a filename to use. (If the parameter value in the
 * header contains an absolute or relative path, libsoup will truncate
 * it down to just the final path component, so you do not need to
 * test this yourself.)
 *
 * Content-Disposition is also used in "multipart/form-data", however
 * this is handled automatically by #SoupMultipart and the associated
 * form methods.
 *
 * Return value: %TRUE if @hdrs contains a "Content-Disposition"
 * header, %FALSE if not (in which case *@disposition and *@params
 * will be unchanged).
 *
 * Since: 2.26
 **/
gboolean
soup_message_headers_get_content_disposition (SoupMessageHeaders  *hdrs,
					      char               **disposition,
					      GHashTable         **params)
{
	gpointer orig_key, orig_value;

	if (!parse_content_foo (hdrs, "Content-Disposition",
				disposition, params))
		return FALSE;

	/* If there is a filename parameter, make sure it contains
	 * only a single path component
	 */
	if (params && g_hash_table_lookup_extended (*params, "filename",
						    &orig_key, &orig_value)) {
		char *filename = strrchr (orig_value, '/');

		if (filename)
			g_hash_table_insert (*params, g_strdup (orig_key), filename + 1);
	}
	return TRUE;
}

/**
 * soup_message_headers_set_content_disposition:
 * @hdrs: a #SoupMessageHeaders
 * @disposition: the disposition-type
 * @params: (allow-none) (element-type utf8 utf8): additional
 * parameters, or %NULL
 *
 * Sets the "Content-Disposition" header in @hdrs to @disposition,
 * optionally with additional parameters specified in @params.
 *
 * See soup_message_headers_get_content_disposition() for a discussion
 * of how Content-Disposition is used in HTTP.
 *
 * Since: 2.26
 **/
void
soup_message_headers_set_content_disposition (SoupMessageHeaders  *hdrs,
					      const char          *disposition,
					      GHashTable          *params)
{
	set_content_foo (hdrs, "Content-Disposition", disposition, params);
}

