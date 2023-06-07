/*
 * json_print.c		"print regular or json output, based on json_writer".
 *
 *             This program is free software; you can redistribute it and/or
 *             modify it under the terms of the GNU General Public License
 *             as published by the Free Software Foundation; either version
 *             2 of the License, or (at your option) any later version.
 *
 * Authors:    Julien Fortin, <julien@cumulusnetworks.com>
 */

#include <stdarg.h>
#include <stdio.h>

#include "utils.h"
#include "json_print.h"

static json_writer_t *_jw;

static void __new_json_obj(int json, bool have_array)
{
	if (json) {
		_jw = jsonw_new(stdout);
		if (!_jw) {
			perror("json object");
			exit(1);
		}
		if (pretty)
			jsonw_pretty(_jw, true);
		if (have_array)
			jsonw_start_array(_jw);
	}
}

static void __delete_json_obj(bool have_array)
{
	if (_jw) {
		if (have_array)
			jsonw_end_array(_jw);
		jsonw_destroy(&_jw);
	}
}

void new_json_obj(int json)
{
	__new_json_obj(json, true);
}

void delete_json_obj(void)
{
	__delete_json_obj(true);
}

void new_json_obj_plain(int json)
{
	__new_json_obj(json, false);
}

void delete_json_obj_plain(void)
{
	__delete_json_obj(false);
}

bool is_json_context(void)
{
	return _jw != NULL;
}

json_writer_t *get_json_writer(void)
{
	return _jw;
}

void open_json_object(const char *str)
{
	if (_IS_JSON_CONTEXT(PRINT_JSON)) {
		if (str)
			jsonw_name(_jw, str);
		jsonw_start_object(_jw);
	}
}

void close_json_object(void)
{
	if (_IS_JSON_CONTEXT(PRINT_JSON))
		jsonw_end_object(_jw);
}

/*
 * Start json array or string array using
 * the provided string as json key (if not null)
 * or as array delimiter in non-json context.
 */
void open_json_array(enum output_type type, const char *str)
{
	if (_IS_JSON_CONTEXT(type)) {
		if (str)
			jsonw_name(_jw, str);
		jsonw_start_array(_jw);
	} else if (_IS_FP_CONTEXT(type)) {
		printf("%s", str);
	}
}

/*
 * End json array or string array
 */
void close_json_array(enum output_type type, const char *str)
{
	if (_IS_JSON_CONTEXT(type)) {
		jsonw_end_array(_jw);
	} else if (_IS_FP_CONTEXT(type)) {
		printf("%s", str);
	}
}

/*
 * pre-processor directive to generate similar
 * functions handling different types
 */
#define _PRINT_FUNC(type_name, type)					\
	__attribute__((format(printf, 4, 0)))				\
	int print_color_##type_name(enum output_type t,			\
				    enum color_attr color,		\
				    const char *key,			\
				    const char *fmt,			\
				    type value)				\
	{								\
		int ret = 0;						\
		if (_IS_JSON_CONTEXT(t)) {				\
			if (!key)					\
				jsonw_##type_name(_jw, value);		\
			else						\
				jsonw_##type_name##_field(_jw, key, value); \
		} else if (_IS_FP_CONTEXT(t)) {				\
			ret = color_fprintf(stdout, color, fmt, value); \
		}							\
		return ret;						\
	}
_PRINT_FUNC(int, int);
_PRINT_FUNC(s64, int64_t);
_PRINT_FUNC(hhu, unsigned char);
_PRINT_FUNC(hu, unsigned short);
_PRINT_FUNC(uint, unsigned int);
_PRINT_FUNC(u64, uint64_t);
_PRINT_FUNC(luint, unsigned long);
_PRINT_FUNC(lluint, unsigned long long);
_PRINT_FUNC(float, double);
#undef _PRINT_FUNC

#define _PRINT_NAME_VALUE_FUNC(type_name, type, format_char)		 \
	void print_##type_name##_name_value(const char *name, type value)\
	{								 \
		SPRINT_BUF(format);					 \
									 \
		snprintf(format, SPRINT_BSIZE,				 \
			 "%s %%"#format_char, name);			 \
		print_##type_name(PRINT_ANY, name, format, value);	 \
	}
_PRINT_NAME_VALUE_FUNC(uint, unsigned int, u);
_PRINT_NAME_VALUE_FUNC(string, const char*, s);
#undef _PRINT_NAME_VALUE_FUNC

int print_color_string(enum output_type type,
		       enum color_attr color,
		       const char *key,
		       const char *fmt,
		       const char *value)
{
	int ret = 0;

	if (_IS_JSON_CONTEXT(type)) {
		if (key && !value)
			jsonw_name(_jw, key);
		else if (!key && value)
			jsonw_string(_jw, value);
		else
			jsonw_string_field(_jw, key, value);
	} else if (_IS_FP_CONTEXT(type)) {
		ret = color_fprintf(stdout, color, fmt, value);
	}

	return ret;
}

/*
 * value's type is bool. When using this function in FP context you can't pass
 * a value to it, you will need to use "is_json_context()" to have different
 * branch for json and regular output. grep -r "print_bool" for example
 */
static int __print_color_bool(enum output_type type,
			      enum color_attr color,
			      const char *key,
			      const char *fmt,
			      bool value,
			      const char *str)
{
	int ret = 0;

	if (_IS_JSON_CONTEXT(type)) {
		if (key)
			jsonw_bool_field(_jw, key, value);
		else
			jsonw_bool(_jw, value);
	} else if (_IS_FP_CONTEXT(type)) {
		ret = color_fprintf(stdout, color, fmt, str);
	}

	return ret;
}

int print_color_bool(enum output_type type,
		     enum color_attr color,
		     const char *key,
		     const char *fmt,
		     bool value)
{
	return __print_color_bool(type, color, key, fmt, value,
				  value ? "true" : "false");
}

int print_color_on_off(enum output_type type,
		       enum color_attr color,
		       const char *key,
		       const char *fmt,
		       bool value)
{
	return __print_color_bool(type, color, key, fmt, value,
				  value ? "on" : "off");
}

/*
 * In JSON context uses hardcode %#x format: 42 -> 0x2a
 */
int print_color_0xhex(enum output_type type,
		      enum color_attr color,
		      const char *key,
		      const char *fmt,
		      unsigned long long hex)
{
	int ret = 0;

	if (_IS_JSON_CONTEXT(type)) {
		SPRINT_BUF(b1);

		snprintf(b1, sizeof(b1), "%#llx", hex);
		print_string(PRINT_JSON, key, NULL, b1);
	} else if (_IS_FP_CONTEXT(type)) {
		ret = color_fprintf(stdout, color, fmt, hex);
	}

	return ret;
}

int print_color_hex(enum output_type type,
		    enum color_attr color,
		    const char *key,
		    const char *fmt,
		    unsigned int hex)
{
	int ret = 0;

	if (_IS_JSON_CONTEXT(type)) {
		SPRINT_BUF(b1);

		snprintf(b1, sizeof(b1), "%x", hex);
		if (key)
			jsonw_string_field(_jw, key, b1);
		else
			jsonw_string(_jw, b1);
	} else if (_IS_FP_CONTEXT(type)) {
		ret = color_fprintf(stdout, color, fmt, hex);
	}

	return ret;
}

/*
 * In JSON context we don't use the argument "value" we simply call jsonw_null
 * whereas FP context can use "value" to output anything
 */
int print_color_null(enum output_type type,
		     enum color_attr color,
		     const char *key,
		     const char *fmt,
		     const char *value)
{
	int ret = 0;

	if (_IS_JSON_CONTEXT(type)) {
		if (key)
			jsonw_null_field(_jw, key);
		else
			jsonw_null(_jw);
	} else if (_IS_FP_CONTEXT(type)) {
		ret = color_fprintf(stdout, color, fmt, value);
	}

	return ret;
}

/* Print line separator (if not in JSON mode) */
void print_nl(void)
{
	if (!_jw)
		printf("%s", _SL_);
}

int print_color_rate(bool use_iec, enum output_type type, enum color_attr color,
		     const char *key, const char *fmt, unsigned long long rate)
{
	unsigned long kilo = use_iec ? 1024 : 1000;
	const char *str = use_iec ? "i" : "";
	static char *units[5] = {"", "K", "M", "G", "T"};
	char *buf;
	int rc;
	int i;

	if (_IS_JSON_CONTEXT(type))
		return print_color_lluint(type, color, key, "%llu", rate);

	rate <<= 3; /* bytes/sec -> bits/sec */

	for (i = 0; i < ARRAY_SIZE(units) - 1; i++)  {
		if (rate < kilo)
			break;
		if (((rate % kilo) != 0) && rate < 1000*kilo)
			break;
		rate /= kilo;
	}

	rc = asprintf(&buf, "%.0f%s%sbit", (double)rate, units[i],
		      i > 0 ? str : "");
	if (rc < 0)
		return -1;

	rc = print_color_string(type, color, key, fmt, buf);
	free(buf);
	return rc;
}
