/*
 * Copyright (C) 2010 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup settings settings
 * @ingroup libstrongswan
 *
 * @defgroup settings_t settings
 * @{ @ingroup settings
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

typedef struct settings_t settings_t;

#include "utils/utils.h"
#include "collections/enumerator.h"

/**
 * Convert a string value returned by a key/value enumerator to a boolean.
 *
 * @see settings_t.create_key_value_enumerator()
 * @see settings_t.get_bool()
 * @param value			the string value
 * @param def			the default value, if value is NULL or invalid
 */
bool settings_value_as_bool(char *value, bool def);

/**
 * Convert a string value returned by a key/value enumerator to an integer.
 *
 * @see settings_t.create_key_value_enumerator()
 * @see settings_t.get_int()
 * @param value			the string value
 * @param def			the default value, if value is NULL or invalid
 */
int settings_value_as_int(char *value, int def);

/**
 * Convert a string value returned by a key/value enumerator to a double.
 *
 * @see settings_t.create_key_value_enumerator()
 * @see settings_t.get_double()
 * @param value			the string value
 * @param def			the default value, if value is NULL or invalid
 */
double settings_value_as_double(char *value, double def);

/**
 * Convert a string value returned by a key/value enumerator to a time value.
 *
 * @see settings_t.create_key_value_enumerator()
 * @see settings_t.get_time()
 * @param value			the string value
 * @param def			the default value, if value is NULL or invalid
 */
u_int32_t settings_value_as_time(char *value, u_int32_t def);

/**
 * Generic configuration options read from a config file.
 *
 * The syntax is quite simple:
 * @code
 * settings := (section|keyvalue)*
 * section  := name { settings }
 * keyvalue := key = value\n
 * @endcode
 * E.g.:
 * @code
	a = b
	section-one {
		somevalue = asdf
		subsection {
			othervalue = xxx
		}
		yetanother = zz
	}
	section-two {
	}
	@endcode
 *
 * The values are accessed using the get() functions using dotted keys, e.g.
 *   section-one.subsection.othervalue
 *
 * Currently only a limited set of printf format specifiers are supported
 * (namely %s, %d and %N, see implementation for details).
 *
 * \section includes Including other files
 * Other files can be included, using the include statement e.g.
 * @code
 *   include /somepath/subconfig.conf
 * @endcode
 * Shell patterns like *.conf are possible.
 *
 * If the path is relative, the directory of the file containing the include
 * statement is used as base.
 *
 * Sections loaded from included files extend previously loaded sections,
 * already existing values are replaced.
 *
 * All settings included from files are added relative to the section the
 * include statement is in.
 *
 * The following files result in the same final config as above:
 *
 * @code
	a = b
	section-one {
		somevalue = before include
		include include.conf
	}
	include two.conf
	@endcode
 * include.conf
 * @code
	somevalue = asdf
	subsection {
		othervalue = yyy
	}
	yetanother = zz
	@endcode
 * two.conf
 * @code
	section-one {
		subsection {
			othervalue = xxx
		}
	}
	section-two {
	}
	@endcode
 */
struct settings_t {

	/**
	 * Get a settings value as a string.
	 *
	 * @param key		key including sections, printf style format
	 * @param def		value returned if key not found
	 * @param ...		argument list for key
	 * @return			value pointing to internal string
	 */
	char* (*get_str)(settings_t *this, char *key, char *def, ...);

	/**
	 * Get a boolean yes|no, true|false value.
	 *
	 * @param key		key including sections, printf style format
	 * @param def		value returned if key not found
	 * @param ...		argument list for key
	 * @return			value of the key
	 */
	bool (*get_bool)(settings_t *this, char *key, bool def, ...);

	/**
	 * Get an integer value.
	 *
	 * @param key		key including sections, printf style format
	 * @param def		value returned if key not found
	 * @param ...		argument list for key
	 * @return			value of the key
	 */
	int (*get_int)(settings_t *this, char *key, int def, ...);

	/**
	 * Get an double value.
	 *
	 * @param key		key including sections, printf style format
	 * @param def		value returned if key not found
	 * @param ...		argument list for key
	 * @return			value of the key
	 */
	double (*get_double)(settings_t *this, char *key, double def, ...);

	/**
	 * Get a time value.
	 *
	 * @param key		key including sections, printf style format
	 * @param def		value returned if key not found
	 * @param ...		argument list for key
	 * @return			value of the key (in seconds)
	 */
	u_int32_t (*get_time)(settings_t *this, char *key, u_int32_t def, ...);

	/**
	 * Set a string value.
	 *
	 * @param key		key including sections, printf style format
	 * @param value		value to set (gets cloned)
	 * @param ...		argument list for key
	 */
	void (*set_str)(settings_t *this, char *key, char *value, ...);

	/**
	 * Set a boolean value.
	 *
	 * @param key		key including sections, printf style format
	 * @param value		value to set
	 * @param ...		argument list for key
	 */
	void (*set_bool)(settings_t *this, char *key, bool value, ...);

	/**
	 * Set an integer value.
	 *
	 * @param key		key including sections, printf style format
	 * @param value		value to set
	 * @param ...		argument list for key
	 */
	void (*set_int)(settings_t *this, char *key, int value, ...);

	/**
	 * Set an double value.
	 *
	 * @param key		key including sections, printf style format
	 * @param value		value to set
	 * @param ...		argument list for key
	 */
	void (*set_double)(settings_t *this, char *key, double value, ...);

	/**
	 * Set a time value.
	 *
	 * @param key		key including sections, printf style format
	 * @param def		value to set
	 * @param ...		argument list for key
	 */
	void (*set_time)(settings_t *this, char *key, u_int32_t value, ...);

	/**
	 * Set a default for string value.
	 *
	 * @param key		key including sections, printf style format
	 * @param def		value to set if unconfigured
	 * @param ...		argument list for key
	 * @return			TRUE if a new default value for key has been set
	 */
	bool (*set_default_str)(settings_t *this, char *key, char *value, ...);

	/**
	 * Create an enumerator over subsection names of a section.
	 *
	 * @param section	section including parents, printf style format
	 * @param ...		argument list for key
	 * @return			enumerator over subsection names
	 */
	enumerator_t* (*create_section_enumerator)(settings_t *this,
											   char *section, ...);

	/**
	 * Create an enumerator over key/value pairs in a section.
	 *
	 * @param section	section name to list key/value pairs of, printf style
	 * @param ...		argument list for section
	 * @return			enumerator over (char *key, char *value)
	 */
	enumerator_t* (*create_key_value_enumerator)(settings_t *this,
												 char *section, ...);

	/**
	 * Add a fallback for the given section.
	 *
	 * Example: When the fallback 'section-two' is configured for
	 * 'section-one.two' any failed lookup for a section or key in
	 * 'section-one.two' will result in a lookup for the same section/key
	 * in 'section-two'.
	 *
	 * @note Lookups are depth-first and currently strictly top-down.
	 * For instance, if app.sec had lib1.sec as fallback and lib1 had lib2 as
	 * fallback the keys/sections in lib2.sec would not be considered.  But if
	 * app had lib3 as fallback the contents of lib3.sec would (as app is passed
	 * during the initial lookup).  In the last example the order during
	 * enumerations would be app.sec, lib1.sec, lib3.sec.
	 *
	 * @note Additional arguments will be applied to both section format
	 * strings so they must be compatible.
	 *
	 * @param section	section for which a fallback is configured, printf style
	 * @param fallback	fallback section, printf style
	 * @param ...		argument list for section and fallback
	 */
	void (*add_fallback)(settings_t *this, const char *section,
						 const char *fallback, ...);

	/**
	 * Load settings from the files matching the given pattern.
	 *
	 * If merge is TRUE, existing sections are extended, existing values
	 * replaced, by those found in the loaded files. If it is FALSE, existing
	 * sections are purged before reading the new config.
	 *
	 * @note If any of the files matching the pattern fails to load, no settings
	 * are added at all. So, it's all or nothing.
	 *
	 * @param pattern	file pattern
	 * @param merge		TRUE to merge config with existing values
	 * @return			TRUE, if settings were loaded successfully
	 */
	bool (*load_files)(settings_t *this, char *pattern, bool merge);

	/**
	 * Load settings from the files matching the given pattern.
	 *
	 * If merge is TRUE, existing sections are extended, existing values
	 * replaced, by those found in the loaded files. If it is FALSE, existing
	 * sections are purged before reading the new config.
	 *
	 * All settings are loaded relative to the given section. The section is
	 * created, if it does not yet exist.
	 *
	 * @note If any of the files matching the pattern fails to load, no settings
	 * are added at all. So, it's all or nothing.
	 *
	 * @param pattern	file pattern
	 * @param merge		TRUE to merge config with existing values
	 * @param section	section name of parent section, printf style
	 * @param ...		argument list for section
	 * @return			TRUE, if settings were loaded successfully
	 */
	bool (*load_files_section)(settings_t *this, char *pattern, bool merge,
							   char *section, ...);

	/**
	 * Destroy a settings instance.
	 */
	void (*destroy)(settings_t *this);
};

/**
 * Load settings from a file.
 *
 * @note If parsing the file fails the object is still created.
 *
 * @param file			optional file to read settings from
 * @return				settings object
 */
settings_t *settings_create(char *file);

#endif /** SETTINGS_H_ @}*/
