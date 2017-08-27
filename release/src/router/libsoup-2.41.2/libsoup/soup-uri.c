/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* soup-uri.c : utility functions to parse URLs */

/*
 * Copyright 1999-2003 Ximian, Inc.
 */

#include <string.h>
#include <stdlib.h>

#include "soup-uri.h"
#include "soup.h"
#include "soup-misc-private.h"

/**
 * SECTION:soup-uri
 * @short_description: URIs
 *
 * A #SoupURI represents a (parsed) URI.
 *
 * Many applications will not need to use #SoupURI directly at all; on
 * the client side, soup_message_new() takes a stringified URI, and on
 * the server side, the path and query components are provided for you
 * in the server callback.
 **/

/**
 * SoupURI:
 * @scheme: the URI scheme (eg, "http")
 * @user: a username, or %NULL
 * @password: a password, or %NULL
 * @host: the hostname or IP address
 * @port: the port number on @host
 * @path: the path on @host
 * @query: a query for @path, or %NULL
 * @fragment: a fragment identifier within @path, or %NULL
 *
 * A #SoupURI represents a (parsed) URI. #SoupURI supports RFC 3986
 * (URI Generic Syntax), and can parse any valid URI. However, libsoup
 * only uses "http" and "https" URIs internally; You can use
 * SOUP_URI_VALID_FOR_HTTP() to test if a #SoupURI is a valid HTTP
 * URI.
 *
 * @scheme will always be set in any URI. It is an interned string and
 * is always all lowercase. (If you parse a URI with a non-lowercase
 * scheme, it will be converted to lowercase.) The macros
 * %SOUP_URI_SCHEME_HTTP and %SOUP_URI_SCHEME_HTTPS provide the
 * interned values for "http" and "https" and can be compared against
 * URI @scheme values.
 *
 * @user and @password are parsed as defined in the older URI specs
 * (ie, separated by a colon; RFC 3986 only talks about a single
 * "userinfo" field). Note that @password is not included in the
 * output of soup_uri_to_string(). libsoup does not normally use these
 * fields; authentication is handled via #SoupSession signals.
 *
 * @host contains the hostname, and @port the port specified in the
 * URI. If the URI doesn't contain a hostname, @host will be %NULL,
 * and if it doesn't specify a port, @port may be 0. However, for
 * "http" and "https" URIs, @host is guaranteed to be non-%NULL
 * (trying to parse an http URI with no @host will return %NULL), and
 * @port will always be non-0 (because libsoup knows the default value
 * to use when it is not specified in the URI).
 *
 * @path is always non-%NULL. For http/https URIs, @path will never be
 * an empty string either; if the input URI has no path, the parsed
 * #SoupURI will have a @path of "/".
 *
 * @query and @fragment are optional for all URI types.
 * soup_form_decode() may be useful for parsing @query.
 *
 * Note that @path, @query, and @fragment may contain
 * %<!-- -->-encoded characters. soup_uri_new() calls
 * soup_uri_normalize() on them, but not soup_uri_decode(). This is
 * necessary to ensure that soup_uri_to_string() will generate a URI
 * that has exactly the same meaning as the original. (In theory,
 * #SoupURI should leave @user, @password, and @host partially-encoded
 * as well, but this would be more annoying than useful.)
 **/

/**
 * SOUP_URI_IS_VALID:
 * @uri: a #SoupURI
 *
 * Tests whether @uri is a valid #SoupURI; that is, that it is non-%NULL
 * and its @scheme and @path members are also non-%NULL.
 *
 * This macro does not check whether http and https URIs have a non-%NULL
 * @host member.
 *
 * Return value: %TRUE if @uri is valid for use.
 *
 * Since: 2.38
 **/

/**
 * SOUP_URI_VALID_FOR_HTTP:
 * @uri: a #SoupURI
 *
 * Tests if @uri is a valid #SoupURI for HTTP communication; that is, if
 * it can be used to construct a #SoupMessage.
 *
 * Return value: %TRUE if @uri is a valid "http" or "https" URI.
 *
 * Since: 2.24
 **/

static void append_uri_encoded (GString *str, const char *in, const char *extra_enc_chars);
static char *uri_normalized_copy (const char *str, int length, const char *unescape_extra);

gpointer _SOUP_URI_SCHEME_HTTP, _SOUP_URI_SCHEME_HTTPS;
gpointer _SOUP_URI_SCHEME_FTP;
gpointer _SOUP_URI_SCHEME_FILE, _SOUP_URI_SCHEME_DATA, _SOUP_URI_SCHEME_RESOURCE;

static inline const char *
soup_uri_parse_scheme (const char *scheme, int len)
{
	if (len == 4 && !g_ascii_strncasecmp (scheme, "http", len)) {
		return SOUP_URI_SCHEME_HTTP;
	} else if (len == 5 && !g_ascii_strncasecmp (scheme, "https", len)) {
		return SOUP_URI_SCHEME_HTTPS;
	} else if (len == 8 && !g_ascii_strncasecmp (scheme, "resource", len)) {
		return SOUP_URI_SCHEME_RESOURCE;
	} else {
		char *lower_scheme;

		lower_scheme = g_ascii_strdown (scheme, len);
		scheme = g_intern_static_string (lower_scheme);
		if (scheme != (const char *)lower_scheme)
			g_free (lower_scheme);
		return scheme;
	}
}

static inline guint
soup_scheme_default_port (const char *scheme)
{
	if (scheme == SOUP_URI_SCHEME_HTTP)
		return 80;
	else if (scheme == SOUP_URI_SCHEME_HTTPS)
		return 443;
	else if (scheme == SOUP_URI_SCHEME_FTP)
		return 21;
	else
		return 0;
}

/**
 * soup_uri_new_with_base:
 * @base: a base URI
 * @uri_string: the URI
 *
 * Parses @uri_string relative to @base.
 *
 * Return value: a parsed #SoupURI.
 **/
SoupURI *
soup_uri_new_with_base (SoupURI *base, const char *uri_string)
{
	SoupURI *uri, fixed_base;
	const char *end, *hash, *colon, *at, *path, *question;
	const char *p, *hostend;
	gboolean remove_dot_segments = TRUE;
	int len;

	g_return_val_if_fail (uri_string != NULL, NULL);

	/* Allow a %NULL path in @base, for compatibility */
	if (base && base->scheme && !base->path) {
		g_warn_if_fail (SOUP_URI_IS_VALID (base));

		memcpy (&fixed_base, base, sizeof (SoupURI));
		fixed_base.path = "";
		base = &fixed_base;
	}

	g_return_val_if_fail (base == NULL || SOUP_URI_IS_VALID (base), NULL);

	/* First some cleanup steps (which are supposed to all be no-ops,
	 * but...). Skip initial whitespace, strip out internal tabs and
	 * line breaks, and ignore trailing whitespace.
	 */
	while (g_ascii_isspace (*uri_string))
		uri_string++;

	len = strcspn (uri_string, "\t\n\r");
	if (uri_string[len]) {
		char *clean = g_malloc (strlen (uri_string) + 1), *d;
		const char *s;

		for (s = uri_string, d = clean; *s; s++) {
			if (*s != '\t' && *s != '\n' && *s != '\r')
				*d++ = *s;
		}
		*d = '\0';

		uri = soup_uri_new_with_base (base, clean);
		g_free (clean);
		return uri;
	}
	end = uri_string + len;
	while (end > uri_string && g_ascii_isspace (end[-1]))
		end--;

	uri = g_slice_new0 (SoupURI);

	/* Find fragment. */
	hash = strchr (uri_string, '#');
	if (hash) {
		uri->fragment = uri_normalized_copy (hash + 1, end - hash + 1,
						     NULL);
		end = hash;
	}

	/* Find scheme: initial [a-z+.-]* substring until ":" */
	p = uri_string;
	while (p < end && (g_ascii_isalpha (*p) ||
			   *p == '.' || *p == '+' || *p == '-'))
		p++;

	if (p > uri_string && *p == ':') {
		uri->scheme = soup_uri_parse_scheme (uri_string, p - uri_string);
		uri_string = p + 1;
	}

	if (uri_string == end && !base && !uri->fragment) {
		uri->path = g_strdup ("");
		return uri;
        }

	/* Check for authority */
	if (strncmp (uri_string, "//", 2) == 0) {
		uri_string += 2;

		path = uri_string + strcspn (uri_string, "/?#");
		if (path > end)
			path = end;
		at = strchr (uri_string, '@');
		if (at && at < path) {
			colon = strchr (uri_string, ':');
			if (colon && colon < at) {
				uri->password = uri_decoded_copy (colon + 1,
								  at - colon - 1);
			} else {
				uri->password = NULL;
				colon = at;
			}

			uri->user = uri_decoded_copy (uri_string,
						      colon - uri_string);
			uri_string = at + 1;
		} else
			uri->user = uri->password = NULL;

		/* Find host and port. */
		if (*uri_string == '[') {
			uri_string++;
			hostend = strchr (uri_string, ']');
			if (!hostend || hostend > path) {
				soup_uri_free (uri);
				return NULL;
			}
			if (*(hostend + 1) == ':')
				colon = hostend + 1;
			else
				colon = NULL;
		} else {
			colon = memchr (uri_string, ':', path - uri_string);
			hostend = colon ? colon : path;
		}

		uri->host = uri_decoded_copy (uri_string, hostend - uri_string);

		if (colon && colon != path - 1) {
			char *portend;
			uri->port = strtoul (colon + 1, &portend, 10);
			if (portend != (char *)path) {
				soup_uri_free (uri);
				return NULL;
			}
		}

		uri_string = path;
	}

	/* Find query */
	question = memchr (uri_string, '?', end - uri_string);
	if (question) {
		uri->query = uri_normalized_copy (question + 1,
						  end - (question + 1),
						  NULL);
		end = question;
	}

	if (end != uri_string) {
		uri->path = uri_normalized_copy (uri_string, end - uri_string,
						 NULL);
	}

	/* Apply base URI. This is spelled out in RFC 3986. */
	if (base && !uri->scheme && uri->host)
		uri->scheme = base->scheme;
	else if (base && !uri->scheme) {
		uri->scheme = base->scheme;
		uri->user = g_strdup (base->user);
		uri->password = g_strdup (base->password);
		uri->host = g_strdup (base->host);
		uri->port = base->port;

		if (!uri->path) {
			uri->path = g_strdup (base->path);
			if (!uri->query)
				uri->query = g_strdup (base->query);
			remove_dot_segments = FALSE;
		} else if (*uri->path != '/') {
			char *newpath, *last;

			last = strrchr (base->path, '/');
			if (last) {
				newpath = g_strdup_printf ("%.*s%s",
							   (int)(last + 1 - base->path),
							   base->path,
							   uri->path);
			} else
				newpath = g_strdup_printf ("/%s", uri->path);

			g_free (uri->path);
			uri->path = newpath;
		}
	}

	if (remove_dot_segments && uri->path && *uri->path) {
		char *p, *q;

		/* Remove "./" where "." is a complete segment. */
		for (p = uri->path + 1; *p; ) {
			if (*(p - 1) == '/' &&
			    *p == '.' && *(p + 1) == '/')
				memmove (p, p + 2, strlen (p + 2) + 1);
			else
				p++;
		}
		/* Remove "." at end. */
		if (p > uri->path + 2 &&
		    *(p - 1) == '.' && *(p - 2) == '/')
			*(p - 1) = '\0';

		/* Remove "<segment>/../" where <segment> != ".." */
		for (p = uri->path + 1; *p; ) {
			if (!strncmp (p, "../", 3)) {
				p += 3;
				continue;
			}
			q = strchr (p + 1, '/');
			if (!q)
				break;
			if (strncmp (q, "/../", 4) != 0) {
				p = q + 1;
				continue;
			}
			memmove (p, q + 4, strlen (q + 4) + 1);
			p = uri->path + 1;
		}
		/* Remove "<segment>/.." at end where <segment> != ".." */
		q = strrchr (uri->path, '/');
		if (q && !strcmp (q, "/..")) {
			p = q - 1;
			while (p > uri->path && *p != '/')
				p--;
			if (strncmp (p, "/../", 4) != 0)
				*(p + 1) = 0;
		}

		/* Remove extraneous initial "/.."s */
		while (!strncmp (uri->path, "/../", 4))
			memmove (uri->path, uri->path + 3, strlen (uri->path) - 2);
		if (!strcmp (uri->path, "/.."))
			uri->path[1] = '\0';
	}

	/* HTTP-specific stuff */
	if (uri->scheme == SOUP_URI_SCHEME_HTTP ||
	    uri->scheme == SOUP_URI_SCHEME_HTTPS) {
		if (!uri->path)
			uri->path = g_strdup ("/");
		if (!SOUP_URI_VALID_FOR_HTTP (uri)) {
			soup_uri_free (uri);
			return NULL;
		}
	}

	if (uri->scheme == SOUP_URI_SCHEME_FTP) {
		if (!uri->host) {
			soup_uri_free (uri);
			return NULL;
		}
	}

	if (!uri->port)
		uri->port = soup_scheme_default_port (uri->scheme);
	if (!uri->path)
		uri->path = g_strdup ("");

	return uri;
}

/**
 * soup_uri_new:
 * @uri_string: (allow-none): a URI
 *
 * Parses an absolute URI.
 *
 * You can also pass %NULL for @uri_string if you want to get back an
 * "empty" #SoupURI that you can fill in by hand. (You will need to
 * call at least soup_uri_set_scheme() and soup_uri_set_path(), since
 * those fields are required.)
 *
 * Return value: a #SoupURI, or %NULL if the given string was found to be
 *  invalid.
 **/
SoupURI *
soup_uri_new (const char *uri_string)
{
	SoupURI *uri;

	if (!uri_string)
		return g_slice_new0 (SoupURI);

	uri = soup_uri_new_with_base (NULL, uri_string);
	if (!uri)
		return NULL;
	if (!SOUP_URI_IS_VALID (uri)) {
		soup_uri_free (uri);
		return NULL;
	}

	return uri;
}


/**
 * soup_uri_to_string:
 * @uri: a #SoupURI
 * @just_path_and_query: if %TRUE, output just the path and query portions
 *
 * Returns a string representing @uri.
 *
 * If @just_path_and_query is %TRUE, this concatenates the path and query
 * together. That is, it constructs the string that would be needed in
 * the Request-Line of an HTTP request for @uri.
 *
 * Return value: a string representing @uri, which the caller must free.
 **/
char *
soup_uri_to_string (SoupURI *uri, gboolean just_path_and_query)
{
	GString *str;
	char *return_result;

	g_return_val_if_fail (uri != NULL, NULL);
	g_warn_if_fail (SOUP_URI_IS_VALID (uri));

	str = g_string_sized_new (20);

	if (uri->scheme && !just_path_and_query)
		g_string_append_printf (str, "%s:", uri->scheme);
	if (uri->host && !just_path_and_query) {
		g_string_append (str, "//");
		if (uri->user) {
			append_uri_encoded (str, uri->user, ":;@?/");
			g_string_append_c (str, '@');
		}
		if (strchr (uri->host, ':')) {
			g_string_append_c (str, '[');
			g_string_append (str, uri->host);
			g_string_append_c (str, ']');
		} else
			append_uri_encoded (str, uri->host, ":/");
		if (uri->port && uri->port != soup_scheme_default_port (uri->scheme))
			g_string_append_printf (str, ":%u", uri->port);
		if (!uri->path && (uri->query || uri->fragment))
			g_string_append_c (str, '/');
		else if ((!uri->path || !*uri->path) &&
			 (uri->scheme == SOUP_URI_SCHEME_HTTP ||
			  uri->scheme == SOUP_URI_SCHEME_HTTPS))
			g_string_append_c (str, '/');
	}

	if (uri->path && *uri->path)
		g_string_append (str, uri->path);
	else if (just_path_and_query)
		g_string_append_c (str, '/');

	if (uri->query) {
		g_string_append_c (str, '?');
		g_string_append (str, uri->query);
	}
	if (uri->fragment && !just_path_and_query) {
		g_string_append_c (str, '#');
		g_string_append (str, uri->fragment);
	}

	return_result = str->str;
	g_string_free (str, FALSE);

	return return_result;
}

/**
 * soup_uri_copy:
 * @uri: a #SoupURI
 *
 * Copies @uri
 *
 * Return value: a copy of @uri, which must be freed with soup_uri_free()
 **/
SoupURI *
soup_uri_copy (SoupURI *uri)
{
	SoupURI *dup;

	g_return_val_if_fail (uri != NULL, NULL);
	g_warn_if_fail (SOUP_URI_IS_VALID (uri));

	dup = g_slice_new0 (SoupURI);
	dup->scheme   = uri->scheme;
	dup->user     = g_strdup (uri->user);
	dup->password = g_strdup (uri->password);
	dup->host     = g_strdup (uri->host);
	dup->port     = uri->port;
	dup->path     = g_strdup (uri->path);
	dup->query    = g_strdup (uri->query);
	dup->fragment = g_strdup (uri->fragment);

	return dup;
}

static inline gboolean
parts_equal (const char *one, const char *two, gboolean insensitive)
{
	if (!one && !two)
		return TRUE;
	if (!one || !two)
		return FALSE;
	return insensitive ? !g_ascii_strcasecmp (one, two) : !strcmp (one, two);
}

/**
 * soup_uri_equal:
 * @uri1: a #SoupURI
 * @uri2: another #SoupURI
 *
 * Tests whether or not @uri1 and @uri2 are equal in all parts
 *
 * Return value: %TRUE or %FALSE
 **/
gboolean 
soup_uri_equal (SoupURI *uri1, SoupURI *uri2)
{
	g_return_val_if_fail (uri1 != NULL, FALSE);
	g_return_val_if_fail (uri2 != NULL, FALSE);
	g_warn_if_fail (SOUP_URI_IS_VALID (uri1));
	g_warn_if_fail (SOUP_URI_IS_VALID (uri2));

	if (uri1->scheme != uri2->scheme                         ||
	    uri1->port   != uri2->port                           ||
	    !parts_equal (uri1->user, uri2->user, FALSE)         ||
	    !parts_equal (uri1->password, uri2->password, FALSE) ||
	    !parts_equal (uri1->host, uri2->host, TRUE)          ||
	    !parts_equal (uri1->path, uri2->path, FALSE)         ||
	    !parts_equal (uri1->query, uri2->query, FALSE)       ||
	    !parts_equal (uri1->fragment, uri2->fragment, FALSE))
		return FALSE;

	return TRUE;
}

/**
 * soup_uri_free:
 * @uri: a #SoupURI
 *
 * Frees @uri.
 **/
void
soup_uri_free (SoupURI *uri)
{
	g_return_if_fail (uri != NULL);

	g_free (uri->user);
	g_free (uri->password);
	g_free (uri->host);
	g_free (uri->path);
	g_free (uri->query);
	g_free (uri->fragment);

	g_slice_free (SoupURI, uri);
}

static void
append_uri_encoded (GString *str, const char *in, const char *extra_enc_chars)
{
	const unsigned char *s = (const unsigned char *)in;

	while (*s) {
		if (soup_char_is_uri_percent_encoded (*s) ||
		    soup_char_is_uri_gen_delims (*s) ||
		    (extra_enc_chars && strchr (extra_enc_chars, *s)))
			g_string_append_printf (str, "%%%02X", (int)*s++);
		else
			g_string_append_c (str, *s++);
	}
}

/**
 * soup_uri_encode:
 * @part: a URI part
 * @escape_extra: (allow-none): additional reserved characters to
 * escape (or %NULL)
 *
 * This %<!-- -->-encodes the given URI part and returns the escaped
 * version in allocated memory, which the caller must free when it is
 * done.
 *
 * Return value: the encoded URI part
 **/
char *
soup_uri_encode (const char *part, const char *escape_extra)
{
	GString *str;
	char *encoded;

	g_return_val_if_fail (part != NULL, NULL);

	str = g_string_new (NULL);
	append_uri_encoded (str, part, escape_extra);
	encoded = str->str;
	g_string_free (str, FALSE);

	return encoded;
}

#define XDIGIT(c) ((c) <= '9' ? (c) - '0' : ((c) & 0x4F) - 'A' + 10)
#define HEXCHAR(s) ((XDIGIT (s[1]) << 4) + XDIGIT (s[2]))

char *
uri_decoded_copy (const char *part, int length)
{
	unsigned char *s, *d;
	char *decoded = g_strndup (part, length);

	g_return_val_if_fail (part != NULL, NULL);

	s = d = (unsigned char *)decoded;
	do {
		if (*s == '%') {
			if (!g_ascii_isxdigit (s[1]) ||
			    !g_ascii_isxdigit (s[2])) {
				*d++ = *s;
				continue;
			}
			*d++ = HEXCHAR (s);
			s += 2;
		} else
			*d++ = *s;
	} while (*s++);

	return decoded;
}

/**
 * soup_uri_decode:
 * @part: a URI part
 *
 * Fully %<!-- -->-decodes @part.
 *
 * In the past, this would return %NULL if @part contained invalid
 * percent-encoding, but now it just ignores the problem (as
 * soup_uri_new() already did).
 *
 * Return value: the decoded URI part.
 */
char *
soup_uri_decode (const char *part)
{
	g_return_val_if_fail (part != NULL, NULL);

	return uri_decoded_copy (part, strlen (part));
}

static char *
uri_normalized_copy (const char *part, int length,
		     const char *unescape_extra)
{
	unsigned char *s, *d, c;
	char *normalized = g_strndup (part, length);
	gboolean need_fixup = FALSE;

	if (!unescape_extra)
		unescape_extra = "";

	s = d = (unsigned char *)normalized;
	while (*s) {
		if (*s == '%') {
			if (!g_ascii_isxdigit (s[1]) ||
			    !g_ascii_isxdigit (s[2])) {
				*d++ = *s++;
				continue;
			}

			c = HEXCHAR (s);
			if (soup_char_is_uri_unreserved (c) ||
			    strchr (unescape_extra, c)) {
				*d++ = c;
				s += 3;
			} else {
				/* We leave it unchanged. We used to uppercase percent-encoded
				 * triplets but we do not do it any more as RFC3986 Section 6.2.2.1
				 * says that they only SHOULD be case normalized.
				 */
				*d++ = *s++;
				*d++ = *s++;
				*d++ = *s++;
			}
		} else {
			if (!g_ascii_isgraph (*s) &&
			    !strchr (unescape_extra, *s))
				need_fixup = TRUE;
			*d++ = *s++;
		}
	}
	*d = '\0';

	if (need_fixup) {
		GString *fixed;

		fixed = g_string_new (NULL);
		s = (guchar *)normalized;
		while (*s) {
			if (g_ascii_isgraph (*s) ||
			    strchr (unescape_extra, *s))
				g_string_append_c (fixed, *s);
			else
				g_string_append_printf (fixed, "%%%02X", (int)*s);
			s++;
		}
		g_free (normalized);
		normalized = g_string_free (fixed, FALSE);
	}

	return normalized;
}

/**
 * soup_uri_normalize:
 * @part: a URI part
 * @unescape_extra: reserved characters to unescape (or %NULL)
 *
 * %<!-- -->-decodes any "unreserved" characters (or characters in
 * @unescape_extra) in @part.
 *
 * "Unreserved" characters are those that are not allowed to be used
 * for punctuation according to the URI spec. For example, letters are
 * unreserved, so soup_uri_normalize() will turn
 * <literal>http://example.com/foo/b%<!-- -->61r</literal> into
 * <literal>http://example.com/foo/bar</literal>, which is guaranteed
 * to mean the same thing. However, "/" is "reserved", so
 * <literal>http://example.com/foo%<!-- -->2Fbar</literal> would not
 * be changed, because it might mean something different to the
 * server.
 *
 * In the past, this would return %NULL if @part contained invalid
 * percent-encoding, but now it just ignores the problem (as
 * soup_uri_new() already did).
 *
 * Return value: the normalized URI part
 */
char *
soup_uri_normalize (const char *part, const char *unescape_extra)
{
	g_return_val_if_fail (part != NULL, NULL);

	return uri_normalized_copy (part, strlen (part), unescape_extra);
}


/**
 * soup_uri_uses_default_port:
 * @uri: a #SoupURI
 *
 * Tests if @uri uses the default port for its scheme. (Eg, 80 for
 * http.) (This only works for http, https and ftp; libsoup does not know
 * the default ports of other protocols.)
 *
 * Return value: %TRUE or %FALSE
 **/
gboolean
soup_uri_uses_default_port (SoupURI *uri)
{
	g_return_val_if_fail (uri != NULL, FALSE);
	g_warn_if_fail (SOUP_URI_IS_VALID (uri));

	return uri->port == soup_scheme_default_port (uri->scheme);
}

/**
 * SOUP_URI_SCHEME_HTTP:
 *
 * "http" as an interned string. This can be compared directly against
 * the value of a #SoupURI's <structfield>scheme</structfield>
 **/

/**
 * SOUP_URI_SCHEME_HTTPS:
 *
 * "https" as an interned string. This can be compared directly
 * against the value of a #SoupURI's <structfield>scheme</structfield>
 **/

/**
 * SOUP_URI_SCHEME_RESOURCE:
 *
 * "resource" as an interned string. This can be compared directly
 * against the value of a #SoupURI's <structfield>scheme</structfield>
 *
 * Since: 2.42
 **/

/**
 * soup_uri_get_scheme:
 * @uri: a #SoupURI
 *
 * Gets @uri's scheme.
 *
 * Return value: @uri's scheme.
 *
 * Since: 2.32
 **/
const char *
soup_uri_get_scheme (SoupURI *uri)
{
	g_return_val_if_fail (uri != NULL, NULL);

	return uri->scheme;
}

/**
 * soup_uri_set_scheme:
 * @uri: a #SoupURI
 * @scheme: the URI scheme
 *
 * Sets @uri's scheme to @scheme. This will also set @uri's port to
 * the default port for @scheme, if known.
 **/
void
soup_uri_set_scheme (SoupURI *uri, const char *scheme)
{
	g_return_if_fail (uri != NULL);
	g_return_if_fail (scheme != NULL);

	uri->scheme = soup_uri_parse_scheme (scheme, strlen (scheme));
	uri->port = soup_scheme_default_port (uri->scheme);
}

/**
 * soup_uri_get_user:
 * @uri: a #SoupURI
 *
 * Gets @uri's user.
 *
 * Return value: @uri's user.
 *
 * Since: 2.32
 **/
const char *
soup_uri_get_user (SoupURI *uri)
{
	g_return_val_if_fail (uri != NULL, NULL);

	return uri->user;
}

/**
 * soup_uri_set_user:
 * @uri: a #SoupURI
 * @user: (allow-none): the username, or %NULL
 *
 * Sets @uri's user to @user.
 **/
void
soup_uri_set_user (SoupURI *uri, const char *user)
{
	g_return_if_fail (uri != NULL);

	g_free (uri->user);
	uri->user = g_strdup (user);
}

/**
 * soup_uri_get_password:
 * @uri: a #SoupURI
 *
 * Gets @uri's password.
 *
 * Return value: @uri's password.
 *
 * Since: 2.32
 **/
const char *
soup_uri_get_password (SoupURI *uri)
{
	g_return_val_if_fail (uri != NULL, NULL);

	return uri->password;
}

/**
 * soup_uri_set_password:
 * @uri: a #SoupURI
 * @password: (allow-none): the password, or %NULL
 *
 * Sets @uri's password to @password.
 **/
void
soup_uri_set_password (SoupURI *uri, const char *password)
{
	g_return_if_fail (uri != NULL);

	g_free (uri->password);
	uri->password = g_strdup (password);
}

/**
 * soup_uri_get_host:
 * @uri: a #SoupURI
 *
 * Gets @uri's host.
 *
 * Return value: @uri's host.
 *
 * Since: 2.32
 **/
const char *
soup_uri_get_host (SoupURI *uri)
{
	g_return_val_if_fail (uri != NULL, NULL);

	return uri->host;
}

/**
 * soup_uri_set_host:
 * @uri: a #SoupURI
 * @host: (allow-none): the hostname or IP address, or %NULL
 *
 * Sets @uri's host to @host.
 *
 * If @host is an IPv6 IP address, it should not include the brackets
 * required by the URI syntax; they will be added automatically when
 * converting @uri to a string.
 *
 * http and https URIs should not have a %NULL @host.
 **/
void
soup_uri_set_host (SoupURI *uri, const char *host)
{
	g_return_if_fail (uri != NULL);

	g_free (uri->host);
	uri->host = g_strdup (host);
}

/**
 * soup_uri_get_port:
 * @uri: a #SoupURI
 *
 * Gets @uri's port.
 *
 * Return value: @uri's port.
 *
 * Since: 2.32
 **/
guint
soup_uri_get_port (SoupURI *uri)
{
	g_return_val_if_fail (uri != NULL, 0);

	return uri->port;
}

/**
 * soup_uri_set_port:
 * @uri: a #SoupURI
 * @port: the port, or 0
 *
 * Sets @uri's port to @port. If @port is 0, @uri will not have an
 * explicitly-specified port.
 **/
void
soup_uri_set_port (SoupURI *uri, guint port)
{
	g_return_if_fail (uri != NULL);

	uri->port = port;
}

/**
 * soup_uri_get_path:
 * @uri: a #SoupURI
 *
 * Gets @uri's path.
 *
 * Return value: @uri's path.
 *
 * Since: 2.32
 **/
const char *
soup_uri_get_path (SoupURI *uri)
{
	g_return_val_if_fail (uri != NULL, NULL);

	return uri->path;
}

/**
 * soup_uri_set_path:
 * @uri: a #SoupURI
 * @path: the non-%NULL path
 *
 * Sets @uri's path to @path.
 **/
void
soup_uri_set_path (SoupURI *uri, const char *path)
{
	g_return_if_fail (uri != NULL);

	/* We allow a NULL path for compatibility, but warn about it. */
	if (!path) {
		g_warn_if_fail (path != NULL);
		path = "";
	}

	g_free (uri->path);
	uri->path = g_strdup (path);
}

/**
 * soup_uri_get_query:
 * @uri: a #SoupURI
 *
 * Gets @uri's query.
 *
 * Return value: @uri's query.
 *
 * Since: 2.32
 **/
const char *
soup_uri_get_query (SoupURI *uri)
{
	g_return_val_if_fail (uri != NULL, NULL);

	return uri->query;
}

/**
 * soup_uri_set_query:
 * @uri: a #SoupURI
 * @query: (allow-none): the query
 *
 * Sets @uri's query to @query.
 **/
void
soup_uri_set_query (SoupURI *uri, const char *query)
{
	g_return_if_fail (uri != NULL);

	g_free (uri->query);
	uri->query = g_strdup (query);
}

/**
 * soup_uri_set_query_from_form:
 * @uri: a #SoupURI
 * @form: (element-type utf8 utf8): a #GHashTable containing HTML form
 * information
 *
 * Sets @uri's query to the result of encoding @form according to the
 * HTML form rules. See soup_form_encode_hash() for more information.
 **/
void
soup_uri_set_query_from_form (SoupURI *uri, GHashTable *form)
{
	g_return_if_fail (uri != NULL);

	g_free (uri->query);
	uri->query = soup_form_encode_hash (form);
}

/**
 * soup_uri_set_query_from_fields:
 * @uri: a #SoupURI
 * @first_field: name of the first form field to encode into query
 * @...: value of @first_field, followed by additional field names
 * and values, terminated by %NULL.
 *
 * Sets @uri's query to the result of encoding the given form fields
 * and values according to the * HTML form rules. See
 * soup_form_encode() for more information.
 **/
void
soup_uri_set_query_from_fields (SoupURI    *uri,
				const char *first_field,
				...)
{
	va_list args;

	g_return_if_fail (uri != NULL);

	g_free (uri->query);
	va_start (args, first_field);
	uri->query = soup_form_encode_valist (first_field, args);
	va_end (args);
}

/**
 * soup_uri_get_fragment:
 * @uri: a #SoupURI
 *
 * Gets @uri's fragment.
 *
 * Return value: @uri's fragment.
 *
 * Since: 2.32
 **/
const char *
soup_uri_get_fragment (SoupURI *uri)
{
	g_return_val_if_fail (uri != NULL, NULL);

	return uri->fragment;
}

/**
 * soup_uri_set_fragment:
 * @uri: a #SoupURI
 * @fragment: (allow-none): the fragment
 *
 * Sets @uri's fragment to @fragment.
 **/
void
soup_uri_set_fragment (SoupURI *uri, const char *fragment)
{
	g_return_if_fail (uri != NULL);

	g_free (uri->fragment);
	uri->fragment = g_strdup (fragment);
}

/**
 * soup_uri_copy_host:
 * @uri: a #SoupURI
 *
 * Makes a copy of @uri, considering only the protocol, host, and port
 *
 * Return value: the new #SoupURI
 *
 * Since: 2.28
 **/
SoupURI *
soup_uri_copy_host (SoupURI *uri)
{
	SoupURI *dup;

	g_return_val_if_fail (uri != NULL, NULL);
	g_warn_if_fail (SOUP_URI_IS_VALID (uri));

	dup = soup_uri_new (NULL);
	dup->scheme = uri->scheme;
	dup->host   = g_strdup (uri->host);
	dup->port   = uri->port;
	dup->path   = g_strdup ("");

	return dup;
}

/**
 * soup_uri_host_hash:
 * @key: (type Soup.URI): a #SoupURI with a non-%NULL @host member
 *
 * Hashes @key, considering only the scheme, host, and port.
 *
 * Return value: a hash
 *
 * Since: 2.28
 **/
guint
soup_uri_host_hash (gconstpointer key)
{
	const SoupURI *uri = key;

	g_return_val_if_fail (uri != NULL && uri->host != NULL, 0);
	g_warn_if_fail (SOUP_URI_IS_VALID (uri));

	return GPOINTER_TO_UINT (uri->scheme) + uri->port +
		soup_str_case_hash (uri->host);
}

/**
 * soup_uri_host_equal:
 * @v1: (type Soup.URI): a #SoupURI with a non-%NULL @host member
 * @v2: (type Soup.URI): a #SoupURI with a non-%NULL @host member
 *
 * Compares @v1 and @v2, considering only the scheme, host, and port.
 *
 * Return value: whether or not the URIs are equal in scheme, host,
 * and port.
 *
 * Since: 2.28
 **/
gboolean
soup_uri_host_equal (gconstpointer v1, gconstpointer v2)
{
	const SoupURI *one = v1;
	const SoupURI *two = v2;

	g_return_val_if_fail (one != NULL && two != NULL, one == two);
	g_return_val_if_fail (one->host != NULL && two->host != NULL, one->host == two->host);
	g_warn_if_fail (SOUP_URI_IS_VALID (one));
	g_warn_if_fail (SOUP_URI_IS_VALID (two));

	if (one->scheme != two->scheme)
		return FALSE;
	if (one->port != two->port)
		return FALSE;

	return g_ascii_strcasecmp (one->host, two->host) == 0;
}

G_DEFINE_BOXED_TYPE (SoupURI, soup_uri, soup_uri_copy, soup_uri_free)
