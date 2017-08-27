/*
 * Copyright (C) 2007 Martin Willi
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
 * @defgroup xml xml
 * @{ @ingroup manager
 */

#ifndef XML_H_
#define XML_H_

#include <collections/enumerator.h>

typedef struct xml_t xml_t;

/**
 * Simple enumerator based XML parser.
 *
 * An xml_t is a single node of the XML tree, but also serves as root node
 * and therefore the document.
 * This object has no destructor, the tree gets destroyed when all enumerator
 * instances get destroyed.
 */
struct xml_t {

	/**
	 * Create an enumerator over all children.
	 *
	 * Enumerated values must not be manipulated or freed.
	 *
	 * @return			enumerator over (xml_t* child, char *name, char *value)
	 */
	enumerator_t* (*children)(xml_t *this);

	/**
	 * Get an attribute value by its name.
	 *
	 * @param name		name of the attribute
	 * @return			attribute value, NULL if not found
	 */
	char *(*get_attribute)(xml_t *this, char *name);
};

/**
 * Create a xml instance.
 */
xml_t *xml_create(char *xml);

#endif /** XML_H_ @}*/
