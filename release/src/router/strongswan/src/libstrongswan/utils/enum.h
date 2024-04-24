/*
 * Copyright (C) 2009-2023 Tobias Brunner
 * Copyright (C) 2006-2008 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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
 * @defgroup enum enum
 * @{ @ingroup utils
 */

#ifndef ENUM_H_
#define ENUM_H_

#include <utils/printf_hook/printf_hook.h>

typedef struct enum_name_t enum_name_t;

/**
 * Magic enum_name_t pointer indicating this is an enum name for flags
 */
#define ENUM_FLAG_MAGIC ((enum_name_t*)~(uintptr_t)0)

/**
 * Struct to store names for enums.
 *
 * To print the string representation of enumeration values, the strings
 * are stored in these structures. Every enum_name contains a range
 * of strings, multiple ranges are linked together.
 * Use the convenience macros to define these linked ranges.
 *
 * For a single range, use:
 * @code
   ENUM(name, first, last, string1, string2, ...)
   @endcode
 * For multiple linked ranges, use:
 * @code
   ENUM_BEGIN(name, first, last, string1, string2, ...)
     ENUM_NEXT(name, first, last, last_from_previous, string3, ...)
     ENUM_NEXT(name, first, last, last_from_previous, string4, ...)
   ENUM_END(name, last_from_previous)
   @endcode
 * The ENUM and the ENUM_END define a enum_name_t pointer with the name supplied
 * in "name".
 *
 * Resolving of enum names is done using a printf hook. A printf format
 * character %N is replaced by the enum string. Printf needs two arguments to
 * resolve a %N, the enum_name_t* (the defined name in ENUM_BEGIN) followed
 * by the numerical enum value.
 */
struct enum_name_t {
	/** value of the first enum string, values are expected to be (u_)int, using
	 * int64_t here instead, however, avoids warnings for large unsigned ints */
	int64_t first;
	/** value of the last enum string */
	int64_t last;
	/** next enum_name_t in list, or ENUM_FLAG_MAGIC */
	enum_name_t *next;
	/** array of strings containing names from first to last */
	char *names[];
};

/**
 * Begin a new enum_name list.
 *
 * @param name	name of the enum_name list
 * @param first	enum value of the first enum string
 * @param last	enum value of the last enum string
 * @param ...	a list of strings
 */
#define ENUM_BEGIN(name, first, last, ...) \
	static enum_name_t name##last = {first, last + \
		BUILD_ASSERT(((last)-(first)+1) == countof(((char*[]){__VA_ARGS__}))), \
		NULL, { __VA_ARGS__ }}

/**
 * Continue a enum name list started with ENUM_BEGIN.
 *
 * @param name	name of the enum_name list
 * @param first	enum value of the first enum string
 * @param last	enum value of the last enum string
 * @param prev	enum value of the "last" defined in ENUM_BEGIN/previous ENUM_NEXT
 * @param ...	a list of strings
 */
#define ENUM_NEXT(name, first, last, prev, ...) \
	static enum_name_t name##last = {first, last + \
		BUILD_ASSERT(((last)-(first)+1) == countof(((char*[]){__VA_ARGS__}))), \
		&name##prev, { __VA_ARGS__ }}

/**
 * Complete enum name list started with ENUM_BEGIN.
 *
 * @param name	name of the enum_name list
 * @param prev	enum value of the "last" defined in ENUM_BEGIN/previous ENUM_NEXT
 */
#define ENUM_END(name, prev) enum_name_t *name = &name##prev;

/**
 * Define a enum name with only one range.
 *
 * This is a convenience macro to use when a enum_name list contains only
 * one range, and is equal as defining ENUM_BEGIN followed by ENUM_END.
 *
 * @param name	name of the enum_name list
 * @param first	enum value of the first enum string
 * @param last	enum value of the last enum string
 * @param ...	a list of strings
 */
#define ENUM(name, first, last, ...) \
	ENUM_BEGIN(name, first, last, __VA_ARGS__); ENUM_END(name, last)

/**
 * Define a enum name with only one range for flags.
 *
 * Using an enum list for flags would be overkill. Hence we use a single
 * range with all values in range. The next pointer is abused to mark
 * that the enum name is for flags only. Use NULL if a particular flag
 * is not meant to be printed.
 *
 * @param name	name of the enum_name list
 * @param first	enum value of the first enum string
 * @param last	enum value of the last enum string
 * @param unset	name used if no flags are set
 * @param ...	a list of strings
 */
#define ENUM_FLAGS(name, first, last, unset, ...) \
	static enum_name_t name##last = {first, last + \
		BUILD_ASSERT((__builtin_ffs(last)-__builtin_ffs(first)+1) == \
			countof(((char*[]){__VA_ARGS__}))), \
		ENUM_FLAG_MAGIC, { unset, __VA_ARGS__ }}; ENUM_END(name, last)

/**
 * Define a static enum name that can be added and removed to an existing list
 * via enum_add_enum_names() and enum_remove_enum_names(), respectively.
 *
 * @param name		name of the static enum_name element
 * @param first		enum value of the first enum string
 * @param last		enum value of the last enum string
 * @param ...		a list of strings
 */
#define ENUM_EXT(name, first, last, ...) \
	ENUM_BEGIN(name, first, last, __VA_ARGS__); static ENUM_END(name, last)

/**
 * Register enum names for additional enum values with an existing enum name.
 *
 * @note Must be called while running single-threaded, e.g. when plugins and
 * their features are loaded. Use enum_remove_enum_names() to remove the names
 * during deinitialization.
 *
 * @param e		enum names to add new names to
 * @param names	additional enum names
 */
void enum_add_enum_names(enum_name_t *e, enum_name_t *names);

/**
 * Remove previously registered enum names.
 *
 * @note Must be called while running single-threaded, e.g. when plugins and
 * their features are unloaded.
 *
 * @param e		enum names to remove previously added names from
 * @param names	additional enum names to remove
 */
void enum_remove_enum_names(enum_name_t *e, enum_name_t *names);

/**
 * Convert a enum value to its string representation.
 *
 * @param e		enum names for this enum value
 * @param val	enum value to get string for
 * @return		string for enum, NULL if not found
 */
char *enum_to_name(enum_name_t *e, int val);

/**
 * Convert a enum string back to its enum value.
 *
 * @param e		enum names for this enum value
 * @param name	name to get enum value for
 * @param valp	variable sized pointer receiving value
 * @return		TRUE if enum name found, FALSE otherwise
 */
#define enum_from_name(e, name, valp) ({ \
	int _val; \
	bool _found = enum_from_name_as_int(e, name, &_val); \
	if (_found) \
	{ \
		*(valp) = _val; \
	} \
	_found; })

/**
 * Convert a enum string back to its enum value, integer pointer variant.
 *
 * This variant takes an integer pointer, use enum_from_name() to pass
 * enum type pointers for the result.
 *
 * @param e		enum names for this enum value
 * @param name	name to get enum value for
 * @param val	integer pointer receiving value
 * @return		TRUE if all names found, FALSE otherwise
 */
bool enum_from_name_as_int(enum_name_t *e, const char *name, int *val);

/**
 * Convert a enum value containing flags to its string representation.
 *
 * @param e		enum names for this enum value suitable for flags
 * @param val	enum value to get string for
 * @param buf	buffer to write flag string to
 * @param len	buffer size
 * @return		buf, NULL if buffer too small
 */
char *enum_flags_to_string(enum_name_t *e, u_int val, char *buf, size_t len);

/**
 * Convert a string of flags separated by | to their combined value
 *
 * @param e		enum names for this enum value
 * @param str	string to get enum value for
 * @param valp	variable sized pointer receiving value
 * @return		TRUE if all names found, FALSE otherwise
 */
#define enum_flags_from_string(e, str, valp) ({ \
	u_int _val; \
	bool _found = enum_flags_from_string_as_int(e, str, &_val); \
	if (_found) \
	{ \
		*(valp) = _val; \
	} \
	_found; })

/**
 * Convert a string of flags separated by | to their combined value.
 *
 * This variant takes an unsigned integer pointer, use enum_flags_from_names()
 * to pass enum type pointers for the result.
 *
 * @param e		enum names for this enum value
 * @param str	string to get enum value for
 * @param val	integer pointer receiving value
 * @return		TRUE if enum name found, FALSE otherwise
 */
bool enum_flags_from_string_as_int(enum_name_t *e, const char *str, u_int *val);

/**
 * printf hook function for enum_names_t.
 *
 * Arguments are:
 *	enum_names_t *names, int value
 */
int enum_printf_hook(printf_hook_data_t *data, printf_hook_spec_t *spec,
					 const void *const *args);

#endif /** ENUM_H_ @}*/
