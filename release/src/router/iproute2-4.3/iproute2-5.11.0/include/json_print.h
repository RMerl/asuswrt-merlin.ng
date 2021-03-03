/*
 * json_print.h		"print regular or json output, based on json_writer".
 *
 *             This program is free software; you can redistribute it and/or
 *             modify it under the terms of the GNU General Public License
 *             as published by the Free Software Foundation; either version
 *             2 of the License, or (at your option) any later version.
 *
 * Authors:    Julien Fortin, <julien@cumulusnetworks.com>
 */

#ifndef _JSON_PRINT_H_
#define _JSON_PRINT_H_

#include "json_writer.h"
#include "color.h"

#define _IS_JSON_CONTEXT(type) (is_json_context() && (type & PRINT_JSON || type & PRINT_ANY))
#define _IS_FP_CONTEXT(type)   (!is_json_context() && (type & PRINT_FP || type & PRINT_ANY))

json_writer_t *get_json_writer(void);

/*
 * use:
 *      - PRINT_ANY for context based output
 *      - PRINT_FP for non json specific output
 *      - PRINT_JSON for json specific output
 */
enum output_type {
	PRINT_FP = 1,
	PRINT_JSON = 2,
	PRINT_ANY = 4,
};

void new_json_obj(int json);
void delete_json_obj(void);
void new_json_obj_plain(int json);
void delete_json_obj_plain(void);

bool is_json_context(void);

void open_json_object(const char *str);
void close_json_object(void);
void open_json_array(enum output_type type, const char *delim);
void close_json_array(enum output_type type, const char *delim);

void print_nl(void);

#define _PRINT_FUNC(type_name, type)					\
	int print_color_##type_name(enum output_type t,			\
				    enum color_attr color,		\
				    const char *key,			\
				    const char *fmt,			\
				    type value);			\
									\
	static inline int print_##type_name(enum output_type t,		\
					    const char *key,		\
					    const char *fmt,		\
					    type value)			\
	{								\
		return print_color_##type_name(t, COLOR_NONE, key, fmt,	\
					       value);			\
	}

/* These functions return 0 if printing to a JSON context, number of
 * characters printed otherwise (as calculated by printf(3)).
 */
_PRINT_FUNC(int, int)
_PRINT_FUNC(s64, int64_t)
_PRINT_FUNC(bool, bool)
_PRINT_FUNC(on_off, bool)
_PRINT_FUNC(null, const char*)
_PRINT_FUNC(string, const char*)
_PRINT_FUNC(uint, unsigned int)
_PRINT_FUNC(size, __u32)
_PRINT_FUNC(u64, uint64_t)
_PRINT_FUNC(hhu, unsigned char)
_PRINT_FUNC(hu, unsigned short)
_PRINT_FUNC(hex, unsigned int)
_PRINT_FUNC(0xhex, unsigned long long)
_PRINT_FUNC(luint, unsigned long)
_PRINT_FUNC(lluint, unsigned long long)
_PRINT_FUNC(float, double)
#undef _PRINT_FUNC

#define _PRINT_NAME_VALUE_FUNC(type_name, type, format_char)		  \
	void print_##type_name##_name_value(const char *name, type value) \

_PRINT_NAME_VALUE_FUNC(uint, unsigned int, u);
_PRINT_NAME_VALUE_FUNC(string, const char*, s);
#undef _PRINT_NAME_VALUE_FUNC

int print_color_rate(bool use_iec, enum output_type t, enum color_attr color,
		     const char *key, const char *fmt, unsigned long long rate);

static inline int print_rate(bool use_iec, enum output_type t,
			     const char *key, const char *fmt,
			     unsigned long long rate)
{
	return print_color_rate(use_iec, t, COLOR_NONE, key, fmt, rate);
}

/* A backdoor to the size formatter. Please use print_size() instead. */
char *sprint_size(__u32 sz, char *buf);

#endif /* _JSON_PRINT_H_ */
