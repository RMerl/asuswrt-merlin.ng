/*
 * Copyright (C) 2009 Tobias Brunner
 * Copyright (C) 2006-2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
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
 * Resolving of enum names is done using a printf hook. A printf fromat
 * character %N is replaced by the enum string. Printf needs two arguments to
 * resolve a %N, the enum_name_t* (the defined name in ENUM_BEGIN) followed
 * by the numerical enum value.
 */
struct enum_name_t {
	/** value of the first enum string */
	int first;
	/** value of the last enum string */
	int last;
	/** next enum_name_t in list */
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
#define ENUM_BEGIN(name, first, last, ...) static enum_name_t name##last = {first, last, NULL, { __VA_ARGS__ }}

/**
 * Continue a enum name list startetd with ENUM_BEGIN.
 *
 * @param name	name of the enum_name list
 * @param first	enum value of the first enum string
 * @param last	enum value of the last enum string
 * @param prev	enum value of the "last" defined in ENUM_BEGIN/previous ENUM_NEXT
 * @param ...	a list of strings
 */
#define ENUM_NEXT(name, first, last, prev, ...) static enum_name_t name##last = {first, last, &name##prev, { __VA_ARGS__ }}

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
#define ENUM(name, first, last, ...) ENUM_BEGIN(name, first, last, __VA_ARGS__); ENUM_END(name, last)

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
	int _found = enum_from_name_as_int(e, name, &_val); \
	if (_found) \
	{ \
		*(valp) = _val; \
	} \
	_found; })

/**
 * Convert a enum string back to its enum value, integer pointer variant.
 *
 * This variant takes integer pointer only, use enum_from_name() to pass
 * enum type pointers for the result.
 *
 * @param e		enum names for this enum value
 * @param name	name to get enum value for
 * @param val	integer pointer receiving value
 * @return		TRUE if enum name found, FALSE otherwise
 */
bool enum_from_name_as_int(enum_name_t *e, const char *name, int *val);

/**
 * printf hook function for enum_names_t.
 *
 * Arguments are:
 *	enum_names_t *names, int value
 */
int enum_printf_hook(printf_hook_data_t *data, printf_hook_spec_t *spec,
					 const void *const *args);

#endif /** ENUM_H_ @}*/
