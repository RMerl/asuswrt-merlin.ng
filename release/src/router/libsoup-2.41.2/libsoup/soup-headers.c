/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-headers.c: HTTP message header parsing
 *
 * Copyright (C) 2001-2003, Ximian, Inc.
 */

#include <stdlib.h>
#include <string.h>

#include "soup-headers.h"
#include "soup.h"

/**
 * soup_headers_parse:
 * @str: the header string (including the Request-Line or Status-Line,
 *   but not the trailing blank line)
 * @len: length of @str
 * @dest: #SoupMessageHeaders to store the header values in
 *
 * Parses the headers of an HTTP request or response in @str and
 * stores the results in @dest. Beware that @dest may be modified even
 * on failure.
 *
 * This is a low-level method; normally you would use
 * soup_headers_parse_request() or soup_headers_parse_response().
 *
 * Return value: success or failure
 *
 * Since: 2.26
 **/
gboolean
soup_headers_parse (const char *str, int len, SoupMessageHeaders *dest)
{
	const char *headers_start;
	char *headers_copy, *name, *name_end, *value, *value_end;
	char *eol, *sol, *p;
	gboolean success = FALSE;

	g_return_val_if_fail (str != NULL, FALSE);
	g_return_val_if_fail (dest != NULL, FALSE);

	/* RFC 2616 does allow NUL bytes in the headers, but httpbis
	 * is changing that, and we can't deal with them anyway.
	 */
	if (memchr (str, '\0', len))
		return FALSE;

	/* As per RFC 2616 section 19.3, we treat '\n' as the
	 * line terminator, and '\r', if it appears, merely as
	 * ignorable trailing whitespace.
	 */

	/* Skip over the Request-Line / Status-Line */
	headers_start = memchr (str, '\n', len);
	if (!headers_start)
		return FALSE;

	/* We work on a copy of the headers, which we can write '\0's
	 * into, so that we don't have to individually g_strndup and
	 * then g_free each header name and value.
	 */
	headers_copy = g_strndup (headers_start, len - (headers_start - str));
	value_end = headers_copy;

	while (*(value_end + 1)) {
		name = value_end + 1;
		name_end = strchr (name, ':');

		/* Reject if there is no ':', or the header name is
		 * empty, or it contains whitespace.
		 */
		if (!name_end ||
		    name_end == name ||
		    name + strcspn (name, " \t\r\n") < name_end) {
			/* Ignore this line. Note that if it has
			 * continuation lines, we'll end up ignoring
			 * them too since they'll start with spaces.
			 */
			value_end = strchr (name, '\n');
			if (!value_end)
				goto done;
			continue;
		}

		/* Find the end of the value; ie, an end-of-line that
		 * isn't followed by a continuation line.
		 */
		value = name_end + 1;
		value_end = strchr (name, '\n');
		if (!value_end)
			goto done;
		while (*(value_end + 1) == ' ' || *(value_end + 1) == '\t') {
			value_end = strchr (value_end + 1, '\n');
			if (!value_end)
				goto done;
		}

		*name_end = '\0';
		*value_end = '\0';

		/* Skip leading whitespace */
		while (value < value_end &&
		       (*value == ' ' || *value == '\t' ||
			*value == '\r' || *value == '\n'))
			value++;

		/* Collapse continuation lines */
		while ((eol = strchr (value, '\n'))) {
			/* find start of next line */
			sol = eol + 1;
			while (*sol == ' ' || *sol == '\t')
				sol++;

			/* back up over trailing whitespace on current line */
			while (eol[-1] == ' ' || eol[-1] == '\t' || eol[-1] == '\r')
				eol--;

			/* Delete all but one SP */
			*eol = ' ';
			g_memmove (eol + 1, sol, strlen (sol) + 1);
		}

		/* clip trailing whitespace */
		eol = strchr (value, '\0');
		while (eol > value &&
		       (eol[-1] == ' ' || eol[-1] == '\t' || eol[-1] == '\r'))
			eol--;
		*eol = '\0';

		/* convert (illegal) '\r's to spaces */
		for (p = strchr (value, '\r'); p; p = strchr (p, '\r'))
			*p = ' ';

		soup_message_headers_append (dest, name, value);
        }
	success = TRUE;

done:
	g_free (headers_copy);
	return success;
}

/**
 * soup_headers_parse_request:
 * @str: the headers (up to, but not including, the trailing blank line)
 * @len: length of @str
 * @req_headers: #SoupMessageHeaders to store the header values in
 * @req_method: (out) (allow-none): if non-%NULL, will be filled in with the
 * request method
 * @req_path: (out) (allow-none): if non-%NULL, will be filled in with the
 * request path
 * @ver: (out) (allow-none): if non-%NULL, will be filled in with the HTTP
 * version
 *
 * Parses the headers of an HTTP request in @str and stores the
 * results in @req_method, @req_path, @ver, and @req_headers.
 *
 * Beware that @req_headers may be modified even on failure.
 *
 * Return value: %SOUP_STATUS_OK if the headers could be parsed, or an
 * HTTP error to be returned to the client if they could not be.
 **/
guint
soup_headers_parse_request (const char          *str, 
			    int                  len, 
			    SoupMessageHeaders  *req_headers,
			    char               **req_method,
			    char               **req_path,
			    SoupHTTPVersion     *ver) 
{
	const char *method, *method_end, *path, *path_end;
	const char *version, *version_end, *headers;
	unsigned long major_version, minor_version;
	char *p;

	g_return_val_if_fail (str != NULL, SOUP_STATUS_MALFORMED);

	/* RFC 2616 4.1 "servers SHOULD ignore any empty line(s)
	 * received where a Request-Line is expected."
	 */
	while ((*str == '\r' || *str == '\n') && len > 0) {
		str++;
		len--;
	}
	if (!len)
		return SOUP_STATUS_BAD_REQUEST;

	/* RFC 2616 19.3 "[servers] SHOULD accept any amount of SP or
	 * HT characters between [Request-Line] fields"
	 */

	method = method_end = str;
	while (method_end < str + len && *method_end != ' ' && *method_end != '\t')
		method_end++;
	if (method_end >= str + len)
		return SOUP_STATUS_BAD_REQUEST;

	path = method_end;
	while (path < str + len && (*path == ' ' || *path == '\t'))
		path++;
	if (path >= str + len)
		return SOUP_STATUS_BAD_REQUEST;

	path_end = path;
	while (path_end < str + len && *path_end != ' ' && *path_end != '\t')
		path_end++;
	if (path_end >= str + len)
		return SOUP_STATUS_BAD_REQUEST;

	version = path_end;
	while (version < str + len && (*version == ' ' || *version == '\t'))
		version++;
	if (version + 8 >= str + len)
		return SOUP_STATUS_BAD_REQUEST;

	if (strncmp (version, "HTTP/", 5) != 0 ||
	    !g_ascii_isdigit (version[5]))
		return SOUP_STATUS_BAD_REQUEST;
	major_version = strtoul (version + 5, &p, 10);
	if (*p != '.' || !g_ascii_isdigit (p[1]))
		return SOUP_STATUS_BAD_REQUEST;
	minor_version = strtoul (p + 1, &p, 10);
	version_end = p;
	if (major_version != 1)
		return SOUP_STATUS_HTTP_VERSION_NOT_SUPPORTED;
	if (minor_version > 1)
		return SOUP_STATUS_HTTP_VERSION_NOT_SUPPORTED;

	headers = version_end;
	while (headers < str + len && (*headers == '\r' || *headers == ' '))
		headers++;
	if (headers >= str + len || *headers != '\n')
		return SOUP_STATUS_BAD_REQUEST;

	if (!soup_headers_parse (str, len, req_headers)) 
		return SOUP_STATUS_BAD_REQUEST;

	if (soup_message_headers_get_expectations (req_headers) &
	    SOUP_EXPECTATION_UNRECOGNIZED)
		return SOUP_STATUS_EXPECTATION_FAILED;
	/* RFC 2616 14.10 */
	if (minor_version == 0)
		soup_message_headers_clean_connection_headers (req_headers);

	if (req_method)
		*req_method = g_strndup (method, method_end - method);
	if (req_path)
		*req_path = g_strndup (path, path_end - path);
	if (ver)
		*ver = (minor_version == 0) ? SOUP_HTTP_1_0 : SOUP_HTTP_1_1;

	return SOUP_STATUS_OK;
}

/**
 * soup_headers_parse_status_line:
 * @status_line: an HTTP Status-Line
 * @ver: (out) (allow-none): if non-%NULL, will be filled in with the HTTP
 * version
 * @status_code: (out) (allow-none): if non-%NULL, will be filled in with
 * the status code
 * @reason_phrase: (out) (allow-none): if non-%NULL, will be filled in with
 * the reason phrase
 *
 * Parses the HTTP Status-Line string in @status_line into @ver,
 * @status_code, and @reason_phrase. @status_line must be terminated by
 * either "\0" or "\r\n".
 *
 * Return value: %TRUE if @status_line was parsed successfully.
 **/
gboolean
soup_headers_parse_status_line (const char       *status_line,
				SoupHTTPVersion  *ver,
				guint            *status_code,
				char            **reason_phrase)
{
	unsigned long major_version, minor_version, code;
	const char *code_start, *code_end, *phrase_start, *phrase_end;
	char *p;

	g_return_val_if_fail (status_line != NULL, FALSE);

	if (strncmp (status_line, "HTTP/", 5) == 0 &&
	    g_ascii_isdigit (status_line[5])) {
		major_version = strtoul (status_line + 5, &p, 10);
		if (*p != '.' || !g_ascii_isdigit (p[1]))
			return FALSE;
		minor_version = strtoul (p + 1, &p, 10);
		if (major_version != 1)
			return FALSE;
		if (minor_version > 1)
			return FALSE;
		if (ver)
			*ver = (minor_version == 0) ? SOUP_HTTP_1_0 : SOUP_HTTP_1_1;
	} else if (!strncmp (status_line, "ICY", 3)) {
		/* Shoutcast not-quite-HTTP format */
		if (ver)
			*ver = SOUP_HTTP_1_0;
		p = (char *)status_line + 3;
	} else
		return FALSE;

	code_start = p;
	while (*code_start == ' ' || *code_start == '\t')
		code_start++;
	code_end = code_start;
	while (*code_end >= '0' && *code_end <= '9')
		code_end++;
	if (code_end != code_start + 3)
		return FALSE;
	code = atoi (code_start);
	if (code < 100 || code > 599)
		return FALSE;
	if (status_code)
		*status_code = code;

	phrase_start = code_end;
	while (*phrase_start == ' ' || *phrase_start == '\t')
		phrase_start++;
	phrase_end = phrase_start + strcspn (phrase_start, "\n");
	while (phrase_end > phrase_start &&
	       (phrase_end[-1] == '\r' || phrase_end[-1] == ' ' || phrase_end[-1] == '\t'))
		phrase_end--;
	if (reason_phrase)
		*reason_phrase = g_strndup (phrase_start, phrase_end - phrase_start);

	return TRUE;
}

/**
 * soup_headers_parse_response:
 * @str: the headers (up to, but not including, the trailing blank line)
 * @len: length of @str
 * @headers: #SoupMessageHeaders to store the header values in
 * @ver: (out) (allow-none): if non-%NULL, will be filled in with the HTTP
 * version
 * @status_code: (out) (allow-none): if non-%NULL, will be filled in with
 * the status code
 * @reason_phrase: (out) (allow-none): if non-%NULL, will be filled in with
 * the reason phrase
 *
 * Parses the headers of an HTTP response in @str and stores the
 * results in @ver, @status_code, @reason_phrase, and @headers.
 *
 * Beware that @headers may be modified even on failure.
 *
 * Return value: success or failure.
 **/
gboolean
soup_headers_parse_response (const char          *str, 
			     int                  len, 
			     SoupMessageHeaders  *headers,
			     SoupHTTPVersion     *ver,
			     guint               *status_code,
			     char               **reason_phrase)
{
	SoupHTTPVersion version;

	g_return_val_if_fail (str != NULL, FALSE);

	/* Workaround for broken servers that send extra line breaks
	 * after a response, which we then see prepended to the next
	 * response on that connection.
	 */
	while ((*str == '\r' || *str == '\n') && len > 0) {
		str++;
		len--;
	}
	if (!len)
		return FALSE;

	if (!soup_headers_parse (str, len, headers)) 
		return FALSE;

	if (!soup_headers_parse_status_line (str, 
					     &version, 
					     status_code, 
					     reason_phrase))
		return FALSE;
	if (ver)
		*ver = version;

	/* RFC 2616 14.10 */
	if (version == SOUP_HTTP_1_0)
		soup_message_headers_clean_connection_headers (headers);

	return TRUE;
}


/*
 * Parsing of specific HTTP header types
 */

static const char *
skip_lws (const char *s)
{
	while (g_ascii_isspace (*s))
		s++;
	return s;
}

static const char *
unskip_lws (const char *s, const char *start)
{
	while (s > start && g_ascii_isspace (*(s - 1)))
		s--;
	return s;
}

static const char *
skip_delims (const char *s, char delim)
{
	/* The grammar allows for multiple delimiters */
	while (g_ascii_isspace (*s) || *s == delim)
		s++;
	return s;
}

static const char *
skip_item (const char *s, char delim)
{
	gboolean quoted = FALSE;
	const char *start = s;

	/* A list item ends at the last non-whitespace character
	 * before a delimiter which is not inside a quoted-string. Or
	 * at the end of the string.
	 */

	while (*s) {
		if (*s == '"')
			quoted = !quoted;
		else if (quoted) {
			if (*s == '\\' && *(s + 1))
				s++;
		} else {
			if (*s == delim)
				break;
		}
		s++;
	}

	return unskip_lws (s, start);
}

static GSList *
parse_list (const char *header, char delim)
{
	GSList *list = NULL;
	const char *end;

	header = skip_delims (header, delim);
	while (*header) {
		end = skip_item (header, delim);
		list = g_slist_prepend (list, g_strndup (header, end - header));
		header = skip_delims (end, delim);
	}

	return g_slist_reverse (list);
}

/**
 * soup_header_parse_list:
 * @header: a header value
 *
 * Parses a header whose content is described by RFC2616 as
 * "#something", where "something" does not itself contain commas,
 * except as part of quoted-strings.
 *
 * Return value: (transfer full) (element-type utf8): a #GSList of
 * list elements, as allocated strings
 **/
GSList *
soup_header_parse_list (const char *header)
{
	g_return_val_if_fail (header != NULL, NULL);

	return parse_list (header, ',');
}

typedef struct {
	char *item;
	double qval;
} QualityItem;

static int
sort_by_qval (const void *a, const void *b)
{
	QualityItem *qia = (QualityItem *)a;
	QualityItem *qib = (QualityItem *)b;

	if (qia->qval == qib->qval)
		return 0;
	else if (qia->qval < qib->qval)
		return 1;
	else
		return -1;
}

/**
 * soup_header_parse_quality_list:
 * @header: a header value
 * @unacceptable: (out) (allow-none) (transfer full) (element-type utf8): on
 * return, will contain a list of unacceptable values
 *
 * Parses a header whose content is a list of items with optional
 * "qvalue"s (eg, Accept, Accept-Charset, Accept-Encoding,
 * Accept-Language, TE).
 *
 * If @unacceptable is not %NULL, then on return, it will contain the
 * items with qvalue 0. Either way, those items will be removed from
 * the main list.
 *
 * Return value: (transfer full) (element-type utf8): a #GSList of
 * acceptable values (as allocated strings), highest-qvalue first.
 **/
GSList *
soup_header_parse_quality_list (const char *header, GSList **unacceptable)
{
	GSList *unsorted;
	QualityItem *array;
	GSList *sorted, *iter;
	char *item, *semi;
	const char *param, *equal, *value;
	double qval;
	int n;

	g_return_val_if_fail (header != NULL, NULL);

	if (unacceptable)
		*unacceptable = NULL;

	unsorted = soup_header_parse_list (header);
	array = g_new0 (QualityItem, g_slist_length (unsorted));
	for (iter = unsorted, n = 0; iter; iter = iter->next) {
		item = iter->data;
		qval = 1.0;
		for (semi = strchr (item, ';'); semi; semi = strchr (semi + 1, ';')) {
			param = skip_lws (semi + 1);
			if (*param != 'q')
				continue;
			equal = skip_lws (param + 1);
			if (!equal || *equal != '=')
				continue;
			value = skip_lws (equal + 1);
			if (!value)
				continue;

			if (value[0] != '0' && value[0] != '1')
				continue;
			qval = (double)(value[0] - '0');
			if (value[0] == '0' && value[1] == '.') {
				if (g_ascii_isdigit (value[2])) {
					qval += (double)(value[2] - '0') / 10;
					if (g_ascii_isdigit (value[3])) {
						qval += (double)(value[3] - '0') / 100;
						if (g_ascii_isdigit (value[4]))
							qval += (double)(value[4] - '0') / 1000;
					}
				}
			}

			*semi = '\0';
			break;
		}

		if (qval == 0.0) {
			if (unacceptable) {
				*unacceptable = g_slist_prepend (*unacceptable,
								 item);
			}
		} else {
			array[n].item = item;
			array[n].qval = qval;
			n++;
		}
	}
	g_slist_free (unsorted);

	qsort (array, n, sizeof (QualityItem), sort_by_qval);
	sorted = NULL;
	while (n--)
		sorted = g_slist_prepend (sorted, array[n].item);
	g_free (array);

	return sorted;
}

/**
 * soup_header_free_list: (skip)
 * @list: a #GSList returned from soup_header_parse_list() or
 * soup_header_parse_quality_list()
 *
 * Frees @list.
 **/
void
soup_header_free_list (GSList *list)
{
	g_slist_free_full (list, g_free);
}

/**
 * soup_header_contains:
 * @header: An HTTP header suitable for parsing with
 * soup_header_parse_list()
 * @token: a token
 *
 * Parses @header to see if it contains the token @token (matched
 * case-insensitively). Note that this can't be used with lists
 * that have qvalues.
 *
 * Return value: whether or not @header contains @token
 **/
gboolean
soup_header_contains (const char *header, const char *token)
{
	const char *end;
	guint len = strlen (token);

	g_return_val_if_fail (header != NULL, FALSE);
	g_return_val_if_fail (token != NULL, FALSE);

	header = skip_delims (header, ',');
	while (*header) {
		end = skip_item (header, ',');
		if (end - header == len &&
		    !g_ascii_strncasecmp (header, token, len))
			return TRUE;
		header = skip_delims (end, ',');
	}

	return FALSE;
}

static void
decode_quoted_string (char *quoted_string)
{
	char *src, *dst;

	src = quoted_string + 1;
	dst = quoted_string;
	while (*src && *src != '"') {
		if (*src == '\\' && *(src + 1))
			src++;
		*dst++ = *src++;
	}
	*dst = '\0';
}

static gboolean
decode_rfc5987 (char *encoded_string)
{
	char *q, *decoded;
	gboolean iso_8859_1 = FALSE;

	q = strchr (encoded_string, '\'');
	if (!q)
		return FALSE;
	if (g_ascii_strncasecmp (encoded_string, "UTF-8",
				 q - encoded_string) == 0)
		;
	else if (g_ascii_strncasecmp (encoded_string, "iso-8859-1",
				      q - encoded_string) == 0)
		iso_8859_1 = TRUE;
	else
		return FALSE;

	q = strchr (q + 1, '\'');
	if (!q)
		return FALSE;

	decoded = soup_uri_decode (q + 1);
	if (iso_8859_1) {
		char *utf8 =  g_convert_with_fallback (decoded, -1, "UTF-8",
						       "iso-8859-1", "_",
						       NULL, NULL, NULL);
		g_free (decoded);
		if (!utf8)
			return FALSE;
		decoded = utf8;
	}

	/* If encoded_string was UTF-8, then each 3-character %-escape
	 * will be converted to a single byte, and so decoded is
	 * shorter than encoded_string. If encoded_string was
	 * iso-8859-1, then each 3-character %-escape will be
	 * converted into at most 2 bytes in UTF-8, and so it's still
	 * shorter.
	 */
	strcpy (encoded_string, decoded);
	g_free (decoded);
	return TRUE;
}

static GHashTable *
parse_param_list (const char *header, char delim)
{
	GHashTable *params;
	GSList *list, *iter;
	char *item, *eq, *name_end, *value;
	gboolean override;

	params = g_hash_table_new_full (soup_str_case_hash, 
					soup_str_case_equal,
					g_free, NULL);

	list = parse_list (header, delim);
	for (iter = list; iter; iter = iter->next) {
		item = iter->data;
		override = FALSE;

		eq = strchr (item, '=');
		if (eq) {
			name_end = (char *)unskip_lws (eq, item);
			if (name_end == item) {
				/* That's no good... */
				g_free (item);
				continue;
			}

			*name_end = '\0';

			value = (char *)skip_lws (eq + 1);

			if (name_end[-1] == '*' && name_end > item + 1) {
				name_end[-1] = '\0';
				if (!decode_rfc5987 (value)) {
					g_free (item);
					continue;
				}
				override = TRUE;
			} else if (*value == '"')
				decode_quoted_string (value);
		} else
			value = NULL;

		if (override || !g_hash_table_lookup (params, item))
			g_hash_table_replace (params, item, value);
		else
			g_free (item);
	}

	g_slist_free (list);
	return params;
}

/**
 * soup_header_parse_param_list:
 * @header: a header value
 *
 * Parses a header which is a comma-delimited list of something like:
 * <literal>token [ "=" ( token | quoted-string ) ]</literal>.
 *
 * Tokens that don't have an associated value will still be added to
 * the resulting hash table, but with a %NULL value.
 * 
 * This also handles RFC5987 encoding (which in HTTP is mostly used
 * for giving UTF8-encoded filenames in the Content-Disposition
 * header).
 *
 * Return value: (element-type utf8 utf8) (transfer full): a
 * #GHashTable of list elements, which can be freed with
 * soup_header_free_param_list().
 **/
GHashTable *
soup_header_parse_param_list (const char *header)
{
	g_return_val_if_fail (header != NULL, NULL);

	return parse_param_list (header, ',');
}

/**
 * soup_header_parse_semi_param_list:
 * @header: a header value
 *
 * Parses a header which is a semicolon-delimited list of something
 * like: <literal>token [ "=" ( token | quoted-string ) ]</literal>.
 *
 * Tokens that don't have an associated value will still be added to
 * the resulting hash table, but with a %NULL value.
 * 
 * This also handles RFC5987 encoding (which in HTTP is mostly used
 * for giving UTF8-encoded filenames in the Content-Disposition
 * header).
 *
 * Return value: (element-type utf8 utf8) (transfer full): a
 * #GHashTable of list elements, which can be freed with
 * soup_header_free_param_list().
 *
 * Since: 2.24
 **/
GHashTable *
soup_header_parse_semi_param_list (const char *header)
{
	g_return_val_if_fail (header != NULL, NULL);

	return parse_param_list (header, ';');
}

/**
 * soup_header_free_param_list:
 * @param_list: (element-type utf8 utf8): a #GHashTable returned from soup_header_parse_param_list()
 * or soup_header_parse_semi_param_list()
 *
 * Frees @param_list.
 **/
void
soup_header_free_param_list (GHashTable *param_list)
{
	g_return_if_fail (param_list != NULL);

	g_hash_table_destroy (param_list);
}

static void
append_param_rfc5987 (GString    *string,
		      const char *name,
		      const char *value)
{
	char *encoded;

	g_string_append (string, name);
	g_string_append (string, "*=UTF-8''");
	encoded = soup_uri_encode (value, " *'%()<>@,;:\\\"/[]?=");
	g_string_append (string, encoded);
	g_free (encoded);
}

static void
append_param_quoted (GString    *string,
		     const char *name,
		     const char *value)
{
	int len;

	g_string_append (string, name);
	g_string_append (string, "=\"");
	while (*value) {
		while (*value == '\\' || *value == '"') {
			g_string_append_c (string, '\\');
			g_string_append_c (string, *value++);
		}
		len = strcspn (value, "\\\"");
		g_string_append_len (string, value, len);
		value += len;
	}
	g_string_append_c (string, '"');
}

static void
append_param_internal (GString    *string,
		       const char *name,
		       const char *value,
		       gboolean    allow_token)
{
	const char *v;
	gboolean use_token = allow_token;

	for (v = value; *v; v++) {
		if (*v & 0x80) {
			if (g_utf8_validate (value, -1, NULL)) {
				append_param_rfc5987 (string, name, value);
				return;
			} else {
				use_token = FALSE;
				break;
			}
		} else if (!soup_char_is_token (*v))
			use_token = FALSE;
	}

	if (use_token) {
		g_string_append (string, name);
		g_string_append_c (string, '=');
		g_string_append (string, value);
	} else
		append_param_quoted (string, name, value);
}

/**
 * soup_header_g_string_append_param_quoted:
 * @string: a #GString being used to construct an HTTP header value
 * @name: a parameter name
 * @value: a parameter value
 *
 * Appends something like <literal>@name="@value"</literal> to
 * @string, taking care to escape any quotes or backslashes in @value.
 *
 * If @value is (non-ASCII) UTF-8, this will instead use RFC 5987
 * encoding, just like soup_header_g_string_append_param().
 *
 * Since: 2.30
 **/
void
soup_header_g_string_append_param_quoted (GString    *string,
					  const char *name,
					  const char *value)
{
	g_return_if_fail (string != NULL);
	g_return_if_fail (name != NULL);
	g_return_if_fail (value != NULL);

	append_param_internal (string, name, value, FALSE);
}

/**
 * soup_header_g_string_append_param:
 * @string: a #GString being used to construct an HTTP header value
 * @name: a parameter name
 * @value: a parameter value, or %NULL
 *
 * Appends something like <literal>@name=@value</literal> to @string,
 * taking care to quote @value if needed, and if so, to escape any
 * quotes or backslashes in @value.
 *
 * Alternatively, if @value is a non-ASCII UTF-8 string, it will be
 * appended using RFC5987 syntax. Although in theory this is supposed
 * to work anywhere in HTTP that uses this style of parameter, in
 * reality, it can only be used portably with the Content-Disposition
 * "filename" parameter.
 *
 * If @value is %NULL, this will just append @name to @string.
 *
 * Since: 2.26
 **/
void
soup_header_g_string_append_param (GString    *string,
				   const char *name,
				   const char *value)
{
	g_return_if_fail (string != NULL);
	g_return_if_fail (name != NULL);

	if (!value) {
		g_string_append (string, name);
		return;
	}

	append_param_internal (string, name, value, TRUE);
}
