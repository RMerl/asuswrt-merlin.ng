/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* soup-form.c : utility functions for HTML forms */

/*
 * Copyright 2008 Red Hat, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "soup-form.h"
#include "soup.h"

/**
 * SECTION:soup-form
 * @short_description: HTML form handling
 * @see_also: #SoupMultipart
 *
 * libsoup contains several help methods for processing HTML forms as
 * defined by <ulink
 * url="http://www.w3.org/TR/html401/interact/forms.html#h-17.13">the
 * HTML 4.01 specification</ulink>.
 **/

/**
 * SOUP_FORM_MIME_TYPE_URLENCODED:
 *
 * A macro containing the value
 * <literal>"application/x-www-form-urlencoded"</literal>; the default
 * MIME type for POSTing HTML form data.
 *
 * Since: 2.26
 **/

/**
 * SOUP_FORM_MIME_TYPE_MULTIPART:
 *
 * A macro containing the value
 * <literal>"multipart/form-data"</literal>; the MIME type used for
 * posting form data that contains files to be uploaded.
 *
 * Since: 2.26
 **/

#define XDIGIT(c) ((c) <= '9' ? (c) - '0' : ((c) & 0x4F) - 'A' + 10)
#define HEXCHAR(s) ((XDIGIT (s[1]) << 4) + XDIGIT (s[2]))

static gboolean
form_decode (char *part)
{
	unsigned char *s, *d;

	s = d = (unsigned char *)part;
	do {
		if (*s == '%') {
			if (!g_ascii_isxdigit (s[1]) ||
			    !g_ascii_isxdigit (s[2]))
				return FALSE;
			*d++ = HEXCHAR (s);
			s += 2;
		} else if (*s == '+')
			*d++ = ' ';
		else
			*d++ = *s;
	} while (*s++);

	return TRUE;
}

/**
 * soup_form_decode:
 * @encoded_form: data of type "application/x-www-form-urlencoded"
 *
 * Decodes @form, which is an urlencoded dataset as defined in the
 * HTML 4.01 spec.
 *
 * Return value: (element-type utf8 utf8) (transfer full): a hash
 * table containing the name/value pairs from @encoded_form, which you
 * can free with g_hash_table_destroy().
 **/
GHashTable *
soup_form_decode (const char *encoded_form)
{
	GHashTable *form_data_set;
	char **pairs, *eq, *name, *value;
	int i;

	form_data_set = g_hash_table_new_full (g_str_hash, g_str_equal,
					       g_free, NULL);
	pairs = g_strsplit (encoded_form, "&", -1);
	for (i = 0; pairs[i]; i++) {
		name = pairs[i];
		eq = strchr (name, '=');
		if (eq) {
			*eq = '\0';
			value = eq + 1;
		} else
			value = NULL;
		if (!value || !form_decode (name) || !form_decode (value)) {
			g_free (name);
			continue;
		}

		g_hash_table_replace (form_data_set, name, value);
	}
	g_free (pairs);

	return form_data_set;
}

/**
 * soup_form_decode_multipart:
 * @msg: a #SoupMessage containing a "multipart/form-data" request body
 * @file_control_name: (allow-none): the name of the HTML file upload control, or %NULL
 * @filename: (out) (allow-none): return location for the name of the uploaded file, or %NULL
 * @content_type: (out) (allow-none): return location for the MIME type of the uploaded file, or %NULL
 * @file: (out) (allow-none): return location for the uploaded file data, or %NULL
 *
 * Decodes the "multipart/form-data" request in @msg; this is a
 * convenience method for the case when you have a single file upload
 * control in a form. (Or when you don't have any file upload
 * controls, but are still using "multipart/form-data" anyway.) Pass
 * the name of the file upload control in @file_control_name, and
 * soup_form_decode_multipart() will extract the uploaded file data
 * into @filename, @content_type, and @file. All of the other form
 * control data will be returned (as strings, as with
 * soup_form_decode()) in the returned #GHashTable.
 *
 * You may pass %NULL for @filename, @content_type and/or @file if you do not
 * care about those fields. soup_form_decode_multipart() may also
 * return %NULL in those fields if the client did not provide that
 * information. You must free the returned filename and content-type
 * with g_free(), and the returned file data with soup_buffer_free().
 *
 * If you have a form with more than one file upload control, you will
 * need to decode it manually, using soup_multipart_new_from_message()
 * and soup_multipart_get_part().
 *
 * Return value: (element-type utf8 utf8) (transfer full): a hash
 * table containing the name/value pairs (other than
 * @file_control_name) from @msg, which you can free with
 * g_hash_table_destroy(). On error, it will return %NULL.
 *
 * Since: 2.26
 **/
GHashTable *
soup_form_decode_multipart (SoupMessage *msg, const char *file_control_name,
			    char **filename, char **content_type,
			    SoupBuffer **file)
{
	SoupMultipart *multipart;
	GHashTable *form_data_set, *params;
	SoupMessageHeaders *part_headers;
	SoupBuffer *part_body;
	char *disposition, *name;
	int i;

	g_return_val_if_fail (SOUP_IS_MESSAGE (msg), NULL);

	multipart = soup_multipart_new_from_message (msg->request_headers,
						     msg->request_body);
	if (!multipart)
		return NULL;

	if (filename)
		*filename = NULL;
	if (content_type)
		*content_type = NULL;
	if (file)
		*file = NULL;

	form_data_set = g_hash_table_new_full (g_str_hash, g_str_equal,
					       g_free, g_free);
	for (i = 0; i < soup_multipart_get_length (multipart); i++) {
		soup_multipart_get_part (multipart, i, &part_headers, &part_body);
		if (!soup_message_headers_get_content_disposition (
			    part_headers, &disposition, &params))
			continue;
		name = g_hash_table_lookup (params, "name");
		if (g_ascii_strcasecmp (disposition, "form-data") != 0 ||
		    !name) {
			g_free (disposition);
			g_hash_table_destroy (params);
			continue;
		}

		if (file_control_name && !strcmp (name, file_control_name)) {
			if (filename)
				*filename = g_strdup (g_hash_table_lookup (params, "filename"));
			if (content_type)
				*content_type = g_strdup (soup_message_headers_get_content_type (part_headers, NULL));
			if (file)
				*file = soup_buffer_copy (part_body);
		} else {
			g_hash_table_insert (form_data_set,
					     g_strdup (name),
					     g_strndup (part_body->data,
							part_body->length));
		}

		g_free (disposition);
		g_hash_table_destroy (params);
	}

	soup_multipart_free (multipart);
	return form_data_set;
}

static void
append_form_encoded (GString *str, const char *in)
{
	const unsigned char *s = (const unsigned char *)in;

	while (*s) {
		if (*s == ' ') {
			g_string_append_c (str, '+');
			s++;
		} else if (!g_ascii_isalnum (*s))
			g_string_append_printf (str, "%%%02X", (int)*s++);
		else
			g_string_append_c (str, *s++);
	}
}

static void
encode_pair (GString *str, const char *name, const char *value)
{
	g_return_if_fail (name != NULL);
	g_return_if_fail (value != NULL);

	if (str->len)
		g_string_append_c (str, '&');
	append_form_encoded (str, name);
	g_string_append_c (str, '=');
	append_form_encoded (str, value);
}

/**
 * soup_form_encode:
 * @first_field: name of the first form field
 * @...: value of @first_field, followed by additional field names
 * and values, terminated by %NULL.
 *
 * Encodes the given field names and values into a value of type
 * "application/x-www-form-urlencoded", as defined in the HTML 4.01
 * spec.
 *
 * This method requires you to know the names of the form fields (or
 * at the very least, the total number of fields) at compile time; for
 * working with dynamic forms, use soup_form_encode_hash() or
 * soup_form_encode_datalist().
 *
 * Return value: the encoded form
 **/
char *
soup_form_encode (const char *first_field, ...)
{
	va_list args;
	char *encoded;

	va_start (args, first_field);
	encoded = soup_form_encode_valist (first_field, args);
	va_end (args);

	return encoded;
}

/**
 * soup_form_encode_hash:
 * @form_data_set: (element-type utf8 utf8): a hash table containing
 * name/value pairs (as strings)
 *
 * Encodes @form_data_set into a value of type
 * "application/x-www-form-urlencoded", as defined in the HTML 4.01
 * spec.
 *
 * Note that the HTML spec states that "The control names/values are
 * listed in the order they appear in the document." Since this method
 * takes a hash table, it cannot enforce that; if you care about the
 * ordering of the form fields, use soup_form_encode_datalist().
 *
 * Return value: the encoded form
 **/
char *
soup_form_encode_hash (GHashTable *form_data_set)
{
	GString *str = g_string_new (NULL);
	GHashTableIter iter;
	gpointer name, value;

	g_hash_table_iter_init (&iter, form_data_set);
	while (g_hash_table_iter_next (&iter, &name, &value))
		encode_pair (str, name, value);
	return g_string_free (str, FALSE);
}

static void
datalist_encode_foreach (GQuark key_id, gpointer value, gpointer str)
{
	encode_pair (str, g_quark_to_string (key_id), value);
}

/**
 * soup_form_encode_datalist:
 * @form_data_set: a datalist containing name/value pairs
 *
 * Encodes @form_data_set into a value of type
 * "application/x-www-form-urlencoded", as defined in the HTML 4.01
 * spec. Unlike soup_form_encode_hash(), this preserves the ordering
 * of the form elements, which may be required in some situations.
 *
 * Return value: the encoded form
 **/
char *
soup_form_encode_datalist (GData **form_data_set)
{
	GString *str = g_string_new (NULL);

	g_datalist_foreach (form_data_set, datalist_encode_foreach, str);
	return g_string_free (str, FALSE);
}

/**
 * soup_form_encode_valist:
 * @first_field: name of the first form field
 * @args: pointer to additional values, as in soup_form_encode()
 *
 * See soup_form_encode(). This is mostly an internal method, used by
 * various other methods such as soup_uri_set_query_from_fields() and
 * soup_form_request_new().
 *
 * Return value: the encoded form
 **/
char *
soup_form_encode_valist (const char *first_field, va_list args)
{
	GString *str = g_string_new (NULL);
	const char *name, *value;

	name = first_field;
	value = va_arg (args, const char *);
	while (name && value) {
		encode_pair (str, name, value);

		name = va_arg (args, const char *);
		if (name)
			value = va_arg (args, const char *);
	}

	return g_string_free (str, FALSE);
}

static SoupMessage *
soup_form_request_for_data (const char *method, const char *uri_string,
			    char *form_data)
{
	SoupMessage *msg;
	SoupURI *uri;

	uri = soup_uri_new (uri_string);
	if (!uri)
		return NULL;

	if (!strcmp (method, "GET")) {
		g_free (uri->query);
		uri->query = form_data;

		msg = soup_message_new_from_uri (method, uri);
	} else if (!strcmp (method, "POST") || !strcmp (method, "PUT")) {
		msg = soup_message_new_from_uri (method, uri);

		soup_message_set_request (
			msg, SOUP_FORM_MIME_TYPE_URLENCODED,
			SOUP_MEMORY_TAKE,
			form_data, strlen (form_data));
	} else {
		g_warning ("invalid method passed to soup_form_request_new");
		g_free (form_data);

		/* Don't crash */
		msg = soup_message_new_from_uri (method, uri);
	}
	soup_uri_free (uri);

	return msg;
}

/**
 * soup_form_request_new:
 * @method: the HTTP method, either "GET" or "POST"
 * @uri: the URI to send the form data to
 * @first_field: name of the first form field
 * @...: value of @first_field, followed by additional field names
 * and values, terminated by %NULL.
 *
 * Creates a new %SoupMessage and sets it up to send the given data
 * to @uri via @method. (That is, if @method is "GET", it will encode
 * the form data into @uri's query field, and if @method is "POST", it
 * will encode it into the %SoupMessage's request_body.)
 *
 * Return value: (transfer full): the new %SoupMessage
 **/
SoupMessage *
soup_form_request_new (const char *method, const char *uri,
		       const char  *first_field, ...)
{
	va_list args;
	char *form_data;

	va_start (args, first_field);
	form_data = soup_form_encode_valist (first_field, args);
	va_end (args);

	return soup_form_request_for_data (method, uri, form_data);
}

/**
 * soup_form_request_new_from_hash:
 * @method: the HTTP method, either "GET" or "POST"
 * @uri: the URI to send the form data to
 * @form_data_set: (element-type utf8 utf8): the data to send to @uri
 *
 * Creates a new %SoupMessage and sets it up to send @form_data_set to
 * @uri via @method, as with soup_form_request_new().
 *
 * Return value: (transfer full): the new %SoupMessage
 **/
SoupMessage *
soup_form_request_new_from_hash (const char *method, const char *uri,
				 GHashTable *form_data_set)
{
	return soup_form_request_for_data (
		method, uri, soup_form_encode_hash (form_data_set));
}

/**
 * soup_form_request_new_from_datalist:
 * @method: the HTTP method, either "GET" or "POST"
 * @uri: the URI to send the form data to
 * @form_data_set: the data to send to @uri
 *
 * Creates a new %SoupMessage and sets it up to send @form_data_set to
 * @uri via @method, as with soup_form_request_new().
 *
 * Return value: (transfer full): the new %SoupMessage
 **/
SoupMessage *
soup_form_request_new_from_datalist (const char *method, const char *uri,
				     GData **form_data_set)
{
	return soup_form_request_for_data (
		method, uri, soup_form_encode_datalist (form_data_set));
}

/**
 * soup_form_request_new_from_multipart:
 * @uri: the URI to send the form data to
 * @multipart: a "multipart/form-data" #SoupMultipart
 *
 * Creates a new %SoupMessage and sets it up to send @multipart to
 * @uri via POST.
 *
 * To send a <literal>"multipart/form-data"</literal> POST, first
 * create a #SoupMultipart, using %SOUP_FORM_MIME_TYPE_MULTIPART as
 * the MIME type. Then use soup_multipart_append_form_string() and
 * soup_multipart_append_form_file() to add the value of each form
 * control to the multipart. (These are just convenience methods, and
 * you can use soup_multipart_append_part() if you need greater
 * control over the part headers.) Finally, call
 * soup_form_request_new_from_multipart() to serialize the multipart
 * structure and create a #SoupMessage.
 *
 * Return value: (transfer full): the new %SoupMessage
 *
 * Since: 2.26
 **/
SoupMessage *
soup_form_request_new_from_multipart (const char *uri,
				      SoupMultipart *multipart)
{
	SoupMessage *msg;

	msg = soup_message_new ("POST", uri);
	soup_multipart_to_message (multipart, msg->request_headers,
				   msg->request_body);
	return msg;
}
