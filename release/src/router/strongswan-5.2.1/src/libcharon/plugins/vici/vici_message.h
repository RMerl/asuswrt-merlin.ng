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
 * @defgroup vici_message vici_message
 * @{ @ingroup vici_dispatcher
 */

#ifndef VICI_MESSAGE_H_
#define VICI_MESSAGE_H_

#include <library.h>

typedef struct vici_message_t vici_message_t;
typedef struct vici_parse_context_t vici_parse_context_t;
typedef enum vici_type_t vici_type_t;

/**
 * Vici message encoding types
 */
enum vici_type_t {
	/** never used in an argument list, needed by dump as initial value */
	VICI_START =         0,

	/** begin of new section, argument is section name as char* */
	VICI_SECTION_START = 1,
	/** end of current section, no arguments */
	VICI_SECTION_END =   2,
	/** key/value, arguments are key as char*, value as chunk_t */
	VICI_KEY_VALUE =     3,
	/** list start, argument is list name as char* */
	VICI_LIST_START =    4,
	/** list item, argument is item value as chunk_t */
	VICI_LIST_ITEM =     5,
	/** end of list, no arguments */
	VICI_LIST_END =      6,

	/** end of argument list, no arguments (never encoded) */
	VICI_END =           7
};

/**
 * Callback function for key/value and list items, invoked by parse().
 *
 * @param user		user data, as passed to parse()
 * @param message	message currently parsing
 * @param name		name of key or list
 * @param value		parsed value
 * @return			TRUE if parsed successfully
 */
typedef bool (*vici_value_cb_t)(void *user, vici_message_t *message,
								char *name, chunk_t value);

/**
 * Callback function for sections, invoked by parse().
 *
 * @param user		user data, as passed to parse()
 * @param message	message currently parsing
 * @param ctx		parse context, to pass to recursive parse() invocations.
 * @param name		name of the section
 * @return			TRUE if parsed successfully
 */
typedef bool (*vici_section_cb_t)(void *user, vici_message_t *message,
								  vici_parse_context_t *ctx, char *name);

/**
 * Names for vici encoding types
 */
extern enum_name_t *vici_type_names;

/**
 * Vici message representation, encoding/decoding routines.
 */
struct vici_message_t {

	/**
	 * Create an enumerator over message contents.
	 *
	 * The enumerator takes a fixed list of arguments, but depending on the
	 * type may set not all of them. It returns VICI_END as last argument
	 * to indicate the message end, and returns FALSE if parsing the message
	 * failed.
	 *
	 * @return		enumerator over (vici_type_t, char*, chunk_t)
	 */
	enumerator_t* (*create_enumerator)(vici_message_t *this);

	/**
	 * Get the value of a key/value pair as a string.
	 *
	 * @param def	default value if not found
	 * @param fmt	printf style format string for key, with sections
	 * @param ...	arguments to fmt string
	 * @return		string
	 */
	char* (*get_str)(vici_message_t *this, char *def, char *fmt, ...);

	/**
	 * Get the value of a key/value pair as a string, va_list variant.
	 *
	 * @param def	default value if not found
	 * @param fmt	printf style format string for key, with sections
	 * @param args	arguments to fmt string
	 * @return		string
	 */
	char* (*vget_str)(vici_message_t *this, char *def, char *fmt, va_list args);

	/**
	 * Get the value of a key/value pair as integer.
	 *
	 * @param def	default value if not found
	 * @param fmt	printf style format string for key, with sections
	 * @param ...	arguments to fmt string
	 * @return		value
	 */
	int (*get_int)(vici_message_t *this, int def, char *fmt, ...);

	/**
	 * Get the value of a key/value pair as integer, va_list variant
	 *
	 * @param def	default value if not found
	 * @param fmt	printf style format string for key, with sections
	 * @param args	arguments to fmt string
	 * @return		value
	 */
	int (*vget_int)(vici_message_t *this, int def, char *fmt, va_list args);

	/**
	 * Get the raw value of a key/value pair.
	 *
	 * @param def	default value if not found
	 * @param fmt	printf style format string for key, with sections
	 * @param ...	arguments to fmt string
	 * @return		value
	 */
	chunk_t (*get_value)(vici_message_t *this, chunk_t def, char *fmt, ...);

	/**
	 * Get the raw value of a key/value pair, va_list variant.
	 *
	 * @param def	default value if not found
	 * @param fmt	printf style format string for key, with sections
	 * @param args	arguments to fmt string
	 * @return		value
	 */
	chunk_t (*vget_value)(vici_message_t *this, chunk_t def,
						 char *fmt, va_list args);

	/**
	 * Get encoded message.
	 *
	 * @return		message data, points to internal data
	 */
	chunk_t (*get_encoding)(vici_message_t *this);

	/**
	 * Parse a message using callback functions.
	 *
	 * Any of the callbacks may be NULL to skip this kind of item. Callbacks are
	 * invoked for the current section level only. To descent into sections,
	 * call parse() from within a section callback using the provided parse
	 * context.
	 *
	 * @param ctx		parse context, NULL for root level
	 * @param section	callback invoked for each section
	 * @param kv		callback invoked for key/value pairs
	 * @param li		callback invoked for list items
	 * @param user		user data to pass to callbacks
	 * @return			TRUE if parsed successfully
	 */
	bool (*parse)(vici_message_t *this, vici_parse_context_t *ctx,
				  vici_section_cb_t section, vici_value_cb_t kv,
				  vici_value_cb_t li, void *user);

	/**
	 * Dump a message text representation to a FILE stream.
	 *
	 * @param label		label to print for message
	 * @param pretty	use pretty print with indentation
	 * @param out		FILE stream to dump to
	 * @return			TRUE if message valid
	 */
	bool (*dump)(vici_message_t *this, char *label, bool pretty, FILE *out);

	/**
	 * Destroy a vici_message_t.
	 */
	void (*destroy)(vici_message_t *this);
};

/**
 * Create a vici_message from encoded data.
 *
 * @param data			message encoding
 * @param cleanup		TRUE to free data during
 * @return				message representation
 */
vici_message_t *vici_message_create_from_data(chunk_t data, bool cleanup);

/**
 * Create a vici_message from an enumerator.
 *
 * The enumerator uses the same signature as the enumerator returned
 * by create_enumerator(), and gets destroyed by this function. It should
 * return VICI_END to close the message, return FALSE to indicate a failure.
 *
 * @param enumerator	enumerator over (vici_type_t, char*, chunk_t)
 * @return				message representation, NULL on error
 */
vici_message_t *vici_message_create_from_enumerator(enumerator_t *enumerator);

/**
 * Create vici message from a variable argument list.
 *
 * @param type			first type beginning message
 * @param ...			vici_type_t and args, terminated by VICI_END
 * @return				message representation, NULL on error
 */
vici_message_t *vici_message_create_from_args(vici_type_t type, ...);

/**
 * Check if a chunk has a printable string, and print it to buf.
 *
 * @param chunk			chunk containing potential string
 * @param buf			buffer to write string to
 * @param size			size of buf
 * @return				TRUE if printable and string written to buf
 */
bool vici_stringify(chunk_t chunk, char *buf, size_t size);

/**
 * Verify the occurrence of a given type for given section/list nesting
 */
bool vici_verify_type(vici_type_t type, u_int section, bool list);

#endif /** VICI_MESSAGE_H_ @}*/
