/*
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 * Copyright (c) 2009 Hewlett-Packard Development Company, L.P.
 * Copyright (c) 2015-2016 Rainer Gerhards
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#ifndef _fj_json_object_h_
#define _fj_json_object_h_

#ifdef __GNUC__
#define THIS_FUNCTION_IS_DEPRECATED(func) func __attribute__ ((deprecated))
#else
#define THIS_FUNCTION_IS_DEPRECATED(func) func
#endif

#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FJSON_OBJECT_DEF_HASH_ENTRIES 16
/* number of subjects within a children page. One page is allocated
 * with each json object, and extensions are alwas done in page
 * size increments. The size should be a compromise between not
 * wasting too much space but also not doing too frequent mallocs.
 * note: each page *entry* currently needs ~20 Bytes (x64). If this
 * is important, check the actual number (sizeof(struct _fjson_child)).
 */
#define FJSON_OBJECT_CHLD_PG_SIZE 8

/**
 * A flag for the fjson_object_to_json_string_ext() and
 * fjson_object_to_file_ext() functions which causes the output
 * to have no extra whitespace or formatting applied.
 */
#define FJSON_TO_STRING_PLAIN      0
/**
 * A flag for the fjson_object_to_json_string_ext() and
 * fjson_object_to_file_ext() functions which causes the output to have
 * minimal whitespace inserted to make things slightly more readable.
 */
#define FJSON_TO_STRING_SPACED     (1<<0)
/**
 * A flag for the fjson_object_to_json_string_ext() and
 * fjson_object_to_file_ext() functions which causes
 * the output to be formatted.
 *
 * See the "Two Space Tab" option at http://jsonformatter.curiousconcept.com/
 * for an example of the format.
 */
#define FJSON_TO_STRING_PRETTY     (1<<1)
/**
 * A flag for the fjson_object_to_json_string_ext() and
 * fjson_object_to_file_ext() functions which causes
 * the output to be formatted.
 *
 * Instead of a "Two Space Tab" this gives a single tab character.
 */
#define FJSON_TO_STRING_PRETTY_TAB (1<<3)
/**
 * A flag to drop trailing zero for float values
 */
#define FJSON_TO_STRING_NOZERO     (1<<2)

/**
 * A flag for the fjson_object_object_add_ex function which
 * causes the value to be added without a check if it already exists.
 * Note: it is the responsibilty of the caller to ensure that no
 * key is added multiple times. If this is done, results are
 * unpredictable. While this option is somewhat dangerous, it
 * permits potentially large performance savings in code that
 * knows for sure the key values are unique (e.g. because the
 * code adds a well-known set of constant key values).
 */
#define FJSON_OBJECT_ADD_KEY_IS_NEW (1<<1)
/**
 * A flag for the fjson_object_object_add_ex function which
 * flags the key as being constant memory. This means that
 * the key will NOT be copied via strdup(), resulting in a
 * potentially huge performance win (malloc, strdup and
 * free are usually performance hogs). It is acceptable to
 * use this flag for keys in non-constant memory blocks if
 * the caller ensure that the memory holding the key lives
 * longer than the corresponding json object. However, this
 * is somewhat dangerous and should only be done if really
 * justified.
 * The general use-case for this flag is cases where the
 * key is given as a real constant value in the function
 * call, e.g. as in
 *   fjson_object_object_add_ex(obj, "ip", json,
 *       FJSON_OBJECT_KEY_IS_CONSTANT);
 */
#define FJSON_OBJECT_KEY_IS_CONSTANT (1<<2)

#undef FALSE
#define FALSE ((fjson_bool)0)

#undef TRUE
#define TRUE ((fjson_bool)1)

extern const char *fjson_number_chars;
extern const char *fjson_hex_chars;

/* CAW: added for ANSI C iteration correctness */
struct fjson_object_iter
{
	char *key;
	struct fjson_object *val;
	struct lh_entry *entry;
};

/* forward structure definitions */

typedef int fjson_bool;
typedef struct printbuf printbuf;
typedef struct lh_table lh_table;
typedef struct array_list array_list;
typedef struct fjson_object fjson_object;
typedef struct fjson_object_iter fjson_object_iter;
typedef struct fjson_tokener fjson_tokener;

/**
 * Type for a user-supplied write function
 */
typedef size_t (fjson_write_fn)(void *ptr, const char *buffer, size_t size);

/* supported object types */

typedef enum fjson_type {
	/* If you change this, be sure to update fjson_type_to_name() too */
	fjson_type_null,
	fjson_type_boolean,
	fjson_type_double,
	fjson_type_int,
	fjson_type_object,
	fjson_type_array,
	fjson_type_string
} fjson_type;

/* reference counting functions */

/**
 * Increment the reference count of fjson_object, thereby grabbing shared
 * ownership of obj.
 *
 * @param obj the fjson_object instance
 */
extern struct fjson_object* fjson_object_get(struct fjson_object *obj);

/**
 * Decrement the reference count of fjson_object and free if it reaches zero.
 * You must have ownership of obj prior to doing this or you will cause an
 * imbalance in the reference count.
 *
 * @param obj the fjson_object instance
 * @returns 1 if the object was freed.
 */
int fjson_object_put(struct fjson_object *obj);

/**
 * Check if the fjson_object is of a given type
 * @param obj the fjson_object instance
 * @param type one of:
	fjson_type_null (i.e. obj == NULL),
	fjson_type_boolean,
	fjson_type_double,
	fjson_type_int,
	fjson_type_object,
	fjson_type_array,
	fjson_type_string
 */
extern int fjson_object_is_type(struct fjson_object *obj, enum fjson_type type);

/**
 * Get the type of the fjson_object.  See also fjson_type_to_name() to turn this
 * into a string suitable, for instance, for logging.
 *
 * @param obj the fjson_object instance
 * @returns type being one of:
	fjson_type_null (i.e. obj == NULL),
	fjson_type_boolean,
	fjson_type_double,
	fjson_type_int,
	fjson_type_object,
	fjson_type_array,
	fjson_type_string
 */
extern enum fjson_type fjson_object_get_type(struct fjson_object *obj);

/**
 * Get the size of the json string if it was dumped
 * @param obj object to calculate the size of
 * @returns the size of the json string
 */
extern size_t fjson_object_size(struct fjson_object *obj);

/**
 * Extended version of the above function that accept a flags parameter identical
 * to the fjson_object_dump_ext() function that you can use the specify how to
 * format the string for which the size is calculated
 * @param obj the object to calculate the size of
 * @param flags extra flags
 * @return size_t
 */
extern size_t fjson_object_size_ext(struct fjson_object *obj, int flags);

/**
 * Dump object to a user-supplied function.
 * Equivalent to fjson_object_write_ext(obj, FJSON_TO_STRING_SPACED, func, ptr)
 * @param obj object to be written
 * @param func your function that will be called to write the data
 * @param ptr pointer that will be passed as first argument to your function
 * @returns number of bytes written (the sum of all return values of calls to func)
 */
extern size_t fjson_object_dump(struct fjson_object *obj, fjson_write_fn *func, void *ptr);

/**
 * Extended dump function that allows passing extra option. You can use all
 * FJSON_TO_STRING_* constants for the flags
 * @param obj object to be written
 * @param flags extra flags
 * @param func your function that will be called to write the data
 * @param ptr pointer that will be passed as first argument to your function
 * @returns number of bytes written (the sum of all return values of calls to func)
 */
extern size_t fjson_object_dump_ext(struct fjson_object *obj, int flags, fjson_write_fn *func, void *ptr);

/**
 * Dump function that uses a user-supplied temporary buffer for dumping the
 * json. Both the above declared fjson_object_dump() and fjson_object_dump_ext()
 * functions uses an internal buffer of 128 bytes that is first filled before
 * the user-supplied function is called. This buffer prevents that many calls
 * to the callback function are done for single quotes, comma's and curly
 * braces. All these calls are first buffered and grouped into a single call
 * to the user space function. However, since the buffer limit is somewhat
 * arbitrary, you can also use this fjson_object_dump_buffered() function to
 * use your own temporary buffer. Note that the buffer might be completely
 * overwritten during the call to this function, and that the contents of the
 * buffer are undefined after the call.
 * @param obj object to be written
 * @param flags extra flags
 * @param temp your temporary buffer that is used to group calls
 * @param size size of your temporary buffer
 * @param func your function that will be called to write the data
 * @param ptr pointer that will be passed as first argument to your function
 */
extern size_t fjson_object_dump_buffered(struct fjson_object *obj, int flags, char *temp,
size_t size, fjson_write_fn *func, void *ptr);

/**
 * Write the json tree to a file
 * Equivalent to fjson_object_write_ext(obj, FJSON_TO_STRING_SPACED, fp)
 * @param obj object to be written
 * @param fp file-pointer to which output will be written
 * @returns number of bytes written
 */
extern size_t fjson_object_write(struct fjson_object *obj, FILE *fp);

/**
 * Extended write function that allows flags to be passed
 * @param obj object to be written
 * @param flags extra flags
 * @param fp file-pointer to which output will be written
 * @returns number of bytes written
 */
extern size_t fjson_object_write_ext(struct fjson_object *obj, int flags, FILE *fp);

/** Stringify object to json format.
 * Equivalent to fjson_object_to_json_string_ext(obj, FJSON_TO_STRING_SPACED)
 * The pointer you get is an internal of your json object. You don't
 * have to free it, later use of fjson_object_put() should be sufficient.
 * If you can not ensure there's no concurrent access to *obj use
 * strdup().
 * @param obj the fjson_object instance
 * @returns a string in JSON format
 */
extern const char* fjson_object_to_json_string(struct fjson_object *obj);

/** Stringify object to json format
 * @see fjson_object_to_json_string() for details on how to free string.
 * @param obj the fjson_object instance
 * @param flags formatting options, see FJSON_TO_STRING_PRETTY and other constants
 * @returns a string in JSON format
 */
extern const char* fjson_object_to_json_string_ext(struct fjson_object *obj, int
flags);


/* object type methods */

/** Create a new empty object with a reference count of 1.  The caller of
 * this object initially has sole ownership.  Remember, when using
 * fjson_object_object_add or fjson_object_array_put_idx, ownership will
 * transfer to the object/array.  Call fjson_object_get if you want to maintain
 * shared ownership or also add this object as a child of multiple objects or
 * arrays.  Any ownerships you acquired but did not transfer must be released
 * through fjson_object_put.
 *
 * @returns a fjson_object of type fjson_type_object
 */
extern struct fjson_object* fjson_object_new_object(void);

/** Get the size of an object in terms of the number of fields it has.
 * @param obj the fjson_object whose length to return
 */
extern int fjson_object_object_length(struct fjson_object* obj);

/** Add an object field to a fjson_object of type fjson_type_object
 *
 * The reference count will *not* be incremented. This is to make adding
 * fields to objects in code more compact. If you want to retain a reference
 * to an added object, independent of the lifetime of obj, you must wrap the
 * passed object with fjson_object_get.
 *
 * Upon calling this, the ownership of val transfers to obj.  Thus you must
 * make sure that you do in fact have ownership over this object.  For instance,
 * fjson_object_new_object will give you ownership until you transfer it,
 * whereas fjson_object_object_get does not.
 *
 * @param obj the fjson_object instance
 * @param key the object field name (a private copy will be duplicated)
 * @param val a fjson_object or NULL member to associate with the given field
 */
extern void fjson_object_object_add(struct fjson_object* obj, const char *key,
				   struct fjson_object *val);

/** Add an object field to a fjson_object of type fjson_type_object
 *
 * The semantics are identical to fjson_object_object_add, except that an
 * additional flag fields gives you more control over some detail aspects
 * of processing. See the description of FJSON_OBJECT_ADD_* flags for more
 * details.
 *
 * @param obj the fjson_object instance
 * @param key the object field name (a private copy will be duplicated)
 * @param val a fjson_object or NULL member to associate with the given field
 * @param opts process-modifying options. To specify multiple options, use
 *             arithmetic or (OPT1|OPT2)
 */
extern void fjson_object_object_add_ex(struct fjson_object* obj, const char *key,
				   struct fjson_object *val, const unsigned opts);

/** Get the fjson_object associate with a given object field
 *
 * *No* reference counts will be changed.  There is no need to manually adjust
 * reference counts through the fjson_object_put/fjson_object_get methods unless
 * you need to have the child (value) reference maintain a different lifetime
 * than the owning parent (obj). Ownership of the returned value is retained
 * by obj (do not do fjson_object_put unless you have done a fjson_object_get).
 * If you delete the value from obj (fjson_object_object_del) and wish to access
 * the returned reference afterwards, make sure you have first gotten shared
 * ownership through fjson_object_get (& don't forget to do a fjson_object_put
 * or transfer ownership to prevent a memory leak).
 *
 * @param obj the fjson_object instance
 * @param key the object field name
 * @returns the fjson_object associated with the given field name
 * @deprecated Please use fjson_object_object_get_ex
 */
THIS_FUNCTION_IS_DEPRECATED(extern struct fjson_object* fjson_object_object_get(struct fjson_object* obj,
						  const char *key));

/** Get the fjson_object associated with a given object field.
 *
 * This returns true if the key is found, false in all other cases (including
 * if obj isn't a fjson_type_object).
 *
 * *No* reference counts will be changed.  There is no need to manually adjust
 * reference counts through the fjson_object_put/fjson_object_get methods unless
 * you need to have the child (value) reference maintain a different lifetime
 * than the owning parent (obj).  Ownership of value is retained by obj.
 *
 * @param obj the fjson_object instance
 * @param key the object field name
 * @param value a pointer where to store a reference to the fjson_object
 *              associated with the given field name.
 *
 *              It is safe to pass a NULL value.
 * @returns whether or not the key exists
 */
extern fjson_bool fjson_object_object_get_ex(struct fjson_object* obj,
	const char *key,
	struct fjson_object **value);

/** Delete the given fjson_object field
 *
 * The reference count will be decremented for the deleted object.  If there
 * are no more owners of the value represented by this key, then the value is
 * freed.  Otherwise, the reference to the value will remain in memory.
 *
 * @param obj the fjson_object instance
 * @param key the object field name
 */
extern void fjson_object_object_del(struct fjson_object* obj, const char *key);


/* Array type methods */

/** Create a new empty fjson_object of type fjson_type_array
 * @returns a fjson_object of type fjson_type_array
 */
extern struct fjson_object* fjson_object_new_array(void);

/** Get the arraylist of a fjson_object of type fjson_type_array
 * @param obj the fjson_object instance
 * @returns an arraylist
 */
extern struct array_list* fjson_object_get_array(struct fjson_object *obj);

/** Get the length of a fjson_object of type fjson_type_array
 * @param obj the fjson_object instance
 * @returns an int
 */
extern int fjson_object_array_length(struct fjson_object *obj);

/** Sorts the elements of jso of type fjson_type_array
*
* Pointers to the fjson_object pointers will be passed as the two arguments
* to @sort_fn
*
* @param obj the fjson_object instance
* @param sort_fn a sorting function
*/
extern void fjson_object_array_sort(struct fjson_object *jso, int(*sort_fn)(const void *, const void *));

/** Binary search a sorted array for a specified key object.
 *
 * It depends on your compare function what's sufficient as a key.
 * Usually you create some dummy object with the parameter compared in
 * it, to identify the right item you're actually looking for.
 *
 * @see fjson_object_array_sort() for hints on the compare function.
 *
 * @param key a dummy fjson_object with the right key
 * @param jso the array object we're searching
 * @param sort_fn the sort/compare function
 *
 * @return the wanted fjson_object instance
 */
extern struct fjson_object* fjson_object_array_bsearch(
		const struct fjson_object *key,
		const struct fjson_object *jso,
		int (*sort_fn)(const void *, const void *));

/** Add an element to the end of a fjson_object of type fjson_type_array
 *
 * The reference count will *not* be incremented. This is to make adding
 * fields to objects in code more compact. If you want to retain a reference
 * to an added object you must wrap the passed object with fjson_object_get
 *
 * @param obj the fjson_object instance
 * @param val the fjson_object to be added
 */
extern int fjson_object_array_add(struct fjson_object *obj,
				 struct fjson_object *val);

/** Insert or replace an element at a specified index in an array (a fjson_object of type fjson_type_array)
 *
 * The reference count will *not* be incremented. This is to make adding
 * fields to objects in code more compact. If you want to retain a reference
 * to an added object you must wrap the passed object with fjson_object_get
 *
 * The reference count of a replaced object will be decremented.
 *
 * The array size will be automatically be expanded to the size of the
 * index if the index is larger than the current size.
 *
 * @param obj the fjson_object instance
 * @param idx the index to insert the element at
 * @param val the fjson_object to be added
 */
extern int fjson_object_array_put_idx(struct fjson_object *obj, int idx,
					 struct fjson_object *val);

/** Get the element at specificed index of the array (a fjson_object of type fjson_type_array)
 * @param obj the fjson_object instance
 * @param idx the index to get the element at
 * @returns the fjson_object at the specified index (or NULL)
 */
extern struct fjson_object* fjson_object_array_get_idx(struct fjson_object *obj,
							 int idx);

/* fjson_bool type methods */

/** Create a new empty fjson_object of type fjson_type_boolean
 * @param b a fjson_bool TRUE or FALSE (1 or 0)
 * @returns a fjson_object of type fjson_type_boolean
 */
extern struct fjson_object* fjson_object_new_boolean(fjson_bool b);

/** Get the fjson_bool value of a fjson_object
 *
 * The type is coerced to a fjson_bool if the passed object is not a fjson_bool.
 * integer and double objects will return FALSE if there value is zero
 * or TRUE otherwise. If the passed object is a string it will return
 * TRUE if it has a non zero length. If any other object type is passed
 * TRUE will be returned if the object is not NULL.
 *
 * @param obj the fjson_object instance
 * @returns a fjson_bool
 */
extern fjson_bool fjson_object_get_boolean(struct fjson_object *obj);


/* int type methods */

/** Create a new empty fjson_object of type fjson_type_int
 * Note that values are stored as 64-bit values internally.
 * To ensure the full range is maintained, use fjson_object_new_int64 instead.
 * @param i the integer
 * @returns a fjson_object of type fjson_type_int
 */
extern struct fjson_object* fjson_object_new_int(int32_t i);


/** Create a new empty fjson_object of type fjson_type_int
 * @param i the integer
 * @returns a fjson_object of type fjson_type_int
 */
extern struct fjson_object* fjson_object_new_int64(int64_t i);


/** Get the int value of a fjson_object
 *
 * The type is coerced to a int if the passed object is not a int.
 * double objects will return their integer conversion. Strings will be
 * parsed as an integer. If no conversion exists then 0 is returned
 * and errno is set to EINVAL. null is equivalent to 0 (no error values set)
 *
 * Note that integers are stored internally as 64-bit values.
 * If the value of too big or too small to fit into 32-bit, INT32_MAX or
 * INT32_MIN are returned, respectively.
 *
 * @param obj the fjson_object instance
 * @returns an int
 */
extern int32_t fjson_object_get_int(struct fjson_object *obj);

/** Get the int value of a fjson_object
 *
 * The type is coerced to a int64 if the passed object is not a int64.
 * double objects will return their int64 conversion. Strings will be
 * parsed as an int64. If no conversion exists then 0 is returned.
 *
 * NOTE: Set errno to 0 directly before a call to this function to determine
 * whether or not conversion was successful (it does not clear the value for
 * you).
 *
 * @param obj the fjson_object instance
 * @returns an int64
 */
extern int64_t fjson_object_get_int64(struct fjson_object *obj);


/* double type methods */

/** Create a new empty fjson_object of type fjson_type_double
 * @param d the double
 * @returns a fjson_object of type fjson_type_double
 */
extern struct fjson_object* fjson_object_new_double(double d);

/**
 * Create a new fjson_object of type fjson_type_double, using
 * the exact representation of the value.
 *
 * This allows for numbers that would otherwise get displayed
 * inefficiently (e.g. 12.3 => "12.300000000000001") to be
 * serialized with the more convenient form.
 *
 * Note: this is used by fjson_tokener_parse_ex() to allow for
 *   an exact re-serialization of a parsed object.
 *
 * @param d the numeric value of the double.
 * @param ds the string representation of the double.  This will be copied.
 */
extern struct fjson_object* fjson_object_new_double_s(double d, const char *ds);

/** Get the double floating point value of a fjson_object
 *
 * The type is coerced to a double if the passed object is not a double.
 * integer objects will return their double conversion. Strings will be
 * parsed as a double. If no conversion exists then 0.0 is returned and
 * errno is set to EINVAL. null is equivalent to 0 (no error values set)
 *
 * If the value is too big to fit in a double, then the value is set to
 * the closest infinity with errno set to ERANGE. If strings cannot be
 * converted to their double value, then EINVAL is set & NaN is returned.
 *
 * Arrays of length 0 are interpreted as 0 (with no error flags set).
 * Arrays of length 1 are effectively cast to the equivalent object and
 * converted using the above rules.  All other arrays set the error to
 * EINVAL & return NaN.
 *
 * NOTE: Set errno to 0 directly before a call to this function to
 * determine whether or not conversion was successful (it does not clear
 * the value for you).
 *
 * @param obj the fjson_object instance
 * @returns a double floating point number
 */
extern double fjson_object_get_double(struct fjson_object *obj);


/* string type methods */

/** Create a new empty fjson_object of type fjson_type_string
 *
 * A copy of the string is made and the memory is managed by the fjson_object
 *
 * @param s the string
 * @returns a fjson_object of type fjson_type_string
 */
extern struct fjson_object* fjson_object_new_string(const char *s);

extern struct fjson_object* fjson_object_new_string_len(const char *s, int len);

/** Get the string value of a fjson_object
 *
 * If the passed object is not of type fjson_type_string then the JSON
 * representation of the object is returned.
 *
 * The returned string memory is managed by the fjson_object and will
 * be freed when the reference count of the fjson_object drops to zero.
 *
 * @param obj the fjson_object instance
 * @returns a string
 */
extern const char* fjson_object_get_string(struct fjson_object *obj);

/** Get the string length of a fjson_object
 *
 * If the passed object is not of type fjson_type_string then zero
 * will be returned.
 *
 * @param obj the fjson_object instance
 * @returns int
 */
extern int fjson_object_get_string_len(struct fjson_object *obj);


/** Get the number of direct members inside a json object.
 *
 * @param obj the fjson_object instance
 * @returns int
 */
int fjson_object_get_member_count(struct fjson_object *jso);


/* The following is a source code compatibility layer
 * in regard to json-c.
 * It currently is aimed at the rsyslog family of projects,
 * we may extend or drop it later.
 */
#ifndef FJSON_NATIVE_API_ONLY

#define JSON_C_TO_STRING_PLAIN      FJSON_TO_STRING_PLAIN
#define JSON_C_TO_STRING_SPACED FJSON_TO_STRING_SPACED
#define JSON_C_TO_STRING_PRETTY FJSON_TO_STRING_PRETTY
#define JSON_C_TO_STRING_PRETTY_TAB FJSON_TO_STRING_PRETTY_TAB
#define JSON_C_TO_STRING_NOZERO FJSON_TO_STRING_NOZERO
#define JSON_C_OBJECT_ADD_KEY_IS_NEW FJSON_OBJECT_ADD_KEY_IS_NEW
#define JSON_C_OBJECT_KEY_IS_CONSTANT FJSON_OBJECT_KEY_IS_CONSTANT


/* forward structure definitions */

#if 0
typedef int fjson_bool;
typedef struct printbuf printbuf;
typedef struct lh_table lh_table;
typedef struct array_list array_list;
typedef struct fjson_object fjson_object;
typedef struct fjson_tokener fjson_tokener;
#endif

#define json_bool fjson_bool
#define json_type fjson_type
#define json_type_null fjson_type_null
#define json_type_boolean fjson_type_boolean
#define json_type_double fjson_type_double
#define json_type_int fjson_type_int
#define json_type_object fjson_type_object
#define json_type_array fjson_type_array
#define json_type_string fjson_type_string
#define json_object_iter fjson_object_iter

#define json_object fjson_object

#define json_object_get fjson_object_get
#define json_object_put fjson_object_put
#define json_object_is_type fjson_object_is_type
#define json_object_get_type(x) fjson_object_get_type((x))
#define json_object_to_json_string(x) fjson_object_to_json_string((x))
#define json_object_to_json_string_ext(a, b) fjson_object_to_json_string_ext((a), (b))
#define json_object_new_object() fjson_object_new_object()
#define json_object_object_length(a) fjson_object_object_length((a))
#define json_object_object_add(a, b, c) fjson_object_object_add((a), (b), (c))
#define json_object_object_add_ex fjson_object_object_add_ex
#define json_object_object_get_ex fjson_object_object_get_ex
#define json_object_object_get fjson_object_object_get
#define json_object_object_del fjson_object_object_del
#define json_object_new_array fjson_object_new_array
#define json_object_get_array fjson_object_get_array
#define json_object_array_length fjson_object_array_length
#define json_object_array_sort fjson_object_array_sort
#define json_object_array_bsearch fjson_object_array_bsearch
#define json_object_array_add fjson_object_array_add
#define json_object_array_put_idx fjson_object_array_put_idx
#define json_object_array_get_idx fjson_object_array_get_idx
#define json_object_new_boolean fjson_object_new_boolean
#define json_object_get_boolean fjson_object_get_boolean
#define json_object_new_int fjson_object_new_int
#define json_object_new_int64 fjson_object_new_int64
#define json_object_new_double fjson_object_new_double
#define json_object_new_double_s fjson_object_new_double_s
#define json_object_get_double fjson_object_get_double
#define json_object_new_string fjson_object_new_string
#define json_object_new_string_len fjson_object_new_string_len
#define json_object_get_string fjson_object_get_string
#define json_object_get_int fjson_object_get_int
#define json_object_get_int64 fjson_object_get_int64
#define json_object_get_string_len fjson_object_get_string_len
#define json_object_get_member_count fjson_object_get_member_count


#endif
#ifdef __cplusplus
}
#endif

#endif
