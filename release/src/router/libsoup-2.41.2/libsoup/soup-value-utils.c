/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-type-utils.c: GValue and GType-related utilities
 *
 * Copyright (C) 2007 Red Hat, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "soup-value-utils.h"

/**
 * SECTION:soup-value-utils
 * @short_description: #GValue utilities
 *
 * These methods are useful for manipulating #GValue<!-- -->s, and in
 * particular, arrays and hash tables of #GValue<!-- -->s, in a
 * slightly nicer way than the standard #GValue API.
 *
 * They are written for use with soup-xmlrpc, but they also work with
 * types not used by XML-RPC.
 **/

/**
 * SOUP_VALUE_SETV:
 * @val: a #GValue
 * @type: a #GType
 * @args: #va_list pointing to a value of type @type
 *
 * Copies an argument of type @type from @args into @val. @val will
 * point directly to the value in @args rather than copying it, so you
 * must g_value_copy() it if you want it to remain valid.
 **/

/**
 * SOUP_VALUE_GETV:
 * @val: a #GValue
 * @type: a #GType
 * @args: #va_list pointing to a value of type pointer-to-@type
 *
 * Extracts a value of type @type from @val into @args. The return
 * value will point to the same data as @val rather than being a copy
 * of it.
 **/

static void
soup_value_hash_value_free (gpointer val)
{
	g_value_unset (val);
	g_free (val);
}

/**
 * soup_value_hash_new:
 *
 * Creates a #GHashTable whose keys are strings and whose values
 * are #GValue.
 *
 * Return value: (element-type utf8 GValue) (transfer full): a new
 * empty #GHashTable
 **/
GHashTable *
soup_value_hash_new (void)
{
	return g_hash_table_new_full (g_str_hash, g_str_equal,
				      g_free, soup_value_hash_value_free);
}

static void
soup_value_hash_insert_valist (GHashTable *hash, const char *first_key,
			       va_list args)
{
	const char *key;
	GType type;
	GValue value;

	key = first_key;
	while (key) {
		type = va_arg (args, GType);
		SOUP_VALUE_SETV (&value, type, args);

		soup_value_hash_insert_value (hash, key, &value);
		key = va_arg (args, const char *);
	}
}

/**
 * soup_value_hash_new_with_vals:
 * @first_key: the key for the first value
 * @...: the type of @first_key, followed by the value, followed
 * by additional key/type/value triplets, terminated by %NULL
 *
 * Creates a #GHashTable whose keys are strings and whose values
 * are #GValue, and initializes it with the provided data. As
 * with soup_value_hash_insert(), the keys and values are copied
 * rather than being inserted directly.
 *
 * Return value: (element-type utf8 GValue) (transfer full): a new
 * #GHashTable, initialized with the given values
 **/
GHashTable *
soup_value_hash_new_with_vals (const char *first_key, ...)
{
	GHashTable *hash = soup_value_hash_new ();
	va_list args;

	va_start (args, first_key);
	soup_value_hash_insert_valist (hash, first_key, args);
	va_end (args);

	return hash;
}

/**
 * soup_value_hash_insert_value:
 * @hash: (element-type utf8 GValue): a value hash
 * @key: the key
 * @value: a value
 *
 * Inserts @value into @hash. (Unlike with g_hash_table_insert(), both
 * the key and the value are copied).
 **/
void
soup_value_hash_insert_value (GHashTable *hash, const char *key, GValue *value)
{
	GValue *copy = g_new0 (GValue, 1);

	g_value_init (copy, G_VALUE_TYPE (value));
	g_value_copy (value, copy);
	g_hash_table_insert (hash, g_strdup (key), copy);
}

/**
 * soup_value_hash_insert:
 * @hash: (element-type utf8 GValue): a value hash
 * @key: the key
 * @type: a #GType
 * @...: a value of type @type
 *
 * Inserts the provided value of type @type into @hash. (Unlike with
 * g_hash_table_insert(), both the key and the value are copied).
 **/
void
soup_value_hash_insert (GHashTable *hash, const char *key, GType type, ...)
{
	va_list args;
	GValue val;

	va_start (args, type);
	SOUP_VALUE_SETV (&val, type, args);
	va_end (args);
	soup_value_hash_insert_value (hash, key, &val);
}

/**
 * soup_value_hash_insert_vals:
 * @hash: (element-type utf8 GValue): a value hash
 * @first_key: the key for the first value
 * @...: the type of @first_key, followed by the value, followed
 * by additional key/type/value triplets, terminated by %NULL
 *
 * Inserts the given data into @hash. As with
 * soup_value_hash_insert(), the keys and values are copied rather
 * than being inserted directly.
 **/
void
soup_value_hash_insert_vals (GHashTable *hash, const char *first_key, ...)
{
	va_list args;

	va_start (args, first_key);
	soup_value_hash_insert_valist (hash, first_key, args);
	va_end (args);
}

/**
 * soup_value_hash_lookup:
 * @hash: (element-type utf8 GValue): a value hash
 * @key: the key to look up
 * @type: a #GType
 * @...: a value of type pointer-to-@type
 *
 * Looks up @key in @hash and stores its value into the provided
 * location.
 *
 * Return value: %TRUE if @hash contained a value with key @key and
 * type @type, %FALSE if not.
 **/
gboolean
soup_value_hash_lookup (GHashTable *hash, const char *key, GType type, ...)
{
	va_list args;
	GValue *value;

	value = g_hash_table_lookup (hash, key);
	if (!value || !G_VALUE_HOLDS (value, type))
		return FALSE;

	va_start (args, type);
	SOUP_VALUE_GETV (value, type, args);
	va_end (args);

	return TRUE;
}

/**
 * soup_value_hash_lookup_vals:
 * @hash: (element-type utf8 GValue): a value hash
 * @first_key: the first key to look up
 * @...: the type of @first_key, a pointer to that type, and
 * then additional key/type/pointer triplets, terminated
 * by %NULL.
 *
 * Looks up a number of keys in @hash and returns their values.
 *
 * Return value: %TRUE if all of the keys were found, %FALSE
 * if any were missing; note that you will generally need to
 * initialize each destination variable to a reasonable default
 * value, since there is no way to tell which keys were found
 * and which were not.
 **/
gboolean
soup_value_hash_lookup_vals (GHashTable *hash, const char *first_key, ...)
{
	va_list args;
	GValue *value;
	const char *key;
	GType type;
	gboolean found_all = TRUE;

	va_start (args, first_key);
	key = first_key;
	while (key) {
		type = va_arg (args, GType);

		value = g_hash_table_lookup (hash, key);
		if (!value || !G_VALUE_HOLDS (value, type)) {
			found_all = FALSE;
			/* skip a pointer */
			va_arg (args, gpointer);
		} else
			SOUP_VALUE_GETV (value, type, args);

		key = va_arg (args, const char *);
	}
	va_end (args);

	return found_all;
}


#ifdef G_GNUC_BEGIN_IGNORE_DEPRECATIONS
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
#endif

/**
 * soup_value_array_from_args:
 * @args: arguments to create a #GValueArray from
 *
 * Creates a #GValueArray from the provided arguments, which must
 * consist of pairs of a #GType and a value of that type, terminated
 * by %G_TYPE_INVALID. (The array will contain copies of the provided
 * data rather than pointing to the passed-in data directly.)
 *
 * Return value: a new #GValueArray, or %NULL if an error occurred.
 **/
GValueArray *
soup_value_array_from_args (va_list args)
{
	GValueArray *array;
	GType type;
	GValue val;

	array = g_value_array_new (1);
	while ((type = va_arg (args, GType)) != G_TYPE_INVALID) {
		SOUP_VALUE_SETV (&val, type, args);
		g_value_array_append (array, &val);
	}
	return array;
}

/**
 * soup_value_array_to_args:
 * @array: a #GValueArray
 * @args: arguments to extract @array into
 *
 * Extracts a #GValueArray into the provided arguments, which must
 * consist of pairs of a #GType and a value of pointer-to-that-type,
 * terminated by %G_TYPE_INVALID. The returned values will point to the
 * same memory as the values in the array.
 *
 * Return value: success or failure
 **/
gboolean
soup_value_array_to_args (GValueArray *array, va_list args)
{
	GType type;
	GValue *value;
	int i;

	for (i = 0; i < array->n_values; i++) {
		type = va_arg (args, GType);
		if (type == G_TYPE_INVALID)
			return FALSE;
		value = g_value_array_get_nth (array, i);
		if (!G_VALUE_HOLDS (value, type))
			return FALSE;
		SOUP_VALUE_GETV (value, type, args);
	}
	return TRUE;
}

/**
 * soup_value_array_new:
 *
 * Creates a new %GValueArray. (This is just a wrapper around
 * g_value_array_new(), for naming consistency purposes.)
 *
 * Return value: a new %GValueArray
 **/
GValueArray *
soup_value_array_new (void)
{
	return g_value_array_new (1);
}

static void
soup_value_array_append_valist (GValueArray *array,
				GType first_type, va_list args)
{
	GType type;
	GValue value;

	type = first_type;
	while (type != G_TYPE_INVALID) {
		SOUP_VALUE_SETV (&value, type, args);

		g_value_array_append (array, &value);
		type = va_arg (args, GType);
	}
}

/**
 * soup_value_array_new_with_vals:
 * @first_type: the type of the first value to add
 * @...: the first value to add, followed by other type/value
 * pairs, terminated by %G_TYPE_INVALID
 *
 * Creates a new %GValueArray and copies the provided values
 * into it.
 *
 * Return value: a new %GValueArray
 **/
GValueArray *
soup_value_array_new_with_vals (GType first_type, ...)
{
	GValueArray *array = soup_value_array_new ();
	va_list args;

	va_start (args, first_type);
	soup_value_array_append_valist (array, first_type, args);
	va_end (args);

	return array;
}

/**
 * soup_value_array_insert:
 * @array: a #GValueArray
 * @index_: the index to insert at
 * @type: a #GType
 * @...: a value of type @type
 *
 * Inserts the provided value of type @type into @array as with
 * g_value_array_insert(). (The provided data is copied rather than
 * being inserted directly.)
 **/
void
soup_value_array_insert (GValueArray *array, guint index_, GType type, ...)
{
	va_list args;
	GValue val;

	va_start (args, type);
	SOUP_VALUE_SETV (&val, type, args);
	va_end (args);
	g_value_array_insert (array, index_, &val);
}

/**
 * soup_value_array_append:
 * @array: a #GValueArray
 * @type: a #GType
 * @...: a value of type @type
 *
 * Appends the provided value of type @type to @array as with
 * g_value_array_append(). (The provided data is copied rather than
 * being inserted directly.)
 **/
void
soup_value_array_append (GValueArray *array, GType type, ...)
{
	va_list args;
	GValue val;

	va_start (args, type);
	SOUP_VALUE_SETV (&val, type, args);
	va_end (args);
	g_value_array_append (array, &val);
}

/**
 * soup_value_array_append_vals:
 * @array: a #GValueArray
 * @first_type: the type of the first value to add
 * @...: the first value to add, followed by other type/value
 * pairs, terminated by %G_TYPE_INVALID
 *
 * Appends the provided values into @array as with
 * g_value_array_append(). (The provided data is copied rather than
 * being inserted directly.)
 **/
void
soup_value_array_append_vals (GValueArray *array, GType first_type, ...)
{
	va_list args;

	va_start (args, first_type);
	soup_value_array_append_valist (array, first_type, args);
	va_end (args);
}

/**
 * soup_value_array_get_nth:
 * @array: a #GValueArray
 * @index_: the index to look up
 * @type: a #GType
 * @...: a value of type pointer-to-@type
 *
 * Gets the @index_ element of @array and stores its value into the
 * provided location.
 *
 * Return value: %TRUE if @array contained a value with index @index_
 * and type @type, %FALSE if not.
 **/
gboolean
soup_value_array_get_nth (GValueArray *array, guint index_, GType type, ...)
{
	GValue *value;
	va_list args;

	value = g_value_array_get_nth (array, index_);
	if (!value || !G_VALUE_HOLDS (value, type))
		return FALSE;

	va_start (args, type);
	SOUP_VALUE_GETV (value, type, args);
	va_end (args);
	return TRUE;
}

#ifdef G_GNUC_END_IGNORE_DEPRECATIONS
G_GNUC_END_IGNORE_DEPRECATIONS
#endif

static GByteArray *
soup_byte_array_copy (GByteArray *ba)
{
	GByteArray *copy;

	copy = g_byte_array_sized_new (ba->len);
	g_byte_array_append (copy, ba->data, ba->len);
	return copy;
}

static void
soup_byte_array_free (GByteArray *ba)
{
	g_byte_array_free (ba, TRUE);
}

/**
 * SOUP_TYPE_BYTE_ARRAY:
 *
 * glib did not used to define a #GType for #GByteArray, so libsoup
 * defines this one itself.
 **/
typedef GByteArray SoupByteArray;
G_DEFINE_BOXED_TYPE (SoupByteArray, soup_byte_array, soup_byte_array_copy, soup_byte_array_free)
