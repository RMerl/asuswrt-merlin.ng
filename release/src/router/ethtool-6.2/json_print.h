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

bool is_json_context(void);

void fflush_fp(void);

void open_json_object(const char *str);
void close_json_object(void);
void open_json_array(const char *key, const char *str);
void close_json_array(const char *delim);

void print_nl(void);

#define _PRINT_FUNC(type_name, type)					\
	void print_##type_name(enum output_type t,			\
			       const char *key,				\
			       const char *fmt,				\
			       type value)				\

_PRINT_FUNC(int, int);
_PRINT_FUNC(s64, int64_t);
_PRINT_FUNC(bool, bool);
_PRINT_FUNC(null, const char*);
_PRINT_FUNC(string, const char*);
_PRINT_FUNC(uint, unsigned int);
_PRINT_FUNC(u64, uint64_t);
_PRINT_FUNC(hhu, unsigned char);
_PRINT_FUNC(hu, unsigned short);
_PRINT_FUNC(hex, unsigned int);
_PRINT_FUNC(0xhex, unsigned long long);
_PRINT_FUNC(luint, unsigned long);
_PRINT_FUNC(lluint, unsigned long long);
_PRINT_FUNC(float, double);
#undef _PRINT_FUNC

#endif /* _JSON_PRINT_H_ */
