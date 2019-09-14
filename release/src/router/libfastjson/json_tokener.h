/*
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#ifndef _fj_json_tokener_h_
#define _fj_json_tokener_h_

#include <stddef.h>
#include "json_object.h"

#ifdef __cplusplus
extern "C" {
#endif

enum fjson_tokener_error {
	fjson_tokener_success,
	fjson_tokener_continue,
	fjson_tokener_error_depth,
	fjson_tokener_error_parse_eof,
	fjson_tokener_error_parse_unexpected,
	fjson_tokener_error_parse_null,
	fjson_tokener_error_parse_boolean,
	fjson_tokener_error_parse_number,
	fjson_tokener_error_parse_array,
	fjson_tokener_error_parse_object_key_name,
	fjson_tokener_error_parse_object_key_sep,
	fjson_tokener_error_parse_object_value_sep,
	fjson_tokener_error_parse_string,
	fjson_tokener_error_parse_comment,
	fjson_tokener_error_size
};

enum fjson_tokener_state {
	fjson_tokener_state_eatws,
	fjson_tokener_state_start,
	fjson_tokener_state_finish,
	fjson_tokener_state_null,
	fjson_tokener_state_comment_start,
	fjson_tokener_state_comment,
	fjson_tokener_state_comment_eol,
	fjson_tokener_state_comment_end,
	fjson_tokener_state_string,
	fjson_tokener_state_string_escape,
	fjson_tokener_state_escape_unicode,
	fjson_tokener_state_boolean,
	fjson_tokener_state_number,
	fjson_tokener_state_array,
	fjson_tokener_state_array_add,
	fjson_tokener_state_array_sep,
	fjson_tokener_state_object_field_start,
	fjson_tokener_state_object_field,
	fjson_tokener_state_object_field_end,
	fjson_tokener_state_object_value,
	fjson_tokener_state_object_value_add,
	fjson_tokener_state_object_sep,
	fjson_tokener_state_array_after_sep,
	fjson_tokener_state_object_field_start_after_sep,
	fjson_tokener_state_inf
};

struct fjson_tokener_srec
{
	enum fjson_tokener_state state, saved_state;
	struct fjson_object *obj;
	struct fjson_object *current;
	char *obj_field_name;
};

#define FJSON_TOKENER_DEFAULT_DEPTH 32

struct fjson_tokener
{
	char *str;
	struct printbuf *pb;
	int max_depth, depth, is_double, st_pos, char_offset;
	enum fjson_tokener_error err;
	unsigned int ucs_char;
	char quote_char;
	struct fjson_tokener_srec *stack;
	int flags;
};

/**
 * Be strict when parsing JSON input.  Use caution with
 * this flag as what is considered valid may become more
 * restrictive from one release to the next, causing your
 * code to fail on previously working input.
 *
 * This flag is not set by default.
 *
 * @see fjson_tokener_set_flags()
 */
#define FJSON_TOKENER_STRICT  0x01

/**
 * Given an error previously returned by fjson_tokener_get_error(),
 * return a human readable description of the error.
 *
 * @return a generic error message is returned if an invalid error value is provided.
 */
const char *fjson_tokener_error_desc(enum fjson_tokener_error jerr);

/**
 * Retrieve the error caused by the last call to fjson_tokener_parse_ex(),
 * or fjson_tokener_success if there is no error.
 *
 * When parsing a JSON string in pieces, if the tokener is in the middle
 * of parsing this will return fjson_tokener_continue.
 *
 * See also fjson_tokener_error_desc().
 */
enum fjson_tokener_error fjson_tokener_get_error(struct fjson_tokener *tok);

extern struct fjson_tokener* fjson_tokener_new(void);
extern struct fjson_tokener* fjson_tokener_new_ex(int depth);
extern void fjson_tokener_free(struct fjson_tokener *tok);
extern void fjson_tokener_reset(struct fjson_tokener *tok);
extern struct fjson_object* fjson_tokener_parse(const char *str);
extern struct fjson_object* fjson_tokener_parse_verbose(const char *str, enum fjson_tokener_error *error);

/**
 * Set flags that control how parsing will be done.
 */
extern void fjson_tokener_set_flags(struct fjson_tokener *tok, int flags);

/**
 * Parse a string and return a non-NULL fjson_object if a valid JSON value
 * is found.  The string does not need to be a JSON object or array;
 * it can also be a string, number or boolean value.
 *
 * A partial JSON string can be parsed.  If the parsing is incomplete,
 * NULL will be returned and fjson_tokener_get_error() will be return
 * fjson_tokener_continue.
 * fjson_tokener_parse_ex() can then be called with additional bytes in str
 * to continue the parsing.
 *
 * If fjson_tokener_parse_ex() returns NULL and the error anything other than
 * fjson_tokener_continue, a fatal error has occurred and parsing must be
 * halted.  Then tok object must not be re-used until fjson_tokener_reset() is
 * called.
 *
 * When a valid JSON value is parsed, a non-NULL fjson_object will be
 * returned.  Also, fjson_tokener_get_error() will return fjson_tokener_success.
 * Be sure to check the type with fjson_object_is_type() or
 * fjson_object_get_type() before using the object.
 *
 * @b XXX this shouldn't use internal fields:
 * Trailing characters after the parsed value do not automatically cause an
 * error.  It is up to the caller to decide whether to treat this as an
 * error or to handle the additional characters, perhaps by parsing another
 * json value starting from that point.
 *
 * Extra characters can be detected by comparing the tok->char_offset against
 * the length of the last len parameter passed in.
 *
 * The tokener does \b not maintain an internal buffer so the caller is
 * responsible for calling fjson_tokener_parse_ex with an appropriate str
 * parameter starting with the extra characters.
 *
 * This interface is presently not 64-bit clean due to the int len argument
 * so the function limits the maximum string size to INT32_MAX (2GB).
 * If the function is called with len == -1 then strlen is called to check
 * the string length is less than INT32_MAX (2GB)
 *
 * Example:
 * @code
fjson_object *jobj = NULL;
const char *mystring = NULL;
int stringlen = 0;
enum fjson_tokener_error jerr;
do {
	mystring = ...  // get JSON string, e.g. read from file, etc...
	stringlen = strlen(mystring);
	jobj = fjson_tokener_parse_ex(tok, mystring, stringlen);
} while ((jerr = fjson_tokener_get_error(tok)) == fjson_tokener_continue);
if (jerr != fjson_tokener_success)
{
	fprintf(stderr, "Error: %s\n", fjson_tokener_error_desc(jerr));
	// Handle errors, as appropriate for your application.
}
if (tok->char_offset < stringlen) // XXX shouldn't access internal fields
{
	// Handle extra characters after parsed object as desired.
	// e.g. issue an error, parse another object from that point, etc...
}
// Success, use jobj here.

@endcode
 *
 * @param tok a fjson_tokener previously allocated with fjson_tokener_new()
 * @param str an string with any valid JSON expression, or portion of.  This does not need to be null terminated.
 * @param len the length of str
 */
extern struct fjson_object* fjson_tokener_parse_ex(struct fjson_tokener *tok,
						 const char *str, int len);

#ifndef FJSON_NATIVE_API_ONLY
#define json_tokener fjson_tokener
#define json_tokener_error fjson_tokener_error
extern const char* fjson_tokener_errors[15];
#define json_tokener_errors fjson_tokener_errors
#define json_tokener_continue fjson_tokener_continue
#define json_tokener_reset fjson_tokener_reset

#define json_tokener_new() fjson_tokener_new()
#define json_tokener_parse fjson_tokener_parse
#define json_tokener_parse_ex(a, b, c) fjson_tokener_parse_ex((a), (b), (c))
#define json_tokener_free(a) fjson_tokener_free((a))
#define json_tokener_error_desc(a) fjson_tokener_error_desc((a))
#endif

#ifdef __cplusplus
}
#endif

#endif
