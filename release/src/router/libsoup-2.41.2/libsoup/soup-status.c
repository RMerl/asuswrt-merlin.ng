/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-status.c: Status code descriptions
 *
 * Copyright (C) 2001-2003, Ximian, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "soup-status.h"
#include "soup.h"

/**
 * SECTION:soup-status
 * @short_description: HTTP (and libsoup) status codes
 *
 **/

/**
 * SOUP_STATUS_IS_TRANSPORT_ERROR:
 * @status: a status code
 *
 * Tests if @status is a libsoup transport error.
 *
 * Return value: %TRUE or %FALSE
 **/
/**
 * SOUP_STATUS_IS_INFORMATIONAL:
 * @status: an HTTP status code
 *
 * Tests if @status is an Informational (1xx) response.
 *
 * Return value: %TRUE or %FALSE
 **/
/**
 * SOUP_STATUS_IS_SUCCESSFUL:
 * @status: an HTTP status code
 *
 * Tests if @status is a Successful (2xx) response.
 *
 * Return value: %TRUE or %FALSE
 **/
/**
 * SOUP_STATUS_IS_REDIRECTION:
 * @status: an HTTP status code
 *
 * Tests if @status is a Redirection (3xx) response.
 *
 * Return value: %TRUE or %FALSE
 **/
/**
 * SOUP_STATUS_IS_CLIENT_ERROR:
 * @status: an HTTP status code
 *
 * Tests if @status is a Client Error (4xx) response.
 *
 * Return value: %TRUE or %FALSE
 **/
/**
 * SOUP_STATUS_IS_SERVER_ERROR:
 * @status: an HTTP status code
 *
 * Tests if @status is a Server Error (5xx) response.
 *
 * Return value: %TRUE or %FALSE
 **/

/**
 * SoupKnownStatusCode:
 * @SOUP_STATUS_NONE: No status available. (Eg, the message has not
 * been sent yet)
 * @SOUP_STATUS_CANCELLED: Message was cancelled locally
 * @SOUP_STATUS_CANT_RESOLVE: Unable to resolve destination host name
 * @SOUP_STATUS_CANT_RESOLVE_PROXY: Unable to resolve proxy host name
 * @SOUP_STATUS_CANT_CONNECT: Unable to connect to remote host
 * @SOUP_STATUS_CANT_CONNECT_PROXY: Unable to connect to proxy
 * @SOUP_STATUS_SSL_FAILED: SSL/TLS negotiation failed
 * @SOUP_STATUS_IO_ERROR: A network error occurred, or the other end
 * closed the connection unexpectedly
 * @SOUP_STATUS_MALFORMED: Malformed data (usually a programmer error)
 * @SOUP_STATUS_TRY_AGAIN: Used internally
 * @SOUP_STATUS_TOO_MANY_REDIRECTS: There were too many redirections
 * @SOUP_STATUS_TLS_FAILED: Used internally
 * @SOUP_STATUS_CONTINUE: 100 Continue (HTTP)
 * @SOUP_STATUS_SWITCHING_PROTOCOLS: 101 Switching Protocols (HTTP)
 * @SOUP_STATUS_PROCESSING: 102 Processing (WebDAV)
 * @SOUP_STATUS_OK: 200 Success (HTTP). Also used by many lower-level
 * soup routines to indicate success.
 * @SOUP_STATUS_CREATED: 201 Created (HTTP)
 * @SOUP_STATUS_ACCEPTED: 202 Accepted (HTTP)
 * @SOUP_STATUS_NON_AUTHORITATIVE: 203 Non-Authoritative Information
 * (HTTP)
 * @SOUP_STATUS_NO_CONTENT: 204 No Content (HTTP)
 * @SOUP_STATUS_RESET_CONTENT: 205 Reset Content (HTTP)
 * @SOUP_STATUS_PARTIAL_CONTENT: 206 Partial Content (HTTP)
 * @SOUP_STATUS_MULTI_STATUS: 207 Multi-Status (WebDAV)
 * @SOUP_STATUS_MULTIPLE_CHOICES: 300 Multiple Choices (HTTP)
 * @SOUP_STATUS_MOVED_PERMANENTLY: 301 Moved Permanently (HTTP)
 * @SOUP_STATUS_FOUND: 302 Found (HTTP)
 * @SOUP_STATUS_MOVED_TEMPORARILY: 302 Moved Temporarily (old name,
 * RFC 2068)
 * @SOUP_STATUS_SEE_OTHER: 303 See Other (HTTP)
 * @SOUP_STATUS_NOT_MODIFIED: 304 Not Modified (HTTP)
 * @SOUP_STATUS_USE_PROXY: 305 Use Proxy (HTTP)
 * @SOUP_STATUS_NOT_APPEARING_IN_THIS_PROTOCOL: 306 [Unused] (HTTP)
 * @SOUP_STATUS_TEMPORARY_REDIRECT: 307 Temporary Redirect (HTTP)
 * @SOUP_STATUS_BAD_REQUEST: 400 Bad Request (HTTP)
 * @SOUP_STATUS_UNAUTHORIZED: 401 Unauthorized (HTTP)
 * @SOUP_STATUS_PAYMENT_REQUIRED: 402 Payment Required (HTTP)
 * @SOUP_STATUS_FORBIDDEN: 403 Forbidden (HTTP)
 * @SOUP_STATUS_NOT_FOUND: 404 Not Found (HTTP)
 * @SOUP_STATUS_METHOD_NOT_ALLOWED: 405 Method Not Allowed (HTTP)
 * @SOUP_STATUS_NOT_ACCEPTABLE: 406 Not Acceptable (HTTP)
 * @SOUP_STATUS_PROXY_AUTHENTICATION_REQUIRED: 407 Proxy Authentication
 * Required (HTTP)
 * @SOUP_STATUS_PROXY_UNAUTHORIZED: shorter alias for
 * %SOUP_STATUS_PROXY_AUTHENTICATION_REQUIRED
 * @SOUP_STATUS_REQUEST_TIMEOUT: 408 Request Timeout (HTTP)
 * @SOUP_STATUS_CONFLICT: 409 Conflict (HTTP)
 * @SOUP_STATUS_GONE: 410 Gone (HTTP)
 * @SOUP_STATUS_LENGTH_REQUIRED: 411 Length Required (HTTP)
 * @SOUP_STATUS_PRECONDITION_FAILED: 412 Precondition Failed (HTTP)
 * @SOUP_STATUS_REQUEST_ENTITY_TOO_LARGE: 413 Request Entity Too Large
 * (HTTP)
 * @SOUP_STATUS_REQUEST_URI_TOO_LONG: 414 Request-URI Too Long (HTTP)
 * @SOUP_STATUS_UNSUPPORTED_MEDIA_TYPE: 415 Unsupported Media Type
 * (HTTP)
 * @SOUP_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE: 416 Requested Range
 * Not Satisfiable (HTTP)
 * @SOUP_STATUS_INVALID_RANGE: shorter alias for
 * %SOUP_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE
 * @SOUP_STATUS_EXPECTATION_FAILED: 417 Expectation Failed (HTTP)
 * @SOUP_STATUS_UNPROCESSABLE_ENTITY: 422 Unprocessable Entity
 * (WebDAV)
 * @SOUP_STATUS_LOCKED: 423 Locked (WebDAV)
 * @SOUP_STATUS_FAILED_DEPENDENCY: 424 Failed Dependency (WebDAV)
 * @SOUP_STATUS_INTERNAL_SERVER_ERROR: 500 Internal Server Error
 * (HTTP)
 * @SOUP_STATUS_NOT_IMPLEMENTED: 501 Not Implemented (HTTP)
 * @SOUP_STATUS_BAD_GATEWAY: 502 Bad Gateway (HTTP)
 * @SOUP_STATUS_SERVICE_UNAVAILABLE: 503 Service Unavailable (HTTP)
 * @SOUP_STATUS_GATEWAY_TIMEOUT: 504 Gateway Timeout (HTTP)
 * @SOUP_STATUS_HTTP_VERSION_NOT_SUPPORTED: 505 HTTP Version Not
 * Supported (HTTP)
 * @SOUP_STATUS_INSUFFICIENT_STORAGE: 507 Insufficient Storage
 * (WebDAV)
 * @SOUP_STATUS_NOT_EXTENDED: 510 Not Extended (RFC 2774)
 * 
 * These represent the known HTTP status code values, plus various
 * network and internal errors.
 **/

/**
 * SOUP_HTTP_ERROR:
 *
 * A #GError domain representing an HTTP status. Use a
 * #SoupKnownStatusCode for the <structfield>code</structfield>
 * value.
 **/


/* The reason_phrases are not localized because:
 *
 * 1. Only ASCII can be used portably in the HTTP Status-Line, so we
 *    would not be able to return localized reason phrases from
 *    SoupServer anyway.
 *
 * 2. Having a way for clients to get a localized version of a status
 *    code would just encourage them to present those strings to the
 *    user, which is bad because many of them are fairly
 *    incomprehensible anyway.
 */

static const struct {
	guint code;
	const char *phrase;
} reason_phrases [] = {
	/* Transport errors */
	{ SOUP_STATUS_CANCELLED,                  "Cancelled" },
	{ SOUP_STATUS_CANT_RESOLVE,               "Cannot resolve hostname" },
	{ SOUP_STATUS_CANT_RESOLVE_PROXY,         "Cannot resolve proxy hostname" },
	{ SOUP_STATUS_CANT_CONNECT,               "Cannot connect to destination" },
	{ SOUP_STATUS_CANT_CONNECT_PROXY,         "Cannot connect to proxy" },
	{ SOUP_STATUS_SSL_FAILED,                 "SSL handshake failed" },
	{ SOUP_STATUS_IO_ERROR,                   "Connection terminated unexpectedly" },
	{ SOUP_STATUS_MALFORMED,                  "Message Corrupt" },
	{ SOUP_STATUS_TOO_MANY_REDIRECTS,         "Too many redirects" },

	/* Informational */
	{ SOUP_STATUS_CONTINUE,                   "Continue" },
	{ SOUP_STATUS_SWITCHING_PROTOCOLS,        "Switching Protocols" },
	{ SOUP_STATUS_PROCESSING,                 "Processing" },

	/* Success */
	{ SOUP_STATUS_OK,                         "OK" },
	{ SOUP_STATUS_CREATED,                    "Created" },
	{ SOUP_STATUS_ACCEPTED,                   "Accepted" },
	{ SOUP_STATUS_NON_AUTHORITATIVE,          "Non-Authoritative Information" },
	{ SOUP_STATUS_NO_CONTENT,                 "No Content" },
	{ SOUP_STATUS_RESET_CONTENT,              "Reset Content" },
	{ SOUP_STATUS_PARTIAL_CONTENT,            "Partial Content" },
	{ SOUP_STATUS_MULTI_STATUS,               "Multi-Status" },

	/* Redirection */
	{ SOUP_STATUS_MULTIPLE_CHOICES,           "Multiple Choices" },
	{ SOUP_STATUS_MOVED_PERMANENTLY,          "Moved Permanently" },
	{ SOUP_STATUS_FOUND,                      "Found" },
	{ SOUP_STATUS_SEE_OTHER,                  "See Other" },
	{ SOUP_STATUS_NOT_MODIFIED,               "Not Modified" },
	{ SOUP_STATUS_USE_PROXY,                  "Use Proxy" },
	{ SOUP_STATUS_TEMPORARY_REDIRECT,         "Temporary Redirect" },

	/* Client error */
	{ SOUP_STATUS_BAD_REQUEST,                "Bad Request" },
	{ SOUP_STATUS_UNAUTHORIZED,               "Unauthorized" },
	{ SOUP_STATUS_PAYMENT_REQUIRED,           "Payment Required" },
	{ SOUP_STATUS_FORBIDDEN,                  "Forbidden" },
	{ SOUP_STATUS_NOT_FOUND,                  "Not Found" },
	{ SOUP_STATUS_METHOD_NOT_ALLOWED,         "Method Not Allowed" },
	{ SOUP_STATUS_NOT_ACCEPTABLE,             "Not Acceptable" },
	{ SOUP_STATUS_PROXY_UNAUTHORIZED,         "Proxy Authentication Required" },
	{ SOUP_STATUS_REQUEST_TIMEOUT,            "Request Timeout" },
	{ SOUP_STATUS_CONFLICT,                   "Conflict" },
	{ SOUP_STATUS_GONE,                       "Gone" },
	{ SOUP_STATUS_LENGTH_REQUIRED,            "Length Required" },
	{ SOUP_STATUS_PRECONDITION_FAILED,        "Precondition Failed" },
	{ SOUP_STATUS_REQUEST_ENTITY_TOO_LARGE,   "Request Entity Too Large" },
	{ SOUP_STATUS_REQUEST_URI_TOO_LONG,       "Request-URI Too Long" },
	{ SOUP_STATUS_UNSUPPORTED_MEDIA_TYPE,     "Unsupported Media Type" },
	{ SOUP_STATUS_INVALID_RANGE,              "Requested Range Not Satisfiable" },
	{ SOUP_STATUS_EXPECTATION_FAILED,         "Expectation Failed" },
	{ SOUP_STATUS_UNPROCESSABLE_ENTITY,       "Unprocessable Entity" },
	{ SOUP_STATUS_LOCKED,                     "Locked" },
	{ SOUP_STATUS_FAILED_DEPENDENCY,          "Failed Dependency" },

	/* Server error */
	{ SOUP_STATUS_INTERNAL_SERVER_ERROR,      "Internal Server Error" },
	{ SOUP_STATUS_NOT_IMPLEMENTED,            "Not Implemented" },
	{ SOUP_STATUS_BAD_GATEWAY,                "Bad Gateway" },
	{ SOUP_STATUS_SERVICE_UNAVAILABLE,        "Service Unavailable" },
	{ SOUP_STATUS_GATEWAY_TIMEOUT,            "Gateway Timeout" },
	{ SOUP_STATUS_HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported" },
	{ SOUP_STATUS_INSUFFICIENT_STORAGE,       "Insufficient Storage" },
	{ SOUP_STATUS_NOT_EXTENDED,               "Not Extended" },

	{ 0 }
};

/**
 * soup_status_get_phrase:
 * @status_code: an HTTP status code
 *
 * Looks up the stock HTTP description of @status_code. This is used
 * by soup_message_set_status() to get the correct text to go with a
 * given status code.
 *
 * <emphasis>There is no reason for you to ever use this
 * function.</emphasis> If you wanted the textual description for the
 * #SoupMessage:status_code of a given #SoupMessage, you should just
 * look at the message's #SoupMessage:reason_phrase. However, you
 * should only do that for use in debugging messages; HTTP reason
 * phrases are not localized, and are not generally very descriptive
 * anyway, and so they should never be presented to the user directly.
 * Instead, you should create you own error messages based on the
 * status code, and on what you were trying to do.
 *
 * Return value: the (terse, English) description of @status_code
 **/
const char *
soup_status_get_phrase (guint status_code)
{
	int i;

	for (i = 0; reason_phrases [i].code; i++) {
		if (reason_phrases [i].code == status_code)
			return reason_phrases [i].phrase;
	}

	return "Unknown Error";
}

/**
 * soup_status_proxify:
 * @status_code: a status code
 *
 * Turns %SOUP_STATUS_CANT_RESOLVE into
 * %SOUP_STATUS_CANT_RESOLVE_PROXY and %SOUP_STATUS_CANT_CONNECT into
 * %SOUP_STATUS_CANT_CONNECT_PROXY. Other status codes are passed
 * through unchanged.
 *
 * Return value: the "proxified" equivalent of @status_code.
 *
 * Since: 2.26
 **/
guint
soup_status_proxify (guint status_code)
{
	if (status_code == SOUP_STATUS_CANT_RESOLVE)
		return SOUP_STATUS_CANT_RESOLVE_PROXY;
	else if (status_code == SOUP_STATUS_CANT_CONNECT)
		return SOUP_STATUS_CANT_CONNECT_PROXY;
	else
		return status_code;
}

GQuark
soup_http_error_quark (void)
{
	static GQuark error;
	if (!error)
		error = g_quark_from_static_string ("soup_http_error_quark");
	return error;
}
