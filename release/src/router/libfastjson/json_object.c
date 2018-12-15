/*
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 * Copyright (c) 2009 Hewlett-Packard Development Company, L.P.
 * Copyright (c) 2015-2017 Rainer Gerhards
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#include "config.h"

/* this is a work-around until we manage to fix configure.ac */
#ifndef _AIX
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <assert.h>
#include <stdint.h>

#include "debug.h"
#include "atomic.h"
#include "printbuf.h"
#include "arraylist.h"
#include "json.h"
#include "json_object.h"
#include "json_object_private.h"
#include "json_object_iterator.h"
#include "json_util.h"

#if !defined(HAVE_STRDUP)
# error You do not have strdup on your system.
#endif /* HAVE_STRDUP */

#if !defined(HAVE_SNPRINTF)
# error You do not have snprintf on your system.
#endif /* HAVE_SNPRINTF */

const char *fjson_number_chars = "0123456789.+-eE";
const char *fjson_hex_chars = "0123456789abcdefABCDEF";

static void fjson_object_generic_delete(struct fjson_object* jso);
static struct fjson_object* fjson_object_new(enum fjson_type o_type);

static fjson_object_to_json_string_fn fjson_object_object_to_json_string;
static fjson_object_to_json_string_fn fjson_object_boolean_to_json_string;
static fjson_object_to_json_string_fn fjson_object_int_to_json_string;
static fjson_object_to_json_string_fn fjson_object_double_to_json_string;
static fjson_object_to_json_string_fn fjson_object_string_to_json_string;
static fjson_object_to_json_string_fn fjson_object_array_to_json_string;

static int do_case_sensitive_comparison = 1;
void fjson_global_do_case_sensitive_comparison(const int newval)
{
	do_case_sensitive_comparison = newval;
}

/* helper for accessing the optimized string data component in fjson_object
 */
static const char *
get_string_component(struct fjson_object *jso)
{
	return (jso->o.c_string.len < LEN_DIRECT_STRING_DATA) ?
		   jso->o.c_string.str.data : jso->o.c_string.str.ptr;
}

/* string escaping
 *
 * String escaping is a surprisingly performance intense operation.
 * I spent many hours in the profiler, and the root problem seems
 * to be that there is no easy way to detect the character classes
 * that need to be escaped, where the root cause is that these
 * characters are spread all over the ascii table. I tried
 * several approaches, including call tables, re-structuring
 * the case condition, different types of if conditions and
 * reordering the if conditions. What worked out best is this:
 * The regular case is that a character must not be escaped. So
 * we want to process that as fast as possible. In order to
 * detect this as quickly as possible, we have a lookup table
 * that tells us if a char needs escaping ("needsEscape", below).
 * This table has a spot for each ascii code. Note that it uses
 * chars, because anything larger causes worse cache operation
 * and anything smaller requires bit indexing and masking
 * operations, which are also comparatively costly. So plain
 * chars work best. What we then do is a single lookup into the
 * table to detect if we need to escape a character. If we need,
 * we go into the depth of actual escape detection. But if we
 * do NOT need to escape, we just quickly advance the index
 * and are done with that char. Note that it may look like the
 * extra table lookup costs performance, but right the contrary
 * is the case. We get amore than 30% performance increase due
 * to it (compared to the latest version of the code that did not
 * do the lookups).
 * rgerhards@adiscon.com, 2015-11-18
 * I have renamed needsEscape to char_needsEscape and made it
 * external. This makes it possible to share the implementation
 * with the newly contributed json_print.c module.
 * rgerhards, 2016-11-30
 */

char char_needsEscape[256] = {
	1, 1, 1, 1, 1, 1, 1, 1, /* ascii codes 0 .. 7 */
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 1, 0, 0, 0, 0, 0, /* ascii codes 32 .. 39 */
	0, 0, 0, 0, 0, 0, 0, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

static void fjson_escape_str(struct printbuf *pb, const char *str)
{
	const char *start_offset = str;
	while(1) { /* broken below on 0-byte */
		if(char_needsEscape[*((unsigned char*)str)]) {
			if(*str == '\0')
				break;
			if(str != start_offset)
				printbuf_memappend_no_nul(pb, start_offset, str - start_offset);
			switch(*str) {
			case '\b': printbuf_memappend_no_nul(pb, "\\b", 2);
				break;
			case '\n': printbuf_memappend_no_nul(pb, "\\n", 2);
				break;
			case '\r': printbuf_memappend_no_nul(pb, "\\r", 2);
				break;
			case '\t': printbuf_memappend_no_nul(pb, "\\t", 2);
				break;
			case '\f': printbuf_memappend_no_nul(pb, "\\f", 2);
				break;
			case '"': printbuf_memappend_no_nul(pb, "\\\"", 2);
				break;
			case '\\': printbuf_memappend_no_nul(pb, "\\\\", 2);
				break;
			case '/': printbuf_memappend_no_nul(pb, "\\/", 2);
				break;
			default: sprintbuf(pb, "\\u00%c%c",
				fjson_hex_chars[*str >> 4],
				fjson_hex_chars[*str & 0xf]);
				break;
			}
			start_offset = ++str;
		} else
			++str;
	}
	if(str != start_offset)
		printbuf_memappend_no_nul(pb, start_offset, str - start_offset);
}


/* reference counting */

extern struct fjson_object* fjson_object_get(struct fjson_object *jso)
{
	if (!jso) return jso;
	ATOMIC_INC_AND_FETCH_int(&jso->_ref_count, &jso->_mut_ref_count);
	return jso;
}

int fjson_object_put(struct fjson_object *jso)
{
	if(!jso) return 0;

	const int cnt = ATOMIC_DEC_AND_FETCH(&jso->_ref_count, &jso->_mut_ref_count);
	if(cnt > 0) return 0;

	jso->_delete(jso);
	return 1;
}


/* generic object construction and destruction parts */

static void fjson_object_generic_delete(struct fjson_object* jso)
{
	if (jso) {
		printbuf_free(jso->_pb);
		DESTROY_ATOMIC_HELPER_MUT(jso->_mut_ref_count);
		free(jso);
	}
}

static struct fjson_object* fjson_object_new(const enum fjson_type o_type)
{
	struct fjson_object *const jso = (struct fjson_object*)calloc(sizeof(struct fjson_object), 1);
	if (!jso)
		return NULL;
	jso->o_type = o_type;
	jso->_ref_count = 1;
	jso->_delete = &fjson_object_generic_delete;
	INIT_ATOMIC_HELPER_MUT(jso->_mut_ref_count);
	return jso;
}


/* type checking functions */

int fjson_object_is_type(struct fjson_object *jso, enum fjson_type type)
{
	if (!jso)
		return (type == fjson_type_null);
	return (jso->o_type == type);
}

enum fjson_type fjson_object_get_type(struct fjson_object *jso)
{
	if (!jso)
		return fjson_type_null;
	return jso->o_type;
}

/* extended conversion to string */

const char* fjson_object_to_json_string_ext(struct fjson_object *jso, int flags)
{
	if (!jso)
		return "null";

	if ((!jso->_pb) && !(jso->_pb = printbuf_new()))
		return NULL;

	printbuf_reset(jso->_pb);

	jso->_to_json_string(jso, jso->_pb, 0, flags);

	printbuf_terminate_string(jso->_pb);
	return jso->_pb->buf;
}

/* backwards-compatible conversion to string */

const char* fjson_object_to_json_string(struct fjson_object *jso)
{
	return fjson_object_to_json_string_ext(jso, FJSON_TO_STRING_SPACED);
}

static void indent(struct printbuf *pb, int level, int flags)
{
	if (flags & FJSON_TO_STRING_PRETTY)
	{
		if (flags & FJSON_TO_STRING_PRETTY_TAB)
		{
			printbuf_memset(pb, -1, '\t', level);
		}
		else
		{
			printbuf_memset(pb, -1, ' ', level * 2);
		}
	}
}

/* fjson_object_object */

static int fjson_object_object_to_json_string(struct fjson_object* jso,
						 struct printbuf *pb,
						 int level,
						 int flags)
{
	struct fjson_object *val;
	int had_children = 0;

	printbuf_memappend_char(pb, '{' /*}*/);
	if (flags & FJSON_TO_STRING_PRETTY)
		printbuf_memappend_char(pb, '\n');
	struct fjson_object_iterator it = fjson_object_iter_begin(jso);
	struct fjson_object_iterator itEnd = fjson_object_iter_end(jso);
	while (!fjson_object_iter_equal(&it, &itEnd)) {
		if (had_children)
		{
			printbuf_memappend_char(pb, ',');
			if (flags & FJSON_TO_STRING_PRETTY)
				printbuf_memappend_char(pb, '\n');
		}
		had_children = 1;
		if (flags & FJSON_TO_STRING_SPACED)
			printbuf_memappend_char(pb, ' ');
		indent(pb, level+1, flags);
		printbuf_memappend_char(pb, '\"');
		fjson_escape_str(pb, fjson_object_iter_peek_name(&it));
		if (flags & FJSON_TO_STRING_SPACED)
			printbuf_memappend_no_nul(pb, "\": ", 3);
		else
			printbuf_memappend_no_nul(pb, "\":", 2);
		val = fjson_object_iter_peek_value(&it);
		if(val == NULL)
			printbuf_memappend_no_nul(pb, "null", 4);
		else
			val->_to_json_string(val, pb, level+1,flags);
		fjson_object_iter_next(&it);
	}
	if (flags & FJSON_TO_STRING_PRETTY)
	{
		if (had_children)
			printbuf_memappend_no_nul(pb, "\n",1);
		indent(pb,level,flags);
	}
	if (flags & FJSON_TO_STRING_SPACED)
		printbuf_memappend_no_nul(pb, /*{*/ " }", 2);
	else
		printbuf_memappend_char(pb, /*{*/ '}');
	return 0; /* we need to keep compatible with the API */
}


static void fjson_object_object_delete(struct fjson_object *const __restrict__ jso)
{
	struct _fjson_child_pg *pg = &jso->o.c_obj.pg;
	struct _fjson_child_pg *del = NULL; /* do NOT delete first elt! */
	while (pg != NULL) {
		for (int i = 0 ; i < FJSON_OBJECT_CHLD_PG_SIZE ; ++i) {
			if (pg->children[i].k == NULL)
				continue; /* indicates empty slot */
			if(!pg->children[i].flags.k_is_constant)
				free ((void*)pg->children[i].k);
			fjson_object_put (pg->children[i].v);
		}
		pg = pg->next;
		free(del);
		del = pg;
	}
	fjson_object_generic_delete(jso);
}

struct fjson_object* fjson_object_new_object(void)
{
	struct fjson_object *jso = fjson_object_new(fjson_type_object);
	if (!jso)
		return NULL;
	jso->_delete = &fjson_object_object_delete;
	jso->_to_json_string = &fjson_object_object_to_json_string;
	jso->o.c_obj.nelem = 0;
	jso->o.c_obj.lastpg = &jso->o.c_obj.pg;
	return jso;
}


/* finds the child with given key if it exists in a json object
 * and returns a pointer to it. Returns NULL if not found.
 */
static struct _fjson_child*
_fjson_find_child(struct fjson_object *const __restrict__ jso,
	const char *const key)
{
	struct fjson_object_iterator it = fjson_object_iter_begin(jso);
	struct fjson_object_iterator itEnd = fjson_object_iter_end(jso);
	while (!fjson_object_iter_equal(&it, &itEnd)) {
		if (do_case_sensitive_comparison) {
			if (!strcmp (key, fjson_object_iter_peek_name(&it)))
				return _fjson_object_iter_peek_child(&it);
		} else {
			if (!strcasecmp (key, fjson_object_iter_peek_name(&it)))
				return _fjson_object_iter_peek_child(&it);
		}
		fjson_object_iter_next(&it);
	}
	return NULL;
}

/* get an empty entry/slot for adding a new child. If the current data
 * structure is full, alloc a new page. Returns NULL on (malloc) error.
 */
static struct _fjson_child *
fjson_child_get_empty_etry(struct fjson_object *const __restrict__ jso)
{
	struct _fjson_child *chld = NULL;
	struct _fjson_child_pg *pg;

	if (jso->o.c_obj.ndeleted > 0) {
		/* we first fill deleted spots */
		pg = &jso->o.c_obj.pg;
		while (chld == NULL) {
			for (int i = 0 ; i < FJSON_OBJECT_CHLD_PG_SIZE ; ++i) {
				if(pg->children[i].k == NULL) {
					chld = &(pg->children[i]);
					--jso->o.c_obj.ndeleted;
					goto done;
				}
			}
			pg = pg->next;
		}
		/* if we reach this point, we have a program error */
		assert(0);
		goto done;
	}

	const int pg_idx = jso->o.c_obj.nelem % FJSON_OBJECT_CHLD_PG_SIZE;
	if (jso->o.c_obj.nelem > 0 && pg_idx == 0) {
		if((pg = calloc(1, sizeof(struct _fjson_child_pg))) == NULL) {
			errno = ENOMEM;
			goto done;
		}
		jso->o.c_obj.lastpg->next = pg;
		jso->o.c_obj.lastpg = pg;
	}
	pg = jso->o.c_obj.lastpg;
	if (pg->children[pg_idx].k == NULL) {
		/* we can use this spot and save us search time */
		chld = &(pg->children[pg_idx]);
		goto done;
	}

done:	return chld;
}

void fjson_object_object_add_ex(struct fjson_object *const __restrict__ jso,
	const char *const key,
	struct fjson_object *const val,
	const unsigned opts)
{
	// We lookup the entry and replace the value, rather than just deleting
	// and re-adding it, so the existing key remains valid.
	struct _fjson_child *chld;
	chld = (opts & FJSON_OBJECT_ADD_KEY_IS_NEW) ? NULL : _fjson_find_child(jso, key);
	if (chld != NULL) {
		if (chld->v != NULL)
			fjson_object_put(chld->v);
		chld->v = val;
		goto done;
	}

	/* insert new entry */
	if ((chld = fjson_child_get_empty_etry(jso)) == NULL)
		goto done;
	chld->k = (opts & FJSON_OBJECT_KEY_IS_CONSTANT) ? key : strdup(key);
	chld->flags.k_is_constant = (opts & FJSON_OBJECT_KEY_IS_CONSTANT) != 0;
	chld->v = val;
	++jso->o.c_obj.nelem;

done:
	return;
}

void fjson_object_object_add(struct fjson_object *const __restrict__ jso,
	const char *const key,
	struct fjson_object *const val)
{
	fjson_object_object_add_ex(jso, key, val, 0);
}


int fjson_object_object_length(struct fjson_object *jso)
{
	return jso->o.c_obj.nelem;
}

struct fjson_object* fjson_object_object_get(struct fjson_object* jso, const char *key)
{
	struct fjson_object *result = NULL;
	fjson_object_object_get_ex(jso, key, &result);
	return result;
}

fjson_bool fjson_object_object_get_ex(struct fjson_object* jso, const char *key, struct fjson_object **value)
{
	if (value != NULL)
		*value = NULL;

	if (NULL == jso)
		return FALSE;

	if(jso->o_type == fjson_type_object) {
		struct _fjson_child *const chld = _fjson_find_child(jso, key);
		if (chld == 0) {
			return FALSE;
		} else {
			if (value != NULL)
				*value = chld->v;
			return TRUE;
		}
	} else {
		if (value != NULL)
			*value = NULL;
		return FALSE;
	}
}

void fjson_object_object_del(struct fjson_object* jso, const char *key)
{
	struct _fjson_child *const chld = _fjson_find_child(jso, key);
	if (chld != NULL) {
		if(!chld->flags.k_is_constant) {
			free((void*)chld->k);
		}
		fjson_object_put(chld->v);
		chld->flags.k_is_constant = 0;
		chld->k = NULL;
		chld->v = NULL;
		--jso->o.c_obj.nelem;
		++jso->o.c_obj.ndeleted;
	}
}


/* fjson_object_boolean */

static int fjson_object_boolean_to_json_string(struct fjson_object* jso,
						  struct printbuf *pb,
						  int __attribute__((unused)) level,
						  int __attribute__((unused)) flags)
{
	if (jso->o.c_boolean)
		printbuf_memappend_no_nul(pb, "true", 4);
	else
		printbuf_memappend_no_nul(pb, "false", 5);
	return 0; /* we need to keep compatible with the API */
}

struct fjson_object* fjson_object_new_boolean(fjson_bool b)
{
	struct fjson_object *jso = fjson_object_new(fjson_type_boolean);
	if (!jso)
		return NULL;
	jso->_to_json_string = &fjson_object_boolean_to_json_string;
	jso->o.c_boolean = b;
	return jso;
}

fjson_bool fjson_object_get_boolean(struct fjson_object *jso)
{
	if (!jso)
		return FALSE;
	switch(jso->o_type)
	{
	case fjson_type_boolean:
		return jso->o.c_boolean;
	case fjson_type_int:
		return (jso->o.c_int64 != 0);
	case fjson_type_double:
		return (jso->o.c_double.value != 0);
	case fjson_type_string:
		return (jso->o.c_string.len != 0);
	case fjson_type_null:
	case fjson_type_object:
	case fjson_type_array:
	default:
		return FALSE;
	}
}


/* fjson_object_int */

static int fjson_object_int_to_json_string(struct fjson_object* jso,
					  struct printbuf *pb,
					  int __attribute__((unused)) level,
					  int __attribute__((unused)) flags)
{
	sprintbuf(pb, "%" PRId64, jso->o.c_int64);
	return 0; /* we need to keep compatible with the API */
}

struct fjson_object* fjson_object_new_int(int32_t i)
{
	struct fjson_object *jso = fjson_object_new(fjson_type_int);
	if (!jso)
		return NULL;
	jso->_to_json_string = &fjson_object_int_to_json_string;
	jso->o.c_int64 = i;
	return jso;
}

int32_t fjson_object_get_int(struct fjson_object *jso)
{
	int64_t cint64;
	enum fjson_type o_type;

	if(!jso) return 0;

	o_type = jso->o_type;
	cint64 = jso->o.c_int64;

	if (o_type == fjson_type_string) {
		/*
		 * Parse strings into 64-bit numbers, then use the
		 * 64-to-32-bit number handling below.
		 */
		if (fjson_parse_int64(get_string_component(jso), &cint64) != 0)
			return 0; /* whoops, it didn't work. */
		o_type = fjson_type_int;
	}

	switch(o_type) {
	case fjson_type_int:
		/* Make sure we return the correct values for out of range numbers. */
		if (cint64 <= INT32_MIN)
			return INT32_MIN;
		else if (cint64 >= INT32_MAX)
			return INT32_MAX;
		else
			return (int32_t)cint64;
	case fjson_type_double:
		return (int32_t)jso->o.c_double.value;
	case fjson_type_boolean:
		return jso->o.c_boolean;
	case fjson_type_null:
	case fjson_type_object:
	case fjson_type_array:
	case fjson_type_string:
	default:
		return 0;
	}
}

struct fjson_object* fjson_object_new_int64(int64_t i)
{
	struct fjson_object *jso = fjson_object_new(fjson_type_int);
	if (!jso)
		return NULL;
	jso->_to_json_string = &fjson_object_int_to_json_string;
	jso->o.c_int64 = i;
	return jso;
}

int64_t fjson_object_get_int64(struct fjson_object *jso)
{
	int64_t cint;

	if (!jso)
		return 0;
	switch(jso->o_type)
	{
	case fjson_type_int:
		return jso->o.c_int64;
	case fjson_type_double:
		return (int64_t)jso->o.c_double.value;
	case fjson_type_boolean:
		return jso->o.c_boolean;
	case fjson_type_string:
		if (fjson_parse_int64(get_string_component(jso), &cint) == 0)
			return cint;
		ATTR_FALLTHROUGH
	case fjson_type_null:
	case fjson_type_object:
	case fjson_type_array:
	default:
		return 0;
	}
}


/* fjson_object_double */

static int fjson_object_double_to_json_string(struct fjson_object* jso,
						 struct printbuf *pb,
						 int __attribute__((unused)) level,
						 int __attribute__((unused)) flags)
{
	char buf[128], *p, *q;
	int size;
	double dummy;  /* needed for modf() */
	
	if (jso->o.c_double.source) {
		printbuf_memappend_no_nul(pb, jso->o.c_double.source, strlen(jso->o.c_double.source));
		return 0; /* we need to keep compatible with the API */
	}
	
	/* Although JSON RFC does not support
	 * NaN or Infinity as numeric values
	 * ECMA 262 section 9.8.1 defines
	 * how to handle these cases as strings
	 */
	if(isnan(jso->o.c_double.value))
		size = snprintf(buf, sizeof(buf), "NaN");
	else if(isinf(jso->o.c_double.value))
		if(jso->o.c_double.value > 0)
			size = snprintf(buf, sizeof(buf), "Infinity");
		else
			size = snprintf(buf, sizeof(buf), "-Infinity");
	else
		size = snprintf(buf, sizeof(buf),
			(modf(jso->o.c_double.value, &dummy)==0)?"%.17g.0":"%.17g",
			jso->o.c_double.value);

	p = strchr(buf, ',');
	if (p) {
		*p = '.';
	} else {
		p = strchr(buf, '.');
	}
	if (p && (flags & FJSON_TO_STRING_NOZERO)) {
		/* last useful digit, always keep 1 zero */
		p++;
		for (q=p ; *q ; q++) {
			if (*q!='0') p=q;
		}
		/* drop trailing zeroes */
		*(++p) = 0;
		size = p-buf;
	}
	printbuf_memappend_no_nul(pb, buf, size);
	return 0; /* we need to keep compatible with the API */
}

static void fjson_object_double_delete(struct fjson_object *jso)
{
	free(jso->o.c_double.source);
	fjson_object_generic_delete(jso);
}

struct fjson_object* fjson_object_new_double(double d)
{
	struct fjson_object *jso = fjson_object_new(fjson_type_double);
	if (!jso)
		return NULL;
	jso->_to_json_string = &fjson_object_double_to_json_string;
	jso->o.c_double.value = d;
	jso->o.c_double.source = NULL;
	return jso;
}

struct fjson_object* fjson_object_new_double_s(double d, const char *ds)
{
	struct fjson_object *jso = fjson_object_new_double(d);
	if (!jso)
		return NULL;

	jso->o.c_double.source = strdup(ds);
	if (!jso->o.c_double.source)
	{
		fjson_object_generic_delete(jso);
		errno = ENOMEM;
		return NULL;
	}
	jso->_delete = &fjson_object_double_delete;
	return jso;
}

double fjson_object_get_double(struct fjson_object *jso)
{
	double cdouble;
	char *errPtr = NULL;

	if(!jso) return 0.0;
	switch(jso->o_type) {
	case fjson_type_double:
		return jso->o.c_double.value;
	case fjson_type_int:
		return jso->o.c_int64;
	case fjson_type_boolean:
		return jso->o.c_boolean;
	case fjson_type_string:
		errno = 0;
		cdouble = strtod(get_string_component(jso), &errPtr);

		/* if conversion stopped at the first character, return 0.0 */
		if (errPtr == get_string_component(jso))
			return 0.0;

		/*
		* Check that the conversion terminated on something sensible
		*
		* For example, { "pay" : 123AB } would parse as 123.
		*/
		if (*errPtr != '\0')
			return 0.0;

		/*
		* If strtod encounters a string which would exceed the
		* capacity of a double, it returns +/- HUGE_VAL and sets
		* errno to ERANGE. But +/- HUGE_VAL is also a valid result
		* from a conversion, so we need to check errno.
		*
		* Underflow also sets errno to ERANGE, but it returns 0 in
		* that case, which is what we will return anyway.
		*
		* See CERT guideline ERR30-C
		*/
		if ((HUGE_VAL == cdouble || -HUGE_VAL == cdouble) &&
			(ERANGE == errno))
			cdouble = 0.0;
		return cdouble;
	case fjson_type_null:
	case fjson_type_object:
	case fjson_type_array:
	default:
		return 0.0;
	}
}


/* fjson_object_string */

static int fjson_object_string_to_json_string(struct fjson_object* jso,
						 struct printbuf *pb,
						 int __attribute__((unused)) level,
						 int __attribute__((unused)) flags)
{
	printbuf_memappend_char(pb, '\"');
	fjson_escape_str(pb, get_string_component(jso));
	printbuf_memappend_char(pb, '\"');
	return 0; /* we need to keep compatible with the API */
}

static void fjson_object_string_delete(struct fjson_object* jso)
{
	if(jso->o.c_string.len >= LEN_DIRECT_STRING_DATA)
		free(jso->o.c_string.str.ptr);
	fjson_object_generic_delete(jso);
}

struct fjson_object* fjson_object_new_string(const char *s)
{
	struct fjson_object *jso = fjson_object_new(fjson_type_string);
	if (!jso)
		return NULL;
	jso->_delete = &fjson_object_string_delete;
	jso->_to_json_string = &fjson_object_string_to_json_string;
	jso->o.c_string.len = strlen(s);
	if(jso->o.c_string.len < LEN_DIRECT_STRING_DATA) {
		memcpy(jso->o.c_string.str.data, s, jso->o.c_string.len);
	} else {
		jso->o.c_string.str.ptr = strdup(s);
		if (!jso->o.c_string.str.ptr)
		{
			fjson_object_generic_delete(jso);
			errno = ENOMEM;
			return NULL;
		}
	}
	return jso;
}

struct fjson_object* fjson_object_new_string_len(const char *s, int len)
{
	char *dstbuf;
	struct fjson_object *jso = fjson_object_new(fjson_type_string);
	if (!jso)
		return NULL;
	jso->_delete = &fjson_object_string_delete;
	jso->_to_json_string = &fjson_object_string_to_json_string;
	if(len < LEN_DIRECT_STRING_DATA) {
		dstbuf = jso->o.c_string.str.data;
	} else {
		jso->o.c_string.str.ptr = (char*)malloc(len + 1);
		if (!jso->o.c_string.str.ptr)
		{
			fjson_object_generic_delete(jso);
			errno = ENOMEM;
			return NULL;
		}
		dstbuf = jso->o.c_string.str.ptr;
	}
	memcpy(dstbuf, (void *)s, len);
	dstbuf[len] = '\0';
	jso->o.c_string.len = len;
	return jso;
}

const char* fjson_object_get_string(struct fjson_object *jso)
{
	if (!jso)
		return NULL;
	if(jso->o_type == fjson_type_string)
		return get_string_component(jso);
	else
		return fjson_object_to_json_string(jso);
}

int fjson_object_get_string_len(struct fjson_object *jso)
{
	if (!jso)
		return 0;
	if(jso->o_type == fjson_type_string)
		return jso->o.c_string.len;
	else
		return 0;
}


/* fjson_object_array */

static int fjson_object_array_to_json_string(struct fjson_object* jso,
	struct printbuf *pb,
	int level,
	int flags)
{
	int had_children = 0;
	int ii;
	printbuf_memappend_char(pb, '[');
	if (flags & FJSON_TO_STRING_PRETTY)
		printbuf_memappend_char(pb, '\n');
	for(ii=0; ii < fjson_object_array_length(jso); ii++)
	{
		struct fjson_object *val;
		if (had_children)
		{
			printbuf_memappend_char(pb, ',');
			if (flags & FJSON_TO_STRING_PRETTY)
				printbuf_memappend_char(pb, '\n');
		}
		had_children = 1;
		if (flags & FJSON_TO_STRING_SPACED)
			printbuf_memappend_char(pb, ' ');
		indent(pb, level + 1, flags);
		val = fjson_object_array_get_idx(jso, ii);
		if(val == NULL)
			printbuf_memappend_no_nul(pb, "null", 4);
		else
			val->_to_json_string(val, pb, level+1, flags);
	}
	if (flags & FJSON_TO_STRING_PRETTY)
	{
		if (had_children)
			printbuf_memappend_char(pb, '\n');
		indent(pb,level,flags);
	}

	if (flags & FJSON_TO_STRING_SPACED)
		printbuf_memappend_no_nul(pb, " ]", 2);
	else
		printbuf_memappend_char(pb, ']');
	return 0; /* we need to keep compatible with the API */
}

static void fjson_object_array_entry_free(void *data)
{
	fjson_object_put((struct fjson_object*)data);
}

static void fjson_object_array_delete(struct fjson_object* jso)
{
	array_list_free(jso->o.c_array);
	fjson_object_generic_delete(jso);
}

struct fjson_object* fjson_object_new_array(void)
{
	struct fjson_object *jso = fjson_object_new(fjson_type_array);
	if (!jso)
		return NULL;
	jso->_delete = &fjson_object_array_delete;
	jso->_to_json_string = &fjson_object_array_to_json_string;
	jso->o.c_array = array_list_new(&fjson_object_array_entry_free);
	return jso;
}

struct array_list* fjson_object_get_array(struct fjson_object *jso)
{
	if (!jso)
		return NULL;
	if(jso->o_type == fjson_type_array)
		return jso->o.c_array;
	else
		return NULL;
}

void fjson_object_array_sort(struct fjson_object *jso, int(*sort_fn)(const void *, const void *))
{
	array_list_sort(jso->o.c_array, sort_fn);
}

struct fjson_object* fjson_object_array_bsearch(
		const struct fjson_object *key,
		const struct fjson_object *jso,
		int (*sort_fn)(const void *, const void *))
{
	struct fjson_object **result;

	result = (struct fjson_object **)array_list_bsearch(
			(const void **)&key, jso->o.c_array, sort_fn);

	if (!result)
		return NULL;
	return *result;
}

int fjson_object_array_length(struct fjson_object *jso)
{
	return array_list_length(jso->o.c_array);
}

int fjson_object_array_add(struct fjson_object *jso,struct fjson_object *val)
{
	return array_list_add(jso->o.c_array, val);
}

int fjson_object_array_put_idx(struct fjson_object *jso, int idx,
				  struct fjson_object *val)
{
	return array_list_put_idx(jso->o.c_array, idx, val);
}

struct fjson_object* fjson_object_array_get_idx(struct fjson_object *jso,
						  int idx)
{
	return (struct fjson_object*)array_list_get_idx(jso->o.c_array, idx);
}

int fjson_object_get_member_count(struct fjson_object *jso)
{
	return jso->o.c_obj.nelem;
}
