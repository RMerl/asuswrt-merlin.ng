/*
 * Copyright (C) 2007 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup fast_request fast_request
 * @{ @ingroup libfast
 */

#ifndef FAST_REQUEST_H_
#define FAST_REQUEST_H_

#include <fcgiapp.h>
#include <library.h>

typedef struct fast_request_t fast_request_t;

/**
 * A HTTP request, encapsulates FCGX_Request.
 *
 * The response is also handled through the request object.
 */
struct fast_request_t {

	/**
	 * Add a cookie to the reply (Set-Cookie header).
	 *
	 * @param name		name of the cookie to set
	 * @param value		value of the cookie
	 */
	void (*add_cookie)(fast_request_t *this, char *name, char *value);

	/**
	 * Get a cookie the client sent in the request.
	 *
	 * @param name		name of the cookie
	 * @return			cookie value, NULL if no such cookie found
	 */
	char* (*get_cookie)(fast_request_t *this, char *name);

	/**
	 * Get the request path relative to the application.
	 *
	 * @return			path
	 */
	char* (*get_path)(fast_request_t *this);

	/**
	 * Get the base path of the application.
	 *
	 * @return			base path
	 */
	char* (*get_base)(fast_request_t *this);

	/**
	 * Get the remote host address of this request.
	 *
	 * @return			host address as string
	 */
	char* (*get_host)(fast_request_t *this);

	/**
	 * Get the user agent string.
	 *
	 * @return			user agent string
	 */
	char* (*get_user_agent)(fast_request_t *this);

	/**
	 * Get a post/get variable included in the request.
	 *
	 * @param name		name of the POST/GET variable
	 * @return			value, NULL if not found
	 */
	char* (*get_query_data)(fast_request_t *this, char *name);

	/**
	 * Get an arbitrary environment variable.
	 *
	 * @param name		name of the environment variable
	 * @return			value, NULL if not found
	 */
	char* (*get_env_var)(fast_request_t *this, char *name);

	/**
	 * Read raw POST/PUT data from HTTP request.
	 *
	 * @param buf		buffer to read data into
	 * @param len		size of the supplied buffer
	 * @return			number of bytes read, < 0 on error
	 */
	int (*read_data)(fast_request_t *this, char *buf, int len);

	/**
	 * Close the session and it's context after handling.
	 */
	void (*close_session)(fast_request_t *this);

	/**
	 * Has the session been closed by close_session()?
	 *
	 * @return			TRUE if session has been closed
	 */
	bool (*session_closed)(fast_request_t *this);

	/**
	 * Redirect the client to another location.
	 *
	 * @param fmt		location format string
	 * @param ...		variable argument for fmt
	 */
	void (*redirect)(fast_request_t *this, char *fmt, ...);

	/**
	 * Get the HTTP referer.
	 *
	 * @return			HTTP referer
	 */
	char* (*get_referer)(fast_request_t *this);

	/**
	 * Redirect back to the referer.
	 */
	void (*to_referer)(fast_request_t *this);

	/**
	 * Set a template value.
	 *
	 * @param key		key to set
	 * @param value		value to set key to
	 */
	void (*set)(fast_request_t *this, char *key, char *value);

	/**
	 * Set a template value using format strings.
	 *
	 * Format string is in the form "key=value", where printf like format
	 * substitution occurs over the whole string.
	 *
	 * @param format	printf like format string
	 * @param ...		variable argument list
	 */
	void (*setf)(fast_request_t *this, char *format, ...);

	/**
	 * Render a template.
	 *
	 * The render() function additionally sets a HDF variable "base"
	 * which points to the root of the web application and allows to point to
	 * other targets without to worry about path location.
	 *
	 * @param template	clearsilver template file location
	 */
	void (*render)(fast_request_t *this, char *template);

	/**
	 * Stream a format string to the client.
	 *
	 * Stream is not closed and may be called multiple times to allow
	 * server-push functionality.
	 *
	 * @param format	printf like format string
	 * @param ...		argmuent list to format string
	 * @return			number of streamed bytes, < 0 if stream closed
	 */
	int (*streamf)(fast_request_t *this, char *format, ...);

	/**
	 * Serve a request with headers and a body.
	 *
	 * @param headers	HTTP headers, \n separated
	 * @param chunk		body to write to output
	 */
	void (*serve)(fast_request_t *this, char *headers, chunk_t chunk);

	/**
	 * Send a file from the file system.
	 *
	 * @param path		path to file to serve
	 * @param mime		mime type of file to send, or NULL
	 * @return			TRUE if file served successfully
	 */
	bool (*sendfile)(fast_request_t *this, char *path, char *mime);

	/**
	 * Increase the reference count to the stream.
	 *
	 * @return			this with increased refcount
	 */
	fast_request_t* (*get_ref)(fast_request_t *this);

	/**
	 * Destroy the fast_request_t.
	 */
	void (*destroy) (fast_request_t *this);
};

/**
 * Create a request from the fastcgi struct.
 *
 * @param fd			file descripter opened with FCGX_OpenSocket
 * @param debug			no stripping, no compression, timing information
 */
fast_request_t *fast_request_create(int fd, bool debug);

#endif /** REQUEST_H_ @}*/
