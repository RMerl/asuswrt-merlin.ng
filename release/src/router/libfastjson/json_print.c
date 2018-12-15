/*
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 * Copyright (c) 2009 Hewlett-Packard Development Company, L.P.
 * Copyright (c) 2015 Rainer Gerhards
 * Copyright (c) 2016 Copernica BV
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#include "config.h"

/* this is a work-around until we manage to fix configure.ac */
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef HAVE_STDARG_H
# include <stdarg.h>
#else /* !HAVE_STDARG_H */
# error Not enough var arg support!
#endif /* HAVE_STDARG_H */

#include "json_object.h"
#include "json_object_private.h"
#include "json_object_iterator.h"


#if !defined(HAVE_SNPRINTF)
# error You do not have snprintf on your system.
#endif /* HAVE_SNPRINTF */

#if !defined(HAVE_VASPRINTF)
/* CAW: compliant version of vasprintf */
/* Note: on OpenCSW, we have vasprintf() inside the headers, but not inside the lib.
 * So we need to use a different name, else we get issues with redefinitions. We
 * we solve this by using the macro below, which just renames the function BUT
 * does not affect the (variadic) arguments.
 * rgerhards, 2017-04-11
 */
#define  vasprintf rs_vasprintf
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
static int rs_vasprintf(char **buf, const char *fmt, va_list ap)
{
	int chars;
	char *b;
	static char _T_emptybuffer = '\0';

	if(!buf) { return -1; }

	/* CAW: RAWR! We have to hope to god here that vsnprintf doesn't overwrite
	   our buffer like on some 64bit sun systems.... but hey, its time to move on */
	chars = vsnprintf(&_T_emptybuffer, 0, fmt, ap)+1;
	if(chars < 0) { chars *= -1; } /* CAW: old glibc versions have this problem */

	b = (char*)malloc(sizeof(char)*chars);
	if(!b) { return -1; }

	if((chars = vsprintf(b, fmt, ap)) < 0) {
		free(b);
	} else {
		*buf = b;
	}

	return chars;
}
#pragma GCC diagnostic pop
#endif /* !HAVE_VASPRINTF */

/**
 *  Internal structure that we use for buffering the print output
 */
struct buffer {
	char *buffer;
	size_t size;
	size_t filled;
	fjson_write_fn *overflow;
	void *ptr;
};

/**
 *  Internal method to flush the buffer
 *  @param  buffer
 *  @return size_t
 */
static size_t buffer_flush(struct buffer *buffer)
{
	// call the user-supplied overflow function
	size_t result = buffer->overflow(buffer->ptr, buffer->buffer, buffer->filled);

	// buffer is empty now
	buffer->filled = 0;

	// done
	return result;
}

/**
 *  Internal method to append data to the buffer
 *  @param  buffer
 *  @param  data
 *  @param  size
 *  @return size_t
 */
static size_t buffer_append(struct buffer *buffer, const char *data, size_t size)
{
	// return value
	size_t result = 0;

	// is the data to big to fit in the buffer?
	if (buffer->filled + size > buffer->size)
	{
		// flush current buffer
		if (buffer->filled > 0) result += buffer_flush(buffer);

		// does it still not fit? then we pass it to the callback immediately
		if (size > buffer->size) return result + buffer->overflow(buffer->ptr, data, size);
	}

	// append to the buffer
	memcpy(buffer->buffer + buffer->filled, data, size);

	// update buffer size
	buffer->filled += size;

	// done
	return result;
}

/**
 *  Internal method to printf() into the buffer
 *  @param  buffer
 *  @param  format
 *  @param ...
 *  @return size_t
 */
__attribute__((__format__(__printf__, 2, 3)))
static size_t buffer_printf(struct buffer *buffer, const char *format, ...)
{
	// return value
	size_t result = 0;

	// variables used in this function
	va_list arguments;
	char *tmp;
	int size;

	// make sure we have sufficient room in our buffer
	if (buffer->size - buffer->filled < 32) result += buffer_flush(buffer);

	// initialize varargs
	va_start(arguments, format);

	// write to the buffer (note the extra char for the extra null that is written by vsnprintf())
	size = vsnprintf(buffer->buffer + buffer->filled, buffer->size - buffer->filled - 1, format, arguments);

	// clean up varargs (it is not possible to reuse the vararg arguments later on,
	// the have to be reset and possible reinitialized later on)
	va_end(arguments);

	// was this all successful?
	if (size >= 0 && size < (int)(buffer->size - buffer->filled))
	{
		// this was a major success
		buffer->filled += size;
	}
	else if (size > 0 && size < (int)buffer->size)
	{
		// there was not enough room in the buffer, but it would have been enough if
		// we would have been able to use the entire buffer, so we reset the buffer,
		// and retry the whole procedure
		result += buffer_flush(buffer);

		// buffer is empty now, we can retry, start with the vararg initialization
		va_start(arguments, format);

		// format into the buffer, again
		buffer->size += vsnprintf(buffer->buffer + buffer->filled,
			buffer->size - buffer->filled - 1, format, arguments);

		// clean up varargs
		va_end(arguments);
	}
	else
	{
		// initialize varargs
		va_start(arguments, format);

		// our own buffer is not big enough to fit the text, we are going to use
		// a dynamically allocated buffer using vasprintf(), init varargs first
		va_start(arguments, format);

		// use dynamically allocated vasprintf() call
		size = vasprintf(&tmp, format, arguments);

		// clean up varargs
		va_end(arguments);

		// was this a success?
		if (size > 0) result += buffer_append(buffer, tmp, size);

		// deallocate the memory
		if (size >= 0) free(tmp);
	}

	// done
	return result;
}

/* Forward declaration of the write function */
static size_t write(struct fjson_object *jso, int level, int flags, struct buffer *buffer);

/**
 *  helper for accessing the optimized string data component in fjson_object
 *  @param  jso
 *  @return
 */
static const char *get_string_component(struct fjson_object *jso)
{
	return (jso->o.c_string.len < LEN_DIRECT_STRING_DATA) ?
		   jso->o.c_string.str.data : jso->o.c_string.str.ptr;
}

/**
 *  string escaping
 *
 *  String escaping is a surprisingly performance intense operation.
 *  I spent many hours in the profiler, and the root problem seems
 *  to be that there is no easy way to detect the character classes
 *  that need to be escaped, where the root cause is that these
 *  characters are spread all over the ascii table. I tried
 *  several approaches, including call tables, re-structuring
 *  the case condition, different types of if conditions and
 *  reordering the if conditions. What worked out best is this:
 *  The regular case is that a character must not be escaped. So
 *  we want to process that as fast as possible. In order to
 *  detect this as quickly as possible, we have a lookup table
 *  that tells us if a char needs escaping ("needsEscape", below).
 *  This table has a spot for each ascii code. Note that it uses
 *  chars, because anything larger causes worse cache operation
 *  and anything smaller requires bit indexing and masking
 *  operations, which are also comparatively costly. So plain
 *  chars work best. What we then do is a single lookup into the
 *  table to detect if we need to escape a character. If we need,
 *  we go into the depth of actual escape detection. But if we
 *  do NOT need to escape, we just quickly advance the index
 *  and are done with that char. Note that it may look like the
 *  extra table lookup costs performance, but right the contrary
 *  is the case. We get amore than 30% performance increase due
 *  to it (compared to the latest version of the code that did not
 *  do the lookups).
 *  rgerhards@adiscon.com, 2015-11-18
 *  using now external char_needsEscape array. -- rgerhards, 2016-11-30
 */
extern const char char_needsEscape[256];

/**
 *  Function to escape a string
 *  @param  str     the string to be escaped
 *  @param  buffer  the internal buffer to write to
 *  @return size_t  number of bytes written
 */
static size_t escape(const char *str, struct buffer *buffer)
{
	size_t result = 0;
	const char *start_offset = str;
	while(1) { /* broken below on 0-byte */
		if(char_needsEscape[*((unsigned char*)str)]) {
			if(*str == '\0') break;
			if(str != start_offset) result += buffer_append(buffer, start_offset, str - start_offset);
			switch(*str) {
			case '\b':  result += buffer_append(buffer, "\\b", 2); break;
			case '\n':  result += buffer_append(buffer, "\\n", 2); break;
			case '\r':  result += buffer_append(buffer, "\\r", 2); break;
			case '\t':  result += buffer_append(buffer, "\\t", 2); break;
			case '\f':  result += buffer_append(buffer, "\\f", 2); break;
			case '"':   result += buffer_append(buffer, "\\\"", 2); break;
			case '\\':  result += buffer_append(buffer, "\\\\", 2); break;
			case '/':   result += buffer_append(buffer, "\\/", 2); break;
			default:
				result += buffer_printf(buffer, "\\u00%c%c",
					fjson_hex_chars[*str >> 4], fjson_hex_chars[*str & 0xf]);
				break;
			}
			start_offset = ++str;
		} else
			++str;
	}
	if(str != start_offset) result += buffer_append(buffer, start_offset, str - start_offset);
	return result;
}

/* add indentation */

static size_t indent(int level, int flags, struct buffer *buffer)
{
	// result variable, and loop counter
	size_t result = 0;
	int i;

	// skip if pretty-printing is not needed
	if (!(flags & FJSON_TO_STRING_PRETTY)) return 0;

	// iterate to add the spaces
	for (i = 0; i < level; ++i)
	{
		// write a tab or two spaces
		if (flags & FJSON_TO_STRING_PRETTY_TAB) result += buffer_append(buffer, "\t", 1);
		else result += buffer_append(buffer, "  ", 2);
	}

	// done
	return result;
}

/* write a json object */

static size_t write_object(struct fjson_object* jso, int level, int flags, struct buffer *buffer)
{
	int had_children = 0;
	size_t result = 0;

	result += buffer_append(buffer, "{" /*}*/, 1);
	if (flags & FJSON_TO_STRING_PRETTY) result += buffer_append(buffer, "\n", 1);
	struct fjson_object_iterator it = fjson_object_iter_begin(jso);
	struct fjson_object_iterator itEnd = fjson_object_iter_end(jso);
	while (!fjson_object_iter_equal(&it, &itEnd)) {
		if (had_children)
		{
			result += buffer_append(buffer, ",", 1);
			if (flags & FJSON_TO_STRING_PRETTY) result += buffer_append(buffer, "\n", 1);
		}
		had_children = 1;
		if (flags & FJSON_TO_STRING_SPACED) result += buffer_append(buffer, " ", 1);
		result += indent(level+1, flags, buffer);
		result += buffer_append(buffer, "\"", 1);
		result += escape(fjson_object_iter_peek_name(&it), buffer);
		if (flags & FJSON_TO_STRING_SPACED) result += buffer_append(buffer, "\": ", 3);
		else result += buffer_append(buffer, "\":", 2);
		result += write(fjson_object_iter_peek_value(&it), level+1, flags, buffer);
		fjson_object_iter_next(&it);
	}
	if (flags & FJSON_TO_STRING_PRETTY)
	{
		if (had_children) result += buffer_append(buffer, "\n", 1);
		result += indent(level, flags, buffer);
	}
	if (flags & FJSON_TO_STRING_SPACED) result += buffer_append(buffer, /*{*/ " }", 2);
	else result += buffer_append(buffer, /*{*/ "}", 1);
	return result;
}

/* write a json boolean */

static size_t write_boolean(struct fjson_object* jso, struct buffer *buffer)
{
	if (jso->o.c_boolean) return buffer_append(buffer, "true", 4);
	else return buffer_append(buffer, "false", 5);
}

/* write a json int */

static size_t write_int(struct fjson_object* jso, struct buffer *buffer)
{
	// printf into the buffer
	return buffer_printf(buffer, "%" PRId64, jso->o.c_int64);
}

/* write a json floating point */

static size_t write_double(struct fjson_object* jso, int flags, struct buffer *buffer)
{
	// return value for the function
	size_t result = 0;

	// helper functions to fix the output
	char *buf, *p, *q;

	// needed for modf()
	double dummy;

	// if the original value is set, we reuse that
	if (jso->o.c_double.source) return buffer_append(buffer, jso->o.c_double.source, strlen(jso->o.c_double.source));

	/* Although JSON RFC does not support
	 * NaN or Infinity as numeric values
	 * ECMA 262 section 9.8.1 defines
	 * how to handle these cases as strings
	 */
	if(isnan(jso->o.c_double.value)) return buffer_append(buffer, "NaN", 3);
	if(isinf(jso->o.c_double.value)) return buffer_printf(buffer, jso->o.c_double.value > 0 ? "Infinity" : "-Infinity");

	// store the beginning of the buffer (this is where buffer_printf() will most likely write)
	buf = buffer->buffer + buffer->filled;

	// write to the buffer
	result = buffer_printf(buffer, (modf(jso->o.c_double.value, &dummy)==0)?"%.17g.0":"%.17g", jso->o.c_double.value);

	// if the buffer got flushed
	if (buffer->buffer + buffer->filled < buf) buf = buffer->buffer;

	// if localization stuff caused "," to be generated instead of "."
	// @todo is there not a nicer way to work around that???
	p = strchr(buf, ',');
	if (p) {
		*p = '.';
	} else {
		p = strchr(buf, '.');
	}

	// remove trailing zero's
	if (p && (flags & FJSON_TO_STRING_NOZERO)) {
		/* last useful digit, always keep 1 zero */
		p++;
		for (q=p ; *q ; q++) {
			if (*q!='0') p=q;
		}
		/* drop trailing zeroes */
		buffer->filled = p - buffer->buffer;
	}

	// done
	return result;
}

/* write a json string */

static size_t write_string(struct fjson_object* jso, struct buffer *buffer)
{
	return buffer_append(buffer, "\"", 1) + escape(get_string_component(jso), buffer) + buffer_append(buffer, "\"", 1);
}

/* write a json array */

static size_t write_array(struct fjson_object* jso, int level, int flags, struct buffer *buffer)
{
	int had_children = 0;
	int ii;
	size_t result = 0;
	result += buffer_append(buffer, "[", 1);
	if (flags & FJSON_TO_STRING_PRETTY) result += buffer_append(buffer, "\n", 1);
	for(ii=0; ii < fjson_object_array_length(jso); ii++)
	{
		if (had_children)
		{
			result += buffer_append(buffer, ",", 1);
			if (flags & FJSON_TO_STRING_PRETTY) result += buffer_append(buffer, "\n", 1);
		}
		had_children = 1;
		if (flags & FJSON_TO_STRING_SPACED) result += buffer_append(buffer, " ", 1);
		result += indent(level + 1, flags, buffer);
		result += write(fjson_object_array_get_idx(jso, ii), level+1, flags, buffer);
	}
	if (flags & FJSON_TO_STRING_PRETTY)
	{
		if (had_children) result += buffer_append(buffer, "\n", 1);
		result += indent(level, flags, buffer);
	}

	if (flags & FJSON_TO_STRING_SPACED) result += buffer_append(buffer, " ]", 2);
	else result += buffer_append(buffer, "]", 1);
	return result;
}

/* write a json value */

static size_t write(struct fjson_object *jso, int level, int flags, struct buffer *buffer)
{
	// if object is not set
	if (!jso) return buffer_append(buffer, "null", 4);

	// check type
	switch(jso->o_type) {
	case fjson_type_null:       return buffer_append(buffer, "null", 4);
	case fjson_type_boolean:    return write_boolean(jso, buffer);
	case fjson_type_double:     return write_double(jso, flags, buffer);
	case fjson_type_int:        return write_int(jso, buffer);
	case fjson_type_object:     return write_object(jso, level, flags, buffer);
	case fjson_type_array:      return write_array(jso, level, flags, buffer);
	case fjson_type_string:     return write_string(jso, buffer);
	default:                    return 0;
	}
}

/* wrapper around fwrite() that has the same signature as fjson_write_fn */

static size_t fwrite_wrapper(void *ptr, const char *buffer, size_t size)
{
	return fwrite(buffer, 1, size, ptr);
}

/* dummy output function that does not output, but is used to calculate the size */

static size_t calculate(void __attribute__((unused)) *ptr, const char __attribute__((unused)) *buffer, size_t size)
{
	return size;
}

/* extended dump to which the helper buffer can be passed */

size_t fjson_object_dump_buffered(struct fjson_object *jso, int flags, char *temp,
size_t size, fjson_write_fn *func, void *ptr)
{
	// construct a buffer
	struct buffer object;

	// initialize the properties
	object.buffer = temp;
	object.size = size;
	object.filled = 0;
	object.overflow = func;
	object.ptr = ptr;

	// write the value
	size_t result = write(jso, 0, flags, &object);

	// ready if buffer is now empty
	if (object.size == 0) return result;

	// flush the buffer
	return result + buffer_flush(&object);
}

/* extended dump function to string */

size_t fjson_object_dump_ext(struct fjson_object *jso, int flags, fjson_write_fn *func, void *ptr)
{
	// create a local 1k buffer on the stack
	char buffer[1024];

	// pass on to the other function
	return fjson_object_dump_buffered(jso, flags, buffer, 1024, func, ptr);
}

/* more simple write function */

size_t fjson_object_dump(struct fjson_object *jso, fjson_write_fn *func, void *ptr)
{
	// write the value
	return fjson_object_dump_ext(jso, FJSON_TO_STRING_SPACED, func, ptr);
}

/* extended function to calculate the size */

size_t fjson_object_size_ext(struct fjson_object *jso, int flags)
{
	// write the value with a dummy function (this is a simple implementation that
	// can later be optimized in a pure size-calculating function)
	return fjson_object_dump_ext(jso, flags, &calculate, NULL);
}

/* function to calculate the size */

size_t fjson_object_size(struct fjson_object *jso)
{
	// write the value with a dummy function (this is a simple implementation that
	// can later be optimized in a pure size-calculating function)
	return fjson_object_dump(jso, &calculate, NULL);
}

/* write to a file* */

size_t fjson_object_write(struct fjson_object *obj, FILE *fp)
{
	return fjson_object_dump_ext(obj, FJSON_TO_STRING_SPACED, fwrite_wrapper, fp);
}

/* write to a file with custom output flags */

size_t fjson_object_write_ext(struct fjson_object *obj, int flags, FILE *fp)
{
	return fjson_object_dump_ext(obj, flags, fwrite_wrapper, fp);
}

