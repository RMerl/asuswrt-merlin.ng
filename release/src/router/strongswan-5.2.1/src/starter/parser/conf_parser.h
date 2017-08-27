/*
 * Copyright (C) 2013-2014 Tobias Brunner
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
 * @defgroup starter starter
 *
 * @defgroup conf_parser conf_parser
 * @{ @ingroup starter
 */

#ifndef CONF_PARSER_H_
#define CONF_PARSER_H_

#include <library.h>
#include <collections/dictionary.h>

typedef enum conf_parser_section_t conf_parser_section_t;
typedef struct conf_parser_t conf_parser_t;

/**
 * Type of section
 */
enum conf_parser_section_t {
	/**
	 * config setup
	 */
	CONF_PARSER_CONFIG_SETUP,

	/**
	 * conn _name_
	 */
	CONF_PARSER_CONN,

	/**
	 * ca _name_
	 */
	CONF_PARSER_CA,
};

/**
 * Parser for ipsec.conf
 */
struct conf_parser_t {

	/**
	 * Parse the config file.
	 *
	 * @return		TRUE if config file was parsed successfully
	 */
	bool (*parse)(conf_parser_t *this);

	/**
	 * Get the names of all sections of the given type.
	 *
	 * @note Returns an empty enumerator for the config setup section.
	 *
	 * @return		enumerator over char*
	 */
	enumerator_t *(*get_sections)(conf_parser_t *this,
								  conf_parser_section_t type);

	/**
	 * Get the section with the given type and name.
	 *
	 * @note The name is ignored for the config setup section.
	 *
	 * @return		dictionary with settings
	 */
	dictionary_t *(*get_section)(conf_parser_t *this,
								 conf_parser_section_t type, char *name);

	/**
	 * Add a section while parsing.
	 *
	 * @note This method can only be called while parsing the config file.
	 *
	 * @param type	type of section to add
	 * @param name	name of the section, if applicable (gets adopted)
	 * @return		TRUE if the section already existed (settings get added)
	 */
	bool (*add_section)(conf_parser_t *this, conf_parser_section_t type,
						char *name);

	/**
	 * Add a key/value pair to the latest section.
	 *
	 * @note This method can only be called while parsing the config file.
	 *
	 * @param name	key string (gets adopted)
	 * @param value	optional value string (gets adopted), if no value is
	 * 				specified the key is set empty
	 */
	void (*add_setting)(conf_parser_t *this, char *key, char *value);


	/**
	 * Destroy a conf_parser_t instance.
	 */
	void (*destroy)(conf_parser_t *this);
};

/**
 * Create a conf_parser_t instance.
 *
 * @param file		ipsec.conf file to parse (gets copied)
 * @return			conf_parser_t instance
 */
conf_parser_t *conf_parser_create(const char *file);

#endif /** CONF_PARSER_H_ @}*/