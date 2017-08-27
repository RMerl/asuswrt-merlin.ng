/*
 * $Id: json_tokener.h,v 1.10 2006/07/25 03:24:50 Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#ifndef _json_tokener_h_
#define _json_tokener_h_

#include <stddef.h>
#include "json_object.h"

#ifdef __cplusplus
extern "C" {
#endif

enum json_tokener_error {
  json_tokener_success,
  json_tokener_continue,
  json_tokener_error_depth,
  json_tokener_error_parse_eof,
  json_tokener_error_parse_unexpected,
  json_tokener_error_parse_null,
  json_tokener_error_parse_boolean,
  json_tokener_error_parse_number,
  json_tokener_error_parse_array,
  json_tokener_error_parse_object_key_name,
  json_tokener_error_parse_object_key_sep,
  json_tokener_error_parse_object_value_sep,
  json_tokener_error_parse_string,
  json_tokener_error_parse_comment
};

enum json_tokener_state {
  json_tokener_state_eatws,
  json_tokener_state_start,
  json_tokener_state_finish,
  json_tokener_state_null,
  json_tokener_state_comment_start,
  json_tokener_state_comment,
  json_tokener_state_comment_eol,
  json_tokener_state_comment_end,
  json_tokener_state_string,
  json_tokener_state_string_escape,
  json_tokener_state_escape_unicode,
  json_tokener_state_boolean,
  json_tokener_state_number,
  json_tokener_state_array,
  json_tokener_state_array_add,
  json_tokener_state_array_sep,
  json_tokener_state_object_field_start,
  json_tokener_state_object_field,
  json_tokener_state_object_field_end,
  json_tokener_state_object_value,
  json_tokener_state_object_value_add,
  json_tokener_state_object_sep,
  json_tokener_state_array_after_sep,
  json_tokener_state_object_field_start_after_sep
};

struct json_tokener_srec
{
  enum json_tokener_state state, saved_state;
  struct json_object *obj;
  struct json_object *current;
  char *obj_field_name;
};

#define JSON_TOKENER_DEFAULT_DEPTH 32

struct json_tokener
{
  char *str;
  struct printbuf *pb;
  int max_depth, depth, is_double, st_pos, char_offset;
  enum json_tokener_error err;
  unsigned int ucs_char;
  char quote_char;
  struct json_tokener_srec *stack;
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
 * @see json_tokener_set_flags()
 */
#define JSON_TOKENER_STRICT  0x01

/**
 * Given an error previously returned by json_tokener_get_error(),
 * return a human readable description of the error.
 *
 * @return a generic error message is returned if an invalid error value is provided.
 */
const char *json_tokener_error_desc(enum json_tokener_error jerr);

extern const char* json_tokener_errors[];

/**
 * Retrieve the error caused by the last call to json_tokener_parse_ex(),
 * or json_tokener_success if there is no error.
 *
 * When parsing a JSON string in pieces, if the tokener is in the middle
 * of parsing this will return json_tokener_continue.
 *
 * See also json_tokener_error_desc().
 */
enum json_tokener_error json_tokener_get_error(struct json_tokener *tok);

extern struct json_tokener* json_tokener_new(void);
extern struct json_tokener* json_tokener_new_ex(int depth);
extern void json_tokener_free(struct json_tokener *tok);
extern void json_tokener_reset(struct json_tokener *tok);
extern struct json_object* json_tokener_parse(const char *str);
extern struct json_object* json_tokener_parse_verbose(const char *str, enum json_tokener_error *error);

/**
 * Set flags that control how parsing will be done.
 */
extern void json_tokener_set_flags(struct json_tokener *tok, int flags);

extern struct json_object* json_tokener_parse_ex(struct json_tokener *tok,
						 const char *str, int len);

#ifdef __cplusplus
}
#endif

#endif
