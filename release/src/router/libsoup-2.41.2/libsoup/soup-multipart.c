/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-multipart.c: multipart HTTP message bodies
 *
 * Copyright (C) 2008 Red Hat, Inc.
 */

#include <string.h>

#include "soup-multipart.h"
#include "soup.h"

/**
 * SECTION:soup-multipart
 * @short_description: multipart HTTP message bodies
 * @see_also: #SoupMessageBody, #SoupMessageHeaders
 *
 **/

/**
 * SoupMultipart:
 *
 * Represents a multipart HTTP message body, parsed according to the
 * syntax of RFC 2046. Of particular interest to HTTP are
 * <literal>multipart/byte-ranges</literal> and
 * <literal>multipart/form-data</literal>.
 *
 * Although the headers of a #SoupMultipart body part will contain the
 * full headers from that body part, libsoup does not interpret them
 * according to MIME rules. For example, each body part is assumed to
 * have "binary" Content-Transfer-Encoding, even if its headers
 * explicitly state otherwise. In other words, don't try to use
 * #SoupMultipart for handling real MIME multiparts.
 *
 * Since: 2.26
 **/

struct SoupMultipart {
	char *mime_type, *boundary;
	GPtrArray *headers, *bodies;
};

static SoupMultipart *
soup_multipart_new_internal (char *mime_type, char *boundary)
{
	SoupMultipart *multipart;

	multipart = g_slice_new (SoupMultipart);
	multipart->mime_type = mime_type;
	multipart->boundary = boundary;
	multipart->headers = g_ptr_array_new_with_free_func ((GDestroyNotify)soup_message_headers_free);
	multipart->bodies = g_ptr_array_new_with_free_func ((GDestroyNotify)soup_buffer_free);

	return multipart;
}

static char *
generate_boundary (void)
{
	static int counter;
	struct {
		GTimeVal timeval;
		int counter;
	} data;

	/* avoid valgrind warning */
	if (sizeof (data) != sizeof (data.timeval) + sizeof (data.counter))
		memset (&data, 0, sizeof (data));

	g_get_current_time (&data.timeval);
	data.counter = counter++;

	/* The maximum boundary string length is 69 characters, and a
	 * stringified SHA256 checksum is 64 bytes long.
	 */
	return g_compute_checksum_for_data (G_CHECKSUM_SHA256,
					    (const guchar *)&data,
					    sizeof (data));
}

/**
 * soup_multipart_new:
 * @mime_type: the MIME type of the multipart to create.
 *
 * Creates a new empty #SoupMultipart with a randomly-generated
 * boundary string. Note that @mime_type must be the full MIME type,
 * including "multipart/".
 *
 * Return value: a new empty #SoupMultipart of the given @mime_type
 *
 * Since: 2.26
 **/
SoupMultipart *
soup_multipart_new (const char *mime_type)
{
	return soup_multipart_new_internal (g_strdup (mime_type),
					    generate_boundary ());
}

static const char *
find_boundary (const char *start, const char *end,
	       const char *boundary, int boundary_len)
{
	const char *b;

	for (b = memchr (start, '-', end - start);
	     b && b + boundary_len + 4 < end;
	     b = memchr (b + 2, '-', end - (b + 2))) {
		/* Check for "--boundary" */
		if (b[1] != '-' ||
		    memcmp (b + 2, boundary, boundary_len) != 0)
			continue;

		/* Check that it's at start of line */
		if (!(b == start || (b[-1] == '\n' && b[-2] == '\r')))
			continue;

		/* Check for "--" or "\r\n" after boundary */
		if ((b[boundary_len + 2] == '-' && b[boundary_len + 3] == '-') ||
		    (b[boundary_len + 2] == '\r' && b[boundary_len + 3] == '\n'))
			return b;
	}
	return NULL;
}

/**
 * soup_multipart_new_from_message:
 * @headers: the headers of the HTTP message to parse
 * @body: the body of the HTTP message to parse
 *
 * Parses @headers and @body to form a new #SoupMultipart
 *
 * Return value: a new #SoupMultipart (or %NULL if the message couldn't
 * be parsed or wasn't multipart).
 *
 * Since: 2.26
 **/
SoupMultipart *
soup_multipart_new_from_message (SoupMessageHeaders *headers,
				 SoupMessageBody *body)
{
	SoupMultipart *multipart;
	const char *content_type, *boundary;
	GHashTable *params;
	int boundary_len;
	SoupBuffer *flattened;
	const char *start, *split, *end, *body_end;
	SoupMessageHeaders *part_headers;
	SoupBuffer *part_body;

	content_type = soup_message_headers_get_content_type (headers, &params);
	if (!content_type)
		return NULL;

	boundary = g_hash_table_lookup (params, "boundary");
	if (strncmp (content_type, "multipart/", 10) != 0 || !boundary) {
		g_hash_table_destroy (params);
		return NULL;
	}

	multipart = soup_multipart_new_internal (
		g_strdup (content_type), g_strdup (boundary));
	g_hash_table_destroy (params);

	flattened = soup_message_body_flatten (body);
	body_end = flattened->data + flattened->length;
	boundary = multipart->boundary;
	boundary_len = strlen (boundary);

	/* skip preamble */
	start = find_boundary (flattened->data, body_end,
			       boundary, boundary_len);
	if (!start) {
		soup_multipart_free (multipart);
		soup_buffer_free (flattened);
		return NULL;
	}

	while (start[2 + boundary_len] != '-') {
		end = find_boundary (start + 2 + boundary_len, body_end,
				     boundary, boundary_len);
		if (!end) {
			soup_multipart_free (multipart);
			soup_buffer_free (flattened);
			return NULL;
		}

		split = strstr (start, "\r\n\r\n");
		if (!split || split > end) {
			soup_multipart_free (multipart);
			soup_buffer_free (flattened);
			return NULL;
		}
		split += 4;

		/* @start points to the start of the boundary line
		 * preceding this part, and @split points to the end
		 * of the headers / start of the body.
		 *
		 * We tell soup_headers_parse() to start parsing at
		 * @start, because it skips the first line of the
		 * input anyway (expecting it to be either a
		 * Request-Line or Status-Line).
		 */
		part_headers = soup_message_headers_new (SOUP_MESSAGE_HEADERS_MULTIPART);
		g_ptr_array_add (multipart->headers, part_headers);
		if (!soup_headers_parse (start, split - 2 - start,
					 part_headers)) {
			soup_multipart_free (multipart);
			soup_buffer_free (flattened);
			return NULL;
		}

		/* @split, as previously mentioned, points to the
		 * start of the body, and @end points to the start of
		 * the following boundary line, which is to say 2 bytes
		 * after the end of the body.
		 */
		part_body = soup_buffer_new_subbuffer (flattened,
						       split - flattened->data,
						       end - 2 - split);
		g_ptr_array_add (multipart->bodies, part_body);

		start = end;
	}

	soup_buffer_free (flattened);
	return multipart;
}

/**
 * soup_multipart_get_length:
 * @multipart: a #SoupMultipart
 *
 * Gets the number of body parts in @multipart
 *
 * Return value: the number of body parts in @multipart
 *
 * Since: 2.26
 **/
int
soup_multipart_get_length (SoupMultipart *multipart)
{
	return multipart->bodies->len;
}

/**
 * soup_multipart_get_part:
 * @multipart: a #SoupMultipart
 * @part: the part number to get (counting from 0)
 * @headers: (out) (transfer none): return location for the MIME part
 * headers
 * @body: (out) (transfer none): return location for the MIME part
 * body
 *
 * Gets the indicated body part from @multipart.
 *
 * Return value: %TRUE on success, %FALSE if @part is out of range (in
 * which case @headers and @body won't be set)
 *
 * Since: 2.26
 **/
gboolean
soup_multipart_get_part (SoupMultipart *multipart, int part,
			 SoupMessageHeaders **headers, SoupBuffer **body)
{
	if (part < 0 || part >= multipart->bodies->len)
		return FALSE;
	*headers = multipart->headers->pdata[part];
	*body = multipart->bodies->pdata[part];
	return TRUE;
}

/**
 * soup_multipart_append_part:
 * @multipart: a #SoupMultipart
 * @headers: the MIME part headers
 * @body: the MIME part body
 *
 * Adds a new MIME part to @multipart with the given headers and body.
 * (The multipart will make its own copies of @headers and @body, so
 * you should free your copies if you are not using them for anything
 * else.)
 *
 * Since: 2.26
 **/
void
soup_multipart_append_part (SoupMultipart      *multipart,
			    SoupMessageHeaders *headers,
			    SoupBuffer         *body)
{
	SoupMessageHeaders *headers_copy;
	SoupMessageHeadersIter iter;
	const char *name, *value;

	/* Copying @headers is annoying, but the alternatives seem
	 * worse:
	 *
	 * 1) We don't want to use g_boxed_copy, because
	 *    SoupMessageHeaders actually implements that as just a
	 *    ref, which would be confusing since SoupMessageHeaders
	 *    is mutable and the caller might modify @headers after
	 *    appending it.
	 *
	 * 2) We can't change SoupMessageHeaders to not just do a ref
	 *    from g_boxed_copy, because that would break language
	 *    bindings (which need to be able to hold a ref on
	 *    msg->request_headers, but don't want to duplicate it).
	 *
	 * 3) We don't want to steal the reference to @headers,
	 *    because then we'd have to either also steal the
	 *    reference to @body (which would be inconsistent with
	 *    other SoupBuffer methods), or NOT steal the reference to
	 *    @body, in which case there'd be inconsistency just
	 *    between the two arguments of this method!
	 */
	headers_copy = soup_message_headers_new (SOUP_MESSAGE_HEADERS_MULTIPART);
	soup_message_headers_iter_init (&iter, headers);
	while (soup_message_headers_iter_next (&iter, &name, &value))
		soup_message_headers_append (headers_copy, name, value);

	g_ptr_array_add (multipart->headers, headers_copy);
	g_ptr_array_add (multipart->bodies, soup_buffer_copy (body));
}

/**
 * soup_multipart_append_form_string:
 * @multipart: a multipart (presumably of type "multipart/form-data")
 * @control_name: the name of the control associated with @data
 * @data: the body data
 *
 * Adds a new MIME part containing @data to @multipart, using
 * "Content-Disposition: form-data", as per the HTML forms
 * specification. See soup_form_request_new_from_multipart() for more
 * details.
 *
 * Since: 2.26
 **/ 
void
soup_multipart_append_form_string (SoupMultipart *multipart,
				   const char *control_name, const char *data)
{
	SoupBuffer *body;

	body = soup_buffer_new (SOUP_MEMORY_COPY, data, strlen (data));
	soup_multipart_append_form_file (multipart, control_name,
					 NULL, NULL, body);
	soup_buffer_free (body);
}

/**
 * soup_multipart_append_form_file:
 * @multipart: a multipart (presumably of type "multipart/form-data")
 * @control_name: the name of the control associated with this file
 * @filename: the name of the file, or %NULL if not known
 * @content_type: the MIME type of the file, or %NULL if not known
 * @body: the file data
 *
 * Adds a new MIME part containing @body to @multipart, using
 * "Content-Disposition: form-data", as per the HTML forms
 * specification. See soup_form_request_new_from_multipart() for more
 * details.
 *
 * Since: 2.26
 **/ 
void
soup_multipart_append_form_file (SoupMultipart *multipart,
				 const char *control_name, const char *filename,
				 const char *content_type, SoupBuffer *body)
{
	SoupMessageHeaders *headers;
	GString *disposition;

	headers = soup_message_headers_new (SOUP_MESSAGE_HEADERS_MULTIPART);
	disposition = g_string_new ("form-data; ");
	soup_header_g_string_append_param_quoted (disposition, "name", control_name);
	if (filename) {
		g_string_append (disposition, "; ");
		soup_header_g_string_append_param_quoted (disposition, "filename", filename);
	}
	soup_message_headers_append (headers, "Content-Disposition",
				     disposition->str);
	g_string_free (disposition, TRUE);

	if (content_type) {
		soup_message_headers_append (headers, "Content-Type",
					     content_type);
	}

	g_ptr_array_add (multipart->headers, headers);
	g_ptr_array_add (multipart->bodies, soup_buffer_copy (body));
}

/**
 * soup_multipart_to_message:
 * @multipart: a #SoupMultipart
 * @dest_headers: the headers of the HTTP message to serialize @multipart to
 * @dest_body: the body of the HTTP message to serialize @multipart to
 *
 * Serializes @multipart to @dest_headers and @dest_body.
 *
 * Since: 2.26
 **/
void
soup_multipart_to_message (SoupMultipart *multipart,
			   SoupMessageHeaders *dest_headers,
			   SoupMessageBody *dest_body)
{
	SoupMessageHeaders *part_headers;
	SoupBuffer *part_body;
	SoupMessageHeadersIter iter;
	const char *name, *value;
	GString *str;
	GHashTable *params;
	int i;

	params = g_hash_table_new (g_str_hash, g_str_equal);
	g_hash_table_insert (params, "boundary", multipart->boundary);
	soup_message_headers_set_content_type (dest_headers,
					       multipart->mime_type,
					       params);
	g_hash_table_destroy (params);

	for (i = 0; i < multipart->bodies->len; i++) {
		part_headers = multipart->headers->pdata[i];
		part_body = multipart->bodies->pdata[i];

		str = g_string_new (i == 0 ? NULL : "\r\n");
		g_string_append (str, "--");
		g_string_append (str, multipart->boundary);
		g_string_append (str, "\r\n");
		soup_message_headers_iter_init (&iter, part_headers);
		while (soup_message_headers_iter_next (&iter, &name, &value))
			g_string_append_printf (str, "%s: %s\r\n", name, value);
		g_string_append (str, "\r\n");
		soup_message_body_append (dest_body, SOUP_MEMORY_TAKE,
					  str->str, str->len);
		g_string_free (str, FALSE);

		soup_message_body_append_buffer (dest_body, part_body);
	}

	str = g_string_new ("\r\n--");
	g_string_append (str, multipart->boundary);
	g_string_append (str, "--\r\n");
	soup_message_body_append (dest_body, SOUP_MEMORY_TAKE,
				  str->str, str->len);
	g_string_free (str, FALSE);

	/* (The "\r\n" after the close-delimiter seems wrong according
	 * to my reading of RFCs 2046 and 2616, but that's what
	 * everyone else does.)
	 */
}

/**
 * soup_multipart_free:
 * @multipart: a #SoupMultipart
 *
 * Frees @multipart
 *
 * Since: 2.26
 **/
void
soup_multipart_free (SoupMultipart *multipart)
{
	g_free (multipart->mime_type);
	g_free (multipart->boundary);
	g_ptr_array_free (multipart->headers, TRUE);
	g_ptr_array_free (multipart->bodies, TRUE);

	g_slice_free (SoupMultipart, multipart);
}

static SoupMultipart *
soup_multipart_copy (SoupMultipart *multipart)
{
	SoupMultipart *copy;
	int i;

	copy = soup_multipart_new_internal (g_strdup (multipart->mime_type),
					    g_strdup (multipart->boundary));
	for (i = 0; i < multipart->bodies->len; i++) {
		soup_multipart_append_part (copy,
					    multipart->headers->pdata[i],
					    multipart->bodies->pdata[i]);
	}
	return copy;
}

G_DEFINE_BOXED_TYPE (SoupMultipart, soup_multipart, soup_multipart_copy, soup_multipart_free)
