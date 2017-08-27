/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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
 * @defgroup libvici libvici
 * @{ @ingroup vici
 *
 * libvici is a low-level client library for the "Versatile IKE Control
 * Interface" protocol. While it uses libstrongswan and its thread-pool for
 * asynchronous message delivery, this interface does not directly depend on
 * libstrongswan interfaces and should be stable.
 *
 * This interface provides the following basic functions:
 *
 * - vici_init()/vici_deinit(): Library initialization functions
 * - vici_connect(): Connect to a vici service
 * - vici_disconnect(): Disconnect from a vici service
 *
 * Library initialization implicitly initializes libstrongswan and a small
 * thread pool.
 *
 * Connecting requires an uri, which is currently either a UNIX socket path
 * prefixed with unix://, or a hostname:port touple prefixed with tcp://.
 * Passing NULL takes the system default socket path.
 *
 * After the connection has been established, request messages can be sent.
 * Only a single thread may operate on a single connection instance
 * simultaneously. To construct request messages, use the following functions:
 *
 * - vici_add_key_value() / vici_add_key_valuef(): Add key/value pairs
 * - vici_begin(): Start constructing a new request message
 * - vici_begin_section(): Open a new section to add contents to
 * - vici_end_section(): Close a previously opened session
 * - vici_begin_list(): Open a new list to add list items to
 * - vici_end_list(): Close a previously opened list
 * - vici_add_list_item() / vici_add_list_itemf(): Add list item
 *
 * Once the request message is complete, it can be sent or cancelled with:
 *
 * - vici_submit()
 * - vici_free_req()
 *
 * If submitting a message is successful, a response message is returned. It
 * can be processed using the following functions:
 *
 * - vici_parse(): Parse content type
 * - vici_parse_name(): Parse name if content type provides one
 * - vici_parse_name_eq(): Parse name and check if matches string
 * - vici_parse_value() / vici_parse_value_str(): Parse value for content type
 * - vici_dump(): Dump a full response to a FILE stream
 * - vici_free_res(): Free response after use
 *
 * Usually vici_parse() is called in a loop, and depending on the returned
 * type the name and value can be inspected.
 *
 * To register or unregister for asynchronous event messages vici_register() is
 * used. The registered callback gets invoked by an asynchronous thread. To
 * parse the event message, the vici_parse*() functions can be used.
 */

#ifndef LIBVICI_H_
#define LIBVICI_H_

#include <stdio.h>

/**
 * Opaque vici connection contex.
 */
typedef struct vici_conn_t vici_conn_t;

/**
 * Opaque vici request message.
 */
typedef struct vici_req_t vici_req_t;

/**
 * Opaque vici response/event message.
 */
typedef struct vici_res_t vici_res_t;

/**
 * Vici parse result, as returned by vici_parse().
 */
typedef enum {
	/** encountered a section start, has a name */
	VICI_PARSE_BEGIN_SECTION,
	/** encountered a section end */
	VICI_PARSE_END_SECTION,
	/** encountered a list start, has a name */
	VICI_PARSE_BEGIN_LIST,
	/** encountered a list element, has a value */
	VICI_PARSE_LIST_ITEM,
	/** encountered a list end */
	VICI_PARSE_END_LIST,
	/** encountered a key/value pair, has a name and a value */
	VICI_PARSE_KEY_VALUE,
	/** encountered valid end of message */
	VICI_PARSE_END,
	/** parse error */
	VICI_PARSE_ERROR,
} vici_parse_t;

/**
 * Callback function invoked for received event messages.
 *
 * It is not allowed to call vici_submit() from this callback.
 *
 * @param user		user data, as passed to vici_connect
 * @param name		name of received event
 * @param msg		associated event message, destroyed by libvici
 */
typedef void (*vici_event_cb_t)(void *user, char *name, vici_res_t *msg);

/**
 * Callback function for key/value and list items, invoked by vici_parse_cb().
 *
 * @param user		user data, as passed to vici_parse_cb()
 * @param res		message currently parsing
 * @param name		name of key or list
 * @param value		value buffer
 * @param len		length of value buffer
 * @return			0 if parsed successfully
 */
typedef int	(*vici_parse_value_cb_t)(void *user, vici_res_t *res, char *name,
									 void *value, int len);

/**
 * Callback function for sections, invoked by vici_parse_cb().
 *
 * @param user		user data, as passed to vici_parse_cb()
 * @param res		message currently parsing
 * @param name		name of the section
 * @return			0 if parsed successfully
 */
typedef int (*vici_parse_section_cb_t)(void *user, vici_res_t *res, char *name);

/**
 * Open a new vici connection.
 *
 * On error, NULL is returned and errno is set appropriately.
 *
 * @param uri		URI to connect to, NULL to use system default
 * @return			opaque vici connection context, NULL on error
 */
vici_conn_t* vici_connect(char *uri);

/**
 * Close a vici connection.
 *
 * @param conn		connection context
 */
void vici_disconnect(vici_conn_t *conn);

/**
 * Begin a new vici message request.
 *
 * This function always succeeds.
 *
 * @param name		name of request command
 * @return			request message, to add contents
 */
vici_req_t* vici_begin(char *name);

/**
 * Begin a new section in a vici request message.
 *
 * @param req		request message to create a new section in
 * @param name		name of section to create
 */
void vici_begin_section(vici_req_t *req, char *name);

/**
 * End a previously opened section.
 *
 * @param req		request message to close an open section in
 */
void vici_end_section(vici_req_t *req);

/**
 * Add a key/value pair, using an as-is blob as value.
 *
 * @param req		request message to add key/value pair to
 * @param key		key name of key/value pair
 * @param buf		pointer to blob to add as value
 * @param len		length of value blob to add
 */
void vici_add_key_value(vici_req_t *req, char *key, void *buf, int len);

/**
 * Add a key/value pair, setting value from a printf() format string.
 *
 * @param req		request message to add key/value pair to
 * @param key		key name of key/value pair
 * @param fmt		format string for value
 * @param ...		arguments to format string
 */
void vici_add_key_valuef(vici_req_t *req, char *key, char *fmt, ...);

/**
 * Begin a list in a request message.
 *
 * After starting a list, only list items can be added until the list gets
 * closed by vici_end_list().
 *
 * @param req		request message to begin list in
 * @param name		name of list to begin
 */
void vici_begin_list(vici_req_t *req, char *name);

/**
 * Add a list item to a currently open list, using an as-is blob.
 *
 * @param req		request message to add list item to
 * @param buf		pointer to blob to add as value
 * @param len		length of value blob to add
 */
void vici_add_list_item(vici_req_t *req, void *buf, int len);

/**
 * Add a list item to a currently open list, using a printf() format string.
 *
 * @param req		request message to add list item to
 * @param fmt		format string to create value from
 * @param ...		arguments to format string
 */
void vici_add_list_itemf(vici_req_t *req, char *fmt, ...);

/**
 * End a previously opened list in a request message.
 *
 * @param req		request message to end list in
 */
void vici_end_list(vici_req_t *req);

/**
 * Submit a request message, and wait for response.
 *
 * The request messages gets cleaned up by this call and gets invalid.
 * On error, NULL is returned an errno is set to:
 * - EINVAL if the request is invalid/incomplete
 * - ENOENT if the command is unknown
 * - EBADMSG if the response is invalid
 * - Any other IO related errno
 *
 * @param req		request message to send
 * @param conn		connection context to send message over
 * @return			response message, NULL on error
 */
vici_res_t* vici_submit(vici_req_t *req, vici_conn_t *conn);

/**
 * Cancel a request message started.
 *
 * If a request created by vici_begin() does not get submitted using
 * vici_submit(), it has to get freed using this call.
 *
 * @param req		request message to clean up
 */
void vici_free_req(vici_req_t *req);

/**
 * Dump a message text representation to a FILE stream.
 *
 * On error, errno is set to:
 * - EBADMSG if the message is invalid
 *
 * @param res		response message to dump
 * @param label		a label to print for this message
 * @param pretty	use pretty print with indentation
 * @param out		FILE to dump to
 * @return			0 if dumped complete message, 1 on error
 */
int vici_dump(vici_res_t *res, char *label, int pretty, FILE *out);

/**
 * Parse next element from a vici response message.
 *
 * @param res		response message to parse
 * @return			parse result
 */
vici_parse_t vici_parse(vici_res_t *res);

/**
 * Parse name tag / key of a previously parsed element.
 *
 * This call is valid only after vici_parse() returned VICI_PARSE_KEY_VALUE,
 * VICI_PARSE_BEGIN_SECTION or VICI_PARSE_BEGIN_LIST.
 *
 * The string is valid until vici_free_res() is called.
 *
 * On error, errno is set to:
 *- EINVAL if not in valid parser state
 *
 * @param res		response message to parse
 * @return			name tag / key, NULL on error
 */
char* vici_parse_name(vici_res_t *res);

/**
 * Compare name tag / key of a previusly parsed element.
 *
 * This call is valid only after vici_parse() returned VICI_PARSE_KEY_VALUE,
 * VICI_PARSE_BEGIN_SECTION or VICI_PARSE_BEGIN_LIST.
 *
 * @param res		response message to parse
 * @param name		string to compare
 * @return			1 if name equals, 0 if not
 */
int vici_parse_name_eq(vici_res_t *res, char *name);

/**
 * Parse value of a previously parsed element, as a blob.
 *
 * This call is valid only after vici_parse() returned VICI_PARSE_KEY_VALUE or
 * VICI_PARSE_LIST_ITEM.
 *
 * The string is valid until vici_free_res() is called.
 *
 * On error, errno is set to:
 * - EINVAL if not in valid parser state
 *
 * @param res		response message to parse
 * @param len		pointer receiving value length
 * @return			pointer to value, NULL on error
 */
void* vici_parse_value(vici_res_t *res, int *len);

/**
 * Parse value of a previously parsed element, as a string.
 *
 * This call is valid only after vici_parse() returned VICI_PARSE_KEY_VALUE or
 * VICI_PARSE_LIST_ITEM.
 *
 * This call is successful only if the value contains no non-printable
 * characters. The string is valid until vici_free_res() is called.
 *
 * On error, errno is set to:
 * - EBADMSG if value is not a printable string
 * - EINVAL if not in valid parser state
 *
 * @param res		response message to parse
 * @return			value as string, NULL on error
 */
char* vici_parse_value_str(vici_res_t *res);

/**
 * Parse a complete message with callbacks.
 *
 * Any of the callbacks may be NULL to skip this kind of item. Callbacks are
 * invoked for the current section level only. To descent into sections, call
 * vici_parse_cb() from within a section callback.
 *
 * On error, errno is set to:
 * - EBADMSG if message encoding invalid
 * - Any other errno set by the invoked callbacks
 *
 * @param res		message to parse
 * @param section	callback invoked for each section
 * @param kv		callback invoked for key/value pairs
 * @param li		callback invoked for list items
 * @param user		user data to pass to callbacks
 * @return			0 if parsing successful
 */
int vici_parse_cb(vici_res_t *res, vici_parse_section_cb_t section,
				  vici_parse_value_cb_t kv, vici_parse_value_cb_t li,
				  void *user);

/*
 * Find a blob value in a message for a given key.
 *
 * Sections can be selected by prefixing them separated by dots.
 *
 * @param res		response message to parse
 * @param len		length of returned object
 * @param fmt		printf format string of key and sections
 * @param ...		arguments to format string
 * @return			blob value, having *len bytes, NULL if not found
 */
void *vici_find(vici_res_t *res, int *len, char *fmt, ...);

/**
 * Find a string value in a message for a given key.
 *
 * Sections can be selected by prefixing them separated by dots.
 *
 * @param res		response message to parse
 * @param def		default value, if key not found
 * @param fmt		printf format string of key and sections
 * @param ...		arguments to format string
 * @return			string, def if not found
 */
char* vici_find_str(vici_res_t *res, char *def, char *fmt, ...);

/**
 * Find an integer value in a message for a given key.
 *
 * Sections can be selected by prefixing them separated by dots.
 *
 * @param res		response message to parse
 * @param def		default value, if key not found
 * @param fmt		printf format string of key and sections
 * @param ...		arguments to format string
 * @return			integer value, def if not found
 */
int vici_find_int(vici_res_t *res, int def, char *fmt, ...);

/**
 * Clean up a received response message.
 *
 * Event messages get cleaned up by the library, it is not allowed to call
 * vici_free_res() from within a vici_event_cb_t.
 *
 * @param res		response message to free
 */
void vici_free_res(vici_res_t *res);

/**
 * (Un-)Register for events of a given kind.
 *
 * Events callbacks get invoked by a different thread from the libstrongswan
 * thread pool.
 * On failure, errno is set to:
 * - ENOENT if the event name is unknown
 * - EBADMSG if the response is invalid
 * - Any other IO related errno
 *
 * @param conn		connection context
 * @param name		name of event messages to register to
 * @param cb		callback function to register, NULL to unregister
 * @param user		user data passed to callback invocations
 * @return			0 if registered successfully
 */
int vici_register(vici_conn_t *conn, char *name, vici_event_cb_t cb, void *user);

/**
 * Initialize libvici before first time use.
 */
void vici_init();

/**
 * Deinitialize libvici after use.
 */
void vici_deinit();

#endif /** LIBVICI_H_ @}*/
